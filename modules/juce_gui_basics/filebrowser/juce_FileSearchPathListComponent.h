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
    Shows a set of file paths in a list, allowing them to be added, removed or
    re-ordered.

    @see FileSearchPath

    @tags{GUI}
*/
class JUCE_API  FileSearchPathListComponent  : public Component,
                                               public SettableTooltipClient,
                                               public FileDragAndDropTarget,
                                               private ListBoxModel
{
public:
    //==============================================================================
    /** Creates an empty FileSearchPathListComponent. */
    FileSearchPathListComponent();

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
    int getNumRows() override;
    /** @internal */
    void paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override;
    /** @internal */
    void deleteKeyPressed (int lastRowSelected) override;
    /** @internal */
    void returnKeyPressed (int lastRowSelected) override;
    /** @internal */
    void listBoxItemDoubleClicked (int row, const MouseEvent&) override;
    /** @internal */
    void selectedRowsChanged (int lastRowSelected) override;
    /** @internal */
    void resized() override;
    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    bool isInterestedInFileDrag (const StringArray&) override;
    /** @internal */
    void filesDropped (const StringArray& files, int, int) override;

private:
    //==============================================================================
    FileSearchPath path;
    File defaultBrowseTarget;
    std::unique_ptr<FileChooser> chooser;

    ListBox listBox;
    TextButton addButton, removeButton, changeButton;
    DrawableButton upButton, downButton;

    void changed();
    void updateButtons();

    void addPath();
    void deleteSelected();
    void editSelected();
    void moveSelection (int);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileSearchPathListComponent)
};

} // namespace juce
