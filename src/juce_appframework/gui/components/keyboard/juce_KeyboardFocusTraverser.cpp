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

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_KeyboardFocusTraverser.h"
#include "../juce_Component.h"


//==============================================================================
KeyboardFocusTraverser::KeyboardFocusTraverser()
{
}

KeyboardFocusTraverser::~KeyboardFocusTraverser()
{
}

//==============================================================================
// This will sort a set of components, so that they are ordered in terms of
// left-to-right and then top-to-bottom.
class ScreenPositionComparator
{
public:
    ScreenPositionComparator() {}

    static int compareElements (const Component* const first, const Component* const second) throw()
    {
        int explicitOrder1 = first->getExplicitFocusOrder();
        if (explicitOrder1 <= 0)
            explicitOrder1 = INT_MAX / 2;

        int explicitOrder2 = second->getExplicitFocusOrder();
        if (explicitOrder2 <= 0)
            explicitOrder2 = INT_MAX / 2;

        if (explicitOrder1 != explicitOrder2)
            return explicitOrder1 - explicitOrder2;

        const int diff = first->getY() - second->getY();

        return (diff == 0) ? first->getX() - second->getX()
                           : diff;
    }
};

static void findAllFocusableComponents (Component* const parent, Array <Component*>& comps)
{
    if (parent->getNumChildComponents() > 0)
    {
        Array <Component*> localComps;
        ScreenPositionComparator comparator;

        int i;
        for (i = parent->getNumChildComponents(); --i >= 0;)
        {
            Component* const c = parent->getChildComponent (i);

            if (c->isVisible() && c->isEnabled())
                localComps.addSorted (comparator, c);
        }

        for (i = 0; i < localComps.size(); ++i)
        {
            Component* const c = localComps.getUnchecked (i);

            if (c->getWantsKeyboardFocus())
                comps.add (c);

            if (! c->isFocusContainer())
                findAllFocusableComponents (c, comps);
        }
    }
}

static Component* getIncrementedComponent (Component* const current, const int delta) throw()
{
    Component* focusContainer = current->getParentComponent();

    if (focusContainer != 0)
    {
        while (focusContainer->getParentComponent() != 0 && ! focusContainer->isFocusContainer())
            focusContainer = focusContainer->getParentComponent();

        if (focusContainer != 0)
        {
            Array <Component*> comps;
            findAllFocusableComponents (focusContainer, comps);

            if (comps.size() > 0)
            {
                const int index = comps.indexOf (current);
                return comps [(index + comps.size() + delta) % comps.size()];
            }
        }
    }

    return 0;
}

Component* KeyboardFocusTraverser::getNextComponent (Component* current)
{
    return getIncrementedComponent (current, 1);
}

Component* KeyboardFocusTraverser::getPreviousComponent (Component* current)
{
    return getIncrementedComponent (current, -1);
}

Component* KeyboardFocusTraverser::getDefaultComponent (Component* parentComponent)
{
    Array <Component*> comps;

    if (parentComponent != 0)
        findAllFocusableComponents (parentComponent, comps);

    return comps.getFirst();
}


END_JUCE_NAMESPACE
