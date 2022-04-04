/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

MenuBarModel::MenuBarModel() noexcept
    : manager (nullptr)
{
}

MenuBarModel::~MenuBarModel()
{
    setApplicationCommandManagerToWatch (nullptr);
}

//==============================================================================
void MenuBarModel::menuItemsChanged()
{
    triggerAsyncUpdate();
}

void MenuBarModel::setApplicationCommandManagerToWatch (ApplicationCommandManager* newManager)
{
    if (manager != newManager)
    {
        if (manager != nullptr)
            manager->removeListener (this);

        manager = newManager;

        if (manager != nullptr)
            manager->addListener (this);
    }
}

void MenuBarModel::addListener (Listener* newListener)
{
    listeners.add (newListener);
}

void MenuBarModel::removeListener (Listener* listenerToRemove)
{
    // Trying to remove a listener that isn't on the list!
    // If this assertion happens because this object is a dangling pointer, make sure you've not
    // deleted this menu model while it's still being used by something (e.g. by a MenuBarComponent)
    jassert (listeners.contains (listenerToRemove));

    listeners.remove (listenerToRemove);
}

//==============================================================================
void MenuBarModel::handleAsyncUpdate()
{
    listeners.call ([this] (Listener& l) { l.menuBarItemsChanged (this); });
}

void MenuBarModel::applicationCommandInvoked (const ApplicationCommandTarget::InvocationInfo& info)
{
    listeners.call ([this, &info] (Listener& l) { l.menuCommandInvoked (this, info); });
}

void MenuBarModel::applicationCommandListChanged()
{
    menuItemsChanged();
}

void MenuBarModel::handleMenuBarActivate (bool isActive)
{
    menuBarActivated (isActive);
    listeners.call ([this, isActive] (Listener& l) { l.menuBarActivated (this, isActive); });
}

void MenuBarModel::menuBarActivated (bool) {}
void MenuBarModel::Listener::menuBarActivated (MenuBarModel*, bool) {}

} // namespace juce
