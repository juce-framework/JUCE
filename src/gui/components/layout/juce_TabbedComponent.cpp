/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_TabbedComponent.h"
#include "../../graphics/geometry/juce_RectangleList.h"


//==============================================================================
class TabCompButtonBar  : public TabbedButtonBar
{
public:
    TabCompButtonBar (TabbedComponent* const owner_,
                      const TabbedButtonBar::Orientation orientation)
        : TabbedButtonBar (orientation),
          owner (owner_)
    {
    }

    ~TabCompButtonBar()
    {
    }

    void currentTabChanged (const int newCurrentTabIndex,
                            const String& newTabName)
    {
        owner->changeCallback (newCurrentTabIndex, newTabName);
    }

    void popupMenuClickOnTab (const int tabIndex,
                              const String& tabName)
    {
        owner->popupMenuClickOnTab (tabIndex, tabName);
    }

    const Colour getTabBackgroundColour (const int tabIndex)
    {
        return owner->tabs->getTabBackgroundColour (tabIndex);
    }

    TabBarButton* createTabButton (const String& tabName, const int tabIndex)
    {
        return owner->createTabButton (tabName, tabIndex);
    }

    juce_UseDebuggingNewOperator

private:
    TabbedComponent* const owner;

    TabCompButtonBar (const TabCompButtonBar&);
    const TabCompButtonBar& operator= (const TabCompButtonBar&);
};


TabbedComponent::TabbedComponent (const TabbedButtonBar::Orientation orientation)
    : panelComponent (0),
      tabDepth (30),
      outlineThickness (1),
      edgeIndent (0)
{
    addAndMakeVisible (tabs = new TabCompButtonBar (this, orientation));
}

TabbedComponent::~TabbedComponent()
{
    clearTabs();
    delete tabs;
}

//==============================================================================
void TabbedComponent::setOrientation (const TabbedButtonBar::Orientation orientation)
{
    tabs->setOrientation (orientation);
    resized();
}

TabbedButtonBar::Orientation TabbedComponent::getOrientation() const throw()
{
    return tabs->getOrientation();
}

void TabbedComponent::setTabBarDepth (const int newDepth)
{
    if (tabDepth != newDepth)
    {
        tabDepth = newDepth;
        resized();
    }
}

TabBarButton* TabbedComponent::createTabButton (const String& tabName, const int tabIndex)
{
    return new TabBarButton (tabName, tabs, tabIndex);
}

//==============================================================================
void TabbedComponent::clearTabs()
{
    if (panelComponent != 0)
    {
        panelComponent->setVisible (false);
        removeChildComponent (panelComponent);
        panelComponent = 0;
    }

    tabs->clearTabs();

    for (int i = contentComponents.size(); --i >= 0;)
    {
        Component* const c = contentComponents.getUnchecked(i);

        // be careful not to delete these components until they've been removed from the tab component
        jassert (c == 0 || c->isValidComponent());

        if (c != 0 && c->getComponentPropertyBool (T("deleteByTabComp_"), false, false))
            delete c;
    }

    contentComponents.clear();
}

void TabbedComponent::addTab (const String& tabName,
                              const Colour& tabBackgroundColour,
                              Component* const contentComponent,
                              const bool deleteComponentWhenNotNeeded,
                              const int insertIndex)
{
    contentComponents.insert (insertIndex, contentComponent);

    if (contentComponent != 0)
        contentComponent->setComponentProperty (T("deleteByTabComp_"), deleteComponentWhenNotNeeded);

    tabs->addTab (tabName, tabBackgroundColour, insertIndex);
}

void TabbedComponent::setTabName (const int tabIndex,
                                  const String& newName)
{
    tabs->setTabName (tabIndex, newName);
}

void TabbedComponent::removeTab (const int tabIndex)
{
    Component* const c = contentComponents [tabIndex];

    if (c != 0 && c->getComponentPropertyBool (T("deleteByTabComp_"), false, false))
    {
        if (c == panelComponent)
            panelComponent = 0;

        delete c;
    }

    contentComponents.remove (tabIndex);

    tabs->removeTab (tabIndex);
}

int TabbedComponent::getNumTabs() const
{
    return tabs->getNumTabs();
}

const StringArray TabbedComponent::getTabNames() const
{
    return tabs->getTabNames();
}

Component* TabbedComponent::getTabContentComponent (const int tabIndex) const throw()
{
    return contentComponents [tabIndex];
}

const Colour TabbedComponent::getTabBackgroundColour (const int tabIndex) const throw()
{
    return tabs->getTabBackgroundColour (tabIndex);
}

