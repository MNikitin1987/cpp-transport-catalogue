#pragma once

#include <filesystem>
#include <string>
#include <fstream>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <transport_catalogue.pb.h>
#include <map_renderer.pb.h>

using namespace std;



class Serializator {

public:
    Serializator(
        tc::TransportCatalogue& transport_catalogue,
        renderer::MapRenderer& renderer,
        TransportRouter& router);

    void SetFile(const string_view& file);
    void Serialize();
    void DeSerialize();


private:
    void WriteStops();
    void WriteBuses();
    void WriteDistances();
    void WriteRenderSettings();
    void WriteRouterSettings();

    void ReadStops();
    void ReadBuses();
    void ReadDistances();
    void ReadRenderSettings();
    void ReadRouterSettings();

    mr::Color PbColorFromSvg(const svg::Color& color);
    svg::Color SvgColorFromPb(const mr::Color& color);

    
private:
    tc::TransportCatalogue& db_;
    renderer::MapRenderer& renderer_;
    TransportRouter& router_;

    filesystem::path file_;
    pb::TransportCatalogue pb_;

    vector<string_view> all_stops_;
};