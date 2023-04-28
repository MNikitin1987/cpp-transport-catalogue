#include "svg.h"

using namespace std;

namespace svg {

    void Object::Render(const RenderContext& context) const {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);

        context.out << endl;
    }

    // ---------- Circle ------------------

    Circle& Circle::SetCenter(Point center) {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius) {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Polyline ------------------

    Polyline& Polyline::AddPoint(Point point) {
        points_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        bool first = true;
        for (const auto& point : points_) {
            if (first) {
                first = false;
            }
            else {
                out << " "sv;
            }
            out << point.x << ","sv << point.y;
        }
        out << "\""sv;
        RenderAttrs(out);
        out << "/>"sv;
    }

    // ---------- Text ------------------

    Text& Text::SetPosition(Point pos) {
        pos_ = pos;
        return *this;
    }

    // Задаёт смещение относительно опорной точки (атрибуты dx, dy)
    Text& Text::SetOffset(Point offset) {
        offset_ = offset;
        return *this;
    }

    // Задаёт размеры шрифта (атрибут font-size)
    Text& Text::SetFontSize(uint32_t size) {
        size_ = size;
        return *this;
    }

    // Задаёт название шрифта (атрибут font-family)
    Text& Text::SetFontFamily(string font_family) {
        font_family_ = move(font_family);
        return *this;
    }

    // Задаёт толщину шрифта (атрибут font-weight)
    Text& Text::SetFontWeight(string font_weight) {
        font_weight_ = move(font_weight);
        return *this;
    }

    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& Text::SetData(string data) {
        data_ = move(data);
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const {
        auto& out = context.out;
        out << "<text"sv;
        RenderAttrs(out);
        out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\""sv;
        out << " dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\""sv;
        out << " font-size=\""sv << size_ << "\"";

        if (!font_family_.empty()) {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }

        if (!font_weight_.empty()) {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }

        out << ">"sv;

        for (const auto c : data_) {
            switch (c) {
            case '"':
                out << "&quot;"sv;
                break;
            case '\'':
                out << "&apos;"sv;
                break;
            case '<':
                out << "&lt;"sv;
                break;
            case '>':
                out << "&gt;"sv;
                break;
            case '&':
                out << "&amp;"sv;
                break;
            default:
                out << c;
            }
        }
        out << "</text>"sv;
    }

    void Document::Render(ostream& out) const {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;

        for (const auto& obj : objects_) {
            RenderContext ctx(out, 2, 2);
            obj.get()->Render(ctx);
        }
        out << "</svg>"sv;
    }

    void Document::AddPtr(unique_ptr<Object>&& obj) {
        objects_.emplace_back(move(obj));
    }

    ostream& operator<<(ostream& out, StrokeLineJoin join) {

        switch (join) {
        case StrokeLineJoin::ARCS:
            out << "arcs"sv;
            break;
        case StrokeLineJoin::BEVEL:
            out << "bevel"sv;
            break;
        case StrokeLineJoin::MITER:
            out << "miter"sv;
            break;
        case StrokeLineJoin::MITER_CLIP:
            out << "miter-clip"sv;
            break;
        case StrokeLineJoin::ROUND:
            out << "round"sv;
            break;
        }
        return out;
    }

    ostream& operator<<(ostream& out, StrokeLineCap cap) {
        switch (cap) {
        case StrokeLineCap::BUTT:
            out << "butt"sv;
            break;
        case StrokeLineCap::ROUND:
            out << "round"sv;
            break;
        case StrokeLineCap::SQUARE:
            out << "square"sv;
            break;
        }
        return out;
    }

    struct ColorPrinter {
        ostream& out;
        void operator()(monostate) const {
            out << "none"sv;
        }
        void operator()(Rgb color) {
            out << "rgb("sv;
            out << static_cast<int>(color.red) << ","sv;
            out << static_cast<int>(color.green) << ","sv;
            out << static_cast<int>(color.blue);
            out << ")";
        }
        void operator()(Rgba color) {
            out << "rgba("sv;
            out << static_cast<int>(color.red) << ","sv;
            out << static_cast<int>(color.green) << ","sv;
            out << static_cast<int>(color.blue) << ","sv;
            out << color.opacity;
            out << ")";
        }
        void operator()(string color) {
            out << color;
        }
    };
    ostream& operator<<(ostream& out, Color color) {
        visit(ColorPrinter{ out }, color);
        return out;
    }
}  // namespace svg