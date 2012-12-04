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
#include "../native/juce_RenderingHelpers.h"

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
    LowLevelGraphicsSoftwareRenderer (const Image& imageToRenderOnto);
    LowLevelGraphicsSoftwareRenderer (const Image& imageToRenderOnto, const Point<int>& origin,
                                      const RectangleList& initialClip);
    ~LowLevelGraphicsSoftwareRenderer();

    bool isVectorDevice() const;
    void setOrigin (int x, int y);
    void addTransform (const AffineTransform&);
    float getScaleFactor();
    bool clipToRectangle (const Rectangle<int>&);
    bool clipToRectangleList (const RectangleList&);
    void excludeClipRectangle (const Rectangle<int>&);
    void clipToPath (const Path&, const AffineTransform&);
    void clipToImageAlpha (const Image&, const AffineTransform&);
    bool clipRegionIntersects (const Rectangle<int>&);
    Rectangle<int> getClipBounds() const;
    bool isClipEmpty() const;

    void saveState();
    void restoreState();

    void beginTransparencyLayer (float opacity);
    void endTransparencyLayer();

    void setFill (const FillType&);
    void setOpacity (float opacity);
    void setInterpolationQuality (Graphics::ResamplingQuality);

    void fillRect (const Rectangle<int>&, bool replaceExistingContents);
    void fillPath (const Path&, const AffineTransform&);

    void drawImage (const Image&, const AffineTransform&);

    void drawLine (const Line <float>&);
    void drawVerticalLine (int x, float top, float bottom);
    void drawHorizontalLine (int x, float top, float bottom);

    void setFont (const Font&);
    const Font& getFont();
    void drawGlyph (int glyphNumber, float x, float y);
    void drawGlyph (int glyphNumber, const AffineTransform&);

    const Image& getImage() const noexcept                                          { return savedState->image; }
    const RenderingHelpers::TranslationOrTransform& getTransform() const noexcept   { return savedState->transform; }

protected:
    RenderingHelpers::SavedStateStack <RenderingHelpers::SoftwareRendererSavedState> savedState;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LowLevelGraphicsSoftwareRenderer)
};


#endif   // __JUCE_LOWLEVELGRAPHICSSOFTWARERENDERER_JUCEHEADER__
