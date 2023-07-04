#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "serialization.h"
#include "json.h"

#include <iostream>
#include <string>
#include <iomanip>

using namespace std;
using namespace json;
using namespace renderer;
using namespace req_handler;


namespace json_reader {


	class JSONReader {

	public:
		JSONReader(TransportCatalogue& db, MapRenderer& map_renderer, RequestHandler& handler, TransportRouter& router, Serializator& serializator);
		void LoadMakeBase(istream& is);
		void LoadProcessRequests(istream& is);

	private:
		void ReadCatalog(const json::Array& base_requests);
		void ReadRenderSettings(const json::Dict& settings);
		void ReadRequests(const json::Array& stat_requests);
		void ReadRoutingSettings(const json::Dict& routing_settings);

	private:
		TransportCatalogue& db_;
		MapRenderer& map_renderer_;
		RequestHandler& handler_;
		TransportRouter& router_;
		Serializator& serializator_;
	};
}