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

MultiDocumentPanelWindow::MultiDocumentPanelWindow (Colour backgroundColour)
    : DocumentWindow (String(), backgroundColour,
                      DocumentWindow::maximiseButton | DocumentWindow::closeButton, false)
{
}

MultiDocumentPanelWindow::~MultiDocumentPanelWindow()
{
}

//==============================================================================
void MultiDocumentPanelWindow::maximiseButtonPressed()
{
    if (auto* owner = getOwner())
        owner->setLayoutMode (MultiDocumentPanel::MaximisedWindowsWithTabs);
    else
        jassertfalse; // these windows are only designed to be used inside a MultiDocumentPanel!
}

void MultiDocumentPanelWindow::closeButtonPressed()
{
    if (auto* owner = getOwner())
        owner->closeDocumentAsync (getContentComponent(), true, nullptr);
    else
        jassertfalse; // these windows are only designed to be used inside a MultiDocumentPanel!
}

void MultiDocumentPanelWindow::activeWindowStatusChanged()
{
    DocumentWindow::activeWindowStatusChanged();
    updateActiveDocument();
}

void MultiDocumentPanelWindow::broughtToFront()
{
    DocumentWindow::broughtToFront();
    updateActiveDocument();
}

void MultiDocumentPanelWindow::updateActiveDocument()
{
    if (auto* owner = getOwner())
        owner->updateActiveDocumentFromUIState();
}

MultiDocumentPanel* MultiDocumentPanelWindow::getOwner() const noexcept
{
    return findParentComponentOfClass<MultiDocumentPanel>();
}

//==============================================================================
struct MultiDocumentPanel::TabbedComponentInternal   : public TabbedComponent
{
    TabbedComponentInternal() : TabbedComponent (TabbedButtonBar::TabsAtTop) {}

    void currentTabChanged (int, const String&) override
    {
        if (auto* owner = findParentComponentOfClass<MultiDocumentPanel>())
            owner->updateActiveDocumentFromUIState();
    }
};


//==============================================================================
MultiDocumentPanel::MultiDocumentPanel()
{
    setOpaque (true);
}

MultiDocumentPanel::~MultiDocumentPanel()
{
    for (int i = components.size(); --i >= 0;)
        if (auto* component = components[i])
            closeDocumentInternal (component);
}

//==============================================================================
namespace MultiDocHelpers
{
    static bool shouldDeleteComp (Component* const c)
    {
        return c->getProperties() ["mdiDocumentDelete_"];
    }
}

#if JUCE_MODAL_LOOPS_PERMITTED
bool MultiDocumentPanel::closeAllDocuments (const bool checkItsOkToCloseFirst)
{
    while (! components.isEmpty())
        if (! closeDocument (components.getLast(), checkItsOkToCloseFirst))
            return false;

    return true;
}
#endif

void MultiDocumentPanel::closeLastDocumentRecursive (SafePointer<MultiDocumentPanel> parent,
                                                     bool checkItsOkToCloseFirst,
                                                     std::function<void (bool)> callback)
{
    if (parent->components.isEmpty())
    {
        NullCheckedInvocation::invoke (callback, true);
        return;
    }

    parent->closeDocumentAsync (parent->components.getLast(),
                                checkItsOkToCloseFirst,
                                [parent, checkItsOkToCloseFirst, callback] (bool closeResult)
    {
        if (parent == nullptr)
            return;

        if (! closeResult)
        {
            NullCheckedInvocation::invoke (callback, false);
            return;
        }

        parent->closeLastDocumentRecursive (parent, checkItsOkToCloseFirst, std::move (callback));
    });
}

void MultiDocumentPanel::closeAllDocumentsAsync (bool checkItsOkToCloseFirst, std::function<void (bool)> callback)
{
    closeLastDocumentRecursive (this, checkItsOkToCloseFirst, std::move (callback));
}

#if JUCE_MODAL_LOOPS_PERMITTED
bool MultiDocumentPanel::tryToCloseDocument (Component*)
{
    // If you hit this assertion then you need to implement this method in a subclass.
    jassertfalse;
    return false;
}
#endif

MultiDocumentPanelWindow* MultiDocumentPanel::createNewDocumentWindow()
{
    return new MultiDocumentPanelWindow (backgroundColour);
}

