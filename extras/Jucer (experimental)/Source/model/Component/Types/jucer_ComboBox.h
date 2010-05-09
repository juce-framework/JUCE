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
  ADD_TO_LIST (ComboBoxHandler);
#else

#include "../jucer_ComponentDocument.h"


//==============================================================================
class ComboBoxHandler  : public ComponentTypeHelper<ComboBox>
{
public:
    ComboBoxHandler() : ComponentTypeHelper<ComboBox> ("ComboBox", "COMBOBOX", "comboBox")
    {
        addEditableColour (ComboBox::backgroundColourId, "Background", "backgroundColour");
        addEditableColour (ComboBox::textColourId, "Text", "textColour");
        addEditableColour (ComboBox::outlineColourId, "Outline", "outlineColour");
        addEditableColour (ComboBox::buttonColourId, "Button", "buttonColour");
        addEditableColour (ComboBox::arrowColourId, "Arrow", "arrowColour");
    }

    ~ComboBoxHandler()  {}

    Component* createComponent()                { return new ComboBox (String::empty); }
    const Rectangle<int> getDefaultSize()       { return Rectangle<int> (0, 0, 180, 24); }

    void initialiseNew (ComponentDocument& document, ValueTree& state)
    {
        state.setProperty ("items", "Item 1\nItem 2", 0);
        state.setProperty ("editable", false, 0);
        state.setProperty ("textJustification", (int) Justification::centredLeft, 0);
        state.setProperty ("unselectedText", "", 0);
        state.setProperty ("noItemsText", "(No Choices)", 0);
    }

    void updateItems (ComboBox* comp, const String& itemString)
    {
        StringArray items;
        items.addLines (itemString);
        items.removeEmptyStrings (true);

        StringArray existingItems;

        for (int i = 0; i < comp->getNumItems(); ++i)
            existingItems.add (comp->getItemText (i));

        if (existingItems != items)
        {
            comp->clear();

            for (int i = 0; i < items.size(); ++i)
                comp->addItem (items[i], i + 1);
        }
    }

    void update (ComponentDocument& document, ComboBox* comp, const ValueTree& state)
    {
        updateItems (comp, state ["items"]);
        comp->setEditableText (state ["editable"]);
        comp->setJustificationType ((int) state ["textJustification"]);
        comp->setTextWhenNothingSelected (state ["unselectedText"].toString());
        comp->setTextWhenNoChoicesAvailable (state ["noItemsText"].toString());
    }

    void createProperties (ComponentDocument& document, ValueTree& state, Array <PropertyComponent*>& props)
    {
        addTooltipProperty (document, state, props);
        addFocusOrderProperty (document, state, props);

        props.add (new TextPropertyComponent (getValue ("items", state, document), "Items", 16384, true));
        props.getLast()->setTooltip ("A list of items to use to initialise the ComboBox");

        props.add (new BooleanPropertyComponent (getValue ("editable", state, document), "Editable", "Text is editable"));
        props.add (createJustificationProperty ("Text Position", getValue ("textJustification", state, document), false));
        props.add (new TextPropertyComponent (getValue ("unselectedText", state, document), "Text when none selected", 512, false));
        props.add (new TextPropertyComponent (getValue ("noItemsText", state, document), "Text when no items", 512, false));

        addEditableColourProperties (document, state, props);
    }
};

#endif
