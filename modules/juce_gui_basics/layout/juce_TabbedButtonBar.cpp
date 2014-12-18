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

TabBarButton::TabBarButton (const String& name, TabbedButtonBar& owner_)
    : Button (name), owner (owner_), overlapPixels (0), extraCompPlacement (afterText)
{
    setWantsKeyboardFocus (false);
}

TabBarButton::~TabBarButton() {}

int TabBarButton::getIndex() const                      { return owner.indexOfTabButton (this); }
Colour TabBarButton::getTabBackgroundColour() const     { return owner.getTabBackgroundColour (getIndex()); }
bool TabBarButton::isFrontTab() const                   { return getToggleState(); }

void TabBarButton::paintButton (Graphics& g, const bool isMouseOverButton, const bool isButtonDown)
{
    getLookAndFeel().drawTabButton (*this, g, isMouseOverButton, isButtonDown);
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

    if (owner.isVertical())
    {
        if (isPositiveAndBelow (mx, getWidth())
             && my >= area.getY() + overlapPixels && my < area.getBottom() - overlapPixels)
            return true;
    }
    else
    {
        if (isPositiveAndBelow (my, getHeight())
             && mx >= area.getX() + overlapPixels && mx < area.getRight() - overlapPixels)
            return true;
    }

    Path p;
    getLookAndFeel().createTabButtonShape (*this, p, false, false);

    return p.contains ((float) (mx - area.getX()),
                       (float) (my - area.getY()));
}

int TabBarButton::getBestTabLength (const int depth)
{
    return getLookAndFeel().getTabButtonBestWidth (*this, depth);
}

void TabBarButton::calcAreas (Rectangle<int>& extraComp, Rectangle<int>& textArea) const
{
    LookAndFeel& lf = getLookAndFeel();
    textArea = getActiveArea();

    const int depth = owner.isVertical() ? textArea.getWidth() : textArea.getHeight();
    const int overlap = lf.getTabButtonOverlap (depth);

    if (overlap > 0)
    {
        if (owner.isVertical())
            textArea.reduce (0, overlap);
        else
            textArea.reduce (overlap, 0);
    }

    if (extraComponent != nullptr)
    {
        extraComp = lf.getTabButtonExtraComponentBounds (*this, textArea, *extraComponent);

        const TabbedButtonBar::Orientation orientation = owner.getOrientation();

        if (orientation == TabbedButtonBar::TabsAtLeft || orientation == TabbedButtonBar::TabsAtRight)
        {
            if (extraComp.getCentreY() > textArea.getCentreY())
                textArea.setBottom (jmin (textArea.getBottom(), extraComp.getY()));
            else
                textArea.setTop (jmax (textArea.getY(), extraComp.getBottom()));
        }
        else
        {
            if (extraComp.getCentreX() > textArea.getCentreX())
                textArea.setRight (jmin (textArea.getRight(), extraComp.getX()));
            else
                textArea.setLeft (jmax (textArea.getX(), extraComp.getRight()));
        }
    }
}

Rectangle<int> TabBarButton::getTextArea() const
{
    Rectangle<int> extraComp, textArea;
    calcAreas (extraComp, textArea);
    return textArea;
}

Rectangle<int> TabBarButton::getActiveArea() const
{
    Rectangle<int> r (getLocalBounds());
    const int spaceAroundImage = getLookAndFeel().getTabButtonSpaceAroundImage();
    const TabbedButtonBar::Orientation orientation = owner.getOrientation();

    if (orientation != TabbedButtonBar::TabsAtLeft)      r.removeFromRight  (spaceAroundImage);
    if (orientation != TabbedButtonBar::TabsAtRight)     r.removeFromLeft   (spaceAroundImage);
    if (orientation != TabbedButtonBar::TabsAtBottom)    r.removeFromTop    (spaceAroundImage);
    if (orientation != TabbedButtonBar::TabsAtTop)       r.removeFromBottom (spaceAroundImage);

    return r;
}

void TabBarButton::setExtraComponent (Component* comp, ExtraComponentPlacement placement)
{
    jassert (extraCompPlacement == beforeText || extraCompPlacement == afterText);
    extraCompPlacement = placement;
    addAndMakeVisible (extraComponent = comp);
    resized();
}

void TabBarButton::childBoundsChanged (Component* c)
{
    if (c == extraComponent)
    {
        owner.resized();
        resized();
    }
}

void TabBarButton::resized()
{
    if (extraComponent != nullptr)
    {
        Rectangle<int> extraComp, textArea;
        calcAreas (extraComp, textArea);

        if (! extraComp.isEmpty())
            extraComponent->setBounds (extraComp);
    }
}

//==============================================================================
class TabbedButtonBar::BehindFrontTabComp  : public Component,
                                             public ButtonListener // (can't use Button::Listener due to idiotic VC2005 bug)
{
public:
    BehindFrontTabComp (TabbedButtonBar& tb)  : owner (tb)
    {
        setInterceptsMouseClicks (false, false);
    }

    void paint (Graphics& g) override
    {
        getLookAndFeel().drawTabAreaBehindFrontButton (owner, g, getWidth(), getHeight());
    }

    void enablementChanged() override
    {
        repaint();
    }

    void buttonClicked (Button*) override
    {
        owner.showExtraItemsMenu();
    }

private:
    TabbedButtonBar& owner;

    JUCE_DECLARE_NON_COPYABLE (BehindFrontTabComp)
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
                              Colour tabBackgroundColour,
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
        newTab->button = createTabButton (tabName, insertIndex);
        jassert (newTab->button != nullptr);

        tabs.insert (insertIndex, newTab);
        currentTabIndex = tabs.indexOf (currentTab);
        addAndMakeVisible (newTab->button, insertIndex);

        resized();

        if (currentTabIndex < 0)
            setCurrentTabIndex (0);
    }
}