void MultiDocumentPanel::addWindow (Component* component)
{
    auto* dw = createNewDocumentWindow();

    dw->setResizable (true, false);
    dw->setContentNonOwned (component, true);
    dw->setName (component->getName());

    auto bkg = component->getProperties() ["mdiDocumentBkg_"];
    dw->setBackgroundColour (bkg.isVoid() ? backgroundColour : Colour ((uint32) static_cast<int> (bkg)));

    int x = 4;

    if (auto* topComp = getChildren().getLast())
        if (topComp->getX() == x && topComp->getY() == x)
            x += 16;

    dw->setTopLeftPosition (x, x);

    auto pos = component->getProperties() ["mdiDocumentPos_"];
    if (pos.toString().isNotEmpty())
        dw->restoreWindowStateFromString (pos.toString());

    addAndMakeVisible (dw);
    dw->toFront (true);
}

bool MultiDocumentPanel::addDocument (Component* const component,
                                      Colour docColour,
                                      const bool deleteWhenRemoved)
{
    // If you try passing a full DocumentWindow or ResizableWindow in here, you'll end up
    // with a frame-within-a-frame! Just pass in the bare content component.
    jassert (dynamic_cast<ResizableWindow*> (component) == nullptr);

    if (component == nullptr || (maximumNumDocuments > 0 && components.size() >= maximumNumDocuments))
        return false;

    components.add (component);
    component->getProperties().set ("mdiDocumentDelete_", deleteWhenRemoved);
    component->getProperties().set ("mdiDocumentBkg_", (int) docColour.getARGB());
    component->addComponentListener (this);

    if (mode == FloatingWindows)
    {
        if (isFullscreenWhenOneDocument())
        {
            if (components.size() == 1)
            {
                addAndMakeVisible (component);
            }
            else
            {
                if (components.size() == 2)
                    addWindow (components.getFirst());

                addWindow (component);
            }
        }
        else
        {
           addWindow (component);
        }
    }
    else
    {
        if (tabComponent == nullptr && components.size() > numDocsBeforeTabsUsed)
        {
            tabComponent.reset (new TabbedComponentInternal());
            addAndMakeVisible (tabComponent.get());

            auto temp = components;

            for (auto& c : temp)
                tabComponent->addTab (c->getName(), docColour, c, false);

            resized();
        }
        else
        {
            if (tabComponent != nullptr)
                tabComponent->addTab (component->getName(), docColour, component, false);
            else
                addAndMakeVisible (component);
        }

        setActiveDocument (component);
    }

    resized();
    updateActiveDocument (component);
    return true;
}

void MultiDocumentPanel::recreateLayout()
{
    tabComponent.reset();

    for (int i = getNumChildComponents(); --i >= 0;)
    {
        std::unique_ptr<MultiDocumentPanelWindow> dw (dynamic_cast<MultiDocumentPanelWindow*> (getChildComponent (i)));

        if (dw != nullptr)
        {
            dw->getContentComponent()->getProperties().set ("mdiDocumentPos_", dw->getWindowStateAsString());
            dw->clearContentComponent();
        }
    }

    resized();

    auto tempComps = components;
    components.clear();

    {
        // We want to preserve the activeComponent, so we are blocking the changes originating
        // from addDocument()
        const ScopedValueSetter<bool> scope { isLayoutBeingChanged, true };

        for (auto* c : tempComps)
            addDocument (c,
                         Colour ((uint32) static_cast<int> (c->getProperties().getWithDefault ("mdiDocumentBkg_",
                                                                                               (int) Colours::white.getARGB()))),
                         MultiDocHelpers::shouldDeleteComp (c));
    }

    if (activeComponent != nullptr)
        setActiveDocument (activeComponent);

    updateActiveDocumentFromUIState();
}

