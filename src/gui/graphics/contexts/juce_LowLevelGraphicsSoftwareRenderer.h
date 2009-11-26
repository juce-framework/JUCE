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

#ifndef __JUCE_LOWLEVELGRAPHICSSOFTWARERENDERER_JUCEHEADER__
#define __JUCE_LOWLEVELGRAPHICSSOFTWARERENDERER_JUCEHEADER__

#include "juce_LowLevelGraphicsContext.h"
class LLGCSavedState;

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
    LowLevelGraphicsSoftwareRenderer (Image& imageToRenderOn);
    ~LowLevelGraphicsSoftwareRenderer();

    bool isVectorDevice() const;

    //==============================================================================
    void setOrigin (int x, int y);

    bool reduceClipRegion (int x, int y, int w, int h);
    bool reduceClipRegion (const RectangleList& clipRegion);
    void excludeClipRegion (int x, int y, int w, int h);

    void clipToPath (const Path& path, const AffineTransform& transform);
    void clipToImage (Image& image, int imageX, int imageY);

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
    void fillPath (const Path& path, const AffineTransform& transform);

    void fillPathWithImage (const Path& path, const AffineTransform& transform,
                            const Image& image, int imageX, int imageY);

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
    Image& image;

    LLGCSavedState* currentState;
    OwnedArray <LLGCSavedState> stateStack;

/*    void drawVertical (const int x, const double top, const double bottom);
    void drawHorizontal (const int y, const double top, const double bottom);

    void clippedFillRectWithColour (const Rectangle& clipRect, int x, int y, int w, int h, const Colour& colour, const bool replaceExistingContents);

    void clippedFillPath (int clipX, int clipY, int clipW, int clipH, const Path& path, const AffineTransform& transform);
    void clippedFillPathWithImage (int clipX, int clipY, int clipW, int clipH, const Path& path, const AffineTransform& transform,
                                   const Image& image, int imageX, int imageY, float alpha);

    void clippedFillAlphaChannel (int clipX, int clipY, int clipW, int clipH, const Image& alphaImage, int alphaImageX, int alphaImageY);
    void clippedFillAlphaChannelWithImage (int clipX, int clipY, int clipW, int clipH, const Image& alphaImage, int alphaImageX, int alphaImageY,
                                           const Image& fillerImage, int fillerImageX, int fillerImageY, const float opacity);

    //==============================================================================
    void clippedBlendImage (int clipX, int clipY, int clipW, int clipH, const Image& sourceImage,
                            int destX, int destY, int destW, int destH, int sourceX, int sourceY);

    void clippedBlendImageWarping (int clipX, int clipY, int clipW, int clipH, const Image& sourceImage,
                                   int srcClipX, int srcClipY, int srcClipW, int srcClipH,
                                   const AffineTransform& transform);

    //==============================================================================
    void clippedDrawLine (int clipX, int clipY, int clipW, int clipH, double x1, double y1, double x2, double y2);

    void clippedDrawVerticalLine (int clipX, int clipY, int clipW, int clipH, const int x, double top, double bottom);
    void clippedDrawHorizontalLine (int clipX, int clipY, int clipW, int clipH, const int x, double top, double bottom);*/

    LowLevelGraphicsSoftwareRenderer (const LowLevelGraphicsSoftwareRenderer& other);
    const LowLevelGraphicsSoftwareRenderer& operator= (const LowLevelGraphicsSoftwareRenderer&);
};



#endif   // __JUCE_LOWLEVELGRAPHICSSOFTWARERENDERER_JUCEHEADER__
