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

#ifndef JUCE_MAC_COREGRAPHICSCONTEXT_H_INCLUDED
#define JUCE_MAC_COREGRAPHICSCONTEXT_H_INCLUDED

//==============================================================================
class CoreGraphicsContext   : public LowLevelGraphicsContext
{
public:
    CoreGraphicsContext (CGContextRef context, const float flipHeight, const float targetScale);
    ~CoreGraphicsContext();

    //==============================================================================
    bool isVectorDevice() const override         { return false; }

    void setOrigin (Point<int>) override;
    void addTransform (const AffineTransform&) override;
    float getPhysicalPixelScaleFactor() override;
    bool clipToRectangle (const Rectangle<int>&) override;
    bool clipToRectangleList (const RectangleList<int>&) override;
    void excludeClipRectangle (const Rectangle<int>&) override;
    void clipToPath (const Path&, const AffineTransform&) override;
    void clipToImageAlpha (const Image&, const AffineTransform&) override;
    bool clipRegionIntersects (const Rectangle<int>&) override;
    Rectangle<int> getClipBounds() const override;
    bool isClipEmpty() const override;

    //==============================================================================
    void saveState() override;
    void restoreState() override;
    void beginTransparencyLayer (float opacity) override;
    void endTransparencyLayer() override;

    //==============================================================================
    void setFill (const FillType&) override;
    void setOpacity (float) override;
    void setInterpolationQuality (Graphics::ResamplingQuality) override;

    //==============================================================================
    void fillRect (const Rectangle<int>&, bool replaceExistingContents) override;
    void fillRect (const Rectangle<float>&) override;
    void fillRectList (const RectangleList<float>&) override;
    void fillPath (const Path&, const AffineTransform&) override;
    void drawImage (const Image& sourceImage, const AffineTransform&) override;

    //==============================================================================
    void drawLine (const Line<float>&) override;
    void setFont (const Font&) override;
    const Font& getFont() override;
    void drawGlyph (int glyphNumber, const AffineTransform&) override;
    bool drawTextLayout (const AttributedString&, const Rectangle<float>&) override;

private:
    CGContextRef context;
    const CGFloat flipHeight;
    float targetScale;
    CGColorSpaceRef rgbColourSpace, greyColourSpace;
    mutable Rectangle<int> lastClipRect;
    mutable bool lastClipRectIsValid;

    struct SavedState
    {
        SavedState();
        SavedState (const SavedState&);
        ~SavedState();

        void setFill (const FillType&);

        FillType fillType;
        Font font;
        CGFontRef fontRef;
        CGAffineTransform fontTransform;
        CGGradientRef gradient;
    };

    ScopedPointer<SavedState> state;
    OwnedArray<SavedState> stateStack;

    void drawGradient();
    void createPath (const Path&) const;
    void createPath (const Path&, const AffineTransform&) const;
    void flip() const;
    void applyTransform (const AffineTransform&) const;
    void drawImage (const Image&, const AffineTransform&, bool fillEntireClipAsTiles);
    bool clipToRectangleListWithoutTest (const RectangleList<int>&);
    void fillCGRect (const CGRect&, bool replaceExistingContents);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoreGraphicsContext)
};

#endif   // JUCE_MAC_COREGRAPHICSCONTEXT_H_INCLUDED
