/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

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
struct TabbedComponent::ButtonBar final : public TabbedButtonBar
{
    ButtonBar (TabbedComponent& tabComp, TabbedButtonBar::Orientation o)
        : TabbedButtonBar (o), owner (tabComp)
    {
    }

    void currentTabChanged (int newCurrentTabIndex, const String& newTabName) override
    {
        owner.changeCallback (newCurrentTabIndex, newTabName);
    }

    void popupMenuClickOnTab (int tabIndex, const String& tabName) override
    {
        owner.popupMenuClickOnTab (tabIndex, tabName);
    }

    Colour getTabBackgroundColour (int tabIndex)
    {
        return owner.tabs->getTabBackgroundColour (tabIndex);
    }

    TabBarButton* createTabButton (const String& tabName, int tabIndex) override
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

//==============================================================================
std::unique_ptr<AccessibilityHandler> TabbedComponent::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::group);
}

} // namespace juce
