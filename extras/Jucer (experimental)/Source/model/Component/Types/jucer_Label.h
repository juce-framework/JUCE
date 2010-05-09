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
  ADD_TO_LIST (LabelHandler);
#else

#include "../jucer_ComponentDocument.h"


//==============================================================================
class LabelHandler  : public ComponentTypeHelper<Label>
{
public:
    LabelHandler() : ComponentTypeHelper<Label> ("Label", "LABEL", "label")
    {
        addEditableColour (Label::backgroundColourId, "Background", "backgroundColour");
        addEditableColour (Label::textColourId, "Text Colour", "textColour");
        addEditableColour (Label::outlineColourId, "Outline Colour", "outlineColour");
    }

    ~LabelHandler()  {}

    Component* createComponent()                { return new Label (String::empty, String::empty); }
    const Rectangle<int> getDefaultSize()       { return Rectangle<int> (0, 0, 180, 24); }

    void update (ComponentDocument& document, Label* comp, const ValueTree& state)
    {
        comp->setText (state ["text"].toString(), false);
        comp->setFont (getFontFromState (state, "fontName", "fontSize", "fontStyle"));
        int editMode = (int) state ["editMode"];
        comp->setEditable (editMode == 2, editMode == 3, false);
        comp->setJustificationType ((int) state ["justification"]);
    }

    void initialiseNew (ComponentDocument& document, ValueTree& state)
    {
        state.setProperty ("text", "New Label", 0);
        state.setProperty ("fontSize", 14, 0);
        state.setProperty ("editMode", 1, 0);
        state.setProperty ("justification", (int) Justification::centredLeft, 0);
    }

    void createProperties (ComponentDocument& document, ValueTree& state, Array <PropertyComponent*>& props)
    {
        addTooltipProperty (document, state, props);
        addFocusOrderProperty (document, state, props);

        props.add (new TextPropertyComponent (getValue ("text", state, document), "Text", 16384, true));
        props.getLast()->setTooltip ("The label's text.");

        props.add (createJustificationProperty ("Layout", state.getPropertyAsValue ("justification", document.getUndoManager()), false));

        const char* const editModes[] = { "Read-only", "Edit on Single-Click", "Edit on Double-Click", 0 };
        props.add (new ChoicePropertyComponent (state.getPropertyAsValue ("editMode", document.getUndoManager()),
                                                "Edit Mode", StringArray (editModes)));

        createFontProperties (props, state, "fontName", "fontSize", "fontStyle", document.getUndoManager());

        addEditableColourProperties (document, state, props);
    }
};

#endif
