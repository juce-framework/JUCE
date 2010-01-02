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

#ifndef __JUCE_COLOURGRADIENT_JUCEHEADER__
#define __JUCE_COLOURGRADIENT_JUCEHEADER__

#include "juce_Colour.h"
#include "../geometry/juce_AffineTransform.h"
#include "../../../containers/juce_Array.h"
#include "../../../containers/juce_HeapBlock.h"


//==============================================================================
/**
    Describes the layout and colours that should be used to paint a colour gradient.

    @see Graphics::setGradientFill
*/
class JUCE_API  ColourGradient
{
public:
    //==============================================================================
    /** Creates a gradient object.

        (x1, y1) is the location to draw with colour1. Likewise (x2, y2) is where
        colour2 should be. In between them there's a gradient.

        If isRadial is true, the colours form a circular gradient with (x1, y1) at
        its centre.

        The alpha transparencies of the colours are used, so note that
        if you blend from transparent to a solid colour, the RGB of the transparent
        colour will become visible in parts of the gradient. e.g. blending
        from Colour::transparentBlack to Colours::white will produce a
        muddy grey colour midway, but Colour::transparentWhite to Colours::white
        will be white all the way across.

        @see ColourGradient
    */
    ColourGradient (const Colour& colour1,
                    const float x1,
                    const float y1,
                    const Colour& colour2,
                    const float x2,
                    const float y2,
                    const bool isRadial) throw();

    /** Creates an uninitialised gradient.

        If you use this constructor instead of the other one, be sure to set all the
        object's public member variables before using it!
    */
    ColourGradient() throw();

    /** Destructor */
    ~ColourGradient() throw();

    //==============================================================================
    /** Removes any colours that have been added.

        This will also remove any start and end colours, so the gradient won't work. You'll
        need to add more colours with addColour().
    */
    void clearColours() throw();

    /** Adds a colour at a point along the length of the gradient.

        This allows the gradient to go through a spectrum of colours, instead of just a
        start and end colour.

        @param proportionAlongGradient      a value between 0 and 1.0, which is the proportion
                                            of the distance along the line between the two points
                                            at which the colour should occur.
        @param colour                       the colour that should be used at this point
    */
    void addColour (const double proportionAlongGradient,
                    const Colour& colour) throw();

    /** Multiplies the alpha value of all the colours by the given scale factor */
    void multiplyOpacity (const float multiplier) throw();

    //==============================================================================
    /** Returns the number of colour-stops that have been added. */
    int getNumColours() const throw();

    /** Returns the position along the length of the gradient of the colour with this index.

        The index is from 0 to getNumColours() - 1. The return value will be between 0.0 and 1.0
    */
    double getColourPosition (const int index) const throw();

    /** Returns the colour that was added with a given index.

        The index is from 0 to getNumColours() - 1. The return value will be between 0.0 and 1.0
    */
    const Colour getColour (const int index) const throw();

    /** Returns the an interpolated colour at any position along the gradient.
        @param position     the position along the gradient, between 0 and 1
    */
    const Colour getColourAtPosition (const float position) const throw();

    //==============================================================================
    /** Creates a set of interpolated premultiplied ARGB values.
        This will resize the HeapBlock, fill it with the colours, and will return the number of
        colours that it added.
    */
    int createLookupTable (const AffineTransform& transform, HeapBlock <PixelARGB>& resultLookupTable) const throw();

    /** Returns true if all colours are opaque. */
    bool isOpaque() const throw();

    /** Returns true if all colours are completely transparent. */
    bool isInvisible() const throw();

    //==============================================================================
    float x1;
    float y1;

    float x2;
    float y2;

    /** If true, the gradient should be filled circularly, centred around
        (x1, y1), with (x2, y2) defining a point on the circumference.

        If false, the gradient is linear between the two points.
    */
    bool isRadial;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    Array <uint32> colours;
};


#endif   // __JUCE_COLOURGRADIENT_JUCEHEADER__
