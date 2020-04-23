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

#include "../UI/jucer_JucerDocumentEditor.h"

//==============================================================================
template <class ComponentType>
class ComponentUndoableAction    : public UndoableAction
{
public:
    ComponentUndoableAction (ComponentType* const comp,
                             ComponentLayout& layout_)
        : layout (layout_),
          componentIndex (layout_.indexOfComponent (comp))
    {
        jassert (comp != nullptr);
        jassert (componentIndex >= 0);
    }

    ComponentType* getComponent() const
    {
        ComponentType* const c = dynamic_cast<ComponentType*> (layout.getComponent (componentIndex));
        jassert (c != nullptr);
        return c;
    }

    int getSizeInUnits()    { return 2; }

protected:
    ComponentLayout& layout;
    const int componentIndex;

    void changed() const
    {
        jassert (layout.getDocument() != nullptr);
        layout.getDocument()->changed();
    }

    void showCorrectTab() const
    {
        if (JucerDocumentEditor* const ed = JucerDocumentEditor::getActiveDocumentHolder())
            ed->showLayout();

        if (layout.getSelectedSet().getNumSelected() == 0)
            if (ComponentType* const c = dynamic_cast<ComponentType*> (layout.getComponent (componentIndex)))
                layout.getSelectedSet().selectOnly (getComponent());
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentUndoableAction)
};
