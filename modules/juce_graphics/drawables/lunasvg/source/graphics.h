#ifndef LUNASVG_GRAPHICS_H
#define LUNASVG_GRAPHICS_H

#include "../plutovg/include/plutovg.h"

#include <cstdint>
#include <algorithm>
#include <utility>
#include <memory>
#include <vector>
#include <array>
#include <string>
namespace {

namespace lunasvg {

enum class LineCap : uint8_t {
    Butt = JUCE_PLUTOVG_LINE_CAP_BUTT,
    Round = JUCE_PLUTOVG_LINE_CAP_ROUND,
    Square = JUCE_PLUTOVG_LINE_CAP_SQUARE
};

enum class LineJoin : uint8_t {
    Miter = JUCE_PLUTOVG_LINE_JOIN_MITER,
    Round = JUCE_PLUTOVG_LINE_JOIN_ROUND,
    Bevel = JUCE_PLUTOVG_LINE_JOIN_BEVEL
};

enum class FillRule : uint8_t {
    NonZero = JUCE_PLUTOVG_FILL_RULE_NON_ZERO,
    EvenOdd = JUCE_PLUTOVG_FILL_RULE_EVEN_ODD
};

enum class SpreadMethod : uint8_t {
    Pad = JUCE_PLUTOVG_SPREAD_METHOD_PAD,
    Reflect = JUCE_PLUTOVG_SPREAD_METHOD_REFLECT,
    Repeat = JUCE_PLUTOVG_SPREAD_METHOD_REPEAT
};

class Color {
public:
    constexpr Color() = default;
    constexpr explicit Color(uint32_t value) : m_value(value) {}
    constexpr Color(int r, int g, int b, int a = 255) : m_value(a << 24 | r << 16 | g << 8 | b) {}

    constexpr uint8_t alpha() const { return (m_value >> 24) & 0xff; }
    constexpr uint8_t red() const { return (m_value >> 16) & 0xff; }
    constexpr uint8_t green() const { return (m_value >> 8) & 0xff; }
    constexpr uint8_t blue() const { return (m_value >> 0) & 0xff; }

    constexpr float alphaF() const { return alpha() / 255.f; }
    constexpr float redF() const { return red() / 255.f; }
    constexpr float greenF() const { return green() / 255.f; }
    constexpr float blueF() const { return blue() / 255.f; }

    constexpr uint32_t value() const { return m_value; }

    constexpr bool isOpaque() const { return alpha() == 255; }
    constexpr bool isVisible() const { return alpha() > 0; }

    constexpr Color opaqueColor() const { return Color(m_value | 0xFF000000); }
    constexpr Color colorWithAlpha(float opacity) const;

    static const Color Transparent;
    static const Color Black;
    static const Color White;

private:
    uint32_t m_value = 0;
};

constexpr Color Color::colorWithAlpha(float opacity) const
{
    auto rgb = m_value & 0x00FFFFFF;
    auto a = static_cast<int>(alpha() * std::clamp(opacity, 0.f, 1.f));
    return Color(rgb | a << 24);
}

class Point {
public:
    constexpr Point() = default;
    constexpr Point(const juce_plutovg_point_t& point) : Point(point.x, point.y) {}
    constexpr Point(float x, float y) : x(x), y(y) {}

    constexpr void move(float dx, float dy) { x += dx; y += dy; }
    constexpr void move(float d) { move(d, d); }
    constexpr void move(const Point& p) { move(p.x, p.y); }

    constexpr void scale(float sx, float sy) { x *= sx; y *= sy; }
    constexpr void scale(float s) { scale(s, s); }

    constexpr float dot(const Point& p) const { return x * p.x + y * p.y; }

public:
    float x{0};
    float y{0};
};

constexpr Point operator+(const Point& a, const Point& b)
{
    return Point(a.x + b.x, a.y + b.y);
}

constexpr Point operator-(const Point& a, const Point& b)
{
    return Point(a.x - b.x, a.y - b.y);
}

constexpr Point operator-(const Point& a)
{
    return Point(-a.x, -a.y);
}

constexpr Point& operator+=(Point& a, const Point& b)
{
    a.move(b);
    return a;
}

constexpr Point& operator-=(Point& a, const Point& b)
{
    a.move(-b);
    return a;
}

constexpr float operator*(const Point& a, const Point& b)
{
    return a.dot(b);
}

class Size {
public:
    constexpr Size() = default;
    constexpr Size(float w, float h) : w(w), h(h) {}

