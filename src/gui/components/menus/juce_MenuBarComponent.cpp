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

#include "juce_MenuBarComponent.h"
#include "../juce_Desktop.h"
#include "../lookandfeel/juce_LookAndFeel.h"


//==============================================================================
class DummyMenuComponent  : public Component
{
    DummyMenuComponent (const DummyMenuComponent&);
    const DummyMenuComponent& operator= (const DummyMenuComponent&);

public:
    DummyMenuComponent()    {}
    ~DummyMenuComponent()   {}

    void inputAttemptWhenModal()
    {
        exitModalState (0);
    }
};

//==============================================================================
MenuBarComponent::MenuBarComponent (MenuBarModel* model_)
    : model (0),
      itemUnderMouse (-1),
      currentPopupIndex (-1),
      indexToShowAgain (-1),
      lastMouseX (0),
      lastMouseY (0),
      inModalState (false)
{
    setRepaintsOnMouseActivity (true);
    setWantsKeyboardFocus (false);
    setMouseClickGrabsKeyboardFocus (false);

    setModel (model_);
}

MenuBarComponent::~MenuBarComponent()
{
    setModel (0);

    Desktop::getInstance().removeGlobalMouseListener (this);
    currentPopup = 0;
}

void MenuBarComponent::setModel (MenuBarModel* const newModel)
{
    if (model != newModel)
    {
        if (model != 0)
            model->removeListener (this);

        model = newModel;

        if (model != 0)
            model->addListener (this);

        repaint();
        menuBarItemsChanged (0);
    }
}

//==============================================================================
void MenuBarComponent::paint (Graphics& g)
{
    const bool isMouseOverBar = currentPopupIndex >= 0 || itemUnderMouse >= 0 || isMouseOver();

    getLookAndFeel().drawMenuBarBackground (g,
                                            getWidth(),
                                            getHeight(),
                                            isMouseOverBar,
                                            *this);

    if (model != 0)
    {
        for (int i = 0; i < menuNames.size(); ++i)
        {
            g.saveState();
            g.setOrigin (xPositions [i], 0);
            g.reduceClipRegion (0, 0, xPositions[i + 1] - xPositions[i], getHeight());

            getLookAndFeel().drawMenuBarItem (g,
                                              xPositions[i + 1] - xPositions[i],
                                              getHeight(),
                                              i,
                                              menuNames[i],
                                              i == itemUnderMouse,
                                              i == currentPopupIndex,
                                              isMouseOverBar,
                                              *this);

            g.restoreState();
        }
    }
}

void MenuBarComponent::resized()
{
    xPositions.clear();
    int x = 2;
    xPositions.add (x);

    for (int i = 0; i < menuNames.size(); ++i)
    {
        x += getLookAndFeel().getMenuBarItemWidth (*this, i, menuNames[i]);

        xPositions.add (x);
    }
}

int MenuBarComponent::getItemAt (const int x, const int y)
{
    for (int i = 0; i < xPositions.size(); ++i)
        if (x >= xPositions[i] && x < xPositions[i + 1])
            return reallyContains (x, y, true) ? i : -1;

    return -1;
}

void MenuBarComponent::repaintMenuItem (int index)
{
    if (((unsigned int) index) < (unsigned int) xPositions.size())
    {
        const int x1 = xPositions [index];
        const int x2 = xPositions [index + 1];

        repaint (x1 - 2, 0, x2 - x1 + 4, getHeight());
    }
}

void MenuBarComponent::updateItemUnderMouse (int x, int y)
{
    const int newItem = getItemAt (x, y);

    if (itemUnderMouse != newItem)
    {
        repaintMenuItem (itemUnderMouse);
        itemUnderMouse = newItem;
        repaintMenuItem (itemUnderMouse);
    }
}

void MenuBarComponent::hideCurrentMenu()
{
    currentPopup = 0;
    repaint();
}

