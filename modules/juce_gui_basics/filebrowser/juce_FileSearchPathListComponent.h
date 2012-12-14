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

#ifndef __JUCE_FILESEARCHPATHLISTCOMPONENT_JUCEHEADER__
#define __JUCE_FILESEARCHPATHLISTCOMPONENT_JUCEHEADER__

#include "../widgets/juce_ListBox.h"
#include "../buttons/juce_DrawableButton.h"
#include "../buttons/juce_TextButton.h"
#include "../mouse/juce_FileDragAndDropTarget.h"


//==============================================================================
/**
    Shows a set of file paths in a list, allowing them to be added, removed or
    re-ordered.

    @see FileSearchPath
*/
class JUCE_API  FileSearchPathListComponent  : public Component,
                                               public SettableTooltipClient,
                                               public FileDragAndDropTarget,
                                               private ButtonListener,  // (can't use Button::Listener due to idiotic VC2005 bug)
                                               private ListBoxModel
{
public:
    //==============================================================================
    /** Creates an empty FileSearchPathListComponent. */
    FileSearchPathListComponent();

    /** Destructor. */
    ~FileSearchPathListComponent();

    //==============================================================================
    /** Returns the path as it is currently shown. */
    const FileSearchPath& getPath() const noexcept                  { return path; }

    /** Changes the current path. */
    void setPath (const FileSearchPath& newPath);

    /** Sets a file or directory to be the default starting point for the browser to show.

        This is only used if the current file hasn't been set.
    */
    void setDefaultBrowseTarget (const File& newDefaultDirectory);

    /** A set of colour IDs to use to change the colour of various aspects of the label.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        backgroundColourId      = 0x1004100, /**< The background colour to fill the component with.
                                                  Make this transparent if you don't want the background to be filled. */
    };

    //==============================================================================
    /** @internal */
    int getNumRows();
    /** @internal */
    void paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool rowIsSelected);
    /** @internal */
    void deleteKeyPressed (int lastRowSelected);
    /** @internal */
    void returnKeyPressed (int lastRowSelected);
    /** @internal */
    void listBoxItemDoubleClicked (int row, const MouseEvent&);
    /** @internal */
    void selectedRowsChanged (int lastRowSelected);
    /** @internal */
    void resized();
    /** @internal */
    void paint (Graphics& g);
    /** @internal */
    bool isInterestedInFileDrag (const StringArray& files);
    /** @internal */
    void filesDropped (const StringArray& files, int, int);
    /** @internal */
    void buttonClicked (Button* button);


private:
    //==============================================================================
    FileSearchPath path;
    File defaultBrowseTarget;

    ListBox listBox;
    TextButton addButton, removeButton, changeButton;
    DrawableButton upButton, downButton;

    void changed();
    void updateButtons();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileSearchPathListComponent)
};


#endif   // __JUCE_FILESEARCHPATHLISTCOMPONENT_JUCEHEADER__
