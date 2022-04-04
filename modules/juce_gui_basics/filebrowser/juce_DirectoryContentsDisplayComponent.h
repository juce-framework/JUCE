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
    A base class for components that display a list of the files in a directory.

    @see DirectoryContentsList

    @tags{GUI}
*/
class JUCE_API  DirectoryContentsDisplayComponent
{
public:
    //==============================================================================
    /** Creates a DirectoryContentsDisplayComponent for a given list of files. */
    DirectoryContentsDisplayComponent (DirectoryContentsList& listToShow);

    /** Destructor. */
    virtual ~DirectoryContentsDisplayComponent();

    //==============================================================================
    /** The list that this component is displaying */
    DirectoryContentsList& directoryContentsList;

    //==============================================================================
    /** Returns the number of files the user has got selected.
        @see getSelectedFile
    */
    virtual int getNumSelectedFiles() const = 0;

    /** Returns one of the files that the user has currently selected.
        The index should be in the range 0 to (getNumSelectedFiles() - 1).
        @see getNumSelectedFiles
    */
    virtual File getSelectedFile (int index) const = 0;

    /** Deselects any selected files. */
    virtual void deselectAllFiles() = 0;

    /** Scrolls this view to the top. */
    virtual void scrollToTop() = 0;

    /** If the specified file is in the list, it will become the only selected item
        (and if the file isn't in the list, all other items will be deselected). */
    virtual void setSelectedFile (const File&) = 0;

    //==============================================================================
    /** Adds a listener to be told when files are selected or clicked.
        @see removeListener
    */
    void addListener (FileBrowserListener* listener);

    /** Removes a listener.
        @see addListener
    */
    void removeListener (FileBrowserListener* listener);


    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the list.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        highlightColourId          = 0x1000540, /**< The colour to use to fill a highlighted row of the list. */
        textColourId               = 0x1000541, /**< The colour for the text. */
        highlightedTextColourId    = 0x1000542  /**< The colour with which to draw the text in highlighted sections. */
    };

    //==============================================================================
    /** @internal */
    void sendSelectionChangeMessage();
    /** @internal */
    void sendDoubleClickMessage (const File&);
    /** @internal */
    void sendMouseClickMessage (const File&, const MouseEvent&);

protected:
    //==============================================================================
    ListenerList<FileBrowserListener> listeners;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DirectoryContentsDisplayComponent)
};

} // namespace juce
