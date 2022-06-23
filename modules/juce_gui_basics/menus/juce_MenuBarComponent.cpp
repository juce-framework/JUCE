/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

class MenuBarComponent::AccessibleItemComponent  : public Component
{
public:
    AccessibleItemComponent (MenuBarComponent& comp, const String& menuItemName)
        : owner (comp),
          name (menuItemName)
    {
        setInterceptsMouseClicks (false, false);
    }

    const String& getName() const noexcept    { return name; }

private:
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        class ComponentHandler  : public AccessibilityHandler
        {
        public:
            explicit ComponentHandler (AccessibleItemComponent& item)
                : AccessibilityHandler (item,
                                        AccessibilityRole::menuItem,
                                        getAccessibilityActions (item)),
                  itemComponent (item)
            {
            }

            String getTitle() const override  { return itemComponent.name; }

        private:
            static AccessibilityActions getAccessibilityActions (AccessibleItemComponent& item)
            {
                auto showMenu = [&item] { item.owner.showMenu (item.owner.indexOfItemComponent (&item)); };

                return AccessibilityActions().addAction (AccessibilityActionType::focus,
                                                         [&item] { item.owner.setItemUnderMouse (item.owner.indexOfItemComponent (&item)); })
                                             .addAction (AccessibilityActionType::press,    showMenu)
                                             .addAction (AccessibilityActionType::showMenu, showMenu);
            }

            AccessibleItemComponent& itemComponent;
        };

        return std::make_unique<ComponentHandler> (*this);
    }

    MenuBarComponent& owner;
    const String name;
};

MenuBarComponent::MenuBarComponent (MenuBarModel* m)
{
    setRepaintsOnMouseActivity (true);
    setWantsKeyboardFocus (false);
    setMouseClickGrabsKeyboardFocus (false);

    setModel (m);
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
    const auto isMouseOverBar = (currentPopupIndex >= 0 || itemUnderMouse >= 0 || isMouseOver());

    getLookAndFeel().drawMenuBarBackground (g, getWidth(), getHeight(), isMouseOverBar, *this);

    if (model == nullptr)
        return;

    for (size_t i = 0; i < itemComponents.size(); ++i)
    {
        const auto& itemComponent = itemComponents[i];
        const auto itemBounds = itemComponent->getBounds();

        Graphics::ScopedSaveState ss (g);

        g.setOrigin (itemBounds.getX(), 0);
        g.reduceClipRegion (0, 0, itemBounds.getWidth(), itemBounds.getHeight());

        getLookAndFeel().drawMenuBarItem (g,
                                          itemBounds.getWidth(),
                                          itemBounds.getHeight(),
                                          (int) i,
                                          itemComponent->getName(),
                                          (int) i == itemUnderMouse,
                                          (int) i == currentPopupIndex,
                                          isMouseOverBar,
                                          *this);
    }
}

void MenuBarComponent::resized()
{
    int x = 0;

    for (size_t i = 0; i < itemComponents.size(); ++i)
    {
        auto& itemComponent = itemComponents[i];

        auto w = getLookAndFeel().getMenuBarItemWidth (*this, (int) i, itemComponent->getName());
        itemComponent->setBounds (x, 0, w, getHeight());
        x += w;
    }
}

int MenuBarComponent::getItemAt (Point<int> p)
{
    for (size_t i = 0; i < itemComponents.size(); ++i)
        if (itemComponents[i]->getBounds().contains (p) && reallyContains (p, true))
            return (int) i;

    return -1;
}

void MenuBarComponent::repaintMenuItem (int index)
{
    if (isPositiveAndBelow (index, (int) itemComponents.size()))
    {
        auto itemBounds = itemComponents[(size_t) index]->getBounds();

        repaint (itemBounds.getX() - 2,
                 0,
                 itemBounds.getWidth() + 4,
                 itemBounds.getHeight());
    }
}

