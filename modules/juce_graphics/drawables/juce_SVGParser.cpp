/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#include <juce_graphics/drawables/lunasvg/plutovg/source/plutovg-private.h>

#include <stack>

namespace juce
{

std::unique_ptr<Drawable> Drawable::createFromSVGFile (const File& svgFile)
{
    return createFromSVGString (svgFile.loadFileAsString());
}

Path Drawable::parseSVGPath (const String& svgPath)
{
    const auto svgString = "<svg xmlns=\"http://www.w3.org/2000/svg\"><path d=" + svgPath.quoted() + " /></svg>";
    const auto composite = createFromSVGString (svgString);

    if (composite->getNumChildren() != 1)
    {
        jassertfalse;
        return {};
    }

    auto* p = dynamic_cast<const DrawablePath*> (&composite->getChild (0));

    if (p == nullptr)
        return {};

    return p->getPath();
}

struct LunaSvgConverters
{
    LunaSvgConverters() = delete;

    static Path toJucePath (const lunasvg::Path& p,
                            const lunasvg::FillRule& fillRule = lunasvg::FillRule::NonZero)
    {
        juce::Path output;

        for (auto it = lunasvg::PathIterator { p }; ! it.isDone(); it.next())
        {
            std::array<lunasvg::Point, 3> points;

            switch (it.currentSegment (points))
            {
                case lunasvg::PathCommand::MoveTo:
                    output.startNewSubPath (points[0].x, points[0].y);
                    break;

                case lunasvg::PathCommand::LineTo:
                    output.lineTo (points[0].x, points[0].y);
                    break;

                case lunasvg::PathCommand::CubicTo:
                    output.cubicTo (
                        points[0].x, points[0].y, points[1].x, points[1].y, points[2].x, points[2].y);
                    break;

                case lunasvg::PathCommand::Close:
                    output.closeSubPath();
                    break;
            }
        }

        output.setUsingNonZeroWinding (fillRule == lunasvg::FillRule::NonZero);
        return output;
    }

    static Rectangle<float> toJuceRectangle (const lunasvg::Rect& r)
    {
        return { r.x, r.y, r.w, r.h };
    }

    static PathStrokeType::JointStyle toJuceJointStyle (const lunasvg::LineJoin& lineCap)
    {
        switch (lineCap)
        {
            case lunasvg::LineJoin::Miter:
                return PathStrokeType::mitered;

            case lunasvg::LineJoin::Round:
                return PathStrokeType::curved;

            case lunasvg::LineJoin::Bevel:
                return PathStrokeType::beveled;
        }

        jassertfalse;
        return PathStrokeType::beveled;
    }

    static PathStrokeType::EndCapStyle toJuceEndCapStyle (const lunasvg::LineCap& lineCap)
    {
        switch (lineCap)
        {
            case lunasvg::LineCap::Butt:
                return PathStrokeType::butt;

            case lunasvg::LineCap::Round:
                return PathStrokeType::rounded;

            case lunasvg::LineCap::Square:
                return PathStrokeType::square;
        }

        jassertfalse;
        return PathStrokeType::square;
    }

    static AffineTransform toJuceTransform (const lunasvg::Transform& transform)
    {
        const auto& m = transform.matrix();
        return AffineTransform { m.a, m.c, m.e, m.b, m.d, m.f };
    }

    static Colour toJuceColour (const lunasvg::Color& colour)
    {
        return Colour::fromRGBA (colour.red(), colour.green(), colour.blue(), colour.alpha());
    }

    static Colour toJuceColour (const juce_plutovg_color_t& colour)
    {
        return Colour::fromFloatRGBA (colour.r, colour.g, colour.b, colour.a);
    }

    static ColourGradient::SpreadMethod toJuceSpreadMethod (const lunasvg::SpreadMethod& spread)
    {
        switch (spread)
        {
            case lunasvg::SpreadMethod::Pad:
                return ColourGradient::SpreadMethod::pad;

            case lunasvg::SpreadMethod::Reflect:
                return ColourGradient::SpreadMethod::reflect;

            case lunasvg::SpreadMethod::Repeat:
                return ColourGradient::SpreadMethod::repeat;
        }

        jassertfalse;
        return ColourGradient::SpreadMethod::pad;
    }

    static Image toJuceImage (juce_plutovg_surface* s)
    {
        if (s == nullptr || s->data == nullptr || s->width <= 0 || s->height <= 0)
            return {};

        Image image { Image::ARGB, s->width, s->height, false };
        Image::BitmapData destData { image, Image::BitmapData::writeOnly };

        for (int y = 0; y < s->height; ++y)
        {
            const auto* src = s->data + (size_t) (y * s->stride);
            auto* dest = destData.getLinePointer (y);
            memcpy (dest, src, (size_t) s->width * 4);
        }

        return image;
    }

