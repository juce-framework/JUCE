/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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
    ComponentUndoableAction& operator= (const ComponentUndoableAction&);
};



#endif   // __JUCER_COMPONENTUNDOABLEACTION_JUCEHEADER__
