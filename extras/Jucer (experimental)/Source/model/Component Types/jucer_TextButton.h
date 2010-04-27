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
    TextButtonHandler() : ComponentTypeHelper<TextButton> ("TextButton", "TEXTBUTTON", "textButton")  {}
    ~TextButtonHandler()  {}

    Component* createComponent()                { return new TextButton (String::empty); }
    const Rectangle<int> getDefaultSize()       { return Rectangle<int> (0, 0, 150, 24); }

    void update (ComponentDocument& document, TextButton* comp, const ValueTree& state)
    {
        comp->setButtonText (state ["text"].toString());
    }

    void initialiseNew (ComponentDocument& document, ValueTree& state)
    {
        state.setProperty ("text", "New Button", 0);
    }

    void createProperties (ComponentDocument& document, ValueTree& state, Array <PropertyComponent*>& props)
    {
        props.add (new TextPropertyComponent (getValue ("text", state, document), "Button Text", 1024, false));
        props.getLast()->setTooltip ("The button's text.");
    }
};

#endif
