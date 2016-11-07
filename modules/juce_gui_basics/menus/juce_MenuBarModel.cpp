/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

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
    listeners.call (&MenuBarModel::Listener::menuBarItemsChanged, this);
}

void MenuBarModel::applicationCommandInvoked (const ApplicationCommandTarget::InvocationInfo& info)
{
    listeners.call (&MenuBarModel::Listener::menuCommandInvoked, this, info);
}

void MenuBarModel::applicationCommandListChanged()
{
    menuItemsChanged();
}

void MenuBarModel::handleMenuBarActivate (bool isActive)
{
    menuBarActivated (isActive);
    listeners.call (&MenuBarModel::Listener::menuBarActivated, this, isActive);
}

void MenuBarModel::menuBarActivated (bool) {}
void MenuBarModel::Listener::menuBarActivated (MenuBarModel*, bool) {}
