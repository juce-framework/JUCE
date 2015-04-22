/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_LOWLEVELGRAPHICSCONTEXT_H_INCLUDED
#define JUCE_LOWLEVELGRAPHICSCONTEXT_H_INCLUDED


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


#endif   // JUCE_LOWLEVELGRAPHICSCONTEXT_H_INCLUDED
