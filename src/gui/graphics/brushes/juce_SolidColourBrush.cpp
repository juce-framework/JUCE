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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_SolidColourBrush.h"
#include "../contexts/juce_LowLevelGraphicsContext.h"


//==============================================================================
SolidColourBrush::SolidColourBrush() throw()
    : colour (0xff000000)
{
}

SolidColourBrush::SolidColourBrush (const Colour& colour_) throw()
    : colour (colour_)
{
}

SolidColourBrush::~SolidColourBrush() throw()
{
}

Brush* SolidColourBrush::createCopy() const throw()
{
    return new SolidColourBrush (colour);
}

void SolidColourBrush::applyTransform (const AffineTransform& /*transform*/) throw()
{
}

void SolidColourBrush::multiplyOpacity (const float multiple) throw()
{
    colour = colour.withMultipliedAlpha (multiple);
}

bool SolidColourBrush::isInvisible() const throw()
{
    return colour.isTransparent();
}

void SolidColourBrush::paintPath (LowLevelGraphicsContext& context,
                                  const Path& path, const AffineTransform& transform) throw()
{
    if (! colour.isTransparent())
        context.fillPathWithColour (path, transform, colour, EdgeTable::Oversampling_4times);
}

void SolidColourBrush::paintRectangle (LowLevelGraphicsContext& context,
                                       int x, int y, int w, int h) throw()
{
    if (! colour.isTransparent())
        context.fillRectWithColour (x, y, w, h, colour, false);
}

void SolidColourBrush::paintAlphaChannel (LowLevelGraphicsContext& context,
                                          const Image& alphaChannelImage, int imageX, int imageY,
                                          int x, int y, int w, int h) throw()
{
    if (! colour.isTransparent())
    {
        context.saveState();

        if (context.reduceClipRegion (x, y, w, h))
            context.fillAlphaChannelWithColour (alphaChannelImage, imageX, imageY, colour);

        context.restoreState();
    }
}

void SolidColourBrush::paintVerticalLine (LowLevelGraphicsContext& context,
                                          int x, float y1, float y2) throw()
{
    context.drawVerticalLine (x, y1, y2, colour);
}

void SolidColourBrush::paintHorizontalLine (LowLevelGraphicsContext& context,
                                            int y, float x1, float x2) throw()
{
    context.drawHorizontalLine (y, x1, x2, colour);
}

void SolidColourBrush::paintLine (LowLevelGraphicsContext& context,
                                  float x1, float y1, float x2, float y2) throw()
{
    context.drawLine (x1, y1, x2, y2, colour);
}


END_JUCE_NAMESPACE