    constexpr void expand(float dw, float dh) { w += dw; h += dh; }
    constexpr void expand(float d) { expand(d, d); }
    constexpr void expand(const Size& s) { expand(s.w, s.h); }

    constexpr void scale(float sw, float sh) { w *= sw; h *= sh; }
    constexpr void scale(float s) { scale(s, s); }

    constexpr bool isEmpty() const { return w <= 0.f || h <= 0.f; }
    constexpr bool isZero() const { return w <= 0.f && h <= 0.f; }
    constexpr bool isValid() const { return w >= 0.f && h >= 0.f; }

public:
    float w{0};
    float h{0};
};

constexpr Size operator+(const Size& a, const Size& b)
{
    return Size(a.w + b.w, a.h + b.h);
}

constexpr Size operator-(const Size& a, const Size& b)
{
    return Size(a.w - b.w, a.h - b.h);
}

constexpr Size operator-(const Size& a)
{
    return Size(-a.w, -a.h);
}

constexpr Size& operator+=(Size& a, const Size& b)
{
    a.expand(b);
    return a;
}

constexpr Size& operator-=(Size& a, const Size& b)
{
    a.expand(-b);
    return a;
}

class Box;

class Rect {
public:
    constexpr Rect() = default;
    constexpr explicit Rect(const Size& size) : Rect(size.w, size.h) {}
    constexpr Rect(float width, float height) : Rect(0, 0, width, height) {}
    constexpr Rect(const Point& origin, const Size& size) : Rect(origin.x, origin.y, size.w, size.h) {}
    constexpr Rect(const juce_plutovg_rect_t& rect) : Rect(rect.x, rect.y, rect.w, rect.h) {}
    constexpr Rect(float x, float y, float w, float h) : x(x), y(y), w(w), h(h) {}

    Rect(const Box& box);

    constexpr void move(float dx, float dy) { x += dx; y += dy; }
    constexpr void move(float d) { move(d, d); }
    constexpr void move(const Point& p) { move(p.x, p.y); }

    constexpr void scale(float sx, float sy) { x *= sx; y *= sy; w *= sx; h *= sy; }
    constexpr void scale(float s) { scale(s, s); }

    constexpr void inflate(float dx, float dy) { x -= dx; y -= dy; w += dx * 2.f; h += dy * 2.f; }
    constexpr void inflate(float d) { inflate(d, d); }

    constexpr bool contains(float px, float py) const { return px >= x && px <= x + w && py >= y && py <= y + h; }
    constexpr bool contains(const Point& p) const { return contains(p.x, p.y); }

    constexpr Rect intersected(const Rect& rect) const;
    constexpr Rect united(const Rect& rect) const;

    constexpr Rect& intersect(const Rect& o);
    constexpr Rect& unite(const Rect& o);

    constexpr Point origin() const { return Point(x, y); }
    constexpr Size size() const { return Size(w, h); }

    constexpr float right() const { return x + w; }
    constexpr float bottom() const { return y + h; }

    constexpr bool isEmpty() const { return w <= 0.f || h <= 0.f; }
    constexpr bool isZero() const { return w <= 0.f && h <= 0.f; }
    constexpr bool isValid() const { return w >= 0.f && h >= 0.f; }

