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

#ifndef __JUCE_DRAWABLEBUTTON_JUCEHEADER__
#define __JUCE_DRAWABLEBUTTON_JUCEHEADER__

#include "juce_Button.h"
#include "../../graphics/drawables/juce_Drawable.h"


//==============================================================================
/**
    A button that displays a Drawable.

    Up to three Drawable objects can be given to this button, to represent the
    'normal', 'over' and 'down' states.

    @see Button
*/
class JUCE_API  DrawableButton  : public Button
{
public:
    //==============================================================================
    enum ButtonStyle
    {
        ImageFitted,                /**< The button will just display the images, but will resize and centre them to fit inside it. */
        ImageRaw,                   /**< The button will just display the images in their normal size and position.
                                         This leaves it up to the caller to make sure the images are the correct size and position for the button. */
        ImageAboveTextLabel,        /**< Draws the button as a text label across the bottom with the image resized and scaled to fit above it. */
        ImageOnButtonBackground     /**< Draws the button as a standard rounded-rectangle button with the image on top. */
    };

    //==============================================================================
    /** Creates a DrawableButton.

        After creating one of these, use setImages() to specify the drawables to use.

        @param buttonName           the name to give the component
        @param buttonStyle          the layout to use

        @see ButtonStyle, setButtonStyle, setImages
    */
    DrawableButton (const String& buttonName,
                    const ButtonStyle buttonStyle);

    /** Destructor. */
    ~DrawableButton();

    //==============================================================================
    /** Sets up the images to draw for the various button states.

        The button will keep its own internal copies of these drawables.

        @param normalImage      the thing to draw for the button's 'normal' state. An internal copy
                                will be made of the object passed-in if it is non-zero.
        @param overImage        the thing to draw for the button's 'over' state - if this is
                                zero, the button's normal image will be used when the mouse is
                                over it. An internal copy will be made of the object passed-in
                                if it is non-zero.
        @param downImage        the thing to draw for the button's 'down' state - if this is
                                zero, the 'over' image will be used instead (or the normal image
                                as a last resort). An internal copy will be made of the object
                                passed-in if it is non-zero.
        @param disabledImage    an image to draw when the button is disabled. If this is zero,
                                the normal image will be drawn with a reduced opacity instead.
                                An internal copy will be made of the object passed-in if it is
                                non-zero.
        @param normalImageOn    same as the normalImage, but this is used when the button's toggle
                                state is 'on'. If this is 0, the normal image is used instead
        @param overImageOn      same as the overImage, but this is used when the button's toggle
                                state is 'on'. If this is 0, the normalImageOn is drawn instead
        @param downImageOn      same as the downImage, but this is used when the button's toggle
                                state is 'on'. If this is 0, the overImageOn is drawn instead
        @param disabledImageOn  same as the disabledImage, but this is used when the button's toggle
                                state is 'on'. If this is 0, the normal image will be drawn instead
                                with a reduced opacity
    */
    void setImages (const Drawable* normalImage,
                    const Drawable* overImage = 0,
                    const Drawable* downImage = 0,
                    const Drawable* disabledImage = 0,
                    const Drawable* normalImageOn = 0,
                    const Drawable* overImageOn = 0,
                    const Drawable* downImageOn = 0,
                    const Drawable* disabledImageOn = 0);


    //==============================================================================
    /** Changes the button's style.

        @see ButtonStyle
    */
    void setButtonStyle (const ButtonStyle newStyle);

    //==============================================================================
    /** Changes the button's background colours.

        The toggledOffColour is the colour to use when the button's toggle state
        is off, and toggledOnColour when it's on.

        For an ImageOnly or ImageAboveTextLabel style, the background colour is
        used to fill the background of the component.

        For an ImageOnButtonBackground style, the colour is used to draw the
        button's lozenge shape and exactly how the colour's used will depend
        on the LookAndFeel.
    */
    void setBackgroundColours (const Colour& toggledOffColour,
                               const Colour& toggledOnColour);

    /** Returns the current background colour being used.

        @see setBackgroundColour
    */
    const Colour& getBackgroundColour() const throw();

    /** Gives the button an optional amount of space around the edge of the drawable.

        This will only apply to ImageFitted or ImageRaw styles, it won't affect the
        ones on a button background. If the button is too small for the given gap, a
        smaller gap will be used.

        By default there's a gap of about 3 pixels.
    */
    void setEdgeIndent (const int numPixelsIndent);

    //==============================================================================
    /** Returns the image that the button is currently displaying. */
    const Drawable* getCurrentImage() const throw();
    const Drawable* getNormalImage() const throw();
    const Drawable* getOverImage() const throw();
    const Drawable* getDownImage() const throw();

    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    /** @internal */
    void paintButton (Graphics& g,
                      bool isMouseOverButton,
                      bool isButtonDown);

private:
    //==============================================================================
    ButtonStyle style;
    ScopedPointer <Drawable> normalImage, overImage, downImage, disabledImage;
    ScopedPointer <Drawable> normalImageOn, overImageOn, downImageOn, disabledImageOn;
    Colour backgroundOff, backgroundOn;
    int edgeIndent;

    void deleteImages();
    DrawableButton (const DrawableButton&);
    const DrawableButton& operator= (const DrawableButton&);
};


#endif   // __JUCE_DRAWABLEBUTTON_JUCEHEADER__
