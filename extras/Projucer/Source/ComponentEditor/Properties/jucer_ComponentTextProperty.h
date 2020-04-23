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
class ComponentTextProperty  : public TextPropertyComponent,
                               private ChangeListener
{
public:
    ComponentTextProperty (const String& name,
                           const int maxNumChars_,
                           const bool isMultiLine_,
                           ComponentType* const comp,
                           JucerDocument& doc)
        : TextPropertyComponent (name, maxNumChars_, isMultiLine_),
          component (comp),
          document (doc)
    {
        document.addChangeListener (this);
    }

    ~ComponentTextProperty()
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
