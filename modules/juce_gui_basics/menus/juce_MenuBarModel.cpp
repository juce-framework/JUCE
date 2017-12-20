/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

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

void MenuBarModel::setApplicationCommandManagerToWatch (ApplicationCommandManager* const newManager) noexcept
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

void MenuBarModel::addListener (Listener* const newListener) noexcept
{
    listeners.add (newListener);
}

void MenuBarModel::removeListener (Listener* const listenerToRemove) noexcept
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
