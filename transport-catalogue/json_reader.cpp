#include "json_reader.h"


namespace json_reader {

	JSONReader::JSONReader(TransportCatalogue& db, MapRenderer& map_renderer, RequestHandler& handler, TransportRouter& router)
		: db_(db)
		, map_renderer_(map_renderer)
		, handler_(handler)
		, router_(router) {
	}


	void JSONReader::ReadCatalog(const Array& base_requests) {

		for (const auto& request : base_requests) {
			if (request.AsDict().at("type"s).AsString() == "Stop"s) {
				Stop stop;
				stop.name = request.AsDict().at("name"s).AsString();
				stop.north = request.AsDict().at("latitude"s).AsDouble();
				stop.east = request.AsDict().at("longitude"s).AsDouble();
				db_.AddStop(stop);
			}
		}

		for (const auto& request : base_requests) {
			if (request.AsDict().at("type"s).AsString() == "Stop"s) {
				for (const auto& distance : request.AsDict().at("road_distances"s).AsDict()) {
					db_.SetDistance(request.AsDict().at("name"s).AsString(), distance.first, distance.second.AsDouble());
				}
			}
		}

		for (const auto& request : base_requests) {
			if (request.AsDict().at("type"s).AsString() == "Bus"s) {
				Bus bus;
				bus.name = request.AsDict().at("name"s).AsString();
				bus.round_trip = request.AsDict().at("is_roundtrip"s).AsBool();
				for (const auto& stop : request.AsDict().at("stops"s).AsArray()) {
					bus.stops.push_back(stop.AsString());
				}
				db_.AddBus(bus);
			}
		}
	}


	Color ReadColor(const Node& node);

	void JSONReader::ReadRenderSettings(const Dict& settings) {
		RenderSettings res;

		res.width = settings.at("width"s).AsDouble();
		res.height = settings.at("height"s).AsDouble();
		res.padding = settings.at("padding"s).AsDouble();
		res.stop_radius = settings.at("stop_radius"s).AsDouble();
		res.line_width = settings.at("line_width"s).AsDouble();

		res.bus_label_font_size = static_cast<size_t>(settings.at("bus_label_font_size"s).AsInt());
		res.bus_label_offset.dx = settings.at("bus_label_offset"s).AsArray()[0].AsDouble();
		res.bus_label_offset.dy = settings.at("bus_label_offset"s).AsArray()[1].AsDouble();

		res.stop_label_font_size = static_cast<size_t>(settings.at("stop_label_font_size"s).AsInt());
		res.stop_label_offset.dx = settings.at("stop_label_offset"s).AsArray()[0].AsDouble();
		res.stop_label_offset.dy = settings.at("stop_label_offset"s).AsArray()[1].AsDouble();

		res.underlayer_color = ReadColor(settings.at("underlayer_color"s));
		res.underlayer_width = settings.at("underlayer_width"s).AsDouble();
		for (const auto& color : settings.at("color_palette"s).AsArray()) {
			res.color_palette.push_back(ReadColor(color));
		}

		map_renderer_.SetSettings(res);
	}


	void JSONReader::ReadRequests(const Array& stat_requests) {

		for (const auto& request : stat_requests) {
			const auto& req = request.AsDict();

			RequestType type;
			int id;
			string name, route_from, route_to;

			id = req.at("id").AsInt();

			if (req.at("type").AsString() == "Bus"s) {
				type = RequestType::BUS;
			}
			else if (req.at("type").AsString() == "Stop"s) {
				type = RequestType::STOP;
			}
			else if (req.at("type").AsString() == "Map"s) {
				type = RequestType::MAP;
			}
			else if (req.at("type").AsString() == "Route"s) {
				type = RequestType::ROUTE;
			}
			else {
				throw logic_error("JSONReader::ReadRequests: unknown request type"s);
			}

			if (type == RequestType::BUS || type == RequestType::STOP) {
				name = req.at("name"s).AsString();
			}

			if (type == RequestType::ROUTE) {
				route_from = req.at("from"s).AsString();
				route_to = req.at("to"s).AsString();
			}

			handler_.AddRequest({ name, id, type, route_from, route_to });
		}
	}

	void JSONReader::ReadRoutingSettings(const Dict& routing_settings) {
		router_.SetSettings(
			routing_settings.at("bus_wait_time"s).AsInt(),
			routing_settings.at("bus_velocity"s).AsDouble());
	}

	void JSONReader::Load(istream& is) {
		const json::Document doc = json::Load(is);

		ReadCatalog(doc.GetRoot().AsDict().at("base_requests"s).AsArray());
		ReadRenderSettings(doc.GetRoot().AsDict().at("render_settings").AsDict());
		ReadRequests(doc.GetRoot().AsDict().at("stat_requests"s).AsArray());
		ReadRoutingSettings(doc.GetRoot().AsDict().at("routing_settings"s).AsDict());
	}


	Color ReadColor(const Node& node) {
		if (node.IsString()) {
			return node.AsString();
		}
		else {
			const auto r = static_cast<uint8_t>(node.AsArray()[0].AsDouble());
			const auto g = static_cast<uint8_t>(node.AsArray()[1].AsDouble());
			const auto b = static_cast<uint8_t>(node.AsArray()[2].AsDouble());

			if (node.AsArray().size() == 3) {
				return Rgb(r, g, b);
			}
			else {
				const auto a = node.AsArray()[3].AsDouble();
				return Rgba(r, g, b, a);
			}
		}
	}

}

