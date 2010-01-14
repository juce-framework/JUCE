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
#include "../menus/juce_PopupMenu.h"
#include "../lookandfeel/juce_LookAndFeel.h"


//==============================================================================
TabBarButton::TabBarButton (const String& name,
                            TabbedButtonBar* const owner_,
                            const int index)
    : Button (name),
      owner (owner_),
      tabIndex (index),
      overlapPixels (0)
{
    shadow.setShadowProperties (2.2f, 0.7f, 0, 0);
    setComponentEffect (&shadow);
    setWantsKeyboardFocus (false);
}

TabBarButton::~TabBarButton()
{
}

void TabBarButton::paintButton (Graphics& g,
                                bool isMouseOverButton,
                                bool isButtonDown)
{
    int x, y, w, h;
    getActiveArea (x, y, w, h);

    g.setOrigin (x, y);

    getLookAndFeel()
        .drawTabButton (g, w, h,
                        owner->getTabBackgroundColour (tabIndex),
                        tabIndex, getButtonText(), *this,
                        owner->getOrientation(),
                        isMouseOverButton, isButtonDown,
                        getToggleState());
}

void TabBarButton::clicked (const ModifierKeys& mods)
{
    if (mods.isPopupMenu())
        owner->popupMenuClickOnTab (tabIndex, getButtonText());
    else
        owner->setCurrentTabIndex (tabIndex);
}

bool TabBarButton::hitTest (int mx, int my)
{
    int x, y, w, h;
    getActiveArea (x, y, w, h);

    if (owner->getOrientation() == TabbedButtonBar::TabsAtLeft
         || owner->getOrientation() == TabbedButtonBar::TabsAtRight)
    {
        if (((unsigned int) mx) < (unsigned int) getWidth()
             && my >= y + overlapPixels
             && my < y + h - overlapPixels)
            return true;
    }
    else
    {
        if (mx >= x + overlapPixels && mx < x + w - overlapPixels
             && ((unsigned int) my) < (unsigned int) getHeight())
            return true;
    }

    Path p;
    getLookAndFeel()
        .createTabButtonShape (p, w, h, tabIndex, getButtonText(), *this,
                               owner->getOrientation(),
                               false, false, getToggleState());

    return p.contains ((float) (mx - x),
                       (float) (my - y));
}

int TabBarButton::getBestTabLength (const int depth)
{
    return jlimit (depth * 2,
                   depth * 7,
                   getLookAndFeel().getTabButtonBestWidth (tabIndex, getButtonText(), depth, *this));
}

void TabBarButton::getActiveArea (int& x, int& y, int& w, int& h)
{
    x = 0;
    y = 0;
    int r = getWidth();
    int b = getHeight();

    const int spaceAroundImage = getLookAndFeel().getTabButtonSpaceAroundImage();

    if (owner->getOrientation() != TabbedButtonBar::TabsAtLeft)
        r -= spaceAroundImage;

    if (owner->getOrientation() != TabbedButtonBar::TabsAtRight)
        x += spaceAroundImage;

    if (owner->getOrientation() != TabbedButtonBar::TabsAtBottom)
        y += spaceAroundImage;

    if (owner->getOrientation() != TabbedButtonBar::TabsAtTop)
        b -= spaceAroundImage;

    w = r - x;
    h = b - y;
}


//==============================================================================
class TabAreaBehindFrontButtonComponent  : public Component
{
public:
    TabAreaBehindFrontButtonComponent (TabbedButtonBar* const owner_)
        : owner (owner_)
    {
        setInterceptsMouseClicks (false, false);
    }

    ~TabAreaBehindFrontButtonComponent()
    {
    }

    void paint (Graphics& g)
    {
        getLookAndFeel()
            .drawTabAreaBehindFrontButton (g, getWidth(), getHeight(),
                                           *owner, owner->getOrientation());
    }

    void enablementChanged()
    {
        repaint();
    }

private:
    TabbedButtonBar* const owner;

    TabAreaBehindFrontButtonComponent (const TabAreaBehindFrontButtonComponent&);
    const TabAreaBehindFrontButtonComponent& operator= (const TabAreaBehindFrontButtonComponent&);
};


//==============================================================================
TabbedButtonBar::TabbedButtonBar (const Orientation orientation_)
    : orientation (orientation_),
      currentTabIndex (-1),
      extraTabsButton (0)
{
    setInterceptsMouseClicks (false, true);
    addAndMakeVisible (behindFrontTab = new TabAreaBehindFrontButtonComponent (this));
    setFocusContainer (true);
}

TabbedButtonBar::~TabbedButtonBar()
{
    deleteAllChildren();
}

