/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../imaging/juce_Image.h"
#include "../geometry/juce_Path.h"
#include "../geometry/juce_RectangleList.h"
#include "../colour/juce_ColourGradient.h"


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

    /** Cliping co-ords are relative to the origin. */
    virtual bool reduceClipRegion (int x, int y, int w, int h) = 0;

    /** Cliping co-ords are relative to the origin. */
    virtual bool reduceClipRegion (const RectangleList& clipRegion) = 0;

    //virtual bool clipToPath (const Path& path) = 0;
    //virtual bool clipToImageAlpha (Image& image, int imageX, int imageY) = 0;

    /** Cliping co-ords are relative to the origin. */
    virtual void excludeClipRegion (int x, int y, int w, int h) = 0;

    virtual void saveState() = 0;
    virtual void restoreState() = 0;

    virtual bool clipRegionIntersects (int x, int y, int w, int h) = 0;
    virtual const Rectangle getClipBounds() const = 0;
    virtual bool isClipEmpty() const = 0;

    //==============================================================================
    virtual void setColour (const Colour& colour) = 0;
    virtual void setGradient (const ColourGradient& gradient) = 0;
    virtual void setOpacity (float opacity) = 0;
    virtual void setInterpolationQuality (Graphics::ResamplingQuality quality) = 0;

    //==============================================================================
    virtual void fillRect (int x, int y, int w, int h, const bool replaceExistingContents) = 0;
    virtual void fillPath (const Path& path, const AffineTransform& transform) = 0;

    virtual void fillPathWithImage (const Path& path, const AffineTransform& transform,
                                    const Image& image, int imageX, int imageY) = 0;

    virtual void fillAlphaChannel (const Image& alphaImage, int alphaImageX, int alphaImageY) = 0;
    virtual void fillAlphaChannelWithImage (const Image& alphaImage, int alphaImageX, int alphaImageY,
                                            const Image& fillerImage, int fillerImageX, int fillerImageY) = 0;

    //==============================================================================
    virtual void blendImage (const Image& sourceImage,
                             int destX, int destY, int destW, int destH, int sourceX, int sourceY) = 0;

    virtual void blendImageWarping (const Image& sourceImage,
                                    int srcClipX, int srcClipY, int srcClipW, int srcClipH,
                                    const AffineTransform& transform) = 0;

    //==============================================================================
    virtual void drawLine (double x1, double y1, double x2, double y2) = 0;
    virtual void drawVerticalLine (const int x, double top, double bottom) = 0;
    virtual void drawHorizontalLine (const int y, double left, double right) = 0;

    virtual void setFont (const Font& newFont) = 0;
    virtual void drawGlyph (int glyphNumber, float x, float y) = 0;
    virtual void drawGlyph (int glyphNumber, const AffineTransform& transform) = 0;
};


#endif   // __JUCE_LOWLEVELGRAPHICSCONTEXT_JUCEHEADER__
