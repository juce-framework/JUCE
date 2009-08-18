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

#ifndef __JUCER_COMPONENTUNDOABLEACTION_JUCEHEADER__
#define __JUCER_COMPONENTUNDOABLEACTION_JUCEHEADER__

#include "../../ui/jucer_JucerDocumentHolder.h"


//==============================================================================
/**
*/
template <class ComponentType>
class ComponentUndoableAction    : public UndoableAction
{
public:
    ComponentUndoableAction (ComponentType* const comp,
                             ComponentLayout& layout_)
        : layout (layout_),
          componentIndex (layout_.indexOfComponent (comp))
    {
        jassert (comp != 0);
        jassert (componentIndex >= 0);
    }

    ~ComponentUndoableAction()
    {
    }

    ComponentType* getComponent() const
    {
        ComponentType* const c = dynamic_cast <ComponentType*> (layout.getComponent (componentIndex));
        jassert (c != 0);
        return c;
    }

    int getSizeInUnits()    { return 2; }

protected:
    ComponentLayout& layout;
    const int componentIndex;

    void changed() const
    {
        jassert (layout.getDocument() != 0);
        layout.getDocument()->changed();
    }

    void showCorrectTab() const
    {
        JucerDocumentHolder* const docHolder = JucerDocumentHolder::getActiveDocumentHolder();

        if (docHolder != 0)
            docHolder->showLayout();

        if (layout.getSelectedSet().getNumSelected() == 0)
        {
            ComponentType* const c = dynamic_cast <ComponentType*> (layout.getComponent (componentIndex));

            if (c != 0)
                layout.getSelectedSet().selectOnly (getComponent());
        }
    }

private:
    ComponentUndoableAction (const ComponentUndoableAction&);
    const ComponentUndoableAction& operator= (const ComponentUndoableAction&);
};



#endif   // __JUCER_COMPONENTUNDOABLEACTION_JUCEHEADER__
