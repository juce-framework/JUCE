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

#include "juce_PopupMenu.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../juce_ComponentDeletionWatcher.h"
#include "../juce_Desktop.h"
#include "../../graphics/imaging/juce_Image.h"
#include "../keyboard/juce_KeyPressMappingSet.h"
#include "../../../events/juce_Timer.h"
#include "../../../threads/juce_Process.h"
#include "../../../core/juce_Time.h"

static VoidArray activeMenuWindows;


//==============================================================================
class MenuItemInfo
{
public:
    //==============================================================================
    const int itemId;
    String text;
    const Colour textColour;
    const bool active, isSeparator, isTicked, usesColour;
    ScopedPointer <Image> image;
    PopupMenuCustomComponent* const customComp;
    ScopedPointer <PopupMenu> subMenu;
    ApplicationCommandManager* const commandManager;

    //==============================================================================
    MenuItemInfo()
        : itemId (0),
          active (true),
          isSeparator (true),
          isTicked (false),
          usesColour (false),
          customComp (0),
          commandManager (0)
    {
    }

    MenuItemInfo (const int itemId_,
                  const String& text_,
                  const bool active_,
                  const bool isTicked_,
                  const Image* im,
                  const Colour& textColour_,
                  const bool usesColour_,
                  PopupMenuCustomComponent* const customComp_,
                  const PopupMenu* const subMenu_,
                  ApplicationCommandManager* const commandManager_)
        : itemId (itemId_),
          text (text_),
          textColour (textColour_),
          active (active_),
          isSeparator (false),
          isTicked (isTicked_),
          usesColour (usesColour_),
          customComp (customComp_),
          commandManager (commandManager_)
    {
        if (subMenu_ != 0)
            subMenu = new PopupMenu (*subMenu_);

        if (customComp != 0)
            customComp->refCount_++;

        if (im != 0)
            image = im->createCopy();

        if (commandManager_ != 0 && itemId_ != 0)
        {
            String shortcutKey;

            Array <KeyPress> keyPresses (commandManager_->getKeyMappings()
                                            ->getKeyPressesAssignedToCommand (itemId_));

            for (int i = 0; i < keyPresses.size(); ++i)
            {
                const String key (keyPresses.getReference(i).getTextDescription());

                if (shortcutKey.isNotEmpty())
                    shortcutKey << ", ";

                if (key.length() == 1)
                    shortcutKey << "shortcut: '" << key << '\'';
                else
                    shortcutKey << key;
            }

            shortcutKey = shortcutKey.trim();

            if (shortcutKey.isNotEmpty())
                text << "<end>" << shortcutKey;
        }
    }

    MenuItemInfo (const MenuItemInfo& other)
        : itemId (other.itemId),
          text (other.text),
          textColour (other.textColour),
          active (other.active),
          isSeparator (other.isSeparator),
          isTicked (other.isTicked),
          usesColour (other.usesColour),
          customComp (other.customComp),
          commandManager (other.commandManager)
    {
        if (other.subMenu != 0)
            subMenu = new PopupMenu (*(other.subMenu));

        if (other.image != 0)
            image = other.image->createCopy();

        if (customComp != 0)
            customComp->refCount_++;
    }

    ~MenuItemInfo()
    {
        if (customComp != 0 && --(customComp->refCount_) == 0)
            delete customComp;
    }

    bool canBeTriggered() const
    {
        return active && ! (isSeparator || (subMenu != 0));
    }

    bool hasActiveSubMenu() const
    {
        return active && (subMenu != 0);
    }

    juce_UseDebuggingNewOperator

private:
    const MenuItemInfo& operator= (const MenuItemInfo&);
};

//==============================================================================
class MenuItemComponent  : public Component
{
    bool isHighlighted;

public:
    MenuItemInfo itemInfo;

    //==============================================================================
    MenuItemComponent (const MenuItemInfo& itemInfo_)
      : isHighlighted (false),
        itemInfo (itemInfo_)
    {
        if (itemInfo.customComp != 0)
            addAndMakeVisible (itemInfo.customComp);
    }

    ~MenuItemComponent()
    {
        if (itemInfo.customComp != 0)
            removeChildComponent (itemInfo.customComp);
    }

    void getIdealSize (int& idealWidth,
                       int& idealHeight,
                       const int standardItemHeight)
    {
        if (itemInfo.customComp != 0)
        {
            itemInfo.customComp->getIdealSize (idealWidth, idealHeight);
        }
        else
        {
            getLookAndFeel().getIdealPopupMenuItemSize (itemInfo.text,
                                                        itemInfo.isSeparator,
                                                        standardItemHeight,
                                                        idealWidth,
                                                        idealHeight);
        }
    }

    void paint (Graphics& g)
    {
        if (itemInfo.customComp == 0)
        {
            String mainText (itemInfo.text);
            String endText;
            const int endIndex = mainText.indexOf (T("<end>"));

            if (endIndex >= 0)
            {
                endText = mainText.substring (endIndex + 5).trim();
                mainText = mainText.substring (0, endIndex);
            }

            getLookAndFeel()
                .drawPopupMenuItem (g, getWidth(), getHeight(),
                                    itemInfo.isSeparator,
                                    itemInfo.active,
                                    isHighlighted,
                                    itemInfo.isTicked,
                                    itemInfo.subMenu != 0,
                                    mainText, endText,
                                    itemInfo.image,
                                    itemInfo.usesColour ? &(itemInfo.textColour) : 0);
        }
    }

    void resized()
    {
        if (getNumChildComponents() > 0)
            getChildComponent(0)->setBounds (2, 0, getWidth() - 4, getHeight());
    }

    void setHighlighted (bool shouldBeHighlighted)
    {
        shouldBeHighlighted = shouldBeHighlighted && itemInfo.active;

        if (isHighlighted != shouldBeHighlighted)
        {
            isHighlighted = shouldBeHighlighted;

            if (itemInfo.customComp != 0)
            {
                itemInfo.customComp->isHighlighted = shouldBeHighlighted;
                itemInfo.customComp->repaint();
            }

            repaint();
        }
    }

private:
    MenuItemComponent (const MenuItemComponent&);
    const MenuItemComponent& operator= (const MenuItemComponent&);
};


