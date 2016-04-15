/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCER_PAINTELEMENTUNDOABLEACTION_H_INCLUDED
#define JUCER_PAINTELEMENTUNDOABLEACTION_H_INCLUDED

#include "../ui/jucer_JucerDocumentEditor.h"
#include "jucer_PaintElementGroup.h"


//==============================================================================
template <class ElementType>
class PaintElementUndoableAction    : public UndoableAction
{
public:
    PaintElementUndoableAction (ElementType* const element)
        : routine (*element->getOwner()),
          elementIndex (element->getOwner()->indexOfElement (element))
    {
        jassert (element != 0);

        if (elementIndex < 0)
            findGroupIndices (element->getOwner(), element);

        jassert (elementIndex >= 0);
    }

    ElementType* getElement() const
    {
        if (containerGroups.size() > 0)
        {
            PaintElementGroup* group = 0;
            group = dynamic_cast<PaintElementGroup*> (routine.getElement (containerGroups.getFirst()));

            if (group == 0)
                return 0;

            for (int i = 1; i < containerGroups.size(); ++i)
            {
                group = dynamic_cast<PaintElementGroup*> (group->getElement (containerGroups.getUnchecked(i)));

                if (group == 0)
                    return 0;
            }

            ElementType* const e = dynamic_cast<ElementType*> (group->getElement (elementIndex));
            jassert (e != 0);
            return e;
        }
        else
        {
            ElementType* const e = dynamic_cast<ElementType*> (routine.getElement (elementIndex));
            jassert (e != 0);
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
        jassert (routine.getDocument() != 0);
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
            PaintElementGroup* const pg = dynamic_cast<PaintElementGroup*> (pr->getElement (i));

            if (pg != 0 && pg->containsElement (element))
            {
                containerGroups.add (i);
                findGroupIndices (pg, element);
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
                PaintElementGroup* pg = dynamic_cast<PaintElementGroup*> (group->getElement (i));

                if (pg != 0 && pg->containsElement (element))
                {
                    containerGroups.add (i);
                    findGroupIndices (pg, element);
                }
            }
        }
    }

    PaintElementUndoableAction (const PaintElementUndoableAction&);
    PaintElementUndoableAction& operator= (const PaintElementUndoableAction&);
};



#endif   // JUCER_PAINTELEMENTUNDOABLEACTION_H_INCLUDED
