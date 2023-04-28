#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json.h"
#include "json_builder.h"

#include <vector>
#include <iostream>
#include <sstream>

using namespace std;
using namespace renderer;
using namespace json;

namespace req_handler {

	enum class RequestType {
		STOP,
		BUS,
		MAP
	};

	struct RequestData {
		string name;
		int id;
		RequestType type;
	};

	class RequestHandler {
	public:
		RequestHandler(const TransportCatalogue& db, const MapRenderer& renderer);
		void AddRequest(const RequestData req);
		void ProcessRequests(ostream& out) const;
	private:
		Node ProcessBusRequest(const RequestData& req) const;
		Node ProcessStopRequest(const RequestData& req) const;
		Node ProcessMapRequest(const RequestData& req) const;
	private:
		const TransportCatalogue& db_;
		const MapRenderer& renderer_;
		vector<RequestData> requests_;
	};
}