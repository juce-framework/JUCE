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

class PopupMenu::Item
{
public:
    Item()  : itemID (0), isActive (true), isSeparator (true), isTicked (false),
              usesColour (false), commandManager (nullptr)
    {}

    Item (const int itemId,
          const String& name,
          const bool active,
          const bool ticked,
          const Image& im,
          const Colour& colour,
          const bool useColour,
          CustomComponent* const custom,
          const PopupMenu* const sub,
          ApplicationCommandManager* const manager)

      : itemID (itemId), text (name), textColour (colour),
        isActive (active), isSeparator (false), isTicked (ticked),
        usesColour (useColour), image (im), customComp (custom),
        subMenu (createCopyIfNotNull (sub)), commandManager (manager)
    {
        if (commandManager != nullptr && itemID != 0)
        {
            String shortcutKey;

            const Array <KeyPress> keyPresses (commandManager->getKeyMappings()
                                                    ->getKeyPressesAssignedToCommand (itemID));

            for (int i = 0; i < keyPresses.size(); ++i)
            {
                const String key (keyPresses.getReference(i).getTextDescriptionWithIcons());

                if (shortcutKey.isNotEmpty())
                    shortcutKey << ", ";

                if (key.length() == 1 && key[0] < 128)
                    shortcutKey << "shortcut: '" << key << '\'';
                else
                    shortcutKey << key;
            }

            shortcutKey = shortcutKey.trim();

            if (shortcutKey.isNotEmpty())
                text << "<end>" << shortcutKey;
        }
    }

    Item (const Item& other)
        : itemID (other.itemID),
          text (other.text),
          textColour (other.textColour),
          isActive (other.isActive),
          isSeparator (other.isSeparator),
          isTicked (other.isTicked),
          usesColour (other.usesColour),
          image (other.image),
          customComp (other.customComp),
          subMenu (createCopyIfNotNull (other.subMenu.get())),
          commandManager (other.commandManager)
    {}

    bool canBeTriggered() const noexcept    { return isActive && itemID != 0; }
    bool hasActiveSubMenu() const noexcept  { return isActive && subMenu != nullptr && subMenu->items.size() > 0; }

    //==============================================================================
    const int itemID;
    String text;
    const Colour textColour;
    const bool isActive, isSeparator, isTicked, usesColour;
    Image image;
    ReferenceCountedObjectPtr <CustomComponent> customComp;
    ScopedPointer <PopupMenu> subMenu;
    ApplicationCommandManager* const commandManager;

private:
    Item& operator= (const Item&);

    JUCE_LEAK_DETECTOR (Item)
};


//==============================================================================
class PopupMenu::ItemComponent  : public Component
{
public:
    ItemComponent (const PopupMenu::Item& info, int standardItemHeight, Component* const parent)
      : itemInfo (info),
        isHighlighted (false)
    {
        addAndMakeVisible (itemInfo.customComp);
        parent->addAndMakeVisible (this);

        int itemW = 80;
        int itemH = 16;
        getIdealSize (itemW, itemH, standardItemHeight);
        setSize (itemW, jlimit (2, 600, itemH));

        addMouseListener (parent, false);
    }

    ~ItemComponent()
    {
        removeChildComponent (itemInfo.customComp);
    }

    void getIdealSize (int& idealWidth, int& idealHeight, const int standardItemHeight)
    {
        if (itemInfo.customComp != nullptr)
            itemInfo.customComp->getIdealSize (idealWidth, idealHeight);
        else
            getLookAndFeel().getIdealPopupMenuItemSize (itemInfo.text,
                                                        itemInfo.isSeparator,
                                                        standardItemHeight,
                                                        idealWidth, idealHeight);
    }

    void paint (Graphics& g)
    {
        if (itemInfo.customComp == nullptr)
        {
            String mainText (itemInfo.text);
            String endText;
            const int endIndex = mainText.indexOf ("<end>");

            if (endIndex >= 0)
            {
                endText = mainText.substring (endIndex + 5).trim();
                mainText = mainText.substring (0, endIndex);
            }

            getLookAndFeel()
                .drawPopupMenuItem (g, getWidth(), getHeight(),
                                    itemInfo.isSeparator,
                                    itemInfo.isActive,
                                    isHighlighted,
                                    itemInfo.isTicked,
                                    itemInfo.subMenu != nullptr && (itemInfo.itemID == 0 || itemInfo.subMenu->getNumItems() > 0),
                                    mainText, endText,
                                    itemInfo.image.isValid() ? &itemInfo.image : nullptr,
                                    itemInfo.usesColour ? &(itemInfo.textColour) : nullptr);
        }
    }

    void resized()
    {
        if (Component* const child = getChildComponent (0))
            child->setBounds (getLocalBounds().reduced (2, 0));
    }

    void setHighlighted (bool shouldBeHighlighted)
    {
        shouldBeHighlighted = shouldBeHighlighted && itemInfo.isActive;

        if (isHighlighted != shouldBeHighlighted)
        {
            isHighlighted = shouldBeHighlighted;

            if (itemInfo.customComp != nullptr)
                itemInfo.customComp->setHighlighted (shouldBeHighlighted);

            repaint();
        }
    }

    PopupMenu::Item itemInfo;

private:
    bool isHighlighted;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ItemComponent)
};


//==============================================================================
namespace PopupMenuSettings
{
    const int scrollZone = 24;
    const int borderSize = 2;
    const int timerInterval = 50;
    const int dismissCommandId = 0x6287345f;

    static bool menuWasHiddenBecauseOfAppChange = false;
}

