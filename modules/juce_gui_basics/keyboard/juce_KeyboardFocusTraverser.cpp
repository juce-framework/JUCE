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

namespace juce
{

//==============================================================================
namespace KeyboardFocusTraverserHelpers
{
    static bool isKeyboardFocusable (const Component* comp, const Component* container)
    {
        return comp->getWantsKeyboardFocus() && container->isParentOf (comp);
    }

    static Component* traverse (Component* current, Component* container,
                                detail::FocusHelpers::NavigationDirection direction)
    {
        if (auto* comp = detail::FocusHelpers::navigateFocus (current, container, direction,
                                                              &Component::isKeyboardFocusContainer))
        {
            if (isKeyboardFocusable (comp, container))
                return comp;

            return traverse (comp, container, direction);
        }

        return nullptr;
    }
}

Component* KeyboardFocusTraverser::getNextComponent (Component* current)
{
    return KeyboardFocusTraverserHelpers::traverse (current, current->findKeyboardFocusContainer(),
                                                    detail::FocusHelpers::NavigationDirection::forwards);
}

Component* KeyboardFocusTraverser::getPreviousComponent (Component* current)
{
    return KeyboardFocusTraverserHelpers::traverse (current, current->findKeyboardFocusContainer(),
                                                    detail::FocusHelpers::NavigationDirection::backwards);
}

Component* KeyboardFocusTraverser::getDefaultComponent (Component* parentComponent)
{
    for (auto* comp : getAllComponents (parentComponent))
        if (KeyboardFocusTraverserHelpers::isKeyboardFocusable (comp, parentComponent))
            return comp;

    return nullptr;
}

std::vector<Component*> KeyboardFocusTraverser::getAllComponents (Component* parentComponent)
{
    std::vector<Component*> components;
    detail::FocusHelpers::findAllComponents (parentComponent,
                                             components,
                                             &Component::isKeyboardFocusContainer);

    auto removePredicate = [parentComponent] (const Component* comp)
    {
        return ! KeyboardFocusTraverserHelpers::isKeyboardFocusable (comp, parentComponent);
    };

    components.erase (std::remove_if (std::begin (components), std::end (components), std::move (removePredicate)),
                      std::end (components));

    return components;
}


//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

struct KeyboardFocusTraverserTests final : public UnitTest
{
    KeyboardFocusTraverserTests()
        : UnitTest ("KeyboardFocusTraverser", UnitTestCategories::gui)
    {}

