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

#ifndef __JUCE_SLIDERPROPERTYCOMPONENT_JUCEHEADER__
#define __JUCE_SLIDERPROPERTYCOMPONENT_JUCEHEADER__

#include "juce_PropertyComponent.h"
#include "../controls/juce_Slider.h"


//==============================================================================
/**
    A PropertyComponent that shows its value as a slider.

    @see PropertyComponent, Slider
*/
class JUCE_API  SliderPropertyComponent   : public PropertyComponent,
                                            private SliderListener
{
public:
    //==============================================================================
    /** Creates the property component.

        The ranges, interval and skew factor are passed to the Slider component.

        If you need to customise the slider in other ways, your constructor can
        access the slider member variable and change it directly.
    */
    SliderPropertyComponent (const String& propertyName,
                             const double rangeMin,
                             const double rangeMax,
                             const double interval,
                             const double skewFactor = 1.0);

    /** Destructor. */
    ~SliderPropertyComponent();


    //==============================================================================
    /** Called when the user moves the slider to change its value.

        Your subclass must use this method to update whatever item this property
        represents.
    */
    virtual void setValue (const double newValue) = 0;

    /** Returns the value that the slider should show. */
    virtual const double getValue() const = 0;


    //==============================================================================
    /** @internal */
    void refresh();
    /** @internal */
    void changeListenerCallback (void*);
    /** @internal */
    void sliderValueChanged (Slider*);

    juce_UseDebuggingNewOperator

protected:
    //==============================================================================
    /** The slider component being used in this component.

        Your subclass has access to this in case it needs to customise it in some way.
    */
    Slider* slider;
};


#endif   // __JUCE_SLIDERPROPERTYCOMPONENT_JUCEHEADER__
