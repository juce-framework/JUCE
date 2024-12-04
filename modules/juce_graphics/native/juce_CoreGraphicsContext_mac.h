/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

namespace detail
{
    using ColorSpacePtr     = CFUniquePtr<CGColorSpaceRef>;
    using ContextPtr        = CFUniquePtr<CGContextRef>;
    using DataProviderPtr   = CFUniquePtr<CGDataProviderRef>;
    using ImagePtr          = CFUniquePtr<CGImageRef>;
    using GradientPtr       = CFUniquePtr<CGGradientRef>;
    using ColorPtr          = CFUniquePtr<CGColorRef>;
    using PathPtr           = CFUniquePtr<CGPathRef>;
    using MutablePathPtr    = CFUniquePtr<CGMutablePathRef>;
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
    float getPhysicalPixelScaleFactor() const override;
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
    void fillAll() override;
    void fillRect (const Rectangle<int>&, bool replaceExistingContents) override;
    void fillRect (const Rectangle<float>&) override;
    void fillRectList (const RectangleList<float>&) override;
    void fillPath (const Path&, const AffineTransform&) override;
    void strokePath (const Path& path, const PathStrokeType& strokeType, const AffineTransform& transform) override;
    void drawImage (const Image& sourceImage, const AffineTransform&) override;

    //==============================================================================
    void drawLine (const Line<float>&) override;
    void setFont (const Font&) override;
    const Font& getFont() override;
    void drawGlyphs (Span<const uint16_t>,
                     Span<const Point<float>>,
                     const AffineTransform&) override;

    uint64_t getFrameId() const override { return 0; }

    void drawEllipse (const Rectangle<float>& area, float lineThickness) override;
    void fillEllipse (const Rectangle<float>& area) override;

    void drawRoundedRectangle (const Rectangle<float>& r, float cornerSize, float lineThickness) override;
    void fillRoundedRectangle (const Rectangle<float>& r, float cornerSize) override;

    void drawLineWithThickness (const Line<float>& line, float lineThickness) override;

private:
    //==============================================================================
    detail::ContextPtr context;
    const CGFloat flipHeight;
    detail::ColorSpacePtr rgbColourSpace, greyColourSpace;
    mutable std::optional<Rectangle<int>> lastClipRect;

    struct SavedState;
    std::unique_ptr<SavedState> state;
    OwnedArray<SavedState> stateStack;

    template <class RectType>
    CGRect convertToCGRectFlipped (RectType r) const noexcept;
    void setContextClipToCurrentPath (bool useNonZeroWinding);
    void drawCurrentPath (CGPathDrawingMode mode);
    void drawGradient();
    void createPath (const Path&, const AffineTransform&) const;
    void flip() const;
    void applyTransform (const AffineTransform&) const;
    void drawImage (const Image&, const AffineTransform&, bool fillEntireClipAsTiles);
    bool clipToRectangleListWithoutTest (const RectangleList<float>&);
    void fillCGRect (const CGRect&, bool replaceExistingContents);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoreGraphicsContext)
};

} // namespace juce
