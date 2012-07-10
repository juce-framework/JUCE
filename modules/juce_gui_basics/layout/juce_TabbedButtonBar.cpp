/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

TabBarButton::TabBarButton (const String& name, TabbedButtonBar& owner_)
    : Button (name),
      owner (owner_),
      overlapPixels (0)
{
    shadow.setShadowProperties (2.2f, 0.7f, 0, 0);
    setComponentEffect (&shadow);
    setWantsKeyboardFocus (false);
}

TabBarButton::~TabBarButton()
{
}

int TabBarButton::getIndex() const
{
    return owner.indexOfTabButton (this);
}

void TabBarButton::paintButton (Graphics& g,
                                bool isMouseOverButton,
                                bool isButtonDown)
{
    const Rectangle<int> area (getActiveArea());
    g.setOrigin (area.getX(), area.getY());

    getLookAndFeel().drawTabButton (g, area.getWidth(), area.getHeight(),
                                    owner.getTabBackgroundColour (getIndex()),
                                    getIndex(), getButtonText(), *this,
                                    owner.getOrientation(),
                                    isMouseOverButton, isButtonDown,
                                    getToggleState());
}

void TabBarButton::clicked (const ModifierKeys& mods)
{
    if (mods.isPopupMenu())
        owner.popupMenuClickOnTab (getIndex(), getButtonText());
    else
        owner.setCurrentTabIndex (getIndex());
}

bool TabBarButton::hitTest (int mx, int my)
{
    const Rectangle<int> area (getActiveArea());

    if (owner.getOrientation() == TabbedButtonBar::TabsAtLeft
         || owner.getOrientation() == TabbedButtonBar::TabsAtRight)
    {
        if (isPositiveAndBelow (mx, getWidth())
             && my >= area.getY() + overlapPixels
             && my < area.getBottom() - overlapPixels)
            return true;
    }
    else
    {
        if (mx >= area.getX() + overlapPixels && mx < area.getRight() - overlapPixels
             && isPositiveAndBelow (my, getHeight()))
            return true;
    }

    Path p;
    getLookAndFeel().createTabButtonShape (p, area.getWidth(), area.getHeight(),
                                           getIndex(), getButtonText(), *this,
                                           owner.getOrientation(), false, false, getToggleState());

    return p.contains ((float) (mx - area.getX()),
                       (float) (my - area.getY()));
}

int TabBarButton::getBestTabLength (const int depth)
{
    return jlimit (depth * 2,
                   depth * 7,
                   getLookAndFeel().getTabButtonBestWidth (getIndex(), getButtonText(), depth, *this));
}

Rectangle<int> TabBarButton::getActiveArea()
{
    Rectangle<int> r (getLocalBounds());
    const int spaceAroundImage = getLookAndFeel().getTabButtonSpaceAroundImage();

    if (owner.getOrientation() != TabbedButtonBar::TabsAtLeft)      r.removeFromRight (spaceAroundImage);
    if (owner.getOrientation() != TabbedButtonBar::TabsAtRight)     r.removeFromLeft (spaceAroundImage);
    if (owner.getOrientation() != TabbedButtonBar::TabsAtBottom)    r.removeFromTop (spaceAroundImage);
    if (owner.getOrientation() != TabbedButtonBar::TabsAtTop)       r.removeFromBottom (spaceAroundImage);

    return r;
}


//==============================================================================
class TabbedButtonBar::BehindFrontTabComp  : public Component,
                                             public ButtonListener // (can't use Button::Listener due to idiotic VC2005 bug)
{
public:
    BehindFrontTabComp (TabbedButtonBar& owner_)
        : owner (owner_)
    {
        setInterceptsMouseClicks (false, false);
    }

    void paint (Graphics& g)
    {
        getLookAndFeel().drawTabAreaBehindFrontButton (g, getWidth(), getHeight(),
                                                       owner, owner.getOrientation());
    }

    void enablementChanged()
    {
        repaint();
    }

    void buttonClicked (Button*)
    {
        owner.showExtraItemsMenu();
    }

private:
    TabbedButtonBar& owner;

    JUCE_DECLARE_NON_COPYABLE (BehindFrontTabComp);
};


