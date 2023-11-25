/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

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
    A component that lets the user choose a colour.

    This shows RGB sliders and a colourspace that the user can pick colours from.

    This class is also a ChangeBroadcaster, so listeners can register to be told
    when the colour changes.

    @tags{GUI}
*/
class JUCE_API  ColourSelector  : public Component,
                                  public ChangeBroadcaster
{
public:
    //==============================================================================
    /** Options for the type of selector to show. These are passed into the constructor. */
    enum ColourSelectorOptions
    {
        showAlphaChannel    = 1 << 0,   /**< if set, the colour's alpha channel can be changed as well as its RGB. */

        showColourAtTop     = 1 << 1,   /**< if set, a swatch of the colour is shown at the top of the component. */
        editableColour      = 1 << 2,   /**< if set, the colour shows at the top of the component is editable. */
        showSliders         = 1 << 3,   /**< if set, RGB sliders are shown at the bottom of the component. */
        showColourspace     = 1 << 4    /**< if set, a big HSV selector is shown. */
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
    ~ColourSelector() override;

    //==============================================================================
    /** Returns the colour that the user has currently selected.

        The ColourSelector class is also a ChangeBroadcaster, so listeners can
        register to be told when the colour changes.

        @see setCurrentColour
    */
    Colour getCurrentColour() const;

    /** Changes the colour that is currently being shown.

        @param newColour           the new colour to show
        @param notificationType    whether to send a notification of the change to listeners.
                                   A notification will only be sent if the colour has changed.
    */
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
    virtual void setSwatchColour (int index, const Colour& newColour);


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
    class SwatchComponent;
    class ColourSpaceView;
    class HueSelectorComp;
    class ColourPreviewComp;

    Colour colour;
    float h, s, v;
    std::unique_ptr<Slider> sliders[4];
    std::unique_ptr<ColourSpaceView> colourSpace;
    std::unique_ptr<HueSelectorComp> hueSelector;
    std::unique_ptr<ColourPreviewComp> previewComponent;
    OwnedArray<SwatchComponent> swatchComponents;
    const int flags;
    int edgeGap;

    void setHue (float newH);
    void setSV (float newS, float newV);
    void updateHSV();
    void update (NotificationType);
    void changeColour();
    void paint (Graphics&) override;
    void resized() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ColourSelector)
};

} // namespace juce
