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

#ifndef __JUCE_IMAGEBUTTON_JUCEHEADER__
#define __JUCE_IMAGEBUTTON_JUCEHEADER__

#include "juce_Button.h"


//==============================================================================
/**
    As the title suggests, this is a button containing an image.

    The colour and transparency of the image can be set to vary when the
    button state changes.

    @see Button, ShapeButton, TextButton
*/
class JUCE_API  ImageButton  : public Button
{
public:
    //==============================================================================
    /** Creates an ImageButton.

        Use setImage() to specify the image to use. The colours and opacities that
        are specified here can be changed later using setDrawingOptions().

        @param name                 the name to give the component
    */
    explicit ImageButton (const String& name = String::empty);

    /** Destructor. */
    ~ImageButton();

    //==============================================================================
    /** Sets up the images to draw in various states.

        @param resizeButtonNowToFitThisImage        if true, the button will be immediately
                                                    resized to the same dimensions as the normal image
        @param rescaleImagesWhenButtonSizeChanges   if true, the image will be rescaled to fit the
                                                    button when the button's size changes
        @param preserveImageProportions             if true then any rescaling of the image to fit
                                                    the button will keep the image's x and y proportions
                                                    correct - i.e. it won't distort its shape, although
                                                    this might create gaps around the edges
        @param normalImage                          the image to use when the button is in its normal state.
                                                    button no longer needs it.
        @param imageOpacityWhenNormal               the opacity to use when drawing the normal image.
        @param overlayColourWhenNormal              an overlay colour to use to fill the alpha channel of the
                                                    normal image - if this colour is transparent, no overlay
                                                    will be drawn. The overlay will be drawn over the top of the
                                                    image, so you can basically add a solid or semi-transparent
                                                    colour to the image to brighten or darken it
        @param overImage                            the image to use when the mouse is over the button. If
                                                    you want to use the same image as was set in the normalImage
                                                    parameter, this value can be a null image.
        @param imageOpacityWhenOver                 the opacity to use when drawing the image when the mouse
                                                    is over the button
        @param overlayColourWhenOver                an overlay colour to use to fill the alpha channel of the
                                                    image when the mouse is over - if this colour is transparent,
                                                    no overlay will be drawn
        @param downImage                            an image to use when the button is pressed down. If set
                                                    to a null image, the 'over' image will be drawn instead (or the
                                                    normal image if there isn't an 'over' image either).
        @param imageOpacityWhenDown                 the opacity to use when drawing the image when the button
                                                    is pressed
        @param overlayColourWhenDown                an overlay colour to use to fill the alpha channel of the
                                                    image when the button is pressed down - if this colour is
                                                    transparent, no overlay will be drawn
        @param hitTestAlphaThreshold                if set to zero, the mouse is considered to be over the button
                                                    whenever it's inside the button's bounding rectangle. If
                                                    set to values higher than 0, the mouse will only be
                                                    considered to be over the image when the value of the
                                                    image's alpha channel at that position is greater than
                                                    this level.
    */
    void setImages (bool resizeButtonNowToFitThisImage,
                    bool rescaleImagesWhenButtonSizeChanges,
                    bool preserveImageProportions,
                    const Image& normalImage,
                    float imageOpacityWhenNormal,
                    const Colour& overlayColourWhenNormal,
                    const Image& overImage,
                    float imageOpacityWhenOver,
                    const Colour& overlayColourWhenOver,
                    const Image& downImage,
                    float imageOpacityWhenDown,
                    const Colour& overlayColourWhenDown,
                    float hitTestAlphaThreshold = 0.0f);

    /** Returns the currently set 'normal' image. */
    Image getNormalImage() const;

    /** Returns the image that's drawn when the mouse is over the button.

        If a valid 'over' image has been set, this will return it; otherwise it'll
        just return the normal image.
    */
    Image getOverImage() const;

    /** Returns the image that's drawn when the button is held down.

        If a valid 'down' image has been set, this will return it; otherwise it'll
        return the 'over' image or normal image, depending on what's available.
    */
    Image getDownImage() const;

protected:
    //==============================================================================
    /** @internal */
    bool hitTest (int x, int y);
    /** @internal */
    void paintButton (Graphics& g,
                      bool isMouseOverButton,
                      bool isButtonDown);

private:
    //==============================================================================
    bool scaleImageToFit, preserveProportions;
    uint8 alphaThreshold;
    Rectangle<int> imageBounds;
    Image normalImage, overImage, downImage;
    float normalOpacity, overOpacity, downOpacity;
    Colour normalOverlay, overOverlay, downOverlay;

    Image getCurrentImage() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImageButton)
};


#endif   // __JUCE_IMAGEBUTTON_JUCEHEADER__
