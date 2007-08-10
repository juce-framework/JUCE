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

#ifndef __JUCE_FILESEARCHPATHLISTCOMPONENT_JUCEHEADER__
#define __JUCE_FILESEARCHPATHLISTCOMPONENT_JUCEHEADER__

#include "../controls/juce_ListBox.h"
#include "../buttons/juce_Button.h"
#include "../../../../juce_core/io/files/juce_FileSearchPath.h"


//==============================================================================
/**
    Shows a set of file paths in a list, allowing them to be added, removed or
    re-ordered.

    @see FileSearchPath
*/
class JUCE_API  FileSearchPathListComponent  : public Component,
                                               public SettableTooltipClient,
                                               private ButtonListener,
                                               private ListBoxModel
{
public:
    //==============================================================================
    /** Creates an empty FileSearchPathListComponent.

    */
    FileSearchPathListComponent();

    /** Destructor. */
    ~FileSearchPathListComponent();

    //==============================================================================
    /** Returns the path as it is currently shown. */
    const FileSearchPath& getPath() const throw()                   { return path; }

    /** Changes the current path. */
    void setPath (const FileSearchPath& newPath);

    /** Sets a file or directory to be the default starting point for the browser to show.

        This is only used if the current file hasn't been set.
    */
    void setDefaultBrowseTarget (const File& newDefaultDirectory) throw();

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
    bool filesDropped (const StringArray& filenames, int mouseX, int mouseY);
    /** @internal */
    void buttonClicked (Button* button);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    FileSearchPath path;
    File defaultBrowseTarget;

    ListBox* listBox;
    Button* addButton;
    Button* removeButton;
    Button* changeButton;
    Button* upButton;
    Button* downButton;

    void changed() throw();
    void updateButtons() throw();

    FileSearchPathListComponent (const FileSearchPathListComponent&);
    const FileSearchPathListComponent& operator= (const FileSearchPathListComponent&);
};



#endif   // __JUCE_FILESEARCHPATHLISTCOMPONENT_JUCEHEADER__
