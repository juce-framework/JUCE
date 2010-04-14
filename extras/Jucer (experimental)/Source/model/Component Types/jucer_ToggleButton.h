/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../jucer_ComponentDocument.h"


//==============================================================================
class ToggleButtonHandler  : public ComponentTypeHandler
{
public:
    ToggleButtonHandler() : ComponentTypeHandler ("ToggleButton", "TOGGLEBUTTON", "toggleButton")  {}
    ~ToggleButtonHandler()  {}

    Component* createComponent()                { return new ToggleButton (String::empty); }
    const Rectangle<int> getDefaultSize()       { return Rectangle<int> (0, 0, 180, 24); }

    void updateComponent (ComponentDocument& document, Component* comp, const ValueTree& state)
    {
        ToggleButton* tb = dynamic_cast <ToggleButton*> (comp);
        jassert (tb != 0);

        ComponentTypeHandler::updateComponent (document, comp, state);
        tb->setButtonText (state ["text"].toString());
    }

    void initialiseNewItem (ComponentDocument& document, ValueTree& state)
    {
        ComponentTypeHandler::initialiseNewItem (document, state);
        state.setProperty ("text", "New Toggle Button", 0);
    }

    void createPropertyEditors (ComponentDocument& document, ValueTree& state, Array <PropertyComponent*>& props)
    {
        ComponentTypeHandler::createPropertyEditors (document, state, props);
        props.add (new TextPropertyComponent (getValue ("text", state, document), "Button Text", 1024, false));
        props.getLast()->setTooltip ("The button's text.");
    }
};
