#include "serialization.h"
#include "geo.h"
#include <variant>

Serializator::Serializator(
    tc::TransportCatalogue& db,
    renderer::MapRenderer& renderer,
    TransportRouter& router)
	: db_(db)
    , renderer_(renderer)
    , router_(router) {
}

void Serializator::SetFile(const string_view& file) {
	file_ = file;
}



void Serializator::WriteStops() {
    
    for (const auto& stop_name : all_stops_) {
        
        pb::Stop stop;
        
        stop.set_name(move(string{stop_name}));

        geo::Coordinates coords = db_.GetStopCoords(stop_name);
        stop.set_north(coords.lat);
        stop.set_east(coords.lng);

        stop.set_num(domain::NumFromName(all_stops_, stop_name));

        *pb_.add_stops() = move(stop);
    }
}

void Serializator::WriteBuses() {

    for (const auto& bus_name : db_.GetBusNamesSorted()) {
        pb::Bus pb_bus;

        pb_bus.set_round_trip(db_.IsBusRound(bus_name));

        pb_bus.set_name(move(string{bus_name}));

        for (const auto& stop_name : db_.GetStopNames(bus_name)) {
            pb_bus.add_stops(NumFromName(all_stops_, stop_name));
        }

//repeated uint32 unique_stops = 4;
        *pb_.add_buses() = move(pb_bus);
    }
}

void Serializator::WriteDistances() {
   	for (const auto& [stops, distance] : db_.GetAllDistances()) {
        pb::Distance pb_distance;
        
        pb_distance.set_stop1(NumFromName(all_stops_, stops.first));
        pb_distance.set_stop2(NumFromName(all_stops_, stops.second));
        pb_distance.set_distance(distance);
        
        *pb_.add_distances() = move(pb_distance);
    }
}

void Serializator::WriteRenderSettings() {
    const auto& db_settings = renderer_.GetSettings();

    mr::RenderSettings pb_settings;

    pb_settings.set_width(db_settings.width);
    pb_settings.set_height(db_settings.height);
    pb_settings.set_padding(db_settings.padding);
    pb_settings.set_stop_radius(db_settings.stop_radius);
    pb_settings.set_line_width(db_settings.line_width);

    pb_settings.set_bus_label_font_size(db_settings.bus_label_font_size);
    pb_settings.set_bus_label_dx(db_settings.bus_label_offset.dx);
    pb_settings.set_bus_label_dy(db_settings.bus_label_offset.dy);

    pb_settings.set_stop_label_font_size(db_settings.stop_label_font_size);
    pb_settings.set_stop_label_dx(db_settings.stop_label_offset.dx);
    pb_settings.set_stop_label_dy(db_settings.stop_label_offset.dy);


    *pb_settings.mutable_underlayer_color() = PbColorFromSvg(db_settings.underlayer_color);
    pb_settings.set_underlayer_width(db_settings.underlayer_width);

    for (const auto& color : db_settings.color_palette) {
        *pb_settings.add_palette() = PbColorFromSvg(color);
    }

    *pb_.mutable_render_settings() = move(pb_settings);
}

void Serializator::WriteRouterSettings() {
    tr::Router router;
    tr::Settings settings;

    RoutingSettings db_settings = router_.GetSettings();

    settings.set_stop_wait_time(db_settings.stop_wait_time);
    settings.set_bus_velocity(db_settings.bus_velocity);

    *router.mutable_settings() = move(settings);
    *pb_.mutable_router() = move(router);
}



void Serializator::Serialize() {
    all_stops_ = db_.GetStopNamesSortedNoCheck();

    WriteStops();
    WriteBuses();
    WriteDistances();
    WriteRenderSettings();
    WriteRouterSettings();

	ofstream ofs(file_, ios::binary);
    pb_.SerializeToOstream(&ofs);
}



void Serializator::ReadStops() {
    for (size_t i = 0; i < pb_.stops_size(); ++i) {
        domain::Stop stop;
        stop.name = pb_.stops(i).name();
        stop.north = pb_.stops(i).north();
        stop.east = pb_.stops(i).east();
        db_.AddStop(move(stop));
    }
}

