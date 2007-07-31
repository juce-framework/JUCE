/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_MenuBarModel.h"


//==============================================================================
MenuBarModel::MenuBarModel() throw()
    : manager (0)
{
}

MenuBarModel::~MenuBarModel()
{
    setApplicationCommandManagerToWatch (0);
}

//==============================================================================
void MenuBarModel::menuItemsChanged()
{
    triggerAsyncUpdate();
}

void MenuBarModel::setApplicationCommandManagerToWatch (ApplicationCommandManager* const newManager) throw()
{
    if (manager != newManager)
    {
        if (manager != 0)
            manager->removeListener (this);

        manager = newManager;

        if (manager != 0)
            manager->addListener (this);
    }
}

void MenuBarModel::addListener (MenuBarModelListener* const newListener) throw()
{
    jassert (newListener != 0);
    jassert (! listeners.contains (newListener)); // trying to add a listener to the list twice!

    if (newListener != 0)
        listeners.add (newListener);
}

void MenuBarModel::removeListener (MenuBarModelListener* const listenerToRemove) throw()
{
    // Trying to remove a listener that isn't on the list!
    // If this assertion happens because this object is a dangling pointer, make sure you've not
    // deleted this menu model while it's still being used by something (e.g. by a MenuBarComponent)
    jassert (listeners.contains (listenerToRemove));

    listeners.removeValue (listenerToRemove);
}

void MenuBarModel::handleAsyncUpdate()
{
    for (int i = listeners.size(); --i >= 0;)
    {
        ((MenuBarModelListener*) listeners.getUnchecked (i))->menuBarItemsChanged (this);
        i = jmin (i, listeners.size());
    }
}

//==============================================================================
void MenuBarModel::applicationCommandInvoked (const ApplicationCommandTarget::InvocationInfo& info)
{
    for (int i = listeners.size(); --i >= 0;)
    {
        ((MenuBarModelListener*) listeners.getUnchecked (i))->menuCommandInvoked (this, info);
        i = jmin (i, listeners.size());
    }
}

void MenuBarModel::applicationCommandListChanged()
{
    menuItemsChanged();
}


END_JUCE_NAMESPACE
