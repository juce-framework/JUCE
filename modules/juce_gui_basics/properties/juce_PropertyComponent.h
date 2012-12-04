/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCE_PROPERTYCOMPONENT_JUCEHEADER__
#define __JUCE_PROPERTYCOMPONENT_JUCEHEADER__

class EditableProperty;

#include "../components/juce_Component.h"
#include "../mouse/juce_TooltipClient.h"

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
    ~PropertyComponent();

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
    void paint (Graphics& g);

    /** The default resize method positions any child component to the right of this
        one, based on the look and feel's default label size.
    */
    void resized();

    /** By default, this just repaints the component. */
    void enablementChanged();

protected:
    /** Used by the PropertyPanel to determine how high this component needs to be.
        A subclass can update this value in its constructor but shouldn't alter it later
        as changes won't necessarily be picked up.
    */
    int preferredHeight;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyComponent)
};


#endif   // __JUCE_PROPERTYCOMPONENT_JUCEHEADER__