void Serializator::ReadBuses() {
    for (size_t i = 0; i < pb_.buses_size(); ++i) {
        domain::Bus bus;
        
        bus.name = pb_.buses(i).name();
		bus.round_trip = pb_.buses(i).round_trip();

        for (size_t j = 0; j < pb_.buses(i).stops_size(); ++j) {
			bus.stops.push_back(all_stops_[pb_.buses(i).stops(j)]);
		}

        db_.AddBus(move(bus));
    }
}

void Serializator::ReadDistances() {
    for (size_t i = 0; i < pb_.distances_size(); ++i) {
        db_.SetDistance(
            all_stops_[pb_.distances(i).stop1()],
            all_stops_[pb_.distances(i).stop2()],
            pb_.distances(i).distance());
    }
}

void Serializator::ReadRenderSettings() {
    renderer::RenderSettings db_settings;
    
    db_settings.width = pb_.render_settings().width();
    db_settings.height = pb_.render_settings().height();
    db_settings.padding = pb_.render_settings().padding();
    db_settings.stop_radius = pb_.render_settings().stop_radius();
    db_settings.line_width = pb_.render_settings().line_width();

    db_settings.bus_label_font_size = pb_.render_settings().bus_label_font_size();
    db_settings.bus_label_offset.dx = pb_.render_settings().bus_label_dx();
    db_settings.bus_label_offset.dy = pb_.render_settings().bus_label_dy();

    db_settings.stop_label_font_size = pb_.render_settings().stop_label_font_size();
    db_settings.stop_label_offset.dx = pb_.render_settings().stop_label_dx();
    db_settings.stop_label_offset.dy = pb_.render_settings().stop_label_dy();

    db_settings.underlayer_color = SvgColorFromPb(pb_.render_settings().underlayer_color());
    db_settings.underlayer_width = pb_.render_settings().underlayer_width();

    for (size_t i = 0; i < pb_.render_settings().palette_size(); ++i) {
        db_settings.color_palette.push_back(SvgColorFromPb(pb_.render_settings().palette(i)));
    }

    renderer_.SetSettings(db_settings);
}

void Serializator::ReadRouterSettings() {
    double stop_wait_time;
    double bus_velocity;

    stop_wait_time = pb_.router().settings().stop_wait_time();
    bus_velocity = pb_.router().settings().bus_velocity();

    router_.SetSettings(stop_wait_time, bus_velocity);
}

void Serializator::DeSerialize() {
	ifstream ifs(file_, ios::binary);
    pb_.ParseFromIstream(&ifs);

    ReadStops();
    all_stops_ = db_.GetStopNamesSortedNoCheck();
    ReadBuses();
    ReadDistances();
    ReadRenderSettings();
    ReadRouterSettings();
}



mr::Color Serializator::PbColorFromSvg(const svg::Color& color) {
    mr::Color res;
    
    res.set_is_rgb(holds_alternative<svg::Rgb>(color));
    res.set_is_rgba(holds_alternative<svg::Rgba>(color));
    res.set_is_string(holds_alternative<string>(color));

    if (res.is_rgb()) {
        res.set_r(std::get<svg::Rgb>(color).red);
        res.set_g(std::get<svg::Rgb>(color).green);
        res.set_b(std::get<svg::Rgb>(color).blue);
        res.set_a(0);
        res.set_name(""s);
    }
    else if (res.is_rgba()) {
        res.set_r(std::get<svg::Rgba>(color).red);
        res.set_g(std::get<svg::Rgba>(color).green);
        res.set_b(std::get<svg::Rgba>(color).blue);
        res.set_a(std::get<svg::Rgba>(color).opacity);
        res.set_name(""s);
    }
    else if (res.is_string()) {
        res.set_r(0);
        res.set_g(0);
        res.set_b(0);
        res.set_a(0);
        res.set_name(std::get<string>(color));
    }
    else {
        res.set_r(0);
        res.set_g(0);
        res.set_b(0);
        res.set_a(0);
        res.set_name(""s);
    }
    
    return res;
}



svg::Color Serializator::SvgColorFromPb(const mr::Color& color) {
    svg::Color res;

    if (color.is_rgb()) {
        svg::Rgb rgb(color.r(), color.g(), color.b());
        res = rgb;
        return res;
    }
    
    if (color.is_rgba()) {
        svg::Rgba rgba(color.r(), color.g(), color.b(), color.a());
        res = rgba;
        return res;
    }
    
    if (color.is_string()) {
        res = color.name();
        return res;
    }
    
    return move(res);
}