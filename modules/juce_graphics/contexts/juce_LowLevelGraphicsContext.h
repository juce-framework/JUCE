/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_LOWLEVELGRAPHICSCONTEXT_JUCEHEADER__
#define __JUCE_LOWLEVELGRAPHICSCONTEXT_JUCEHEADER__

#include "../images/juce_Image.h"
#include "../geometry/juce_Path.h"
#include "../geometry/juce_RectangleList.h"
#include "../colour/juce_ColourGradient.h"
#include "../colour/juce_FillType.h"
class AttributedString;

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
*/
class JUCE_API  LowLevelGraphicsContext
{
protected:
    //==============================================================================
    LowLevelGraphicsContext();

public:
    virtual ~LowLevelGraphicsContext();

    /** Returns true if this device is vector-based, e.g. a printer. */
    virtual bool isVectorDevice() const = 0;

    //==============================================================================
    /** Moves the origin to a new position.

        The co-ords are relative to the current origin, and indicate the new position
        of (0, 0).
    */
    virtual void setOrigin (int x, int y) = 0;
    virtual void addTransform (const AffineTransform& transform) = 0;
    virtual float getScaleFactor() = 0;

    virtual float getTargetDeviceScaleFactor()    { return 1.0f; }

    virtual bool clipToRectangle (const Rectangle<int>& r) = 0;
    virtual bool clipToRectangleList (const RectangleList& clipRegion) = 0;
    virtual void excludeClipRectangle (const Rectangle<int>& r) = 0;
    virtual void clipToPath (const Path& path, const AffineTransform& transform) = 0;
    virtual void clipToImageAlpha (const Image& sourceImage, const AffineTransform& transform) = 0;

    virtual bool clipRegionIntersects (const Rectangle<int>& r) = 0;
    virtual Rectangle<int> getClipBounds() const = 0;
    virtual bool isClipEmpty() const = 0;

    virtual void saveState() = 0;
    virtual void restoreState() = 0;

    virtual void beginTransparencyLayer (float opacity) = 0;
    virtual void endTransparencyLayer() = 0;

    //==============================================================================
    virtual void setFill (const FillType& fillType) = 0;
    virtual void setOpacity (float newOpacity) = 0;
    virtual void setInterpolationQuality (Graphics::ResamplingQuality quality) = 0;

    //==============================================================================
    virtual void fillRect (const Rectangle<int>& r, bool replaceExistingContents) = 0;
    virtual void fillPath (const Path& path, const AffineTransform& transform) = 0;

    virtual void drawImage (const Image& sourceImage, const AffineTransform& transform) = 0;

    virtual void drawLine (const Line <float>& line) = 0;
    virtual void drawVerticalLine (int x, float top, float bottom) = 0;
    virtual void drawHorizontalLine (int y, float left, float right) = 0;

    virtual void setFont (const Font& newFont) = 0;
    virtual const Font& getFont() = 0;
    virtual void drawGlyph (int glyphNumber, const AffineTransform& transform) = 0;
    virtual bool drawTextLayout (const AttributedString&, const Rectangle<float>&)  { return false; }
};


#endif   // __JUCE_LOWLEVELGRAPHICSCONTEXT_JUCEHEADER__