    void runTest() override
    {
        ScopedJuceInitialiser_GUI libraryInitialiser;
        const MessageManagerLock mml;

        beginTest ("No child wants keyboard focus");
        {
            TestComponent parent;

            expect (traverser.getDefaultComponent (&parent) == nullptr);
            expect (traverser.getAllComponents (&parent).empty());
        }

        beginTest ("Single child wants keyboard focus");
        {
            TestComponent parent;

            parent.children[5].setWantsKeyboardFocus (true);

            auto* defaultComponent = traverser.getDefaultComponent (&parent);

            expect (defaultComponent == &parent.children[5]);
            expect (defaultComponent->getWantsKeyboardFocus());

            expect (traverser.getNextComponent (defaultComponent) == nullptr);
            expect (traverser.getPreviousComponent (defaultComponent) == nullptr);
            expect (traverser.getAllComponents (&parent).size() == 1);
        }

        beginTest ("Multiple children want keyboard focus");
        {
            TestComponent parent;

            Component* focusChildren[]
            {
                &parent.children[1],
                &parent.children[9],
                &parent.children[3],
                &parent.children[5],
                &parent.children[8],
                &parent.children[0]
            };

            for (auto* focusChild : focusChildren)
                focusChild->setWantsKeyboardFocus (true);

            auto allComponents = traverser.getAllComponents (&parent);

            for (auto* focusChild : focusChildren)
                expect (std::find (allComponents.cbegin(), allComponents.cend(), focusChild) != allComponents.cend());

            auto* componentToTest = traverser.getDefaultComponent (&parent);

            for (;;)
            {
                expect (componentToTest->getWantsKeyboardFocus());
                expect (std::find (std::begin (focusChildren), std::end (focusChildren), componentToTest) != std::end (focusChildren));

                componentToTest = traverser.getNextComponent (componentToTest);

                if (componentToTest == nullptr)
                    break;
            }

            int focusOrder = 1;
            for (auto* focusChild : focusChildren)
                focusChild->setExplicitFocusOrder (focusOrder++);

            componentToTest = traverser.getDefaultComponent (&parent);

            for (auto* focusChild : focusChildren)
            {
                expect (componentToTest == focusChild);
                expect (componentToTest->getWantsKeyboardFocus());

                componentToTest = traverser.getNextComponent (componentToTest);
            }
        }

        beginTest ("Single nested child wants keyboard focus");
        {
            TestComponent parent;
            Component grandparent;

            grandparent.addAndMakeVisible (parent);

            auto& focusChild = parent.children[5];

            focusChild.setWantsKeyboardFocus (true);

            expect (traverser.getDefaultComponent (&grandparent) == &focusChild);
            expect (traverser.getDefaultComponent (&parent) == &focusChild);
            expect (traverser.getNextComponent (&focusChild) == nullptr);
            expect (traverser.getPreviousComponent (&focusChild) == nullptr);
            expect (traverser.getAllComponents (&parent).size() == 1);
        }

        beginTest ("Multiple nested children want keyboard focus");
        {
            TestComponent parent;
            Component grandparent;

            grandparent.addAndMakeVisible (parent);

            Component* focusChildren[]
            {
                &parent.children[1],
                &parent.children[4],
                &parent.children[5]
            };

            for (auto* focusChild : focusChildren)
                focusChild->setWantsKeyboardFocus (true);

            auto allComponents = traverser.getAllComponents (&parent);

            expect (std::equal (allComponents.cbegin(), allComponents.cend(), focusChildren,
                                [] (const Component* c1, const Component* c2) { return c1 == c2; }));

            const auto front = *focusChildren;
            const auto back  = *std::prev (std::end (focusChildren));

            expect (traverser.getDefaultComponent (&grandparent) == front);
            expect (traverser.getDefaultComponent (&parent) == front);
            expect (traverser.getNextComponent (front) == *std::next (std::begin (focusChildren)));
            expect (traverser.getPreviousComponent (back) == *std::prev (std::end (focusChildren), 2));

            std::array<Component, 3> otherParents;

            for (auto& p : otherParents)
            {
                grandparent.addAndMakeVisible (p);
                p.setWantsKeyboardFocus (true);
            }

            expect (traverser.getDefaultComponent (&grandparent) == front);
            expect (traverser.getDefaultComponent (&parent) == front);
            expect (traverser.getNextComponent (back) == &otherParents.front());
            expect (traverser.getNextComponent (&otherParents.back()) == nullptr);
            expect (traverser.getAllComponents (&grandparent).size() == numElementsInArray (focusChildren) + otherParents.size());
            expect (traverser.getAllComponents (&parent).size() == (size_t) numElementsInArray (focusChildren));

            for (auto* focusChild : focusChildren)
                focusChild->setWantsKeyboardFocus (false);

            expect (traverser.getDefaultComponent (&grandparent) == &otherParents.front());
            expect (traverser.getDefaultComponent (&parent) == nullptr);
            expect (traverser.getAllComponents (&grandparent).size() == otherParents.size());
            expect (traverser.getAllComponents (&parent).empty());
        }
    }

private:
    struct TestComponent final : public Component
    {
        TestComponent()
        {
            for (auto& child : children)
                addAndMakeVisible (child);
        }

        std::array<Component, 10> children;
    };

    KeyboardFocusTraverser traverser;
};

static KeyboardFocusTraverserTests keyboardFocusTraverserTests;

#endif

} // namespace juce
