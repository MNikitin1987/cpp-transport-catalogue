
#include "stat_reader.h"

#include <iostream>
#include <string>
#include <iomanip>
#include <vector>

#include "transport_catalogue.h"

using namespace std;


void ProcessBusQuery(string& query_text, TransportCatalogue& cat, ostream& os) {
	string_view bus_name(query_text);

	os << "Bus "s << bus_name << ": "s;
	if (!cat.CheckBus(bus_name)) {
		os << "not found\n"s;
		return;
	}

	os << cat.GetStopCountTotal(bus_name) << " stops on route, "s;
	os << cat.GetStopCountUnique(bus_name) << " unique stops, "s;

	const auto distance = cat.GetRouteLength(bus_name);
	os << setprecision(6);
	os << static_cast<double>(distance.first) << " route length, "s;
	os << distance.second << " curvature\n";
}

void ProcessStopQuery(string& query_text, TransportCatalogue& cat, ostream& os) {
	string_view stop_name(query_text);
	os << "Stop "s << stop_name << ": "s;
	if (!cat.CheckStop(stop_name)) {
		os << "not found\n"s;
		return;
	}
	const auto buses = cat.GetBuses(stop_name);
	if (buses.empty()) {
		os << "no buses"s;
	}
	else {
		os << "buses"s;
		for (const auto& bus : buses) {
			os << " "s << bus;
		}
	}
	os << "\n"s;
}

void ProcessQuery(istream& is, TransportCatalogue& cat, ostream& os) {
	size_t lines_num;
	is >> lines_num;
	string null;
	getline(is, null);

	for (size_t i = 0; i < lines_num; ++i) {
		Query query;
		string buff;
		getline(is, buff);
		if (buff.substr(0, 3) == "Bus"s) {
			query.is_bus = true;
			query.text = buff.substr(4);
		}
		else {
			query.is_bus = false;
			query.text = buff.substr(5);
		}
        if (query.is_bus) {
			ProcessBusQuery(query.text, cat, os);
		}
		else {
			ProcessStopQuery(query.text, cat, os);
		}
	}
}