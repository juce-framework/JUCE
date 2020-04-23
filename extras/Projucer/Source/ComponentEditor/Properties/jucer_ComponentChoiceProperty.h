/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
template <class ComponentType>
class ComponentChoiceProperty  : public ChoicePropertyComponent,
                                 private ChangeListener
{
public:
    ComponentChoiceProperty (const String& name,
                             ComponentType* comp,
                             JucerDocument& doc)
        : ChoicePropertyComponent (name),
          component (comp),
          document (doc)
    {
        document.addChangeListener (this);
    }

    ~ComponentChoiceProperty()
    {
        document.removeChangeListener (this);
    }

    void changeListenerCallback (ChangeBroadcaster*)
    {
        refresh();
    }

protected:
    ComponentType* component;
    JucerDocument& document;
};
