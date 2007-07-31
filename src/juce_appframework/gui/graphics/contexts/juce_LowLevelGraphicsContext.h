/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCE_LOWLEVELGRAPHICSCONTEXT_JUCEHEADER__
#define __JUCE_LOWLEVELGRAPHICSCONTEXT_JUCEHEADER__

#include "../imaging/juce_Image.h"
#include "../geometry/juce_Path.h"
#include "../geometry/juce_RectangleList.h"
#include "../colour/juce_ColourGradient.h"
#include "juce_EdgeTable.h"


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

    /** Cliping co-ords are relative to the origin. */
    virtual void excludeClipRegion (int x, int y, int w, int h) = 0;

    virtual void saveState() = 0;
    virtual void restoreState() = 0;

    virtual bool clipRegionIntersects (int x, int y, int w, int h) = 0;
    virtual const Rectangle getClipBounds() const = 0;
    virtual bool isClipEmpty() const = 0;

    //==============================================================================
    virtual void fillRectWithColour (int x, int y, int w, int h, const Colour& colour, const bool replaceExistingContents) = 0;
    virtual void fillRectWithGradient (int x, int y, int w, int h, const ColourGradient& gradient) = 0;

    virtual void fillPathWithColour (const Path& path, const AffineTransform& transform, const Colour& colour, EdgeTable::OversamplingLevel quality) = 0;
    virtual void fillPathWithGradient (const Path& path, const AffineTransform& transform, const ColourGradient& gradient, EdgeTable::OversamplingLevel quality) = 0;
    virtual void fillPathWithImage (const Path& path, const AffineTransform& transform,
                                    const Image& image, int imageX, int imageY, float alpha, EdgeTable::OversamplingLevel quality) = 0;

    virtual void fillAlphaChannelWithColour (const Image& alphaImage, int alphaImageX, int alphaImageY, const Colour& colour) = 0;
    virtual void fillAlphaChannelWithGradient (const Image& alphaImage, int alphaImageX, int alphaImageY, const ColourGradient& gradient) = 0;
    virtual void fillAlphaChannelWithImage (const Image& alphaImage, int alphaImageX, int alphaImageY,
                                            const Image& fillerImage, int fillerImageX, int fillerImageY, float alpha) = 0;

    //==============================================================================
    virtual void blendImage (const Image& sourceImage,
                             int destX, int destY, int destW, int destH, int sourceX, int sourceY,
                             float alpha) = 0;

    virtual void blendImageRescaling (const Image& sourceImage,
                                      int destX, int destY, int destW, int destH,
                                      int sourceX, int sourceY, int sourceW, int sourceH,
                                      float alpha, const Graphics::ResamplingQuality quality) = 0;

    virtual void blendImageWarping (const Image& sourceImage,
                                    int srcClipX, int srcClipY, int srcClipW, int srcClipH,
                                    const AffineTransform& transform,
                                    float alpha, const Graphics::ResamplingQuality quality) = 0;

    //==============================================================================
    virtual void drawLine (double x1, double y1, double x2, double y2, const Colour& colour) = 0;

    virtual void drawVerticalLine (const int x, double top, double bottom, const Colour& col) = 0;
    virtual void drawHorizontalLine (const int y, double left, double right, const Colour& col) = 0;
};


#endif   // __JUCE_LOWLEVELGRAPHICSCONTEXT_JUCEHEADER__
