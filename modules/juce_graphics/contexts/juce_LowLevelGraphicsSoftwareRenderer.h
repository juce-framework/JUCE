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

#ifndef __JUCE_LOWLEVELGRAPHICSSOFTWARERENDERER_JUCEHEADER__
#define __JUCE_LOWLEVELGRAPHICSSOFTWARERENDERER_JUCEHEADER__

#include "juce_LowLevelGraphicsContext.h"


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
    LowLevelGraphicsSoftwareRenderer (const Image& imageToRenderOn);
    LowLevelGraphicsSoftwareRenderer (const Image& imageToRenderOn, int xOffset, int yOffset, const RectangleList& initialClip);
    ~LowLevelGraphicsSoftwareRenderer();

    bool isVectorDevice() const;

    //==============================================================================
    void setOrigin (int x, int y);
    void addTransform (const AffineTransform& transform);
    float getScaleFactor();

    bool clipToRectangle (const Rectangle<int>& r);
    bool clipToRectangleList (const RectangleList& clipRegion);
    void excludeClipRectangle (const Rectangle<int>& r);
    void clipToPath (const Path& path, const AffineTransform& transform);
    void clipToImageAlpha (const Image& sourceImage, const AffineTransform& transform);

    bool clipRegionIntersects (const Rectangle<int>& r);
    const Rectangle<int> getClipBounds() const;
    bool isClipEmpty() const;

    void saveState();
    void restoreState();

    void beginTransparencyLayer (float opacity);
    void endTransparencyLayer();

    //==============================================================================
    void setFill (const FillType& fillType);
    void setOpacity (float opacity);
    void setInterpolationQuality (Graphics::ResamplingQuality quality);

    //==============================================================================
    void fillRect (const Rectangle<int>& r, bool replaceExistingContents);
    void fillPath (const Path& path, const AffineTransform& transform);

    void drawImage (const Image& sourceImage, const AffineTransform& transform, bool fillEntireClipAsTiles);

    void drawLine (const Line <float>& line);

    void drawVerticalLine (int x, float top, float bottom);
    void drawHorizontalLine (int x, float top, float bottom);

    //==============================================================================
    void setFont (const Font& newFont);
    Font getFont();
    void drawGlyph (int glyphNumber, float x, float y);
    void drawGlyph (int glyphNumber, const AffineTransform& transform);


protected:
    //==============================================================================
    Image image;

    class GlyphCache;
    class CachedGlyph;
    class SavedState;
    friend class ScopedPointer <SavedState>;
    friend class OwnedArray <SavedState>;
    friend class OwnedArray <CachedGlyph>;
    ScopedPointer <SavedState> currentState;
    OwnedArray <SavedState> stateStack;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LowLevelGraphicsSoftwareRenderer);
};



#endif   // __JUCE_LOWLEVELGRAPHICSSOFTWARERENDERER_JUCEHEADER__
