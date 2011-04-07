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

#include "juce_MenuBarComponent.h"
#include "../juce_Desktop.h"
#include "../lookandfeel/juce_LookAndFeel.h"


//==============================================================================
MenuBarComponent::MenuBarComponent (MenuBarModel* model_)
    : model (nullptr),
      itemUnderMouse (-1),
      currentPopupIndex (-1),
      topLevelIndexClicked (0),
      lastMouseX (0),
      lastMouseY (0)
{
    setRepaintsOnMouseActivity (true);
    setWantsKeyboardFocus (false);
    setMouseClickGrabsKeyboardFocus (false);

    setModel (model_);
}

MenuBarComponent::~MenuBarComponent()
{
    setModel (nullptr);
    Desktop::getInstance().removeGlobalMouseListener (this);
}

MenuBarModel* MenuBarComponent::getModel() const noexcept
{
    return model;
}

void MenuBarComponent::setModel (MenuBarModel* const newModel)
{
    if (model != newModel)
    {
        if (model != nullptr)
            model->removeListener (this);

        model = newModel;

        if (model != nullptr)
            model->addListener (this);

        repaint();
        menuBarItemsChanged (nullptr);
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

    if (model != nullptr)
    {
        for (int i = 0; i < menuNames.size(); ++i)
        {
            Graphics::ScopedSaveState ss (g);

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
        }
    }
}

void MenuBarComponent::resized()
{
    xPositions.clear();
    int x = 0;
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
            return reallyContains (Point<int> (x, y), true) ? i : -1;

    return -1;
}

void MenuBarComponent::repaintMenuItem (int index)
{
    if (isPositiveAndBelow (index, xPositions.size()))
    {
        const int x1 = xPositions [index];
        const int x2 = xPositions [index + 1];

        repaint (x1 - 2, 0, x2 - x1 + 4, getHeight());
    }
}

void MenuBarComponent::setItemUnderMouse (const int index)
{
    if (itemUnderMouse != index)
    {
        repaintMenuItem (itemUnderMouse);
        itemUnderMouse = index;
        repaintMenuItem (itemUnderMouse);
    }
}

void MenuBarComponent::setOpenItem (int index)
{
    if (currentPopupIndex != index)
    {
        repaintMenuItem (currentPopupIndex);
        currentPopupIndex = index;
        repaintMenuItem (currentPopupIndex);

        if (index >= 0)
            Desktop::getInstance().addGlobalMouseListener (this);
        else
            Desktop::getInstance().removeGlobalMouseListener (this);
    }
}

void MenuBarComponent::updateItemUnderMouse (int x, int y)
{
    setItemUnderMouse (getItemAt (x, y));
}

void MenuBarComponent::showMenu (int index)
{
    if (index != currentPopupIndex)
    {
        PopupMenu::dismissAllActiveMenus();
        menuBarItemsChanged (nullptr);

        setOpenItem (index);
        setItemUnderMouse (index);

        if (index >= 0)
        {
            PopupMenu m (model->getMenuForIndex (itemUnderMouse,
                                                 menuNames [itemUnderMouse]));

            if (m.lookAndFeel == nullptr)
                m.setLookAndFeel (&getLookAndFeel());

            const Rectangle<int> itemPos (xPositions [index], 0, xPositions [index + 1] - xPositions [index], getHeight());

            m.showMenuAsync (PopupMenu::Options().withTargetComponent (this)
                                                 .withTargetScreenArea (localAreaToGlobal (itemPos))
                                                 .withMinimumWidth (itemPos.getWidth()),
                             ModalCallbackFunction::forComponent (menuBarMenuDismissedCallback, this, index));
        }
    }
}

void MenuBarComponent::menuBarMenuDismissedCallback (int result, MenuBarComponent* bar, int topLevelIndex)
{
    if (bar != nullptr)
        bar->menuDismissed (topLevelIndex, result);
}

void MenuBarComponent::menuDismissed (int topLevelIndex, int itemId)
{
    topLevelIndexClicked = topLevelIndex;
    postCommandMessage (itemId);
}

void MenuBarComponent::handleCommandMessage (int commandId)
{
    const Point<int> mousePos (getMouseXYRelative());
    updateItemUnderMouse (mousePos.getX(), mousePos.getY());

    if (currentPopupIndex == topLevelIndexClicked)
        setOpenItem (-1);

    if (commandId != 0 && model != nullptr)
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
    if (currentPopupIndex < 0)
    {
        const MouseEvent e2 (e.getEventRelativeTo (this));
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

    if (itemUnderMouse < 0 && getLocalBounds().contains (e2.x, e2.y))
    {
        setOpenItem (-1);
        PopupMenu::dismissAllActiveMenus();
    }
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

void MenuBarComponent::menuBarItemsChanged (MenuBarModel* /*menuBarModel*/)
{
    StringArray newNames;

    if (model != nullptr)
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
    if (model == nullptr || (info.commandFlags & ApplicationCommandInfo::dontTriggerVisualFeedback) != 0)
        return;

    for (int i = 0; i < menuNames.size(); ++i)
    {
        const PopupMenu menu (model->getMenuForIndex (i, menuNames [i]));

        if (menu.containsCommandItem (info.commandID))
        {
            setItemUnderMouse (i);
            startTimer (200);
            break;
        }
    }
}

void MenuBarComponent::timerCallback()
{
    stopTimer();

    const Point<int> mousePos (getMouseXYRelative());
    updateItemUnderMouse (mousePos.getX(), mousePos.getY());
}


END_JUCE_NAMESPACE
