#pragma once

#include "transport_catalogue.h"

#include <iostream>

using namespace std;


void ProcessQuery(istream& is, TransportCatalogue& catalog, ostream& os);

struct Query {
	bool is_bus;
	string text;
};