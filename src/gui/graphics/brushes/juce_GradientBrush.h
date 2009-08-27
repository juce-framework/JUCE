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

#ifndef __JUCE_GRADIENTBRUSH_JUCEHEADER__
#define __JUCE_GRADIENTBRUSH_JUCEHEADER__

#include "juce_Brush.h"
#include "../colour/juce_ColourGradient.h"


//==============================================================================
/**
    A Brush that fills areas with a colour gradient.

    The gradient can either be linear or circular.

    @see Brush, Graphics::setBrush, SolidColourBrush, ImageBrush
*/
class JUCE_API  GradientBrush  : public Brush
{
public:
    //==============================================================================
    /** Creates a gradient brush, ready for use in Graphics::setBrush().

        (x1, y1) is the location relative to the origin of the Graphics context,
        at which the colour should be colour1. Likewise for (x2, y2) and colour2.

        If isRadial is true, the colours form a circular gradient with (x1, y1) at
        its centre.

        The alpha transparencies of the colours are used, so the brush
        need not be completely opaque. Note that this means that if you
        blend from transparent to a solid colour, the RGB of the transparent
        colour will become visible in parts of the gradient. e.g. blending
        from Colour::transparentBlack to Colours::white will produce a
        grey colour, but Colour::transparentWhite to Colours::white will be
        white all the way across.

        @see ColourGradient
    */
    GradientBrush (const Colour& colour1,
                   const float x1,
                   const float y1,
                   const Colour& colour2,
                   const float x2,
                   const float y2,
                   const bool isRadial) throw();

    /** Creates a gradient brush from a ColourGradient object.
    */
    GradientBrush (const ColourGradient& gradient) throw();

    /** Destructor. */
    ~GradientBrush() throw();

    //==============================================================================
    /** Returns the current gradient information */
    const ColourGradient& getGradient() const throw()       { return gradient; }

    //==============================================================================
    Brush* createCopy() const throw();

    void applyTransform (const AffineTransform& transform) throw();

    void multiplyOpacity (const float multiple) throw();

    bool isInvisible() const throw();

    void paintPath (LowLevelGraphicsContext& context,
                    const Path& path, const AffineTransform& transform) throw();

    void paintRectangle (LowLevelGraphicsContext& context,
                         int x, int y, int w, int h) throw();

    void paintAlphaChannel (LowLevelGraphicsContext& context,
                            const Image& alphaChannelImage, int imageX, int imageY,
                            int x, int y, int w, int h) throw();

    //==============================================================================
    juce_UseDebuggingNewOperator


protected:
    ColourGradient gradient;

private:
    GradientBrush (const GradientBrush&);
    const GradientBrush& operator= (const GradientBrush&);
};

#endif   // __JUCE_GRADIENTBRUSH_JUCEHEADER__
