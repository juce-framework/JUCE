/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../UI/jucer_JucerDocumentEditor.h"
#include "jucer_PaintElementGroup.h"

//==============================================================================
template <class ElementType>
class PaintElementUndoableAction    : public UndoableAction
{
public:
    JUCE_BEGIN_IGNORE_WARNINGS_MSVC (6011)

    PaintElementUndoableAction (ElementType* const element)
        : routine (*element->getOwner()),
          elementIndex (element->getOwner()->indexOfElement (element))
    {
        jassert (element != nullptr);

        if (element != nullptr && elementIndex < 0)
            findGroupIndices (element->getOwner(), element);

        jassert (elementIndex >= 0);
    }

    JUCE_END_IGNORE_WARNINGS_MSVC

    ElementType* getElement() const
    {
        if (containerGroups.size() > 0)
        {
            auto group = dynamic_cast<PaintElementGroup*> (routine.getElement (containerGroups.getFirst()));

            if (group == nullptr)
                return nullptr;

            for (int i = 1; i < containerGroups.size(); ++i)
            {
                group = dynamic_cast<PaintElementGroup*> (group->getElement (containerGroups.getUnchecked(i)));

                if (group == nullptr)
                    return nullptr;
            }

            auto e = dynamic_cast<ElementType*> (group->getElement (elementIndex));
            jassert (e != nullptr);
            return e;
        }
        else
        {
            auto e = dynamic_cast<ElementType*> (routine.getElement (elementIndex));
            jassert (e != nullptr);
            return e;
        }
    }

    int getSizeInUnits()    { return 2; }

protected:
    PaintRoutine& routine;
    int elementIndex;
    Array <int> containerGroups;

    void changed() const
    {
        jassert (routine.getDocument() != nullptr);
        routine.getDocument()->changed();
    }

    void showCorrectTab() const
    {
        if (JucerDocumentEditor* const docHolder = JucerDocumentEditor::getActiveDocumentHolder())
            docHolder->showGraphics (&routine);

        if (routine.getSelectedElements().getNumSelected() == 0)
            if (ElementType* const e = dynamic_cast<ElementType*> (routine.getElement (elementIndex)))
                routine.getSelectedElements().selectOnly (e);
    }

private:
    void findGroupIndices (PaintRoutine* const pr, PaintElement* const element)
    {
        for (int i = pr->getNumElements(); --i >= 0;)
        {
            if (auto pg = dynamic_cast<PaintElementGroup*> (pr->getElement (i)))
            {
                if (pg->containsElement (element))
                {
                    containerGroups.add (i);
                    findGroupIndices (pg, element);
                }
            }
        }
    }

    void findGroupIndices (PaintElementGroup* const group, PaintElement* const element)
    {
        elementIndex = group->indexOfElement (element);

        if (elementIndex < 0)
        {
            for (int i = group->getNumElements(); --i >= 0;)
            {
                if (auto pg = dynamic_cast<PaintElementGroup*> (group->getElement (i)))
                {
                    if (pg->containsElement (element))
                    {
                        containerGroups.add (i);
                        findGroupIndices (pg, element);
                    }
                }
            }
        }
    }

    PaintElementUndoableAction (const PaintElementUndoableAction&);
    PaintElementUndoableAction& operator= (const PaintElementUndoableAction&);
};
