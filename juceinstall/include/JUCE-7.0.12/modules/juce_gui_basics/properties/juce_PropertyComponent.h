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
    A base class for a component that goes in a PropertyPanel and displays one of
    an item's properties.

    Subclasses of this are used to display a property in various forms, e.g. a
    ChoicePropertyComponent shows its value as a combo box; a SliderPropertyComponent
    shows its value as a slider; a TextPropertyComponent as a text box, etc.

    A subclass must implement the refresh() method which will be called to tell the
    component to update itself, and is also responsible for calling this it when the
    item that it refers to is changed.

    @see PropertyPanel, TextPropertyComponent, SliderPropertyComponent,
         ChoicePropertyComponent, ButtonPropertyComponent, BooleanPropertyComponent

    @tags{GUI}
*/
class JUCE_API  PropertyComponent  : public Component,
                                     public SettableTooltipClient
{
public:
    //==============================================================================
    /** Creates a PropertyComponent.

        @param propertyName     the name is stored as this component's name, and is
                                used as the name displayed next to this component in
                                a property panel
        @param preferredHeight  the height that the component should be given - some
                                items may need to be larger than a normal row height.
                                This value can also be set if a subclass changes the
                                preferredHeight member variable.
    */
    PropertyComponent (const String& propertyName,
                       int preferredHeight = 25);

    /** Destructor. */
    ~PropertyComponent() override;

    //==============================================================================
    /** Returns this item's preferred height.

        This value is specified either in the constructor or by a subclass changing the
        preferredHeight member variable.
    */
    int getPreferredHeight() const noexcept                 { return preferredHeight; }

    void setPreferredHeight (int newHeight) noexcept        { preferredHeight = newHeight; }

    //==============================================================================
    /** Updates the property component if the item it refers to has changed.

        A subclass must implement this method, and other objects may call it to
        force it to refresh itself.

        The subclass should be economical in the amount of work is done, so for
        example it should check whether it really needs to do a repaint rather than
        just doing one every time this method is called, as it may be called when
        the value being displayed hasn't actually changed.
    */
    virtual void refresh() = 0;


    /** The default paint method fills the background and draws a label for the
        item's name.

        @see LookAndFeel::drawPropertyComponentBackground(), LookAndFeel::drawPropertyComponentLabel()
    */
    void paint (Graphics&) override;

    /** The default resize method positions any child component to the right of this
        one, based on the look and feel's default label size.
    */
    void resized() override;

    /** By default, this just repaints the component. */
    void enablementChanged() override;

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the combo box.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        backgroundColourId     = 0x1008300,    /**< The background colour to fill the component with. */
        labelTextColourId      = 0x1008301,    /**< The colour for the property's label text. */
    };

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes. */
    struct JUCE_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        virtual void drawPropertyPanelSectionHeader (Graphics&, const String& name, bool isOpen, int width, int height) = 0;
        virtual void drawPropertyComponentBackground (Graphics&, int width, int height, PropertyComponent&) = 0;
        virtual void drawPropertyComponentLabel (Graphics&, int width, int height, PropertyComponent&) = 0;
        virtual Rectangle<int> getPropertyComponentContentPosition (PropertyComponent&) = 0;
        virtual int getPropertyPanelSectionHeaderHeight (const String& sectionTitle) = 0;
    };

protected:
    /** Used by the PropertyPanel to determine how high this component needs to be.
        A subclass can update this value in its constructor but shouldn't alter it later
        as changes won't necessarily be picked up.
    */
    int preferredHeight;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyComponent)
};

} // namespace juce
