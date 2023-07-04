#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json.h"
#include "json_builder.h"
#include "transport_router.h"

#include <vector>
#include <iostream>
#include <sstream>

using namespace std;
using namespace renderer;
using namespace json;
using namespace graph;

namespace req_handler {

	enum class RequestType {
		STOP,
		BUS,
		MAP,
		ROUTE
	};

	struct RequestData {
		string name;
		int id;
		RequestType type;
		string from;
		string to;
	};

	class RequestHandler {

	public:
		RequestHandler(const TransportCatalogue& db, const MapRenderer& renderer, TransportRouter& router);
		void AddRequest(const RequestData req);
		void ProcessRequests(ostream& out) const;

	private:
		Node ProcessBusRequest(const RequestData& req) const;
		Node ProcessStopRequest(const RequestData& req) const;
		Node ProcessMapRequest(const RequestData& req) const;
		Node ProcessRouteRequest(const RequestData& req) const;

	private:
		const TransportCatalogue& db_;
		const MapRenderer& renderer_;
		TransportRouter& router_;
		vector<RequestData> requests_;
	};

}
 