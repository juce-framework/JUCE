/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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
                                        const int totalWidth,
                                        const int totalHeight);

    ~LowLevelGraphicsPostScriptRenderer();

    //==============================================================================
    bool isVectorDevice() const;
    void setOrigin (int x, int y);

    bool reduceClipRegion (int x, int y, int w, int h);
    bool reduceClipRegion (const RectangleList& clipRegion);
    void excludeClipRegion (int x, int y, int w, int h);

    void saveState();
    void restoreState();

    bool clipRegionIntersects (int x, int y, int w, int h);
    const Rectangle getClipBounds() const;
    bool isClipEmpty() const;

    //==============================================================================
    void setColour (const Colour& colour);
    void setGradient (const ColourGradient& gradient);
    void setOpacity (float opacity);
    void setInterpolationQuality (Graphics::ResamplingQuality quality);

    //==============================================================================
    void fillRect (int x, int y, int w, int h, const bool replaceExistingContents);
    void fillPath (const Path& path, const AffineTransform& transform, EdgeTable::OversamplingLevel quality);

    void fillPathWithImage (const Path& path, const AffineTransform& transform,
                            const Image& image, int imageX, int imageY, EdgeTable::OversamplingLevel quality);

    void fillAlphaChannel (const Image& alphaImage, int imageX, int imageY);
    void fillAlphaChannelWithImage (const Image& alphaImage, int alphaImageX, int alphaImageY,
                                    const Image& fillerImage, int fillerImageX, int fillerImageY);

    //==============================================================================
    void blendImage (const Image& sourceImage, int destX, int destY, int destW, int destH,
                     int sourceX, int sourceY);

    void blendImageWarping (const Image& sourceImage, int srcClipX, int srcClipY, int srcClipW, int srcClipH,
                            const AffineTransform& transform);

    //==============================================================================
    void drawLine (double x1, double y1, double x2, double y2);

    void drawVerticalLine (const int x, double top, double bottom);
    void drawHorizontalLine (const int x, double top, double bottom);

    //==============================================================================
    void setFont (const Font& newFont);
    void drawGlyph (int glyphNumber, float x, float y);
    void drawGlyph (int glyphNumber, const AffineTransform& transform);

    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    //==============================================================================
    OutputStream& out;
    RectangleList* clip;
    int totalWidth, totalHeight, xOffset, yOffset;
    bool needToClip;
    Colour lastColour, colour;
    ColourGradient* gradient;
    Font font;

    struct SavedState
    {
        SavedState (RectangleList* const clip, const int xOffset, const int yOffset,
                    const Colour& colour, ColourGradient* const gradient, const Font& font);
        ~SavedState();

        RectangleList* clip;
        const int xOffset, yOffset;
        Colour colour;
        ColourGradient* gradient;
        Font font;

    private:
        SavedState (const SavedState&);
        const SavedState& operator= (const SavedState&);
    };

    OwnedArray <SavedState> stateStack;

    void writeClip();
    void writeColour (const Colour& colour);
    void writePath (const Path& path) const;
    void writeXY (const float x, const float y) const;
    void writeTransform (const AffineTransform& trans) const;
    void writeImage (const Image& im, const int sx, const int sy, const int maxW, const int maxH) const;

    LowLevelGraphicsPostScriptRenderer (const LowLevelGraphicsPostScriptRenderer& other);
    const LowLevelGraphicsPostScriptRenderer& operator= (const LowLevelGraphicsPostScriptRenderer&);
};



#endif   // __JUCE_LOWLEVELGRAPHICSPOSTSCRIPTRENDERER_JUCEHEADER__