    static BlendMode toJuceBlendMode (const lunasvg::BlendMode& blendMode)
    {
        switch (blendMode)
        {
            case lunasvg::BlendMode::Src:       return BlendMode::source;
            case lunasvg::BlendMode::Src_Over:  return BlendMode::sourceOver;
            case lunasvg::BlendMode::Dst_In:    return BlendMode::destinationIn;
            case lunasvg::BlendMode::Dst_Out:   return BlendMode::destinationOut;
        }

        jassertfalse;
        return BlendMode::source;
    }

    static Rectangle<int> toJuceIntRectangle (const lunasvg::Rect& r)
    {
        return Rectangle<float> { r.x, r.y, r.w, r.h }.toNearestInt();
    }

    static lunasvg::Rect fromJuceRectangle (const Rectangle<float>& r)
    {
        return lunasvg::Rect { r.getX(), r.getY(), r.getWidth(), r.getHeight() };
    }
};

namespace
{

class DrawableCanvas : public lunasvg::Canvas
{
private:
    struct State
    {
        FillType fillType;
        std::shared_ptr<Drawable> clipPath;
    };

protected:
    std::shared_ptr<Canvas> createFromBounds (int x, int y, int width, int height) override
    {
        auto canvas = std::make_shared<DrawableCanvas> (&document);
        canvas->setContentArea (Rectangle<int> (x, y, width, height).toFloat());
        return canvas;
    }

    std::shared_ptr<Canvas> createFromBitmap (const lunasvg::Bitmap&) override
    {
        jassertfalse;
        return {};
    }

public:
    explicit DrawableCanvas (const lunasvg::Document* documentIn)
        : document (*documentIn)
    {
    }

    void setColor (const lunasvg::Color& color) override
    {
        getState().fillType = FillType { LunaSvgConverters::toJuceColour (color) };
    }

    void setColor (float r, float g, float b, float a) override
    {
        getState().fillType = Colour::fromFloatRGBA (r, g, b, a);
    }

    void setLinearGradient (float x1,
                            float y1,
                            float x2,
                            float y2,
                            lunasvg::SpreadMethod spread,
                            const lunasvg::GradientStops& stops,
                            const lunasvg::Transform& transform) override
    {
        ColourGradient gradient;
        gradient.point1 = { x1, y1 };
        gradient.point2 = { x2, y2 };
        gradient.spreadMethod = LunaSvgConverters::toJuceSpreadMethod (spread);

        for (const auto& stop : stops)
            gradient.addColour (stop.offset, LunaSvgConverters::toJuceColour (stop.color));

        const auto gradientTransform = LunaSvgConverters::toJuceTransform (transform);
        getState().fillType = FillType { gradient }.transformed (gradientTransform);
    }

    void setRadialGradient (float cx,
                            float cy,
                            float r,
                            float fx,
                            float fy,
                            lunasvg::SpreadMethod spread,
                            const lunasvg::GradientStops& stops,
                            const lunasvg::Transform& transform) override
    {
        ColourGradient gradient;
        gradient.isRadial = true;

        for (const auto& stop : stops)
            gradient.addColour (stop.offset, LunaSvgConverters::toJuceColour (stop.color));

        const auto gradientTransform = LunaSvgConverters::toJuceTransform (transform);
        const Point<float> endCentre { cx, cy };
        const Point<float> startCentre { fx, fy };

        gradient.point1 = startCentre;
        gradient.point2 = endCentre;
        gradient.endRadius = r;
        gradient.spreadMethod = LunaSvgConverters::toJuceSpreadMethod (spread);

        getState().fillType = FillType { gradient }.transformed (gradientTransform);
    }

    void setTexture (const Canvas& source,
                     lunasvg::TextureType type,
                     float opacity,
                     const lunasvg::Transform& transform) override
    {
        jassertquiet (type == lunasvg::TextureType::Tiled);
        const DrawableCanvas* drawableCanvas = dynamic_cast<const DrawableCanvas*> (&source);

        if (drawableCanvas == nullptr)
        {
            jassertfalse;
            return;
        }

        auto image = drawableCanvas->getJuceSurface();
        auto fill = FillType { std::move (image), LunaSvgConverters::toJuceTransform (transform) };
        fill.setOpacity (opacity);
        getState().fillType = std::move (fill);
    }

    static void applyClipPath (const Drawable* clip, Drawable& drawable)
    {
        if (clip == nullptr)
            return;

        auto clipForDrawable = clip->createCopy();
        auto t = clip->getDrawableTransform();
        clipForDrawable->setDrawableTransform (t.followedBy (drawable.getDrawableTransform().inverted()));
        drawable.setClipPath (std::move (clipForDrawable));
    }

    void fillPath (const lunasvg::Path& path,
                   lunasvg::FillRule fillRule,
                   const lunasvg::Transform& transform) override
    {
        auto p = std::make_unique<DrawablePath>();
        p->setName (document.getElementIdBeingRendered());
        p->setFill (getState().fillType);
        p->setPath (LunaSvgConverters::toJucePath (path, fillRule));
        p->setDrawableTransform (LunaSvgConverters::toJuceTransform (transform));
        applyClipPath (getState().clipPath.get(), *p);
        lastPath = p.get();
        composite->addChild (std::move (p));
    }

