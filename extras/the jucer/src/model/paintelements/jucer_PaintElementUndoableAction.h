/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCER_PAINTELEMENTUNDOABLEACTION_JUCEHEADER__
#define __JUCER_PAINTELEMENTUNDOABLEACTION_JUCEHEADER__

#include "../../ui/jucer_JucerDocumentHolder.h"
#include "jucer_PaintElementGroup.h"


//==============================================================================
/**
*/
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

    ~PaintElementUndoableAction()
    {
    }

    ElementType* getElement() const
    {
        if (containerGroups.size() > 0)
        {
            PaintElementGroup* group = 0;
            group = dynamic_cast <PaintElementGroup*> (routine.getElement (containerGroups.getFirst()));

            if (group == 0)
                return 0;

            for (int i = 1; i < containerGroups.size(); ++i)
            {
                group = dynamic_cast <PaintElementGroup*> (group->getElement (containerGroups.getUnchecked(i)));

                if (group == 0)
                    return 0;
            }

            ElementType* const e = dynamic_cast <ElementType*> (group->getElement (elementIndex));
            jassert (e != 0);
            return e;
        }
        else
        {
            ElementType* const e = dynamic_cast <ElementType*> (routine.getElement (elementIndex));
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
        JucerDocumentHolder* const docHolder = JucerDocumentHolder::getActiveDocumentHolder();

        if (docHolder != 0)
            docHolder->showGraphics (&routine);

        if (routine.getSelectedElements().getNumSelected() == 0)
        {
            ElementType* const e = dynamic_cast <ElementType*> (routine.getElement (elementIndex));

            if (e != 0)
                routine.getSelectedElements().selectOnly (e);
        }
    }

private:
    void findGroupIndices (PaintRoutine* const routine, PaintElement* const element)
    {
        for (int i = routine->getNumElements(); --i >= 0;)
        {
            PaintElementGroup* const pg = dynamic_cast <PaintElementGroup*> (routine->getElement (i));

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
                PaintElementGroup* pg = dynamic_cast <PaintElementGroup*> (group->getElement (i));

                if (pg != 0 && pg->containsElement (element))
                {
                    containerGroups.add (i);
                    findGroupIndices (pg, element);
                }
            }
        }
    }

    PaintElementUndoableAction (const PaintElementUndoableAction&);
    const PaintElementUndoableAction& operator= (const PaintElementUndoableAction&);
};



#endif   // __JUCER_PAINTELEMENTUNDOABLEACTION_JUCEHEADER__