void MultiDocumentPanel::closeDocumentInternal (Component* componentToClose)
{
    // Intellisense warns about component being uninitialised.
    // I'm not sure how a function argument could be uninitialised.
    JUCE_BEGIN_IGNORE_WARNINGS_MSVC (6001)

    const OptionalScopedPointer<Component> component { componentToClose,
                                                       MultiDocHelpers::shouldDeleteComp (componentToClose) };

    component->removeComponentListener (this);

    component->getProperties().remove ("mdiDocumentDelete_");
    component->getProperties().remove ("mdiDocumentBkg_");

    const auto removedIndex = components.indexOf (component);

    if (removedIndex < 0)
    {
        jassertfalse;
        return;
    }

    components.remove (removedIndex);

    // See if the active document needs to change because of closing a document. It should only
    // change if we closed the active document. If so, the next active document should be the
    // subsequent one.
    if (component == activeComponent)
    {
        auto* newActiveComponent = components[jmin (removedIndex, components.size() - 1)];
        updateActiveDocument (newActiveComponent);
    }

    // We update the UI to reflect the new state, but we want to prevent the UI state callback
    // to change the active document.
    const ScopedValueSetter<bool> scope { isLayoutBeingChanged, true };

    if (mode == FloatingWindows)
    {
        for (auto* child : getChildren())
        {
            if (auto* dw = dynamic_cast<MultiDocumentPanelWindow*> (child))
            {
                if (dw->getContentComponent() == component)
                {
                    std::unique_ptr<MultiDocumentPanelWindow> (dw)->clearContentComponent();
                    break;
                }
            }
        }

        if (isFullscreenWhenOneDocument() && components.size() == 1)
        {
            for (int i = getNumChildComponents(); --i >= 0;)
            {
                std::unique_ptr<MultiDocumentPanelWindow> dw (dynamic_cast<MultiDocumentPanelWindow*> (getChildComponent (i)));

                if (dw != nullptr)
                    dw->clearContentComponent();
            }

            addAndMakeVisible (getActiveDocument());
        }
    }
    else
    {
        if (tabComponent != nullptr)
        {
            for (int i = tabComponent->getNumTabs(); --i >= 0;)
                if (tabComponent->getTabContentComponent (i) == component)
                    tabComponent->removeTab (i);
        }
        else
        {
            removeChildComponent (component);
        }

        if (components.size() <= numDocsBeforeTabsUsed && getActiveDocument() != nullptr)
        {
            tabComponent.reset();
            addAndMakeVisible (getActiveDocument());
        }
    }

    resized();

    // This ensures that the active tab is painted properly when a tab is closed!
    if (auto* activeDocument = getActiveDocument())
        setActiveDocument (activeDocument);

    JUCE_END_IGNORE_WARNINGS_MSVC
}

#if JUCE_MODAL_LOOPS_PERMITTED
bool MultiDocumentPanel::closeDocument (Component* component,
                                        const bool checkItsOkToCloseFirst)
{
    // Intellisense warns about component being uninitialised.
    // I'm not sure how a function argument could be uninitialised.
    JUCE_BEGIN_IGNORE_WARNINGS_MSVC (6001)

    if (component == nullptr)
        return true;

    if (components.contains (component))
    {
        if (checkItsOkToCloseFirst && ! tryToCloseDocument (component))
            return false;

        closeDocumentInternal (component);
    }
    else
    {
        jassertfalse;
    }

    return true;

    JUCE_END_IGNORE_WARNINGS_MSVC
}
#endif

void MultiDocumentPanel::closeDocumentAsync (Component* component,
                                             const bool checkItsOkToCloseFirst,
                                             std::function<void (bool)> callback)
{
    // Intellisense warns about component being uninitialised.
    // I'm not sure how a function argument could be uninitialised.
    JUCE_BEGIN_IGNORE_WARNINGS_MSVC (6001)

    if (component == nullptr)
    {
        NullCheckedInvocation::invoke (callback, true);
        return;
    }

    if (components.contains (component))
    {
        if (checkItsOkToCloseFirst)
        {
            tryToCloseDocumentAsync (component,
                                     [parent = SafePointer<MultiDocumentPanel> { this }, component, callback] (bool closedSuccessfully)
            {
                if (parent == nullptr)
                    return;

                if (closedSuccessfully)
                    parent->closeDocumentInternal (component);

                NullCheckedInvocation::invoke (callback, closedSuccessfully);
            });

            return;
        }

        closeDocumentInternal (component);
    }
    else
    {
        jassertfalse;
    }

    NullCheckedInvocation::invoke (callback, true);

    JUCE_END_IGNORE_WARNINGS_MSVC
}

