#pragma once

#include "geo.h"
#include "svg.h"
#include "domain.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>

namespace renderer {
    using namespace std;
    using namespace svg;
    using namespace geo;
    using namespace domain;

    inline const double EPSILON = 1e-6;

    bool IsZero(double value);

    class SphereProjector {
    public:
        // points_begin и points_end задают начало и конец интервала элементов geo::Coordinates
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
            double max_width, double max_height, double padding)
            : padding_(padding) {
            // Если точки поверхности сферы не заданы, вычислять нечего
            if (points_begin == points_end) {
                return;
            }

            // Находим точки с минимальной и максимальной долготой
            const auto [left_it, right_it] = minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
            min_lon_ = left_it->lng;
            const double max_lon = right_it->lng;

            // Находим точки с минимальной и максимальной широтой
            const auto [bottom_it, top_it] = minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
            const double min_lat = bottom_it->lat;
            max_lat_ = top_it->lat;

            // Вычисляем коэффициент масштабирования вдоль координаты x
            optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            // Вычисляем коэффициент масштабирования вдоль координаты y
            optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                // Коэффициенты масштабирования по ширине и высоте ненулевые,
                // берём минимальный из них
                zoom_coeff_ = min(*width_zoom, *height_zoom);
            }
            else if (width_zoom) {
                // Коэффициент масштабирования по ширине ненулевой, используем его
                zoom_coeff_ = *width_zoom;
            }
            else if (height_zoom) {
                // Коэффициент масштабирования по высоте ненулевой, используем его
                zoom_coeff_ = *height_zoom;
            }
        }

        SphereProjector() = default;

        // Проецирует широту и долготу в координаты внутри SVG-изображения
        Point operator()(Coordinates coords) const {
            return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    private:
        double padding_ = 0;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    struct LabelOffset {
        double dx;
        double dy;
    };

    struct RenderSettings {
        double width;
        double height;
        double padding;

        double stop_radius;

        double line_width;

        size_t bus_label_font_size;
        LabelOffset bus_label_offset;

        size_t stop_label_font_size;
        LabelOffset stop_label_offset;

        Color underlayer_color;
        double underlayer_width;
        vector<Color> color_palette;
    };


    class MapRenderer {
    public:
        MapRenderer(TransportCatalogue& cat);

        void SetSettings(RenderSettings& settings);
        svg::Document RenderMap() const;


    private:
        void RenderBusLines(svg::Document& doc) const;
        void RenderBusNames(svg::Document& doc) const;
        void RenderStopCircles(svg::Document& doc) const;
        void RenderStopNames(svg::Document& doc) const;
        svg::Text GetText(const svg::Point& pos, string_view text, bool for_bus) const;

    private:
        const TransportCatalogue& db_;
        RenderSettings settings_;
        SphereProjector projector_;
        vector<string_view> bus_names_;
        vector<string_view> stop_names_;
    };

}