    void strokePath (const lunasvg::Path& path,
                     const lunasvg::StrokeData& strokeData,
                     const lunasvg::Transform& transform) override
    {
        const auto configureStrokeAndResetLastPath = [&] (DrawablePath& drawablePath)
        {
            drawablePath.setStrokeFill (getState().fillType);

            PathStrokeType strokeType { strokeData.lineWidth(),
                                        LunaSvgConverters::toJuceJointStyle (strokeData.lineJoin()),
                                        LunaSvgConverters::toJuceEndCapStyle (strokeData.lineCap()) };

            strokeType.setMiterLimit (strokeData.miterLimit());
            drawablePath.setStrokeType (strokeType);

            drawablePath.setDashLengths (strokeData.dashArray());
            drawablePath.setDashOffset (strokeData.dashOffset());

            lastPath = nullptr;
        };

        const auto jucePath = LunaSvgConverters::toJucePath (path);
        const auto juceTransform = LunaSvgConverters::toJuceTransform (transform);

        if (lastPath != nullptr
            && lastPath->getPath() == jucePath
            && lastPath->getDrawableTransform() == juceTransform)
        {
            configureStrokeAndResetLastPath (*lastPath);
            return;
        }

        auto p = std::make_unique<DrawablePath>();
        p->setName (document.getElementIdBeingRendered());
        p->setFill (Colour::fromRGBA (0, 0, 0, 0));
        configureStrokeAndResetLastPath (*p);
        p->setPath (jucePath);
        p->setDrawableTransform (juceTransform);
        applyClipPath (getState().clipPath.get(), *p);
        composite->addChild (std::move (p));
    }

    class LunaSvgTextParameters
    {
    public:
        LunaSvgTextParameters (const std::u32string_view& textIn,
                               const lunasvg::Font& fontIn,
                               const lunasvg::Point& originIn,
                               const lunasvg::Transform& transformIn)
            : text { toJuceString (textIn) },
              font { fontIn.getFont() },
              origin { originIn.x, originIn.y },
              transform { LunaSvgConverters::toJuceTransform (transformIn) }
        {
        }

        std::unique_ptr<DrawableText> createDrawableText()
        {
            auto t = std::make_unique<DrawableText>();
            t->setPreserveWhitespace (true);
            t->setFont (font, true);
            t->setDrawableTransform (transform);
            t->setText (text);

            const Rectangle<float> bounds { origin.x,
                                            origin.y - font.getAscent(),
                                            GlyphArrangement::getStringWidth (font, text),
                                            font.getHeight() };

            t->setBoundingBox (bounds);
            drawableText = t.get();

            return t;
        }

        void setStrokeOptionsOnLastObject (const StrokeOptions& attrs)
        {
            if (drawableText == nullptr)
                return;

            drawableText->setStrokeOptions (attrs);
        }

        void setFillOnLastObject (const FillType& fill)
        {
            if (drawableText == nullptr)
                return;

            drawableText->setFill (fill);
        }

        bool operator== (const LunaSvgTextParameters& other) const noexcept
        {
            return text == other.text
                && font == other.font
                && origin == other.origin
                && transform == other.transform;
        }

        bool operator!= (const LunaSvgTextParameters& other) const noexcept
        {
            return ! operator== (other);
        }

    private:
        static String toJuceString (const std::u32string_view& view)
        {
            return juce::String { juce::CharPointer_UTF32 (reinterpret_cast<const CharPointer_UTF32::CharType*> (view.data())),
                                  view.length() };
        }

        String text;
        Font font;
        Point<float> origin;
        AffineTransform transform;
        DrawableText* drawableText = nullptr;
    };

    void fillText (const std::u32string_view& text,
                   const lunasvg::Font& font,
                   const lunasvg::Point& origin,
                   const lunasvg::Transform& transform) override
    {
        lastTextParams = LunaSvgTextParameters { text, font, origin, transform };
        auto t = lastTextParams->createDrawableText();
        t->setName (document.getElementIdBeingRendered());
        t->setFill (getState().fillType);
        applyClipPath (getState().clipPath.get(), *t);
        composite->addChild (std::move (t));
    }

    void strokeText (const std::u32string_view& text,
                     float strokeWidth,
                     const lunasvg::Font& font,
                     const lunasvg::Point& origin,
                     const lunasvg::Transform& transform) override
    {
        const auto attrs = StrokeOptions{}.withWidth (strokeWidth).withColour (getState().fillType.colour);
        LunaSvgTextParameters textParams { text, font, origin, transform };

        if (auto lastParams = std::exchange (lastTextParams, {}); lastParams.has_value() && *lastParams == textParams)
        {
            lastParams->setStrokeOptionsOnLastObject (attrs);
            return;
        }

        auto t = textParams.createDrawableText();
        t->setName (document.getElementIdBeingRendered());
        t->setFill (FillType { Colours::transparentBlack });
        t->setStrokeOptions (attrs);
        applyClipPath (getState().clipPath.get(), *t);
        composite->addChild (std::move (t));
    }

