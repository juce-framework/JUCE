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

// Some things to keep in mind when modifying this file:
// - Popup menus may be free-floating or parented. Make sure to test both!
// - Menus may open while the mouse button is down, in which case the following mouse-up may
//   trigger a hovered menu item if the mouse has moved since the menu was displayed.
// - Consider a long menu attached to a button. It's possible for a such a menu to open underneath
//   the mouse cursor. In this case, the menu item underneath the mouse should *not* be initially
//   selected or clickable. Instead, wait until the mouse cursor is moved, which we interpret as the
//   user signalling intent to trigger a menu item.
// - Menu items may be navigated with the cursor keys. The most recent input mechanism should
//   generally win, so pressing a cursor key should cause the mouse state to be ignored until
//   the mouse is next moved.
// - It's possible for menus to overlap, especially in the case of nested submenus. Of course,
//   clicking an overlapping menu should only trigger the topmost menu item.
// - Long menus must update properly when the mouse is completely stationary inside the scroll area
//   at the end of the menu. This means it's not sufficient to drive all menu updates from mouse
//   and keyboard input callbacks. Scrolling must be driven by some other periodic update mechanism
//   such as a timer.

namespace juce
{

namespace PopupMenuSettings
{
    const int scrollZone = 24;
    const int dismissCommandId = 0x6287345f;

    static bool menuWasHiddenBecauseOfAppChange = false;
}

//==============================================================================
struct PopupMenu::HelperClasses
{

class MouseSourceState;
struct MenuWindow;

static bool canBeTriggered (const PopupMenu::Item& item) noexcept
{
    return item.isEnabled
        && item.itemID != 0
        && ! item.isSectionHeader
        && (item.customComponent == nullptr || item.customComponent->isTriggeredAutomatically());
}

static bool hasActiveSubMenu (const PopupMenu::Item& item) noexcept
{
    return item.isEnabled
        && item.subMenu != nullptr
        && item.subMenu->items.size() > 0;
}

//==============================================================================
struct HeaderItemComponent final : public PopupMenu::CustomComponent
{
    HeaderItemComponent (const String& name, const Options& opts)
        : CustomComponent (false), options (opts)
    {
        setName (name);
    }

    void paint (Graphics& g) override
    {
        getLookAndFeel().drawPopupMenuSectionHeaderWithOptions (g,
                                                                getLocalBounds(),
                                                                getName(),
                                                                options);
    }

    void getIdealSize (int& idealWidth, int& idealHeight) override
    {
        getLookAndFeel().getIdealPopupMenuSectionHeaderSizeWithOptions (getName(),
                                                                        -1,
                                                                        idealWidth,
                                                                        idealHeight,
                                                                        options);
    }

    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return createIgnoredAccessibilityHandler (*this);
    }

    const Options& options;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HeaderItemComponent)
};

//==============================================================================
struct ItemComponent final : public Component
{
    ItemComponent (const PopupMenu::Item& i, const PopupMenu::Options& o, MenuWindow& parent)
        : item (i), parentWindow (parent), options (o), customComp (i.customComponent)
    {
        if (item.isSectionHeader)
        {
            customComp = *new HeaderItemComponent (item.text, options);
            setEnabled (false);
        }

        if (customComp != nullptr)
        {
            setItem (*customComp, &item);
            addAndMakeVisible (*customComp);
        }

        parent.addAndMakeVisible (this);

        updateShortcutKeyDescription();

        int itemW = 80;
        int itemH = 16;
        getIdealSize (itemW, itemH, options.getStandardItemHeight());
        setSize (itemW, jlimit (1, 600, itemH));

        addMouseListener (&parent, false);
    }

    ~ItemComponent() override
    {
        if (customComp != nullptr)
            setItem (*customComp, nullptr);

        removeChildComponent (customComp.get());
    }

    void getIdealSize (int& idealWidth, int& idealHeight, const int standardItemHeight)
    {
        if (customComp != nullptr)
            customComp->getIdealSize (idealWidth, idealHeight);
        else
            getLookAndFeel().getIdealPopupMenuItemSizeWithOptions (getTextForMeasurement(),
                                                                   item.isSeparator,
                                                                   standardItemHeight,
                                                                   idealWidth, idealHeight,
                                                                   options);
    }

    void paint (Graphics& g) override
    {
        if (customComp == nullptr)
            getLookAndFeel().drawPopupMenuItemWithOptions (g, getLocalBounds(),
                                                           isHighlighted,
                                                           item,
                                                           options);
    }

    void resized() override
    {
        if (auto* child = getChildComponent (0))
        {
            const auto border = getLookAndFeel().getPopupMenuBorderSizeWithOptions (options);
            child->setBounds (getLocalBounds().reduced (border, 0));
        }
    }

    void setHighlighted (bool shouldBeHighlighted)
    {
        shouldBeHighlighted = shouldBeHighlighted && item.isEnabled;

        if (isHighlighted != shouldBeHighlighted)
        {
            isHighlighted = shouldBeHighlighted;

            if (customComp != nullptr)
                customComp->setHighlighted (shouldBeHighlighted);

            if (isHighlighted)
                if (auto* handler = getAccessibilityHandler())
                    handler->grabFocus();

            repaint();
        }
    }

    static bool isAccessibilityHandlerRequired (const PopupMenu::Item& item)
    {
        return item.isSectionHeader || hasActiveSubMenu (item) || canBeTriggered (item);
    }

    PopupMenu::Item item;

private:
    //==============================================================================
    class ItemAccessibilityHandler final : public AccessibilityHandler
    {
    public:
        explicit ItemAccessibilityHandler (ItemComponent& itemComponentToWrap)
            : AccessibilityHandler (itemComponentToWrap,
                                    isAccessibilityHandlerRequired (itemComponentToWrap.item) ? AccessibilityRole::menuItem
                                                                                              : AccessibilityRole::ignored,
                                    getAccessibilityActions (*this, itemComponentToWrap)),
              itemComponent (itemComponentToWrap)
        {
        }

        String getTitle() const override
        {
            return itemComponent.item.text;
        }

        AccessibleState getCurrentState() const override
        {
            auto state = AccessibilityHandler::getCurrentState().withSelectable()
                                                                .withAccessibleOffscreen();

            if (hasActiveSubMenu (itemComponent.item))
            {
                state = itemComponent.parentWindow.isSubMenuVisible() ? state.withExpandable().withExpanded()
                                                                      : state.withExpandable().withCollapsed();
            }

            if (itemComponent.item.isTicked)
                state = state.withCheckable().withChecked();

            return state.isFocused() ? state.withSelected() : state;
        }

    private:
        static AccessibilityActions getAccessibilityActions (ItemAccessibilityHandler&,
                                                             ItemComponent& item)
        {
            auto onFocus = [&item]
            {
                item.parentWindow.disableMouseMovesOnMenuAndAncestors();
                item.parentWindow.ensureItemComponentIsVisible (item, std::nullopt);
                item.parentWindow.setCurrentlyHighlightedChild (&item);
            };

            auto actions = AccessibilityActions().addAction (AccessibilityActionType::focus, std::move (onFocus));

            if (canBeTriggered (item.item))
            {
                actions.addAction (AccessibilityActionType::press, [&item]
                {
                    item.parentWindow.setCurrentlyHighlightedChild (&item);
                    item.parentWindow.triggerCurrentlyHighlightedItem();
                });
            }

            if (hasActiveSubMenu (item.item))
            {
                auto showSubMenu = [&item]
                {
                    item.parentWindow.showSubMenuFor (&item);

                    if (auto* subMenu = item.parentWindow.activeSubMenu.get())
                        subMenu->setCurrentlyHighlightedChild (subMenu->items.getFirst());
                };

                actions.addAction (AccessibilityActionType::press,    showSubMenu);
                actions.addAction (AccessibilityActionType::showMenu, showSubMenu);
            }

            return actions;
        }

        ItemComponent& itemComponent;
    };

    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return item.isSeparator ? createIgnoredAccessibilityHandler (*this)
                                : std::make_unique<ItemAccessibilityHandler> (*this);
    }

    //==============================================================================
    MenuWindow& parentWindow;
    const PopupMenu::Options& options;
    // NB: we use a copy of the one from the item info in case we're using our own section comp
    ReferenceCountedObjectPtr<CustomComponent> customComp;
    bool isHighlighted = false;

    void updateShortcutKeyDescription()
    {
        if (item.commandManager != nullptr
             && item.itemID != 0
             && item.shortcutKeyDescription.isEmpty())
        {
            String shortcutKey;

            for (auto& keypress : item.commandManager->getKeyMappings()
                                    ->getKeyPressesAssignedToCommand (item.itemID))
            {
                auto key = keypress.getTextDescriptionWithIcons();

                if (shortcutKey.isNotEmpty())
                    shortcutKey << ", ";

                if (key.length() == 1 && key[0] < 128)
                    shortcutKey << "shortcut: '" << key << '\'';
                else
                    shortcutKey << key;
            }

            item.shortcutKeyDescription = shortcutKey.trim();
        }
    }

    String getTextForMeasurement() const
    {
        return item.shortcutKeyDescription.isNotEmpty() ? item.text + "   " + item.shortcutKeyDescription
                                                        : item.text;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ItemComponent)
};

