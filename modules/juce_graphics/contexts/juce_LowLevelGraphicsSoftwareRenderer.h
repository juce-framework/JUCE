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

#ifndef JUCE_LOWLEVELGRAPHICSSOFTWARERENDERER_H_INCLUDED
#define JUCE_LOWLEVELGRAPHICSSOFTWARERENDERER_H_INCLUDED

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
    LowLevelGraphicsSoftwareRenderer (const Image& imageToRenderOnto, Point<int> origin,
                                      const RectangleList& initialClip);
    ~LowLevelGraphicsSoftwareRenderer();

    bool isVectorDevice() const override;
    void setOrigin (int x, int y) override;
    void addTransform (const AffineTransform&) override;
    float getScaleFactor() override;
    bool clipToRectangle (const Rectangle<int>&) override;
    bool clipToRectangleList (const RectangleList&) override;
    void excludeClipRectangle (const Rectangle<int>&) override;
    void clipToPath (const Path&, const AffineTransform&) override;
    void clipToImageAlpha (const Image&, const AffineTransform&) override;
    bool clipRegionIntersects (const Rectangle<int>&) override;
    Rectangle<int> getClipBounds() const override;
    bool isClipEmpty() const override;

    void saveState() override;
    void restoreState() override;

    void beginTransparencyLayer (float opacity) override;
    void endTransparencyLayer() override;

    void setFill (const FillType&) override;
    void setOpacity (float opacity) override;
    void setInterpolationQuality (Graphics::ResamplingQuality) override;

    void fillRect (const Rectangle<int>&, bool replaceExistingContents) override;
    void fillPath (const Path&, const AffineTransform&) override;

    void drawImage (const Image&, const AffineTransform&) override;

    void drawLine (const Line <float>&) override;
    void drawVerticalLine (int x, float top, float bottom) override;
    void drawHorizontalLine (int x, float top, float bottom) override;

    void setFont (const Font&) override;
    const Font& getFont() override;
    void drawGlyph (int glyphNumber, const AffineTransform&) override;

    const Image& getImage() const noexcept                                          { return savedState->image; }
    const RenderingHelpers::TranslationOrTransform& getTransform() const noexcept   { return savedState->transform; }

protected:
    RenderingHelpers::SavedStateStack <RenderingHelpers::SoftwareRendererSavedState> savedState;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LowLevelGraphicsSoftwareRenderer)
};


#endif   // JUCE_LOWLEVELGRAPHICSSOFTWARERENDERER_H_INCLUDED