    void clipPath (const Path& path,
                   const lunasvg::Transform& transform)
    {
        auto clip = std::make_unique<DrawablePath>();
        clip->setPath (path);
        clip->setDrawableTransform (LunaSvgConverters::toJuceTransform (transform));
        getState().clipPath = std::move (clip);
    }

    void clipPath (const lunasvg::Path& path,
                   lunasvg::FillRule clipRule,
                   const lunasvg::Transform& transform) override
    {
        clipPath (LunaSvgConverters::toJucePath (path, clipRule), transform);
    }

    void clipRect (const lunasvg::Rect& rect,
                   lunasvg::FillRule clipRule,
                   const lunasvg::Transform& transform) override
    {
        auto r = LunaSvgConverters::toJuceRectangle (rect);
        Path p;
        p.addRectangle (r);
        p.setUsingNonZeroWinding (clipRule == lunasvg::FillRule::NonZero);
        clipPath (p, transform);
    }

    void drawImage (const lunasvg::Bitmap& image,
                    const lunasvg::Rect& dstRect,
                    const lunasvg::Rect& srcRect,
                    const lunasvg::Transform& transform) override
    {
        auto i = std::make_unique<DrawableImage> (LunaSvgConverters::toJuceImage (image.surface()),
                                                  LunaSvgConverters::toJuceIntRectangle (dstRect),
                                                  LunaSvgConverters::toJuceIntRectangle (srcRect));
        i->setName (document.getElementIdBeingRendered());

        i->setDrawableTransform (LunaSvgConverters::toJuceTransform (transform));
        applyClipPath (getState().clipPath.get(), *i);
        composite->addChild (std::move (i));
    }

    void blendCanvas (const Canvas& canvas, lunasvg::BlendMode blendMode, float opacity) override
    {
        const DrawableCanvas* drawableCanvas = dynamic_cast<const DrawableCanvas*> (&canvas);

        if (drawableCanvas == nullptr)
        {
            // If this happened, we didn't replace all canvas objects inside lunasvg with DrawableCanvas
            jassertfalse;
            return;
        }

        std::unique_ptr<DrawableComposite> other { static_cast<DrawableComposite*> (drawableCanvas->composite->createCopy().release()) };
        other->setBlendMode (LunaSvgConverters::toJuceBlendMode (blendMode));
        other->setOpacity (opacity);
        composite->addChild (std::move (other));
    }

    void save() override
    {
        saveState();
    }

    void restore() override
    {
        restoreState();
    }

    void convertToLuminanceMask() override
    {
        composite->setLuminanceMask (true);
    }

    int x() const override
    {
        return getContentArea().getSmallestIntegerContainer().getX();
    }

    int y() const override
    {
        return getContentArea().getSmallestIntegerContainer().getY();
    }

    int width() const override
    {
        return getContentArea().getSmallestIntegerContainer().getWidth();
    }

    int height() const override
    {
        return getContentArea().getSmallestIntegerContainer().getHeight();
    }

    lunasvg::Rect extents() const override
    {
        return LunaSvgConverters::fromJuceRectangle (getContentArea().getSmallestIntegerContainer().toFloat());
    }

    Image getJuceSurface() const
    {
        const auto extents = getContentArea().getSmallestIntegerContainer();
        Image image { Image::ARGB, extents.getWidth(), extents.getHeight(), true };
        Graphics g (image);
        g.addTransform (AffineTransform::translation ((float) -extents.getX(), (float) -extents.getY()));
        composite->draw (g, 1.0f);
        return image;
    }

    juce_plutovg_surface_t* surface() const override
    {
        // We could implement this by converting the result of getJuceSurface(), but if every angle
        // is covered by our code, this shouldn't be called, and our code can call getJuceSurface()
        // directly.
        jassertfalse;
        return nullptr;
    }

    juce_plutovg_canvas_t* canvas() const override
    {
        // Not implemented
        jassertfalse;
        return nullptr;
    }

    std::unique_ptr<DrawableComposite> release()
    {
        return std::move (composite);
    }

    State& getState()
    {
        if (state.empty())
            state.push (State());

        return state.top();
    }

    void saveState()
    {
        state.push (getState());
    }

    void restoreState()
    {
        if (state.size() > 1)
            state.pop();
    }

    void setContentArea (const Rectangle<float>& area)
    {
        composite->setContentArea (area);
        composite->resetBoundingBoxToContentArea();
    }

    Rectangle<float> getContentArea() const
    {
        return composite->getContentArea();
    }

