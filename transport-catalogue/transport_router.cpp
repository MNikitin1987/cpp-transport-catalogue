#include "transport_router.h"


TransportRouter::TransportRouter(const TransportCatalogue& db)
	: db_(db) {
}

void TransportRouter::SetSettings(int bus_wait_time, double bus_velocity) {
	settings_.stop_wait_time = bus_wait_time;
	settings_.bus_velocity = bus_velocity;
}

size_t TransportRouter::StopFromName(string_view stop_name) {
	return lower_bound(all_stops_.begin(), all_stops_.end(), stop_name) - all_stops_.begin();
}

void TransportRouter::AddEdge(size_t from, size_t to, double weight, string_view name, size_t span_count) {
	graph_->AddEdge({ from, to, weight });
	edges_info_.push_back({name, span_count, weight});
}

void TransportRouter::MakeGraph() {
	all_stops_ = db_.GetStopNamesSorted();

	auto IncidenceFromStopIn = [](size_t stop_num) {
		return stop_num * 2;
	};

	auto IncidenceFromStopOut = [IncidenceFromStopIn](size_t stop_num) {
		return IncidenceFromStopIn(stop_num) + 1;
	};


	auto DistanceToWeight = [this](size_t distance_meters) {
		constexpr double mins_in_hour = 60.0;
		constexpr double meters_in_km = 1000.0;
		return mins_in_hour * distance_meters / meters_in_km / settings_.bus_velocity;
	};

	auto AddBusEdge = [&](string_view stop_from, string_view stop_to, double weight, string_view bus, size_t span_count) {
		const auto incidence_out1 = IncidenceFromStopOut(StopFromName(stop_from));
		const auto incidence_in2 = IncidenceFromStopIn(StopFromName(stop_to));
		AddEdge(incidence_out1, incidence_in2, weight, bus, span_count);
		return;
	};


	graph_ = make_unique<DirectedWeightedGraph<double>>(all_stops_.size() * 2);

	for (size_t i = 0; i < all_stops_.size(); ++i) {
		AddEdge(IncidenceFromStopIn(i), IncidenceFromStopOut(i), settings_.stop_wait_time, all_stops_[i], 0);
	}


	for (const auto& bus : db_.GetBusNamesSorted()) {
		const auto stops = db_.GetStopNames(bus);

		//if (db_.IsBusRound(bus) && stops.front() != stops.back()) {
		//	throw logic_error("TransportRouter::MakeGraph: front != back for round_trip"s);
		//}


		for (size_t from = 0; from < stops.size() - 1; ++from) {
			double weight_forward = 0;
			size_t span_cnt_forward = 0;
			double weight_back = 0;
			size_t span_cnt_back = 0;
			auto prev_stop = from;
			for (size_t to = from + 1; to < stops.size(); ++to) {
				if (db_.IsBusRound(bus)) {
					if (from == 0 && to == stops.size() - 1) {
						continue;
					}
				}
				else {
					const size_t last_stop = stops.size() - 1;
					const size_t back_prev = last_stop - prev_stop;
					const size_t back_to = last_stop - to;
					const size_t back_from = last_stop - from;
					weight_back += DistanceToWeight(db_.GetRoadLength(stops[back_prev], stops[back_to]));
					++span_cnt_back;
					AddBusEdge(stops[back_from], stops[back_to], weight_back, bus, span_cnt_back);
				}

				weight_forward += DistanceToWeight(db_.GetRoadLength(stops[prev_stop], stops[to]));
				++span_cnt_forward;
				AddBusEdge(stops[from], stops[to], weight_forward, bus, span_cnt_forward);
				prev_stop = to;
			}
		}
	}
}


optional<PathInfo> TransportRouter::GetPath(const string_view& from, const string_view& to) {
	optional<PathInfo> res;

	if (!router_) {
		MakeGraph();
		router_ = make_unique<Router<double>>(*graph_);
	}

	auto IncidenceFromStopName = [this](string_view stop_name) {
		return StopFromName(stop_name) * 2;
	};

	if (!binary_search(all_stops_.begin(), all_stops_.end(), from) ||
		!binary_search(all_stops_.begin(), all_stops_.end(), to)) {
		return res;
	}

	const auto path = router_->BuildRoute(IncidenceFromStopName(from), IncidenceFromStopName(to));

	if (!path.has_value()) {
		return res;
	}

	PathInfo data;

	data.total_time = path->weight;

	for (const auto edge : path->edges) {
		data.items.push_back({
			edges_info_[edge].name,
			edges_info_[edge].span_count,
			edges_info_[edge].time });
	}

	res = move(data);

	return res;
}