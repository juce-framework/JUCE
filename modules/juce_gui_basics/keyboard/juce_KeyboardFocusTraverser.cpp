/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

namespace KeyboardFocusHelpers
{
    static int getOrder (const Component* c)
    {
        auto order = c->getExplicitFocusOrder();
        return order > 0 ? order : (std::numeric_limits<int>::max() / 2);
    }

    static void findAllFocusableComponents (Component* parent, Array<Component*>& comps)
    {
        if (parent->getNumChildComponents() != 0)
        {
            Array<Component*> localComps;

            for (auto* c : parent->getChildren())
                if (c->isVisible() && c->isEnabled())
                    localComps.add (c);

            // This will sort so that they are ordered in terms of left-to-right
            // and then top-to-bottom.
            std::stable_sort (localComps.begin(), localComps.end(),
                              [] (const Component* a, const Component* b)
            {
                auto explicitOrder1 = getOrder (a);
                auto explicitOrder2 = getOrder (b);

                if (explicitOrder1 != explicitOrder2)
                    return explicitOrder1 < explicitOrder2;

                if (a->getY() != b->getY())
                    return a->getY() < b->getY();

                return a->getX() < b->getX();
            });

            for (auto* c : localComps)
            {
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

    static Component* getIncrementedComponent (Component* current, int delta)
    {
        if (auto* focusContainer = findFocusContainer (current))
        {
            Array<Component*> comps;
            KeyboardFocusHelpers::findAllFocusableComponents (focusContainer, comps);

            if (! comps.isEmpty())
            {
                auto index = comps.indexOf (current);
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
    Array<Component*> comps;

    if (parentComponent != nullptr)
        KeyboardFocusHelpers::findAllFocusableComponents (parentComponent, comps);

    return comps.getFirst();
}

} // namespace juce