    const lunasvg::Document& document;
    std::stack<State> state;
    std::unique_ptr<DrawableComposite> composite = std::make_unique<DrawableComposite>();
    DrawablePath* lastPath = nullptr;
    std::optional<LunaSvgTextParameters> lastTextParams;
};

}

std::unique_ptr<DrawableComposite> Drawable::createFromSVGString (const String& svgString)
{
    const auto document = lunasvg::Document::loadFromData (svgString.toStdString());

    if (document == nullptr)
        return {};

    auto canvas = std::make_shared<DrawableCanvas> (document.get());
    canvas->setContentArea ({ 0.0f, 0.0f, document->width(), document->height() });
    document->renderToCanvas (canvas, lunasvg::Matrix());

    return canvas->release();
}

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

struct SvgParserTests final : public UnitTest
{
    SvgParserTests() : UnitTest ("SvgParser", UnitTestCategories::graphics)
    {
    }

    void runTest() override
    {
        testCase ("Path parsing regression test", [this]
        {
            const auto juceLogoSvgPathString =
                "m 72.87 84.28 c 49.518 84.341 30.521 65.492 30.4 42.14 30.756 18.936 49.668 "
                "0.312 72.875 0.312 96.082 0.312 114.994 18.936 115.35 42.14 115.229 65.496 "
                "96.226 84.346 72.87 84.28 z m 72.87 5.61 c 52.61 5.538 36.116 21.88 36 42.14 "
                "36.332 62.269 52.744 78.412 72.875 78.412 93.006 78.412 109.418 62.269 109.75 "
                "42.14 109.629 21.879 93.132 5.538 72.87 5.61 z m 77.62 49.59 c 80.16 56.066 "
                "83.079 62.386 86.36 68.52 86.959 69.604 87.99 70.384 89.196 70.666 90.403 70.948 "
                "91.672 70.706 92.69 70 96.165 67.569 99.162 64.518 101.53 61 102.279 59.861 "
                "102.445 58.435 101.975 57.155 101.506 55.876 100.458 54.894 99.15 54.51 92.633 "
                "52.485 86.239 50.084 80 47.32 79.342 47.029 78.573 47.163 78.052 47.66 77.531 "
                "48.157 77.36 48.919 77.62 49.59 z m 81.05 44.27 c 87.597 47.162 94.32 49.637 "
                "101.18 51.68 102.367 52.019 103.642 51.843 104.693 51.194 105.743 50.545 106.472 "
                "49.483 106.7 48.27 107.066 46.247 107.25 44.196 107.25 42.14 107.251 39.883 "
                "107.027 37.632 106.58 35.42 106.306 34.074 105.415 32.934 104.174 32.344 102.933 "
                "31.754 101.487 31.782 100.27 32.42 94.041 35.627 87.642 38.491 81.1 41 80.426 "
                "41.256 79.977 41.897 79.966 42.618 79.955 43.339 80.385 43.993 81.05 44.27 z m "
                "74.47 50.44 c 74.195 49.774 73.545 49.34 72.825 49.34 72.105 49.34 71.455 49.774 "
                "71.18 50.44 68.27 56.903 65.778 63.547 63.72 70.33 63.375 71.521 63.557 72.804 "
                "64.221 73.852 64.884 74.9 65.965 75.613 67.19 75.81 69.068 76.115 70.967 76.269 "
                "72.87 76.27 75.268 76.256 77.657 75.991 80 75.48 81.341 75.219 82.479 74.34 "
                "83.07 73.109 83.661 71.877 83.635 70.439 83 69.23 79.814 63.128 76.967 56.855 "
                "74.47 50.44 z m 71.59 34.12 c 71.855 34.79 72.498 35.234 73.218 35.245 73.939 "
                "35.256 74.595 34.832 74.88 34.17 77.823 27.638 80.336 20.92 82.4 14.06 82.739 "
                "12.88 82.563 11.612 81.915 10.57 81.267 9.527 80.208 8.808 79 8.59 74.674 7.83 "
                "70.244 7.888 65.94 8.76 64.596 9.02 63.455 9.9 62.864 11.135 62.272 12.369 "
                "62.301 13.81 62.94 15.02 66.176 21.221 69.063 27.598 71.59 34.12 z m 46.32 30.3 "
                "c 53.132 32.387 59.811 34.885 66.32 37.78 66.98 38.068 67.748 37.931 68.267 "
                "37.432 68.785 36.933 68.952 36.17 68.69 35.5 66.049 28.709 63.001 22.083 59.56 "
                "15.66 58.959 14.577 57.928 13.8 56.721 13.52 55.515 13.239 54.247 13.483 53.23 "
                "14.19 49.518 16.761 46.351 20.041 43.91 23.84 43.178 24.982 43.026 26.402 43.5 "
                "27.672 43.974 28.943 45.019 29.917 46.32 30.3 z m 68.17 49.18 c 68.453 48.521 "
                "68.311 47.756 67.809 47.243 67.307 46.73 66.545 46.571 65.88 46.84 59.209 49.394 "
                "52.694 52.339 46.37 55.66 45.266 56.244 44.47 57.279 44.19 58.496 43.91 59.713 "
                "44.173 60.992 44.91 62 47.459 65.53 50.656 68.544 54.33 70.88 55.475 71.608 "
                "56.893 71.761 58.167 71.294 59.441 70.828 60.426 69.795 60.83 68.5 62.894 61.921 "
                "65.345 55.47 68.17 49.18 z m 77.79 35.59 c 77.508 36.252 77.652 37.019 78.155 "
                "37.533 78.659 38.047 79.422 38.208 80.09 37.94 86.792 35.368 93.337 32.403 99.69 "
                "29.06 100.773 28.481 101.557 27.466 101.843 26.272 102.13 25.078 101.892 23.818 "
                "101.19 22.81 98.669 19.188 95.474 16.085 91.78 13.67 90.639 12.925 89.216 12.756 "
                "87.932 13.213 86.649 13.67 85.653 14.701 85.24 16 83.151 22.673 80.664 29.215 "
                "77.79 35.59 z m 64.69 40.6 c 58.116 37.689 51.362 35.204 44.47 33.16 43.285 "
                "32.828 42.014 33.012 40.972 33.667 39.931 34.323 39.214 35.388 39 36.6 38.698 "
                "38.431 38.547 40.284 38.55 42.14 38.548 44.629 38.82 47.11 39.36 49.54 39.673 "
                "50.851 40.576 51.944 41.804 52.499 43.032 53.055 44.448 53.011 45.64 52.38 "
                "51.812 49.193 58.155 46.349 64.64 43.86 65.307 43.6 65.751 42.963 65.761 42.247 "
                "65.772 41.531 65.349 40.88 64.69 40.6 z m 20 129.315 c 20 134.315 17.28 137.475 "
                "12.89 137.475 10.52 137.475 8.72 136.475 6.69 133.915 l 6 133.135 -0 138.135 "
                "0.57 138.895 c 3.82 143.255 7.73 145.285 12.88 145.285 21.88 145.285 28.22 "
                "138.715 28.22 129.285 l 28.22 101.185 20 101.185 z m 61.69 126.505 c 61.69 "
                "133.165 57.93 137.505 52.12 137.505 46.31 137.505 42.56 133.195 42.56 126.505 l "
                "42.56 101.185 34.33 101.185 34.33 126.875 c 34.33 137.535 41.73 145.275 51.93 "
                "145.275 61.93 145.275 69.54 137.555 69.93 126.875 l 69.93 101.185 61.69 101.185 "
                "z m 106.83 134.095 c 103.25 136.525 100.65 137.475 97.58 137.475 89.933 136.983 "
                "83.983 130.637 83.983 122.975 83.983 115.313 89.933 108.967 97.58 108.475 100.82 "
                "108.475 103.24 109.355 106.83 111.855 l 107.59 112.385 112.37 106.385 111.62 "
                "105.765 c 107.623 102.453 102.591 100.649 97.4 100.665 89.311 100.494 81.763 "
                "104.711 77.669 111.689 73.574 118.667 73.574 127.313 77.669 134.291 81.763 "
                "141.269 89.311 145.486 97.4 145.315 102.656 145.435 107.774 143.628 111.79 "
                "140.235 l 112.6 139.595 107.6 133.595 z m 145.75 137.285 l 126.69 137.285 126.69 "
                "126.565 144.99 126.565 144.99 118.955 126.69 118.955 126.69 108.795 145.75 "
                "108.795 145.75 101.185 118.47 101.185 118.47 144.715 145.75 144.715 z m 68.015 "
                "83.917 c 60.292 83.015 52.543 79.794 46.449 74.951 37.974 68.215 32.277 58.128 "
                "30.875 47.376 29.303 35.31 33.538 22.7 42.21 13.631 49.154 6.368 58.07 1.902 "
                "68.042 0.695 70.192 0.435 75.566 0.435 77.717 0.695 90.205 2.207 101.181 8.945 "
                "108.154 19.381 116.486 31.852 117.472 47.504 110.759 60.749 108.479 65.249 "
                "106.422 68.108 102.909 71.658 96.123 78.511 87.197 82.838 77.613 83.92 75.586 "
                "84.147 69.969 84.145 68.015 83.917 z m 75.838 78.321 c 84.273 77.906 93.284 "
                "73.643 99.521 67.116 105.497 60.862 108.871 53.393 109.702 44.579 110.334 37.874 "
                "108.356 29.631 104.637 23.471 98.88 13.935 89.397 7.602 78.34 5.906 75.799 5.516 "
                "69.942 5.52 67.38 5.912 53.54 8.034 42.185 17.542 37.81 30.67 35.003 39.096 "
                "35.389 47.937 38.92 56.114 43.797 67.411 53.879 75.524 65.897 77.823 68.033 "
                "78.231 71.997 78.578 73.274 78.468 73.599 78.44 74.754 78.374 75.838 78.321 z";

            const auto juceLogoPathString =
                "m 72.87 84.28 c 122.388 168.621 103.391 149.772 103.27 126.42 134.026 145.356 "
                "152.938 126.732 176.145 126.732 272.227 127.044 291.139 145.668 291.495 168.872 "
                "406.724 234.368 387.721 253.218 364.365 253.152 z m 145.74 89.89 c 198.35 95.428 "
                "181.856 111.77 181.74 132.03 218.072 194.299 234.484 210.442 254.615 210.442 "
                "347.621 288.854 364.033 272.711 364.365 252.582 473.994 274.461 457.497 258.12 "
                "437.235 258.192 z m 223.36 139.48 c 303.52 195.546 306.439 201.866 309.72 208 "
                "396.679 277.604 397.71 278.384 398.916 278.666 489.319 349.614 490.588 349.372 "
                "491.606 348.666 587.771 416.235 590.768 413.184 593.136 409.666 695.415 469.527 "
                "695.581 468.101 695.111 466.821 796.617 522.697 795.569 521.715 794.261 521.331 "
                "886.894 573.816 880.5 571.415 874.261 568.651 953.603 615.68 952.834 615.814 "
                "952.313 616.311 1029.844 664.468 1029.673 665.23 1029.933 665.901 z m 304.41 "
                "183.75 c 392.007 230.912 398.73 233.387 405.59 235.43 507.957 287.449 509.232 "
                "287.273 510.283 286.624 616.026 337.169 616.755 336.107 616.983 334.894 724.049 "
                "381.141 724.233 379.09 724.233 377.034 831.484 416.917 831.26 414.666 830.813 "
                "412.454 937.119 446.528 936.228 445.388 934.987 444.798 1037.92 476.552 1036.474 "
                "476.58 1035.257 477.218 1129.298 512.845 1122.899 515.709 1116.357 518.218 "
                "1196.783 559.474 1196.334 560.115 1196.323 560.836 1276.278 604.175 1276.708 "
                "604.829 1277.373 605.106 z m 378.88 234.19 c 453.075 283.964 452.425 283.53 "
                "451.705 283.53 523.81 332.87 523.16 333.304 522.885 333.97 591.155 390.873 "
                "588.663 397.517 586.605 404.3 649.98 475.821 650.162 477.104 650.826 478.152 "
                "715.71 553.052 716.791 553.765 718.016 553.962 787.084 630.077 788.983 630.231 "
                "790.886 630.232 866.154 706.488 868.543 706.223 870.886 705.712 952.227 780.931 "
                "953.365 780.052 953.956 778.821 1037.617 850.698 1037.591 849.26 1036.956 "
                "848.051 1116.77 911.179 1113.923 904.906 1111.426 898.491 z m 450.47 268.31 c "
                "522.325 303.1 522.968 303.544 523.688 303.555 597.627 338.811 598.283 338.387 "
                "598.568 337.725 676.391 365.363 678.904 358.645 680.968 351.785 763.707 364.665 "
                "763.531 363.397 762.883 362.355 844.15 371.882 843.091 371.163 841.883 370.945 "
                "916.557 378.775 912.127 378.833 907.823 379.705 972.419 388.725 971.278 389.605 "
                "970.687 390.84 1032.959 403.209 1032.988 404.65 1033.627 405.86 1099.803 427.081 "
                "1102.69 433.458 1105.217 439.98 z m 496.79 298.61 c 549.922 330.997 556.601 "
                "333.495 563.11 336.39 630.09 374.458 630.858 374.321 631.377 373.822 700.162 "
                "410.755 700.329 409.992 700.067 409.322 766.116 438.031 763.068 431.405 759.627 "
                "424.982 818.586 439.559 817.555 438.782 816.348 438.502 871.863 451.741 870.595 "
                "451.985 869.578 452.692 919.096 469.453 915.929 472.733 913.488 476.532 956.666 "
                "501.514 956.514 502.934 956.988 504.204 1000.962 533.147 1002.007 534.121 "
                "1003.308 534.504 z m 564.96 347.79 c 633.413 396.311 633.271 395.546 632.769 "
                "395.033 700.076 441.763 699.314 441.604 698.649 441.873 757.858 491.267 751.343 "
                "494.212 745.019 497.533 790.285 553.777 789.489 554.812 789.209 556.029 833.119 "
                "615.742 833.382 617.021 834.119 618.029 881.578 683.559 884.775 686.573 888.449 "
                "688.909 943.924 760.517 945.342 760.67 946.616 760.203 1006.057 831.031 1007.042 "
                "829.998 1007.446 828.703 1070.34 890.624 1072.791 884.173 1075.616 877.883 z m "
                "642.75 383.38 c 720.258 419.632 720.402 420.399 720.905 420.913 799.564 458.96 "
                "800.327 459.121 800.995 458.853 887.787 494.221 894.332 491.256 900.685 487.913 "
                "1001.458 516.394 1002.242 515.379 1002.528 514.185 1104.658 539.263 1104.42 "
                "538.003 1103.718 536.995 1202.387 556.183 1199.192 553.08 1195.498 550.665 "
                "1286.137 563.59 1284.714 563.421 1283.43 563.878 1370.079 577.548 1369.083 "
                "578.579 1368.67 579.878 1451.821 602.551 1449.334 609.093 1446.46 615.468 z m "
                "707.44 423.98 c 765.556 461.669 758.802 459.184 751.91 457.14 795.195 489.968 "
                "793.924 490.152 792.882 490.807 832.813 525.13 832.096 526.195 831.882 527.407 "
                "870.58 565.838 870.429 567.691 870.432 569.547 908.98 614.176 909.252 616.657 "
                "909.792 619.087 949.465 669.938 950.368 671.031 951.596 671.586 994.628 724.641 "
                "996.044 724.597 997.236 723.966 1049.048 773.159 1055.391 770.315 1061.876 "
                "767.826 1127.183 811.426 1127.627 810.789 1127.637 810.073 1193.409 851.604 "
                "1192.986 850.953 1192.327 850.673 z m 727.44 553.295 c 747.44 687.61 744.72 "
                "690.77 740.33 690.77 750.85 828.245 749.05 827.245 747.02 824.685 l 753.02 "
                "957.82 753.02 1095.955 753.59 1234.85 c 757.41 1378.105 761.32 1380.135 766.47 "
                "1380.135 788.35 1525.42 794.69 1518.85 794.69 1509.42 l 822.91 1610.605 842.91 "
                "1711.79 z m 789.13 679.8 c 850.82 812.965 847.06 817.305 841.25 817.305 887.56 "
                "954.81 883.81 950.5 883.81 943.81 l 926.37 1044.995 960.7 1146.18 995.03 "
                "1273.055 c 1029.36 1410.59 1036.76 1418.33 1046.96 1418.33 1108.89 1563.605 "
                "1116.5 1555.885 1116.89 1545.205 l 1186.82 1646.39 1248.51 1747.575 z m 895.96 "
                "813.895 c 999.21 950.42 996.61 951.37 993.54 951.37 1083.473 1088.353 1077.523 "
                "1082.007 1077.523 1074.345 1161.506 1189.658 1167.456 1183.312 1175.103 1182.82 "
                "1275.923 1291.295 1278.343 1292.175 1281.933 1294.675 l 1389.523 1407.06 "
                "1501.893 1513.445 1613.513 1619.21 c 1721.136 1721.663 1716.104 1719.859 "
                "1710.913 1719.875 1800.224 1820.369 1792.676 1824.586 1788.582 1831.564 1862.156 "
                "1950.231 1862.156 1958.877 1866.251 1965.855 1948.014 2107.124 1955.562 2111.341 "
                "1963.651 2111.17 2066.307 2256.605 2071.425 2254.798 2075.441 2251.405 l "
                "2188.041 2391 2295.641 2524.595 z m 1041.71 951.18 l 1168.4 1088.465 1295.09 "
                "1215.03 1440.08 1341.595 1585.07 1460.55 1711.76 1579.505 1838.45 1688.3 1984.2 "
                "1797.095 2129.95 1898.28 2248.42 1999.465 2366.89 2144.18 2512.64 2288.895 z m "
                "1109.725 1035.097 c 1170.017 1118.112 1162.268 1114.891 1156.174 1110.048 "
                "1194.148 1178.263 1188.451 1168.176 1187.049 1157.424 1216.352 1192.734 1220.587 "
                "1180.124 1229.259 1171.055 1278.413 1177.423 1287.329 1172.957 1297.301 1171.75 "
                "1367.493 1172.185 1372.867 1172.185 1375.018 1172.445 1465.223 1174.652 1476.199 "
                "1181.39 1483.172 1191.826 1599.658 1223.678 1600.644 1239.33 1593.931 1252.575 "
                "1702.41 1317.824 1700.353 1320.683 1696.84 1324.233 1792.963 1402.744 1784.037 "
                "1407.071 1774.453 1408.153 1850.039 1492.3 1844.422 1492.298 1842.468 1492.07 z "
                "m 1185.563 1113.418 c 1269.836 1191.324 1278.847 1187.061 1285.084 1180.534 "
                "1390.581 1241.396 1393.955 1233.927 1394.786 1225.113 1505.12 1262.987 1503.142 "
                "1254.744 1499.423 1248.584 1598.303 1262.519 1588.82 1256.186 1577.763 1254.49 "
                "1653.562 1260.006 1647.705 1260.01 1645.143 1260.402 1698.683 1268.436 1687.328 "
                "1277.944 1682.953 1291.072 1717.956 1330.168 1718.342 1339.009 1721.873 1347.186 "
                "1765.67 1414.597 1775.752 1422.71 1787.77 1425.009 1855.803 1503.24 1859.767 "
                "1503.587 1861.044 1503.477 1934.643 1581.917 1935.798 1581.851 1936.882 1581.798 z";

            expect (Drawable::parseSVGPath (juceLogoSvgPathString).toString() == juceLogoPathString);
        });
    }
};

static SvgParserTests svgParserTests;

#endif

} // namespace juce