//==============================================================================
struct MenuWindow final : public Component
{
    MenuWindow (const PopupMenu& menu,
                MenuWindow* parentWindow,
                Options opts,
                bool alignToRectangle,
                ApplicationCommandManager** manager,
                float parentScaleFactor = 1.0f)
        : Component ("menu"),
          parent (parentWindow),
          options (opts.withParentComponent (findNonNullLookAndFeel (menu, parentWindow).getParentComponentForMenuOptions (opts))),
          managerOfChosenCommand (manager),
          componentAttachedTo (options.getTargetComponent()),
          windowCreationTime (Time::getMillisecondCounter()),
          lastFocusedTime (windowCreationTime),
          timeEnteredCurrentChildComp (windowCreationTime),
          scaleFactor (parentWindow != nullptr ? parentScaleFactor : 1.0f)
    {
        setWantsKeyboardFocus (false);
        setMouseClickGrabsKeyboardFocus (false);
        setAlwaysOnTop (true);
        setFocusContainerType (FocusContainerType::focusContainer);

        setLookAndFeel (findLookAndFeel (menu, parentWindow));

        auto& lf = getLookAndFeel();

        if (auto* pc = options.getParentComponent())
        {
            pc->addChildComponent (this);
        }
        else
        {
            const auto shouldDisableAccessibility = [this]
            {
                const auto* compToCheck = parent != nullptr ? parent
                                                            : options.getTargetComponent();

                return compToCheck != nullptr && ! compToCheck->isAccessible();
            }();

            if (shouldDisableAccessibility)
                setAccessible (false);

            addToDesktop (ComponentPeer::windowIsTemporary
                          | ComponentPeer::windowIgnoresKeyPresses
                          | lf.getMenuWindowFlags());
        }

        // Using a global mouse listener means that we get notifications about all mouse events.
        // Without this, drags that are started on a button that displays a menu won't reach the
        // menu, because they *only* target the component that initiated the drag interaction.
        Desktop::getInstance().addGlobalMouseListener (this);

        if (options.getParentComponent() == nullptr && parentWindow == nullptr && lf.shouldPopupMenuScaleWithTargetComponent (options))
            if (auto* targetComponent = options.getTargetComponent())
                scaleFactor = Component::getApproximateScaleFactorForComponent (targetComponent);

        setOpaque (lf.findColour (PopupMenu::backgroundColourId).isOpaque()
                     || ! Desktop::canUseSemiTransparentWindows());

        const auto initialSelectedId = options.getInitiallySelectedItemId();

        for (int i = 0; i < menu.items.size(); ++i)
        {
            auto& item = menu.items.getReference (i);

            if (i + 1 < menu.items.size() || ! item.isSeparator)
            {
                auto* child = items.add (new ItemComponent (item, options, *this));
                child->setExplicitFocusOrder (1 + i);

                if (initialSelectedId != 0 && item.itemID == initialSelectedId)
                    setCurrentlyHighlightedChild (child);
            }
        }

        auto targetArea = options.getTargetScreenArea() / scaleFactor;

        calculateWindowPos (targetArea, alignToRectangle);
        setTopLeftPosition (windowPos.getPosition());

        if (auto visibleID = options.getItemThatMustBeVisible())
        {
            const auto iter = std::find_if (items.begin(), items.end(), [&] (auto* item)
            {
                return item->item.itemID == visibleID;
            });

            if (iter != items.end())
            {
                const auto targetPosition = std::invoke ([&]
                {
                    if (auto* pc = options.getParentComponent())
                        return pc->getLocalPoint (nullptr, targetArea.getTopLeft());

                    return targetArea.getTopLeft();
                });

                ensureItemComponentIsVisible (**iter, targetPosition.getY() - windowPos.getY());
            }
        }

        resizeToBestWindowPos();

        getActiveWindows().add (this);
        lf.preparePopupMenuWindow (*this);

        getMouseState (Desktop::getInstance().getMainMouseSource()); // forces creation of a mouse source watcher for the main mouse
    }

    ~MenuWindow() override
    {
        getActiveWindows().removeFirstMatchingValue (this);
        Desktop::getInstance().removeGlobalMouseListener (this);
        activeSubMenu.reset();
        items.clear();
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        if (isOpaque())
            g.fillAll (Colours::white);

        auto& theme = getLookAndFeel();
        theme.drawPopupMenuBackgroundWithOptions (g, getWidth(), getHeight(), options);

        if (columnWidths.isEmpty())
            return;

        const auto separatorWidth = theme.getPopupMenuColumnSeparatorWidthWithOptions (options);
        const auto border = theme.getPopupMenuBorderSizeWithOptions (options);

        auto currentX = 0;

        std::for_each (columnWidths.begin(), std::prev (columnWidths.end()), [&] (int width)
        {
            const Rectangle<int> separator (currentX + width,
                                            border,
                                            separatorWidth,
                                            getHeight() - border * 2);
            theme.drawPopupMenuColumnSeparatorWithOptions (g, separator, options);
            currentX += width + separatorWidth;
        });
    }

    void paintOverChildren (Graphics& g) override
    {
        auto& lf = getLookAndFeel();

        if (options.getParentComponent())
            lf.drawResizableFrame (g, getWidth(), getHeight(),
                                   BorderSize<int> (getLookAndFeel().getPopupMenuBorderSizeWithOptions (options)));

        if (canScroll())
        {
            if (isTopScrollZoneActive())
            {
                lf.drawPopupMenuUpDownArrowWithOptions (g,
                                                        getWidth(),
                                                        PopupMenuSettings::scrollZone,
                                                        true,
                                                        options);
            }

            if (isBottomScrollZoneActive())
            {
                g.setOrigin (0, getHeight() - PopupMenuSettings::scrollZone);
                lf.drawPopupMenuUpDownArrowWithOptions (g,
                                                        getWidth(),
                                                        PopupMenuSettings::scrollZone,
                                                        false,
                                                        options);
            }
        }
    }

    //==============================================================================
    // hide this and all sub-comps
    void hide (const PopupMenu::Item* item, bool makeInvisible)
    {
        if (isVisible())
        {
            WeakReference<Component> deletionChecker (this);

            activeSubMenu.reset();
            currentChild = nullptr;

            if (item != nullptr
                 && item->commandManager != nullptr
                 && item->itemID != 0)
            {
                *managerOfChosenCommand = item->commandManager;
            }

            auto resultID = options.hasWatchedComponentBeenDeleted() ? 0 : getResultItemID (item);

            exitModalState (resultID);

            if (deletionChecker != nullptr)
            {
                exitingModalState = true;

                if (makeInvisible)
                    setVisible (false);
            }

            if (resultID != 0
                 && item != nullptr
                 && item->action != nullptr)
                MessageManager::callAsync (item->action);
        }
    }

    static int getResultItemID (const PopupMenu::Item* item)
    {
        if (item == nullptr)
            return 0;

        if (auto* cc = item->customCallback.get())
            if (! cc->menuItemTriggered())
                return 0;

        return item->itemID;
    }

    void dismissMenu (const PopupMenu::Item* item)
    {
        if (parent != nullptr)
        {
            parent->dismissMenu (item);
        }
        else
        {
            if (item != nullptr)
            {
                // need a copy of this on the stack as the one passed in will get deleted during this call
                auto mi (*item);
                hide (&mi, false);
            }
            else
            {
                hide (nullptr, true);
            }
        }
    }

    float getDesktopScaleFactor() const override    { return scaleFactor * Desktop::getInstance().getGlobalScaleFactor(); }

    void visibilityChanged() override
    {
        if (! isShowing())
            return;

        auto* accessibleFocus = [this]
        {
          if (currentChild != nullptr)
              if (auto* childHandler = currentChild->getAccessibilityHandler())
                  return childHandler;

            return getAccessibilityHandler();
        }();

        if (accessibleFocus != nullptr)
            accessibleFocus->grabFocus();
    }

