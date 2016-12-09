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

#ifndef JUCE_SLIDERPROPERTYCOMPONENT_H_INCLUDED
#define JUCE_SLIDERPROPERTYCOMPONENT_H_INCLUDED


//==============================================================================
/**
    A PropertyComponent that shows its value as a slider.

    @see PropertyComponent, Slider
*/
class JUCE_API  SliderPropertyComponent   : public PropertyComponent,
                                            private SliderListener  // (can't use Slider::Listener due to idiotic VC2005 bug)
{
protected:
    //==============================================================================
    /** Creates the property component.

        The ranges, interval and skew factor are passed to the Slider component.

        If you need to customise the slider in other ways, your constructor can
        access the slider member variable and change it directly.
    */
    SliderPropertyComponent (const String& propertyName,
                             double rangeMin,
                             double rangeMax,
                             double interval,
                             double skewFactor = 1.0,
                             bool symmetricSkew = false);

public:
    //==============================================================================
    /** Creates the property component.

        The ranges, interval and skew factor are passed to the Slider component.

        If you need to customise the slider in other ways, your constructor can
        access the slider member variable and change it directly.

        Note that if you call this constructor then you must use the Value to interact with
        the value, and you can't override the class with your own setValue or getValue methods.
        If you want to use those methods, call the other constructor instead.
    */
    SliderPropertyComponent (const Value& valueToControl,
                             const String& propertyName,
                             double rangeMin,
                             double rangeMax,
                             double interval,
                             double skewFactor = 1.0,
                             bool symmetricSkew = false);

    /** Destructor. */
    ~SliderPropertyComponent();


    //==============================================================================
    /** Called when the user moves the slider to change its value.

        Your subclass must use this method to update whatever item this property
        represents.
    */
    virtual void setValue (double newValue);

    /** Returns the value that the slider should show. */
    virtual double getValue() const;


    //==============================================================================
    /** @internal */
    void refresh();
    /** @internal */
    void sliderValueChanged (Slider*);

protected:
    /** The slider component being used in this component.
        Your subclass has access to this in case it needs to customise it in some way.
    */
    Slider slider;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SliderPropertyComponent)
};


#endif   // JUCE_SLIDERPROPERTYCOMPONENT_H_INCLUDED
