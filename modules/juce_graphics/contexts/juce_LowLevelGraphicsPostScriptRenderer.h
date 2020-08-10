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

//==============================================================================
/**
    An implementation of LowLevelGraphicsContext that turns the drawing operations
    into a PostScript document.

    @tags{Graphics}
*/
class JUCE_API  LowLevelGraphicsPostScriptRenderer    : public LowLevelGraphicsContext
{
public:
    //==============================================================================
    LowLevelGraphicsPostScriptRenderer (OutputStream& resultingPostScript,
                                        const String& documentTitle,
                                        int totalWidth,
                                        int totalHeight);

    ~LowLevelGraphicsPostScriptRenderer() override;

    //==============================================================================
    bool isVectorDevice() const override;
    void setOrigin (Point<int>) override;
    void addTransform (const AffineTransform&) override;
    float getPhysicalPixelScaleFactor() override;

    bool clipToRectangle (const Rectangle<int>&) override;
    bool clipToRectangleList (const RectangleList<int>&) override;
    void excludeClipRectangle (const Rectangle<int>&) override;
    void clipToPath (const Path&, const AffineTransform&) override;
    void clipToImageAlpha (const Image&, const AffineTransform&) override;

    void saveState() override;
    void restoreState() override;

    void beginTransparencyLayer (float) override;
    void endTransparencyLayer() override;

    bool clipRegionIntersects (const Rectangle<int>&) override;
    Rectangle<int> getClipBounds() const override;
    bool isClipEmpty() const override;

    //==============================================================================
    void setFill (const FillType&) override;
    void setOpacity (float) override;
    void setInterpolationQuality (Graphics::ResamplingQuality) override;

    //==============================================================================
    void fillRect (const Rectangle<int>&, bool replaceExistingContents) override;
    void fillRect (const Rectangle<float>&) override;
    void fillRectList (const RectangleList<float>&) override;
    void fillPath (const Path&, const AffineTransform&) override;
    void drawImage (const Image&, const AffineTransform&) override;
    void drawLine (const Line <float>&) override;

    //==============================================================================
    const Font& getFont() override;
    void setFont (const Font&) override;
    void drawGlyph (int glyphNumber, const AffineTransform&) override;

protected:
    //==============================================================================
    OutputStream& out;
    int totalWidth, totalHeight;
    bool needToClip;
    Colour lastColour;

    /** Describes a saved state */
    struct SavedState
    {
        SavedState();
        SavedState& operator= (const SavedState&) = delete;
        ~SavedState();

        RectangleList<int> clip;
        int xOffset, yOffset;
        FillType fillType;
        Font font;
    };

    OwnedArray<SavedState> stateStack;

    void writeClip();
    void writeColour (Colour colour);
    void writePath (const Path&) const;
    void writeXY (float x, float y) const;
    void writeTransform (const AffineTransform&) const;
    void writeImage (const Image&, int sx, int sy, int maxW, int maxH) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LowLevelGraphicsPostScriptRenderer)
};

} // namespace juce
