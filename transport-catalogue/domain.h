#pragma once

#include <string>
#include <vector>
#include <set>


namespace domain {

	using namespace std;

	struct Stop {
		string name;
		double north;
		double east;
	};

	struct Bus {
		bool round_trip;
		string name;
		vector<string_view> stops;
		set<string_view> unique_stops;
	};


	size_t NumFromName(const vector<string_view>& all_names, const string_view& name);
}