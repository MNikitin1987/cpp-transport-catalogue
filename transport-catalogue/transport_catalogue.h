#pragma once

#include <deque>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>

#include "geo.h"

using namespace std;

class PairHasher {
public:
	std::size_t operator()(const pair<string_view, string_view> stops) const {
		return hasher_(stops.first) * 37 + hasher_(stops.second);
	}
private:
	std::hash<string_view> hasher_;
};

struct Stop {
	string name;
	double x;
	double y;
};

struct Bus {
	bool circled;
	string name;
	vector<string_view> stops;
	set<string_view> unique_stops;
};

class TransportCatalogue {

public:
	TransportCatalogue() = default;
	void AddBus(const Bus& bus);
	void AddStop(const Stop& stop);
	void SetDistance(const string_view stop1, const string_view stop2, const size_t distance);

	bool CheckBus(string_view name);
	bool CheckStop(string_view name);
	size_t GetStopCountTotal(string_view bus_name);
	size_t GetStopCountUnique(string_view bus_name);
	pair<size_t, double> GetRouteLength(string_view bus_name);
	vector<string_view> GetBuses(string_view stop_name);

private:
	deque<Bus> buses_;
	deque<Stop> stops_;
	unordered_map<string_view, const Stop*> name_to_stop_;
	unordered_map<string_view, const Bus*> name_to_bus_;
	unordered_map<string_view, set<string_view>> stop_buses_;
	unordered_map<pair<string_view, string_view>, size_t, PairHasher> stop_distances_;
};