void TabbedButtonBar::setTabName (const int tabIndex, const String& newName)
{
    if (TabInfo* const tab = tabs [tabIndex])
    {
        if (tab->name != newName)
        {
            tab->name = newName;
            tab->button->setButtonText (newName);
            resized();
        }
    }
}

void TabbedButtonBar::removeTab (const int tabIndex, const bool animate)
{
    const int oldIndex = currentTabIndex;
    if (tabIndex == currentTabIndex)
        setCurrentTabIndex (-1);

    tabs.remove (tabIndex);

    setCurrentTabIndex (oldIndex);
    updateTabPositions (animate);
}

void TabbedButtonBar::moveTab (const int currentIndex, const int newIndex, const bool animate)
{
    TabInfo* const currentTab = tabs [currentTabIndex];
    tabs.move (currentIndex, newIndex);
    currentTabIndex = tabs.indexOf (currentTab);
    updateTabPositions (animate);
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
            TabBarButton* tb = tabs.getUnchecked(i)->button;
            tb->setToggleState (i == newIndex, dontSendNotification);
        }

        resized();

        if (sendChangeMessage_)
            sendChangeMessage();

        currentTabChanged (newIndex, getCurrentTabName());
    }
}

TabBarButton* TabbedButtonBar::getTabButton (const int index) const
{
    if (TabInfo* tab = tabs[index])
        return static_cast<TabBarButton*> (tab->button);

    return nullptr;
}

int TabbedButtonBar::indexOfTabButton (const TabBarButton* button) const
{
    for (int i = tabs.size(); --i >= 0;)
        if (tabs.getUnchecked(i)->button == button)
            return i;

    return -1;
}

Rectangle<int> TabbedButtonBar::getTargetBounds (TabBarButton* button) const
{
    if (button == nullptr || indexOfTabButton (button) == -1)
        return Rectangle<int>();

    ComponentAnimator& animator = Desktop::getInstance().getAnimator();

    return animator.isAnimating (button) ? animator.getComponentDestination (button) : button->getBounds();
}

void TabbedButtonBar::lookAndFeelChanged()
{
    extraTabsButton = nullptr;
    resized();
}

void TabbedButtonBar::paint (Graphics& g)
{
    getLookAndFeel().drawTabbedButtonBarBackground (*this, g);
}

void TabbedButtonBar::resized()
{
    updateTabPositions (false);
}

//==============================================================================
void TabbedButtonBar::updateTabPositions (bool animate)
{
    LookAndFeel& lf = getLookAndFeel();

    int depth = getWidth();
    int length = getHeight();

    if (! isVertical())
        std::swap (depth, length);

    const int overlap = lf.getTabButtonOverlap (depth) + lf.getTabButtonSpaceAroundImage() * 2;

    int totalLength = jmax (0, overlap);
    int numVisibleButtons = tabs.size();

    for (int i = 0; i < tabs.size(); ++i)
    {
        TabBarButton* const tb = tabs.getUnchecked(i)->button;

        totalLength += tb->getBestTabLength (depth) - overlap;
        tb->overlapPixels = jmax (0, overlap / 2);
    }

    double scale = 1.0;

    if (totalLength > length)
        scale = jmax (minimumScale, length / (double) totalLength);

    const bool isTooBig = (int) (totalLength * scale) > length;
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

        if (isVertical())
        {
            tabsButtonPos = getHeight() - buttonSize / 2 - 1;
            extraTabsButton->setCentrePosition (getWidth() / 2, tabsButtonPos);
        }
        else
        {
            tabsButtonPos = getWidth() - buttonSize / 2 - 1;
            extraTabsButton->setCentrePosition (tabsButtonPos, getHeight() / 2);
        }

        totalLength = 0;

        for (int i = 0; i < tabs.size(); ++i)
        {
            TabBarButton* const tb = tabs.getUnchecked(i)->button;
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
    ComponentAnimator& animator = Desktop::getInstance().getAnimator();

    for (int i = 0; i < tabs.size(); ++i)
    {
        if (TabBarButton* const tb = getTabButton (i))
        {
            const int bestLength = roundToInt (scale * tb->getBestTabLength (depth));

            if (i < numVisibleButtons)
            {
                const Rectangle<int> newBounds (isVertical() ? Rectangle<int> (0, pos, getWidth(), bestLength)
                                                             : Rectangle<int> (pos, 0, bestLength, getHeight()));

                if (animate)
                {
                    animator.animateComponent (tb, newBounds, 1.0f, 200, false, 3.0, 0.0);
                }
                else
                {
                    animator.cancelAnimation (tb, false);
                    tb->setBounds (newBounds);
                }

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
    if (TabInfo* tab = tabs [tabIndex])
        return tab->colour;

    return Colours::transparentBlack;
}

void TabbedButtonBar::setTabBackgroundColour (const int tabIndex, Colour newColour)
{
    if (TabInfo* const tab = tabs [tabIndex])
    {
        if (tab->colour != newColour)
        {
            tab->colour = newColour;
            repaint();
        }
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

        if (! tab->button->isVisible())
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
