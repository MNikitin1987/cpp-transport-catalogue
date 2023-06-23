#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_reader.h"
#include "transport_router.h"

using namespace std;
using namespace json_reader;
using namespace renderer;

int main() {
	TransportCatalogue cat;
	MapRenderer renderer(cat);
	TransportRouter router(cat);

	RequestHandler handler(cat, renderer, router);

	JSONReader reader(cat, renderer, handler, router);
	reader.Load(cin);

	handler.ProcessRequests(cout);
}