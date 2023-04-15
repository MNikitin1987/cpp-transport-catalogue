#pragma once

#include <string>
#include <vector>
#include <set>
#include "geo.h"

namespace domain {
    using namespace std;
	using namespace geo;

	struct Stop {
		string name;
        Coordinates coords;
	};

	struct Bus {
		bool iscircled;
		string name;
		vector<string_view> stops;
		set<string_view> unique_stops;
	};
}