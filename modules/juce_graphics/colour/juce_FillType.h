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

#ifndef JUCE_FILLTYPE_H_INCLUDED
#define JUCE_FILLTYPE_H_INCLUDED


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
    FillType (const FillType&);

    /** Makes a copy of another FillType. */
    FillType& operator= (const FillType&);

   #if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
    FillType (FillType&&) noexcept;
    FillType& operator= (FillType&&) noexcept;
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
    bool operator== (const FillType&) const;
    bool operator!= (const FillType&) const;

private:
    JUCE_LEAK_DETECTOR (FillType)
};


#endif   // JUCE_FILLTYPE_H_INCLUDED
