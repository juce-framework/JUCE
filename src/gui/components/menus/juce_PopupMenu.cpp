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
#include "juce_PopupMenuCustomComponent.h"
#include "../windows/juce_ComponentPeer.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../juce_Desktop.h"
#include "../../graphics/imaging/juce_Image.h"
#include "../keyboard/juce_KeyPressMappingSet.h"
#include "../../../events/juce_Timer.h"
#include "../../../threads/juce_Process.h"
#include "../../../core/juce_Time.h"


//==============================================================================
class PopupMenu::Item
{
public:
    //==============================================================================
    Item()
      : itemId (0), active (true), isSeparator (true), isTicked (false),
        usesColour (false), customComp (0), commandManager (0)
    {
    }

    Item (const int itemId_,
          const String& text_,
          const bool active_,
          const bool isTicked_,
          const Image* im,
          const Colour& textColour_,
          const bool usesColour_,
          PopupMenuCustomComponent* const customComp_,
          const PopupMenu* const subMenu_,
          ApplicationCommandManager* const commandManager_)
      : itemId (itemId_), text (text_), textColour (textColour_),
        active (active_), isSeparator (false), isTicked (isTicked_),
        usesColour (usesColour_), customComp (customComp_),
        commandManager (commandManager_)
    {
        if (subMenu_ != 0)
            subMenu = new PopupMenu (*subMenu_);

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

    Item (const Item& other)
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
    }

    ~Item()
    {
        customComp = 0;
    }

    bool canBeTriggered() const throw()
    {
        return active && ! (isSeparator || (subMenu != 0));
    }

    bool hasActiveSubMenu() const throw()
    {
        return active && (subMenu != 0);
    }

    //==============================================================================
    const int itemId;
    String text;
    const Colour textColour;
    const bool active, isSeparator, isTicked, usesColour;
    ScopedPointer <Image> image;
    ReferenceCountedObjectPtr <PopupMenuCustomComponent> customComp;
    ScopedPointer <PopupMenu> subMenu;
    ApplicationCommandManager* const commandManager;

    juce_UseDebuggingNewOperator

private:
    Item& operator= (const Item&);
};


//==============================================================================
class PopupMenu::ItemComponent  : public Component
{
public:
    //==============================================================================
    ItemComponent (const PopupMenu::Item& itemInfo_)
      : itemInfo (itemInfo_),
        isHighlighted (false)
    {
        if (itemInfo.customComp != 0)
            addAndMakeVisible (itemInfo.customComp);
    }

    ~ItemComponent()
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
            const int endIndex = mainText.indexOf ("<end>");

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

    PopupMenu::Item itemInfo;

    juce_UseDebuggingNewOperator

private:
    bool isHighlighted;

    ItemComponent (const ItemComponent&);
    ItemComponent& operator= (const ItemComponent&);
};


//==============================================================================
namespace PopupMenuSettings
{
    static const int scrollZone = 24;
    static const int borderSize = 2;
    static const int timerInterval = 50;
    static const int dismissCommandId = 0x6287345f;
}

