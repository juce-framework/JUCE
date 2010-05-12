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
    ComboBoxHandler() : ComponentTypeHelper<ComboBox> ("ComboBox", "ComboBox", "COMBOBOX", "comboBox")
    {
        addEditableColour (ComboBox::backgroundColourId, "Background", "backgroundColour");
        addEditableColour (ComboBox::textColourId, "Text", "textColour");
        addEditableColour (ComboBox::outlineColourId, "Outline", "outlineColour");
        addEditableColour (ComboBox::buttonColourId, "Button", "buttonColour");
        addEditableColour (ComboBox::arrowColourId, "Arrow", "arrowColour");
    }

    ~ComboBoxHandler()  {}

    Component* createComponent()                { return new ComboBox(); }
    const Rectangle<int> getDefaultSize()       { return Rectangle<int> (0, 0, 180, 24); }

    void initialiseNew (ComponentTypeInstance& item)
    {
        item.set ("items", "Item 1\nItem 2");
        item.set ("editable", false);
        item.set ("textJustification", (int) Justification::centredLeft);
        item.set ("unselectedText", "");
        item.set ("noItemsText", "(No Choices)");
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

    void update (ComponentTypeInstance& item, ComboBox* comp)
    {
        updateItems (comp, item ["items"]);
        comp->setEditableText (item ["editable"]);
        comp->setJustificationType ((int) item ["textJustification"]);
        comp->setTextWhenNothingSelected (item ["unselectedText"].toString());
        comp->setTextWhenNoChoicesAvailable (item ["noItemsText"].toString());
    }

    void createProperties (ComponentTypeInstance& item, Array <PropertyComponent*>& props)
    {
        item.addTooltipProperty (props);
        item.addFocusOrderProperty (props);

        props.add (new TextPropertyComponent (item.getValue ("items"), "Items", 16384, true));
        props.getLast()->setTooltip ("A list of items to use to initialise the ComboBox");

        props.add (new BooleanPropertyComponent (item.getValue ("editable"), "Editable", "Text is editable"));
        item.addJustificationProperty (props, "Text Position", item.getValue ("textJustification"), false);
        props.add (new TextPropertyComponent (item.getValue ("unselectedText"), "Text when none selected", 512, false));
        props.add (new TextPropertyComponent (item.getValue ("noItemsText"), "Text when no items", 512, false));

        addEditableColourProperties (item, props);
    }

    void createCode (ComponentTypeInstance& item, CodeGenerator& code)
    {
        code.constructorCode << item.createConstructorStatement (String::empty);
    }
};

#endif
