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

#ifndef __JUCE_DRAWABLEIMAGE_JUCEHEADER__
#define __JUCE_DRAWABLEIMAGE_JUCEHEADER__

#include "juce_Drawable.h"
#include "../positioning/juce_RelativeParallelogram.h"


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
    ~DrawableImage();

    //==============================================================================
    /** Sets the image that this drawable will render. */
    void setImage (const Image& imageToUse);

    /** Returns the current image. */
    const Image& getImage() const noexcept                      { return image; }

    /** Sets the opacity to use when drawing the image. */
    void setOpacity (float newOpacity);

    /** Returns the image's opacity. */
    float getOpacity() const noexcept                           { return opacity; }

    /** Sets a colour to draw over the image's alpha channel.

        By default this is transparent so isn't drawn, but if you set a non-transparent
        colour here, then it will be overlaid on the image, using the image's alpha
        channel as a mask.

        This is handy for doing things like darkening or lightening an image by overlaying
        it with semi-transparent black or white.
    */
    void setOverlayColour (const Colour& newOverlayColour);

    /** Returns the overlay colour. */
    const Colour& getOverlayColour() const noexcept             { return overlayColour; }

    /** Sets the bounding box within which the image should be displayed. */
    void setBoundingBox (const RelativeParallelogram& newBounds);

    /** Returns the position to which the image's top-left corner should be remapped in the target
        coordinate space when rendering this object.
        @see setTransform
    */
    const RelativeParallelogram& getBoundingBox() const noexcept        { return bounds; }

    //==============================================================================
    /** @internal */
    void paint (Graphics& g);
    /** @internal */
    bool hitTest (int x, int y);
    /** @internal */
    Drawable* createCopy() const;
    /** @internal */
    Rectangle<float> getDrawableBounds() const;
    /** @internal */
    void refreshFromValueTree (const ValueTree& tree, ComponentBuilder& builder);
    /** @internal */
    ValueTree createValueTree (ComponentBuilder::ImageProvider* imageProvider) const;
    /** @internal */
    static const Identifier valueTreeType;

    //==============================================================================
    /** Internally-used class for wrapping a DrawableImage's state into a ValueTree. */
    class ValueTreeWrapper   : public Drawable::ValueTreeWrapperBase
    {
    public:
        ValueTreeWrapper (const ValueTree& state);

        var getImageIdentifier() const;
        void setImageIdentifier (const var& newIdentifier, UndoManager* undoManager);
        Value getImageIdentifierValue (UndoManager* undoManager);

        float getOpacity() const;
        void setOpacity (float newOpacity, UndoManager* undoManager);
        Value getOpacityValue (UndoManager* undoManager);

        Colour getOverlayColour() const;
        void setOverlayColour (const Colour& newColour, UndoManager* undoManager);
        Value getOverlayColourValue (UndoManager* undoManager);

        RelativeParallelogram getBoundingBox() const;
        void setBoundingBox (const RelativeParallelogram& newBounds, UndoManager* undoManager);

        static const Identifier opacity, overlay, image, topLeft, topRight, bottomLeft;
    };

private:
    //==============================================================================
    Image image;
    float opacity;
    Colour overlayColour;
    RelativeParallelogram bounds;

    friend class Drawable::Positioner<DrawableImage>;
    bool registerCoordinates (RelativeCoordinatePositionerBase&);
    void recalculateCoordinates (Expression::Scope*);

    DrawableImage& operator= (const DrawableImage&);
    JUCE_LEAK_DETECTOR (DrawableImage)
};


#endif   // __JUCE_DRAWABLEIMAGE_JUCEHEADER__
