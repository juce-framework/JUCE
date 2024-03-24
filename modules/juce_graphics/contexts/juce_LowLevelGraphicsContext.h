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
    virtual float getPhysicalPixelScaleFactor() = 0;

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
    virtual void drawImage (const Image&, const AffineTransform&) = 0;
    virtual void drawLine (const Line<float>&) = 0;

    virtual void setFont (const Font&) = 0;
    virtual const Font& getFont() = 0;
    virtual void drawGlyph (int glyphNumber, const AffineTransform&) = 0;
    virtual bool drawTextLayout (const AttributedString&, const Rectangle<float>&)  { return false; }
};

} // namespace juce
