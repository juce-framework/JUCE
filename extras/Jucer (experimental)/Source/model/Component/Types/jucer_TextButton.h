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
  ADD_TO_LIST (TextButtonHandler);
#else

#include "../jucer_ComponentDocument.h"


//==============================================================================
class TextButtonHandler  : public ComponentTypeHelper<TextButton>
{
public:
    TextButtonHandler() : ComponentTypeHelper<TextButton> ("TextButton", "TEXTBUTTON", "textButton")
    {
        addEditableColour (TextButton::buttonColourId, "Background", "backgroundColour");
        addEditableColour (TextButton::textColourOffId, "Text Colour", "textColour");
    }

    ~TextButtonHandler()  {}

    Component* createComponent()                { return new TextButton (String::empty); }
    const Rectangle<int> getDefaultSize()       { return Rectangle<int> (0, 0, 150, 24); }

    void initialiseNew (ComponentDocument& document, ValueTree& state)
    {
        state.setProperty ("text", "New Button", 0);
        state.setProperty ("radioGroup", 0, 0);
        state.setProperty ("connectedLeft", false, 0);
        state.setProperty ("connectedRight", false, 0);
        state.setProperty ("connectedTop", false, 0);
        state.setProperty ("connectedBottom", false, 0);
    }

    void update (ComponentDocument& document, TextButton* comp, const ValueTree& state)
    {
        comp->setButtonText (state ["text"].toString());
        comp->setRadioGroupId (state ["radioGroup"]);

        int connected = 0;
        if (state ["connectedLeft"])    connected |= TextButton::ConnectedOnLeft;
        if (state ["connectedRight"])   connected |= TextButton::ConnectedOnRight;
        if (state ["connectedTop"])     connected |= TextButton::ConnectedOnTop;
        if (state ["connectedBottom"])  connected |= TextButton::ConnectedOnBottom;

        comp->setConnectedEdges (connected);
    }

    void createProperties (ComponentDocument& document, ValueTree& state, Array <PropertyComponent*>& props)
    {
        addTooltipProperty (document, state, props);
        addFocusOrderProperty (document, state, props);

        props.add (new TextPropertyComponent (getValue ("text", state, document), "Button Text", 1024, false));
        props.getLast()->setTooltip ("The button's text.");

        props.add (new TextPropertyComponent (Value (new NumericValueSource<int> (getValue ("radioGroup", state, document))), "Radio Group", 8, false));
        props.getLast()->setTooltip ("The radio group that this button is a member of.");

        props.add (new BooleanPropertyComponent (getValue ("connectedLeft", state, document), "Connected left", "Connected"));
        props.add (new BooleanPropertyComponent (getValue ("connectedRight", state, document), "Connected right", "Connected"));
        props.add (new BooleanPropertyComponent (getValue ("connectedTop", state, document), "Connected top", "Connected"));
        props.add (new BooleanPropertyComponent (getValue ("connectedBottom", state, document), "Connected bottom", "Connected"));

        addEditableColourProperties (document, state, props);
    }
};


#endif
