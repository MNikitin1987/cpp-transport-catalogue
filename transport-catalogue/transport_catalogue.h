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
    Coordinates coords;
};

struct Bus {
	bool iscircled;
	string name;
	vector<string_view> stops;
	set<string_view> unique_stops;
    // ответ на "порекомендую организовать еще одну структуру, которая будет служить для возврата данных по запросу":
    // структуру добавлю при следующем внесении крупных изменений (спасибо за подсказку)
};

class TransportCatalogue {

public:
	TransportCatalogue() = default;
	void AddBus(const Bus& bus);
	void AddStop(const Stop& stop);
	void SetDistance(const string_view stop_from, const string_view stop_to, const size_t distance);

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