    //==============================================================================
    bool keyPressed (const KeyPress& key) override
    {
        if (key.isKeyCode (KeyPress::downKey))
        {
            selectNextItem (MenuSelectionDirection::forwards);
        }
        else if (key.isKeyCode (KeyPress::upKey))
        {
            selectNextItem (MenuSelectionDirection::backwards);
        }
        else if (key.isKeyCode (KeyPress::leftKey))
        {
            if (parent != nullptr)
            {
                Component::SafePointer<MenuWindow> parentWindow (parent);
                ItemComponent* currentChildOfParent = parentWindow->currentChild;

                hide (nullptr, true);

                if (parentWindow != nullptr)
                    parentWindow->setCurrentlyHighlightedChild (currentChildOfParent);

                disableMouseMovesOnMenuAndAncestors();
            }
            else if (componentAttachedTo != nullptr)
            {
                componentAttachedTo->keyPressed (key);
            }
        }
        else if (key.isKeyCode (KeyPress::rightKey))
        {
            disableMouseMovesOnMenuAndAncestors();

            if (showSubMenuFor (currentChild))
            {
                if (isSubMenuVisible())
                    activeSubMenu->selectNextItem (MenuSelectionDirection::current);
            }
            else if (componentAttachedTo != nullptr)
            {
                componentAttachedTo->keyPressed (key);
            }
        }
        else if (key.isKeyCode (KeyPress::returnKey) || key.isKeyCode (KeyPress::spaceKey))
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

    void inputAttemptWhenModal() override
    {
        WeakReference<Component> deletionChecker (this);

        for (auto* ms : mouseSourceStates)
        {
            ms->handleMouseEventWithPosition (ms->source.getScreenPosition().roundToInt());

            if (deletionChecker == nullptr)
                return;
        }

        if (! isOverAnyMenu())
        {
            if (componentAttachedTo != nullptr)
            {
                // we want to dismiss the menu, but if we do it synchronously, then
                // the mouse-click will be allowed to pass through. That's good, except
                // when the user clicks on the button that originally popped the menu up,
                // as they'll expect the menu to go away, and in fact it'll just
                // come back. So only dismiss synchronously if they're not on the original
                // comp that we're attached to.
                auto mousePos = componentAttachedTo->getMouseXYRelative();

                if (componentAttachedTo->reallyContains (mousePos, true))
                {
                    postCommandMessage (PopupMenuSettings::dismissCommandId); // dismiss asynchronously
                    return;
                }
            }

            dismissMenu (nullptr);
        }
    }

    void handleCommandMessage (int commandId) override
    {
        Component::handleCommandMessage (commandId);

        if (commandId == PopupMenuSettings::dismissCommandId)
            dismissMenu (nullptr);
    }

    //==============================================================================
    void mouseDown  (const MouseEvent& e) override    { handleMouseEvent (e); }

    void mouseUp (const MouseEvent& e) override
    {
        SafePointer self { this };

        handleMouseEvent (e);

        // Check whether this menu was deleted as a result of the mouse being released.
        if (self == nullptr)
            return;

        // If the mouse was down when the menu was created, releasing the mouse should
        // not trigger the item under the mouse, because we might still be handling the click
        // that caused the menu to show in the first place. Once the mouse has been released once,
        // then the user must have clicked the mouse again, so they are attempting to trigger or
        // dismiss the menu.
        mouseUpCanTrigger |= true;
    }

    // Any move/drag after the menu is created will allow the mouse to trigger a highlighted item
    void mouseDrag  (const MouseEvent& e) override    { mouseUpCanTrigger |= true; handleMouseEvent (e); }
    void mouseMove  (const MouseEvent& e) override    { mouseUpCanTrigger |= true; handleMouseEvent (e); }

    void mouseWheelMove (const MouseEvent&, const MouseWheelDetails& wheel) override
    {
        alterChildYPos (roundToInt (-10.0f * wheel.deltaY * PopupMenuSettings::scrollZone));
    }

    bool windowIsStillValid()
    {
        if (! isVisible())
            return false;

        if (componentAttachedTo != options.getTargetComponent())
        {
            dismissMenu (nullptr);
            return false;
        }

        if (auto* currentlyModalWindow = dynamic_cast<MenuWindow*> (Component::getCurrentlyModalComponent()))
            if (! treeContains (currentlyModalWindow))
                return false;

        if (exitingModalState)
            return false;

        return true;
    }

    static Array<MenuWindow*>& getActiveWindows()
    {
        static Array<MenuWindow*> activeMenuWindows;
        return activeMenuWindows;
    }

    MouseSourceState& getMouseState (MouseInputSource source)
    {
        MouseSourceState* mouseState = nullptr;

        for (auto* ms : mouseSourceStates)
        {
            if (ms->source == source)
                mouseState = ms;
            else if (ms->source.getType() != source.getType())
                ms->stopTimer();
        }

        if (mouseState == nullptr)
        {
            mouseState = new MouseSourceState (*this, source);
            mouseSourceStates.add (mouseState);
        }

        return *mouseState;
    }

    //==============================================================================
    bool isOverAnyMenu() const
    {
        return parent != nullptr ? parent->isOverAnyMenu()
                                 : isOverChildren();
    }

    bool isOverChildren() const
    {
        return isVisible()
                && (isAnyMouseOver() || (activeSubMenu != nullptr && activeSubMenu->isOverChildren()));
    }

    bool isAnyMouseOver() const
    {
        for (auto* ms : mouseSourceStates)
            if (ms->isOver())
                return true;

        return false;
    }

    bool treeContains (const MenuWindow* const window) const noexcept
    {
        auto* mw = this;

        while (mw->parent != nullptr)
            mw = mw->parent;

        while (mw != nullptr)
        {
            if (mw == window)
                return true;

            mw = mw->activeSubMenu.get();
        }

        return false;
    }

    bool doesAnyJuceCompHaveFocus()
    {
        if (! detail::WindowingHelpers::isForegroundOrEmbeddedProcess (componentAttachedTo))
            return false;

        if (Component::getCurrentlyFocusedComponent() != nullptr)
            return true;

        for (int i = ComponentPeer::getNumPeers(); --i >= 0;)
        {
            if (ComponentPeer::getPeer (i)->isFocused())
            {
                hasAnyJuceCompHadFocus = true;
                return true;
            }
        }

        return ! hasAnyJuceCompHadFocus;
    }

    //==============================================================================
    Rectangle<int> getParentArea (Point<int> targetPoint, Component* relativeTo = nullptr)
    {
        if (relativeTo != nullptr)
            targetPoint = relativeTo->localPointToGlobal (targetPoint);

        auto* display = Desktop::getInstance().getDisplays().getDisplayForPoint (targetPoint * scaleFactor);
        auto parentArea = display->userArea.getIntersection (display->safeAreaInsets.subtractedFrom (display->totalArea));

        if (auto* pc = options.getParentComponent())
        {
            return pc->getLocalArea (nullptr,
                                     pc->getScreenBounds()
                                           .reduced (getLookAndFeel().getPopupMenuBorderSizeWithOptions (options))
                                           .getIntersection (parentArea));
        }

        return parentArea;
    }

    void calculateWindowPos (Rectangle<int> target, const bool alignToRectangle)
    {
        auto parentArea = getParentArea (target.getCentre()) / scaleFactor;

        if (auto* pc = options.getParentComponent())
            target = pc->getLocalArea (nullptr, target).getIntersection (parentArea);

        auto maxMenuHeight = parentArea.getHeight() - 24;

        int x, y, widthToUse, heightToUse;
        layoutMenuItems (parentArea.getWidth() - 24, maxMenuHeight, widthToUse, heightToUse);

        if (alignToRectangle)
        {
            x = target.getX();

            auto spaceUnder = parentArea.getBottom() - target.getBottom();
            auto spaceOver = target.getY() - parentArea.getY();
            auto bufferHeight = 30;

            if (options.getPreferredPopupDirection() == Options::PopupDirection::upwards)
                y = (heightToUse < spaceOver - bufferHeight  || spaceOver >= spaceUnder) ? target.getY() - heightToUse
                                                                                         : target.getBottom();
            else
                y = (heightToUse < spaceUnder - bufferHeight || spaceUnder >= spaceOver) ? target.getBottom()
                                                                                         : target.getY() - heightToUse;
        }
        else
        {
            bool tendTowardsRight = target.getCentreX() < parentArea.getCentreX();

            if (parent != nullptr)
            {
                if (parent->parent != nullptr)
                {
                    const bool parentGoingRight = (parent->getX() + parent->getWidth() / 2
                                                    > parent->parent->getX() + parent->parent->getWidth() / 2);

                    if (parentGoingRight && target.getRight() + widthToUse < parentArea.getRight() - 4)
                        tendTowardsRight = true;
                    else if ((! parentGoingRight) && target.getX() > widthToUse + 4)
                        tendTowardsRight = false;
                }
                else if (target.getRight() + widthToUse < parentArea.getRight() - 32)
                {
                    tendTowardsRight = true;
                }
            }

            auto biggestSpace = jmax (parentArea.getRight() - target.getRight(),
                                      target.getX() - parentArea.getX()) - 32;

            if (biggestSpace < widthToUse)
            {
                layoutMenuItems (biggestSpace + target.getWidth() / 3, maxMenuHeight, widthToUse, heightToUse);

                if (numColumns > 1)
                    layoutMenuItems (biggestSpace - 4, maxMenuHeight, widthToUse, heightToUse);

                tendTowardsRight = (parentArea.getRight() - target.getRight()) >= (target.getX() - parentArea.getX());
            }

            x = tendTowardsRight ? jmin (parentArea.getRight() - widthToUse - 4, target.getRight())
                                 : jmax (parentArea.getX() + 4, target.getX() - widthToUse);

            if (getLookAndFeel().getPopupMenuBorderSizeWithOptions (options) == 0) // workaround for dismissing the window on mouse up when border size is 0
                x += tendTowardsRight ? 1 : -1;

            const auto border = getLookAndFeel().getPopupMenuBorderSizeWithOptions (options);
            y = target.getCentreY() > parentArea.getCentreY() ? jmax (parentArea.getY(), target.getBottom() - heightToUse) + border
                                                              : target.getY() - border;
        }

        x = jmax (parentArea.getX() + 1, jmin (parentArea.getRight()  - (widthToUse  + 6), x));
        y = jmax (parentArea.getY() + 1, jmin (parentArea.getBottom() - (heightToUse + 6), y));

        windowPos.setBounds (x, y, widthToUse, heightToUse);

        // sets this flag if it's big enough to obscure any of its parent menus
        hideOnExit = parent != nullptr
                      && parent->windowPos.intersects (windowPos.expanded (-4, -4));
    }

    void layoutMenuItems (const int maxMenuW, const int maxMenuH, int& width, int& height)
    {
        // Ensure we don't try to add an empty column after the final item
        if (auto* last = items.getLast())
            last->item.shouldBreakAfter = false;

        const auto isBreak = [] (const ItemComponent* item) { return item->item.shouldBreakAfter; };
        const auto numBreaks = static_cast<int> (std::count_if (items.begin(), items.end(), isBreak));
        numColumns = numBreaks + 1;

        if (numBreaks == 0)
            insertColumnBreaks (maxMenuW, maxMenuH);

        workOutManualSize (maxMenuW);
        height = jmin (contentHeight, maxMenuH);

        needsToScroll = contentHeight > height;

        width = updateYPositions();
    }

    void insertColumnBreaks (const int maxMenuW, const int maxMenuH)
    {
        numColumns = options.getMinimumNumColumns();
        contentHeight = 0;

        auto maximumNumColumns = options.getMaximumNumColumns() > 0 ? options.getMaximumNumColumns() : 7;

        for (;;)
        {
            auto totalW = workOutBestSize (maxMenuW);

            if (totalW > maxMenuW)
            {
                numColumns = jmax (1, numColumns - 1);
                workOutBestSize (maxMenuW); // to update col widths
                break;
            }

            if (totalW > maxMenuW / 2
                || contentHeight < maxMenuH
                || numColumns >= maximumNumColumns)
                break;

            ++numColumns;
        }

        const auto itemsPerColumn = (items.size() + numColumns - 1) / numColumns;

        for (auto i = 0;; i += itemsPerColumn)
        {
            const auto breakIndex = i + itemsPerColumn - 1;

            if (breakIndex >= items.size())
                break;

            items[breakIndex]->item.shouldBreakAfter = true;
        }

        if (! items.isEmpty())
            (*std::prev (items.end()))->item.shouldBreakAfter = false;
    }

    int correctColumnWidths (const int maxMenuW)
    {
        auto totalW = std::accumulate (columnWidths.begin(), columnWidths.end(), 0);
        const auto minWidth = jmin (maxMenuW, options.getMinimumWidth());

        if (totalW < minWidth)
        {
            totalW = minWidth;

            for (auto& column : columnWidths)
                column = totalW / numColumns;
        }

        return totalW;
    }

    void workOutManualSize (const int maxMenuW)
    {
        contentHeight = 0;
        columnWidths.clear();

        for (auto it = items.begin(), end = items.end(); it != end;)
        {
            const auto isBreak = [] (const ItemComponent* item) { return item->item.shouldBreakAfter; };
            const auto nextBreak = std::find_if (it, end, isBreak);
            const auto columnEnd = nextBreak == end ? end : std::next (nextBreak);

            const auto getMaxWidth = [] (int acc, const ItemComponent* item) { return jmax (acc, item->getWidth()); };
            const auto colW = std::accumulate (it, columnEnd, options.getStandardItemHeight(), getMaxWidth);
            const auto adjustedColW = jmin (maxMenuW / jmax (1, numColumns - 2),
                                            colW + getLookAndFeel().getPopupMenuBorderSizeWithOptions (options) * 2);

            const auto sumHeight = [] (int acc, const ItemComponent* item) { return acc + item->getHeight(); };
            const auto colH = std::accumulate (it, columnEnd, 0, sumHeight);

            contentHeight = jmax (contentHeight, colH);
            columnWidths.add (adjustedColW);
            it = columnEnd;
        }

        contentHeight += getLookAndFeel().getPopupMenuBorderSizeWithOptions (options) * 2;

        correctColumnWidths (maxMenuW);
    }

    int workOutBestSize (const int maxMenuW)
    {
        contentHeight = 0;
        int childNum = 0;

        for (int col = 0; col < numColumns; ++col)
        {
            int colW = options.getStandardItemHeight(), colH = 0;

            auto numChildren = jmin (items.size() - childNum,
                                     (items.size() + numColumns - 1) / numColumns);

            for (int i = numChildren; --i >= 0;)
            {
                colW = jmax (colW, items.getUnchecked (childNum + i)->getWidth());
                colH += items.getUnchecked (childNum + i)->getHeight();
            }

            colW = jmin (maxMenuW / jmax (1, numColumns - 2),
                         colW + getLookAndFeel().getPopupMenuBorderSizeWithOptions (options) * 2);

            columnWidths.set (col, colW);
            contentHeight = jmax (contentHeight, colH);

            childNum += numChildren;
        }

        return correctColumnWidths (maxMenuW);
    }

    void ensureItemComponentIsVisible (const ItemComponent& itemComp, std::optional<int> wantedY)
    {
        const auto parentArea = getParentArea (windowPos.getPosition(), options.getParentComponent()) / scaleFactor;

        if (const auto posAndOffset = computePosAndOffsetToEnsureVisibility (windowPos, parentArea, itemComp.getBounds(), contentHeight, wantedY))
        {
            std::tie (windowPos, childYOffset) = std::tie (posAndOffset->windowPos, posAndOffset->childYOffset);
            updateYPositions();
        }
    }

    struct PosAndOffset
    {
        Rectangle<int> windowPos;
        int childYOffset = 0;
    };

    static std::optional<PosAndOffset> computePosAndOffsetToEnsureVisibility (Rectangle<int> windowPos,
                                                                              const Rectangle<int>& parentArea,
                                                                              const Rectangle<int>& itemCompBounds,
                                                                              int contentHeight,
                                                                              std::optional<int> wantedY)
    {
        // If there's no specific wantedY, and the item component is already visible, then we don't
        // need to make any adjustments.
        if (! wantedY.has_value() && 0 <= itemCompBounds.getY() && itemCompBounds.getBottom() <= windowPos.getHeight())
            return {};

        const auto spaceNeededAboveItem = jmin (PopupMenuSettings::scrollZone, itemCompBounds.getY());
        const auto spaceNeededBelowItem = jmin (PopupMenuSettings::scrollZone, contentHeight - itemCompBounds.getBottom());
        const auto parentSpaceTargetY = windowPos.getY() + wantedY.value_or (itemCompBounds.getY());

        // In order to display the visible item over the target area, we need to make sure that
        // there's enough space above and below to hold the scroll areas if they're showing.
        // Ideally, we want to avoid the case where the menu opens with the scroll area over the
        // target area.
        const auto isSpaceToOverlay = spaceNeededAboveItem <= (parentSpaceTargetY - parentArea.getY())
                                   && spaceNeededBelowItem <= (parentArea.getBottom() - (parentSpaceTargetY + itemCompBounds.getHeight()));

        if (wantedY.has_value() && isSpaceToOverlay)
        {
            windowPos = windowPos.withY (parentSpaceTargetY - itemCompBounds.getY())
                                 .withHeight (contentHeight)
                                 .constrainedWithin (parentArea);

            const auto menuSpaceTargetY = parentSpaceTargetY - windowPos.getY();
            const auto offset = itemCompBounds.getY() - menuSpaceTargetY;

            return PosAndOffset { windowPos, offset };
        }

        // If there's not enough space to overlay the menu, then just use the provided menu
        // bounds but try to position the visible item as close to the target area as possible,
        // while avoiding the scroll areas.
        const auto menuSpaceTargetY = jlimit (spaceNeededAboveItem,
                                              windowPos.getHeight() - spaceNeededBelowItem - itemCompBounds.getHeight(),
                                              parentSpaceTargetY - windowPos.getY());
        const auto offset = itemCompBounds.getY() - menuSpaceTargetY;

        return PosAndOffset { windowPos, offset };
    }

    void resizeToBestWindowPos()
    {
        auto r = windowPos;

        if (childYOffset < 0)
        {
            r = r.withTop (r.getY() - childYOffset);
        }
        else if (childYOffset > 0)
        {
            auto spaceAtBottom = r.getHeight() - (contentHeight - childYOffset);

            if (spaceAtBottom > 0)
                r.setSize (r.getWidth(), r.getHeight() - spaceAtBottom);
        }

        setBounds (r);
        updateYPositions();
    }

    void alterChildYPos (int delta)
    {
        if (canScroll())
        {
            childYOffset += delta;

            childYOffset = [&]
            {
                if (delta < 0)
                    return jmax (childYOffset, 0);

                if (delta > 0)
                {
                    const auto limit = contentHeight
                                        - windowPos.getHeight()
                                        + getLookAndFeel().getPopupMenuBorderSizeWithOptions (options);
                    return jmin (childYOffset, limit);
                }

                return childYOffset;
            }();

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
        const auto separatorWidth = getLookAndFeel().getPopupMenuColumnSeparatorWidthWithOptions (options);
        const auto initialY = getLookAndFeel().getPopupMenuBorderSizeWithOptions (options)
                              - (childYOffset + (getY() - windowPos.getY()));

        auto col = 0;
        auto x = 0;
        auto y = initialY;

        for (const auto& item : items)
        {
            jassert (col < columnWidths.size());
            const auto columnWidth = columnWidths[col];
            item->setBounds (x, y, columnWidth, item->getHeight());
            y += item->getHeight();

            if (item->item.shouldBreakAfter)
            {
                col += 1;
                x += columnWidth + separatorWidth;
                y = initialY;
            }
        }

        return std::accumulate (columnWidths.begin(), columnWidths.end(), 0)
               + (separatorWidth * (columnWidths.size() - 1));
    }

    void setCurrentlyHighlightedChild (ItemComponent* child)
    {
        if (currentChild != nullptr)
            currentChild->setHighlighted (false);

        currentChild = child;

        if (currentChild != nullptr)
        {
            currentChild->setHighlighted (true);
            timeEnteredCurrentChildComp = Time::getApproximateMillisecondCounter();
        }

        if (auto* handler = getAccessibilityHandler())
            handler->notifyAccessibilityEvent (AccessibilityEvent::rowSelectionChanged);
    }

    bool isSubMenuVisible() const noexcept          { return activeSubMenu != nullptr && activeSubMenu->isVisible(); }

    bool showSubMenuFor (ItemComponent* childComp)
    {
        activeSubMenu.reset();

        if (childComp == nullptr || ! hasActiveSubMenu (childComp->item))
            return false;

        activeSubMenu.reset (new HelperClasses::MenuWindow (*(childComp->item.subMenu), this,
                                                            options.forSubmenu()
                                                                   .withTargetScreenArea (childComp->getScreenBounds())
                                                                   .withMinimumWidth (0),
                                                            false, managerOfChosenCommand, scaleFactor));

        activeSubMenu->setVisible (true); // (must be called before enterModalState on Windows to avoid DropShadower confusion)
        activeSubMenu->enterModalState (false);
        activeSubMenu->toFront (false);
        return true;
    }

    void triggerCurrentlyHighlightedItem()
    {
        if (currentChild != nullptr && canBeTriggered (currentChild->item))
            dismissMenu (&currentChild->item);
    }

    enum class MenuSelectionDirection
    {
        forwards,
        backwards,
        current
    };

    void selectNextItem (MenuSelectionDirection direction)
    {
        disableMouseMovesOnMenuAndAncestors();

        auto start = [&]
        {
            auto index = items.indexOf (currentChild);

            if (index >= 0)
                return index;

            return direction == MenuSelectionDirection::backwards ? items.size() - 1
                                                                  : 0;
        }();

        auto preIncrement = (direction != MenuSelectionDirection::current && currentChild != nullptr);

        for (int i = items.size(); --i >= 0;)
        {
            if (preIncrement)
                start += (direction == MenuSelectionDirection::backwards ? -1 : 1);

            if (auto* mic = items.getUnchecked ((start + items.size()) % items.size()))
            {
                if (canBeTriggered (mic->item) || hasActiveSubMenu (mic->item))
                {
                    setCurrentlyHighlightedChild (mic);
                    return;
                }
            }

            if (! preIncrement)
                preIncrement = true;
        }
    }

    void disableMouseMovesOnMenuAndAncestors()
    {
        disableMouseMoves = true;

        if (parent != nullptr)
            parent->disableMouseMovesOnMenuAndAncestors();
    }

    bool canScroll() const noexcept                 { return childYOffset != 0 || needsToScroll; }
    bool isTopScrollZoneActive() const noexcept     { return canScroll() && childYOffset > 0; }
    bool isBottomScrollZoneActive() const noexcept  { return canScroll() && childYOffset < contentHeight - windowPos.getHeight(); }

    //==============================================================================
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return std::make_unique<AccessibilityHandler> (*this,
                                                       AccessibilityRole::popupMenu,
                                                       AccessibilityActions().addAction (AccessibilityActionType::focus, [this]
                                                       {
                                                           if (currentChild != nullptr)
                                                           {
                                                               if (auto* handler = currentChild->getAccessibilityHandler())
                                                                   handler->grabFocus();
                                                           }
                                                           else
                                                           {
                                                               selectNextItem (MenuSelectionDirection::forwards);
                                                           }
                                                       }));
    }

    LookAndFeel* findLookAndFeel (const PopupMenu& menu, MenuWindow* parentWindow) const
    {
        return parentWindow != nullptr ? &(parentWindow->getLookAndFeel())
                                       : menu.lookAndFeel.get();
    }

    LookAndFeel& findNonNullLookAndFeel (const PopupMenu& menu, MenuWindow* parentWindow) const
    {
        if (auto* result = findLookAndFeel (menu, parentWindow))
            return *result;

        return getLookAndFeel();
    }

    bool mouseHasBeenOver() const
    {
        return mouseWasOver;
    }

    bool allowMouseUpToTriggerItem() const
    {
        return mouseUpCanTrigger;
    }

    //==============================================================================
    MenuWindow* parent;
    const Options options;
    OwnedArray<ItemComponent> items;
    ApplicationCommandManager** managerOfChosenCommand;
    WeakReference<Component> componentAttachedTo;
    Rectangle<int> windowPos;
    bool needsToScroll = false;
    bool hideOnExit = false, disableMouseMoves = false, hasAnyJuceCompHadFocus = false;
    int numColumns = 0, contentHeight = 0, childYOffset = 0;
    Component::SafePointer<ItemComponent> currentChild;
    std::unique_ptr<MenuWindow> activeSubMenu;
    Array<int> columnWidths;
    uint32 windowCreationTime, lastFocusedTime, timeEnteredCurrentChildComp;
    OwnedArray<MouseSourceState> mouseSourceStates;
    float scaleFactor;
    bool exitingModalState = false;

private:
    void handleMouseEvent (const MouseEvent& e)
    {
        mouseWasOver |= reallyContains (getLocalPoint (nullptr, e.getScreenPosition()), true);
        getMouseState (e.source).handleMouseEventWithPosition (e.getScreenPosition());
    }

    bool mouseWasOver = false;
    bool mouseUpCanTrigger = ! ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MenuWindow)
};

