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
  ADD_TO_LIST (TabbedComponentHandler);
#else

#include "../jucer_ComponentDocument.h"


//==============================================================================
class TabbedComponentHandler  : public ComponentTypeHelper<TabbedComponent>
{
public:
    TabbedComponentHandler() : ComponentTypeHelper<TabbedComponent> ("TabbedComponent", "TABBEDCOMPONENT", "tabbedComponent")
    {
        addEditableColour (TabbedComponent::backgroundColourId, "Background", "backgroundColour");
        addEditableColour (TabbedComponent::outlineColourId, "Outline", "outlineColour");
    }

    ~TabbedComponentHandler()  {}

    Component* createComponent()                { return new TabbedComponent (TabbedButtonBar::TabsAtTop); }
    const Rectangle<int> getDefaultSize()       { return Rectangle<int> (0, 0, 300, 200); }

    void update (ComponentDocument& document, TabbedComponent* comp, const ValueTree& state)
    {
    }

    void initialiseNew (ComponentDocument& document, ValueTree& state)
    {
    }

    void createProperties (ComponentDocument& document, ValueTree& state, Array <PropertyComponent*>& props)
    {
        addEditableColourProperties (document, state, props);
    }
};

#endif
