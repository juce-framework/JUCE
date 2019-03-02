/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

namespace TabbedComponentHelpers
{
    const Identifier deleteComponentId ("deleteByTabComp_");

    static void deleteIfNecessary (Component* comp)
    {
        if (comp != nullptr && (bool) comp->getProperties() [deleteComponentId])
            delete comp;
    }

    static Rectangle<int> getTabArea (Rectangle<int>& content, BorderSize<int>& outline,
                                      TabbedButtonBar::Orientation orientation, int tabDepth)
    {
        switch (orientation)
        {
            case TabbedButtonBar::TabsAtTop:    outline.setTop (0);     return content.removeFromTop (tabDepth);
            case TabbedButtonBar::TabsAtBottom: outline.setBottom (0);  return content.removeFromBottom (tabDepth);
            case TabbedButtonBar::TabsAtLeft:   outline.setLeft (0);    return content.removeFromLeft (tabDepth);
            case TabbedButtonBar::TabsAtRight:  outline.setRight (0);   return content.removeFromRight (tabDepth);
            default: jassertfalse; break;
        }

        return Rectangle<int>();
    }
}

//==============================================================================
struct TabbedComponent::ButtonBar  : public TabbedButtonBar
{
    ButtonBar (TabbedComponent& tabComp, TabbedButtonBar::Orientation o)
        : TabbedButtonBar (o), owner (tabComp)
    {
    }

    void currentTabChanged (int newCurrentTabIndex, const String& newTabName)
    {
        owner.changeCallback (newCurrentTabIndex, newTabName);
    }

    void popupMenuClickOnTab (int tabIndex, const String& tabName)
    {
        owner.popupMenuClickOnTab (tabIndex, tabName);
    }

    Colour getTabBackgroundColour (int tabIndex)
    {
        return owner.tabs->getTabBackgroundColour (tabIndex);
    }

    TabBarButton* createTabButton (const String& tabName, int tabIndex)
    {
        return owner.createTabButton (tabName, tabIndex);
    }

    TabbedComponent& owner;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ButtonBar)
};


//==============================================================================
TabbedComponent::TabbedComponent (TabbedButtonBar::Orientation orientation)
{
    tabs.reset (new ButtonBar (*this, orientation));
    addAndMakeVisible (tabs.get());
}

TabbedComponent::~TabbedComponent()
{
    clearTabs();
    tabs.reset();
}

//==============================================================================
void TabbedComponent::setOrientation (TabbedButtonBar::Orientation orientation)
{
    tabs->setOrientation (orientation);
    resized();
}

TabbedButtonBar::Orientation TabbedComponent::getOrientation() const noexcept
{
    return tabs->getOrientation();
}

void TabbedComponent::setTabBarDepth (int newDepth)
{
    if (tabDepth != newDepth)
    {
        tabDepth = newDepth;
        resized();
    }
}

TabBarButton* TabbedComponent::createTabButton (const String& tabName, int /*tabIndex*/)
{
    return new TabBarButton (tabName, *tabs);
}

//==============================================================================
void TabbedComponent::clearTabs()
{
    if (panelComponent != nullptr)
    {
        panelComponent->setVisible (false);
        removeChildComponent (panelComponent.get());
        panelComponent = nullptr;
    }

    tabs->clearTabs();

    for (int i = contentComponents.size(); --i >= 0;)
        TabbedComponentHelpers::deleteIfNecessary (contentComponents.getReference (i));

    contentComponents.clear();
}

void TabbedComponent::addTab (const String& tabName,
                              Colour tabBackgroundColour,
                              Component* contentComponent,
                              bool deleteComponentWhenNotNeeded,
                              int insertIndex)
{
    contentComponents.insert (insertIndex, WeakReference<Component> (contentComponent));

    if (deleteComponentWhenNotNeeded && contentComponent != nullptr)
        contentComponent->getProperties().set (TabbedComponentHelpers::deleteComponentId, true);

    tabs->addTab (tabName, tabBackgroundColour, insertIndex);
    resized();
}

void TabbedComponent::setTabName (int tabIndex, const String& newName)
{
    tabs->setTabName (tabIndex, newName);
}

