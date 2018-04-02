/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-license
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
    A component that lets the user choose a color.

    This shows RGB sliders and a colorspace that the user can pick colors from.

    This class is also a ChangeBroadcaster, so listeners can register to be told
    when the color changes.

    @tags{GUI}
*/
class JUCE_API  ColorSelector  : public Component,
                                  public ChangeBroadcaster
{
public:
    //==============================================================================
    /** Options for the type of selector to show. These are passed into the constructor. */
    enum ColorSelectorOptions
    {
        showAlphaChannel    = 1 << 0,   /**< if set, the color's alpha channel can be changed as well as its RGB. */

        showColorAtTop     = 1 << 1,   /**< if set, a swatch of the color is shown at the top of the component. */
        showSliders         = 1 << 2,   /**< if set, RGB sliders are shown at the bottom of the component. */
        showColorspace     = 1 << 3    /**< if set, a big HSV selector is shown. */
    };

    //==============================================================================
    /** Creates a ColorSelector object.

        The flags are a combination of values from the ColorSelectorOptions enum, specifying
        which of the selector's features should be visible.

        The edgeGap value specifies the amount of space to leave around the edge.

        gapAroundColorSpaceComponent indicates how much of a gap to put around the
        colorspace and hue selector components.
    */
    ColorSelector (int flags = (showAlphaChannel | showColorAtTop | showSliders | showColorspace),
                    int edgeGap = 4,
                    int gapAroundColorSpaceComponent = 7);

    /** Destructor. */
    ~ColorSelector();

    //==============================================================================
    /** Returns the color that the user has currently selected.

        The ColorSelector class is also a ChangeBroadcaster, so listeners can
        register to be told when the color changes.

        @see setCurrentColor
    */
    Color getCurrentColor() const;

    /** Changes the color that is currently being shown.

        @param newColor           the new color to show
        @param notificationType    whether to send a notification of the change to listeners.
                                   A notification will only be sent if the color has changed.
    */
    void setCurrentColor (Color newColor, NotificationType notificationType = sendNotification);

    //==============================================================================
    /** Tells the selector how many preset color swatches you want to have on the component.

        To enable swatches, you'll need to override getNumSwatches(), getSwatchColor(), and
        setSwatchColor(), to return the number of colors you want, and to set and retrieve
        their values.
    */
    virtual int getNumSwatches() const;

    /** Called by the selector to find out the color of one of the swatches.

        Your subclass should return the color of the swatch with the given index.

        To enable swatches, you'll need to override getNumSwatches(), getSwatchColor(), and
        setSwatchColor(), to return the number of colors you want, and to set and retrieve
        their values.
    */
    virtual Color getSwatchColor (int index) const;

    /** Called by the selector when the user puts a new color into one of the swatches.

        Your subclass should change the color of the swatch with the given index.

        To enable swatches, you'll need to override getNumSwatches(), getSwatchColor(), and
        setSwatchColor(), to return the number of colors you want, and to set and retrieve
        their values.
    */
    virtual void setSwatchColor (int index, const Color& newColor);


    //==============================================================================
    /** A set of color IDs to use to change the color of various aspects of the keyboard.

        These constants can be used either via the Component::setColor(), or LookAndFeel::setColor()
        methods.

        @see Component::setColor, Component::findColor, LookAndFeel::setColor, LookAndFeel::findColor
    */
    enum ColorIds
    {
        backgroundColorId              = 0x1007000,    /**< the color used to fill the component's background. */
        labelTextColorId               = 0x1007001     /**< the color used for the labels next to the sliders. */
    };


private:
    //==============================================================================
    class ColorSpaceView;
    class HueSelectorComp;
    class SwatchComponent;
    class ColorComponentSlider;
    class ColorSpaceMarker;
    class HueSelectorMarker;
    friend class ColorSpaceView;
    friend struct ContainerDeletePolicy<ColorSpaceView>;
    friend class HueSelectorComp;
    friend struct ContainerDeletePolicy<HueSelectorComp>;

    Color color;
    float h, s, v;
    ScopedPointer<Slider> sliders[4];
    ScopedPointer<ColorSpaceView> colorSpace;
    ScopedPointer<HueSelectorComp> hueSelector;
    OwnedArray<SwatchComponent> swatchComponents;
    const int flags;
    int edgeGap;
    Rectangle<int> previewArea;

    void setHue (float newH);
    void setSV (float newS, float newV);
    void updateHSV();
    void update (NotificationType);
    void changeColor();
    void paint (Graphics&) override;
    void resized() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ColorSelector)

   #if JUCE_CATCH_DEPRECATED_CODE_MISUSE
    // This constructor is here temporarily to prevent old code compiling, because the parameters
    // have changed - if you get an error here, update your code to use the new constructor instead..
    ColorSelector (bool);
   #endif
};

} // namespace juce