//==============================================================================
class MouseSourceState final : private Timer
{
public:
    MouseSourceState (MenuWindow& w, MouseInputSource s)
        : window (w), source (s), lastScrollTime (Time::getMillisecondCounter())
    {
        startTimerHz (20);
    }

    ~MouseSourceState() override
    {
        stopTimer();
    }

    void handleMouseEventWithPosition (const Point<int>& e)
    {
        if (! window.windowIsStillValid())
            return;

        startTimerHz (20);
        handleMousePosition (e);
    }

    bool isOver() const
    {
        return window.reallyContains (window.getLocalPoint (nullptr, source.getScreenPosition()).roundToInt(), true);
    }

    using Timer::stopTimer;

    MenuWindow& window;
    MouseInputSource source;

private:
    Point<int> lastMousePos;
    double scrollAcceleration = 0;
    uint32 lastScrollTime = 0, lastMoveTime = 0;
    bool isDown = false;

    // Although most mouse movements can be handled inside mouse event callbacks, scrolling of menus
    // may happen while the mouse is not moving, so periodic timer callbacks are required in this
    // scenario.
    void timerCallback() override
    {
       #if JUCE_WINDOWS
        // touch and pen devices on Windows send an offscreen mouse move after mouse up events
        // but we don't want to forward these on as they will dismiss the menu
        if ((source.isTouch() || source.isPen()) && ! isValidMousePosition())
            return;
       #endif

        handleMouseEventWithPosition (source.getScreenPosition().roundToInt());
    }

