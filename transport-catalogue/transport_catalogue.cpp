#include "transport_catalogue.h"

#include <deque>
#include <string>
#include <unordered_map>
#include <set>

#include "geo.h"

void TransportCatalogue::AddStop(const Stop& stop) {
	stops_.push_back(stop);
	name_to_stop_.insert({ stops_.back().name, &stops_.back() });
}

void TransportCatalogue::AddBus(const Bus& bus) {

	buses_.push_back(bus);

	for (auto& stop : buses_.back().stops) {
		stop = name_to_stop_.find(stop)->first;
	}
	buses_.back().unique_stops = { buses_.back().stops.begin(), buses_.back().stops.end() };

	name_to_bus_.insert({ buses_.back().name, &buses_.back() });

	for (const auto& stop : buses_.back().stops) {
		stop_buses_[stop].insert(name_to_bus_.find(buses_.back().name)->first);
	}
}

void TransportCatalogue::SetDistance(const string_view stop1, const string_view stop2, const size_t distance) {
	auto st1 = name_to_stop_.find(stop1)->first;
	auto st2 = name_to_stop_.find(stop2)->first;
	stop_distances_.insert({ {st1, st2}, distance });
}


bool TransportCatalogue::CheckBus(string_view bus_name) {
	return name_to_bus_.find(bus_name) != name_to_bus_.end();
}

bool TransportCatalogue::CheckStop(string_view stop_name) {
	return name_to_stop_.find(stop_name) != name_to_stop_.end();
}

size_t TransportCatalogue::GetStopCountTotal(string_view bus_name) {
	auto& bus = *name_to_bus_.at(bus_name);
	if (bus.circled) {
		return bus.stops.size() * 2 - 1;
	}
	else {
		return bus.stops.size();
	}
}

size_t TransportCatalogue::GetStopCountUnique(string_view bus_name) {
	auto& bus = *name_to_bus_.at(bus_name);
	return bus.unique_stops.size();
}
pair<size_t, double> TransportCatalogue::GetRouteLength(string_view bus_name) {
	size_t length_road = 0;
	double length_geo = 0.0;

	auto& bus = *name_to_bus_.at(bus_name);

	Coordinates geo_stop1, geo_stop2;
	string_view name_stop1, name_stop2;
	bool first_run = true;

	for (auto& stop : bus.stops) {

		name_stop2 = name_to_stop_[stop]->name;
		geo_stop2.lat = name_to_stop_[stop]->x;
		geo_stop2.lng = name_to_stop_[stop]->y;

		if (first_run) {
			geo_stop1.lat = geo_stop2.lat;
			geo_stop1.lng = geo_stop2.lng;
			name_stop1 = name_stop2;
			first_run = false;
			continue;
		}

		const auto way_to = make_pair(name_stop1, name_stop2);
		const auto way_back = make_pair(name_stop2, name_stop1);

		size_t length;

		if (stop_distances_.count(way_to) != 0) {
			length = stop_distances_.at(way_to);
		}
		else {
			length = stop_distances_.at(way_back);
		}

		if (!bus.circled) {
			length_geo += ComputeDistance(geo_stop1, geo_stop2);
			length_road += length;
		}
		else {
			length_geo += 2.0 * ComputeDistance(geo_stop1, geo_stop2);
			length_road += length;
			if (stop_distances_.count(way_back) != 0) {
				length = stop_distances_.at(way_back);
			}
			else {
				length = stop_distances_.at(way_to);
			}
			length_road += length;
		}

		geo_stop1 = geo_stop2;
		name_stop1 = name_stop2;
	}


	return { length_road, static_cast<double>(length_road) / length_geo };
}

vector<string_view> TransportCatalogue::GetBuses(string_view stop_name) {
	vector<string_view> result;

	if (stop_buses_.find(stop_name) == stop_buses_.end()) {
		return result;
	}

	for (const auto& bus : stop_buses_.at(stop_name)) {
		result.push_back(bus);
	}

	return result;
}