//==============================================================================
void TabbedButtonBar::setOrientation (const Orientation newOrientation)
{
    orientation = newOrientation;

    for (int i = getNumChildComponents(); --i >= 0;)
        getChildComponent (i)->resized();

    resized();
}

TabBarButton* TabbedButtonBar::createTabButton (const String& name, const int index)
{
    return new TabBarButton (name, this, index);
}

//==============================================================================
void TabbedButtonBar::clearTabs()
{
    tabs.clear();
    tabColours.clear();
    currentTabIndex = -1;

    deleteAndZero (extraTabsButton);
    removeChildComponent (behindFrontTab);
    deleteAllChildren();
    addChildComponent (behindFrontTab);

    setCurrentTabIndex (-1);
}

void TabbedButtonBar::addTab (const String& tabName,
                              const Colour& tabBackgroundColour,
                              int insertIndex)
{
    jassert (tabName.isNotEmpty()); // you have to give them all a name..

    if (tabName.isNotEmpty())
    {
        if (((unsigned int) insertIndex) > (unsigned int) tabs.size())
            insertIndex = tabs.size();

        for (int i = tabs.size(); --i >= insertIndex;)
        {
            TabBarButton* const tb = getTabButton (i);

            if (tb != 0)
                tb->tabIndex++;
        }

        tabs.insert (insertIndex, tabName);
        tabColours.insert (insertIndex, tabBackgroundColour);

        TabBarButton* const tb = createTabButton (tabName, insertIndex);
        jassert (tb != 0); // your createTabButton() mustn't return zero!

        addAndMakeVisible (tb, insertIndex);

        resized();

        if (currentTabIndex < 0)
            setCurrentTabIndex (0);
    }
}

void TabbedButtonBar::setTabName (const int tabIndex,
                                  const String& newName)
{
    if (((unsigned int) tabIndex) < (unsigned int) tabs.size()
         && tabs[tabIndex] != newName)
    {
        tabs.set (tabIndex, newName);

        TabBarButton* const tb = getTabButton (tabIndex);

        if (tb != 0)
            tb->setButtonText (newName);

        resized();
    }
}

void TabbedButtonBar::removeTab (const int tabIndex)
{
    if (((unsigned int) tabIndex) < (unsigned int) tabs.size())
    {
        const int oldTabIndex = currentTabIndex;
        if (currentTabIndex == tabIndex)
            currentTabIndex = -1;

        tabs.remove (tabIndex);
        tabColours.remove (tabIndex);

        delete getTabButton (tabIndex);

        for (int i = tabIndex + 1; i <= tabs.size(); ++i)
        {
            TabBarButton* const tb = getTabButton (i);

            if (tb != 0)
                tb->tabIndex--;
        }

        resized();

        setCurrentTabIndex (jlimit (0, jmax (0, tabs.size() - 1), oldTabIndex));
    }
}

void TabbedButtonBar::moveTab (const int currentIndex,
                               const int newIndex)
{
    tabs.move (currentIndex, newIndex);
    tabColours.move (currentIndex, newIndex);
    resized();
}

int TabbedButtonBar::getNumTabs() const
{
    return tabs.size();
}

const StringArray TabbedButtonBar::getTabNames() const
{
    return tabs;
}

void TabbedButtonBar::setCurrentTabIndex (int newIndex, const bool sendChangeMessage_)
{
    if (currentTabIndex != newIndex)
    {
        if (((unsigned int) newIndex) >= (unsigned int) tabs.size())
            newIndex = -1;

        currentTabIndex = newIndex;

        for (int i = 0; i < getNumChildComponents(); ++i)
        {
            TabBarButton* const tb = dynamic_cast <TabBarButton*> (getChildComponent (i));

            if (tb != 0)
                tb->setToggleState (tb->tabIndex == newIndex, false);
        }

        resized();

        if (sendChangeMessage_)
            sendChangeMessage (this);

        currentTabChanged (newIndex, newIndex >= 0 ? tabs [newIndex] : String::empty);
    }
}

TabBarButton* TabbedButtonBar::getTabButton (const int index) const
{
    for (int i = getNumChildComponents(); --i >= 0;)
    {
        TabBarButton* const tb = dynamic_cast <TabBarButton*> (getChildComponent (i));

        if (tb != 0 && tb->tabIndex == index)
            return tb;
    }

    return 0;
}

void TabbedButtonBar::lookAndFeelChanged()
{
    deleteAndZero (extraTabsButton);
    resized();
}

