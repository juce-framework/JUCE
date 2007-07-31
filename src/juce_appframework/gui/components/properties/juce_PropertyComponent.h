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

#ifndef __JUCE_PROPERTYCOMPONENT_JUCEHEADER__
#define __JUCE_PROPERTYCOMPONENT_JUCEHEADER__

class EditableProperty;

#include "../juce_Component.h"


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
class JUCE_API  PropertyComponent  : public Component
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
                       const int preferredHeight = 25);

    /** Destructor. */
    ~PropertyComponent();

    //==============================================================================
    /** Returns this item's preferred height.

        This value is specified either in the constructor or by a subclass changing the
        preferredHeight member variable.
    */
    int getPreferredHeight() const throw()                  { return preferredHeight; }


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

    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    /** Used by the PropertyPanel to determine how high this component needs to be.

        A subclass can update this value in its constructor but shouldn't alter it later
        as changes won't necessarily be picked up.
    */
    int preferredHeight;
};


#endif   // __JUCE_PROPERTYCOMPONENT_JUCEHEADER__
