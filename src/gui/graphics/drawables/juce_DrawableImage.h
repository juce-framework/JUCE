/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#ifndef __JUCE_DRAWABLEIMAGE_JUCEHEADER__
#define __JUCE_DRAWABLEIMAGE_JUCEHEADER__

#include "juce_Drawable.h"


//==============================================================================
/**
    A drawable object which is a bitmap image.

    @see Drawable
*/
class JUCE_API  DrawableImage  : public Drawable
{
public:
    //==============================================================================
    DrawableImage();

    /** Destructor. */
    virtual ~DrawableImage();

    //==============================================================================
    /** Sets the image that this drawable will render.

        An internal copy is made of the image passed-in. If you want to provide an
        image that this object can take charge of without needing to create a copy,
        use the other setImage() method.
    */
    void setImage (const Image& imageToCopy);

    /** Sets the image that this drawable will render.

        A good way to use this is with the ImageCache - if you create an image
        with ImageCache and pass it in here with releaseWhenNotNeeded = true, then
        it'll be released neatly with its reference count being decreased.

        @param imageToUse               the image to render
        @param releaseWhenNotNeeded     if false, a simple pointer is kept to the image; if true,
                                        then the image will be deleted when this object no longer
                                        needs it - unless the image was created by the ImageCache,
                                        in which case it will be released with ImageCache::release().
    */
    void setImage (Image* imageToUse, bool releaseWhenNotNeeded);

    /** Returns the current image. */
    Image* getImage() const throw()                             { return image; }

    /** Clears (and possibly deletes) the currently set image. */
    void clearImage();

    /** Sets the opacity to use when drawing the image. */
    void setOpacity (float newOpacity);

    /** Returns the image's opacity. */
    float getOpacity() const throw()                            { return opacity; }

    /** Sets a colour to draw over the image's alpha channel.

        By default this is transparent so isn't drawn, but if you set a non-transparent
        colour here, then it will be overlaid on the image, using the image's alpha
        channel as a mask.

        This is handy for doing things like darkening or lightening an image by overlaying
        it with semi-transparent black or white.
    */
    void setOverlayColour (const Colour& newOverlayColour);

    /** Returns the overlay colour. */
    const Colour& getOverlayColour() const throw()              { return overlayColour; }

    /** Sets the transform to be applied to this image, by defining the positions
        where three anchor points should end up in the target rendering space.

        @param imageTopLeftPosition     the position that the image's top-left corner should be mapped to
                                        in the target coordinate space.
        @param imageTopRightPosition    the position that the image's top-right corner should be mapped to
                                        in the target coordinate space.
        @param imageBottomLeftPosition  the position that the image's bottom-left corner should be mapped to
                                        in the target coordinate space.
    */
    void setTransform (const Point<float>& imageTopLeftPosition,
                       const Point<float>& imageTopRightPosition,
                       const Point<float>& imageBottomLeftPosition);

    /** Returns the position to which the image's top-left corner should be remapped in the target
        coordinate space when rendering this object.
        @see setTransform
    */
    const Point<float>& getTargetPositionForTopLeft() const throw()         { return controlPoints[0]; }

    /** Returns the position to which the image's top-right corner should be remapped in the target
        coordinate space when rendering this object.
        @see setTransform
    */
    const Point<float>& getTargetPositionForTopRight() const throw()        { return controlPoints[1]; }

    /** Returns the position to which the image's bottom-left corner should be remapped in the target
        coordinate space when rendering this object.
        @see setTransform
    */
    const Point<float>& getTargetPositionForBottomLeft() const throw()      { return controlPoints[2]; }

    //==============================================================================
    /** @internal */
    void render (const Drawable::RenderingContext& context) const;
    /** @internal */
    const Rectangle<float> getBounds() const;
    /** @internal */
    bool hitTest (float x, float y) const;
    /** @internal */
    Drawable* createCopy() const;
    /** @internal */
    const Rectangle<float> refreshFromValueTree (const ValueTree& tree, ImageProvider* imageProvider);
    /** @internal */
    const ValueTree createValueTree (ImageProvider* imageProvider) const;
    /** @internal */
    static const Identifier valueTreeType;
    /** @internal */
    const Identifier getValueTreeType() const    { return valueTreeType; }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    Image* image;
    bool canDeleteImage;
    float opacity;
    Colour overlayColour;
    Point<float> controlPoints[3];

    const AffineTransform getTransform() const;

    DrawableImage (const DrawableImage&);
    DrawableImage& operator= (const DrawableImage&);
};


#endif   // __JUCE_DRAWABLEIMAGE_JUCEHEADER__
