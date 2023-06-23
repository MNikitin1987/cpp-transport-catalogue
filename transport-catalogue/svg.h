#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>

using namespace std;


namespace svg {

    class Rgb {
    public:
        Rgb() = default;
        Rgb(uint8_t r, uint8_t g, uint8_t b)
            : red(r)
            , green(g)
            , blue(b) {
        }
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    class Rgba {
    public:
        Rgba() = default;
        Rgba(uint8_t r, uint8_t g, uint8_t b)
            : red(r)
            , green(g)
            , blue(b) {
        }
        Rgba(uint8_t r, uint8_t g, uint8_t b, double a)
            : red(r)
            , green(g)
            , blue(b)
            , opacity(a) {
        }
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
        double opacity = 1.0;
    };

    using Color = variant<monostate, Rgb, Rgba, string>;
    inline const Color NoneColor{ "none" };

    enum class StrokeLineCap {
        BUTT,
        ROUND,
        SQUARE,
    };

    enum class StrokeLineJoin {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,
    };

    struct Point {
        Point() = default;
        Point(double x, double y)
            : x(x)
            , y(y) {
        }
        double x = 0;
        double y = 0;
    };

    ostream& operator<<(ostream& out, StrokeLineJoin join);
    ostream& operator<<(ostream& out, StrokeLineCap cap);
    ostream& operator<<(ostream& out, Color color);

    /*
     * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
     * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
     */
    struct RenderContext {
        RenderContext(ostream& out)
            : out(out) {
        }

        RenderContext(ostream& out, int indent_step, int indent = 0)
            : out(out)
            , indent_step(indent_step)
            , indent(indent) {
        }

        RenderContext Indented() const {
            return { out, indent_step, indent + indent_step };
        }

        void RenderIndent() const {
            for (int i = 0; i < indent; ++i) {
                out.put(' ');
            }
        }

        ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    class ObjectContainer;

    class Drawable {
    public:
        virtual ~Drawable() = default;

        virtual void Draw(ObjectContainer& container) const = 0;
    };


    template <typename Owner>
    class PathProps {
    public:

        Owner& SetFillColor(Color color) {
            fill_color_ = move(color);
            return AsOwner();
        }
        Owner& SetStrokeColor(Color color) {
            stroke_color_ = move(color);
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width) {
            stroke_width_ = width;
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
            stroke_line_cap_ = line_cap;
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
            stroke_line_join_ = line_join;
            return AsOwner();
        }

    protected:
        ~PathProps() = default;

        void RenderAttrs(ostream& out) const {

            if (!holds_alternative<monostate>(fill_color_)) {
                out << " fill=\""sv;
                out << fill_color_;
                out << "\""sv;
            }

            if (!holds_alternative<monostate>(stroke_color_)) {
                out << " stroke=\""sv;
                out << stroke_color_;
                out << "\""sv;
            }

            if (stroke_width_) {
                out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
            }

            if (stroke_line_cap_) {
                out << " stroke-linecap=\""sv << *stroke_line_cap_ << "\""sv;
            }

            if (stroke_line_join_) {
                out << " stroke-linejoin=\""sv << *stroke_line_join_ << "\""sv;
            }

        }

    private:
        Owner& AsOwner() {
            return static_cast<Owner&>(*this);
        }

        Color fill_color_;
        Color stroke_color_;
        optional<double> stroke_width_;
        optional<StrokeLineCap> stroke_line_cap_;
        optional<StrokeLineJoin> stroke_line_join_;
    };

    /*
     * Абстрактный базовый класс Object служит для унифицированного хранения
     * конкретных тегов SVG-документа
     * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
     */
    class Object {
    public:
        void Render(const RenderContext& context) const;
        virtual ~Object() = default;
    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    class ObjectContainer {
    public:
        virtual ~ObjectContainer() = default;

        template <typename T>
        void Add(T object) {
            AddPtr(make_unique<T>(move(object)));
        }

        virtual void AddPtr(unique_ptr<Object>&& obj) = 0;
    };



    class Circle : public Object, public PathProps<Circle> {
    public:
        Circle& SetCenter(Point center);
        Circle& SetRadius(double radius);

    private:
        void RenderObject(const RenderContext& context) const override;

    private:
        Point center_;
        double radius_ = 1.0;
    };


    class Polyline : public Object, public PathProps<Polyline> {
    public:
        Polyline& AddPoint(Point point);

    private:
        void RenderObject(const RenderContext& context) const override;
        vector<Point> points_;
    };


    class Text : public Object, public PathProps<Text> {
    public:
        Text& SetPosition(Point pos);
        Text& SetOffset(Point offset);
        Text& SetFontSize(size_t size);
        Text& SetFontFamily(string font_family);
        Text& SetFontWeight(string font_weight);
        Text& SetData(string data);

    private:
        void RenderObject(const RenderContext& context) const override;
    private:
        Point pos_;
        Point offset_;
        uint32_t size_ = 1;
        string font_family_;
        string font_weight_;
        string data_;
    };


    class Document : public ObjectContainer {
    public:
        void AddPtr(unique_ptr<Object>&& obj);
        void Render(ostream& out) const;

    private:
        vector<unique_ptr<Object>> objects_;
    };

}  // namespace svg