void MenuBarComponent::showMenu (int index)
{
    if (index != currentPopupIndex)
    {
        if (inModalState)
        {
            hideCurrentMenu();
            indexToShowAgain = index;
            return;
        }

        indexToShowAgain = -1;
        currentPopupIndex = -1;
        itemUnderMouse = index;
        currentPopup = 0;
        menuBarItemsChanged (0);

        Component* const prevFocused = getCurrentlyFocusedComponent();

        ScopedPointer <ComponentDeletionWatcher> prevCompDeletionChecker;
        if (prevFocused != 0)
            prevCompDeletionChecker = new ComponentDeletionWatcher (prevFocused);

        ComponentDeletionWatcher deletionChecker (this);

        enterModalState (false);
        inModalState = true;
        int result = 0;
        ApplicationCommandManager* managerOfChosenCommand = 0;

        Desktop::getInstance().addGlobalMouseListener (this);

        for (;;)
        {
            const int x = getScreenX() + xPositions [itemUnderMouse];
            const int w = xPositions [itemUnderMouse + 1] - xPositions [itemUnderMouse];

            currentPopupIndex = itemUnderMouse;
            indexToShowAgain = -1;
            repaint();

            if (((unsigned int) itemUnderMouse) < (unsigned int) menuNames.size())
            {
                PopupMenu m (model->getMenuForIndex (itemUnderMouse,
                                                     menuNames [itemUnderMouse]));

                if (m.lookAndFeel == 0)
                    m.setLookAndFeel (&getLookAndFeel());

                currentPopup = m.createMenuComponent (x, getScreenY(),
                                                      w, getHeight(),
                                                      0, w, 0, 0,
                                                      true, this,
                                                      &managerOfChosenCommand,
                                                      this);
            }

            if (currentPopup == 0)
            {
                currentPopup = new DummyMenuComponent();
                addAndMakeVisible (currentPopup);
            }

            currentPopup->enterModalState (false);
            currentPopup->toFront (false);  // need to do this after making it modal, or it could
                                            // be stuck behind other comps that are already modal..
            result = currentPopup->runModalLoop();

            if (deletionChecker.hasBeenDeleted())
                return;

            const int lastPopupIndex = currentPopupIndex;
            currentPopup = 0;
            currentPopupIndex = -1;

            if (result != 0)
            {
                topLevelIndexClicked = lastPopupIndex;
                break;
            }
            else if (indexToShowAgain >= 0)
            {
                menuBarItemsChanged (0);
                repaint();
                itemUnderMouse = indexToShowAgain;

                if (((unsigned int) itemUnderMouse) >= (unsigned int) menuNames.size())
                    break;
            }
            else
            {
                break;
            }
        }

        Desktop::getInstance().removeGlobalMouseListener (this);

        inModalState = false;
        exitModalState (0);

        if (prevCompDeletionChecker != 0 && ! prevCompDeletionChecker->hasBeenDeleted())
            prevFocused->grabKeyboardFocus();

        int mx, my;
        getMouseXYRelative (mx, my);
        updateItemUnderMouse (mx, my);
        repaint();

        if (result != 0)
        {
            if (managerOfChosenCommand != 0)
            {
                ApplicationCommandTarget::InvocationInfo info (result);
                info.invocationMethod = ApplicationCommandTarget::InvocationInfo::fromMenu;

                managerOfChosenCommand->invoke (info, true);
            }

            postCommandMessage (result);
        }
    }
}

void MenuBarComponent::handleCommandMessage (int commandId)
{
    if (model != 0)
        model->menuItemSelected (commandId, topLevelIndexClicked);
}

//==============================================================================
void MenuBarComponent::mouseEnter (const MouseEvent& e)
{
    if (e.eventComponent == this)
        updateItemUnderMouse (e.x, e.y);
}

void MenuBarComponent::mouseExit (const MouseEvent& e)
{
    if (e.eventComponent == this)
        updateItemUnderMouse (e.x, e.y);
}

void MenuBarComponent::mouseDown (const MouseEvent& e)
{
    const MouseEvent e2 (e.getEventRelativeTo (this));

    if (currentPopupIndex < 0)
    {
        updateItemUnderMouse (e2.x, e2.y);

        currentPopupIndex = -2;
        showMenu (itemUnderMouse);
    }
}

void MenuBarComponent::mouseDrag (const MouseEvent& e)
{
    const MouseEvent e2 (e.getEventRelativeTo (this));

    const int item = getItemAt (e2.x, e2.y);

    if (item >= 0)
        showMenu (item);
}

void MenuBarComponent::mouseUp (const MouseEvent& e)
{
    const MouseEvent e2 (e.getEventRelativeTo (this));

    updateItemUnderMouse (e2.x, e2.y);

    if (itemUnderMouse < 0 && dynamic_cast <DummyMenuComponent*> ((Component*) currentPopup) != 0)
        hideCurrentMenu();
}

void MenuBarComponent::mouseMove (const MouseEvent& e)
{
    const MouseEvent e2 (e.getEventRelativeTo (this));

    if (lastMouseX != e2.x || lastMouseY != e2.y)
    {
        if (currentPopupIndex >= 0)
        {
            const int item = getItemAt (e2.x, e2.y);

            if (item >= 0)
                showMenu (item);
        }
        else
        {
            updateItemUnderMouse (e2.x, e2.y);
        }

        lastMouseX = e2.x;
        lastMouseY = e2.y;
    }
}

bool MenuBarComponent::keyPressed (const KeyPress& key)
{
    bool used = false;
    const int numMenus = menuNames.size();
    const int currentIndex = jlimit (0, menuNames.size() - 1, currentPopupIndex);

    if (key.isKeyCode (KeyPress::leftKey))
    {
        showMenu ((currentIndex + numMenus - 1) % numMenus);
        used = true;
    }
    else if (key.isKeyCode (KeyPress::rightKey))
    {
        showMenu ((currentIndex + 1) % numMenus);
        used = true;
    }

    return used;
}

void MenuBarComponent::inputAttemptWhenModal()
{
    hideCurrentMenu();
}

void MenuBarComponent::menuBarItemsChanged (MenuBarModel* /*menuBarModel*/)
{
    StringArray newNames;

    if (model != 0)
        newNames = model->getMenuBarNames();

    if (newNames != menuNames)
    {
        menuNames = newNames;
        repaint();
        resized();
    }
}

void MenuBarComponent::menuCommandInvoked (MenuBarModel* /*menuBarModel*/,
                                           const ApplicationCommandTarget::InvocationInfo& info)
{
    if (model == 0
         || (info.commandFlags & ApplicationCommandInfo::dontTriggerVisualFeedback) != 0)
        return;

    for (int i = 0; i < menuNames.size(); ++i)
    {
        const PopupMenu menu (model->getMenuForIndex (i, menuNames [i]));

        if (menu.containsCommandItem (info.commandID))
        {
            itemUnderMouse = i;
            repaintMenuItem (i);
            startTimer (200);

            break;
        }
    }
}

void MenuBarComponent::timerCallback()
{
    stopTimer();

    int mx, my;
    getMouseXYRelative (mx, my);
    updateItemUnderMouse (mx, my);
}


END_JUCE_NAMESPACE