    void handleMousePosition (Point<int> globalMousePos)
    {
        auto localMousePos = window.getLocalPoint (nullptr, globalMousePos);
        auto timeNow = Time::getMillisecondCounter();

        if (timeNow > window.timeEnteredCurrentChildComp + 100
             && window.reallyContains (localMousePos, true)
             && window.currentChild != nullptr
             && ! (window.disableMouseMoves || window.isSubMenuVisible()))
        {
            window.showSubMenuFor (window.currentChild);
        }

        highlightItemUnderMouse (globalMousePos, localMousePos, timeNow);

        const bool overScrollArea = scrollIfNecessary (localMousePos, timeNow);
        const bool isOverAny = window.isOverAnyMenu();

        if (window.hideOnExit && window.mouseHasBeenOver() && ! isOverAny)
            window.hide (nullptr, true);
        else
            checkButtonState (localMousePos, timeNow, isDown, overScrollArea, isOverAny);
    }

    void checkButtonState (Point<int> localMousePos, const uint32 timeNow,
                           const bool wasDown, const bool overScrollArea, const bool isOverAny)
    {
        isDown = window.mouseHasBeenOver()
                    && (ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown()
                         || ComponentPeer::getCurrentModifiersRealtime().isAnyMouseButtonDown());

        const auto reallyContained = window.reallyContains (localMousePos, true);

        if (! window.doesAnyJuceCompHaveFocus() && ! reallyContained)
        {
            if (timeNow > window.lastFocusedTime + 10)
            {
                PopupMenuSettings::menuWasHiddenBecauseOfAppChange = true;
                window.dismissMenu (nullptr);
                // Note: This object may have been deleted by the previous call.
            }
        }
        else if (wasDown && timeNow > window.windowCreationTime + 250 && ! isDown && ! overScrollArea)
        {
            if (reallyContained && window.allowMouseUpToTriggerItem())
                window.triggerCurrentlyHighlightedItem();
            else if ((window.mouseHasBeenOver() || ! window.allowMouseUpToTriggerItem()) && ! isOverAny)
                window.dismissMenu (nullptr);

            // Note: This object may have been deleted by the previous call.
        }
        else
        {
            window.lastFocusedTime = timeNow;
        }
    }

    void highlightItemUnderMouse (Point<int> globalMousePos, Point<int> localMousePos, uint32 timeNow)
    {
        const auto mouseTimedOut = lastMoveTime != 0 && 350 < (timeNow - lastMoveTime);
        const auto mouseHasMoved = 2 < lastMousePos.getDistanceFrom (globalMousePos);
        const auto isMouseOver = window.reallyContains (localMousePos, true);

        if (mouseHasMoved && isMouseOver)
        {
            window.disableMouseMoves = false;
            lastMoveTime = timeNow;
        }

        if (! mouseHasMoved && ! mouseTimedOut)
            return;

        if (window.disableMouseMoves)
            return;

        if (window.activeSubMenu != nullptr && window.activeSubMenu->isOverChildren())
            return;

        const auto isMovingTowardsMenu = isMouseOver
                                      && globalMousePos != lastMousePos
                                      && isMovingTowardsSubmenu (globalMousePos);

        lastMousePos = globalMousePos;

        if (isMovingTowardsMenu)
            return;

        auto* componentUnderMouse = window.getComponentAt (localMousePos);
        auto* childComponentUnderMouse = componentUnderMouse != &window ? componentUnderMouse : nullptr;

        auto* itemUnderMouse = std::invoke ([&]() -> ItemComponent*
        {
            if (auto* candidate = dynamic_cast<ItemComponent*> (childComponentUnderMouse))
                return candidate;

            if (childComponentUnderMouse != nullptr)
                return childComponentUnderMouse->findParentComponentOfClass<ItemComponent>();

            return nullptr;
        });

        if (itemUnderMouse == window.currentChild)
            return;

        if (! isMouseOver && window.activeSubMenu != nullptr && window.activeSubMenu->isVisible())
            return;

        if (isMouseOver && childComponentUnderMouse != nullptr && window.activeSubMenu != nullptr)
            window.activeSubMenu->hide (nullptr, true);

        if (! isMouseOver && ! window.mouseHasBeenOver())
            return;

        window.setCurrentlyHighlightedChild (isMouseOver ? itemUnderMouse : nullptr);
    }

