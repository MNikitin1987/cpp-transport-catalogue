#include "input_reader.h"

#include <iostream>
#include <string>
#include <vector>

#include "transport_catalogue.h"

using namespace std;


Stop ParseStop(const string& buff) {
	Stop res;
	size_t begin, end;

	begin = 5;
	end = buff.find_first_of(':');
	res.name = buff.substr(begin, end - begin);

	begin = end + 2;
	end = buff.find_first_of(',');
    res.coords.lat = stod(buff.substr(begin, end - begin));
	begin = end + 2;
    res.coords.lng = stod(buff.substr(begin, end - begin));

	return res;
}


void ParseDistance(TransportCatalogue& cat, const string& line) {
	string_view text(line);
	text.remove_prefix(5);

	string_view name1 = text.substr(0, text.find_first_of(':'));

	size_t pos = text.find(',');
	pos = text.find(',', pos + 1);
	if (pos == text.npos) {
		return;
	}

	text.remove_prefix(pos + 2);

	while (true) {
		pos = text.find('m');
		if (pos == text.npos) {
			break;
		}

		size_t distance = stoul(string(text.substr(0, pos)));
		text.remove_prefix(pos + 5);

		pos = text.find(',');
		string_view name2 = text.substr(0, pos);
		cat.SetDistance(name1, name2, distance);
		if (pos == text.npos) {
			break;
		}
		text.remove_prefix(pos + 2);
	}

}


Bus ParseBus(const string& line) {
	Bus res;

	string_view buff(line);
	size_t pos;

	pos = 4;
	buff.remove_prefix(pos);

	pos = buff.find(':');
	res.name = buff.substr(0, pos);

	buff.remove_prefix(pos + 1);

	res.iscircled = buff.find('>') == buff.npos;

	while (true) {
		buff.remove_prefix(1);

		if (res.iscircled) {
			pos = buff.find('-');
		}
		else {
			pos = buff.find('>');
		}

		res.stops.push_back(buff.substr(0, pos - 1));
		buff.remove_prefix(pos + 1);
		if (pos == buff.npos) {
			break;
		}

	}

	return res;
}


TransportCatalogue ReadCatalog(istream& is) {
	TransportCatalogue result;

	size_t lines_num;
	is >> lines_num;
	{
		string null;
		getline(is, null);
	}

	vector<string> buses;
	vector<string> stops;

	for (size_t i = 0; i < lines_num; ++i) {
		string buff;
		getline(is, buff);

		if (buff.substr(0, 3) == "Bus"s) {
			buses.push_back(move(buff));
			continue;
		}

		const auto stop = ParseStop(buff);
		result.AddStop(stop);
		stops.push_back(move(buff));
	}


	for (string& line : stops) {
		ParseDistance(result, line);
	}


	for (string& line : buses) {
		const auto bus = ParseBus(line);
		result.AddBus(bus);
	}

	return result;
}
