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

#ifdef ADD_TO_LIST
  ADD_TO_LIST (LabelHandler);
#else

#include "../jucer_ComponentDocument.h"


//==============================================================================
class LabelHandler  : public ComponentTypeHelper<Label>
{
public:
    LabelHandler() : ComponentTypeHelper<Label> ("Label", "LABEL", "label")  {}
    ~LabelHandler()  {}

    Component* createComponent()                { return new Label (String::empty, String::empty); }
    const Rectangle<int> getDefaultSize()       { return Rectangle<int> (0, 0, 180, 24); }

    void update (ComponentDocument& document, Label* comp, const ValueTree& state)
    {
        comp->setText (state ["text"].toString(), false);
    }

    void initialiseNew (ComponentDocument& document, ValueTree& state)
    {
        state.setProperty ("text", "New Label", 0);
    }

    void createProperties (ComponentDocument& document, ValueTree& state, Array <PropertyComponent*>& props)
    {
        props.add (new TextPropertyComponent (getValue ("text", state, document), "Text", 16384, true));
        props.getLast()->setTooltip ("The label's text.");
    }
};

#endif