//==============================================================================
class PopupMenu::Window  : public Component,
                           private Timer
{
public:
    Window (const PopupMenu& menu, Window* const parentWindow,
            const Options& opts,
            const bool alignToRectangle,
            const bool shouldDismissOnMouseUp,
            ApplicationCommandManager** const manager)
       : Component ("menu"),
         owner (parentWindow),
         options (opts),
         activeSubMenu (nullptr),
         managerOfChosenCommand (manager),
         componentAttachedTo (options.targetComponent),
         isOver (false),
         hasBeenOver (false),
         isDown (false),
         needsToScroll (false),
         dismissOnMouseUp (shouldDismissOnMouseUp),
         hideOnExit (false),
         disableMouseMoves (false),
         hasAnyJuceCompHadFocus (false),
         numColumns (0),
         contentHeight (0),
         childYOffset (0),
         menuCreationTime (Time::getMillisecondCounter()),
         lastMouseMoveTime (0),
         timeEnteredCurrentChildComp (0),
         scrollAcceleration (1.0)
    {
        lastFocusedTime = lastScrollTime = menuCreationTime;
        setWantsKeyboardFocus (false);
        setMouseClickGrabsKeyboardFocus (false);
        setAlwaysOnTop (true);

        setLookAndFeel (owner != nullptr ? &(owner->getLookAndFeel())
                                         : menu.lookAndFeel);

        setOpaque (getLookAndFeel().findColour (PopupMenu::backgroundColourId).isOpaque()
                     || ! Desktop::canUseSemiTransparentWindows());

        for (int i = 0; i < menu.items.size(); ++i)
        {
            PopupMenu::Item* const item = menu.items.getUnchecked(i);

            if (i < menu.items.size() - 1 || ! item->isSeparator)
                items.add (new PopupMenu::ItemComponent (*item, options.standardHeight, this));
        }

        calculateWindowPos (options.targetArea, alignToRectangle);
        setTopLeftPosition (windowPos.getPosition());
        updateYPositions();

        if (options.visibleItemID != 0)
        {
            const int y = options.targetArea.getY() - windowPos.getY();
            ensureItemIsVisible (options.visibleItemID,
                                 isPositiveAndBelow (y, windowPos.getHeight()) ? y : -1);
        }

        resizeToBestWindowPos();
        addToDesktop (ComponentPeer::windowIsTemporary
                       | ComponentPeer::windowIgnoresKeyPresses
                       | getLookAndFeel().getMenuWindowFlags());

        getActiveWindows().add (this);
        Desktop::getInstance().addGlobalMouseListener (this);
    }

    ~Window()
    {
        getActiveWindows().removeFirstMatchingValue (this);
        Desktop::getInstance().removeGlobalMouseListener (this);
        activeSubMenu = nullptr;
        items.clear();
    }

    //==============================================================================
    void paint (Graphics& g)
    {
        if (isOpaque())
            g.fillAll (Colours::white);

        getLookAndFeel().drawPopupMenuBackground (g, getWidth(), getHeight());
    }

    void paintOverChildren (Graphics& g)
    {
        if (canScroll())
        {
            LookAndFeel& lf = getLookAndFeel();

            if (isTopScrollZoneActive())
                lf.drawPopupMenuUpDownArrow (g, getWidth(), PopupMenuSettings::scrollZone, true);

            if (isBottomScrollZoneActive())
            {
                g.setOrigin (0, getHeight() - PopupMenuSettings::scrollZone);
                lf.drawPopupMenuUpDownArrow (g, getWidth(), PopupMenuSettings::scrollZone, false);
            }
        }
    }

    //==============================================================================
    // hide this and all sub-comps
    void hide (const PopupMenu::Item* const item, const bool makeInvisible)
    {
        if (isVisible())
        {
            WeakReference<Component> deletionChecker (this);

            activeSubMenu = nullptr;
            currentChild = nullptr;

            if (item != nullptr
                 && item->commandManager != nullptr
                 && item->itemID != 0)
            {
                *managerOfChosenCommand = item->commandManager;
            }

            exitModalState (item != nullptr ? item->itemID : 0);

            if (makeInvisible && (deletionChecker != nullptr))
                setVisible (false);
        }
    }

    void dismissMenu (const PopupMenu::Item* const item)
    {
        if (owner != nullptr)
        {
            owner->dismissMenu (item);
        }
        else
        {
            if (item != nullptr)
            {
                // need a copy of this on the stack as the one passed in will get deleted during this call
                const PopupMenu::Item mi (*item);
                hide (&mi, false);
            }
            else
            {
                hide (nullptr, false);
            }
        }
    }

    //==============================================================================
    void mouseMove (const MouseEvent&)    { timerCallback(); }
    void mouseDown (const MouseEvent&)    { timerCallback(); }
    void mouseDrag (const MouseEvent&)    { timerCallback(); }
    void mouseUp   (const MouseEvent&)    { timerCallback(); }

    void mouseWheelMove (const MouseEvent&, const MouseWheelDetails& wheel)
    {
        alterChildYPos (roundToInt (-10.0f * wheel.deltaY * PopupMenuSettings::scrollZone));
        lastMousePos = Point<int> (-1, -1);
    }

    bool keyPressed (const KeyPress& key)
    {
        if (key.isKeyCode (KeyPress::downKey))
        {
            selectNextItem (1);
        }
        else if (key.isKeyCode (KeyPress::upKey))
        {
            selectNextItem (-1);
        }
        else if (key.isKeyCode (KeyPress::leftKey))
        {
            if (owner != nullptr)
            {
                Component::SafePointer<Window> parentWindow (owner);
                PopupMenu::ItemComponent* currentChildOfParent = parentWindow->currentChild;

                hide (nullptr, true);

                if (parentWindow != nullptr)
                    parentWindow->setCurrentlyHighlightedChild (currentChildOfParent);

                disableTimerUntilMouseMoves();
            }
            else if (componentAttachedTo != nullptr)
            {
                componentAttachedTo->keyPressed (key);
            }
        }
        else if (key.isKeyCode (KeyPress::rightKey))
        {
            disableTimerUntilMouseMoves();

            if (showSubMenuFor (currentChild))
            {
                if (isSubMenuVisible())
                    activeSubMenu->selectNextItem (1);
            }
            else if (componentAttachedTo != nullptr)
            {
                componentAttachedTo->keyPressed (key);
            }
        }
        else if (key.isKeyCode (KeyPress::returnKey))
        {
            triggerCurrentlyHighlightedItem();
        }
        else if (key.isKeyCode (KeyPress::escapeKey))
        {
            dismissMenu (nullptr);
        }
        else
        {
            return false;
        }

        return true;
    }

    void inputAttemptWhenModal()
    {
        WeakReference<Component> deletionChecker (this);

        timerCallback();

        if (deletionChecker != nullptr && ! isOverAnyMenu())
        {
            if (componentAttachedTo != nullptr)
            {
                // we want to dismiss the menu, but if we do it synchronously, then
                // the mouse-click will be allowed to pass through. That's good, except
                // when the user clicks on the button that orginally popped the menu up,
                // as they'll expect the menu to go away, and in fact it'll just
                // come back. So only dismiss synchronously if they're not on the original
                // comp that we're attached to.
                const Point<int> mousePos (componentAttachedTo->getMouseXYRelative());

                if (componentAttachedTo->reallyContains (mousePos, true))
                {
                    postCommandMessage (PopupMenuSettings::dismissCommandId); // dismiss asynchrounously
                    return;
                }
            }

            dismissMenu (nullptr);
        }
    }

    void handleCommandMessage (int commandId)
    {
        Component::handleCommandMessage (commandId);

        if (commandId == PopupMenuSettings::dismissCommandId)
            dismissMenu (nullptr);
    }

    //==============================================================================
    void timerCallback()
    {
        if (! isVisible())
            return;

        if (componentAttachedTo != options.targetComponent)
        {
            dismissMenu (nullptr);
            return;
        }

        if (Window* currentlyModalWindow = dynamic_cast <Window*> (Component::getCurrentlyModalComponent()))
            if (! treeContains (currentlyModalWindow))
                return;

        startTimer (PopupMenuSettings::timerInterval);  // do this in case it was called from a mouse
                                                        // move rather than a real timer callback

        const Point<int> globalMousePos (Desktop::getMousePosition());
        const Point<int> localMousePos (getLocalPoint (nullptr, globalMousePos));

        const uint32 timeNow = Time::getMillisecondCounter();

        if (timeNow > timeEnteredCurrentChildComp + 100
             && reallyContains (localMousePos, true)
             && currentChild != nullptr
             && ! (disableMouseMoves || isSubMenuVisible()))
        {
            showSubMenuFor (currentChild);
        }

        highlightItemUnderMouse (globalMousePos, localMousePos, timeNow);

        const bool overScrollArea = scrollIfNecessary (localMousePos, timeNow);
        const bool wasDown = isDown;

        bool isOverAny = isOverAnyMenu();

        if (activeSubMenu != nullptr && hideOnExit && hasBeenOver && ! isOverAny)
        {
            activeSubMenu->updateMouseOverStatus (globalMousePos);
            isOverAny = isOverAnyMenu();
        }

        if (hideOnExit && hasBeenOver && ! isOverAny)
            hide (nullptr, true);
        else
            checkButtonState (localMousePos, timeNow, wasDown, overScrollArea, isOverAny);
    }

    static Array<Window*>& getActiveWindows()
    {
        static Array<Window*> activeMenuWindows;
        return activeMenuWindows;
    }

    //==============================================================================
private:
    Window* owner;
    const Options options;
    OwnedArray <PopupMenu::ItemComponent> items;
    Component::SafePointer<PopupMenu::ItemComponent> currentChild;
    ScopedPointer <Window> activeSubMenu;
    ApplicationCommandManager** managerOfChosenCommand;
    WeakReference<Component> componentAttachedTo;
    Rectangle<int> windowPos;
    Point<int> lastMousePos;
    bool isOver, hasBeenOver, isDown, needsToScroll;
    bool dismissOnMouseUp, hideOnExit, disableMouseMoves, hasAnyJuceCompHadFocus;
    int numColumns, contentHeight, childYOffset;
    Array<int> columnWidths;
    uint32 menuCreationTime, lastFocusedTime, lastScrollTime, lastMouseMoveTime, timeEnteredCurrentChildComp;
    double scrollAcceleration;

    //==============================================================================
    bool overlaps (const Rectangle<int>& r) const
    {
        return r.intersects (getBounds())
                || (owner != nullptr && owner->overlaps (r));
    }

    bool isOverAnyMenu() const
    {
        return owner != nullptr ? owner->isOverAnyMenu()
                                : isOverChildren();
    }

    bool isOverChildren() const
    {
        return isVisible()
                && (isOver || (activeSubMenu != nullptr && activeSubMenu->isOverChildren()));
    }

    void updateMouseOverStatus (const Point<int>& globalMousePos)
    {
        isOver = reallyContains (getLocalPoint (nullptr, globalMousePos), true);

        if (activeSubMenu != nullptr)
            activeSubMenu->updateMouseOverStatus (globalMousePos);
    }

    bool treeContains (const Window* const window) const noexcept
    {
        const Window* mw = this;

        while (mw->owner != nullptr)
            mw = mw->owner;

        while (mw != nullptr)
        {
            if (mw == window)
                return true;

            mw = mw->activeSubMenu;
        }

        return false;
    }

    bool doesAnyJuceCompHaveFocus()
    {
        bool anyFocused = Process::isForegroundProcess();

        if (anyFocused && Component::getCurrentlyFocusedComponent() == nullptr)
        {
            // because no component at all may have focus, our test here will
            // only be triggered when something has focus and then loses it.
            anyFocused = ! hasAnyJuceCompHadFocus;

            for (int i = ComponentPeer::getNumPeers(); --i >= 0;)
            {
                if (ComponentPeer::getPeer (i)->isFocused())
                {
                    anyFocused = true;
                    hasAnyJuceCompHadFocus = true;
                    break;
                }
            }
        }

        return anyFocused;
    }

    //==============================================================================
    void calculateWindowPos (const Rectangle<int>& target, const bool alignToRectangle)
    {
        const Rectangle<int> mon (Desktop::getInstance().getDisplays()
                                     .getDisplayContaining (target.getCentre())
                                                           #if JUCE_MAC
                                                            .userArea);
                                                           #else
                                                            .totalArea); // on windows, don't stop the menu overlapping the taskbar
                                                           #endif

        const int maxMenuHeight = mon.getHeight() - 24;

        int x, y, widthToUse, heightToUse;
        layoutMenuItems (mon.getWidth() - 24, maxMenuHeight, widthToUse, heightToUse);

        if (alignToRectangle)
        {
            x = target.getX();

            const int spaceUnder = mon.getHeight() - (target.getBottom() - mon.getY());
            const int spaceOver = target.getY() - mon.getY();

            if (heightToUse < spaceUnder - 30 || spaceUnder >= spaceOver)
                y = target.getBottom();
            else
                y = target.getY() - heightToUse;
        }
        else
        {
            bool tendTowardsRight = target.getCentreX() < mon.getCentreX();

            if (owner != nullptr)
            {
                if (owner->owner != nullptr)
                {
                    const bool ownerGoingRight = (owner->getX() + owner->getWidth() / 2
                                                    > owner->owner->getX() + owner->owner->getWidth() / 2);

                    if (ownerGoingRight && target.getRight() + widthToUse < mon.getRight() - 4)
                        tendTowardsRight = true;
                    else if ((! ownerGoingRight) && target.getX() > widthToUse + 4)
                        tendTowardsRight = false;
                }
                else if (target.getRight() + widthToUse < mon.getRight() - 32)
                {
                    tendTowardsRight = true;
                }
            }

            const int biggestSpace = jmax (mon.getRight() - target.getRight(),
                                           target.getX() - mon.getX()) - 32;

            if (biggestSpace < widthToUse)
            {
                layoutMenuItems (biggestSpace + target.getWidth() / 3, maxMenuHeight, widthToUse, heightToUse);

                if (numColumns > 1)
                    layoutMenuItems (biggestSpace - 4, maxMenuHeight, widthToUse, heightToUse);

                tendTowardsRight = (mon.getRight() - target.getRight()) >= (target.getX() - mon.getX());
            }

            if (tendTowardsRight)
                x = jmin (mon.getRight() - widthToUse - 4, target.getRight());
            else
                x = jmax (mon.getX() + 4, target.getX() - widthToUse);

            y = target.getY();
            if (target.getCentreY() > mon.getCentreY())
                y = jmax (mon.getY(), target.getBottom() - heightToUse);
        }

        x = jmax (mon.getX() + 1, jmin (mon.getRight() - (widthToUse + 6), x));
        y = jmax (mon.getY() + 1, jmin (mon.getBottom() - (heightToUse + 6), y));

        windowPos.setBounds (x, y, widthToUse, heightToUse);

        // sets this flag if it's big enough to obscure any of its parent menus
        hideOnExit = owner != nullptr
                      && owner->windowPos.intersects (windowPos.expanded (-4, -4));
    }

    void layoutMenuItems (const int maxMenuW, const int maxMenuH, int& width, int& height)
    {
        numColumns = 0;
        contentHeight = 0;
        int totalW;

        const int maximumNumColumns = options.maxColumns > 0 ? options.maxColumns : 7;

        do
        {
            ++numColumns;
            totalW = workOutBestSize (maxMenuW);

            if (totalW > maxMenuW)
            {
                numColumns = jmax (1, numColumns - 1);
                workOutBestSize (maxMenuW); // to update col widths
                break;
            }
            else if (totalW > maxMenuW / 2 || contentHeight < maxMenuH)
            {
                break;
            }

        } while (numColumns < maximumNumColumns);

        const int actualH = jmin (contentHeight, maxMenuH);

        needsToScroll = contentHeight > actualH;

        width = updateYPositions();
        height = actualH + PopupMenuSettings::borderSize * 2;
    }

    int workOutBestSize (const int maxMenuW)
    {
        int totalW = 0;
        contentHeight = 0;
        int childNum = 0;

        for (int col = 0; col < numColumns; ++col)
        {
            int colW = options.standardHeight, colH = 0;

            const int numChildren = jmin (items.size() - childNum,
                                          (items.size() + numColumns - 1) / numColumns);

            for (int i = numChildren; --i >= 0;)
            {
                colW = jmax (colW, items.getUnchecked (childNum + i)->getWidth());
                colH += items.getUnchecked (childNum + i)->getHeight();
            }

            colW = jmin (maxMenuW / jmax (1, numColumns - 2), colW + PopupMenuSettings::borderSize * 2);

            columnWidths.set (col, colW);
            totalW += colW;
            contentHeight = jmax (contentHeight, colH);

            childNum += numChildren;
        }

        if (totalW < options.minWidth)
        {
            totalW = options.minWidth;

            for (int col = 0; col < numColumns; ++col)
                columnWidths.set (0, totalW / numColumns);
        }

        return totalW;
    }

    void ensureItemIsVisible (const int itemID, int wantedY)
    {
        jassert (itemID != 0)

        for (int i = items.size(); --i >= 0;)
        {
            PopupMenu::ItemComponent* const m = items.getUnchecked(i);

            if (m != nullptr
                && m->itemInfo.itemID == itemID
                && windowPos.getHeight() > PopupMenuSettings::scrollZone * 4)
            {
                const int currentY = m->getY();

                if (wantedY > 0 || currentY < 0 || m->getBottom() > windowPos.getHeight())
                {
                    if (wantedY < 0)
                        wantedY = jlimit (PopupMenuSettings::scrollZone,
                                          jmax (PopupMenuSettings::scrollZone,
                                                windowPos.getHeight() - (PopupMenuSettings::scrollZone + m->getHeight())),
                                          currentY);

                    const Rectangle<int> mon (Desktop::getInstance().getDisplays()
                                                .getDisplayContaining (windowPos.getPosition()).userArea);

                    int deltaY = wantedY - currentY;

                    windowPos.setSize (jmin (windowPos.getWidth(), mon.getWidth()),
                                       jmin (windowPos.getHeight(), mon.getHeight()));

                    const int newY = jlimit (mon.getY(),
                                             mon.getBottom() - windowPos.getHeight(),
                                             windowPos.getY() + deltaY);

                    deltaY -= newY - windowPos.getY();

                    childYOffset -= deltaY;
                    windowPos.setPosition (windowPos.getX(), newY);

                    updateYPositions();
                }

                break;
            }
        }
    }

    void resizeToBestWindowPos()
    {
        Rectangle<int> r (windowPos);

        if (childYOffset < 0)
        {
            r = r.withTop (r.getY() - childYOffset);
        }
        else if (childYOffset > 0)
        {
            const int spaceAtBottom = r.getHeight() - (contentHeight - childYOffset);

            if (spaceAtBottom > 0)
                r.setSize (r.getWidth(), r.getHeight() - spaceAtBottom);
        }

        setBounds (r);
        updateYPositions();
    }

    void alterChildYPos (const int delta)
    {
        if (canScroll())
        {
            childYOffset += delta;

            if (delta < 0)
            {
                childYOffset = jmax (childYOffset, 0);
            }
            else if (delta > 0)
            {
                childYOffset = jmin (childYOffset,
                                     contentHeight - windowPos.getHeight() + PopupMenuSettings::borderSize);
            }

            updateYPositions();
        }
        else
        {
            childYOffset = 0;
        }

        resizeToBestWindowPos();
        repaint();
    }

    int updateYPositions()
    {
        int x = 0;
        int childNum = 0;

        for (int col = 0; col < numColumns; ++col)
        {
            const int numChildren = jmin (items.size() - childNum,
                                          (items.size() + numColumns - 1) / numColumns);

            const int colW = columnWidths [col];

            int y = PopupMenuSettings::borderSize - (childYOffset + (getY() - windowPos.getY()));

            for (int i = 0; i < numChildren; ++i)
            {
                Component* const c = items.getUnchecked (childNum + i);
                c->setBounds (x, y, colW, c->getHeight());
                y += c->getHeight();
            }

            x += colW;
            childNum += numChildren;
        }

        return x;
    }

    void setCurrentlyHighlightedChild (PopupMenu::ItemComponent* const child)
    {
        if (currentChild != nullptr)
            currentChild->setHighlighted (false);

        currentChild = child;

        if (currentChild != nullptr)
        {
            currentChild->setHighlighted (true);
            timeEnteredCurrentChildComp = Time::getApproximateMillisecondCounter();
        }
    }

    bool isSubMenuVisible() const noexcept          { return activeSubMenu != nullptr && activeSubMenu->isVisible(); }

    bool showSubMenuFor (PopupMenu::ItemComponent* const childComp)
    {
        activeSubMenu = nullptr;

        if (childComp != nullptr
             && childComp->itemInfo.hasActiveSubMenu())
        {
            activeSubMenu = new Window (*(childComp->itemInfo.subMenu), this,
                                        options.withTargetScreenArea (childComp->getScreenBounds())
                                               .withMinimumWidth (0)
                                               .withTargetComponent (nullptr),
                                        false, dismissOnMouseUp, managerOfChosenCommand);

            activeSubMenu->setVisible (true); // (must be called before enterModalState on Windows to avoid DropShadower confusion)
            activeSubMenu->enterModalState (false);
            activeSubMenu->toFront (false);
            return true;
        }

        return false;
    }

    void highlightItemUnderMouse (const Point<int>& globalMousePos, const Point<int>& localMousePos, const uint32 timeNow)
    {
        if (globalMousePos != lastMousePos || timeNow > lastMouseMoveTime + 350)
        {
            isOver = reallyContains (localMousePos, true);

            if (isOver)
                hasBeenOver = true;

            if (lastMousePos.getDistanceFrom (globalMousePos) > 2)
            {
                lastMouseMoveTime = timeNow;

                if (disableMouseMoves && isOver)
                    disableMouseMoves = false;
            }

            if (disableMouseMoves || (activeSubMenu != nullptr && activeSubMenu->isOverChildren()))
                return;

            bool isMovingTowardsMenu = false;

            if (isOver && (activeSubMenu != nullptr) && globalMousePos != lastMousePos)
            {
                // try to intelligently guess whether the user is moving the mouse towards a currently-open
                // submenu. To do this, look at whether the mouse stays inside a triangular region that
                // extends from the last mouse pos to the submenu's rectangle..

                float subX = (float) activeSubMenu->getScreenX();

                if (activeSubMenu->getX() > getX())
                {
                    lastMousePos -= Point<int> (2, 0);  // to enlarge the triangle a bit, in case the mouse only moves a couple of pixels
                }
                else
                {
                    lastMousePos += Point<int> (2, 0);
                    subX += activeSubMenu->getWidth();
                }

                Path areaTowardsSubMenu;
                areaTowardsSubMenu.addTriangle ((float) lastMousePos.x, (float) lastMousePos.y,
                                                subX, (float) activeSubMenu->getScreenY(),
                                                subX, (float) (activeSubMenu->getScreenY() + activeSubMenu->getHeight()));

                isMovingTowardsMenu = areaTowardsSubMenu.contains (globalMousePos.toFloat());
            }

            lastMousePos = globalMousePos;

            if (! isMovingTowardsMenu)
            {
                Component* c = getComponentAt (localMousePos);
                if (c == this)
                    c = nullptr;

                PopupMenu::ItemComponent* itemUnderMouse = dynamic_cast <PopupMenu::ItemComponent*> (c);

                if (itemUnderMouse == nullptr && c != nullptr)
                    itemUnderMouse = c->findParentComponentOfClass<PopupMenu::ItemComponent>();

                if (itemUnderMouse != currentChild
                      && (isOver || (activeSubMenu == nullptr) || ! activeSubMenu->isVisible()))
                {
                    if (isOver && (c != nullptr) && (activeSubMenu != nullptr))
                        activeSubMenu->hide (nullptr, true);

                    if (! isOver)
                        itemUnderMouse = nullptr;

                    setCurrentlyHighlightedChild (itemUnderMouse);
                }
            }
        }
    }

    void checkButtonState (const Point<int>& localMousePos, const uint32 timeNow,
                           const bool wasDown, const bool overScrollArea, const bool isOverAny)
    {
        isDown = hasBeenOver
                    && (ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown()
                         || ModifierKeys::getCurrentModifiersRealtime().isAnyMouseButtonDown());

        if (! doesAnyJuceCompHaveFocus())
        {
            if (timeNow > lastFocusedTime + 10)
            {
                PopupMenuSettings::menuWasHiddenBecauseOfAppChange = true;
                dismissMenu (nullptr);
                // Note: this object may have been deleted by the previous call..
            }
        }
        else if (wasDown && timeNow > menuCreationTime + 250
                   && ! (isDown || overScrollArea))
        {
            isOver = reallyContains (localMousePos, true);

            if (isOver)
                triggerCurrentlyHighlightedItem();
            else if ((hasBeenOver || ! dismissOnMouseUp) && ! isOverAny)
                dismissMenu (nullptr);

            // Note: this object may have been deleted by the previous call..
        }
        else
        {
            lastFocusedTime = timeNow;
        }
    }

    void triggerCurrentlyHighlightedItem()
    {
        if (currentChild != nullptr
             && currentChild->itemInfo.canBeTriggered()
             && (currentChild->itemInfo.customComp == nullptr
                  || currentChild->itemInfo.customComp->isTriggeredAutomatically()))
        {
            dismissMenu (&currentChild->itemInfo);
        }
    }

    void selectNextItem (const int delta)
    {
        disableTimerUntilMouseMoves();
        PopupMenu::ItemComponent* mic = nullptr;
        bool wasLastOne = (currentChild == nullptr);
        const int numItems = items.size();

        for (int i = 0; i < numItems + 1; ++i)
        {
            int index = (delta > 0) ? i : (numItems - 1 - i);
            index = (index + numItems) % numItems;

            mic = items.getUnchecked (index);

            if (mic != nullptr && (mic->itemInfo.canBeTriggered() || mic->itemInfo.hasActiveSubMenu())
                 && wasLastOne)
                break;

            if (mic == currentChild)
                wasLastOne = true;
        }

        setCurrentlyHighlightedChild (mic);
    }

    void disableTimerUntilMouseMoves()
    {
        disableMouseMoves = true;

        if (owner != nullptr)
            owner->disableTimerUntilMouseMoves();
    }

    bool canScroll() const noexcept                 { return childYOffset != 0 || needsToScroll; }
    bool isTopScrollZoneActive() const noexcept     { return canScroll() && childYOffset > 0; }
    bool isBottomScrollZoneActive() const noexcept  { return canScroll() && childYOffset < contentHeight - windowPos.getHeight(); }

    bool scrollIfNecessary (const Point<int>& localMousePos, const uint32 timeNow)
    {
        if (canScroll()
             && (isOver || (isDown && isPositiveAndBelow (localMousePos.x, getWidth()))))
        {
            if (isTopScrollZoneActive() && localMousePos.y < PopupMenuSettings::scrollZone)
                return scroll (timeNow, -1);

            if (isBottomScrollZoneActive() && localMousePos.y > getHeight() - PopupMenuSettings::scrollZone)
                return scroll (timeNow, 1);
        }

        scrollAcceleration = 1.0;
        return false;
    }

    bool scroll (const uint32 timeNow, const int direction)
    {
        if (timeNow > lastScrollTime + 20)
        {
            scrollAcceleration = jmin (4.0, scrollAcceleration * 1.04);
            int amount = 0;

            for (int i = 0; i < items.size() && amount == 0; ++i)
                amount = ((int) scrollAcceleration) * items.getUnchecked(i)->getHeight();

            alterChildYPos (amount * direction);
            lastScrollTime = timeNow;
        }

        lastMousePos = Point<int> (-1, -1); // to trigger a mouse-move
        return true;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Window)
};


