/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

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