void MenuBarComponent::setItemUnderMouse (int index)
{
    if (itemUnderMouse == index)
        return;

    repaintMenuItem (itemUnderMouse);
    itemUnderMouse = index;
    repaintMenuItem (itemUnderMouse);

    if (isPositiveAndBelow (itemUnderMouse, (int) itemComponents.size()))
        if (auto* handler = itemComponents[(size_t) itemUnderMouse]->getAccessibilityHandler())
            handler->grabFocus();
}

void MenuBarComponent::setOpenItem (int index)
{
    if (currentPopupIndex != index)
    {
        if (currentPopupIndex < 0 && index >= 0)
            model->handleMenuBarActivate (true);
        else if (currentPopupIndex >= 0 && index < 0)
            model->handleMenuBarActivate (false);

        repaintMenuItem (currentPopupIndex);
        currentPopupIndex = index;
        repaintMenuItem (currentPopupIndex);

        auto& desktop = Desktop::getInstance();

        if (index >= 0)
            desktop.addGlobalMouseListener (this);
        else
            desktop.removeGlobalMouseListener (this);
    }
}

void MenuBarComponent::updateItemUnderMouse (Point<int> p)
{
    setItemUnderMouse (getItemAt (p));
}

void MenuBarComponent::showMenu (int index)
{
    if (index != currentPopupIndex)
    {
        PopupMenu::dismissAllActiveMenus();
        menuBarItemsChanged (nullptr);

        setOpenItem (index);
        setItemUnderMouse (index);

        if (isPositiveAndBelow (index, (int) itemComponents.size()))
        {
            const auto& itemComponent = itemComponents[(size_t) index];
            auto m = model->getMenuForIndex (itemUnderMouse, itemComponent->getName());

            if (m.lookAndFeel == nullptr)
                m.setLookAndFeel (&getLookAndFeel());

            auto itemBounds = itemComponent->getBounds();

            const auto callback = [ref = SafePointer<MenuBarComponent> (this), index] (int result)
            {
                if (ref != nullptr)
                    ref->menuDismissed (index, result);
            };

            m.showMenuAsync (PopupMenu::Options().withTargetComponent (this)
                                                 .withTargetScreenArea (localAreaToGlobal (itemBounds))
                                                 .withMinimumWidth (itemBounds.getWidth()),
                             callback);
        }
    }
}

void MenuBarComponent::menuDismissed (int topLevelIndex, int itemId)
{
    topLevelIndexClicked = topLevelIndex;
    postCommandMessage (itemId);
}

void MenuBarComponent::handleCommandMessage (int commandId)
{
    updateItemUnderMouse (getMouseXYRelative());

    if (currentPopupIndex == topLevelIndexClicked)
        setOpenItem (-1);

    if (commandId != 0 && model != nullptr)
        model->menuItemSelected (commandId, topLevelIndexClicked);
}

//==============================================================================
void MenuBarComponent::mouseEnter (const MouseEvent& e)
{
    if (e.eventComponent == this)
        updateItemUnderMouse (e.getPosition());
}

void MenuBarComponent::mouseExit (const MouseEvent& e)
{
    if (e.eventComponent == this)
        updateItemUnderMouse (e.getPosition());
}

void MenuBarComponent::mouseDown (const MouseEvent& e)
{
    if (currentPopupIndex < 0)
    {
        updateItemUnderMouse (e.getEventRelativeTo (this).getPosition());

        currentPopupIndex = -2;
        showMenu (itemUnderMouse);
    }
}

void MenuBarComponent::mouseDrag (const MouseEvent& e)
{
    const auto item = getItemAt (e.getEventRelativeTo (this).getPosition());

    if (item >= 0)
        showMenu (item);
}

void MenuBarComponent::mouseUp (const MouseEvent& e)
{
    const auto e2 = e.getEventRelativeTo (this);

    updateItemUnderMouse (e2.getPosition());

    if (itemUnderMouse < 0 && getLocalBounds().contains (e2.x, e2.y))
    {
        setOpenItem (-1);
        PopupMenu::dismissAllActiveMenus();
    }
}