//==============================================================================
PopupMenu::PopupMenu()
    : lookAndFeel (nullptr)
{
}

PopupMenu::PopupMenu (const PopupMenu& other)
    : lookAndFeel (other.lookAndFeel)
{
    items.addCopiesOf (other.items);
}

PopupMenu& PopupMenu::operator= (const PopupMenu& other)
{
    if (this != &other)
    {
        lookAndFeel = other.lookAndFeel;

        clear();
        items.addCopiesOf (other.items);
    }

    return *this;
}

#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
PopupMenu::PopupMenu (PopupMenu&& other) noexcept
    : lookAndFeel (other.lookAndFeel)
{
    items.swapWithArray (other.items);
}

PopupMenu& PopupMenu::operator= (PopupMenu&& other) noexcept
{
    jassert (this != &other); // hopefully the compiler should make this situation impossible!

    items.swapWithArray (other.items);
    lookAndFeel = other.lookAndFeel;
    return *this;
}
#endif

PopupMenu::~PopupMenu()
{
}

void PopupMenu::clear()
{
    items.clear();
}

void PopupMenu::addItem (const int itemResultID, const String& itemText,
                         const bool isActive, const bool isTicked, const Image& iconToUse)
{
    jassert (itemResultID != 0);    // 0 is used as a return value to indicate that the user
                                    // didn't pick anything, so you shouldn't use it as the id
                                    // for an item..

    items.add (new Item (itemResultID, itemText, isActive, isTicked, iconToUse,
                         Colours::black, false, nullptr, nullptr, nullptr));
}

