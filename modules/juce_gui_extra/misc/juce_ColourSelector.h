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

#ifndef JUCE_COLOURSELECTOR_H_INCLUDED
#define JUCE_COLOURSELECTOR_H_INCLUDED


//==============================================================================
/**
    A component that lets the user choose a colour.

    This shows RGB sliders and a colourspace that the user can pick colours from.

    This class is also a ChangeBroadcaster, so listeners can register to be told
    when the colour changes.
*/
class JUCE_API  ColourSelector  : public Component,
                                  public ChangeBroadcaster,
                                  protected SliderListener
{
public:
    //==============================================================================
    /** Options for the type of selector to show. These are passed into the constructor. */
    enum ColourSelectorOptions
    {
        showAlphaChannel    = 1 << 0,   /**< if set, the colour's alpha channel can be changed as well as its RGB. */

        showColourAtTop     = 1 << 1,   /**< if set, a swatch of the colour is shown at the top of the component. */
        showSliders         = 1 << 2,   /**< if set, RGB sliders are shown at the bottom of the component. */
        showColourspace     = 1 << 3    /**< if set, a big HSV selector is shown. */
    };

    //==============================================================================
    /** Creates a ColourSelector object.

        The flags are a combination of values from the ColourSelectorOptions enum, specifying
        which of the selector's features should be visible.

        The edgeGap value specifies the amount of space to leave around the edge.

        gapAroundColourSpaceComponent indicates how much of a gap to put around the
        colourspace and hue selector components.
    */
    ColourSelector (int flags = (showAlphaChannel | showColourAtTop | showSliders | showColourspace),
                    int edgeGap = 4,
                    int gapAroundColourSpaceComponent = 7);

    /** Destructor. */
    ~ColourSelector();

    //==============================================================================
    /** Returns the colour that the user has currently selected.

        The ColourSelector class is also a ChangeBroadcaster, so listeners can
        register to be told when the colour changes.

        @see setCurrentColour
    */
    Colour getCurrentColour() const;

    /** Changes the colour that is currently being shown. */
    void setCurrentColour (Colour newColour, NotificationType notificationType = sendNotification);

    //==============================================================================
    /** Tells the selector how many preset colour swatches you want to have on the component.

        To enable swatches, you'll need to override getNumSwatches(), getSwatchColour(), and
        setSwatchColour(), to return the number of colours you want, and to set and retrieve
        their values.
    */
    virtual int getNumSwatches() const;

    /** Called by the selector to find out the colour of one of the swatches.

        Your subclass should return the colour of the swatch with the given index.

        To enable swatches, you'll need to override getNumSwatches(), getSwatchColour(), and
        setSwatchColour(), to return the number of colours you want, and to set and retrieve
        their values.
    */
    virtual Colour getSwatchColour (int index) const;

    /** Called by the selector when the user puts a new colour into one of the swatches.

        Your subclass should change the colour of the swatch with the given index.

        To enable swatches, you'll need to override getNumSwatches(), getSwatchColour(), and
        setSwatchColour(), to return the number of colours you want, and to set and retrieve
        their values.
    */
    virtual void setSwatchColour (int index, const Colour& newColour) const;


    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the keyboard.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        backgroundColourId              = 0x1007000,    /**< the colour used to fill the component's background. */
        labelTextColourId               = 0x1007001     /**< the colour used for the labels next to the sliders. */
    };


private:
    //==============================================================================
    class ColourSpaceView;
    class HueSelectorComp;
    class SwatchComponent;
    class ColourComponentSlider;
    class ColourSpaceMarker;
    class HueSelectorMarker;
    friend class ColourSpaceView;
    friend struct ContainerDeletePolicy<ColourSpaceView>;
    friend class HueSelectorComp;
    friend struct ContainerDeletePolicy<HueSelectorComp>;

    Colour colour;
    float h, s, v;
    ScopedPointer<Slider> sliders[4];
    ScopedPointer<ColourSpaceView> colourSpace;
    ScopedPointer<HueSelectorComp> hueSelector;
    OwnedArray<SwatchComponent> swatchComponents;
    const int flags;
    int edgeGap;
    Rectangle<int> previewArea;

    void setHue (float newH);
    void setSV (float newS, float newV);
    void updateHSV();
    void update (NotificationType);
    void sliderValueChanged (Slider*) override;
    void paint (Graphics&) override;
    void resized() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ColourSelector)

   #if JUCE_CATCH_DEPRECATED_CODE_MISUSE
    // This constructor is here temporarily to prevent old code compiling, because the parameters
    // have changed - if you get an error here, update your code to use the new constructor instead..
    ColourSelector (bool);
   #endif
};


#endif   // JUCE_COLOURSELECTOR_H_INCLUDED
