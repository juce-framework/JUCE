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

#ifndef __JUCE_SLIDERLISTENER_JUCEHEADER__
#define __JUCE_SLIDERLISTENER_JUCEHEADER__

class Slider;


//==============================================================================
/**
    A class for receiving callbacks from a Slider.

    To be told when a slider's value changes, you can register a SliderListener
    object using Slider::addListener().

    @see Slider::addListener, Slider::removeListener
*/
class JUCE_API  SliderListener
{
public:
    //==============================================================================
    /** Destructor. */
    virtual ~SliderListener() {}

    //==============================================================================
    /** Called when the slider's value is changed.

        This may be caused by dragging it, or by typing in its text entry box,
        or by a call to Slider::setValue().

        You can find out the new value using Slider::getValue().

        @see Slider::valueChanged
    */
    virtual void sliderValueChanged (Slider* slider) = 0;

    //==============================================================================
    /** Called when the slider is about to be dragged.

        This is called when a drag begins, then it's followed by multiple calls
        to sliderValueChanged(), and then sliderDragEnded() is called after the
        user lets go.

        @see sliderDragEnded, Slider::startedDragging
    */
    virtual void sliderDragStarted (Slider* slider);

    /** Called after a drag operation has finished.

        @see sliderDragStarted, Slider::stoppedDragging
    */
    virtual void sliderDragEnded (Slider* slider);
};

#endif   // __JUCE_SLIDERLISTENER_JUCEHEADER__