    bool isMovingTowardsSubmenu (Point<int> newGlobalPos) const
    {
        if (window.activeSubMenu == nullptr)
            return false;

        // try to intelligently guess whether the user is moving the mouse towards a currently-open
        // submenu. To do this, look at whether the mouse stays inside a triangular region that
        // extends from the last mouse pos to the submenu's rectangle..

        auto itemScreenBounds = window.activeSubMenu->getScreenBounds();
        auto subX = (float) itemScreenBounds.getX();

        auto oldGlobalPos = lastMousePos;

        if (itemScreenBounds.getX() > window.getX())
        {
            oldGlobalPos -= Point<int> (2, 0);  // to enlarge the triangle a bit, in case the mouse only moves a couple of pixels
        }
        else
        {
            oldGlobalPos += Point<int> (2, 0);
            subX += (float) itemScreenBounds.getWidth();
        }

        Path areaTowardsSubMenu;
        areaTowardsSubMenu.addTriangle ((float) oldGlobalPos.x, (float) oldGlobalPos.y,
                                        subX, (float) itemScreenBounds.getY(),
                                        subX, (float) itemScreenBounds.getBottom());

        return areaTowardsSubMenu.contains (newGlobalPos.toFloat());
    }

    bool scrollIfNecessary (Point<int> localMousePos, const uint32 timeNow)
    {
        if (window.canScroll()
             && isPositiveAndBelow (localMousePos.x, window.getWidth())
             && (isPositiveAndBelow (localMousePos.y, window.getHeight()) || source.isDragging()))
        {
            if (window.isTopScrollZoneActive() && localMousePos.y < PopupMenuSettings::scrollZone)
                return scroll (timeNow, -1);

            if (window.isBottomScrollZoneActive() && localMousePos.y > window.getHeight() - PopupMenuSettings::scrollZone)
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

            for (int i = 0; i < window.items.size() && amount == 0; ++i)
                amount = ((int) scrollAcceleration) * window.items.getUnchecked (i)->getHeight();

            window.alterChildYPos (amount * direction);
            lastScrollTime = timeNow;
        }

        return true;
    }

   #if JUCE_WINDOWS
    bool isValidMousePosition()
    {
        auto screenPos = source.getScreenPosition();
        auto localPos = (window.activeSubMenu == nullptr) ? window.getLocalPoint (nullptr, screenPos)
                                                          : window.activeSubMenu->getLocalPoint (nullptr, screenPos);

        if (localPos.x < 0 && localPos.y < 0)
            return false;

        return true;
    }
   #endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MouseSourceState)
};

//==============================================================================
struct NormalComponentWrapper final : public PopupMenu::CustomComponent
{
    NormalComponentWrapper (Component& comp, int w, int h, bool triggerMenuItemAutomaticallyWhenClicked)
        : PopupMenu::CustomComponent (triggerMenuItemAutomaticallyWhenClicked),
          width (w), height (h)
    {
        addAndMakeVisible (comp);
    }

    void getIdealSize (int& idealWidth, int& idealHeight) override
    {
        idealWidth = width;
        idealHeight = height;
    }

    void resized() override
    {
        if (auto* child = getChildComponent (0))
            child->setBounds (getLocalBounds());
    }

    const int width, height;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NormalComponentWrapper)
};

};

//==============================================================================
PopupMenu::PopupMenu (const PopupMenu& other)
    : items (other.items),
      lookAndFeel (other.lookAndFeel)
{
}

PopupMenu& PopupMenu::operator= (const PopupMenu& other)
{
    if (this != &other)
    {
        items = other.items;
        lookAndFeel = other.lookAndFeel;
    }

    return *this;
}

PopupMenu::PopupMenu (PopupMenu&& other) noexcept
    : items (std::move (other.items)),
      lookAndFeel (std::move (other.lookAndFeel))
{
}

PopupMenu& PopupMenu::operator= (PopupMenu&& other) noexcept
{
    items = std::move (other.items);
    lookAndFeel = other.lookAndFeel;
    return *this;
}

PopupMenu::~PopupMenu() = default;

void PopupMenu::clear()
{
    items.clear();
}

//==============================================================================
PopupMenu::Item::Item() = default;
PopupMenu::Item::Item (String t) : text (std::move (t)), itemID (-1) {}

PopupMenu::Item::Item (Item&&) = default;
PopupMenu::Item& PopupMenu::Item::operator= (Item&&) = default;

PopupMenu::Item::Item (const Item& other)
  : text (other.text),
    itemID (other.itemID),
    action (other.action),
    subMenu (createCopyIfNotNull (other.subMenu.get())),
    image (other.image != nullptr ? other.image->createCopy() : nullptr),
    customComponent (other.customComponent),
    customCallback (other.customCallback),
    commandManager (other.commandManager),
    shortcutKeyDescription (other.shortcutKeyDescription),
    colour (other.colour),
    isEnabled (other.isEnabled),
    isTicked (other.isTicked),
    isSeparator (other.isSeparator),
    isSectionHeader (other.isSectionHeader),
    shouldBreakAfter (other.shouldBreakAfter)
{}

PopupMenu::Item& PopupMenu::Item::operator= (const Item& other)
{
    text = other.text;
    itemID = other.itemID;
    action = other.action;
    subMenu.reset (createCopyIfNotNull (other.subMenu.get()));
    image = other.image != nullptr ? other.image->createCopy() : std::unique_ptr<Drawable>();
    customComponent = other.customComponent;
    customCallback = other.customCallback;
    commandManager = other.commandManager;
    shortcutKeyDescription = other.shortcutKeyDescription;
    colour = other.colour;
    isEnabled = other.isEnabled;
    isTicked = other.isTicked;
    isSeparator = other.isSeparator;
    isSectionHeader = other.isSectionHeader;
    shouldBreakAfter = other.shouldBreakAfter;
    return *this;
}

PopupMenu::Item& PopupMenu::Item::setTicked (bool shouldBeTicked) & noexcept
{
    isTicked = shouldBeTicked;
    return *this;
}

PopupMenu::Item& PopupMenu::Item::setEnabled (bool shouldBeEnabled) & noexcept
{
    isEnabled = shouldBeEnabled;
    return *this;
}

PopupMenu::Item& PopupMenu::Item::setAction (std::function<void()> newAction) & noexcept
{
    action = std::move (newAction);
    return *this;
}

PopupMenu::Item& PopupMenu::Item::setID (int newID) & noexcept
{
    itemID = newID;
    return *this;
}

PopupMenu::Item& PopupMenu::Item::setColour (Colour newColour) & noexcept
{
    colour = newColour;
    return *this;
}

PopupMenu::Item& PopupMenu::Item::setCustomComponent (ReferenceCountedObjectPtr<CustomComponent> comp) & noexcept
{
    customComponent = comp;
    return *this;
}

PopupMenu::Item& PopupMenu::Item::setImage (std::unique_ptr<Drawable> newImage) & noexcept
{
    image = std::move (newImage);
    return *this;
}

PopupMenu::Item&& PopupMenu::Item::setTicked (bool shouldBeTicked) && noexcept
{
    isTicked = shouldBeTicked;
    return std::move (*this);
}

PopupMenu::Item&& PopupMenu::Item::setEnabled (bool shouldBeEnabled) && noexcept
{
    isEnabled = shouldBeEnabled;
    return std::move (*this);
}

PopupMenu::Item&& PopupMenu::Item::setAction (std::function<void()> newAction) && noexcept
{
    action = std::move (newAction);
    return std::move (*this);
}

PopupMenu::Item&& PopupMenu::Item::setID (int newID) && noexcept
{
    itemID = newID;
    return std::move (*this);
}

PopupMenu::Item&& PopupMenu::Item::setColour (Colour newColour) && noexcept
{
    colour = newColour;
    return std::move (*this);
}

PopupMenu::Item&& PopupMenu::Item::setCustomComponent (ReferenceCountedObjectPtr<CustomComponent> comp) && noexcept
{
    customComponent = comp;
    return std::move (*this);
}

PopupMenu::Item&& PopupMenu::Item::setImage (std::unique_ptr<Drawable> newImage) && noexcept
{
    image = std::move (newImage);
    return std::move (*this);
}

void PopupMenu::addItem (Item newItem)
{
    // An ID of 0 is used as a return value to indicate that the user
    // didn't pick anything, so you shouldn't use it as the ID for an item.
    jassert (newItem.itemID != 0
              || newItem.isSeparator || newItem.isSectionHeader
              || newItem.subMenu != nullptr);

    items.add (std::move (newItem));
}

