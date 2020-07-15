/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

namespace detail
{
    struct ColorSpaceDelete
    {
        void operator() (CGColorSpaceRef ptr) const noexcept { CGColorSpaceRelease (ptr); }
    };

    struct ContextDelete
    {
        void operator() (CGContextRef ptr) const noexcept { CGContextRelease (ptr); }
    };

    struct DataProviderDelete
    {
        void operator() (CGDataProviderRef ptr) const noexcept { CGDataProviderRelease (ptr); }
    };

    struct ImageDelete
    {
        void operator() (CGImageRef ptr) const noexcept { CGImageRelease (ptr); }
    };

    struct GradientDelete
    {
        void operator() (CGGradientRef ptr) const noexcept { CGGradientRelease (ptr); }
    };

    //==============================================================================
    using ColorSpacePtr = std::unique_ptr<CGColorSpace, ColorSpaceDelete>;
    using ContextPtr = std::unique_ptr<CGContext, ContextDelete>;
    using DataProviderPtr = std::unique_ptr<CGDataProvider, DataProviderDelete>;
    using ImagePtr = std::unique_ptr<CGImage, ImageDelete>;
    using GradientPtr = std::unique_ptr<CGGradient, GradientDelete>;
}

//==============================================================================
class CoreGraphicsContext   : public LowLevelGraphicsContext
{
public:
    CoreGraphicsContext (CGContextRef context, float flipHeight);
    ~CoreGraphicsContext() override;

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
    //==============================================================================
    detail::ContextPtr context;
    const CGFloat flipHeight;
    detail::ColorSpacePtr rgbColourSpace, greyColourSpace;
    mutable Rectangle<int> lastClipRect;
    mutable bool lastClipRectIsValid = false;

    struct SavedState
    {
        SavedState();
        SavedState (const SavedState&);
        ~SavedState();

        void setFill (const FillType&);

        FillType fillType;
        Font font;
        CGFontRef fontRef = {};
        CGAffineTransform textMatrix = CGAffineTransformIdentity,
                   inverseTextMatrix = CGAffineTransformIdentity;
        detail::GradientPtr gradient = {};
    };

    std::unique_ptr<SavedState> state;
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

} // namespace juce
