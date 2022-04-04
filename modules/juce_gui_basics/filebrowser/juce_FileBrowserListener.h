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

//==============================================================================
/**
    A listener for user selection events in a file browser.

    This is used by a FileBrowserComponent or FileListComponent.

    @tags{GUI}
*/
class JUCE_API  FileBrowserListener
{
public:
    //==============================================================================
    /** Destructor. */
    virtual ~FileBrowserListener();

    //==============================================================================
    /** Callback when the user selects a different file in the browser. */
    virtual void selectionChanged() = 0;

    /** Callback when the user clicks on a file in the browser. */
    virtual void fileClicked (const File& file, const MouseEvent& e) = 0;

    /** Callback when the user double-clicks on a file in the browser. */
    virtual void fileDoubleClicked (const File& file) = 0;

    /** Callback when the browser's root folder changes. */
    virtual void browserRootChanged (const File& newRoot) = 0;
};

} // namespace juce