    static const Rect Empty;
    static const Rect Invalid;
    static const Rect Infinite;

public:
    float x{0};
    float y{0};
    float w{0};
    float h{0};
};

constexpr Rect Rect::intersected(const Rect& rect) const
{
    if(!rect.isValid())
        return *this;
    if(!isValid())
        return rect;
    auto l = std::max(x, rect.x);
    auto t = std::max(y, rect.y);
    auto r = std::min(x + w, rect.x + rect.w);
    auto b = std::min(y + h, rect.y + rect.h);
    if(l >= r || t >= b)
        return Rect::Empty;
    return Rect(l, t, r - l, b - t);
}

constexpr Rect Rect::united(const Rect& rect) const
{
    if(!rect.isValid())
        return *this;
    if(!isValid())
        return rect;
    auto l = std::min(x, rect.x);
    auto t = std::min(y, rect.y);
    auto r = std::max(x + w, rect.x + rect.w);
    auto b = std::max(y + h, rect.y + rect.h);
    return Rect(l, t, r - l, b - t);
}

constexpr Rect& Rect::intersect(const Rect& o)
{
    *this = intersected(o);
    return *this;
}

constexpr Rect& Rect::unite(const Rect& o)
{
    *this = united(o);
    return *this;
}

class Matrix;

class Transform {
public:
    Transform();
    Transform(const Matrix& matrix);
    Transform(float a, float b, float c, float d, float e, float f);
    Transform(const juce_plutovg_matrix_t& matrix) : m_matrix(matrix) {}

    Transform operator*(const Transform& transform) const;
    Transform& operator*=(const Transform& transform);

    Transform& multiply(const Transform& transform);
    Transform& translate(float tx, float ty);
    Transform& scale(float sx, float sy);
    Transform& rotate(float angle, float cx = 0.f, float cy = 0.f);
    Transform& shear(float shx, float shy);

    Transform& postMultiply(const Transform& transform);
    Transform& postTranslate(float tx, float ty);
    Transform& postScale(float sx, float sy);
    Transform& postRotate(float angle, float cx = 0.f, float cy = 0.f);
    Transform& postShear(float shx, float shy);

    Transform inverse() const;
    Transform& invert();

    void reset();

    Point mapPoint(float x, float y) const;
    Point mapPoint(const Point& point) const;
    Rect mapRect(const Rect& rect) const;

    float xScale() const;
    float yScale() const;

    const juce_plutovg_matrix_t& matrix() const { return m_matrix; }
    juce_plutovg_matrix_t& matrix() { return m_matrix; }

    bool parse(const char* data, size_t length);

    static Transform translated(float tx, float ty);
    static Transform scaled(float sx, float sy);
    static Transform rotated(float angle, float cx, float cy);
    static Transform sheared(float shx, float shy);

    static const Transform Identity;

private:
    juce_plutovg_matrix_t m_matrix;
};

enum class PathCommand {
    MoveTo = JUCE_PLUTOVG_PATH_COMMAND_MOVE_TO,
    LineTo = JUCE_PLUTOVG_PATH_COMMAND_LINE_TO,
    CubicTo = JUCE_PLUTOVG_PATH_COMMAND_CUBIC_TO,
    Close = JUCE_PLUTOVG_PATH_COMMAND_CLOSE
};

class Path {
public:
    Path() = default;
    Path(const Path& path);
    Path(Path&& path);
    ~Path();

    Path& operator=(const Path& path);
    Path& operator=(Path&& path);

    void swap(Path& path);

    void moveTo(float x, float y);
    void lineTo(float x, float y);
    void quadTo(float x1, float y1, float x2, float y2);
    void cubicTo(float x1, float y1, float x2, float y2, float x3, float y3);
    void arcTo(float rx, float ry, float xAxisRotation, bool largeArcFlag, bool sweepFlag, float x, float y);
    void close();

    void addEllipse(float cx, float cy, float rx, float ry);
    void addRoundRect(float x, float y, float w, float h, float rx, float ry);
    void addRect(float x, float y, float w, float h);

    void addEllipse(const Point& center, const Size& radii);
    void addRoundRect(const Rect& rect, const Size& radii);
    void addRect(const Rect& rect);

    void reset();

    Rect boundingRect() const;
    bool isEmpty() const;
    bool isUnique() const;
    bool isNull() const { return m_data == nullptr; }
    juce_plutovg_path_t* data() const { return m_data; }

    bool parse(const char* data, size_t length);

private:
    juce_plutovg_path_t* release();
    juce_plutovg_path_t* ensure();
    juce_plutovg_path_t* m_data = nullptr;
};

inline void Path::swap(Path& path)
{
    std::swap(m_data, path.m_data);
}

inline juce_plutovg_path_t* Path::release()
{
    return std::exchange(m_data, nullptr);
}

class PathIterator {
public:
    PathIterator(const Path& path);

