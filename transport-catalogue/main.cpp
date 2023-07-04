#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json_reader.h"
#include "transport_router.h"
#include "serialization.h"

#include <fstream>
#include <iostream>
#include <string_view>

using namespace std;
using namespace json_reader;
using namespace renderer;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}



int main(int argc, char* argv[]) {

    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    tc::TransportCatalogue cat;

    MapRenderer renderer(cat);
    TransportRouter router(cat);
    Serializator serializator(cat, renderer, router);
    RequestHandler handler(cat, renderer, router);
    JSONReader reader(cat, renderer, handler, router, serializator);


    if (mode == "make_base"sv) {
        reader.LoadMakeBase(cin);
        serializator.Serialize();
    }
    else if (mode == "process_requests"sv) {
        reader.LoadProcessRequests(cin);
        serializator.DeSerialize();
        renderer.InitProjector();
        handler.ProcessRequests(cout);
    }
    else {
        PrintUsage();
        return 1;
    }
}