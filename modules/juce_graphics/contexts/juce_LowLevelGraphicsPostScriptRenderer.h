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

#ifndef JUCE_LOWLEVELGRAPHICSPOSTSCRIPTRENDERER_H_INCLUDED
#define JUCE_LOWLEVELGRAPHICSPOSTSCRIPTRENDERER_H_INCLUDED


//==============================================================================
/**
    An implementation of LowLevelGraphicsContext that turns the drawing operations
    into a PostScript document.

*/
class JUCE_API  LowLevelGraphicsPostScriptRenderer    : public LowLevelGraphicsContext
{
public:
    //==============================================================================
    LowLevelGraphicsPostScriptRenderer (OutputStream& resultingPostScript,
                                        const String& documentTitle,
                                        int totalWidth,
                                        int totalHeight);

    ~LowLevelGraphicsPostScriptRenderer();

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

    struct SavedState
    {
        SavedState();
        ~SavedState();

        RectangleList<int> clip;
        int xOffset, yOffset;
        FillType fillType;
        Font font;

    private:
        SavedState& operator= (const SavedState&);
    };

    OwnedArray <SavedState> stateStack;

    void writeClip();
    void writeColour (Colour colour);
    void writePath (const Path&) const;
    void writeXY (float x, float y) const;
    void writeTransform (const AffineTransform&) const;
    void writeImage (const Image&, int sx, int sy, int maxW, int maxH) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LowLevelGraphicsPostScriptRenderer)
};



#endif   // JUCE_LOWLEVELGRAPHICSPOSTSCRIPTRENDERER_H_INCLUDED