void PopupMenu::addItem (String itemText, std::function<void()> action)
{
    addItem (std::move (itemText), true, false, std::move (action));
}

void PopupMenu::addItem (String itemText, bool isActive, bool isTicked, std::function<void()> action)
{
    Item i (std::move (itemText));
    i.action = std::move (action);
    i.isEnabled = isActive;
    i.isTicked = isTicked;
    addItem (std::move (i));
}

void PopupMenu::addItem (int itemResultID, String itemText, bool isActive, bool isTicked)
{
    Item i (std::move (itemText));
    i.itemID = itemResultID;
    i.isEnabled = isActive;
    i.isTicked = isTicked;
    addItem (std::move (i));
}

static std::unique_ptr<Drawable> createDrawableFromImage (const Image& im)
{
    if (im.isValid())
    {
        auto d = new DrawableImage();
        d->setImage (im);
        return std::unique_ptr<Drawable> (d);
    }

    return {};
}

void PopupMenu::addItem (int itemResultID, String itemText, bool isActive, bool isTicked, const Image& iconToUse)
{
    addItem (itemResultID, std::move (itemText), isActive, isTicked, createDrawableFromImage (iconToUse));
}

void PopupMenu::addItem (int itemResultID, String itemText, bool isActive,
                         bool isTicked, std::unique_ptr<Drawable> iconToUse)
{
    Item i (std::move (itemText));
    i.itemID = itemResultID;
    i.isEnabled = isActive;
    i.isTicked = isTicked;
    i.image = std::move (iconToUse);
    addItem (std::move (i));
}

void PopupMenu::addCommandItem (ApplicationCommandManager* commandManager,
                                const CommandID commandID,
                                String displayName,
                                std::unique_ptr<Drawable> iconToUse)
{
    jassert (commandManager != nullptr && commandID != 0);

    if (auto* registeredInfo = commandManager->getCommandForID (commandID))
    {
        ApplicationCommandInfo info (*registeredInfo);
        auto* target = commandManager->getTargetForCommand (commandID, info);

        Item i;
        i.text = displayName.isNotEmpty() ? std::move (displayName) : info.shortName;
        i.itemID = (int) commandID;
        i.commandManager = commandManager;
        i.isEnabled = target != nullptr && (info.flags & ApplicationCommandInfo::isDisabled) == 0;
        i.isTicked = (info.flags & ApplicationCommandInfo::isTicked) != 0;
        i.image = std::move (iconToUse);
        addItem (std::move (i));
    }
}

void PopupMenu::addColouredItem (int itemResultID, String itemText, Colour itemTextColour,
                                 bool isActive, bool isTicked, std::unique_ptr<Drawable> iconToUse)
{
    Item i (std::move (itemText));
    i.itemID = itemResultID;
    i.colour = itemTextColour;
    i.isEnabled = isActive;
    i.isTicked = isTicked;
    i.image = std::move (iconToUse);
    addItem (std::move (i));
}

void PopupMenu::addColouredItem (int itemResultID, String itemText, Colour itemTextColour,
                                 bool isActive, bool isTicked, const Image& iconToUse)
{
    Item i (std::move (itemText));
    i.itemID = itemResultID;
    i.colour = itemTextColour;
    i.isEnabled = isActive;
    i.isTicked = isTicked;
    i.image = createDrawableFromImage (iconToUse);
    addItem (std::move (i));
}

void PopupMenu::addCustomItem (int itemResultID,
                               std::unique_ptr<CustomComponent> cc,
                               std::unique_ptr<const PopupMenu> subMenu,
                               const String& itemTitle)
{
    Item i;
    i.text = itemTitle;
    i.itemID = itemResultID;
    i.customComponent = cc.release();
    i.subMenu.reset (createCopyIfNotNull (subMenu.get()));

    // If this assertion is hit, this item will be visible to screen readers but with
    // no name, which may be confusing to users.
    // It's probably a good idea to add a title for this menu item that describes
    // the meaning of the item, or the contents of the submenu, as appropriate.
    // If you don't want this menu item to be press-able directly, pass "false" to the
    // constructor of the CustomComponent.
    jassert (! (HelperClasses::ItemComponent::isAccessibilityHandlerRequired (i) && itemTitle.isEmpty()));

    addItem (std::move (i));
}

void PopupMenu::addCustomItem (int itemResultID,
                               Component& customComponent,
                               int idealWidth, int idealHeight,
                               bool triggerMenuItemAutomaticallyWhenClicked,
                               std::unique_ptr<const PopupMenu> subMenu,
                               const String& itemTitle)
{
    auto comp = std::make_unique<HelperClasses::NormalComponentWrapper> (customComponent, idealWidth, idealHeight,
                                                                         triggerMenuItemAutomaticallyWhenClicked);
    addCustomItem (itemResultID, std::move (comp), std::move (subMenu), itemTitle);
}

void PopupMenu::addSubMenu (String subMenuName, PopupMenu subMenu, bool isActive)
{
    addSubMenu (std::move (subMenuName), std::move (subMenu), isActive, nullptr, false, 0);
}

void PopupMenu::addSubMenu (String subMenuName, PopupMenu subMenu, bool isActive,
                            const Image& iconToUse, bool isTicked, int itemResultID)
{
    addSubMenu (std::move (subMenuName), std::move (subMenu), isActive,
                createDrawableFromImage (iconToUse), isTicked, itemResultID);
}

void PopupMenu::addSubMenu (String subMenuName, PopupMenu subMenu, bool isActive,
                            std::unique_ptr<Drawable> iconToUse, bool isTicked, int itemResultID)
{
    Item i (std::move (subMenuName));
    i.itemID = itemResultID;
    i.isEnabled = isActive && (itemResultID != 0 || subMenu.getNumItems() > 0);
    i.subMenu.reset (new PopupMenu (std::move (subMenu)));
    i.isTicked = isTicked;
    i.image = std::move (iconToUse);
    addItem (std::move (i));
}

void PopupMenu::addSeparator()
{
    if (items.size() > 0 && ! items.getLast().isSeparator)
    {
        Item i;
        i.isSeparator = true;
        addItem (std::move (i));
    }
}

void PopupMenu::addSectionHeader (String title)
{
    Item i (std::move (title));
    i.itemID = 0;
    i.isSectionHeader = true;
    addItem (std::move (i));
}

void PopupMenu::addColumnBreak()
{
    if (! items.isEmpty())
        std::prev (items.end())->shouldBreakAfter = true;
}

//==============================================================================
PopupMenu::Options::Options()
{
    targetArea.setPosition (Desktop::getMousePosition());
}

template <typename Member, typename Item>
static PopupMenu::Options with (PopupMenu::Options options, Member&& member, Item&& item)
{
    options.*member = std::forward<Item> (item);
    return options;
}

PopupMenu::Options PopupMenu::Options::withTargetComponent (Component* comp) const
{
    auto o = with (with (*this, &Options::targetComponent, comp), &Options::topLevelTarget, comp);

    if (comp != nullptr)
        o.targetArea = comp->getScreenBounds();

    return o;
}

PopupMenu::Options PopupMenu::Options::withTargetComponent (Component& comp) const
{
    return withTargetComponent (&comp);
}

PopupMenu::Options PopupMenu::Options::withTargetScreenArea (Rectangle<int> area) const
{
    return with (*this, &Options::targetArea, area);
}

PopupMenu::Options PopupMenu::Options::withMousePosition() const
{
    return withTargetScreenArea (Rectangle<int>{}.withPosition (Desktop::getMousePosition()));
}

PopupMenu::Options PopupMenu::Options::withDeletionCheck (Component& comp) const
{
    return with (with (*this, &Options::isWatchingForDeletion, true),
                 &Options::componentToWatchForDeletion,
                 &comp);
}

PopupMenu::Options PopupMenu::Options::withMinimumWidth (int w) const
{
    return with (*this, &Options::minWidth, w);
}

PopupMenu::Options PopupMenu::Options::withMinimumNumColumns (int cols) const
{
    return with (*this, &Options::minColumns, cols);
}

PopupMenu::Options PopupMenu::Options::withMaximumNumColumns (int cols) const
{
    return with (*this, &Options::maxColumns, cols);
}

PopupMenu::Options PopupMenu::Options::withStandardItemHeight (int height) const
{
    return with (*this, &Options::standardHeight, height);
}

PopupMenu::Options PopupMenu::Options::withItemThatMustBeVisible (int idOfItemToBeVisible) const
{
    return with (*this, &Options::visibleItemID, idOfItemToBeVisible);
}

PopupMenu::Options PopupMenu::Options::withParentComponent (Component* parent) const
{
    return with (*this, &Options::parentComponent, parent);
}

PopupMenu::Options PopupMenu::Options::withPreferredPopupDirection (PopupDirection direction) const
{
    return with (*this, &Options::preferredPopupDirection, direction);
}

PopupMenu::Options PopupMenu::Options::withInitiallySelectedItem (int idOfItemToBeSelected) const
{
    return with (*this, &Options::initiallySelectedItemId, idOfItemToBeSelected);
}

PopupMenu::Options PopupMenu::Options::forSubmenu() const
{
    return with (*this, &Options::targetComponent, nullptr);
}

Component* PopupMenu::createWindow (const Options& options,
                                    ApplicationCommandManager** managerOfChosenCommand) const
{
   #if JUCE_WINDOWS
    const auto scope = [&]() -> std::unique_ptr<ScopedThreadDPIAwarenessSetter>
    {
        if (auto* target = options.getTargetComponent())
            if (auto* handle = target->getWindowHandle())
                return std::make_unique<ScopedThreadDPIAwarenessSetter> (handle);

        return nullptr;
    }();
   #endif

    return items.isEmpty() ? nullptr
                           : new HelperClasses::MenuWindow (*this, nullptr, options,
                                                            ! options.getTargetScreenArea().isEmpty(),
                                                            managerOfChosenCommand);
}

