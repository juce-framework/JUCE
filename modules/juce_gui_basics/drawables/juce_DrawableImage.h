/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifndef JUCE_DRAWABLEIMAGE_H_INCLUDED
#define JUCE_DRAWABLEIMAGE_H_INCLUDED


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
    DrawableImage (const DrawableImage&);

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
    void setOverlayColour (Colour newOverlayColour);

    /** Returns the overlay colour. */
    Colour getOverlayColour() const noexcept                    { return overlayColour; }

    /** Sets the bounding box within which the image should be displayed. */
    void setBoundingBox (const RelativeParallelogram& newBounds);

    /** Returns the position to which the image's top-left corner should be remapped in the target
        coordinate space when rendering this object.
        @see setTransform
    */
    const RelativeParallelogram& getBoundingBox() const noexcept        { return bounds; }

    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    bool hitTest (int x, int y) override;
    /** @internal */
    Drawable* createCopy() const override;
    /** @internal */
    Rectangle<float> getDrawableBounds() const override;
    /** @internal */
    void refreshFromValueTree (const ValueTree& tree, ComponentBuilder&);
    /** @internal */
    ValueTree createValueTree (ComponentBuilder::ImageProvider*) const override;
    /** @internal */
    static const Identifier valueTreeType;

    //==============================================================================
    /** Internally-used class for wrapping a DrawableImage's state into a ValueTree. */
    class ValueTreeWrapper   : public Drawable::ValueTreeWrapperBase
    {
    public:
        ValueTreeWrapper (const ValueTree& state);

        var getImageIdentifier() const;
        void setImageIdentifier (const var&, UndoManager*);
        Value getImageIdentifierValue (UndoManager*);

        float getOpacity() const;
        void setOpacity (float newOpacity, UndoManager*);
        Value getOpacityValue (UndoManager*);

        Colour getOverlayColour() const;
        void setOverlayColour (Colour newColour, UndoManager*);
        Value getOverlayColourValue (UndoManager*);

        RelativeParallelogram getBoundingBox() const;
        void setBoundingBox (const RelativeParallelogram&, UndoManager*);

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


#endif   // JUCE_DRAWABLEIMAGE_H_INCLUDED
