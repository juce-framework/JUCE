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

#ifndef __JUCE_FILLTYPE_JUCEHEADER__
#define __JUCE_FILLTYPE_JUCEHEADER__

#include "../colour/juce_ColourGradient.h"
#include "../../../containers/juce_ScopedPointer.h"
class Image;


//==============================================================================
/**
    Represents a colour or fill pattern to use for rendering paths.

    This is used by the Graphics and DrawablePath classes as a way to encapsulate
    a brush type. It can either be a solid colour, a gradient, or a tiled image.

    @see Graphics::setFillType, DrawablePath::setFill
*/
class JUCE_API  FillType
{
public:
    /** Creates a default fill type, of solid black. */
    FillType() throw();

    /** Creates a fill type of a solid colour.
        @see setColour
    */
    FillType (const Colour& colour) throw();

    /** Creates a gradient fill type.
        @see setGradient
    */
    FillType (const ColourGradient& gradient) throw();

    /** Creates a tiled image fill type. The transform allows you to set the scaling, offset
        and rotation of the pattern.
        @see setTiledImage
    */
    FillType (const Image& image, const AffineTransform& transform) throw();

    /** Creates a copy of another FillType. */
    FillType (const FillType& other) throw();

    /** Makes a copy of another FillType. */
    const FillType& operator= (const FillType& other) throw();

    /** Destructor. */
    ~FillType() throw();

    /** Returns true if this is a solid colour fill, and not a gradient or image. */
    bool isColour() const throw()           { return gradient == 0 && image == 0; }

    /** Returns true if this is a gradient fill. */
    bool isGradient() const throw()         { return gradient != 0; }

    /** Returns true if this is a tiled image pattern fill. */
    bool isTiledImage() const throw()       { return image != 0; }

    /** Turns this object into a solid colour fill.
        If the object was an image or gradient, those fields will no longer be valid. */
    void setColour (const Colour& newColour) throw();

    /** Turns this object into a gradient fill. */
    void setGradient (const ColourGradient& newGradient) throw();

    /** Turns this object into a tiled image fill type. The transform allows you to set
        the scaling, offset and rotation of the pattern.
    */
    void setTiledImage (const Image& image, const AffineTransform& transform) throw();

    /** Changes the opacity that should be used.
        If the fill is a solid colour, this just changes the opacity of that colour. For
        gradients and image tiles, it changes the opacity that will be used for them.
    */
    void setOpacity (const float newOpacity) throw();

    /** Returns the current opacity to be applied to the colour, gradient, or image.
        @see setOpacity
    */
    float getOpacity() const throw()        { return colour.getFloatAlpha(); }

    /** The solid colour being used.

        If the fill type is not a solid colour, the alpha channel of this colour indicates
        the opacity that should be used for the fill, and the RGB channels are ignored.
    */
    Colour colour;

    /** Returns the gradient that should be used for filling.
        This will be zero if the object is some other type of fill.
        If a gradient is active, the overall opacity with which it should be applied
        is indicated by the alpha channel of the colour variable.
    */
    ScopedPointer <ColourGradient> gradient;

    /** Returns the image that should be used for tiling.
        The FillType object just keeps a pointer to this image, it doesn't own it, so you have to
        be careful to make sure the image doesn't get deleted while it's being used.
        If an image fill is active, the overall opacity with which it should be applied
        is indicated by the alpha channel of the colour variable.
    */
    const Image* image;

    /** The transform that should be applied to the image or gradient that's being drawn.
    */
    AffineTransform transform;

    juce_UseDebuggingNewOperator
};


#endif   // __JUCE_FILLTYPE_JUCEHEADER__