//==============================================================================
// This invokes any command manager commands and deletes the menu window when it is dismissed
struct PopupMenuCompletionCallback final : public ModalComponentManager::Callback
{
    PopupMenuCompletionCallback() = default;

    void modalStateFinished (int result) override
    {
        if (managerOfChosenCommand != nullptr && result != 0)
        {
            ApplicationCommandTarget::InvocationInfo info (result);
            info.invocationMethod = ApplicationCommandTarget::InvocationInfo::fromMenu;

            managerOfChosenCommand->invoke (info, true);
        }

        // (this would be the place to fade out the component, if that's what's required)
        component.reset();

        if (PopupMenuSettings::menuWasHiddenBecauseOfAppChange)
            return;

        if (auto* focusComponent = Component::getCurrentlyFocusedComponent())
        {
            const auto focusedIsNotMinimised = [focusComponent]
            {
                if (auto* peer = focusComponent->getPeer())
                    return ! peer->isMinimised();

                return false;
            }();

            if (focusedIsNotMinimised)
            {
                if (auto* topLevel = focusComponent->getTopLevelComponent())
                    topLevel->toFront (true);

                if (focusComponent->isShowing() && ! focusComponent->hasKeyboardFocus (true))
                    focusComponent->grabKeyboardFocus();
            }
        }
    }

    ApplicationCommandManager* managerOfChosenCommand = nullptr;
    std::unique_ptr<Component> component;

    JUCE_DECLARE_NON_COPYABLE (PopupMenuCompletionCallback)
};

int PopupMenu::showWithOptionalCallback (const Options& options,
                                         ModalComponentManager::Callback* userCallback,
                                         [[maybe_unused]] bool canBeModal)
{
    std::unique_ptr<ModalComponentManager::Callback> userCallbackDeleter (userCallback);
    std::unique_ptr<PopupMenuCompletionCallback> callback (new PopupMenuCompletionCallback());

    if (auto* window = createWindow (options, &(callback->managerOfChosenCommand)))
    {
        callback->component.reset (window);

        PopupMenuSettings::menuWasHiddenBecauseOfAppChange = false;

        window->setVisible (true); // (must be called before enterModalState on Windows to avoid DropShadower confusion)
        window->enterModalState (false, userCallbackDeleter.release());
        ModalComponentManager::getInstance()->attachCallback (window, callback.release());

        window->toFront (false);  // need to do this after making it modal, or it could
                                  // be stuck behind other comps that are already modal..

       #if JUCE_MODAL_LOOPS_PERMITTED
        if (userCallback == nullptr && canBeModal)
            return window->runModalLoop();
       #else
        jassert (! (userCallback == nullptr && canBeModal));
       #endif
    }

    return 0;
}

//==============================================================================
#if JUCE_MODAL_LOOPS_PERMITTED
int PopupMenu::showMenu (const Options& options)
{
    return showWithOptionalCallback (options, nullptr, true);
}
#endif

void PopupMenu::showMenuAsync (const Options& options)
{
    showWithOptionalCallback (options, nullptr, false);
}

void PopupMenu::showMenuAsync (const Options& options, ModalComponentManager::Callback* userCallback)
{
   #if ! JUCE_MODAL_LOOPS_PERMITTED
    jassert (userCallback != nullptr);
   #endif

    showWithOptionalCallback (options, userCallback, false);
}

void PopupMenu::showMenuAsync (const Options& options, std::function<void (int)> userCallback)
{
    showWithOptionalCallback (options, ModalCallbackFunction::create (userCallback), false);
}

//==============================================================================
#if JUCE_MODAL_LOOPS_PERMITTED
int PopupMenu::show (int itemIDThatMustBeVisible, int minimumWidth,
                     int maximumNumColumns, int standardItemHeight,
                     ModalComponentManager::Callback* callback)
{
    return showWithOptionalCallback (Options().withItemThatMustBeVisible (itemIDThatMustBeVisible)
                                              .withMinimumWidth (minimumWidth)
                                              .withMaximumNumColumns (maximumNumColumns)
                                              .withStandardItemHeight (standardItemHeight),
                                     callback, true);
}

int PopupMenu::showAt (Rectangle<int> screenAreaToAttachTo,
                       int itemIDThatMustBeVisible, int minimumWidth,
                       int maximumNumColumns, int standardItemHeight,
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
                       int itemIDThatMustBeVisible, int minimumWidth,
                       int maximumNumColumns, int standardItemHeight,
                       ModalComponentManager::Callback* callback)
{
    auto options = Options().withItemThatMustBeVisible (itemIDThatMustBeVisible)
                            .withMinimumWidth (minimumWidth)
                            .withMaximumNumColumns (maximumNumColumns)
                            .withStandardItemHeight (standardItemHeight);

    if (componentToAttachTo != nullptr)
        options = options.withTargetComponent (componentToAttachTo);

    return showWithOptionalCallback (options, callback, true);
}
#endif

bool JUCE_CALLTYPE PopupMenu::dismissAllActiveMenus()
{
    auto& windows = HelperClasses::MenuWindow::getActiveWindows();
    auto numWindows = windows.size();

    for (int i = numWindows; --i >= 0;)
    {
        if (auto* pmw = windows[i])
        {
            pmw->setLookAndFeel (nullptr);
            pmw->dismissMenu (nullptr);
        }
    }

    return numWindows > 0;
}

//==============================================================================
int PopupMenu::getNumItems() const noexcept
{
    int num = 0;

    for (auto& mi : items)
        if (! mi.isSeparator)
            ++num;

    return num;
}

bool PopupMenu::containsCommandItem (const int commandID) const
{
    for (auto& mi : items)
        if ((mi.itemID == commandID && mi.commandManager != nullptr)
              || (mi.subMenu != nullptr && mi.subMenu->containsCommandItem (commandID)))
            return true;

    return false;
}

bool PopupMenu::containsAnyActiveItems() const noexcept
{
    for (auto& mi : items)
    {
        if (mi.subMenu != nullptr)
        {
            if (mi.subMenu->containsAnyActiveItems())
                return true;
        }
        else if (mi.isEnabled)
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

void PopupMenu::setItem (CustomComponent& c, const Item* itemToUse)
{
    c.item = itemToUse;
    c.repaint();
}

//==============================================================================
PopupMenu::CustomComponent::CustomComponent() : CustomComponent (true) {}

PopupMenu::CustomComponent::CustomComponent (bool autoTrigger)
    : triggeredAutomatically (autoTrigger)
{
}

void PopupMenu::CustomComponent::setHighlighted (bool shouldBeHighlighted)
{
    isHighlighted = shouldBeHighlighted;
    repaint();
}

void PopupMenu::CustomComponent::triggerMenuItem()
{
    if (auto* mic = findParentComponentOfClass<HelperClasses::ItemComponent>())
    {
        if (auto* pmw = mic->findParentComponentOfClass<HelperClasses::MenuWindow>())
        {
            pmw->dismissMenu (&mic->item);
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
PopupMenu::CustomCallback::CustomCallback() {}
PopupMenu::CustomCallback::~CustomCallback() {}

//==============================================================================
PopupMenu::MenuItemIterator::MenuItemIterator (const PopupMenu& m, bool recurse) : searchRecursively (recurse)
{
    index.add (0);
    menus.add (&m);
}

PopupMenu::MenuItemIterator::~MenuItemIterator() = default;

bool PopupMenu::MenuItemIterator::next()
{
    if (index.size() == 0 || menus.getLast()->items.size() == 0)
        return false;

    currentItem = const_cast<PopupMenu::Item*> (&(menus.getLast()->items.getReference (index.getLast())));

    if (searchRecursively && currentItem->subMenu != nullptr)
    {
        index.add (0);
        menus.add (currentItem->subMenu.get());
    }
    else
    {
        index.setUnchecked (index.size() - 1, index.getLast() + 1);
    }

    while (index.size() > 0 && index.getLast() >= (int) menus.getLast()->items.size())
    {
        index.removeLast();
        menus.removeLast();

        if (index.size() > 0)
            index.setUnchecked (index.size() - 1, index.getLast() + 1);
    }

    return true;
}

PopupMenu::Item& PopupMenu::MenuItemIterator::getItem() const
{
    jassert (currentItem != nullptr);
    return *(currentItem);
}

void PopupMenu::LookAndFeelMethods::drawPopupMenuBackground (Graphics&, int, int) {}

void PopupMenu::LookAndFeelMethods::drawPopupMenuItem (Graphics&, const Rectangle<int>&,
                                                       bool, bool, bool,
                                                       bool, bool,
                                                       const String&,
                                                       const String&,
                                                       const Drawable*,
                                                       const Colour*) {}

void PopupMenu::LookAndFeelMethods::drawPopupMenuSectionHeader (Graphics&, const Rectangle<int>&,
                                                                const String&) {}

void PopupMenu::LookAndFeelMethods::drawPopupMenuUpDownArrow (Graphics&, int, int, bool) {}

void PopupMenu::LookAndFeelMethods::getIdealPopupMenuItemSize (const String&, bool, int, int&, int&) {}

int PopupMenu::LookAndFeelMethods::getPopupMenuBorderSize() { return 0; }

} // namespace juce