void TabbedButtonBar::resized()
{
    const double minimumScale = 0.7;
    int depth = getWidth();
    int length = getHeight();

    if (orientation == TabsAtTop || orientation == TabsAtBottom)
        swapVariables (depth, length);

    const int overlap = getLookAndFeel().getTabButtonOverlap (depth)
                            + getLookAndFeel().getTabButtonSpaceAroundImage() * 2;

    int i, totalLength = overlap;
    int numVisibleButtons = tabs.size();

    for (i = 0; i < getNumChildComponents(); ++i)
    {
        TabBarButton* const tb = dynamic_cast <TabBarButton*> (getChildComponent (i));

        if (tb != 0)
        {
            totalLength += tb->getBestTabLength (depth) - overlap;
            tb->overlapPixels = overlap / 2;
        }
    }

    double scale = 1.0;

    if (totalLength > length)
        scale = jmax (minimumScale, length / (double) totalLength);

    const bool isTooBig = totalLength * scale > length;
    int tabsButtonPos = 0;

    if (isTooBig)
    {
        if (extraTabsButton == 0)
        {
            addAndMakeVisible (extraTabsButton = getLookAndFeel().createTabBarExtrasButton());
            extraTabsButton->addButtonListener (this);
            extraTabsButton->setAlwaysOnTop (true);
            extraTabsButton->setTriggeredOnMouseDown (true);
        }

        const int buttonSize = jmin (proportionOfWidth (0.7f), proportionOfHeight (0.7f));
        extraTabsButton->setSize (buttonSize, buttonSize);

        if (orientation == TabsAtTop || orientation == TabsAtBottom)
        {
            tabsButtonPos = getWidth() - buttonSize / 2 - 1;
            extraTabsButton->setCentrePosition (tabsButtonPos, getHeight() / 2);
        }
        else
        {
            tabsButtonPos = getHeight() - buttonSize / 2 - 1;
            extraTabsButton->setCentrePosition (getWidth() / 2, tabsButtonPos);
        }

        totalLength = 0;

        for (i = 0; i < tabs.size(); ++i)
        {
            TabBarButton* const tb = getTabButton (i);

            if (tb != 0)
            {
                const int newLength = totalLength + tb->getBestTabLength (depth);

                if (i > 0 && newLength * minimumScale > tabsButtonPos)
                {
                    totalLength += overlap;
                    break;
                }

                numVisibleButtons = i + 1;
                totalLength = newLength - overlap;

            }
        }

        scale = jmax (minimumScale, tabsButtonPos / (double) totalLength);
    }
    else
    {
        deleteAndZero (extraTabsButton);
    }

    int pos = 0;

    TabBarButton* frontTab = 0;

    for (i = 0; i < tabs.size(); ++i)
    {
        TabBarButton* const tb = getTabButton (i);

        if (tb != 0)
        {
            const int bestLength = roundToInt (scale * tb->getBestTabLength (depth));

            if (i < numVisibleButtons)
            {
                if (orientation == TabsAtTop || orientation == TabsAtBottom)
                    tb->setBounds (pos, 0, bestLength, getHeight());
                else
                    tb->setBounds (0, pos, getWidth(), bestLength);

                tb->toBack();

                if (tb->tabIndex == currentTabIndex)
                    frontTab = tb;

                tb->setVisible (true);
            }
            else
            {
                tb->setVisible (false);
            }

            pos += bestLength - overlap;
        }
    }

    behindFrontTab->setBounds (0, 0, getWidth(), getHeight());

    if (frontTab != 0)
    {
        frontTab->toFront (false);
        behindFrontTab->toBehind (frontTab);
    }
}

//==============================================================================
const Colour TabbedButtonBar::getTabBackgroundColour (const int tabIndex)
{
    return tabColours [tabIndex];
}

void TabbedButtonBar::setTabBackgroundColour (const int tabIndex, const Colour& newColour)
{
    if (((unsigned int) tabIndex) < (unsigned int) tabColours.size()
         && tabColours [tabIndex] != newColour)
    {
        tabColours.set (tabIndex, newColour);
        repaint();
    }
}

void TabbedButtonBar::buttonClicked (Button* button)
{
    if (extraTabsButton == button)
    {
        PopupMenu m;

        for (int i = 0; i < tabs.size(); ++i)
        {
            TabBarButton* const tb = getTabButton (i);

            if (tb != 0 && ! tb->isVisible())
                m.addItem (tb->tabIndex + 1, tabs[i], true, i == currentTabIndex);
        }

        const int res = m.showAt (extraTabsButton);

        if (res != 0)
            setCurrentTabIndex (res - 1);
    }
}

//==============================================================================
void TabbedButtonBar::currentTabChanged (const int, const String&)
{
}

void TabbedButtonBar::popupMenuClickOnTab (const int, const String&)
{
}

END_JUCE_NAMESPACE