//==============================================================================
static const int scrollZone = 24;
static const int borderSize = 2;
static const int timerInterval = 50;
static const int dismissCommandId = 0x6287345f;

static bool wasHiddenBecauseOfAppChange = false;

//==============================================================================
class PopupMenuWindow  : public Component,
                         private Timer
{
public:
    //==============================================================================
    PopupMenuWindow()
       : Component (T("menu")),
         owner (0),
         currentChild (0),
         activeSubMenu (0),
         menuBarComponent (0),
         managerOfChosenCommand (0),
         componentAttachedTo (0),
         lastMouseX (0),
         lastMouseY (0),
         minimumWidth (0),
         maximumNumColumns (7),
         standardItemHeight (0),
         isOver (false),
         hasBeenOver (false),
         isDown (false),
         needsToScroll (false),
         hideOnExit (false),
         disableMouseMoves (false),
         hasAnyJuceCompHadFocus (false),
         numColumns (0),
         contentHeight (0),
         childYOffset (0),
         timeEnteredCurrentChildComp (0),
         scrollAcceleration (1.0)
    {
        menuCreationTime = lastFocused = lastScroll = Time::getMillisecondCounter();
        setWantsKeyboardFocus (true);
        setMouseClickGrabsKeyboardFocus (false);

        setOpaque (true);
        setAlwaysOnTop (true);

        Desktop::getInstance().addGlobalMouseListener (this);

        activeMenuWindows.add (this);
    }

    ~PopupMenuWindow()
    {
        activeMenuWindows.removeValue (this);

        Desktop::getInstance().removeGlobalMouseListener (this);

        jassert (activeSubMenu == 0 || activeSubMenu->isValidComponent());
        delete activeSubMenu;

        deleteAllChildren();
        attachedCompWatcher = 0;
    }

    //==============================================================================
    static PopupMenuWindow* create (const PopupMenu& menu,
                                    const bool dismissOnMouseUp,
                                    PopupMenuWindow* const owner_,
                                    const int minX, const int maxX,
                                    const int minY, const int maxY,
                                    const int minimumWidth,
                                    const int maximumNumColumns,
                                    const int standardItemHeight,
                                    const bool alignToRectangle,
                                    const int itemIdThatMustBeVisible,
                                    Component* const menuBarComponent,
                                    ApplicationCommandManager** managerOfChosenCommand,
                                    Component* const componentAttachedTo)
    {
        if (menu.items.size() > 0)
        {
            int totalItems = 0;

            ScopedPointer <PopupMenuWindow> mw (new PopupMenuWindow());
            mw->setLookAndFeel (menu.lookAndFeel);
            mw->setWantsKeyboardFocus (false);
            mw->minimumWidth = minimumWidth;
            mw->maximumNumColumns = maximumNumColumns;
            mw->standardItemHeight = standardItemHeight;
            mw->dismissOnMouseUp = dismissOnMouseUp;

            for (int i = 0; i < menu.items.size(); ++i)
            {
                MenuItemInfo* const item = (MenuItemInfo*) menu.items.getUnchecked(i);

                mw->addItem (*item);
                ++totalItems;
            }

            if (totalItems > 0)
            {
                mw->owner = owner_;
                mw->menuBarComponent = menuBarComponent;
                mw->managerOfChosenCommand = managerOfChosenCommand;
                mw->componentAttachedTo = componentAttachedTo;
                mw->attachedCompWatcher = componentAttachedTo != 0 ? new ComponentDeletionWatcher (componentAttachedTo) : 0;

                mw->calculateWindowPos (minX, maxX, minY, maxY, alignToRectangle);
                mw->setTopLeftPosition (mw->windowPos.getX(),
                                        mw->windowPos.getY());
                mw->updateYPositions();

                if (itemIdThatMustBeVisible != 0)
                {
                    const int y = minY - mw->windowPos.getY();
                    mw->ensureItemIsVisible (itemIdThatMustBeVisible,
                                             (((unsigned int) y) < (unsigned int) mw->windowPos.getHeight()) ? y : -1);
                }

                mw->resizeToBestWindowPos();
                mw->addToDesktop (ComponentPeer::windowIsTemporary
                                    | mw->getLookAndFeel().getMenuWindowFlags());

                return mw.release();
            }
        }

        return 0;
    }

    //==============================================================================
    void paint (Graphics& g)
    {
        getLookAndFeel().drawPopupMenuBackground (g, getWidth(), getHeight());
    }

    void paintOverChildren (Graphics& g)
    {
        if (isScrolling())
        {
            LookAndFeel& lf = getLookAndFeel();

            if (isScrollZoneActive (false))
                lf.drawPopupMenuUpDownArrow (g, getWidth(), scrollZone, true);

            if (isScrollZoneActive (true))
            {
                g.setOrigin (0, getHeight() - scrollZone);
                lf.drawPopupMenuUpDownArrow (g, getWidth(), scrollZone, false);
            }
        }
    }

    bool isScrollZoneActive (bool bottomOne) const
    {
        return isScrolling()
                && (bottomOne
                        ? childYOffset < contentHeight - windowPos.getHeight()
                        : childYOffset > 0);
    }

    //==============================================================================
    void addItem (const MenuItemInfo& item)
    {
        MenuItemComponent* const mic = new MenuItemComponent (item);
        addAndMakeVisible (mic);

        int itemW = 80;
        int itemH = 16;
        mic->getIdealSize (itemW, itemH, standardItemHeight);
        mic->setSize (itemW, jlimit (2, 600, itemH));
        mic->addMouseListener (this, false);
    }

    //==============================================================================
    // hide this and all sub-comps
    void hide (const MenuItemInfo* const item)
    {
        if (isVisible())
        {
            jassert (activeSubMenu == 0 || activeSubMenu->isValidComponent());

            deleteAndZero (activeSubMenu);
            currentChild = 0;

            exitModalState (item != 0 ? item->itemId : 0);
            setVisible (false);

            if (item != 0
                 && item->commandManager != 0
                 && item->itemId != 0)
            {
                *managerOfChosenCommand = item->commandManager;
            }
        }
    }

    void dismissMenu (const MenuItemInfo* const item)
    {
        if (owner != 0)
        {
            owner->dismissMenu (item);
        }
        else
        {
            if (item != 0)
            {
                // need a copy of this on the stack as the one passed in will get deleted during this call
                const MenuItemInfo mi (*item);
                hide (&mi);
            }
            else
            {
                hide (0);
            }
        }
    }

    //==============================================================================
    void mouseMove (const MouseEvent&)
    {
        timerCallback();
    }

    void mouseDown (const MouseEvent&)
    {
        timerCallback();
    }

    void mouseDrag (const MouseEvent&)
    {
        timerCallback();
    }

    void mouseUp (const MouseEvent&)
    {
        timerCallback();
    }

    void mouseWheelMove (const MouseEvent&, float /*amountX*/, float amountY)
    {
        alterChildYPos (roundToInt (-10.0f * amountY * scrollZone));
        lastMouseX = -1;
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
            PopupMenuWindow* parentWindow = owner;

            if (parentWindow != 0)
            {
                MenuItemComponent* currentChildOfParent
                    = (parentWindow != 0) ? parentWindow->currentChild : 0;

                hide (0);

                if (parentWindow->isValidComponent())
                    parentWindow->setCurrentlyHighlightedChild (currentChildOfParent);

                disableTimerUntilMouseMoves();
            }
            else if (menuBarComponent != 0)
            {
                menuBarComponent->keyPressed (key);
            }
        }
        else if (key.isKeyCode (KeyPress::rightKey))
        {
            disableTimerUntilMouseMoves();

            if (showSubMenuFor (currentChild))
            {
                jassert (activeSubMenu == 0 || activeSubMenu->isValidComponent());

                if (activeSubMenu != 0 && activeSubMenu->isVisible())
                    activeSubMenu->selectNextItem (1);
            }
            else if (menuBarComponent != 0)
            {
                menuBarComponent->keyPressed (key);
            }
        }
        else if (key.isKeyCode (KeyPress::returnKey))
        {
            triggerCurrentlyHighlightedItem();
        }
        else if (key.isKeyCode (KeyPress::escapeKey))
        {
            dismissMenu (0);
        }
        else
        {
            return false;
        }

        return true;
    }

    void inputAttemptWhenModal()
    {
        timerCallback();

        if (! isOverAnyMenu())
        {
            if (componentAttachedTo != 0 && ! attachedCompWatcher->hasBeenDeleted())
            {
                // we want to dismiss the menu, but if we do it synchronously, then
                // the mouse-click will be allowed to pass through. That's good, except
                // when the user clicks on the button that orginally popped the menu up,
                // as they'll expect the menu to go away, and in fact it'll just
                // come back. So only dismiss synchronously if they're not on the original
                // comp that we're attached to.
                int mx, my;
                componentAttachedTo->getMouseXYRelative (mx, my);

                if (componentAttachedTo->reallyContains (mx, my, true))
                {
                    postCommandMessage (dismissCommandId); // dismiss asynchrounously
                    return;
                }
            }

            dismissMenu (0);
        }
    }

    void handleCommandMessage (int commandId)
    {
        Component::handleCommandMessage (commandId);

        if (commandId == dismissCommandId)
            dismissMenu (0);
    }

    //==============================================================================
    void timerCallback()
    {
        if (! isVisible())
            return;

        if (attachedCompWatcher != 0 && attachedCompWatcher->hasBeenDeleted())
        {
            dismissMenu (0);
            return;
        }

        PopupMenuWindow* currentlyModalWindow = dynamic_cast <PopupMenuWindow*> (Component::getCurrentlyModalComponent());

        if (currentlyModalWindow != 0 && ! treeContains (currentlyModalWindow))
            return;

        startTimer (timerInterval);  // do this in case it was called from a mouse
                                     // move rather than a real timer callback

        int mx, my;
        Desktop::getMousePosition (mx, my);

        int x = mx, y = my;
        globalPositionToRelative (x, y);

        const uint32 now = Time::getMillisecondCounter();

        if (now > timeEnteredCurrentChildComp + 100
             && reallyContains (x, y, true)
             && currentChild->isValidComponent()
             && (! disableMouseMoves)
             && ! (activeSubMenu != 0 && activeSubMenu->isVisible()))
        {
            showSubMenuFor (currentChild);
        }

        if (mx != lastMouseX || my != lastMouseY || now > lastMouseMoveTime + 350)
        {
            highlightItemUnderMouse (mx, my, x, y);
        }

        bool overScrollArea = false;

        if (isScrolling()
             && (isOver || (isDown && ((unsigned int) x) < (unsigned int) getWidth()))
             && ((isScrollZoneActive (false) && y < scrollZone)
                  || (isScrollZoneActive (true) && y > getHeight() - scrollZone)))
        {
            if (now > lastScroll + 20)
            {
                scrollAcceleration = jmin (4.0, scrollAcceleration * 1.04);
                int amount = 0;

                for (int i = 0; i < getNumChildComponents() && amount == 0; ++i)
                    amount = ((int) scrollAcceleration) * getChildComponent (i)->getHeight();

                alterChildYPos (y < scrollZone ? -amount : amount);

                lastScroll = now;
            }

            overScrollArea = true;
            lastMouseX = -1; // trigger a mouse-move
        }
        else
        {
            scrollAcceleration = 1.0;
        }

        const bool wasDown = isDown;
        bool isOverAny = isOverAnyMenu();

        if (hideOnExit && hasBeenOver && (! isOverAny) && activeSubMenu != 0)
        {
            activeSubMenu->updateMouseOverStatus (mx, my);
            isOverAny = isOverAnyMenu();
        }

        if (hideOnExit && hasBeenOver && ! isOverAny)
        {
            hide (0);
        }
        else
        {
            isDown = hasBeenOver
                        && (ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown()
                            || ModifierKeys::getCurrentModifiersRealtime().isAnyMouseButtonDown());

            bool anyFocused = Process::isForegroundProcess();

            if (anyFocused && Component::getCurrentlyFocusedComponent() == 0)
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

            if (! anyFocused)
            {
                if (now > lastFocused + 10)
                {
                    wasHiddenBecauseOfAppChange = true;
                    dismissMenu (0);

                    return;  // may have been deleted by the previous call..
                }
            }
            else if (wasDown && now > menuCreationTime + 250
                       && ! (isDown || overScrollArea))
            {
                isOver = reallyContains (x, y, true);

                if (isOver)
                {
                    triggerCurrentlyHighlightedItem();
                }
                else if ((hasBeenOver || ! dismissOnMouseUp) && ! isOverAny)
                {
                    dismissMenu (0);
                }

                return;  // may have been deleted by the previous calls..
            }
            else
            {
                lastFocused = now;
            }
        }
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    PopupMenuWindow* owner;
    MenuItemComponent* currentChild;
    PopupMenuWindow* activeSubMenu;
    Component* menuBarComponent;
    ApplicationCommandManager** managerOfChosenCommand;
    Component* componentAttachedTo;
    ScopedPointer <ComponentDeletionWatcher> attachedCompWatcher;
    Rectangle windowPos;
    int lastMouseX, lastMouseY;
    int minimumWidth, maximumNumColumns, standardItemHeight;
    bool isOver, hasBeenOver, isDown, needsToScroll;
    bool dismissOnMouseUp, hideOnExit, disableMouseMoves, hasAnyJuceCompHadFocus;
    int numColumns, contentHeight, childYOffset;
    Array <int> columnWidths;
    uint32 menuCreationTime, lastFocused, lastScroll, lastMouseMoveTime, timeEnteredCurrentChildComp;
    double scrollAcceleration;

    //==============================================================================
    bool overlaps (const Rectangle& r) const
    {
        return r.intersects (getBounds())
                || (owner != 0 && owner->overlaps (r));
    }

    bool isOverAnyMenu() const
    {
        return (owner != 0) ? owner->isOverAnyMenu()
                            : isOverChildren();
    }

    bool isOverChildren() const
    {
        jassert (activeSubMenu == 0 || activeSubMenu->isValidComponent());

        return isVisible()
                && (isOver || (activeSubMenu != 0 && activeSubMenu->isOverChildren()));
    }

    void updateMouseOverStatus (const int mx, const int my)
    {
        int rx = mx, ry = my;
        globalPositionToRelative (rx, ry);
        isOver = reallyContains (rx, ry, true);

        if (activeSubMenu != 0)
            activeSubMenu->updateMouseOverStatus (mx, my);
    }

    bool treeContains (const PopupMenuWindow* const window) const
    {
        const PopupMenuWindow* mw = this;

        while (mw->owner != 0)
            mw = mw->owner;

        while (mw != 0)
        {
            if (mw == window)
                return true;

            mw = mw->activeSubMenu;
        }

        return false;
    }

    //==============================================================================
    void calculateWindowPos (const int minX, const int maxX,
                             const int minY, const int maxY,
                             const bool alignToRectangle)
    {
        const Rectangle mon (Desktop::getInstance()
                                .getMonitorAreaContaining ((minX + maxX) / 2,
                                                           (minY + maxY) / 2,
#if JUCE_MAC
                                                           true));
#else
                                                           false)); // on windows, don't stop the menu overlapping the taskbar
