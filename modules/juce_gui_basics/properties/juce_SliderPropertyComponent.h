/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

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
    A PropertyComponent that shows its value as a slider.

    @see PropertyComponent, Slider

    @tags{GUI}
*/
class JUCE_API  SliderPropertyComponent   : public PropertyComponent
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
    ~SliderPropertyComponent() override;


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
    void refresh() override;

protected:
    /** The slider component being used in this component.
        Your subclass has access to this in case it needs to customise it in some way.
    */
    Slider slider;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SliderPropertyComponent)
};

} // namespace juce