//==============================================================================
class PopupMenu::Window  : public Component,
                           private Timer
{
public:
    //==============================================================================
    Window()
       : Component ("menu"),
         owner (0),
         currentChild (0),
         activeSubMenu (0),
         menuBarComponent (0),
         managerOfChosenCommand (0),
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

        getActiveWindows().add (this);
    }

    ~Window()
    {
        getActiveWindows().removeValue (this);

        Desktop::getInstance().removeGlobalMouseListener (this);

        jassert (activeSubMenu == 0 || activeSubMenu->isValidComponent());
        activeSubMenu = 0;

        deleteAllChildren();
    }

    //==============================================================================
    static Window* create (const PopupMenu& menu,
                           const bool dismissOnMouseUp,
                           Window* const owner_,
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

            ScopedPointer <Window> mw (new Window());
            mw->setLookAndFeel (menu.lookAndFeel);
            mw->setWantsKeyboardFocus (false);
            mw->minimumWidth = minimumWidth;
            mw->maximumNumColumns = maximumNumColumns;
            mw->standardItemHeight = standardItemHeight;
            mw->dismissOnMouseUp = dismissOnMouseUp;

            for (int i = 0; i < menu.items.size(); ++i)
            {
                PopupMenu::Item* const item = menu.items.getUnchecked(i);

                mw->addItem (*item);
                ++totalItems;
            }

            if (totalItems > 0)
            {
                mw->owner = owner_;
                mw->menuBarComponent = menuBarComponent;
                mw->managerOfChosenCommand = managerOfChosenCommand;
                mw->componentAttachedTo = componentAttachedTo;
                mw->componentAttachedToOriginal = componentAttachedTo;

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
                lf.drawPopupMenuUpDownArrow (g, getWidth(), PopupMenuSettings::scrollZone, true);

            if (isScrollZoneActive (true))
            {
                g.setOrigin (0, getHeight() - PopupMenuSettings::scrollZone);
                lf.drawPopupMenuUpDownArrow (g, getWidth(), PopupMenuSettings::scrollZone, false);
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
    void addItem (const PopupMenu::Item& item)
    {
        PopupMenu::ItemComponent* const mic = new PopupMenu::ItemComponent (item);
        addAndMakeVisible (mic);

        int itemW = 80;
        int itemH = 16;
        mic->getIdealSize (itemW, itemH, standardItemHeight);
        mic->setSize (itemW, jlimit (2, 600, itemH));
        mic->addMouseListener (this, false);
    }

    //==============================================================================
    // hide this and all sub-comps
    void hide (const PopupMenu::Item* const item)
    {
        if (isVisible())
        {
            jassert (activeSubMenu == 0 || activeSubMenu->isValidComponent());

            activeSubMenu = 0;
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

    void dismissMenu (const PopupMenu::Item* const item)
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
                const PopupMenu::Item mi (*item);
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
        alterChildYPos (roundToInt (-10.0f * amountY * PopupMenuSettings::scrollZone));
        lastMouse = Point<int> (-1, -1);
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
            if (owner != 0)
            {
                Component::SafePointer<Window> parentWindow (owner);
                PopupMenu::ItemComponent* currentChildOfParent = parentWindow->currentChild;

                hide (0);

                if (parentWindow != 0)
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
            if (componentAttachedTo != 0)
            {
                // we want to dismiss the menu, but if we do it synchronously, then
                // the mouse-click will be allowed to pass through. That's good, except
                // when the user clicks on the button that orginally popped the menu up,
                // as they'll expect the menu to go away, and in fact it'll just
                // come back. So only dismiss synchronously if they're not on the original
                // comp that we're attached to.
                const Point<int> mousePos (componentAttachedTo->getMouseXYRelative());

                if (componentAttachedTo->reallyContains (mousePos.getX(), mousePos.getY(), true))
                {
                    postCommandMessage (PopupMenuSettings::dismissCommandId); // dismiss asynchrounously
                    return;
                }
            }

            dismissMenu (0);
        }
    }

    void handleCommandMessage (int commandId)
    {
        Component::handleCommandMessage (commandId);

        if (commandId == PopupMenuSettings::dismissCommandId)
            dismissMenu (0);
    }

    //==============================================================================
    void timerCallback()
    {
        if (! isVisible())
            return;

        if (componentAttachedTo != componentAttachedToOriginal)
        {
            dismissMenu (0);
            return;
        }

        Window* currentlyModalWindow = dynamic_cast <Window*> (Component::getCurrentlyModalComponent());

        if (currentlyModalWindow != 0 && ! treeContains (currentlyModalWindow))
            return;

        startTimer (PopupMenuSettings::timerInterval);  // do this in case it was called from a mouse
                                                        // move rather than a real timer callback

        const Point<int> globalMousePos (Desktop::getMousePosition());
        const Point<int> localMousePos (globalPositionToRelative (globalMousePos));

        const uint32 now = Time::getMillisecondCounter();

        if (now > timeEnteredCurrentChildComp + 100
             && reallyContains (localMousePos.getX(), localMousePos.getY(), true)
             && currentChild->isValidComponent()
             && (! disableMouseMoves)
             && ! (activeSubMenu != 0 && activeSubMenu->isVisible()))
        {
            showSubMenuFor (currentChild);
        }

        if (globalMousePos != lastMouse || now > lastMouseMoveTime + 350)
        {
            highlightItemUnderMouse (globalMousePos, localMousePos);
        }

        bool overScrollArea = false;

        if (isScrolling()
             && (isOver || (isDown && ((unsigned int) localMousePos.getX()) < (unsigned int) getWidth()))
             && ((isScrollZoneActive (false) && localMousePos.getY() < PopupMenuSettings::scrollZone)
                  || (isScrollZoneActive (true) && localMousePos.getY() > getHeight() - PopupMenuSettings::scrollZone)))
        {
            if (now > lastScroll + 20)
            {
                scrollAcceleration = jmin (4.0, scrollAcceleration * 1.04);
                int amount = 0;

                for (int i = 0; i < getNumChildComponents() && amount == 0; ++i)
                    amount = ((int) scrollAcceleration) * getChildComponent (i)->getHeight();

                alterChildYPos (localMousePos.getY() < PopupMenuSettings::scrollZone ? -amount : amount);

                lastScroll = now;
            }

            overScrollArea = true;
            lastMouse = Point<int> (-1, -1); // trigger a mouse-move
        }
        else
        {
            scrollAcceleration = 1.0;
        }

        const bool wasDown = isDown;
        bool isOverAny = isOverAnyMenu();

        if (hideOnExit && hasBeenOver && (! isOverAny) && activeSubMenu != 0)
        {
            activeSubMenu->updateMouseOverStatus (globalMousePos);
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
                    wasHiddenBecauseOfAppChange() = true;
                    dismissMenu (0);

                    return;  // may have been deleted by the previous call..
                }
            }
            else if (wasDown && now > menuCreationTime + 250
                       && ! (isDown || overScrollArea))
            {
                isOver = reallyContains (localMousePos.getX(), localMousePos.getY(), true);

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

    static Array<Window*>& getActiveWindows()
    {
        static Array<Window*> activeMenuWindows;
        return activeMenuWindows;
    }

    static bool& wasHiddenBecauseOfAppChange() throw()
    {
        static bool b = false;
        return b;
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    Window* owner;
    PopupMenu::ItemComponent* currentChild;
    ScopedPointer <Window> activeSubMenu;
    Component* menuBarComponent;
    ApplicationCommandManager** managerOfChosenCommand;
    Component::SafePointer<Component> componentAttachedTo;
    Component* componentAttachedToOriginal;
    Rectangle<int> windowPos;
    Point<int> lastMouse;
    int minimumWidth, maximumNumColumns, standardItemHeight;
    bool isOver, hasBeenOver, isDown, needsToScroll;
    bool dismissOnMouseUp, hideOnExit, disableMouseMoves, hasAnyJuceCompHadFocus;
    int numColumns, contentHeight, childYOffset;
    Array <int> columnWidths;
    uint32 menuCreationTime, lastFocused, lastScroll, lastMouseMoveTime, timeEnteredCurrentChildComp;
    double scrollAcceleration;

    //==============================================================================
    bool overlaps (const Rectangle<int>& r) const
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

    void updateMouseOverStatus (const Point<int>& globalMousePos)
    {
        const Point<int> relPos (globalPositionToRelative (globalMousePos));
        isOver = reallyContains (relPos.getX(), relPos.getY(), true);

        if (activeSubMenu != 0)
            activeSubMenu->updateMouseOverStatus (globalMousePos);
    }

    bool treeContains (const Window* const window) const throw()
    {
        const Window* mw = this;

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
        const Rectangle<int> mon (Desktop::getInstance()
                                     .getMonitorAreaContaining (Point<int> ((minX + maxX) / 2,
                                                                            (minY + maxY) / 2),
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
        height = actualH + PopupMenuSettings::borderSize * 2;
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

            colW = jmin (maxMenuW / jmax (1, numColumns - 2), colW + PopupMenuSettings::borderSize * 2);

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
            PopupMenu::ItemComponent* const m = static_cast <PopupMenu::ItemComponent*> (getChildComponent (i));

            if (m != 0
                && m->itemInfo.itemId == itemId
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

                    const Rectangle<int> mon (Desktop::getInstance().getMonitorAreaContaining (windowPos.getPosition(), true));

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
            const int numChildren = jmin (getNumChildComponents() - childNum,
                                          (getNumChildComponents() + numColumns - 1) / numColumns);

            const int colW = columnWidths [col];

            int y = PopupMenuSettings::borderSize - (childYOffset + (getY() - windowPos.getY()));

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

    bool isScrolling() const throw()
    {
        return childYOffset != 0 || needsToScroll;
    }

    void setCurrentlyHighlightedChild (PopupMenu::ItemComponent* const child)
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

    bool showSubMenuFor (PopupMenu::ItemComponent* const childComp)
    {
        jassert (activeSubMenu == 0 || activeSubMenu->isValidComponent());
        activeSubMenu = 0;

        if (childComp->isValidComponent() && childComp->itemInfo.hasActiveSubMenu())
        {
            const Point<int> topLeft (childComp->relativePositionToGlobal (Point<int>()));
            const Point<int> bottomRight (childComp->relativePositionToGlobal (Point<int> (childComp->getWidth(), childComp->getHeight())));

            activeSubMenu = Window::create (*(childComp->itemInfo.subMenu),
                                            dismissOnMouseUp,
                                            this,
                                            topLeft.getX(), bottomRight.getX(), topLeft.getY(), bottomRight.getY(),
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

    void highlightItemUnderMouse (const Point<int>& globalMousePos, const Point<int>& localMousePos)
    {
        isOver = reallyContains (localMousePos.getX(), localMousePos.getY(), true);

        if (isOver)
            hasBeenOver = true;

        if (lastMouse.getDistanceFrom (globalMousePos) > 2)
        {
            lastMouseMoveTime = Time::getApproximateMillisecondCounter();

            if (disableMouseMoves && isOver)
                disableMouseMoves = false;
        }

        if (disableMouseMoves || (activeSubMenu != 0 && activeSubMenu->isOverChildren()))
            return;

        bool isMovingTowardsMenu = false;

        jassert (activeSubMenu == 0 || activeSubMenu->isValidComponent())

        if (isOver && (activeSubMenu != 0) && globalMousePos != lastMouse)
        {
            // try to intelligently guess whether the user is moving the mouse towards a currently-open
            // submenu. To do this, look at whether the mouse stays inside a triangular region that
            // extends from the last mouse pos to the submenu's rectangle..

            float subX = (float) activeSubMenu->getScreenX();

            if (activeSubMenu->getX() > getX())
            {
                lastMouse -= Point<int> (2, 0);  // to enlarge the triangle a bit, in case the mouse only moves a couple of pixels
            }
            else
            {
                lastMouse += Point<int> (2, 0);
                subX += activeSubMenu->getWidth();
            }

            Path areaTowardsSubMenu;
            areaTowardsSubMenu.addTriangle ((float) lastMouse.getX(),
                                            (float) lastMouse.getY(),
                                            subX,
                                            (float) activeSubMenu->getScreenY(),
                                            subX,
                                            (float) (activeSubMenu->getScreenY() + activeSubMenu->getHeight()));

            isMovingTowardsMenu = areaTowardsSubMenu.contains ((float) globalMousePos.getX(), (float) globalMousePos.getY());
        }

        lastMouse = globalMousePos;

        if (! isMovingTowardsMenu)
        {
            Component* c = getComponentAt (localMousePos.getX(), localMousePos.getY());
            if (c == this)
                c = 0;

            PopupMenu::ItemComponent* mic = dynamic_cast <PopupMenu::ItemComponent*> (c);

            if (mic == 0 && c != 0)
                mic = c->findParentComponentOfClass ((PopupMenu::ItemComponent*) 0);

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
        PopupMenu::ItemComponent* mic = 0;
        bool wasLastOne = (currentChild == 0);
        const int numItems = getNumChildComponents();

        for (int i = 0; i < numItems + 1; ++i)
        {
            int index = (delta > 0) ? i : (numItems - 1 - i);
            index = (index + numItems) % numItems;

            mic = dynamic_cast <PopupMenu::ItemComponent*> (getChildComponent (index));

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

    Window (const Window&);
    Window& operator= (const Window&);
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
        items.add (new Item (*other.items.getUnchecked(i)));
}

PopupMenu& PopupMenu::operator= (const PopupMenu& other)
{
    if (this != &other)
    {
        lookAndFeel = other.lookAndFeel;

        clear();
        items.ensureStorageAllocated (other.items.size());

        for (int i = 0; i < other.items.size(); ++i)
            items.add (new Item (*other.items.getUnchecked(i)));
    }

    return *this;
}

PopupMenu::~PopupMenu()
{
    clear();
}

void PopupMenu::clear()
{
    items.clear();
    separatorPending = false;
}

void PopupMenu::addSeparatorIfPending()
{
    if (separatorPending)
    {
        separatorPending = false;

        if (items.size() > 0)
            items.add (new Item());
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

    items.add (new Item (itemResultId, itemText, isActive, isTicked,
                         iconToUse, Colours::black, false, 0, 0, 0));
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

        items.add (new Item (commandID,
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

    items.add (new Item (itemResultId, itemText, isActive, isTicked,
                         iconToUse, itemTextColour, true, 0, 0, 0));
}

//==============================================================================
void PopupMenu::addCustomItem (const int itemResultId,
                               PopupMenuCustomComponent* const customComponent)
{
    jassert (itemResultId != 0);    // 0 is used as a return value to indicate that the user
                                    // didn't pick anything, so you shouldn't use it as the id
                                    // for an item..

    addSeparatorIfPending();

    items.add (new Item (itemResultId, String::empty, true, false, 0,
                         Colours::black, false, customComponent, 0, 0));
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
    NormalComponentWrapper& operator= (const NormalComponentWrapper&);
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

    items.add (new Item (0, subMenuName, isActive && (subMenu.getNumItems() > 0), isTicked,
                         iconToUse, Colours::black, false, 0, &subMenu, 0));
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
    Window* const pw
        = Window::create (*this,
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
    Component::SafePointer<Component> prevFocused (Component::getCurrentlyFocusedComponent());
    Component::SafePointer<Component> prevTopLevel ((prevFocused != 0) ? prevFocused->getTopLevelComponent() : 0);

    Window::wasHiddenBecauseOfAppChange() = false;

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

        if (! Window::wasHiddenBecauseOfAppChange())
        {
            if (prevTopLevel != 0)
                prevTopLevel->toFront (true);

            if (prevFocused != 0)
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
    const Point<int> mousePos (Desktop::getMousePosition());

    return showAt (mousePos.getX(), mousePos.getY(),
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
    for (int i = Window::getActiveWindows().size(); --i >= 0;)
    {
        Window* const pmw = Window::getActiveWindows()[i];

        if (pmw != 0)
            pmw->dismissMenu (0);
    }
}

//==============================================================================
int PopupMenu::getNumItems() const throw()
{
    int num = 0;

    for (int i = items.size(); --i >= 0;)
        if (! (items.getUnchecked(i))->isSeparator)
            ++num;

    return num;
}

bool PopupMenu::containsCommandItem (const int commandID) const
{
    for (int i = items.size(); --i >= 0;)
    {
        const Item* mi = items.getUnchecked (i);

        if ((mi->itemId == commandID && mi->commandManager != 0)
            || (mi->subMenu != 0 && mi->subMenu->containsCommandItem (commandID)))
        {
            return true;
        }
    }

    return false;
}

bool PopupMenu::containsAnyActiveItems() const throw()
{
    for (int i = items.size(); --i >= 0;)
    {
        const Item* const mi = items.getUnchecked (i);

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
    : isHighlighted (false),
      isTriggeredAutomatically (isTriggeredAutomatically_)
{
}

PopupMenuCustomComponent::~PopupMenuCustomComponent()
{
}

void PopupMenuCustomComponent::triggerMenuItem()
{
    PopupMenu::ItemComponent* const mic = dynamic_cast <PopupMenu::ItemComponent*> (getParentComponent());

    if (mic != 0)
    {
        PopupMenu::Window* const pmw = dynamic_cast <PopupMenu::Window*> (mic->getParentComponent());

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

    const Item* const item = menu.items.getUnchecked (index);
    ++index;

    itemName = item->customComp != 0 ? item->customComp->getName() : item->text;
    subMenu = item->subMenu;
    itemId = item->itemId;

    isSeparator = item->isSeparator;
    isTicked = item->isTicked;
    isEnabled = item->active;
    isSectionHeader = dynamic_cast <HeaderItemComponent*> ((PopupMenuCustomComponent*) item->customComp) != 0;
    isCustomComponent = (! isSectionHeader) && item->customComp != 0;
    customColour = item->usesColour ? &(item->textColour) : 0;
    customImage = item->image;
    commandManager = item->commandManager;

    return true;
}


END_JUCE_NAMESPACE
