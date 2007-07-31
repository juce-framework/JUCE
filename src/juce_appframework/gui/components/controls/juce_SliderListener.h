/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

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