void PopupMenu::addCommandItem (ApplicationCommandManager* commandManager,
                                const int commandID,
                                const String& displayName)
{
    jassert (commandManager != nullptr && commandID != 0);

    if (const ApplicationCommandInfo* const registeredInfo = commandManager->getCommandForID (commandID))
    {
        ApplicationCommandInfo info (*registeredInfo);
        ApplicationCommandTarget* const target = commandManager->getTargetForCommand (commandID, info);

        items.add (new Item (commandID,
                             displayName.isNotEmpty() ? displayName
                                                      : info.shortName,
                             target != nullptr && (info.flags & ApplicationCommandInfo::isDisabled) == 0,
                             (info.flags & ApplicationCommandInfo::isTicked) != 0,
                             Image::null,
                             Colours::black,
                             false,
                             nullptr, nullptr,
                             commandManager));
    }
}

void PopupMenu::addColouredItem (const int itemResultID,
                                 const String& itemText,
                                 const Colour& itemTextColour,
                                 const bool isActive,
                                 const bool isTicked,
                                 const Image& iconToUse)
{
    jassert (itemResultID != 0);    // 0 is used as a return value to indicate that the user
                                    // didn't pick anything, so you shouldn't use it as the id
                                    // for an item..

    items.add (new Item (itemResultID, itemText, isActive, isTicked, iconToUse,
                         itemTextColour, true, nullptr, nullptr, nullptr));
}

