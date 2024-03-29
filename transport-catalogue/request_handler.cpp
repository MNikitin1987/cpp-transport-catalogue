#include "request_handler.h"


namespace req_handler {

	RequestHandler::RequestHandler(const TransportCatalogue& db, const MapRenderer& renderer, TransportRouter& router)
		: db_(db)
		, renderer_(renderer)
		, router_(router) {
	}


	void RequestHandler::AddRequest(const RequestData req) {
		requests_.push_back(req);
	}


	void RequestHandler::ProcessRequests(ostream& os) const {
		json::Builder res;

		res.StartArray();

		for (const auto& req : requests_) {
			if (req.type == RequestType::BUS) {
				res.Value(ProcessBusRequest(req));
			}
			else if (req.type == RequestType::STOP) {
				res.Value(ProcessStopRequest(req));
			} 
			else if (req.type == RequestType::MAP) {
				res.Value(ProcessMapRequest(req));
			}
			else if (req.type == RequestType::ROUTE) {
				res.Value(ProcessRouteRequest(req));
			}
		}

		Print(json::Document{ res.EndArray().Build()}, os);
	}


	Node RequestHandler::ProcessBusRequest(const RequestData& req) const {
		json::Builder res;

		res.StartDict();
		res.Key("request_id"s)			.Value(req.id);

		if (!db_.CheckBus(req.name)) {
			res.Key("error_message"s)	.Value("not found"s);
			return res.EndDict().Build();
		}

		const auto distance = db_.GetRouteLength(req.name);
		res.Key("curvature"s)			.Value(distance.second);
		res.Key("route_length"s)		.Value(static_cast<int>(distance.first));
		res.Key("stop_count"s)			.Value(static_cast<int>(db_.GetStopCountTotal(req.name)));
		res.Key("unique_stop_count"s)	.Value(static_cast<int>(db_.GetStopCountUnique(req.name)));

		return res.EndDict().Build();
	}


	Node RequestHandler::ProcessStopRequest(const RequestData& req) const {
		json::Builder res;

		res.StartDict();
		res.Key("request_id"s)			.Value(req.id);

		if (!db_.CheckStop(req.name)) {
			res.Key("error_message"s)	.Value("not found"s);
			res.EndDict();
			return res.Build();
		}

		res.Key("buses"s).StartArray();
		const auto bus_names = db_.GetBusesAtStop(req.name);
		for (const auto& bus : bus_names) {
			res.Value(string(bus));
		}

		return res.EndArray().EndDict().Build();
	}


	Node RequestHandler::ProcessMapRequest(const RequestData& req) const {
		json::Builder res;

		res.StartDict();
		res.Key("request_id"s).Value(req.id);

		ostringstream svg_doc;
		renderer_.RenderMap().Render(svg_doc);
		Node svg_print(svg_doc.str());

		res.Key("map"s).Value(svg_print);
		
		return res.EndDict().Build();
	}

	Node RequestHandler::ProcessRouteRequest(const RequestData& req) const {
		json::Builder res;
		res.StartDict();
		res.Key("request_id"s).Value(req.id);

		const auto path = router_.GetPath(req.from, req.to);

		if (!path.has_value()) {
			res.Key("error_message"s).Value("not found"s);
			return res.EndDict().Build();
		}

		res.Key("total_time"s).Value(path->total_time);

		res.Key("items"s).StartArray();

		for (const auto& item : path->items) {
			res.StartDict();

			res.Key("time"s).Value(item.time);
			res.Key("type"s).Value(item.span_count != 0 ? "Bus"s : "Wait"s);
			res.Key(item.span_count != 0 ? "bus"s : "stop_name"s).Value(string{ item.name});

			if (item.span_count != 0) {
				res.Key("span_count"s).Value(static_cast<int>(item.span_count));
			}

			res.EndDict();
		}
		res.EndArray();


		return res.EndDict().Build();
	}

} 
