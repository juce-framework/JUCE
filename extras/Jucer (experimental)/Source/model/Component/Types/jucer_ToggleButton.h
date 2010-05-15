/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#ifdef ADD_TO_LIST
  ADD_TO_LIST (ToggleButtonHandler);
#else

#include "../jucer_ComponentDocument.h"


//==============================================================================
class ToggleButtonHandler  : public ComponentTypeHelper<ToggleButton>
{
public:
    ToggleButtonHandler() : ComponentTypeHelper<ToggleButton> ("ToggleButton", "ToggleButton", "TOGGLEBUTTON", "toggleButton")
    {
        addEditableColour (ToggleButton::textColourId, "Text Colour", "textColour");
    }

    ~ToggleButtonHandler()  {}

    Component* createComponent()                { return new ToggleButton(); }
    const Rectangle<int> getDefaultSize()       { return Rectangle<int> (0, 0, 180, 24); }

    void initialiseNew (ComponentTypeInstance& item)
    {
        item.set (Ids::text, "New Toggle Button");
        item.set (Ids::initialState, false);
    }

    void update (ComponentTypeInstance& item, ToggleButton* comp)
    {
        comp->setButtonText (item [Ids::text].toString());
        comp->setToggleState (item [Ids::initialState], false);
    }

    void createProperties (ComponentTypeInstance& item, Array <PropertyComponent*>& props)
    {
        item.addTooltipProperty (props);
        item.addFocusOrderProperty (props);

        props.add (new TextPropertyComponent (item.getValue (Ids::text), "Button Text", 1024, false));
        props.getLast()->setTooltip ("The button's text.");

        props.add (new BooleanPropertyComponent (item.getValue (Ids::initialState), "Initial State", "Enabled initially"));

        addEditableColourProperties (item, props);
    }

    void createCode (ComponentTypeInstance& item, CodeGenerator& code)
    {
        code.constructorCode += item.createConstructorStatement (String::empty);
    }
};

#endif