//==============================================================================
class PopupMenu::NormalComponentWrapper : public PopupMenu::CustomComponent
{
public:
    NormalComponentWrapper (Component* const comp, const int w, const int h,
                            const bool triggerMenuItemAutomaticallyWhenClicked)
        : PopupMenu::CustomComponent (triggerMenuItemAutomaticallyWhenClicked),
          width (w), height (h)
    {
        addAndMakeVisible (comp);
    }

    void getIdealSize (int& idealWidth, int& idealHeight)
    {
        idealWidth = width;
        idealHeight = height;
    }

    void resized()
    {
        if (Component* const child = getChildComponent(0))
            child->setBounds (getLocalBounds());
    }

private:
    const int width, height;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NormalComponentWrapper)
};

void PopupMenu::addCustomItem (const int itemID, CustomComponent* const cc, const PopupMenu* subMenu)
{
    jassert (itemID != 0);    // 0 is used as a return value to indicate that the user
                              // didn't pick anything, so you shouldn't use it as the id
                              // for an item..

    items.add (new Item (itemID, String::empty, true, false, Image::null,
                         Colours::black, false, cc, subMenu, nullptr));
}

void PopupMenu::addCustomItem (const int itemResultID,
                               Component* customComponent,
                               int idealWidth, int idealHeight,
                               const bool triggerMenuItemAutomaticallyWhenClicked,
                               const PopupMenu* subMenu)
{
    items.add (new Item (itemResultID, String::empty, true, false, Image::null,
                         Colours::black, false,
                         new NormalComponentWrapper (customComponent, idealWidth, idealHeight,
                                                     triggerMenuItemAutomaticallyWhenClicked),
                         subMenu, nullptr));
}