//==============================================================================
TabbedButtonBar::TabbedButtonBar (const Orientation orientation_)
    : orientation (orientation_),
      minimumScale (0.7),
      currentTabIndex (-1)
{
    setInterceptsMouseClicks (false, true);
    addAndMakeVisible (behindFrontTab = new BehindFrontTabComp (*this));
    setFocusContainer (true);
}

TabbedButtonBar::~TabbedButtonBar()
{
    tabs.clear();
    extraTabsButton = nullptr;
}

//==============================================================================
void TabbedButtonBar::setOrientation (const Orientation newOrientation)
{
    orientation = newOrientation;

    for (int i = getNumChildComponents(); --i >= 0;)
        getChildComponent (i)->resized();

    resized();
}

TabBarButton* TabbedButtonBar::createTabButton (const String& name, const int /*index*/)
{
    return new TabBarButton (name, *this);
}

void TabbedButtonBar::setMinimumTabScaleFactor (double newMinimumScale)
{
    minimumScale = newMinimumScale;
    resized();
}

//==============================================================================
void TabbedButtonBar::clearTabs()
{
    tabs.clear();
    extraTabsButton = nullptr;
    setCurrentTabIndex (-1);
}

void TabbedButtonBar::addTab (const String& tabName,
                              const Colour& tabBackgroundColour,
                              int insertIndex)
{
    jassert (tabName.isNotEmpty()); // you have to give them all a name..

    if (tabName.isNotEmpty())
    {
        if (! isPositiveAndBelow (insertIndex, tabs.size()))
            insertIndex = tabs.size();

        TabInfo* const currentTab = tabs [currentTabIndex];

        TabInfo* newTab = new TabInfo();
        newTab->name = tabName;
        newTab->colour = tabBackgroundColour;
        newTab->component = createTabButton (tabName, insertIndex);
        jassert (newTab->component != nullptr);

        tabs.insert (insertIndex, newTab);
        currentTabIndex = tabs.indexOf (currentTab);
        addAndMakeVisible (newTab->component, insertIndex);

        resized();

        if (currentTabIndex < 0)
            setCurrentTabIndex (0);
    }
}

void TabbedButtonBar::setTabName (const int tabIndex, const String& newName)
{
    TabInfo* const tab = tabs [tabIndex];

    if (tab != nullptr && tab->name != newName)
    {
        tab->name = newName;
        tab->component->setButtonText (newName);
        resized();
    }
}

void TabbedButtonBar::removeTab (const int tabIndex)
{
    if (tabIndex == currentTabIndex)
        setCurrentTabIndex (-1);

    TabInfo* const currentTab = tabs [currentTabIndex];
    tabs.remove (tabIndex);
    currentTabIndex = tabs.indexOf (currentTab);
    resized();
}

void TabbedButtonBar::moveTab (const int currentIndex, const int newIndex)
{
    TabInfo* const currentTab = tabs [currentTabIndex];
    tabs.move (currentIndex, newIndex);
    currentTabIndex = tabs.indexOf (currentTab);
    resized();
}

int TabbedButtonBar::getNumTabs() const
{
    return tabs.size();
}

String TabbedButtonBar::getCurrentTabName() const
{
    TabInfo* tab = tabs [currentTabIndex];
    return tab == nullptr ? String::empty : tab->name;
}

StringArray TabbedButtonBar::getTabNames() const
{
    StringArray names;

    for (int i = 0; i < tabs.size(); ++i)
        names.add (tabs.getUnchecked(i)->name);

    return names;
}

void TabbedButtonBar::setCurrentTabIndex (int newIndex, const bool sendChangeMessage_)
{
    if (currentTabIndex != newIndex)
    {
        if (! isPositiveAndBelow (newIndex, tabs.size()))
            newIndex = -1;

        currentTabIndex = newIndex;

        for (int i = 0; i < tabs.size(); ++i)
        {
            TabBarButton* tb = tabs.getUnchecked(i)->component;
            tb->setToggleState (i == newIndex, false);
        }

        resized();

        if (sendChangeMessage_)
            sendChangeMessage();

        currentTabChanged (newIndex, getCurrentTabName());
    }
}

TabBarButton* TabbedButtonBar::getTabButton (const int index) const
{
    TabInfo* const tab = tabs[index];
    return tab == nullptr ? nullptr : static_cast <TabBarButton*> (tab->component);
}