int MultiDocumentPanel::getNumDocuments() const noexcept
{
    return components.size();
}

Component* MultiDocumentPanel::getDocument (const int index) const noexcept
{
    return components [index];
}

Component* MultiDocumentPanel::getActiveDocument() const noexcept
{
    return activeComponent;
}

void MultiDocumentPanel::setActiveDocument (Component* component)
{
    jassert (component != nullptr);

    if (mode == FloatingWindows)
    {
        component = getContainerComp (component);

        if (component != nullptr)
            component->toFront (true);
    }
    else if (tabComponent != nullptr)
    {
        jassert (components.indexOf (component) >= 0);

        for (int i = tabComponent->getNumTabs(); --i >= 0;)
        {
            if (tabComponent->getTabContentComponent (i) == component)
            {
                tabComponent->setCurrentTabIndex (i);
                break;
            }
        }
    }
    else
    {
        component->grabKeyboardFocus();
    }
}

void MultiDocumentPanel::activeDocumentChanged()
{
}

void MultiDocumentPanel::setMaximumNumDocuments (const int newNumber)
{
    maximumNumDocuments = newNumber;
}

void MultiDocumentPanel::useFullscreenWhenOneDocument (const bool shouldUseTabs)
{
    const auto newNumDocsBeforeTabsUsed = shouldUseTabs ? 1 : 0;

    if (std::exchange (numDocsBeforeTabsUsed, newNumDocsBeforeTabsUsed) != newNumDocsBeforeTabsUsed)
        recreateLayout();
}

bool MultiDocumentPanel::isFullscreenWhenOneDocument() const noexcept
{
    return numDocsBeforeTabsUsed != 0;
}

//==============================================================================
void MultiDocumentPanel::setLayoutMode (const LayoutMode newLayoutMode)
{
    if (std::exchange (mode, newLayoutMode) != newLayoutMode)
        recreateLayout();
}

void MultiDocumentPanel::setBackgroundColour (Colour newBackgroundColour)
{
    if (backgroundColour != newBackgroundColour)
    {
        backgroundColour = newBackgroundColour;
        setOpaque (newBackgroundColour.isOpaque());
        repaint();
    }
}

//==============================================================================
void MultiDocumentPanel::paint (Graphics& g)
{
    g.fillAll (backgroundColour);
}

void MultiDocumentPanel::resized()
{
    if (mode == MaximisedWindowsWithTabs || components.size() == numDocsBeforeTabsUsed)
    {
        for (auto* child : getChildren())
            child->setBounds (getLocalBounds());
    }

    setWantsKeyboardFocus (components.size() == 0);
}

Component* MultiDocumentPanel::getContainerComp (Component* c) const
{
    if (mode == FloatingWindows)
    {
        for (auto* child : getChildren())
            if (auto* dw = dynamic_cast<MultiDocumentPanelWindow*> (child))
                if (dw->getContentComponent() == c)
                    return dw;
    }

    return c;
}

void MultiDocumentPanel::componentNameChanged (Component&)
{
    if (mode == FloatingWindows)
    {
        for (auto* child : getChildren())
            if (auto* dw = dynamic_cast<MultiDocumentPanelWindow*> (child))
                dw->setName (dw->getContentComponent()->getName());
    }
    else if (tabComponent != nullptr)
    {
        for (int i = tabComponent->getNumTabs(); --i >= 0;)
            tabComponent->setTabName (i, tabComponent->getTabContentComponent (i)->getName());
    }
}

void MultiDocumentPanel::updateActiveDocumentFromUIState()
{
    auto* newActiveComponent = [&]() -> Component*
    {
        if (mode == FloatingWindows)
        {
            for (auto* c : components)
            {
                if (auto* window = static_cast<MultiDocumentPanelWindow*> (c->getParentComponent()))
                    if (window->isActiveWindow())
                        return c;
            }
        }

        if (tabComponent != nullptr)
            if (auto* current = tabComponent->getCurrentContentComponent())
                return current;

        return activeComponent;
    }();

    updateActiveDocument (newActiveComponent);
}

void MultiDocumentPanel::updateActiveDocument (Component* component)
{
    if (isLayoutBeingChanged)
        return;

    if (std::exchange (activeComponent, component) != component)
        activeDocumentChanged();
}

} // namespace juce