//==============================================================================
void PopupMenu::addSubMenu (const String& subMenuName,
                            const PopupMenu& subMenu,
                            const bool isActive,
                            const Image& iconToUse,
                            const bool isTicked,
                            const int itemResultID)
{
    items.add (new Item (itemResultID, subMenuName, isActive && (itemResultID != 0 || subMenu.getNumItems() > 0), isTicked,
                         iconToUse, Colours::black, false, nullptr, &subMenu, nullptr));
}

void PopupMenu::addSeparator()
{
    if (items.size() > 0 && ! items.getLast()->isSeparator)
        items.add (new Item());
}

//==============================================================================
class PopupMenu::HeaderItemComponent  : public PopupMenu::CustomComponent
{
public:
    HeaderItemComponent (const String& name)
        : PopupMenu::CustomComponent (false)
    {
        setName (name);
    }

    void paint (Graphics& g)
    {
        g.setFont (getLookAndFeel().getPopupMenuFont().boldened());
        g.setColour (findColour (PopupMenu::headerTextColourId));

        g.drawFittedText (getName(),
                          12, 0, getWidth() - 16, proportionOfHeight (0.8f),
                          Justification::bottomLeft, 1);
    }

    void getIdealSize (int& idealWidth, int& idealHeight)
    {
        getLookAndFeel().getIdealPopupMenuItemSize (getName(), false, -1, idealWidth, idealHeight);
        idealHeight += idealHeight / 2;
        idealWidth += idealWidth / 4;
    }

private:
    JUCE_LEAK_DETECTOR (HeaderItemComponent)
};

