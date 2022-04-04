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

DirectoryContentsDisplayComponent::DirectoryContentsDisplayComponent (DirectoryContentsList& l)
    : directoryContentsList (l)
{
}

DirectoryContentsDisplayComponent::~DirectoryContentsDisplayComponent()
{
}

//==============================================================================
FileBrowserListener::~FileBrowserListener()
{
}

void DirectoryContentsDisplayComponent::addListener (FileBrowserListener* l)    { listeners.add (l); }
void DirectoryContentsDisplayComponent::removeListener (FileBrowserListener* l) { listeners.remove (l); }

void DirectoryContentsDisplayComponent::sendSelectionChangeMessage()
{
    Component::BailOutChecker checker (dynamic_cast<Component*> (this));
    listeners.callChecked (checker, [] (FileBrowserListener& l) { l.selectionChanged(); });
}

void DirectoryContentsDisplayComponent::sendMouseClickMessage (const File& file, const MouseEvent& e)
{
    if (directoryContentsList.getDirectory().exists())
    {
        Component::BailOutChecker checker (dynamic_cast<Component*> (this));
        listeners.callChecked (checker, [&] (FileBrowserListener& l) { l.fileClicked (file, e); });
    }
}

void DirectoryContentsDisplayComponent::sendDoubleClickMessage (const File& file)
{
    if (directoryContentsList.getDirectory().exists())
    {
        Component::BailOutChecker checker (dynamic_cast<Component*> (this));
        listeners.callChecked (checker, [&] (FileBrowserListener& l) { l.fileDoubleClicked (file); });
    }
}

} // namespace juce