void MenuBarComponent::mouseMove (const MouseEvent& e)
{
    const auto e2 = e.getEventRelativeTo (this);

    if (lastMousePos != e2.getPosition())
    {
        if (currentPopupIndex >= 0)
        {
            const auto item = getItemAt (e2.getPosition());

            if (item >= 0)
                showMenu (item);
        }
        else
        {
            updateItemUnderMouse (e2.getPosition());
        }

        lastMousePos = e2.getPosition();
    }
}

bool MenuBarComponent::keyPressed (const KeyPress& key)
{
    const auto numMenus = (int) itemComponents.size();

    if (numMenus > 0)
    {
        const auto currentIndex = jlimit (0, numMenus - 1, currentPopupIndex);

        if (key.isKeyCode (KeyPress::leftKey))
        {
            showMenu ((currentIndex + numMenus - 1) % numMenus);
            return true;
        }

        if (key.isKeyCode (KeyPress::rightKey))
        {
            showMenu ((currentIndex + 1) % numMenus);
            return true;
        }
    }

    return false;
}

void MenuBarComponent::menuBarItemsChanged (MenuBarModel*)
{
    StringArray newNames;

    if (model != nullptr)
        newNames = model->getMenuBarNames();

    auto itemsHaveChanged = [this, &newNames]
    {
        if ((int) itemComponents.size() != newNames.size())
            return true;

        for (size_t i = 0; i < itemComponents.size(); ++i)
            if (itemComponents[i]->getName() != newNames[(int) i])
                return true;

        return false;
    }();

    if (itemsHaveChanged)
    {
        updateItemComponents (newNames);

        repaint();
        resized();
    }
}

void MenuBarComponent::updateItemComponents (const StringArray& menuNames)
{
    itemComponents.clear();

    for (const auto& name : menuNames)
    {
        itemComponents.push_back (std::make_unique<AccessibleItemComponent> (*this, name));
        addAndMakeVisible (*itemComponents.back());
    }
}

int MenuBarComponent::indexOfItemComponent (AccessibleItemComponent* itemComponent) const
{
    const auto iter = std::find_if (itemComponents.cbegin(), itemComponents.cend(),
                                    [itemComponent] (const std::unique_ptr<AccessibleItemComponent>& c) { return c.get() == itemComponent; });

    if (iter != itemComponents.cend())
        return (int) std::distance (itemComponents.cbegin(), iter);

    jassertfalse;
    return -1;
}

void MenuBarComponent::menuCommandInvoked (MenuBarModel*, const ApplicationCommandTarget::InvocationInfo& info)
{
    if (model == nullptr || (info.commandFlags & ApplicationCommandInfo::dontTriggerVisualFeedback) != 0)
        return;

    for (size_t i = 0; i < itemComponents.size(); ++i)
    {
        const auto menu = model->getMenuForIndex ((int) i, itemComponents[i]->getName());

        if (menu.containsCommandItem (info.commandID))
        {
            setItemUnderMouse ((int) i);
            startTimer (200);
            break;
        }
    }
}

void MenuBarComponent::timerCallback()
{
    stopTimer();
    updateItemUnderMouse (getMouseXYRelative());
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> MenuBarComponent::createAccessibilityHandler()
{
    struct MenuBarComponentAccessibilityHandler  : public AccessibilityHandler
    {
        explicit MenuBarComponentAccessibilityHandler (MenuBarComponent& menuBarComponent)
            : AccessibilityHandler (menuBarComponent, AccessibilityRole::menuBar)
        {
        }

        AccessibleState getCurrentState() const override  { return AccessibleState().withIgnored(); }
    };

    return std::make_unique<MenuBarComponentAccessibilityHandler> (*this);
}

} // namespace juce
