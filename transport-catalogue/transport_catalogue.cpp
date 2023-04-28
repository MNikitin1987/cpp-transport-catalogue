#include "transport_catalogue.h"

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

bool TransportCatalogue::CheckBus(string_view bus_name) const {
	return name_to_bus_.find(bus_name) != name_to_bus_.end();
}

bool TransportCatalogue::CheckStop(string_view stop_name) const {
	return name_to_stop_.find(stop_name) != name_to_stop_.end();
}

size_t TransportCatalogue::GetStopCountTotal(string_view bus_name) const {
	auto& bus = *name_to_bus_.at(bus_name);
	if (bus.circled) {
		return bus.stops.size() * 2 - 1;
	}
	else {
		return bus.stops.size();
	}
}

size_t TransportCatalogue::GetStopCountUnique(string_view bus_name) const {
	auto& bus = *name_to_bus_.at(bus_name);
	return bus.unique_stops.size();
}

pair<size_t, double> TransportCatalogue::GetRouteLength(string_view bus_name) const {
	size_t length_road = 0;
	double length_geo = 0.0;

	auto& bus = *name_to_bus_.at(bus_name);

	Coordinates geo_stop1{}, geo_stop2{};
	string_view name_stop1, name_stop2;
	bool first_run = true;

	for (const auto& stop : bus.stops) {

		name_stop2 = name_to_stop_.at(stop)->name;
		geo_stop2.lat = name_to_stop_.at(stop)->north;
		geo_stop2.lng = name_to_stop_.at(stop)->east;

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


vector<string_view> TransportCatalogue::GetBusesAtStop(string_view stop_name) const {
	vector<string_view> result;

	if (stop_buses_.find(stop_name) == stop_buses_.end()) {
		return result;
	}

	for (const auto& bus : stop_buses_.at(stop_name)) {
		result.push_back(bus);
	}

	return result;
}

vector<string_view> TransportCatalogue::GetBusNamesSorted() const {
	vector<string_view> res;

	for (const auto& bus : buses_) {
		string_view name(bus.name);
		res.push_back(move(name));
	}

	sort(res.begin(), res.end());

	return res;
}

vector<Coordinates> TransportCatalogue::GetCoordinates() const {
	vector<Coordinates> res;

	size_t stops_num = 0;
	for (const auto& [_, bus] : name_to_bus_) {
		stops_num += bus->stops.size();
	}
	res.reserve(stops_num);

	for (const auto& [_, bus] : name_to_bus_) {
		for (const auto& stop : bus->stops) {
			//if (!stop_buses_.at(stop).empty()) {//! 
			res.push_back({ name_to_stop_.at(stop)->north, name_to_stop_.at(stop)->east });
			//}
		}
	}

	sort(
		res.begin(),
		res.end(),
		[](Coordinates& l, Coordinates& r) {
			return l.lat + l.lng < r.lat + r.lng;
		});

	res.erase(
		unique(res.begin(), res.end()),
		res.end());

	return res;
}

vector<Coordinates> TransportCatalogue::GetPath(const string_view bus_name, const bool for_bus_name) const {
	vector<Coordinates> res;

	const auto& bus = *name_to_bus_.at(bus_name);

	for (auto it = bus.stops.begin(); it != bus.stops.end(); ++it) {
		res.push_back({ name_to_stop_.at(*it)->north,name_to_stop_.at(*it)->east });
	}

	if (bus.circled && !for_bus_name) {
		for (auto it = bus.stops.rbegin() + 1; it != bus.stops.rend(); ++it) {
			res.push_back({ name_to_stop_.at(*it)->north,name_to_stop_.at(*it)->east });
		}
	}

	return res;
}

bool TransportCatalogue::GetBusRound(const string_view bus_name) const {
	return !name_to_bus_.at(bus_name)->circled;
}

vector<string_view> TransportCatalogue::GetStopNamesSorted() const {
	vector<string_view> res;

	for (const auto& stop : stops_) {
		string_view stop_name(stop.name);
		if (stop_buses_.find(stop_name) != stop_buses_.end()) {
			res.push_back(move(stop_name));
		}
	}

	sort(res.begin(), res.end());

	return res;
}

Coordinates TransportCatalogue::GetStopCoords(const string_view stop_name) const {
	return { name_to_stop_.at(stop_name)->north, name_to_stop_.at(stop_name)->east };
}