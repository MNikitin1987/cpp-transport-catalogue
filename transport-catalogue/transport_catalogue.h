#pragma once

#include <iostream>
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>
#include <algorithm>

#include "geo.h"
#include "domain.h"

using namespace std;
using namespace geo;
using namespace domain;


namespace tc {
	class PairHasher {
	public:
		size_t operator()(const pair<string_view, string_view> stops) const {
			return hasher_(stops.first) * 37 + hasher_(stops.second);
		}
	private:
		hash<string_view> hasher_;
	};


	class TransportCatalogue {

	public:
		TransportCatalogue() = default;
		void AddBus(const Bus& bus);
		void AddStop(const Stop& stop);
		void SetDistance(const string_view stop1, const string_view stop2, const size_t distance);

		bool CheckBus(string_view name) const;
		bool CheckStop(string_view name) const;
		size_t GetStopCountTotal(string_view bus_name) const;
		size_t GetStopCountUnique(string_view bus_name) const;
		pair<size_t, double> GetRouteLength(string_view bus_name) const;
		vector<string_view> GetBusesAtStop(string_view stop_name) const;

		vector<Coordinates> GetCoordinates() const;
		vector<string_view> GetBusNamesSorted() const;
		bool IsBusRound(const string_view bus_name) const;
		vector<string_view> GetStopNamesSorted() const;
		vector<string_view> GetStopNamesSortedNoCheck() const;
		Coordinates GetStopCoords(const string_view stop_name) const;

		vector<Coordinates> GetPathCoords(const string_view bus_name) const;
		vector<Coordinates> GetPathCoordsOneWay(const string_view bus_name) const;
		vector<string_view> GetStopNames(const string_view bus_name) const;
		size_t GetDistance(string_view from, string_view to) const;
		vector<pair<pair<string_view, string_view>, size_t>> GetAllDistances() const;

	private:
		deque<Bus> buses_;
		deque<Stop> stops_;
		unordered_map<string_view, const Stop*> name_to_stop_;
		unordered_map<string_view, const Bus*> name_to_bus_;
		unordered_map<string_view, set<string_view>> stop_buses_;
		unordered_map<pair<string_view, string_view>, size_t, PairHasher> stop_distances_;
	};

}