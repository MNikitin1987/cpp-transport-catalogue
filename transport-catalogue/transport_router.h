#pragma once

#include <memory>
#include <algorithm>
#include <map>

#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"

using namespace graph;
using namespace tc;


struct RoutingSettings {
	double stop_wait_time = 0.0;
	double bus_velocity = 0.0;
};



struct ItemInfo {
	string_view name;
	size_t span_count = 0;
	double time = 0;
};

struct PathInfo {
	vector<ItemInfo> items;
	double total_time;
};

class TransportRouter {

public:
	TransportRouter(const TransportCatalogue& db);

	void SetSettings(int bus_wait_time, double bus_velocity);
	RoutingSettings GetSettings() const;
	optional<PathInfo> GetPath(const string_view& from, const string_view& to);

private:
	void MakeGraph();
	void AddEdge(size_t from, size_t to, double weight, string_view name, size_t span_count);

private:
	const TransportCatalogue& db_;
	RoutingSettings settings_;
	unique_ptr<DirectedWeightedGraph<double>> graph_;
	unique_ptr<Router<double>> router_;
	vector<string_view> all_stops_;
	vector<ItemInfo> edges_info_;
};