void PopupMenu::addSectionHeader (const String& title)
{
    addCustomItem (0X4734a34f, new HeaderItemComponent (title));
}

//==============================================================================
PopupMenu::Options::Options()
    : targetComponent (nullptr),
      visibleItemID (0),
      minWidth (0),
      maxColumns (0),
      standardHeight (0)
{
    targetArea.setPosition (Desktop::getMousePosition());
}

PopupMenu::Options PopupMenu::Options::withTargetComponent (Component* comp) const noexcept
{
    Options o (*this);
    o.targetComponent = comp;

    if (comp != nullptr)
        o.targetArea = comp->getScreenBounds();

    return o;
}

PopupMenu::Options PopupMenu::Options::withTargetScreenArea (const Rectangle<int>& area) const noexcept
{
    Options o (*this);
    o.targetArea = area;
    return o;
}

PopupMenu::Options PopupMenu::Options::withMinimumWidth (int w) const noexcept
{
    Options o (*this);
    o.minWidth = w;
    return o;
}

PopupMenu::Options PopupMenu::Options::withMaximumNumColumns (int cols) const noexcept
{
    Options o (*this);
    o.maxColumns = cols;
    return o;
}

PopupMenu::Options PopupMenu::Options::withStandardItemHeight (int height) const noexcept
{
    Options o (*this);
    o.standardHeight = height;
    return o;
}

PopupMenu::Options PopupMenu::Options::withItemThatMustBeVisible (int idOfItemToBeVisible) const noexcept
{
    Options o (*this);
    o.visibleItemID = idOfItemToBeVisible;
    return o;
}

Component* PopupMenu::createWindow (const Options& options,
                                    ApplicationCommandManager** managerOfChosenCommand) const
{
    if (items.size() > 0)
        return new Window (*this, nullptr, options,
                           ! options.targetArea.isEmpty(),
                           ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown(),
                           managerOfChosenCommand);

    return nullptr;
}

//==============================================================================
// This invokes any command manager commands and deletes the menu window when it is dismissed
class PopupMenuCompletionCallback  : public ModalComponentManager::Callback
{
public:
    PopupMenuCompletionCallback()
        : managerOfChosenCommand (nullptr),
          prevFocused (Component::getCurrentlyFocusedComponent()),
          prevTopLevel (prevFocused != nullptr ? prevFocused->getTopLevelComponent() : nullptr)
    {
        PopupMenuSettings::menuWasHiddenBecauseOfAppChange = false;
    }

    void modalStateFinished (int result)
    {
        if (managerOfChosenCommand != nullptr && result != 0)
        {
            ApplicationCommandTarget::InvocationInfo info (result);
            info.invocationMethod = ApplicationCommandTarget::InvocationInfo::fromMenu;

            managerOfChosenCommand->invoke (info, true);
        }

        // (this would be the place to fade out the component, if that's what's required)
        component = nullptr;

        if (! PopupMenuSettings::menuWasHiddenBecauseOfAppChange)
        {
            if (prevTopLevel != nullptr)
                prevTopLevel->toFront (true);

            if (prevFocused != nullptr)
                prevFocused->grabKeyboardFocus();
        }
    }

    ApplicationCommandManager* managerOfChosenCommand;
    ScopedPointer<Component> component;
    WeakReference<Component> prevFocused, prevTopLevel;

private:
    JUCE_DECLARE_NON_COPYABLE (PopupMenuCompletionCallback)
};

int PopupMenu::showWithOptionalCallback (const Options& options, ModalComponentManager::Callback* const userCallback,
                                         const bool canBeModal)
{
    ScopedPointer<ModalComponentManager::Callback> userCallbackDeleter (userCallback);
    ScopedPointer<PopupMenuCompletionCallback> callback (new PopupMenuCompletionCallback());

    Component* window = createWindow (options, &(callback->managerOfChosenCommand));
    if (window == nullptr)
        return 0;

    callback->component = window;

    window->setVisible (true); // (must be called before enterModalState on Windows to avoid DropShadower confusion)
    window->enterModalState (false, userCallbackDeleter.release());
    ModalComponentManager::getInstance()->attachCallback (window, callback.release());

    window->toFront (false);  // need to do this after making it modal, or it could
                              // be stuck behind other comps that are already modal..

   #if JUCE_MODAL_LOOPS_PERMITTED
    return (userCallback == nullptr && canBeModal) ? window->runModalLoop() : 0;
   #else
    jassert (! (userCallback == nullptr && canBeModal));
    return 0;
   #endif
}

//==============================================================================
#if JUCE_MODAL_LOOPS_PERMITTED
int PopupMenu::showMenu (const Options& options)
{
    return showWithOptionalCallback (options, nullptr, true);
}
#endif