    PathCommand currentSegment(std::array<Point, 3>& points) const;
    bool isDone() const { return m_index >= m_size; }
    void next();

private:
    const juce_plutovg_path_element_t* m_elements;
    const int m_size;
    int m_index;
};

class FontFace {
public:
    FontFace() = default;
    explicit FontFace(juce_plutovg_font_face_t* face);
    FontFace(const void* data, size_t length, juce_plutovg_destroy_func_t destroy_func, void* closure);
    FontFace(const char* filename);
    FontFace(const FontFace& face);
    FontFace(FontFace&& face);
    ~FontFace();

    FontFace& operator=(const FontFace& face);
    FontFace& operator=(FontFace&& face);

    void swap(FontFace& face);

    bool isNull() const { return m_face == nullptr; }
    juce_plutovg_font_face_t* get() const { return m_face; }

private:
    juce_plutovg_font_face_t* release();
    juce_plutovg_font_face_t* m_face = nullptr;
};

class FontFaceCache {
public:
    bool addFontFace(const std::string& family, bool bold, bool italic, const FontFace& face);
    FontFace getFontFace(const std::string& family, bool bold, bool italic) const;

private:
    FontFaceCache();
    juce_plutovg_font_face_cache_t* m_cache;
    friend FontFaceCache* fontFaceCache();
};

FontFaceCache* fontFaceCache();

using Font = juce::detail::LunaSvgFontReplacement;

enum class TextureType {
    Plain = JUCE_PLUTOVG_TEXTURE_TYPE_PLAIN,
    Tiled = JUCE_PLUTOVG_TEXTURE_TYPE_TILED
};

enum class BlendMode {
    Src = JUCE_PLUTOVG_OPERATOR_SRC,
    Src_Over = JUCE_PLUTOVG_OPERATOR_SRC_OVER,
    Dst_In = JUCE_PLUTOVG_OPERATOR_DST_IN,
    Dst_Out = JUCE_PLUTOVG_OPERATOR_DST_OUT
};

using DashArray = std::vector<float>;

class StrokeData {
public:
    explicit StrokeData(float lineWidth = 1.f) : m_lineWidth(lineWidth) {}

    void setLineWidth(float lineWidth) { m_lineWidth = lineWidth; }
    float lineWidth() const { return m_lineWidth; }

    void setMiterLimit(float miterLimit) { m_miterLimit = miterLimit; }
    float miterLimit() const { return m_miterLimit; }

    void setDashOffset(float dashOffset) { m_dashOffset = dashOffset; }
    float dashOffset() const { return m_dashOffset; }

    void setDashArray(DashArray dashArray) { m_dashArray = std::move(dashArray); }
    const DashArray& dashArray() const { return m_dashArray; }

    void setLineCap(LineCap lineCap) { m_lineCap = lineCap; }
    LineCap lineCap() const { return m_lineCap; }

    void setLineJoin(LineJoin lineJoin) { m_lineJoin = lineJoin; }
    LineJoin lineJoin() const { return m_lineJoin; }

private:
    float m_lineWidth;
    float m_miterLimit{4.f};
    float m_dashOffset{0.f};
    LineCap m_lineCap{LineCap::Butt};
    LineJoin m_lineJoin{LineJoin::Miter};
    DashArray m_dashArray;
};

using GradientStop = juce_plutovg_gradient_stop_t;
using GradientStops = std::vector<GradientStop>;

class Bitmap;

class Canvas {
public:
    std::shared_ptr<Canvas> create(const Bitmap& bitmap);
    std::shared_ptr<Canvas> create(float x, float y, float width, float height);
    std::shared_ptr<Canvas> create(const Rect& extents);

    virtual void setColor(const Color& color) = 0;
    virtual void setColor(float r, float g, float b, float a) = 0;
    virtual void setLinearGradient(float x1, float y1, float x2, float y2, SpreadMethod spread, const GradientStops& stops, const Transform& transform) = 0;
    virtual void setRadialGradient(float cx, float cy, float r, float fx, float fy, SpreadMethod spread, const GradientStops& stops, const Transform& transform) = 0;
    virtual void setTexture(const Canvas& source, TextureType type, float opacity, const Transform& transform) = 0;

