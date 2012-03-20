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

void DirectoryContentsDisplayComponent::addListener (FileBrowserListener* const listener)
{
    listeners.add (listener);
}

void DirectoryContentsDisplayComponent::removeListener (FileBrowserListener* const listener)
{
    listeners.remove (listener);
}

void DirectoryContentsDisplayComponent::sendSelectionChangeMessage()
{
    Component::BailOutChecker checker (dynamic_cast <Component*> (this));
    listeners.callChecked (checker, &FileBrowserListener::selectionChanged);
}

void DirectoryContentsDisplayComponent::sendMouseClickMessage (const File& file, const MouseEvent& e)
{
    if (fileList.getDirectory().exists())
    {
        Component::BailOutChecker checker (dynamic_cast <Component*> (this));
        listeners.callChecked (checker, &FileBrowserListener::fileClicked, file, e);
    }
}

void DirectoryContentsDisplayComponent::sendDoubleClickMessage (const File& file)
{
    if (fileList.getDirectory().exists())
    {
        Component::BailOutChecker checker (dynamic_cast <Component*> (this));
        listeners.callChecked (checker, &FileBrowserListener::fileDoubleClicked, file);
    }
}
