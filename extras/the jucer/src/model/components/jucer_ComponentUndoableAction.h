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
