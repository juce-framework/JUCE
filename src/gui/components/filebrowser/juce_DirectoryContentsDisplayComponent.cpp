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

#include "juce_DirectoryContentsDisplayComponent.h"
#include "../juce_ComponentDeletionWatcher.h"


//==============================================================================
DirectoryContentsDisplayComponent::DirectoryContentsDisplayComponent (DirectoryContentsList& listToShow)
    : fileList (listToShow)
{
}

DirectoryContentsDisplayComponent::~DirectoryContentsDisplayComponent()
{
}

//==============================================================================
FileBrowserListener::~FileBrowserListener()
{
}

void DirectoryContentsDisplayComponent::addListener (FileBrowserListener* const listener) throw()
{
    jassert (listener != 0);

    if (listener != 0)
        listeners.add (listener);
}

void DirectoryContentsDisplayComponent::removeListener (FileBrowserListener* const listener) throw()
{
    listeners.removeValue (listener);
}

void DirectoryContentsDisplayComponent::sendSelectionChangeMessage()
{
    const ComponentDeletionWatcher deletionWatcher (dynamic_cast <Component*> (this));

    for (int i = listeners.size(); --i >= 0;)
    {
        ((FileBrowserListener*) listeners.getUnchecked (i))->selectionChanged();

        if (deletionWatcher.hasBeenDeleted())
            return;

        i = jmin (i, listeners.size() - 1);
    }
}

void DirectoryContentsDisplayComponent::sendMouseClickMessage (const File& file, const MouseEvent& e)
{
    if (fileList.getDirectory().exists())
    {
        const ComponentDeletionWatcher deletionWatcher (dynamic_cast <Component*> (this));

        for (int i = listeners.size(); --i >= 0;)
        {
            ((FileBrowserListener*) listeners.getUnchecked (i))->fileClicked (file, e);

            if (deletionWatcher.hasBeenDeleted())
                return;

            i = jmin (i, listeners.size() - 1);
        }
    }
}

void DirectoryContentsDisplayComponent::sendDoubleClickMessage (const File& file)
{
    if (fileList.getDirectory().exists())
    {
        const ComponentDeletionWatcher deletionWatcher (dynamic_cast <Component*> (this));

        for (int i = listeners.size(); --i >= 0;)
        {
            ((FileBrowserListener*) listeners.getUnchecked (i))->fileDoubleClicked (file);

            if (deletionWatcher.hasBeenDeleted())
                return;

            i = jmin (i, listeners.size() - 1);
        }
    }
}


END_JUCE_NAMESPACE
