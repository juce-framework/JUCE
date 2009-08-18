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
    ImageButton (const String& name);

    /** Destructor. */
    ~ImageButton();

    //==============================================================================
    /** Sets up the images to draw in various states.

        Important! Bear in mind that if you pass the same image in for more than one of
        these parameters, this button will delete it (or release from the ImageCache)
        multiple times!

        @param resizeButtonNowToFitThisImage        if true, the button will be immediately
                                                    resized to the same dimensions as the normal image
        @param rescaleImagesWhenButtonSizeChanges   if true, the image will be rescaled to fit the
                                                    button when the button's size changes
        @param preserveImageProportions             if true then any rescaling of the image to fit
                                                    the button will keep the image's x and y proportions
                                                    correct - i.e. it won't distort its shape, although
                                                    this might create gaps around the edges
        @param normalImage                          the image to use when the button is in its normal state. The
                                                    image passed in will be deleted (or released if it
                                                    was created by the ImageCache class) when the
                                                    button no longer needs it.
        @param imageOpacityWhenNormal               the opacity to use when drawing the normal image.
        @param overlayColourWhenNormal              an overlay colour to use to fill the alpha channel of the
                                                    normal image - if this colour is transparent, no overlay
                                                    will be drawn. The overlay will be drawn over the top of the
                                                    image, so you can basically add a solid or semi-transparent
                                                    colour to the image to brighten or darken it
        @param overImage                            the image to use when the mouse is over the button. If
                                                    you want to use the same image as was set in the normalImage
                                                    parameter, this value can be 0. As for normalImage, it
                                                    will be deleted or released by the button when no longer
                                                    needed
        @param imageOpacityWhenOver                 the opacity to use when drawing the image when the mouse
                                                    is over the button
        @param overlayColourWhenOver                an overlay colour to use to fill the alpha channel of the
                                                    image when the mouse is over - if this colour is transparent,
                                                    no overlay will be drawn
        @param downImage                            an image to use when the button is pressed down. If set
                                                    to zero, the 'over' image will be drawn instead (or the
                                                    normal image if there isn't an 'over' image either). This
                                                    image will be deleted or released by the button when no
                                                    longer needed
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
    void setImages (const bool resizeButtonNowToFitThisImage,
                    const bool rescaleImagesWhenButtonSizeChanges,
                    const bool preserveImageProportions,
                    Image* const normalImage,
                    const float imageOpacityWhenNormal,
                    const Colour& overlayColourWhenNormal,
                    Image* const overImage,
                    const float imageOpacityWhenOver,
                    const Colour& overlayColourWhenOver,
                    Image* const downImage,
                    const float imageOpacityWhenDown,
                    const Colour& overlayColourWhenDown,
                    const float hitTestAlphaThreshold = 0.0f);

    /** Returns the currently set 'normal' image. */
    Image* getNormalImage() const throw();

    /** Returns the image that's drawn when the mouse is over the button.

        If an 'over' image has been set, this will return it; otherwise it'll
        just return the normal image.
    */
    Image* getOverImage() const throw();

    /** Returns the image that's drawn when the button is held down.

        If a 'down' image has been set, this will return it; otherwise it'll
        return the 'over' image or normal image, depending on what's available.
    */
    Image* getDownImage() const throw();

    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    /** @internal */
    bool hitTest (int x, int y);
    /** @internal */
    void paintButton (Graphics& g,
                      bool isMouseOverButton,
                      bool isButtonDown);

private:
    //==============================================================================
    bool scaleImageToFit, preserveProportions;
    unsigned char alphaThreshold;
    int imageX, imageY, imageW, imageH;
    Image* normalImage;
    Image* overImage;
    Image* downImage;
    float normalOpacity, overOpacity, downOpacity;
    Colour normalOverlay, overOverlay, downOverlay;

    Image* getCurrentImage() const;
    void deleteImages();

    ImageButton (const ImageButton&);
    const ImageButton& operator= (const ImageButton&);
};


#endif   // __JUCE_IMAGEBUTTON_JUCEHEADER__
