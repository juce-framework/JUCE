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
  ADD_TO_LIST (GroupComponentHandler);
#else

#include "../jucer_ComponentDocument.h"


//==============================================================================
class GroupComponentHandler  : public ComponentTypeHelper<GroupComponent>
{
public:
    GroupComponentHandler() : ComponentTypeHelper<GroupComponent> ("GroupComponent", "GroupComponent", "GROUPCOMPONENT", "group")
    {
        addEditableColour (GroupComponent::outlineColourId, "Outline", "outlineColour");
        addEditableColour (GroupComponent::textColourId, "Text Colour", "textColour");
    }

    ~GroupComponentHandler()  {}

    Component* createComponent()                { return new GroupComponent(); }
    const Rectangle<int> getDefaultSize()       { return Rectangle<int> (0, 0, 200, 200); }

    void initialiseNew (ComponentTypeInstance& item)
    {
        item.set ("text", "Group");
        item.set ("justification", (int) Justification::left);
    }

    void update (ComponentTypeInstance& item, GroupComponent* comp)
    {
        comp->setText (item ["text"].toString());
        comp->setTextLabelPosition ((int) item ["justification"]);
    }

    void createProperties (ComponentTypeInstance& item, Array <PropertyComponent*>& props)
    {
        item.addTooltipProperty (props);
        item.addFocusOrderProperty (props);

        props.add (new TextPropertyComponent (item.getValue ("text"), "Label", 512, false));
        props.getLast()->setTooltip ("The group's display name.");

        item.addJustificationProperty (props, "Text Position", item.getValue ("justification"), true);
        addEditableColourProperties (item, props);
    }

    void createCode (ComponentTypeInstance& item, CodeGenerator& code)
    {
        code.constructorCode << item.createConstructorStatement (String::empty);
    }
};

#endif