void TabbedComponent::setTabBackgroundColour (const int tabIndex, const Colour& newColour)
{
    tabs->setTabBackgroundColour (tabIndex, newColour);

    if (getCurrentTabIndex() == tabIndex)
        repaint();
}

void TabbedComponent::setCurrentTabIndex (const int newTabIndex, const bool sendChangeMessage)
{
    tabs->setCurrentTabIndex (newTabIndex, sendChangeMessage);
}

int TabbedComponent::getCurrentTabIndex() const
{
    return tabs->getCurrentTabIndex();
}

const String& TabbedComponent::getCurrentTabName() const
{
    return tabs->getCurrentTabName();
}

void TabbedComponent::setOutline (int thickness)
{
    outlineThickness = thickness;
    repaint();
}

void TabbedComponent::setIndent (const int indentThickness)
{
    edgeIndent = indentThickness;
}

void TabbedComponent::paint (Graphics& g)
{
    g.fillAll (findColour (backgroundColourId));

    const TabbedButtonBar::Orientation o = getOrientation();

    int x = 0;
    int y = 0;
    int r = getWidth();
    int b = getHeight();

    if (o == TabbedButtonBar::TabsAtTop)
        y += tabDepth;
    else if (o == TabbedButtonBar::TabsAtBottom)
        b -= tabDepth;
    else if (o == TabbedButtonBar::TabsAtLeft)
        x += tabDepth;
    else if (o == TabbedButtonBar::TabsAtRight)
        r -= tabDepth;

    g.reduceClipRegion (x, y, r - x, b - y);
    g.fillAll (tabs->getTabBackgroundColour (getCurrentTabIndex()));

    if (outlineThickness > 0)
    {
        if (o == TabbedButtonBar::TabsAtTop)
            --y;
        else if (o == TabbedButtonBar::TabsAtBottom)
            ++b;
        else if (o == TabbedButtonBar::TabsAtLeft)
            --x;
        else if (o == TabbedButtonBar::TabsAtRight)
            ++r;

        g.setColour (findColour (outlineColourId));
        g.drawRect (x, y, r - x, b - y, outlineThickness);
    }
}

void TabbedComponent::resized()
{
    const TabbedButtonBar::Orientation o = getOrientation();
    const int indent = edgeIndent + outlineThickness;
    BorderSize indents (indent);

    if (o == TabbedButtonBar::TabsAtTop)
    {
        tabs->setBounds (0, 0, getWidth(), tabDepth);
        indents.setTop (tabDepth + edgeIndent);
    }
    else if (o == TabbedButtonBar::TabsAtBottom)
    {
        tabs->setBounds (0, getHeight() - tabDepth, getWidth(), tabDepth);
        indents.setBottom (tabDepth + edgeIndent);
    }
    else if (o == TabbedButtonBar::TabsAtLeft)
    {
        tabs->setBounds (0, 0, tabDepth, getHeight());
        indents.setLeft (tabDepth + edgeIndent);
    }
    else if (o == TabbedButtonBar::TabsAtRight)
    {
        tabs->setBounds (getWidth() - tabDepth, 0, tabDepth, getHeight());
        indents.setRight (tabDepth + edgeIndent);
    }

    const Rectangle bounds (indents.subtractedFrom (Rectangle (0, 0, getWidth(), getHeight())));

    for (int i = contentComponents.size(); --i >= 0;)
        if (contentComponents.getUnchecked (i) != 0)
            contentComponents.getUnchecked (i)->setBounds (bounds);
}

void TabbedComponent::lookAndFeelChanged()
{
    for (int i = contentComponents.size(); --i >= 0;)
        if (contentComponents.getUnchecked (i) != 0)
            contentComponents.getUnchecked (i)->lookAndFeelChanged();
}

void TabbedComponent::changeCallback (const int newCurrentTabIndex,
                                      const String& newTabName)
{
    if (panelComponent != 0)
    {
        panelComponent->setVisible (false);
        removeChildComponent (panelComponent);
        panelComponent = 0;
    }

    if (getCurrentTabIndex() >= 0)
    {
        panelComponent = contentComponents [getCurrentTabIndex()];

        if (panelComponent != 0)
        {
            // do these ops as two stages instead of addAndMakeVisible() so that the
            // component has always got a parent when it gets the visibilityChanged() callback
            addChildComponent (panelComponent);
            panelComponent->setVisible (true);
            panelComponent->toFront (true);
        }

        repaint();
    }

    resized();

    currentTabChanged (newCurrentTabIndex, newTabName);
}

void TabbedComponent::currentTabChanged (const int, const String&)
{
}

void TabbedComponent::popupMenuClickOnTab (const int, const String&)
{
}

END_JUCE_NAMESPACE
