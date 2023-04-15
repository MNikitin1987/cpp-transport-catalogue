#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_reader.h"

using namespace std;
using namespace json_reader;
using namespace renderer;

int main() {
	TransportCatalogue cat;
	MapRenderer renderer(cat);
	RequestHandler handler(cat, renderer);
	JSONReader reader(cat, renderer, handler);
	reader.Load(cin);
	handler.ProcessRequests(cout);
}
