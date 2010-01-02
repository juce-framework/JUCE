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

#ifndef __JUCE_LOWLEVELGRAPHICSSOFTWARERENDERER_JUCEHEADER__
#define __JUCE_LOWLEVELGRAPHICSSOFTWARERENDERER_JUCEHEADER__

#include "juce_LowLevelGraphicsContext.h"
class LLGCSavedState;

//==============================================================================
/**
    A lowest-common-denominator implementation of LowLevelGraphicsContext that does all
    its rendering in memory.

    User code is not supposed to create instances of this class directly - do all your
    rendering via the Graphics class instead.
*/
class JUCE_API  LowLevelGraphicsSoftwareRenderer    : public LowLevelGraphicsContext
{
public:
    //==============================================================================
    LowLevelGraphicsSoftwareRenderer (Image& imageToRenderOn);
    ~LowLevelGraphicsSoftwareRenderer();

    bool isVectorDevice() const;

    //==============================================================================
    void setOrigin (int x, int y);

    bool clipToRectangle (const Rectangle& r);
    bool clipToRectangleList (const RectangleList& clipRegion);
    void excludeClipRectangle (const Rectangle& r);
    void clipToPath (const Path& path, const AffineTransform& transform);
    void clipToImageAlpha (const Image& sourceImage, const Rectangle& srcClip, const AffineTransform& transform);

    bool clipRegionIntersects (const Rectangle& r);
    const Rectangle getClipBounds() const;
    bool isClipEmpty() const;

    void saveState();
    void restoreState();

    //==============================================================================
    void setFill (const FillType& fillType);
    void setOpacity (float opacity);
    void setInterpolationQuality (Graphics::ResamplingQuality quality);

    //==============================================================================
    void fillRect (const Rectangle& r, const bool replaceExistingContents);
    void fillPath (const Path& path, const AffineTransform& transform);

    void drawImage (const Image& sourceImage, const Rectangle& srcClip,
                    const AffineTransform& transform, const bool fillEntireClipAsTiles);

    void drawLine (double x1, double y1, double x2, double y2);

    void drawVerticalLine (const int x, double top, double bottom);
    void drawHorizontalLine (const int x, double top, double bottom);

    //==============================================================================
    void setFont (const Font& newFont);
    const Font getFont();
    void drawGlyph (int glyphNumber, float x, float y);
    void drawGlyph (int glyphNumber, const AffineTransform& transform);

    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    //==============================================================================
    Image& image;

    ScopedPointer <LLGCSavedState> currentState;
    OwnedArray <LLGCSavedState> stateStack;

    LowLevelGraphicsSoftwareRenderer (const LowLevelGraphicsSoftwareRenderer& other);
    const LowLevelGraphicsSoftwareRenderer& operator= (const LowLevelGraphicsSoftwareRenderer&);
};



#endif   // __JUCE_LOWLEVELGRAPHICSSOFTWARERENDERER_JUCEHEADER__
