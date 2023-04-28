#include "map_renderer.h"

/*
 * В этом файле вы можете разместить код, отвечающий за визуализацию карты маршрутов в формате SVG.
 * Визуализация маршрутов вам понадобится во второй части итогового проекта.
 * Пока можете оставить файл пустым.
 */

namespace renderer {

	bool IsZero(double value) {
		return abs(value) < EPSILON;
	}

	MapRenderer::MapRenderer(TransportCatalogue& cat)
		: db_(cat) {
	}

	void MapRenderer::SetSettings(RenderSettings& settings) {
		settings_ = move(settings);

		const auto all_coords = db_.GetCoordinates();
		SphereProjector projector(all_coords.begin(), all_coords.end(), settings_.width, settings_.height, settings_.padding);
		projector_ = move(projector);

		bus_names_ = db_.GetBusNamesSorted();
		stop_names_ = db_.GetStopNamesSorted();
	}

	svg::Document MapRenderer::RenderMap() const {
		svg::Document res;

		RenderBusLines(res);
		RenderBusNames(res);
		RenderStopCircles(res);
		RenderStopNames(res);

		return res;
	}

	void MapRenderer::RenderBusLines(svg::Document& doc) const {
		size_t color_num = 0;

		for (const auto& bus_name : bus_names_) {
			if (db_.GetStopCountTotal(bus_name) == 0 || db_.GetStopCountTotal(bus_name) == 1) {
				continue;
			}

			svg::Polyline line;

			const auto path = db_.GetPath(bus_name, false);
			for (const auto& point : path) {
				line.AddPoint(projector_(point));
			}

			line.SetStrokeWidth(settings_.line_width);
			line.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
			line.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
			line.SetFillColor("none"s);

			line.SetStrokeColor(settings_.color_palette[color_num]);
			if (++color_num == settings_.color_palette.size()) {
				color_num = 0;
			}
			doc.Add(line);
		}
	}

	svg::Text MapRenderer::GetText(
		const svg::Point& pos,
		string_view text,
		bool for_bus) const {

		svg::Text res;

		res.SetPosition(pos);

		if (for_bus) {
			res.SetOffset({ settings_.bus_label_offset.dx, settings_.bus_label_offset.dy });
			res.SetFontSize(settings_.bus_label_font_size);
		}
		else {
			res.SetOffset({ settings_.stop_label_offset.dx, settings_.stop_label_offset.dy });
			res.SetFontSize(settings_.stop_label_font_size);
		}

		res.SetFontFamily("Verdana"s);
		if (for_bus) {
			res.SetFontWeight("bold"s);
		}
		res.SetData(string{ text });
		return res;
	}

	void MapRenderer::RenderBusNames(svg::Document& doc) const {
		size_t color_num = 0;
		for (const auto& bus_name : bus_names_) {
			if (db_.GetStopCountTotal(bus_name) == 0 || db_.GetStopCountTotal(bus_name) == 1) {
				continue;
			}

			const auto path = db_.GetPath(bus_name, true);

			auto txt_beg_subs = GetText(projector_(path.front()), bus_name, true);
			auto txt_beg_main = GetText(projector_(path.front()), bus_name, true);

			auto txt_end_subs = GetText(projector_(path.back()), bus_name, true);
			auto txt_end_main = GetText(projector_(path.back()), bus_name, true);

			txt_beg_subs
				.SetFillColor(settings_.underlayer_color)
				.SetStrokeColor(settings_.underlayer_color)
				.SetStrokeWidth(settings_.underlayer_width)
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

			txt_beg_main.SetFillColor(settings_.color_palette[color_num]);

			doc.Add(txt_beg_subs);
			doc.Add(txt_beg_main);

			if (!db_.GetBusRound(bus_name) && path.front() != path.back()) {
				txt_end_subs
					.SetFillColor(settings_.underlayer_color)
					.SetStrokeColor(settings_.underlayer_color)
					.SetStrokeWidth(settings_.underlayer_width)
					.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
					.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

				txt_end_main.SetFillColor(settings_.color_palette[color_num]);
				doc.Add(txt_end_subs);
				doc.Add(txt_end_main);
			}

			if (++color_num == settings_.color_palette.size()) {
				color_num = 0;
			}
		}
	}

	void MapRenderer::RenderStopCircles(svg::Document& doc) const {

		for (const auto& stop : stop_names_) {
			svg::Circle circle;

			circle.SetFillColor("white"s);
			circle.SetRadius(settings_.stop_radius);
			circle.SetCenter(projector_(db_.GetStopCoords(stop)));

			doc.Add(circle);
		}
	}

	void MapRenderer::RenderStopNames(svg::Document& doc) const {
		for (const auto& stop_name : stop_names_) {

			auto txt_subs = GetText(projector_(db_.GetStopCoords(stop_name)), stop_name, false);
			auto txt_main = GetText(projector_(db_.GetStopCoords(stop_name)), stop_name, false);

			txt_subs
				.SetFillColor(settings_.underlayer_color)
				.SetStrokeColor(settings_.underlayer_color)
				.SetStrokeWidth(settings_.underlayer_width)
				.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
				.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

			txt_main.SetFillColor("black"s);

			doc.Add(txt_subs);
			doc.Add(txt_main);
		}
	}
}