#endif

        int x, y, widthToUse, heightToUse;
        layoutMenuItems (mon.getWidth() - 24, widthToUse, heightToUse);

        if (alignToRectangle)
        {
            x = minX;

            const int spaceUnder = mon.getHeight() - (maxY - mon.getY());
            const int spaceOver = minY - mon.getY();

            if (heightToUse < spaceUnder - 30 || spaceUnder >= spaceOver)
                y = maxY;
            else
                y = minY - heightToUse;
        }
        else
        {
            bool tendTowardsRight = (minX + maxX) / 2 < mon.getCentreX();

            if (owner != 0)
            {
                if (owner->owner != 0)
                {
                    const bool ownerGoingRight = (owner->getX() + owner->getWidth() / 2
                                                    > owner->owner->getX() + owner->owner->getWidth() / 2);

                    if (ownerGoingRight && maxX + widthToUse < mon.getRight() - 4)
                        tendTowardsRight = true;
                    else if ((! ownerGoingRight) && minX > widthToUse + 4)
                        tendTowardsRight = false;
                }
                else if (maxX + widthToUse < mon.getRight() - 32)
                {
                    tendTowardsRight = true;
                }
            }

            const int biggestSpace = jmax (mon.getRight() - maxX,
                                           minX - mon.getX()) - 32;

            if (biggestSpace < widthToUse)
            {
                layoutMenuItems (biggestSpace + (maxX - minX) / 3, widthToUse, heightToUse);

                if (numColumns > 1)
                    layoutMenuItems (biggestSpace - 4, widthToUse, heightToUse);

                tendTowardsRight = (mon.getRight() - maxX) >= (minX - mon.getX());
            }

            if (tendTowardsRight)
                x = jmin (mon.getRight() - widthToUse - 4, maxX);
            else
                x = jmax (mon.getX() + 4, minX - widthToUse);

            y = minY;
            if ((minY + maxY) / 2 > mon.getCentreY())
                y = jmax (mon.getY(), maxY - heightToUse);
        }

        x = jmax (mon.getX() + 1, jmin (mon.getRight() - (widthToUse + 6), x));
        y = jmax (mon.getY() + 1, jmin (mon.getBottom() - (heightToUse + 6), y));

        windowPos.setBounds (x, y, widthToUse, heightToUse);

        // sets this flag if it's big enough to obscure any of its parent menus
        hideOnExit = (owner != 0)
                      && owner->windowPos.intersects (windowPos.expanded (-4, -4));
    }

    void layoutMenuItems (const int maxMenuW, int& width, int& height)
    {
        numColumns = 0;
        contentHeight = 0;
        const int maxMenuH = getParentHeight() - 24;
        int totalW;

        do
        {
            ++numColumns;
            totalW = workOutBestSize (maxMenuW);

            if (totalW > maxMenuW)
            {
                numColumns = jmax (1, numColumns - 1);
                totalW = workOutBestSize (maxMenuW); // to update col widths
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
        height = actualH + borderSize * 2;
    }

    int workOutBestSize (const int maxMenuW)
    {
        int totalW = 0;
        contentHeight = 0;
        int childNum = 0;

        for (int col = 0; col < numColumns; ++col)
        {
            int i, colW = 50, colH = 0;

            const int numChildren = jmin (getNumChildComponents() - childNum,
                                          (getNumChildComponents() + numColumns - 1) / numColumns);

            for (i = numChildren; --i >= 0;)
            {
                colW = jmax (colW, getChildComponent (childNum + i)->getWidth());
                colH += getChildComponent (childNum + i)->getHeight();
            }

            colW = jmin (maxMenuW / jmax (1, numColumns - 2), colW + borderSize * 2);

            columnWidths.set (col, colW);
            totalW += colW;
            contentHeight = jmax (contentHeight, colH);

            childNum += numChildren;
        }

        if (totalW < minimumWidth)
        {
            totalW = minimumWidth;

            for (int col = 0; col < numColumns; ++col)
                columnWidths.set (0, totalW / numColumns);
        }

        return totalW;
    }

    void ensureItemIsVisible (const int itemId, int wantedY)
    {
        jassert (itemId != 0)

        for (int i = getNumChildComponents(); --i >= 0;)
        {
            MenuItemComponent* const m = (MenuItemComponent*) getChildComponent (i);

            if (m != 0
                && m->itemInfo.itemId == itemId
                && windowPos.getHeight() > scrollZone * 4)
            {
                const int currentY = m->getY();

                if (wantedY > 0 || currentY < 0 || m->getBottom() > windowPos.getHeight())
                {
                    if (wantedY < 0)
                        wantedY = jlimit (scrollZone,
                                          jmax (scrollZone, windowPos.getHeight() - (scrollZone + m->getHeight())),
                                          currentY);

                    const Rectangle mon (Desktop::getInstance()
                                         .getMonitorAreaContaining (windowPos.getX(),
                                                                    windowPos.getY(),
                                                                    true));

                    int deltaY = wantedY - currentY;

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
        Rectangle r (windowPos);

        if (childYOffset < 0)
        {
            r.setBounds (r.getX(), r.getY() - childYOffset,
                         r.getWidth(), r.getHeight() + childYOffset);
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
        if (isScrolling())
        {
            childYOffset += delta;

            if (delta < 0)
            {
                childYOffset = jmax (childYOffset, 0);
            }
            else if (delta > 0)
            {
                childYOffset = jmin (childYOffset,
                                     contentHeight - windowPos.getHeight() + borderSize);
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
            const int numChildren = jmin (getNumChildComponents() - childNum,
                                          (getNumChildComponents() + numColumns - 1) / numColumns);

            const int colW = columnWidths [col];

            int y = borderSize - (childYOffset + (getY() - windowPos.getY()));

            for (int i = 0; i < numChildren; ++i)
            {
                Component* const c = getChildComponent (childNum + i);
                c->setBounds (x, y, colW, c->getHeight());
                y += c->getHeight();
            }

            x += colW;
            childNum += numChildren;
        }

        return x;
    }

    bool isScrolling() const
    {
        return childYOffset != 0 || needsToScroll;
    }

    void setCurrentlyHighlightedChild (MenuItemComponent* const child)
    {
        if (currentChild->isValidComponent())
            currentChild->setHighlighted (false);

        currentChild = child;

        if (currentChild != 0)
        {
            currentChild->setHighlighted (true);
            timeEnteredCurrentChildComp = Time::getApproximateMillisecondCounter();
        }
    }

    bool showSubMenuFor (MenuItemComponent* const childComp)
    {
        jassert (activeSubMenu == 0 || activeSubMenu->isValidComponent());
        deleteAndZero (activeSubMenu);

        if (childComp->isValidComponent() && childComp->itemInfo.hasActiveSubMenu())
        {
            int left = 0, top = 0;
            childComp->relativePositionToGlobal (left, top);
            int right = childComp->getWidth(), bottom = childComp->getHeight();
            childComp->relativePositionToGlobal (right, bottom);

            activeSubMenu = PopupMenuWindow::create (*(childComp->itemInfo.subMenu),
                                                     dismissOnMouseUp,
                                                     this,
                                                     left, right, top, bottom,
                                                     0, maximumNumColumns,
                                                     standardItemHeight,
                                                     false, 0, menuBarComponent,
                                                     managerOfChosenCommand,
                                                     componentAttachedTo);

            if (activeSubMenu != 0)
            {
                activeSubMenu->setVisible (true);
                activeSubMenu->enterModalState (false);
                activeSubMenu->toFront (false);
                return true;
            }
        }

        return false;
    }

    void highlightItemUnderMouse (const int mx, const int my, const int x, const int y)
    {
        isOver = reallyContains (x, y, true);

        if (isOver)
            hasBeenOver = true;

        if (abs (lastMouseX - mx) > 2 || abs (lastMouseY - my) > 2)
        {
            lastMouseMoveTime = Time::getApproximateMillisecondCounter();

            if (disableMouseMoves && isOver)
                disableMouseMoves = false;
        }

        if (disableMouseMoves)
            return;

        bool isMovingTowardsMenu = false;

        jassert (activeSubMenu == 0 || activeSubMenu->isValidComponent())

        if (isOver && (activeSubMenu != 0) && (mx != lastMouseX || my != lastMouseY))
        {
            // try to intelligently guess whether the user is moving the mouse towards a currently-open
            // submenu. To do this, look at whether the mouse stays inside a triangular region that
            // extends from the last mouse pos to the submenu's rectangle..

            float subX = (float) activeSubMenu->getScreenX();

            if (activeSubMenu->getX() > getX())
            {
                lastMouseX -= 2;  // to enlarge the triangle a bit, in case the mouse only moves a couple of pixels
            }
            else
            {
                lastMouseX += 2;
                subX += activeSubMenu->getWidth();
            }

            Path areaTowardsSubMenu;
            areaTowardsSubMenu.addTriangle ((float) lastMouseX,
                                            (float) lastMouseY,
                                            subX,
                                            (float) activeSubMenu->getScreenY(),
                                            subX,
                                            (float) (activeSubMenu->getScreenY() + activeSubMenu->getHeight()));

            isMovingTowardsMenu = areaTowardsSubMenu.contains ((float) mx, (float) my);
        }

        lastMouseX = mx;
        lastMouseY = my;

        if (! isMovingTowardsMenu)
        {
            Component* c = getComponentAt (x, y);
            if (c == this)
                c = 0;

            MenuItemComponent* mic = dynamic_cast <MenuItemComponent*> (c);

            if (mic == 0 && c != 0)
                mic = c->findParentComponentOfClass ((MenuItemComponent*) 0);

            if (mic != currentChild
                 && (isOver || (activeSubMenu == 0) || ! activeSubMenu->isVisible()))
            {
                if (isOver && (c != 0) && (activeSubMenu != 0))
                {
                    activeSubMenu->hide (0);
                }

                if (! isOver)
                    mic = 0;

                setCurrentlyHighlightedChild (mic);
            }
        }
    }

    void triggerCurrentlyHighlightedItem()
    {
        if (currentChild->isValidComponent()
             && currentChild->itemInfo.canBeTriggered()
             && (currentChild->itemInfo.customComp == 0
                  || currentChild->itemInfo.customComp->isTriggeredAutomatically))
        {
            dismissMenu (&currentChild->itemInfo);
        }
    }

    void selectNextItem (const int delta)
    {
        disableTimerUntilMouseMoves();
        MenuItemComponent* mic = 0;
        bool wasLastOne = (currentChild == 0);
        const int numItems = getNumChildComponents();

        for (int i = 0; i < numItems + 1; ++i)
        {
            int index = (delta > 0) ? i : (numItems - 1 - i);
            index = (index + numItems) % numItems;

            mic = dynamic_cast <MenuItemComponent*> (getChildComponent (index));

            if (mic != 0 && (mic->itemInfo.canBeTriggered() || mic->itemInfo.hasActiveSubMenu())
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

        if (owner != 0)
            owner->disableTimerUntilMouseMoves();
    }

    PopupMenuWindow (const PopupMenuWindow&);
    const PopupMenuWindow& operator= (const PopupMenuWindow&);
};


//==============================================================================
PopupMenu::PopupMenu()
    : lookAndFeel (0),
      separatorPending (false)
{
}

PopupMenu::PopupMenu (const PopupMenu& other)
    : lookAndFeel (other.lookAndFeel),
      separatorPending (false)
{
    items.ensureStorageAllocated (other.items.size());

    for (int i = 0; i < other.items.size(); ++i)
        items.add (new MenuItemInfo (*(const MenuItemInfo*) other.items.getUnchecked(i)));
}

const PopupMenu& PopupMenu::operator= (const PopupMenu& other)
{
    if (this != &other)
    {
        lookAndFeel = other.lookAndFeel;

        clear();
        items.ensureStorageAllocated (other.items.size());

        for (int i = 0; i < other.items.size(); ++i)
            items.add (new MenuItemInfo (*(const MenuItemInfo*) other.items.getUnchecked(i)));
    }

    return *this;
}

PopupMenu::~PopupMenu()
{
    clear();
}

void PopupMenu::clear()
{
    for (int i = items.size(); --i >= 0;)
        delete (MenuItemInfo*) items.getUnchecked(i);

    items.clear();
    separatorPending = false;
}

void PopupMenu::addSeparatorIfPending()
{
    if (separatorPending)
    {
        separatorPending = false;

        if (items.size() > 0)
            items.add (new MenuItemInfo());
    }
}

void PopupMenu::addItem (const int itemResultId,
                         const String& itemText,
                         const bool isActive,
                         const bool isTicked,
                         const Image* const iconToUse)
{
    jassert (itemResultId != 0);    // 0 is used as a return value to indicate that the user
                                    // didn't pick anything, so you shouldn't use it as the id
                                    // for an item..

    addSeparatorIfPending();

    items.add (new MenuItemInfo (itemResultId,
                                 itemText,
                                 isActive,
                                 isTicked,
                                 iconToUse,
                                 Colours::black,
                                 false,
                                 0, 0, 0));
}

void PopupMenu::addCommandItem (ApplicationCommandManager* commandManager,
                                const int commandID,
                                const String& displayName)
{
    jassert (commandManager != 0 && commandID != 0);

    const ApplicationCommandInfo* const registeredInfo = commandManager->getCommandForID (commandID);

    if (registeredInfo != 0)
    {
        ApplicationCommandInfo info (*registeredInfo);
        ApplicationCommandTarget* const target = commandManager->getTargetForCommand (commandID, info);

        addSeparatorIfPending();

        items.add (new MenuItemInfo (commandID,
                                     displayName.isNotEmpty() ? displayName
                                                              : info.shortName,
                                     target != 0 && (info.flags & ApplicationCommandInfo::isDisabled) == 0,
                                     (info.flags & ApplicationCommandInfo::isTicked) != 0,
                                     0,
                                     Colours::black,
                                     false,
                                     0, 0,
                                     commandManager));
    }
}

void PopupMenu::addColouredItem (const int itemResultId,
                                 const String& itemText,
                                 const Colour& itemTextColour,
                                 const bool isActive,
                                 const bool isTicked,
                                 const Image* const iconToUse)
{
    jassert (itemResultId != 0);    // 0 is used as a return value to indicate that the user
                                    // didn't pick anything, so you shouldn't use it as the id
                                    // for an item..

    addSeparatorIfPending();

    items.add (new MenuItemInfo (itemResultId,
                                 itemText,
                                 isActive,
                                 isTicked,
                                 iconToUse,
                                 itemTextColour,
                                 true,
                                 0, 0, 0));
}

//==============================================================================
void PopupMenu::addCustomItem (const int itemResultId,
                               PopupMenuCustomComponent* const customComponent)
{
    jassert (itemResultId != 0);    // 0 is used as a return value to indicate that the user
                                    // didn't pick anything, so you shouldn't use it as the id
                                    // for an item..

    addSeparatorIfPending();

    items.add (new MenuItemInfo (itemResultId,
                                 String::empty,
                                 true,
                                 false,
                                 0,
                                 Colours::black,
                                 false,
                                 customComponent,
                                 0, 0));
}

class NormalComponentWrapper : public PopupMenuCustomComponent
{
public:
    NormalComponentWrapper (Component* const comp,
                            const int w, const int h,
                            const bool triggerMenuItemAutomaticallyWhenClicked)
        : PopupMenuCustomComponent (triggerMenuItemAutomaticallyWhenClicked),
          width (w),
          height (h)
    {
        addAndMakeVisible (comp);
    }

    ~NormalComponentWrapper() {}

    void getIdealSize (int& idealWidth, int& idealHeight)
    {
        idealWidth = width;
        idealHeight = height;
    }

    void resized()
    {
        if (getChildComponent(0) != 0)
            getChildComponent(0)->setBounds (0, 0, getWidth(), getHeight());
    }

    juce_UseDebuggingNewOperator

private:
    const int width, height;

    NormalComponentWrapper (const NormalComponentWrapper&);
    const NormalComponentWrapper& operator= (const NormalComponentWrapper&);
};

void PopupMenu::addCustomItem (const int itemResultId,
                               Component* customComponent,
                               int idealWidth, int idealHeight,
                               const bool triggerMenuItemAutomaticallyWhenClicked)
{
    addCustomItem (itemResultId,
                   new NormalComponentWrapper (customComponent,
                                               idealWidth, idealHeight,
                                               triggerMenuItemAutomaticallyWhenClicked));
}

//==============================================================================
void PopupMenu::addSubMenu (const String& subMenuName,
                            const PopupMenu& subMenu,
                            const bool isActive,
                            Image* const iconToUse,
                            const bool isTicked)
{
    addSeparatorIfPending();

    items.add (new MenuItemInfo (0,
                                 subMenuName,
                                 isActive && (subMenu.getNumItems() > 0),
                                 isTicked,
                                 iconToUse,
                                 Colours::black,
                                 false,
                                 0,
                                 &subMenu,
                                 0));
}

void PopupMenu::addSeparator()
{
    separatorPending = true;
}


//==============================================================================
class HeaderItemComponent  : public PopupMenuCustomComponent
{
public:
    HeaderItemComponent (const String& name)
        : PopupMenuCustomComponent (false)
    {
        setName (name);
    }

    ~HeaderItemComponent()
    {
    }

    void paint (Graphics& g)
    {
        Font f (getLookAndFeel().getPopupMenuFont());
        f.setBold (true);
        g.setFont (f);

        g.setColour (findColour (PopupMenu::headerTextColourId));

        g.drawFittedText (getName(),
                          12, 0, getWidth() - 16, proportionOfHeight (0.8f),
                          Justification::bottomLeft, 1);
    }

    void getIdealSize (int& idealWidth,
                       int& idealHeight)
    {
        getLookAndFeel().getIdealPopupMenuItemSize (getName(), false, -1, idealWidth, idealHeight);
        idealHeight += idealHeight / 2;
        idealWidth += idealWidth / 4;
    }

    juce_UseDebuggingNewOperator
};

void PopupMenu::addSectionHeader (const String& title)
{
    addCustomItem (0X4734a34f, new HeaderItemComponent (title));
}

//==============================================================================
Component* PopupMenu::createMenuComponent (const int x, const int y, const int w, const int h,
                                           const int itemIdThatMustBeVisible,
                                           const int minimumWidth,
                                           const int maximumNumColumns,
                                           const int standardItemHeight,
                                           const bool alignToRectangle,
                                           Component* menuBarComponent,
                                           ApplicationCommandManager** managerOfChosenCommand,
                                           Component* const componentAttachedTo)
{
    PopupMenuWindow* const pw
        = PopupMenuWindow::create (*this,
                                   ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown(),
                                   0,
                                   x, x + w,
                                   y, y + h,
                                   minimumWidth,
                                   maximumNumColumns,
                                   standardItemHeight,
                                   alignToRectangle,
                                   itemIdThatMustBeVisible,
                                   menuBarComponent,
                                   managerOfChosenCommand,
                                   componentAttachedTo);

    if (pw != 0)
        pw->setVisible (true);

    return pw;
}

int PopupMenu::showMenu (const int x, const int y, const int w, const int h,
                         const int itemIdThatMustBeVisible,
                         const int minimumWidth,
                         const int maximumNumColumns,
                         const int standardItemHeight,
                         const bool alignToRectangle,
                         Component* const componentAttachedTo)
{
    Component* const prevFocused = Component::getCurrentlyFocusedComponent();

    ScopedPointer <ComponentDeletionWatcher> deletionChecker[2];
    if (prevFocused != 0)
        deletionChecker[0] = new ComponentDeletionWatcher (prevFocused);

    Component* const prevTopLevel = (prevFocused != 0) ? prevFocused->getTopLevelComponent() : 0;

    if (prevTopLevel != 0)
        deletionChecker[1] = new ComponentDeletionWatcher (prevTopLevel);

    wasHiddenBecauseOfAppChange = false;

    int result = 0;
    ApplicationCommandManager* managerOfChosenCommand = 0;

    ScopedPointer <Component> popupComp (createMenuComponent (x, y, w, h,
                                                              itemIdThatMustBeVisible,
                                                              minimumWidth,
                                                              maximumNumColumns > 0 ? maximumNumColumns : 7,
                                                              standardItemHeight,
                                                              alignToRectangle, 0,
                                                              &managerOfChosenCommand,
                                                              componentAttachedTo));

    if (popupComp != 0)
    {
        popupComp->enterModalState (false);
        popupComp->toFront (false);  // need to do this after making it modal, or it could
                                     // be stuck behind other comps that are already modal..

        result = popupComp->runModalLoop();
        popupComp = 0;

        if (! wasHiddenBecauseOfAppChange)
        {
            if (deletionChecker[1] != 0 && ! deletionChecker[1]->hasBeenDeleted())
                prevTopLevel->toFront (true);

            if (deletionChecker[0] != 0 && ! deletionChecker[0]->hasBeenDeleted())
                prevFocused->grabKeyboardFocus();
        }
    }

    if (managerOfChosenCommand != 0 && result != 0)
    {
        ApplicationCommandTarget::InvocationInfo info (result);
        info.invocationMethod = ApplicationCommandTarget::InvocationInfo::fromMenu;

        managerOfChosenCommand->invoke (info, true);
    }

    return result;
}

int PopupMenu::show (const int itemIdThatMustBeVisible,
                     const int minimumWidth,
                     const int maximumNumColumns,
                     const int standardItemHeight)
{
    int x, y;
    Desktop::getMousePosition (x, y);

    return showAt (x, y,
                   itemIdThatMustBeVisible,
                   minimumWidth,
                   maximumNumColumns,
                   standardItemHeight);
}

int PopupMenu::showAt (const int screenX,
                       const int screenY,
                       const int itemIdThatMustBeVisible,
                       const int minimumWidth,
                       const int maximumNumColumns,
                       const int standardItemHeight)
{
    return showMenu (screenX, screenY, 1, 1,
                     itemIdThatMustBeVisible,
                     minimumWidth, maximumNumColumns,
                     standardItemHeight,
                     false, 0);
}

int PopupMenu::showAt (Component* componentToAttachTo,
                       const int itemIdThatMustBeVisible,
                       const int minimumWidth,
                       const int maximumNumColumns,
                       const int standardItemHeight)
{
    if (componentToAttachTo != 0)
    {
        return showMenu (componentToAttachTo->getScreenX(),
                         componentToAttachTo->getScreenY(),
                         componentToAttachTo->getWidth(),
                         componentToAttachTo->getHeight(),
                         itemIdThatMustBeVisible,
                         minimumWidth,
                         maximumNumColumns,
                         standardItemHeight,
                         true, componentToAttachTo);
    }
    else
    {
        return show (itemIdThatMustBeVisible,
                     minimumWidth,
                     maximumNumColumns,
                     standardItemHeight);
    }
}

void JUCE_CALLTYPE PopupMenu::dismissAllActiveMenus()
{
    for (int i = activeMenuWindows.size(); --i >= 0;)
    {
        PopupMenuWindow* const pmw = (PopupMenuWindow*) activeMenuWindows[i];

        if (pmw != 0)
            pmw->dismissMenu (0);
    }
}

//==============================================================================
int PopupMenu::getNumItems() const
{
    int num = 0;

    for (int i = items.size(); --i >= 0;)
        if (! ((MenuItemInfo*) items.getUnchecked(i))->isSeparator)
            ++num;

    return num;
}

bool PopupMenu::containsCommandItem (const int commandID) const
{
    for (int i = items.size(); --i >= 0;)
    {
        const MenuItemInfo* mi = (const MenuItemInfo*) items.getUnchecked (i);

        if ((mi->itemId == commandID && mi->commandManager != 0)
            || (mi->subMenu != 0 && mi->subMenu->containsCommandItem (commandID)))
        {
            return true;
        }
    }

    return false;
}

bool PopupMenu::containsAnyActiveItems() const
{
    for (int i = items.size(); --i >= 0;)
    {
        const MenuItemInfo* const mi = (const MenuItemInfo*) items.getUnchecked (i);

        if (mi->subMenu != 0)
        {
            if (mi->subMenu->containsAnyActiveItems())
                return true;
        }
        else if (mi->active)
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
PopupMenuCustomComponent::PopupMenuCustomComponent (const bool isTriggeredAutomatically_)
    : refCount_ (0),
      isHighlighted (false),
      isTriggeredAutomatically (isTriggeredAutomatically_)
{
}

PopupMenuCustomComponent::~PopupMenuCustomComponent()
{
    jassert (refCount_ == 0); // should be deleted only by the menu component, as they keep a ref-count.
}

void PopupMenuCustomComponent::triggerMenuItem()
{
    MenuItemComponent* const mic = dynamic_cast<MenuItemComponent*> (getParentComponent());

    if (mic != 0)
    {
        PopupMenuWindow* const pmw = dynamic_cast<PopupMenuWindow*> (mic->getParentComponent());

        if (pmw != 0)
        {
            pmw->dismissMenu (&mic->itemInfo);
        }
        else
        {
            // something must have gone wrong with the component hierarchy if this happens..
            jassertfalse
        }
    }
    else
    {
        // why isn't this component inside a menu? Not much point triggering the item if
        // there's no menu.
        jassertfalse
    }
}

//==============================================================================
PopupMenu::MenuItemIterator::MenuItemIterator (const PopupMenu& menu_)
    : subMenu (0),
      itemId (0),
      isSeparator (false),
      isTicked (false),
      isEnabled (false),
      isCustomComponent (false),
      isSectionHeader (false),
      customColour (0),
      customImage (0),
      menu (menu_),
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

    const MenuItemInfo* const item = (const MenuItemInfo*) menu.items.getUnchecked (index);
    ++index;

    itemName = item->customComp != 0 ? item->customComp->getName() : item->text;
    subMenu = item->subMenu;
    itemId = item->itemId;

    isSeparator = item->isSeparator;
    isTicked = item->isTicked;
    isEnabled = item->active;
    isSectionHeader = dynamic_cast <HeaderItemComponent*> (item->customComp) != 0;
    isCustomComponent = (! isSectionHeader) && item->customComp != 0;
    customColour = item->usesColour ? &(item->textColour) : 0;
    customImage = item->image;
    commandManager = item->commandManager;

    return true;
}


END_JUCE_NAMESPACE
