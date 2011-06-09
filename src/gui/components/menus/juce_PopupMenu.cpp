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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_PopupMenu.h"
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
        usesColour (false), commandManager (nullptr)
    {
    }

    Item (const int itemId_,
          const String& text_,
          const bool active_,
          const bool isTicked_,
          const Image& im,
          const Colour& textColour_,
          const bool usesColour_,
          CustomComponent* const customComp_,
          const PopupMenu* const subMenu_,
          ApplicationCommandManager* const commandManager_)
      : itemId (itemId_), text (text_), textColour (textColour_),
        active (active_), isSeparator (false), isTicked (isTicked_),
        usesColour (usesColour_), image (im), customComp (customComp_),
        commandManager (commandManager_)
    {
        if (subMenu_ != nullptr)
            subMenu = new PopupMenu (*subMenu_);

        if (commandManager_ != nullptr && itemId_ != 0)
        {
            String shortcutKey;

            Array <KeyPress> keyPresses (commandManager_->getKeyMappings()
                                            ->getKeyPressesAssignedToCommand (itemId_));

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
        : itemId (other.itemId),
          text (other.text),
          textColour (other.textColour),
          active (other.active),
          isSeparator (other.isSeparator),
          isTicked (other.isTicked),
          usesColour (other.usesColour),
          image (other.image),
          customComp (other.customComp),
          commandManager (other.commandManager)
    {
        if (other.subMenu != nullptr)
            subMenu = new PopupMenu (*(other.subMenu));
    }

    bool canBeTriggered() const noexcept    { return active && ! (isSeparator || (subMenu != nullptr)); }
    bool hasActiveSubMenu() const noexcept  { return active && (subMenu != nullptr); }

    //==============================================================================
    const int itemId;
    String text;
    const Colour textColour;
    const bool active, isSeparator, isTicked, usesColour;
    Image image;
    ReferenceCountedObjectPtr <CustomComponent> customComp;
    ScopedPointer <PopupMenu> subMenu;
    ApplicationCommandManager* const commandManager;

private:
    Item& operator= (const Item&);

    JUCE_LEAK_DETECTOR (Item);
};


//==============================================================================
class PopupMenu::ItemComponent  : public Component
{
public:
    //==============================================================================
    ItemComponent (const PopupMenu::Item& itemInfo_, int standardItemHeight, Component* const parent)
      : itemInfo (itemInfo_),
        isHighlighted (false)
    {
        if (itemInfo.customComp != nullptr)
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
        if (itemInfo.customComp != nullptr)
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
                                    itemInfo.active,
                                    isHighlighted,
                                    itemInfo.isTicked,
                                    itemInfo.subMenu != 0,
                                    mainText, endText,
                                    itemInfo.image.isValid() ? &itemInfo.image : 0,
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

            if (itemInfo.customComp != nullptr)
                itemInfo.customComp->setHighlighted (shouldBeHighlighted);

            repaint();
        }
    }

    PopupMenu::Item itemInfo;

private:
    bool isHighlighted;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ItemComponent);
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
    //==============================================================================
    Window (const PopupMenu& menu, Window* const owner_, const Rectangle<int>& target,
            const bool alignToRectangle, const int itemIdThatMustBeVisible,
            const int minimumWidth_, const int maximumNumColumns_,
            const int standardItemHeight_, const bool dismissOnMouseUp_,
            ApplicationCommandManager** const managerOfChosenCommand_,
            Component* const componentAttachedTo_)
       : Component ("menu"),
         owner (owner_),
         activeSubMenu (nullptr),
         managerOfChosenCommand (managerOfChosenCommand_),
         componentAttachedTo (componentAttachedTo_),
         componentAttachedToOriginal (componentAttachedTo_),
         minimumWidth (minimumWidth_),
         maximumNumColumns (maximumNumColumns_),
         standardItemHeight (standardItemHeight_),
         isOver (false),
         hasBeenOver (false),
         isDown (false),
         needsToScroll (false),
         dismissOnMouseUp (dismissOnMouseUp_),
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
        lastFocused = lastScroll = menuCreationTime;
        setWantsKeyboardFocus (false);
        setMouseClickGrabsKeyboardFocus (false);
        setAlwaysOnTop (true);

        setLookAndFeel (menu.lookAndFeel);
        setOpaque (getLookAndFeel().findColour (PopupMenu::backgroundColourId).isOpaque() || ! Desktop::canUseSemiTransparentWindows());

        for (int i = 0; i < menu.items.size(); ++i)
            items.add (new PopupMenu::ItemComponent (*menu.items.getUnchecked(i), standardItemHeight, this));

        calculateWindowPos (target, alignToRectangle);
        setTopLeftPosition (windowPos.getX(), windowPos.getY());
        updateYPositions();

        if (itemIdThatMustBeVisible != 0)
        {
            const int y = target.getY() - windowPos.getY();
            ensureItemIsVisible (itemIdThatMustBeVisible,
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
        getActiveWindows().removeValue (this);
        Desktop::getInstance().removeGlobalMouseListener (this);
        activeSubMenu = nullptr;
        items.clear();
    }

    //==============================================================================
    static Window* create (const PopupMenu& menu,
                           bool dismissOnMouseUp,
                           Window* const owner_,
                           const Rectangle<int>& target,
                           int minimumWidth,
                           int maximumNumColumns,
                           int standardItemHeight,
                           bool alignToRectangle,
                           int itemIdThatMustBeVisible,
                           ApplicationCommandManager** managerOfChosenCommand,
                           Component* componentAttachedTo)
    {
        if (menu.items.size() > 0)
            return new Window (menu, owner_, target, alignToRectangle, itemIdThatMustBeVisible,
                               minimumWidth, maximumNumColumns, standardItemHeight, dismissOnMouseUp,
                               managerOfChosenCommand, componentAttachedTo);

        return nullptr;
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
                && (bottomOne ? childYOffset < contentHeight - windowPos.getHeight()
                              : childYOffset > 0);
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
                 && item->itemId != 0)
            {
                *managerOfChosenCommand = item->commandManager;
            }

            exitModalState (item != nullptr ? item->itemId : 0);

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
                hide (0, false);
            }
        }
    }

    //==============================================================================
    void mouseMove (const MouseEvent&)    { timerCallback(); }
    void mouseDown (const MouseEvent&)    { timerCallback(); }
    void mouseDrag (const MouseEvent&)    { timerCallback(); }
    void mouseUp   (const MouseEvent&)    { timerCallback(); }

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
            if (owner != nullptr)
            {
                Component::SafePointer<Window> parentWindow (owner);
                PopupMenu::ItemComponent* currentChildOfParent = parentWindow->currentChild;

                hide (0, true);

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
                if (activeSubMenu != nullptr && activeSubMenu->isVisible())
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

        if (currentlyModalWindow != nullptr && ! treeContains (currentlyModalWindow))
            return;

        startTimer (PopupMenuSettings::timerInterval);  // do this in case it was called from a mouse
                                                        // move rather than a real timer callback

        const Point<int> globalMousePos (Desktop::getMousePosition());
        const Point<int> localMousePos (getLocalPoint (nullptr, globalMousePos));

        const uint32 now = Time::getMillisecondCounter();

        if (now > timeEnteredCurrentChildComp + 100
             && reallyContains (localMousePos, true)
             && currentChild != nullptr
             && (! disableMouseMoves)
             && ! (activeSubMenu != nullptr && activeSubMenu->isVisible()))
        {
            showSubMenuFor (currentChild);
        }

        if (globalMousePos != lastMouse || now > lastMouseMoveTime + 350)
        {
            highlightItemUnderMouse (globalMousePos, localMousePos);
        }

        bool overScrollArea = false;

        if (isScrolling()
             && (isOver || (isDown && isPositiveAndBelow (localMousePos.getX(), getWidth())))
             && ((isScrollZoneActive (false) && localMousePos.getY() < PopupMenuSettings::scrollZone)
                  || (isScrollZoneActive (true) && localMousePos.getY() > getHeight() - PopupMenuSettings::scrollZone)))
        {
            if (now > lastScroll + 20)
            {
                scrollAcceleration = jmin (4.0, scrollAcceleration * 1.04);
                int amount = 0;

                for (int i = 0; i < items.size() && amount == 0; ++i)
                    amount = ((int) scrollAcceleration) * items.getUnchecked(i)->getHeight();

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

        if (hideOnExit && hasBeenOver && (! isOverAny) && activeSubMenu != nullptr)
        {
            activeSubMenu->updateMouseOverStatus (globalMousePos);
            isOverAny = isOverAnyMenu();
        }

        if (hideOnExit && hasBeenOver && ! isOverAny)
        {
            hide (0, true);
        }
        else
        {
            isDown = hasBeenOver
                        && (ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown()
                            || ModifierKeys::getCurrentModifiersRealtime().isAnyMouseButtonDown());

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

            if (! anyFocused)
            {
                if (now > lastFocused + 10)
                {
                    PopupMenuSettings::menuWasHiddenBecauseOfAppChange = true;
                    dismissMenu (0);

                    return;  // may have been deleted by the previous call..
                }
            }
            else if (wasDown && now > menuCreationTime + 250
                       && ! (isDown || overScrollArea))
            {
                isOver = reallyContains (localMousePos, true);

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

    //==============================================================================
private:
    Window* owner;
    OwnedArray <PopupMenu::ItemComponent> items;
    Component::SafePointer<PopupMenu::ItemComponent> currentChild;
    ScopedPointer <Window> activeSubMenu;
    ApplicationCommandManager** managerOfChosenCommand;
    WeakReference<Component> componentAttachedTo;
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
        const Point<int> relPos (getLocalPoint (nullptr, globalMousePos));
        isOver = reallyContains (relPos, true);

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

    //==============================================================================
    void calculateWindowPos (const Rectangle<int>& target, const bool alignToRectangle)
    {
        const Rectangle<int> mon (Desktop::getInstance()
                                     .getMonitorAreaContaining (target.getCentre(),
                                                               #if JUCE_MAC
                                                                true));
                                                               #else
                                                                false)); // on windows, don't stop the menu overlapping the taskbar
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
            int i, colW = standardItemHeight, colH = 0;

            const int numChildren = jmin (items.size() - childNum,
                                          (items.size() + numColumns - 1) / numColumns);

            for (i = numChildren; --i >= 0;)
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

        for (int i = items.size(); --i >= 0;)
        {
            PopupMenu::ItemComponent* const m = items.getUnchecked(i);

            if (m != nullptr
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

    bool isScrolling() const noexcept
    {
        return childYOffset != 0 || needsToScroll;
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

    bool showSubMenuFor (PopupMenu::ItemComponent* const childComp)
    {
        activeSubMenu = nullptr;

        if (childComp != nullptr && childComp->itemInfo.hasActiveSubMenu())
        {
            activeSubMenu = Window::create (*(childComp->itemInfo.subMenu),
                                            dismissOnMouseUp,
                                            this,
                                            childComp->getScreenBounds(),
                                            0, maximumNumColumns,
                                            standardItemHeight,
                                            false, 0, managerOfChosenCommand,
                                            componentAttachedTo);

            if (activeSubMenu != nullptr)
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
        isOver = reallyContains (localMousePos, true);

        if (isOver)
            hasBeenOver = true;

        if (lastMouse.getDistanceFrom (globalMousePos) > 2)
        {
            lastMouseMoveTime = Time::getApproximateMillisecondCounter();

            if (disableMouseMoves && isOver)
                disableMouseMoves = false;
        }

        if (disableMouseMoves || (activeSubMenu != nullptr && activeSubMenu->isOverChildren()))
            return;

        bool isMovingTowardsMenu = false;

        if (isOver && (activeSubMenu != nullptr) && globalMousePos != lastMouse)
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
            areaTowardsSubMenu.addTriangle ((float) lastMouse.getX(), (float) lastMouse.getY(),
                                            subX, (float) activeSubMenu->getScreenY(),
                                            subX, (float) (activeSubMenu->getScreenY() + activeSubMenu->getHeight()));

            isMovingTowardsMenu = areaTowardsSubMenu.contains ((float) globalMousePos.getX(), (float) globalMousePos.getY());
        }

        lastMouse = globalMousePos;

        if (! isMovingTowardsMenu)
        {
            Component* c = getComponentAt (localMousePos.getX(), localMousePos.getY());
            if (c == this)
                c = nullptr;

            PopupMenu::ItemComponent* mic = dynamic_cast <PopupMenu::ItemComponent*> (c);

            if (mic == nullptr && c != nullptr)
                mic = c->findParentComponentOfClass ((PopupMenu::ItemComponent*) nullptr);

            if (mic != currentChild
                 && (isOver || (activeSubMenu == nullptr) || ! activeSubMenu->isVisible()))
            {
                if (isOver && (c != nullptr) && (activeSubMenu != nullptr))
                    activeSubMenu->hide (0, true);

                if (! isOver)
                    mic = nullptr;

                setCurrentlyHighlightedChild (mic);
            }
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Window);
};


//==============================================================================
PopupMenu::PopupMenu()
    : lookAndFeel (nullptr),
      separatorPending (false)
{
}

PopupMenu::PopupMenu (const PopupMenu& other)
    : lookAndFeel (other.lookAndFeel),
      separatorPending (false)
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

void PopupMenu::addItem (const int itemResultId, const String& itemText,
                         const bool isActive, const bool isTicked, const Image& iconToUse)
{
    jassert (itemResultId != 0);    // 0 is used as a return value to indicate that the user
                                    // didn't pick anything, so you shouldn't use it as the id
                                    // for an item..

    addSeparatorIfPending();

    items.add (new Item (itemResultId, itemText, isActive, isTicked, iconToUse,
                         Colours::black, false, 0, 0, 0));
}

void PopupMenu::addCommandItem (ApplicationCommandManager* commandManager,
                                const int commandID,
                                const String& displayName)
{
    jassert (commandManager != nullptr && commandID != 0);

    const ApplicationCommandInfo* const registeredInfo = commandManager->getCommandForID (commandID);

    if (registeredInfo != nullptr)
    {
        ApplicationCommandInfo info (*registeredInfo);
        ApplicationCommandTarget* const target = commandManager->getTargetForCommand (commandID, info);

        addSeparatorIfPending();

        items.add (new Item (commandID,
                             displayName.isNotEmpty() ? displayName
                                                      : info.shortName,
                             target != nullptr && (info.flags & ApplicationCommandInfo::isDisabled) == 0,
                             (info.flags & ApplicationCommandInfo::isTicked) != 0,
                             Image::null,
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
                                 const Image& iconToUse)
{
    jassert (itemResultId != 0);    // 0 is used as a return value to indicate that the user
                                    // didn't pick anything, so you shouldn't use it as the id
                                    // for an item..

    addSeparatorIfPending();

    items.add (new Item (itemResultId, itemText, isActive, isTicked, iconToUse,
                         itemTextColour, true, 0, 0, 0));
}

//==============================================================================
void PopupMenu::addCustomItem (const int itemResultId, CustomComponent* const customComponent)
{
    jassert (itemResultId != 0);    // 0 is used as a return value to indicate that the user
                                    // didn't pick anything, so you shouldn't use it as the id
                                    // for an item..

    addSeparatorIfPending();

    items.add (new Item (itemResultId, String::empty, true, false, Image::null,
                         Colours::black, false, customComponent, 0, 0));
}

class NormalComponentWrapper : public PopupMenu::CustomComponent
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
        if (getChildComponent(0) != nullptr)
            getChildComponent(0)->setBounds (getLocalBounds());
    }

private:
    const int width, height;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NormalComponentWrapper);
};

void PopupMenu::addCustomItem (const int itemResultId,
                               Component* customComponent,
                               int idealWidth, int idealHeight,
                               const bool triggerMenuItemAutomaticallyWhenClicked)
{
    addCustomItem (itemResultId,
                   new NormalComponentWrapper (customComponent, idealWidth, idealHeight,
                                               triggerMenuItemAutomaticallyWhenClicked));
}

//==============================================================================
void PopupMenu::addSubMenu (const String& subMenuName,
                            const PopupMenu& subMenu,
                            const bool isActive,
                            const Image& iconToUse,
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
class HeaderItemComponent  : public PopupMenu::CustomComponent
{
public:
    HeaderItemComponent (const String& name)
        : PopupMenu::CustomComponent (false)
    {
        setName (name);
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

    void getIdealSize (int& idealWidth, int& idealHeight)
    {
        getLookAndFeel().getIdealPopupMenuItemSize (getName(), false, -1, idealWidth, idealHeight);
        idealHeight += idealHeight / 2;
        idealWidth += idealWidth / 4;
    }

private:
    JUCE_LEAK_DETECTOR (HeaderItemComponent);
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

const PopupMenu::Options PopupMenu::Options::withTargetComponent (Component* comp) const
{
    Options o (*this);
    o.targetComponent = comp;

    if (comp != nullptr)
        o.targetArea = comp->getScreenBounds();

    return o;
}

const PopupMenu::Options PopupMenu::Options::withTargetScreenArea (const Rectangle<int>& area) const
{
    Options o (*this);
    o.targetArea = area;
    return o;
}

const PopupMenu::Options PopupMenu::Options::withMinimumWidth (int w) const
{
    Options o (*this);
    o.minWidth = w;
    return o;
}

const PopupMenu::Options PopupMenu::Options::withMaximumNumColumns (int cols) const
{
    Options o (*this);
    o.maxColumns = cols;
    return o;
}

const PopupMenu::Options PopupMenu::Options::withStandardItemHeight (int height) const
{
    Options o (*this);
    o.standardHeight = height;
    return o;
}

const PopupMenu::Options PopupMenu::Options::withItemThatMustBeVisible (int idOfItemToBeVisible) const
{
    Options o (*this);
    o.visibleItemID = idOfItemToBeVisible;
    return o;
}

Component* PopupMenu::createWindow (const Options& options,
                                    ApplicationCommandManager** managerOfChosenCommand) const
{
    return Window::create (*this, ModifierKeys::getCurrentModifiers().isAnyMouseButtonDown(),
                           0, options.targetArea, options.minWidth, options.maxColumns > 0 ? options.maxColumns : 7,
                           options.standardHeight, ! options.targetArea.isEmpty(), options.visibleItemID,
                           managerOfChosenCommand, options.targetComponent);
}

//==============================================================================
// This invokes any command manager commands and deletes the menu window when it is dismissed
class PopupMenuCompletionCallback  : public ModalComponentManager::Callback
{
public:
    PopupMenuCompletionCallback()
        : managerOfChosenCommand (nullptr),
          prevFocused (Component::getCurrentlyFocusedComponent()),
          prevTopLevel (prevFocused != nullptr ? prevFocused->getTopLevelComponent() : 0)
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
    JUCE_DECLARE_NON_COPYABLE (PopupMenuCompletionCallback);
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

    window->enterModalState (false, userCallbackDeleter.release());
    ModalComponentManager::getInstance()->attachCallback (window, callback.release());

    window->toFront (false);  // need to do this after making it modal, or it could
                              // be stuck behind other comps that are already modal..

   #if JUCE_MODAL_LOOPS_PERMITTED
    return (userCallback == nullptr && canBeModal) ? window->runModalLoop() : 0;
   #else
    jassert (userCallback != nullptr && canBeModal);
    return 0;
   #endif
}

//==============================================================================
#if JUCE_MODAL_LOOPS_PERMITTED
int PopupMenu::showMenu (const Options& options)
{
    return showWithOptionalCallback (options, 0, true);
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
int PopupMenu::show (const int itemIdThatMustBeVisible,
                     const int minimumWidth, const int maximumNumColumns,
                     const int standardItemHeight,
                     ModalComponentManager::Callback* callback)
{
    return showWithOptionalCallback (Options().withItemThatMustBeVisible (itemIdThatMustBeVisible)
                                              .withMinimumWidth (minimumWidth)
                                              .withMaximumNumColumns (maximumNumColumns)
                                              .withStandardItemHeight (standardItemHeight),
                                     callback, true);
}

int PopupMenu::showAt (const Rectangle<int>& screenAreaToAttachTo,
                       const int itemIdThatMustBeVisible,
                       const int minimumWidth, const int maximumNumColumns,
                       const int standardItemHeight,
                       ModalComponentManager::Callback* callback)
{
    return showWithOptionalCallback (Options().withTargetScreenArea (screenAreaToAttachTo)
                                              .withItemThatMustBeVisible (itemIdThatMustBeVisible)
                                              .withMinimumWidth (minimumWidth)
                                              .withMaximumNumColumns (maximumNumColumns)
                                              .withStandardItemHeight (standardItemHeight),
                                     callback, true);
}

int PopupMenu::showAt (Component* componentToAttachTo,
                       const int itemIdThatMustBeVisible,
                       const int minimumWidth, const int maximumNumColumns,
                       const int standardItemHeight,
                       ModalComponentManager::Callback* callback)
{
    Options options (Options().withItemThatMustBeVisible (itemIdThatMustBeVisible)
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
    const int numWindows = Window::getActiveWindows().size();
    for (int i = numWindows; --i >= 0;)
    {
        Window* const pmw = Window::getActiveWindows()[i];

        if (pmw != nullptr)
            pmw->dismissMenu (0);
    }

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
        const Item* mi = items.getUnchecked (i);

        if ((mi->itemId == commandID && mi->commandManager != nullptr)
            || (mi->subMenu != nullptr && mi->subMenu->containsCommandItem (commandID)))
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
        const Item* const mi = items.getUnchecked (i);

        if (mi->subMenu != nullptr)
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
PopupMenu::CustomComponent::CustomComponent (const bool isTriggeredAutomatically_)
    : isHighlighted (false),
      triggeredAutomatically (isTriggeredAutomatically_)
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
    PopupMenu::ItemComponent* const mic = dynamic_cast <PopupMenu::ItemComponent*> (getParentComponent());

    if (mic != nullptr)
    {
        PopupMenu::Window* const pmw = dynamic_cast <PopupMenu::Window*> (mic->getParentComponent());

        if (pmw != nullptr)
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
PopupMenu::MenuItemIterator::MenuItemIterator (const PopupMenu& menu_)
    : subMenu (nullptr),
      itemId (0),
      isSeparator (false),
      isTicked (false),
      isEnabled (false),
      isCustomComponent (false),
      isSectionHeader (false),
      customColour (0),
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

    itemName = item->customComp != nullptr ? item->customComp->getName() : item->text;
    subMenu = item->subMenu;
    itemId = item->itemId;

    isSeparator = item->isSeparator;
    isTicked = item->isTicked;
    isEnabled = item->active;
    isSectionHeader = dynamic_cast <HeaderItemComponent*> (static_cast <CustomComponent*> (item->customComp)) != nullptr;
    isCustomComponent = (! isSectionHeader) && item->customComp != nullptr;
    customColour = item->usesColour ? &(item->textColour) : 0;
    customImage = item->image;
    commandManager = item->commandManager;

    return true;
}


END_JUCE_NAMESPACE
