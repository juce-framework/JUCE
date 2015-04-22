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

namespace KeyboardFocusHelpers
{
    // This will sort a set of components, so that they are ordered in terms of
    // left-to-right and then top-to-bottom.
    struct ScreenPositionComparator
    {
        static int compareElements (const Component* const first, const Component* const second)
        {
            const int explicitOrder1 = getOrder (first);
            const int explicitOrder2 = getOrder (second);

            if (explicitOrder1 != explicitOrder2)
                return explicitOrder1 - explicitOrder2;

            const int yDiff = first->getY() - second->getY();

            return yDiff == 0 ? first->getX() - second->getX()
                              : yDiff;
        }

        static int getOrder (const Component* const c)
        {
            const int order = c->getExplicitFocusOrder();
            return order > 0 ? order : (std::numeric_limits<int>::max() / 2);
        }
    };

    static void findAllFocusableComponents (Component* const parent, Array <Component*>& comps)
    {
        if (parent->getNumChildComponents() > 0)
        {
            Array <Component*> localComps;
            ScreenPositionComparator comparator;

            for (int i = parent->getNumChildComponents(); --i >= 0;)
            {
                Component* const c = parent->getChildComponent (i);

                if (c->isVisible() && c->isEnabled())
                    localComps.addSorted (comparator, c);
            }

            for (int i = 0; i < localComps.size(); ++i)
            {
                Component* const c = localComps.getUnchecked (i);

                if (c->getWantsKeyboardFocus())
                    comps.add (c);

                if (! c->isFocusContainer())
                    findAllFocusableComponents (c, comps);
            }
        }
    }

    static Component* findFocusContainer (Component* c)
    {
        c = c->getParentComponent();

        if (c != nullptr)
            while (c->getParentComponent() != nullptr && ! c->isFocusContainer())
                c = c->getParentComponent();

        return c;
    }

    static Component* getIncrementedComponent (Component* const current, const int delta)
    {
        Component* focusContainer = findFocusContainer (current);

        if (focusContainer != nullptr)
        {
            Array <Component*> comps;
            KeyboardFocusHelpers::findAllFocusableComponents (focusContainer, comps);

            if (comps.size() > 0)
            {
                const int index = comps.indexOf (current);
                return comps [(index + comps.size() + delta) % comps.size()];
            }
        }

        return nullptr;
    }
}

//==============================================================================
KeyboardFocusTraverser::KeyboardFocusTraverser() {}
KeyboardFocusTraverser::~KeyboardFocusTraverser() {}

Component* KeyboardFocusTraverser::getNextComponent (Component* current)
{
    jassert (current != nullptr);
    return KeyboardFocusHelpers::getIncrementedComponent (current, 1);
}

Component* KeyboardFocusTraverser::getPreviousComponent (Component* current)
{
    jassert (current != nullptr);
    return KeyboardFocusHelpers::getIncrementedComponent (current, -1);
}

Component* KeyboardFocusTraverser::getDefaultComponent (Component* parentComponent)
{
    Array <Component*> comps;

    if (parentComponent != nullptr)
        KeyboardFocusHelpers::findAllFocusableComponents (parentComponent, comps);

    return comps.getFirst();
}
