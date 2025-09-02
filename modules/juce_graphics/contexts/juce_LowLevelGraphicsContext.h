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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
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

namespace juce
{

//==============================================================================
/**
    Interface class for graphics context objects, used internally by the Graphics class.

    Users are not supposed to create instances of this class directly - do your drawing
    via the Graphics object instead.

    It's a base class for different types of graphics context, that may perform software-based
    or OS-accelerated rendering.

    E.g. the LowLevelGraphicsSoftwareRenderer renders onto an image in memory, but other
    subclasses could render directly to a windows HDC, a Quartz context, or an OpenGL
    context.

    @tags{Graphics}
*/
class JUCE_API  LowLevelGraphicsContext
{
protected:
    //==============================================================================
    LowLevelGraphicsContext() = default;

public:
    virtual ~LowLevelGraphicsContext() = default;

    /** Returns true if this device is vector-based, e.g. a printer. */
    virtual bool isVectorDevice() const = 0;

    //==============================================================================
    /** Moves the origin to a new position.

        The coordinates are relative to the current origin, and indicate the new position
        of (0, 0).
    */
    virtual void setOrigin (Point<int>) = 0;
    virtual void addTransform (const AffineTransform&) = 0;
    virtual float getPhysicalPixelScaleFactor() const = 0;

    virtual bool clipToRectangle (const Rectangle<int>&) = 0;
    virtual bool clipToRectangleList (const RectangleList<int>&) = 0;
    virtual void excludeClipRectangle (const Rectangle<int>&) = 0;
    virtual void clipToPath (const Path&, const AffineTransform&) = 0;
    virtual void clipToImageAlpha (const Image&, const AffineTransform&) = 0;

    virtual bool clipRegionIntersects (const Rectangle<int>&) = 0;
    virtual Rectangle<int> getClipBounds() const = 0;
    virtual bool isClipEmpty() const = 0;

    virtual void saveState() = 0;
    virtual void restoreState() = 0;

    virtual void beginTransparencyLayer (float opacity) = 0;
    virtual void endTransparencyLayer() = 0;

    //==============================================================================
    virtual void setFill (const FillType&) = 0;
    virtual void setOpacity (float) = 0;
    virtual void setInterpolationQuality (Graphics::ResamplingQuality) = 0;

    //==============================================================================
    virtual void fillAll() { fillRect (getClipBounds(), false); }
    virtual void fillRect (const Rectangle<int>&, bool replaceExistingContents) = 0;
    virtual void fillRect (const Rectangle<float>&) = 0;
    virtual void fillRectList (const RectangleList<float>&) = 0;
    virtual void fillPath (const Path&, const AffineTransform&) = 0;

    virtual void drawRect (const Rectangle<float>& rect, float lineThickness)
    {
        auto r = rect;
        RectangleList<float> rects;
        rects.addWithoutMerging (r.removeFromTop    (lineThickness));
        rects.addWithoutMerging (r.removeFromBottom (lineThickness));
        rects.addWithoutMerging (r.removeFromLeft   (lineThickness));
        rects.addWithoutMerging (r.removeFromRight  (lineThickness));
        fillRectList (rects);
    }

    virtual void strokePath (const Path& path, const PathStrokeType& strokeType, const AffineTransform& transform)
    {
        Path stroke;
        strokeType.createStrokedPath (stroke, path, transform, getPhysicalPixelScaleFactor());
        fillPath (stroke, {});
    }

    virtual void drawImage (const Image&, const AffineTransform&) = 0;
    virtual void drawLine (const Line<float>&) = 0;

    virtual void drawLineWithThickness (const Line<float>& line, float lineThickness)
    {
        Path p;
        p.addLineSegment (line, lineThickness);
        fillPath (p, {});
    }

    virtual void setFont (const Font&) = 0;
    virtual const Font& getFont() = 0;

    /** Uses the current font to draw the provided glyph numbers. */
    virtual void drawGlyphs (Span<const uint16_t>,
                             Span<const Point<float>>,
                             const AffineTransform&) = 0;

    /** Returns the optimal ImageType for creating temporary images in this GraphicsContext.

        While this typically matches the GraphicsContext's native ImageType, certain scenarios
        may benefit from using a different format for temporary operations (e.g., for
        performance, memory efficiency, or specific rendering requirements).

        @return A unique_ptr to the recommended ImageType instance for temporary images
    */
    virtual std::unique_ptr<ImageType> getPreferredImageTypeForTemporaryImages() const = 0;

    virtual void drawRoundedRectangle (const Rectangle<float>& r, float cornerSize, float lineThickness)
    {
        Path p;
        p.addRoundedRectangle (r, cornerSize);
        strokePath (p, PathStrokeType (lineThickness), {});
    }

    virtual void fillRoundedRectangle (const Rectangle<float>& r, float cornerSize)
    {
        Path p;
        p.addRoundedRectangle (r, cornerSize);
        fillPath (p, {});
    }

    virtual void drawEllipse (const Rectangle<float>& area, float lineThickness)
    {
        Path p;

        if (approximatelyEqual (area.getWidth(), area.getHeight()))
        {
            // For a circle, we can avoid having to generate a stroke
            p.addEllipse (area.expanded (lineThickness * 0.5f));
            p.addEllipse (area.reduced  (lineThickness * 0.5f));
            p.setUsingNonZeroWinding (false);
            fillPath (p, {});
        }
        else
        {
            p.addEllipse (area);
            strokePath (p, PathStrokeType (lineThickness), {});
        }
    }

    virtual void fillEllipse (const Rectangle<float>& area)
    {
        Path p;
        p.addEllipse (area);
        fillPath (p, {});
    }

    /** Returns an integer that uniquely identifies the current frame.
        Useful for debugging/logging.
    */
    virtual uint64_t getFrameId() const = 0;
};

} // namespace juce
