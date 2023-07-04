#include "domain.h"

namespace domain {

	size_t NumFromName(const vector<string_view>& all_names, const string_view& name) {
		return lower_bound(all_names.begin(), all_names.end(), name) - all_names.begin();
	}
}
