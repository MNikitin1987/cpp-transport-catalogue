#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"
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
		JSONReader(TransportCatalogue& db, MapRenderer& map_renderer, RequestHandler& handler);
		void Load(istream& is);

	private:
		void ReadCatalog(const json::Array& base_requests);
		void ReadRenderSettings(const json::Dict& settings);
		void ReadRequests(const json::Array& stat_requests);

	private:
		TransportCatalogue& db_;
		MapRenderer& map_renderer_;
		RequestHandler& handler_;
	};
}