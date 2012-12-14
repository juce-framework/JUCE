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

#ifndef __JUCE_LOWLEVELGRAPHICSPOSTSCRIPTRENDERER_JUCEHEADER__
#define __JUCE_LOWLEVELGRAPHICSPOSTSCRIPTRENDERER_JUCEHEADER__

#include "juce_LowLevelGraphicsContext.h"


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
    bool isVectorDevice() const;
    void setOrigin (int x, int y);
    void addTransform (const AffineTransform& transform);
    float getScaleFactor();

    bool clipToRectangle (const Rectangle<int>& r);
    bool clipToRectangleList (const RectangleList& clipRegion);
    void excludeClipRectangle (const Rectangle<int>& r);
    void clipToPath (const Path& path, const AffineTransform& transform);
    void clipToImageAlpha (const Image& sourceImage, const AffineTransform& transform);

    void saveState();
    void restoreState();

    void beginTransparencyLayer (float opacity);
    void endTransparencyLayer();

    bool clipRegionIntersects (const Rectangle<int>& r);
    Rectangle<int> getClipBounds() const;
    bool isClipEmpty() const;

    //==============================================================================
    void setFill (const FillType& fillType);
    void setOpacity (float opacity);
    void setInterpolationQuality (Graphics::ResamplingQuality quality);

    //==============================================================================
    void fillRect (const Rectangle<int>& r, bool replaceExistingContents);
    void fillPath (const Path& path, const AffineTransform& transform);

    void drawImage (const Image& sourceImage, const AffineTransform& transform);

    void drawLine (const Line <float>& line);

    void drawVerticalLine (int x, float top, float bottom);
    void drawHorizontalLine (int x, float top, float bottom);

    //==============================================================================
    const Font& getFont();
    void setFont (const Font& newFont);
    void drawGlyph (int glyphNumber, const AffineTransform& transform);

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

        RectangleList clip;
        int xOffset, yOffset;
        FillType fillType;
        Font font;

    private:
        SavedState& operator= (const SavedState&);
    };

    OwnedArray <SavedState> stateStack;

    void writeClip();
    void writeColour (const Colour& colour);
    void writePath (const Path& path) const;
    void writeXY (float x, float y) const;
    void writeTransform (const AffineTransform& trans) const;
    void writeImage (const Image& im, int sx, int sy, int maxW, int maxH) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LowLevelGraphicsPostScriptRenderer)
};



#endif   // __JUCE_LOWLEVELGRAPHICSPOSTSCRIPTRENDERER_JUCEHEADER__
