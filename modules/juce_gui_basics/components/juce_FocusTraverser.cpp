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
Component* FocusTraverser::getNextComponent (Component* current)
{
    jassert (current != nullptr);

    return detail::FocusHelpers::navigateFocus (current,
                                                current->findFocusContainer(),
                                                detail::FocusHelpers::NavigationDirection::forwards,
                                                &Component::isFocusContainer);
}

Component* FocusTraverser::getPreviousComponent (Component* current)
{
    jassert (current != nullptr);

    return detail::FocusHelpers::navigateFocus (current,
                                                current->findFocusContainer(),
                                                detail::FocusHelpers::NavigationDirection::backwards,
                                                &Component::isFocusContainer);
}

Component* FocusTraverser::getDefaultComponent (Component* parentComponent)
{
    if (parentComponent != nullptr)
    {
        std::vector<Component*> components;
        detail::FocusHelpers::findAllComponents (parentComponent,
                                                 components,
                                                 &Component::isFocusContainer);

        if (! components.empty())
            return components.front();
    }

    return nullptr;
}

std::vector<Component*> FocusTraverser::getAllComponents (Component* parentComponent)
{
    std::vector<Component*> components;
    detail::FocusHelpers::findAllComponents (parentComponent,
                                             components,
                                             &Component::isFocusContainer);

    return components;
}

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

struct FocusTraverserTests final : public UnitTest
{
    FocusTraverserTests()
        : UnitTest ("FocusTraverser", UnitTestCategories::gui)
    {}

