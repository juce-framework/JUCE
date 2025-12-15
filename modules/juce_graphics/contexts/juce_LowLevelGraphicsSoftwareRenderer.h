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

//==============================================================================
/**
    A lowest-common-denominator implementation of LowLevelGraphicsContext that does all
    its rendering in memory.

    User code is not supposed to create instances of this class directly - do all your
    rendering via the Graphics class instead.

    @tags{Graphics}
*/
class JUCE_API  LowLevelGraphicsSoftwareRenderer : public LowLevelGraphicsContext
{
public:
    //==============================================================================
    /** Creates a context to render into an image. */
    LowLevelGraphicsSoftwareRenderer (const Image& imageToRenderOnto);

    /** Creates a context to render into a clipped subsection of an image. */
    LowLevelGraphicsSoftwareRenderer (const Image& imageToRenderOnto, Point<int> origin,
                                      const RectangleList<int>& initialClip);

    /** Destructor. */
    ~LowLevelGraphicsSoftwareRenderer() override;

    std::unique_ptr<ImageType> getPreferredImageTypeForTemporaryImages() const override
    {
        return std::make_unique<SoftwareImageType>();
    }

    bool isVectorDevice() const override;
    Rectangle<int> getClipBounds() const override;
    bool isClipEmpty() const override;

    void setOrigin (Point<int> o) override;
    void addTransform (const AffineTransform& t) override;
    float getPhysicalPixelScaleFactor() const override;
    bool clipRegionIntersects (const Rectangle<int>& r) override;
    bool clipToRectangle (const Rectangle<int>& r) override;
    bool clipToRectangleList (const RectangleList<int>& r) override;
    void excludeClipRectangle (const Rectangle<int>& r) override;
    void clipToPath (const Path& path, const AffineTransform& t) override;
    void clipToImageAlpha (const Image& im, const AffineTransform& t) override;
    void saveState() override;
    void restoreState() override;
    void beginTransparencyLayer (float opacity) override;
    void endTransparencyLayer() override;
    void setFill (const FillType& fillType) override;
    void setOpacity (float newOpacity) override;
    void setInterpolationQuality (Graphics::ResamplingQuality quality) override;
    void fillRect (const Rectangle<int>& r, bool replace) override;
    void fillRect (const Rectangle<float>& r) override;
    void fillRectList (const RectangleList<float>& list) override;
    void fillPath (const Path& path, const AffineTransform& t) override;
    void drawImage (const Image& im, const AffineTransform& t) override;
    void drawLine (const Line<float>& line) override;
    void setFont (const Font& newFont) override;
    const Font& getFont() override;
    uint64_t getFrameId() const override;

    void drawGlyphs (Span<const uint16_t> glyphs,
                     Span<const Point<float>> positions,
                     const AffineTransform& t) override;

private:
    class Impl;
    std::unique_ptr<Impl> impl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LowLevelGraphicsSoftwareRenderer)
};

} // namespace juce
