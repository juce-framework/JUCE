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

        An internal copy of this will not be made, so the caller mustn't delete
        the image while it's still being used by this object.

        A good way to use this is with the ImageCache - if you create an image
        with ImageCache and pass it in here with releaseWhenNotNeeded = true, then
        it'll be released neatly with its reference count being decreased.

        @param imageToUse               the image to render
        @param releaseWhenNotNeeded     if false, a simple pointer is kept to the image; if true,
                                        then the image will be deleted when this object no longer
                                        needs it - unless the image was created by the ImageCache,
                                        in which case it will be released with ImageCache::release().
    */
    void setImage (Image* imageToUse,
                   const bool releaseWhenNotNeeded);

    /** Returns the current image. */
    Image* getImage() const throw()                             { return image; }

    /** Clears (and possibly deletes) the currently set image. */
    void clearImage();

    /** Sets the opacity to use when drawing the image. */
    void setOpacity (const float newOpacity);

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


    //==============================================================================
    /** @internal */
    void render (const Drawable::RenderingContext& context) const;
    /** @internal */
    void getBounds (float& x, float& y, float& width, float& height) const;
    /** @internal */
    bool hitTest (float x, float y) const;
    /** @internal */
    Drawable* createCopy() const;
    /** @internal */
    ValueTree createValueTree() const throw();
    /** @internal */
    static DrawableImage* createFromValueTree (const ValueTree& tree) throw();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    Image* image;
    bool canDeleteImage;
    float opacity;
    Colour overlayColour;

    DrawableImage (const DrawableImage&);
    const DrawableImage& operator= (const DrawableImage&);
};


#endif   // __JUCE_DRAWABLEIMAGE_JUCEHEADER__