void TabbedComponent::removeTab (int tabIndex)
{
    if (isPositiveAndBelow (tabIndex, contentComponents.size()))
    {
        TabbedComponentHelpers::deleteIfNecessary (contentComponents.getReference (tabIndex).get());
        contentComponents.remove (tabIndex);
        tabs->removeTab (tabIndex);
    }
}

void TabbedComponent::moveTab (int currentIndex, int newIndex, bool animate)
{
    contentComponents.move (currentIndex, newIndex);
    tabs->moveTab (currentIndex, newIndex, animate);
}

int TabbedComponent::getNumTabs() const
{
    return tabs->getNumTabs();
}

StringArray TabbedComponent::getTabNames() const
{
    return tabs->getTabNames();
}

Component* TabbedComponent::getTabContentComponent (int tabIndex) const noexcept
{
    return contentComponents[tabIndex].get();
}

Colour TabbedComponent::getTabBackgroundColour (int tabIndex) const noexcept
{
    return tabs->getTabBackgroundColour (tabIndex);
}

void TabbedComponent::setTabBackgroundColour (int tabIndex, Colour newColour)
{
    tabs->setTabBackgroundColour (tabIndex, newColour);

    if (getCurrentTabIndex() == tabIndex)
        repaint();
}

void TabbedComponent::setCurrentTabIndex (int newTabIndex, bool sendChangeMessage)
{
    tabs->setCurrentTabIndex (newTabIndex, sendChangeMessage);
}

int TabbedComponent::getCurrentTabIndex() const
{
    return tabs->getCurrentTabIndex();
}

String TabbedComponent::getCurrentTabName() const
{
    return tabs->getCurrentTabName();
}

void TabbedComponent::setOutline (int thickness)
{
    outlineThickness = thickness;
    resized();
    repaint();
}

void TabbedComponent::setIndent (int indentThickness)
{
    edgeIndent = indentThickness;
    resized();
    repaint();
}

void TabbedComponent::paint (Graphics& g)
{
    g.fillAll (findColour (backgroundColourId));

    auto content = getLocalBounds();
    BorderSize<int> outline (outlineThickness);
    TabbedComponentHelpers::getTabArea (content, outline, getOrientation(), tabDepth);

    g.reduceClipRegion (content);
    g.fillAll (tabs->getTabBackgroundColour (getCurrentTabIndex()));

    if (outlineThickness > 0)
    {
        RectangleList<int> rl (content);
        rl.subtract (outline.subtractedFrom (content));

        g.reduceClipRegion (rl);
        g.fillAll (findColour (outlineColourId));
    }
}

void TabbedComponent::resized()
{
    auto content = getLocalBounds();
    BorderSize<int> outline (outlineThickness);

    tabs->setBounds (TabbedComponentHelpers::getTabArea (content, outline, getOrientation(), tabDepth));
    content = BorderSize<int> (edgeIndent).subtractedFrom (outline.subtractedFrom (content));

    for (auto& c : contentComponents)
        if (auto comp = c.get())
            comp->setBounds (content);
}

void TabbedComponent::lookAndFeelChanged()
{
    for (auto& c : contentComponents)
        if (auto comp = c.get())
          comp->lookAndFeelChanged();
}

void TabbedComponent::changeCallback (int newCurrentTabIndex, const String& newTabName)
{
    auto* newPanelComp = getTabContentComponent (getCurrentTabIndex());

    if (newPanelComp != panelComponent)
    {
        if (panelComponent != nullptr)
        {
            panelComponent->setVisible (false);
            removeChildComponent (panelComponent);
        }

        panelComponent = newPanelComp;

        if (panelComponent != nullptr)
        {
            // do these ops as two stages instead of addAndMakeVisible() so that the
            // component has always got a parent when it gets the visibilityChanged() callback
            addChildComponent (panelComponent);
            panelComponent->sendLookAndFeelChange();
            panelComponent->setVisible (true);
            panelComponent->toFront (true);
        }

        repaint();
    }

    resized();
    currentTabChanged (newCurrentTabIndex, newTabName);
}

void TabbedComponent::currentTabChanged (int, const String&) {}
void TabbedComponent::popupMenuClickOnTab (int, const String&) {}

} // namespace juce