    void runTest() override
    {
        ScopedJuceInitialiser_GUI libraryInitialiser;
        const MessageManagerLock mml;

        beginTest ("Basic traversal");
        {
            TestComponent parent;

            expect (traverser.getDefaultComponent (&parent) == &parent.children.front());

            for (auto iter = parent.children.begin(); iter != parent.children.end(); ++iter)
                expect (traverser.getNextComponent (&(*iter)) == (iter == std::prev (parent.children.cend()) ? nullptr
                                                                                                             : &(*std::next (iter))));

            for (auto iter = parent.children.rbegin(); iter != parent.children.rend(); ++iter)
                expect (traverser.getPreviousComponent (&(*iter)) == (iter == std::prev (parent.children.rend()) ? nullptr
                                                                                                                 : &(*std::next (iter))));
            auto allComponents = traverser.getAllComponents (&parent);

            expect (std::equal (allComponents.cbegin(), allComponents.cend(), parent.children.cbegin(),
                                [] (const Component* c1, const Component& c2) { return c1 == &c2; }));
        }

        beginTest ("Disabled components are ignored");
        {
            checkIgnored ([] (Component& c) { c.setEnabled (false); });
        }

        beginTest ("Invisible components are ignored");
        {
            checkIgnored ([] (Component& c) { c.setVisible (false); });
        }

        beginTest ("Explicit focus order comes before unspecified");
        {
            TestComponent parent;

            auto& explicitFocusComponent = parent.children[2];

            explicitFocusComponent.setExplicitFocusOrder (1);
            expect (traverser.getDefaultComponent (&parent) == &explicitFocusComponent);

            expect (traverser.getAllComponents (&parent).front() == &explicitFocusComponent);
        }

        beginTest ("Explicit focus order comparison");
        {
            checkComponentProperties ([this] (Component& child) { child.setExplicitFocusOrder (getRandom().nextInt ({ 1, 100 })); },
                                      [] (const Component& c1, const Component& c2) { return c1.getExplicitFocusOrder()
                                                                                               <= c2.getExplicitFocusOrder(); });
        }

        beginTest ("Left to right");
        {
            checkComponentProperties ([this] (Component& child) { child.setTopLeftPosition (getRandom().nextInt ({ 0, 100 }), 0); },
                                      [] (const Component& c1, const Component& c2) { return c1.getX() <= c2.getX(); });
        }

        beginTest ("Top to bottom");
        {
            checkComponentProperties ([this] (Component& child) { child.setTopLeftPosition (0, getRandom().nextInt ({ 0, 100 })); },
                                      [] (const Component& c1, const Component& c2) { return c1.getY() <= c2.getY(); });
        }

        beginTest ("Focus containers have their own focus");
        {
            Component root;

            TestComponent container;
            container.setFocusContainerType (Component::FocusContainerType::focusContainer);

            root.addAndMakeVisible (container);

            expect (traverser.getDefaultComponent (&root) == &container);
            expect (traverser.getNextComponent (&container) == nullptr);
            expect (traverser.getPreviousComponent (&container) == nullptr);

            expect (traverser.getDefaultComponent (&container) == &container.children.front());

            for (auto iter = container.children.begin(); iter != container.children.end(); ++iter)
                expect (traverser.getNextComponent (&(*iter)) == (iter == std::prev (container.children.cend()) ? nullptr
                                                                                                                : &(*std::next (iter))));

            for (auto iter = container.children.rbegin(); iter != container.children.rend(); ++iter)
                expect (traverser.getPreviousComponent (&(*iter)) == (iter == std::prev (container.children.rend()) ? nullptr
                                                                                                                    : &(*std::next (iter))));

            expect (traverser.getAllComponents (&root).size() == 1);

            auto allContainerComponents = traverser.getAllComponents (&container);

            expect (std::equal (allContainerComponents.cbegin(), allContainerComponents.cend(), container.children.cbegin(),
                                [] (const Component* c1, const Component& c2) { return c1 == &c2; }));
        }

        beginTest ("Non-focus containers pass-through focus");
        {
            Component root;

            TestComponent container;
            container.setFocusContainerType (Component::FocusContainerType::none);

            root.addAndMakeVisible (container);

            expect (traverser.getDefaultComponent (&root) == &container);
            expect (traverser.getNextComponent (&container) == &container.children.front());
            expect (traverser.getPreviousComponent (&container) == nullptr);

            expect (traverser.getDefaultComponent (&container) == &container.children.front());

            for (auto iter = container.children.begin(); iter != container.children.end(); ++iter)
                expect (traverser.getNextComponent (&(*iter)) == (iter == std::prev (container.children.cend()) ? nullptr
                                                                                                                : &(*std::next (iter))));

            for (auto iter = container.children.rbegin(); iter != container.children.rend(); ++iter)
                expect (traverser.getPreviousComponent (&(*iter)) == (iter == std::prev (container.children.rend()) ? &container
                                                                                                                    : &(*std::next (iter))));

            expect (traverser.getAllComponents (&root).size() == container.children.size() + 1);
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

    void checkComponentProperties (std::function<void (Component&)>&& childFn,
                                   std::function<bool (const Component&, const Component&)>&& testProperty)
    {
        TestComponent parent;

        for (auto& child : parent.children)
            childFn (child);

        auto* comp = traverser.getDefaultComponent (&parent);

        for (const auto& child : parent.children)
            if (&child != comp)
                expect (testProperty (*comp, child));

        for (;;)
        {
            auto* next = traverser.getNextComponent (comp);

            if (next == nullptr)
                break;

            expect (testProperty (*comp, *next));
            comp = next;
        }
    }

    void checkIgnored (const std::function<void(Component&)>& makeIgnored)
    {
        TestComponent parent;

        auto iter = parent.children.begin();

        makeIgnored (*iter);
        expect (traverser.getDefaultComponent (&parent) == std::addressof (*std::next (iter)));

        iter += 5;
        makeIgnored (*iter);
        expect (traverser.getNextComponent (std::addressof (*std::prev (iter))) == std::addressof (*std::next (iter)));
        expect (traverser.getPreviousComponent (std::addressof (*std::next (iter))) == std::addressof (*std::prev (iter)));

        auto allComponents = traverser.getAllComponents (&parent);

        expect (std::find (allComponents.cbegin(), allComponents.cend(), &parent.children.front()) == allComponents.cend());
        expect (std::find (allComponents.cbegin(), allComponents.cend(), std::addressof (*iter)) == allComponents.cend());
    }

    FocusTraverser traverser;
};

static FocusTraverserTests focusTraverserTests;

#endif

} // namespace juce