void PopupMenu::showMenuAsync (const Options& options, ModalComponentManager::Callback* userCallback)
{
   #if ! JUCE_MODAL_LOOPS_PERMITTED
    jassert (userCallback != nullptr);
   #endif

    showWithOptionalCallback (options, userCallback, false);
}

//==============================================================================
#if JUCE_MODAL_LOOPS_PERMITTED
int PopupMenu::show (const int itemIDThatMustBeVisible,
                     const int minimumWidth, const int maximumNumColumns,
                     const int standardItemHeight,
                     ModalComponentManager::Callback* callback)
{
    return showWithOptionalCallback (Options().withItemThatMustBeVisible (itemIDThatMustBeVisible)
                                              .withMinimumWidth (minimumWidth)
                                              .withMaximumNumColumns (maximumNumColumns)
                                              .withStandardItemHeight (standardItemHeight),
                                     callback, true);
}

int PopupMenu::showAt (const Rectangle<int>& screenAreaToAttachTo,
                       const int itemIDThatMustBeVisible,
                       const int minimumWidth, const int maximumNumColumns,
                       const int standardItemHeight,
                       ModalComponentManager::Callback* callback)
{
    return showWithOptionalCallback (Options().withTargetScreenArea (screenAreaToAttachTo)
                                              .withItemThatMustBeVisible (itemIDThatMustBeVisible)
                                              .withMinimumWidth (minimumWidth)
                                              .withMaximumNumColumns (maximumNumColumns)
                                              .withStandardItemHeight (standardItemHeight),
                                     callback, true);
}

int PopupMenu::showAt (Component* componentToAttachTo,
                       const int itemIDThatMustBeVisible,
                       const int minimumWidth, const int maximumNumColumns,
                       const int standardItemHeight,
                       ModalComponentManager::Callback* callback)
{
    Options options (Options().withItemThatMustBeVisible (itemIDThatMustBeVisible)
                              .withMinimumWidth (minimumWidth)
                              .withMaximumNumColumns (maximumNumColumns)
                              .withStandardItemHeight (standardItemHeight));

    if (componentToAttachTo != nullptr)
        options = options.withTargetComponent (componentToAttachTo);

    return showWithOptionalCallback (options, callback, true);
}
#endif

bool JUCE_CALLTYPE PopupMenu::dismissAllActiveMenus()
{
    const Array<Window*>& windows = Window::getActiveWindows();
    const int numWindows = windows.size();

    for (int i = numWindows; --i >= 0;)
        if (Window* const pmw = windows[i])
            pmw->dismissMenu (nullptr);

    return numWindows > 0;
}

//==============================================================================
int PopupMenu::getNumItems() const noexcept
{
    int num = 0;

    for (int i = items.size(); --i >= 0;)
        if (! items.getUnchecked(i)->isSeparator)
            ++num;

    return num;
}

bool PopupMenu::containsCommandItem (const int commandID) const
{
    for (int i = items.size(); --i >= 0;)
    {
        const Item& mi = *items.getUnchecked (i);

        if ((mi.itemID == commandID && mi.commandManager != nullptr)
             || (mi.subMenu != nullptr && mi.subMenu->containsCommandItem (commandID)))
        {
            return true;
        }
    }

    return false;
}

bool PopupMenu::containsAnyActiveItems() const noexcept
{
    for (int i = items.size(); --i >= 0;)
    {
        const Item& mi = *items.getUnchecked (i);

        if (mi.subMenu != nullptr)
        {
            if (mi.subMenu->containsAnyActiveItems())
                return true;
        }
        else if (mi.isActive)
        {
            return true;
        }
    }

    return false;
}

void PopupMenu::setLookAndFeel (LookAndFeel* const newLookAndFeel)
{
    lookAndFeel = newLookAndFeel;
}

//==============================================================================
PopupMenu::CustomComponent::CustomComponent (bool autoTrigger)
    : isHighlighted (false),
      triggeredAutomatically (autoTrigger)
{
}

PopupMenu::CustomComponent::~CustomComponent()
{
}

void PopupMenu::CustomComponent::setHighlighted (bool shouldBeHighlighted)
{
    isHighlighted = shouldBeHighlighted;
    repaint();
}

void PopupMenu::CustomComponent::triggerMenuItem()
{
    if (PopupMenu::ItemComponent* const mic = dynamic_cast <PopupMenu::ItemComponent*> (getParentComponent()))
    {
        if (PopupMenu::Window* const pmw = dynamic_cast <PopupMenu::Window*> (mic->getParentComponent()))
        {
            pmw->dismissMenu (&mic->itemInfo);
        }
        else
        {
            // something must have gone wrong with the component hierarchy if this happens..
            jassertfalse;
        }
    }
    else
    {
        // why isn't this component inside a menu? Not much point triggering the item if
        // there's no menu.
        jassertfalse;
    }
}

//==============================================================================
PopupMenu::MenuItemIterator::MenuItemIterator (const PopupMenu& m)
    : subMenu (nullptr),
      itemId (0),
      isSeparator (false),
      isTicked (false),
      isEnabled (false),
      isCustomComponent (false),
      isSectionHeader (false),
      customColour (nullptr),
      menu (m),
      index (0)
{
}

PopupMenu::MenuItemIterator::~MenuItemIterator()
{
}

bool PopupMenu::MenuItemIterator::next()
{
    if (index >= menu.items.size())
        return false;

    const Item* const item = menu.items.getUnchecked (index);
    ++index;

    if (item->isSeparator && index >= menu.items.size()) // (avoid showing a separator at the end)
        return false;

    itemName        = item->customComp != nullptr ? item->customComp->getName() : item->text;
    subMenu         = item->subMenu;
    itemId          = item->itemID;
    isSeparator     = item->isSeparator;
    isTicked        = item->isTicked;
    isEnabled       = item->isActive;
    isSectionHeader = dynamic_cast <HeaderItemComponent*> (static_cast <CustomComponent*> (item->customComp)) != nullptr;
    isCustomComponent = (! isSectionHeader) && item->customComp != nullptr;
    customColour    = item->usesColour ? &(item->textColour) : nullptr;
    customImage     = item->image;
    commandManager  = item->commandManager;

    return true;
}

void PopupMenu::MenuItemIterator::addItemTo (PopupMenu& targetMenu)
{
    targetMenu.items.add (new Item (itemId, itemName, isEnabled, isTicked, customImage,
                                    customColour != nullptr ? *customColour : Colours::black, customColour != nullptr,
                                    nullptr,
                                    subMenu, commandManager));
}