    virtual void fillPath(const Path& path, FillRule fillRule, const Transform& transform) = 0;
    virtual void strokePath(const Path& path, const StrokeData& strokeData, const Transform& transform) = 0;

    virtual void fillText(const std::u32string_view& text, const Font& font, const Point& origin, const Transform& transform) = 0;
    virtual void strokeText(const std::u32string_view& text, float strokeWidth, const Font& font, const Point& origin, const Transform& transform) = 0;

    virtual void clipPath(const Path& path, FillRule clipRule, const Transform& transform) = 0;
    virtual void clipRect(const Rect& rect, FillRule clipRule, const Transform& transform) = 0;

    virtual void drawImage(const Bitmap& image, const Rect& dstRect, const Rect& srcRect, const Transform& transform) = 0;
    virtual void blendCanvas(const Canvas& canvas, BlendMode blendMode, float opacity) = 0;

    virtual void save() = 0;
    virtual void restore() = 0;

    virtual void convertToLuminanceMask() = 0;

    virtual int x() const = 0;
    virtual int y() const = 0;
    virtual int width() const = 0;
    virtual int height() const = 0;

    virtual Rect extents() const = 0;

    virtual juce_plutovg_surface_t* surface() const = 0;
    virtual juce_plutovg_canvas_t* canvas() const = 0;

    virtual ~Canvas() = default;

protected:
    virtual std::shared_ptr<Canvas> createFromBounds(int x, int y, int width, int height) = 0;
    virtual std::shared_ptr<Canvas> createFromBitmap(const Bitmap& bitmap) = 0;
};

class CanvasImpl : public Canvas {
public:
    static std::shared_ptr<Canvas> createCanvasImpl(const Bitmap& bitmap);

    void setColor(const Color& color) override;
    void setColor(float r, float g, float b, float a) override;
    void setLinearGradient(float x1, float y1, float x2, float y2, SpreadMethod spread, const GradientStops& stops, const Transform& transform) override;
    void setRadialGradient(float cx, float cy, float r, float fx, float fy, SpreadMethod spread, const GradientStops& stops, const Transform& transform) override;
    void setTexture(const Canvas& source, TextureType type, float opacity, const Transform& transform) override;

    void fillPath(const Path& path, FillRule fillRule, const Transform& transform) override;
    void strokePath(const Path& path, const StrokeData& strokeData, const Transform& transform) override;

    void fillText(const std::u32string_view& text, const Font& font, const Point& origin, const Transform& transform) override;
    void strokeText(const std::u32string_view& text, float strokeWidth, const Font& font, const Point& origin, const Transform& transform) override;

    void clipPath(const Path& path, FillRule clipRule, const Transform& transform) override;
    void clipRect(const Rect& rect, FillRule clipRule, const Transform& transform) override;

    void drawImage(const Bitmap& image, const Rect& dstRect, const Rect& srcRect, const Transform& transform) override;
    void blendCanvas(const Canvas& canvas, BlendMode blendMode, float opacity) override;

    void save() override;
    void restore() override;

    void convertToLuminanceMask() override;

    int x() const override { return m_x; }
    int y() const override { return m_y; }
    int width() const override;
    int height() const override;

    Rect extents() const override { return Rect(m_x, m_y, width(), height()); }

    juce_plutovg_surface_t* surface() const override { return m_surface; }
    juce_plutovg_canvas_t* canvas() const override { return m_canvas; }

    ~CanvasImpl() override;

protected:
    std::shared_ptr<Canvas> createFromBounds(int x, int y, int width, int height) override;
    std::shared_ptr<Canvas> createFromBitmap(const Bitmap& bitmap) override;

private:
    CanvasImpl(const Bitmap& bitmap);
    CanvasImpl(int x, int y, int width, int height);
    juce_plutovg_surface_t* m_surface;
    juce_plutovg_canvas_t* m_canvas;
    juce_plutovg_matrix_t m_translation;
    const int m_x;
    const int m_y;
};

} // namespace lunasvg
}

#endif // LUNASVG_GRAPHICS_H
