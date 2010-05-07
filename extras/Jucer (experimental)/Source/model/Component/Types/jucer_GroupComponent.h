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
    GroupComponentHandler() : ComponentTypeHelper<GroupComponent> ("GroupComponent", "GROUPCOMPONENT", "group")
    {
        addEditableColour (GroupComponent::outlineColourId, "Outline", "outlineColour");
        addEditableColour (GroupComponent::textColourId, "Text Colour", "textColour");
    }

    ~GroupComponentHandler()  {}

    Component* createComponent()                { return new GroupComponent (String::empty, String::empty); }
    const Rectangle<int> getDefaultSize()       { return Rectangle<int> (0, 0, 200, 200); }

    void update (ComponentDocument& document, GroupComponent* comp, const ValueTree& state)
    {
        comp->setText (state ["text"].toString());
    }

    void initialiseNew (ComponentDocument& document, ValueTree& state)
    {
        state.setProperty ("text", "Group", 0);
    }

    void createProperties (ComponentDocument& document, ValueTree& state, Array <PropertyComponent*>& props)
    {
        addTooltipProperty (document, state, props);
        addFocusOrderProperty (document, state, props);

        props.add (new TextPropertyComponent (getValue ("text", state, document), "Label", 512, false));
        props.getLast()->setTooltip ("The group's display name.");

        addEditableColourProperties (document, state, props);
    }
};

#endif
