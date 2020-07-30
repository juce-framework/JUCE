/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    A button that displays a Drawable.

    Up to three Drawable objects can be given to this button, to represent the
    'normal', 'over' and 'down' states.

    @see Button

    @tags{GUI}
*/
class JUCE_API  DrawableButton  : public Button
{
public:
    //==============================================================================
    enum ButtonStyle
    {
        ImageFitted,                            /**< The button will just display the images, but will resize and centre them to fit inside it. */
        ImageRaw,                               /**< The button will just display the images in their normal size and position.
                                                     This leaves it up to the caller to make sure the images are the correct size and position for the button. */
        ImageAboveTextLabel,                    /**< Draws the button as a text label across the bottom with the image resized and scaled to fit above it. */
        ImageOnButtonBackground,                /**< Draws the button as a standard rounded-rectangle button with the image on top. The image will be resized
                                                     to match the button's proportions.
                                                     Note that if you use this style, the colour IDs that control the button colour are
                                                     TextButton::buttonColourId and TextButton::buttonOnColourId. */
        ImageOnButtonBackgroundOriginalSize,    /** Same as ImageOnButtonBackground, but keeps the original image size. */
        ImageStretched                          /**< Fills the button with a stretched version of the image. */
    };

    //==============================================================================
    /** Creates a DrawableButton.

        After creating one of these, use setImages() to specify the drawables to use.

        @param buttonName           the name to give the component
        @param buttonStyle          the layout to use

        @see ButtonStyle, setButtonStyle, setImages
    */
    DrawableButton (const String& buttonName,
                    ButtonStyle buttonStyle);

    /** Destructor. */
    ~DrawableButton() override;

    //==============================================================================
    /** Sets up the images to draw for the various button states.

        The button will keep its own internal copies of these drawables.

        @param normalImage      the thing to draw for the button's 'normal' state. An internal copy
                                will be made of the object passed-in if it is non-null.
        @param overImage        the thing to draw for the button's 'over' state - if this is
                                null, the button's normal image will be used when the mouse is
                                over it. An internal copy will be made of the object passed-in
                                if it is non-null.
        @param downImage        the thing to draw for the button's 'down' state - if this is
                                null, the 'over' image will be used instead (or the normal image
                                as a last resort). An internal copy will be made of the object
                                passed-in if it is non-null.
        @param disabledImage    an image to draw when the button is disabled. If this is null,
                                the normal image will be drawn with a reduced opacity instead.
                                An internal copy will be made of the object passed-in if it is
                                non-null.
        @param normalImageOn    same as the normalImage, but this is used when the button's toggle
                                state is 'on'. If this is nullptr, the normal image is used instead
        @param overImageOn      same as the overImage, but this is used when the button's toggle
                                state is 'on'. If this is nullptr, the normalImageOn is drawn instead
        @param downImageOn      same as the downImage, but this is used when the button's toggle
                                state is 'on'. If this is nullptr, the overImageOn is drawn instead
        @param disabledImageOn  same as the disabledImage, but this is used when the button's toggle
                                state is 'on'. If this is nullptr, the normal image will be drawn instead
                                with a reduced opacity
    */
    void setImages (const Drawable* normalImage,
                    const Drawable* overImage       = nullptr,
                    const Drawable* downImage       = nullptr,
                    const Drawable* disabledImage   = nullptr,
                    const Drawable* normalImageOn   = nullptr,
                    const Drawable* overImageOn     = nullptr,
                    const Drawable* downImageOn     = nullptr,
                    const Drawable* disabledImageOn = nullptr);


    //==============================================================================
    /** Changes the button's style.
        @see ButtonStyle
    */
    void setButtonStyle (ButtonStyle newStyle);

    /** Returns the current style. */
    ButtonStyle getStyle() const noexcept       { return style; }

    //==============================================================================
    /** Gives the button an optional amount of space around the edge of the drawable.
        By default there's a gap of about 3 pixels.
    */
    void setEdgeIndent (int numPixelsIndent);

    /** Returns the current edge indent size. */
    int getEdgeIndent() const noexcept          { return edgeIndent; }

    //==============================================================================
    /** Returns the image that the button is currently displaying. */
    Drawable* getCurrentImage() const noexcept;

    /** Returns the image that the button will use for its normal state. */
    Drawable* getNormalImage() const noexcept;
    /** Returns the image that the button will use when the mouse is over it. */
    Drawable* getOverImage() const noexcept;
    /** Returns the image that the button will use when the mouse is held down on it. */
    Drawable* getDownImage() const noexcept;

    /** Can be overridden to specify a custom position for the image within the button. */
    virtual Rectangle<float> getImageBounds() const;

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the link.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        Note that when the ImageOnButtonBackground style is used, the colour IDs that control
        the button colour are TextButton::buttonColourId and TextButton::buttonOnColourId.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        textColourId             = 0x1004010,  /**< The colour to use for the button's text label. */
        textColourOnId           = 0x1004013,  /**< The colour to use for the button's text when the button's toggle state is "on". */

        backgroundColourId       = 0x1004011,  /**< The colour used to fill the button's background (when
                                                    the button is toggled 'off'). Note that if you use the
                                                    ImageOnButtonBackground style, you should use TextButton::buttonColourId
                                                    to change the button's colour. */
        backgroundOnColourId     = 0x1004012,  /**< The colour used to fill the button's background (when
                                                    the button is toggled 'on'). Note that if you use the
                                                    ImageOnButtonBackground style, you should use TextButton::buttonOnColourId
                                                    to change the button's colour. */
    };

    //==============================================================================
    /** @internal */
    void paintButton (Graphics&, bool, bool) override;
    /** @internal */
    void buttonStateChanged() override;
    /** @internal */
    void resized() override;
    /** @internal */
    void enablementChanged() override;
    /** @internal */
    void colourChanged() override;

private:
    //==============================================================================
    bool shouldDrawButtonBackground() const  { return style == ImageOnButtonBackground || style == ImageOnButtonBackgroundOriginalSize; }

    //==============================================================================
    ButtonStyle style;
    std::unique_ptr<Drawable> normalImage, overImage, downImage, disabledImage,
                              normalImageOn, overImageOn, downImageOn, disabledImageOn;
    Drawable* currentImage = nullptr;
    int edgeIndent = 3;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DrawableButton)
};

} // namespace juce
