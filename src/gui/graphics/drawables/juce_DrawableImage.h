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
    DrawableImage (const DrawableImage& other);

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

        @param imageToUse               the image to render (may be a null pointer)
        @param releaseWhenNotNeeded     if false, a simple pointer is kept to the image; if true,
                                        then the image will be deleted when this object no longer
                                        needs it - unless the image was created by the ImageCache,
                                        in which case it will be released with ImageCache::release().
    */
    void setImage (Image* imageToUse, bool releaseWhenNotNeeded);

    /** Returns the current image. */
    Image* getImage() const throw()                             { return image; }

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
    void setTransform (const RelativePoint& imageTopLeftPosition,
                       const RelativePoint& imageTopRightPosition,
                       const RelativePoint& imageBottomLeftPosition);

    /** Returns the position to which the image's top-left corner should be remapped in the target
        coordinate space when rendering this object.
        @see setTransform
    */
    const RelativePoint& getTargetPositionForTopLeft() const throw()         { return controlPoints[0]; }

    /** Returns the position to which the image's top-right corner should be remapped in the target
        coordinate space when rendering this object.
        @see setTransform
    */
    const RelativePoint& getTargetPositionForTopRight() const throw()        { return controlPoints[1]; }

    /** Returns the position to which the image's bottom-left corner should be remapped in the target
        coordinate space when rendering this object.
        @see setTransform
    */
    const RelativePoint& getTargetPositionForBottomLeft() const throw()      { return controlPoints[2]; }

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
    void invalidatePoints();
    /** @internal */
    const Rectangle<float> refreshFromValueTree (const ValueTree& tree, ImageProvider* imageProvider);
    /** @internal */
    const ValueTree createValueTree (ImageProvider* imageProvider) const;
    /** @internal */
    static const Identifier valueTreeType;
    /** @internal */
    const Identifier getValueTreeType() const    { return valueTreeType; }

    //==============================================================================
    /** Internally-used class for wrapping a DrawableImage's state into a ValueTree. */
    class ValueTreeWrapper   : public ValueTreeWrapperBase
    {
    public:
        ValueTreeWrapper (const ValueTree& state);

        const var getImageIdentifier() const;
        void setImageIdentifier (const var& newIdentifier, UndoManager* undoManager);

        float getOpacity() const;
        void setOpacity (float newOpacity, UndoManager* undoManager);

        const Colour getOverlayColour() const;
        void setOverlayColour (const Colour& newColour, UndoManager* undoManager);

        const RelativePoint getTargetPositionForTopLeft() const;
        void setTargetPositionForTopLeft (const RelativePoint& newPoint, UndoManager* undoManager);

        const RelativePoint getTargetPositionForTopRight() const;
        void setTargetPositionForTopRight (const RelativePoint& newPoint, UndoManager* undoManager);

        const RelativePoint getTargetPositionForBottomLeft() const;
        void setTargetPositionForBottomLeft (const RelativePoint& newPoint, UndoManager* undoManager);

    private:
        static const Identifier opacity, overlay, image, topLeft, topRight, bottomLeft;
    };

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    Image* image;
    bool canDeleteImage;
    float opacity;
    Colour overlayColour;
    RelativePoint controlPoints[3];

    const AffineTransform calculateTransform() const;

    DrawableImage& operator= (const DrawableImage&);
};


#endif   // __JUCE_DRAWABLEIMAGE_JUCEHEADER__
