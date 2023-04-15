#include "request_handler.h"

namespace req_handler {

	RequestHandler::RequestHandler(const TransportCatalogue& db, const MapRenderer& renderer)
		: db_(db)
		, renderer_(renderer) {
	}

	void RequestHandler::AddRequest(const RequestData req) {
		requests_.push_back(req);
	}

	void RequestHandler::ProcessRequests(ostream& os) const {
		Array res;

		for (const auto& req : requests_) {
			if (req.type == RequestType::BUS) {
				res.push_back(ProcessBusRequest(req));
			}
			else if (req.type == RequestType::STOP) {
				res.push_back(ProcessStopRequest(req));
			} 
			else if (req.type == RequestType::MAP) {
				res.push_back(ProcessMapRequest(req));
			}
		}

		PrintNode(res, os);
	}

	Node RequestHandler::ProcessBusRequest(const RequestData& req) const {
		Dict res;

		res.insert({ "request_id"s, req.id });

		if (!db_.CheckBus(req.name)) {
			res.insert({ "error_message"s, "not found"s });
			return res;
		}

		const auto distance = db_.GetRouteLength(req.name);
		res.insert({ "curvature"s, distance.second });
		res.insert({ "route_length"s, static_cast<int>(distance.first) });
		res.insert({ "stop_count"s, static_cast<int>(db_.GetStopCountTotal(req.name)) });
		res.insert({ "unique_stop_count"s, static_cast<int>(db_.GetStopCountUnique(req.name)) });

		return res;
	}

	Node RequestHandler::ProcessStopRequest(const RequestData& req) const {
		Dict res;

		res.insert({ "request_id"s, req.id });

		if (!db_.CheckStop(req.name)) {
			res.insert({ "error_message"s, "not found"s });
			return res;
		}

		Array buses;
		const auto bus_names = db_.GetBusesAtStop(req.name);
		for (const auto& bus : bus_names) {
			buses.push_back(string(bus));
		}
		
		res.insert({ "buses"s, buses });

		return res;
	}

	Node RequestHandler::ProcessMapRequest(const RequestData& req) const {
		Dict res;

		res.insert({ "request_id"s, req.id });

		const auto doc = renderer_.RenderMap();

		ostringstream svg_doc;
		doc.Render(svg_doc);
		string svg_str = svg_doc.str();
		Node svg_print(svg_str);

		res.insert({"map"s, svg_print});

		return res;
	}
}