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

#ifndef __JUCE_FILLTYPE_JUCEHEADER__
#define __JUCE_FILLTYPE_JUCEHEADER__

#include "../colour/juce_ColourGradient.h"
#include "../images/juce_Image.h"


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
    //==============================================================================
    /** Creates a default fill type, of solid black. */
    FillType() noexcept;

    /** Creates a fill type of a solid colour.
        @see setColour
    */
    FillType (Colour colour) noexcept;

    /** Creates a gradient fill type.
        @see setGradient
    */
    FillType (const ColourGradient& gradient);

    /** Creates a tiled image fill type. The transform allows you to set the scaling, offset
        and rotation of the pattern.
        @see setTiledImage
    */
    FillType (const Image& image, const AffineTransform& transform) noexcept;

    /** Creates a copy of another FillType. */
    FillType (const FillType& other);

    /** Makes a copy of another FillType. */
    FillType& operator= (const FillType& other);

   #if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
    FillType (FillType&& other) noexcept;
    FillType& operator= (FillType&& other) noexcept;
   #endif

    /** Destructor. */
    ~FillType() noexcept;

    //==============================================================================
    /** Returns true if this is a solid colour fill, and not a gradient or image. */
    bool isColour() const noexcept          { return gradient == nullptr && image.isNull(); }

    /** Returns true if this is a gradient fill. */
    bool isGradient() const noexcept        { return gradient != nullptr; }

    /** Returns true if this is a tiled image pattern fill. */
    bool isTiledImage() const noexcept      { return image.isValid(); }

    /** Turns this object into a solid colour fill.
        If the object was an image or gradient, those fields will no longer be valid. */
    void setColour (Colour newColour) noexcept;

    /** Turns this object into a gradient fill. */
    void setGradient (const ColourGradient& newGradient);

    /** Turns this object into a tiled image fill type. The transform allows you to set
        the scaling, offset and rotation of the pattern.
    */
    void setTiledImage (const Image& image, const AffineTransform& transform) noexcept;

    /** Changes the opacity that should be used.
        If the fill is a solid colour, this just changes the opacity of that colour. For
        gradients and image tiles, it changes the opacity that will be used for them.
    */
    void setOpacity (float newOpacity) noexcept;

    /** Returns the current opacity to be applied to the colour, gradient, or image.
        @see setOpacity
    */
    float getOpacity() const noexcept       { return colour.getFloatAlpha(); }

    /** Returns true if this fill type is completely transparent. */
    bool isInvisible() const noexcept;

    /** Returns a copy of this fill, adding the specified transform applied to the
        existing transform.
    */
    FillType transformed (const AffineTransform& transform) const;

    //==============================================================================
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

    /** The image that should be used for tiling.
        If an image fill is active, the overall opacity with which it should be applied
        is indicated by the alpha channel of the colour variable.
    */
    Image image;

    /** The transform that should be applied to the image or gradient that's being drawn. */
    AffineTransform transform;

    //==============================================================================
    bool operator== (const FillType& other) const;
    bool operator!= (const FillType& other) const;

private:
    JUCE_LEAK_DETECTOR (FillType)
};


#endif   // __JUCE_FILLTYPE_JUCEHEADER__