int TabbedButtonBar::indexOfTabButton (const TabBarButton* button) const
{
    for (int i = tabs.size(); --i >= 0;)
        if (tabs.getUnchecked(i)->component == button)
            return i;

    return -1;
}

void TabbedButtonBar::lookAndFeelChanged()
{
    extraTabsButton = nullptr;
    resized();
}

void TabbedButtonBar::resized()
{
    LookAndFeel& lf = getLookAndFeel();

    int depth = getWidth();
    int length = getHeight();

    if (orientation == TabsAtTop || orientation == TabsAtBottom)
        std::swap (depth, length);

    const int overlap = lf.getTabButtonOverlap (depth) + lf.getTabButtonSpaceAroundImage() * 2;

    int i, totalLength = overlap;
    int numVisibleButtons = tabs.size();

    for (i = 0; i < tabs.size(); ++i)
    {
        TabBarButton* const tb = tabs.getUnchecked(i)->component;

        totalLength += tb->getBestTabLength (depth) - overlap;
        tb->overlapPixels = overlap / 2;
    }

    double scale = 1.0;

    if (totalLength > length)
        scale = jmax (minimumScale, length / (double) totalLength);

    const bool isTooBig = totalLength * scale > length;
    int tabsButtonPos = 0;

    if (isTooBig)
    {
        if (extraTabsButton == nullptr)
        {
            addAndMakeVisible (extraTabsButton = lf.createTabBarExtrasButton());
            extraTabsButton->addListener (behindFrontTab);
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
            TabBarButton* const tb = tabs.getUnchecked(i)->component;

            const int newLength = totalLength + tb->getBestTabLength (depth);

            if (i > 0 && newLength * minimumScale > tabsButtonPos)
            {
                totalLength += overlap;
                break;
            }

            numVisibleButtons = i + 1;
            totalLength = newLength - overlap;
        }

        scale = jmax (minimumScale, tabsButtonPos / (double) totalLength);
    }
    else
    {
        extraTabsButton = nullptr;
    }

    int pos = 0;

    TabBarButton* frontTab = nullptr;

    for (i = 0; i < tabs.size(); ++i)
    {
        TabBarButton* const tb = getTabButton (i);

        if (tb != nullptr)
        {
            const int bestLength = roundToInt (scale * tb->getBestTabLength (depth));

            if (i < numVisibleButtons)
            {
                if (orientation == TabsAtTop || orientation == TabsAtBottom)
                    tb->setBounds (pos, 0, bestLength, getHeight());
                else
                    tb->setBounds (0, pos, getWidth(), bestLength);

                tb->toBack();

                if (i == currentTabIndex)
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

    behindFrontTab->setBounds (getLocalBounds());

    if (frontTab != nullptr)
    {
        frontTab->toFront (false);
        behindFrontTab->toBehind (frontTab);
    }
}

//==============================================================================
Colour TabbedButtonBar::getTabBackgroundColour (const int tabIndex)
{
    TabInfo* const tab = tabs [tabIndex];
    return tab == nullptr ? Colours::white : tab->colour;
}

void TabbedButtonBar::setTabBackgroundColour (const int tabIndex, const Colour& newColour)
{
    TabInfo* const tab = tabs [tabIndex];

    if (tab != nullptr && tab->colour != newColour)
    {
        tab->colour = newColour;
        repaint();
    }
}

void TabbedButtonBar::extraItemsMenuCallback (int result, TabbedButtonBar* bar)
{
    if (bar != nullptr && result > 0)
        bar->setCurrentTabIndex (result - 1);
}

void TabbedButtonBar::showExtraItemsMenu()
{
    PopupMenu m;

    for (int i = 0; i < tabs.size(); ++i)
    {
        const TabInfo* const tab = tabs.getUnchecked(i);

        if (! tab->component->isVisible())
            m.addItem (i + 1, tab->name, true, i == currentTabIndex);
    }

    m.showMenuAsync (PopupMenu::Options().withTargetComponent (extraTabsButton),
                     ModalCallbackFunction::forComponent (extraItemsMenuCallback, this));
}

//==============================================================================
void TabbedButtonBar::currentTabChanged (const int, const String&)
{
}

void TabbedButtonBar::popupMenuClickOnTab (const int, const String&)
{
}
