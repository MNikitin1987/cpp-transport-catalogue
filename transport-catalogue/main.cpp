#include <iostream>

#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

#include "geo.h"


int main() {
	TransportCatalogue catalog = ReadCatalog(cin);
	ProcessQuery(cin, catalog, cout);
}