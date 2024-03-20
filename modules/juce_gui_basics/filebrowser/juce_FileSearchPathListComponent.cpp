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

FileSearchPathListComponent::FileSearchPathListComponent()
    : addButton ("+"),
      removeButton ("-"),
      changeButton (TRANS ("change...")),
      upButton ({}, DrawableButton::ImageOnButtonBackground),
      downButton ({}, DrawableButton::ImageOnButtonBackground)
{
    listBox.setModel (this);
    addAndMakeVisible (listBox);
    listBox.setColour (ListBox::backgroundColourId, Colours::black.withAlpha (0.02f));
    listBox.setColour (ListBox::outlineColourId, Colours::black.withAlpha (0.1f));
    listBox.setOutlineThickness (1);

    addAndMakeVisible (addButton);
    addButton.onClick = [this] { addPath(); };
    addButton.setConnectedEdges (Button::ConnectedOnLeft | Button::ConnectedOnRight | Button::ConnectedOnBottom | Button::ConnectedOnTop);

    addAndMakeVisible (removeButton);
    removeButton.onClick = [this] { deleteSelected(); };
    removeButton.setConnectedEdges (Button::ConnectedOnLeft | Button::ConnectedOnRight | Button::ConnectedOnBottom | Button::ConnectedOnTop);

    addAndMakeVisible (changeButton);
    changeButton.onClick = [this] { editSelected(); };

    addAndMakeVisible (upButton);
    upButton.onClick = [this] { moveSelection (-1); };

    auto arrowColour = findColour (ListBox::textColourId);

    {
        Path arrowPath;
        arrowPath.addArrow ({ 50.0f, 100.0f, 50.0f, 0.0f }, 40.0f, 100.0f, 50.0f);
        DrawablePath arrowImage;
        arrowImage.setFill (arrowColour);
        arrowImage.setPath (arrowPath);

        upButton.setImages (&arrowImage);
    }

    addAndMakeVisible (downButton);
    downButton.onClick = [this] { moveSelection (1); };

    {
        Path arrowPath;
        arrowPath.addArrow ({ 50.0f, 0.0f, 50.0f, 100.0f }, 40.0f, 100.0f, 50.0f);
        DrawablePath arrowImage;
        arrowImage.setFill (arrowColour);
        arrowImage.setPath (arrowPath);

        downButton.setImages (&arrowImage);
    }

    updateButtons();
}

void FileSearchPathListComponent::updateButtons()
{
    const bool anythingSelected = listBox.getNumSelectedRows() > 0;

    removeButton.setEnabled (anythingSelected);
    changeButton.setEnabled (anythingSelected);
    upButton.setEnabled (anythingSelected);
    downButton.setEnabled (anythingSelected);
}

void FileSearchPathListComponent::changed()
{
    listBox.updateContent();
    listBox.repaint();
    updateButtons();
}

//==============================================================================
void FileSearchPathListComponent::setPath (const FileSearchPath& newPath)
{
    if (newPath.toString() != path.toString())
    {
        path = newPath;
        changed();
    }
}

void FileSearchPathListComponent::setDefaultBrowseTarget (const File& newDefaultDirectory)
{
    defaultBrowseTarget = newDefaultDirectory;
}

//==============================================================================
int FileSearchPathListComponent::getNumRows()
{
    return path.getNumPaths();
}

void FileSearchPathListComponent::paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll (findColour (TextEditor::highlightColourId));

    g.setColour (findColour (ListBox::textColourId));
    Font f (withDefaultMetrics (FontOptions { (float) height * 0.7f }));
    f.setHorizontalScale (0.9f);
    g.setFont (f);

    g.drawText (path.getRawString (rowNumber),
                4, 0, width - 6, height,
                Justification::centredLeft, true);
}

void FileSearchPathListComponent::deleteKeyPressed (int row)
{
    if (isPositiveAndBelow (row, path.getNumPaths()))
    {
        path.remove (row);
        changed();
    }
}

void FileSearchPathListComponent::returnKeyPressed (int row)
{
    chooser = std::make_unique<FileChooser> (TRANS ("Change folder..."), path.getRawString (row), "*");
    auto chooserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories;

    chooser->launchAsync (chooserFlags, [this, row] (const FileChooser& fc)
    {
        if (fc.getResult() == File{})
            return;

        path.remove (row);
        path.add (fc.getResult(), row);
        changed();
    });
}

void FileSearchPathListComponent::listBoxItemDoubleClicked (int row, const MouseEvent&)
{
    returnKeyPressed (row);
}

void FileSearchPathListComponent::selectedRowsChanged (int)
{
    updateButtons();
}

void FileSearchPathListComponent::paint (Graphics& g)
{
    g.fillAll (findColour (backgroundColourId));
}

void FileSearchPathListComponent::resized()
{
    const int buttonH = 22;
    const int buttonY = getHeight() - buttonH - 4;
    listBox.setBounds (2, 2, getWidth() - 4, buttonY - 5);

    addButton.setBounds (2, buttonY, buttonH, buttonH);
    removeButton.setBounds (addButton.getRight(), buttonY, buttonH, buttonH);

    changeButton.changeWidthToFitText (buttonH);
    downButton.setSize (buttonH * 2, buttonH);
    upButton.setSize (buttonH * 2, buttonH);

    downButton.setTopRightPosition (getWidth() - 2, buttonY);
    upButton.setTopRightPosition (downButton.getX() - 4, buttonY);
    changeButton.setTopRightPosition (upButton.getX() - 8, buttonY);
}

bool FileSearchPathListComponent::isInterestedInFileDrag (const StringArray&)
{
    return true;
}

void FileSearchPathListComponent::filesDropped (const StringArray& filenames, int, int mouseY)
{
    for (int i = filenames.size(); --i >= 0;)
    {
        const File f (filenames[i]);

        if (f.isDirectory())
        {
            auto row = listBox.getRowContainingPosition (0, mouseY - listBox.getY());
            path.add (f, row);
            changed();
        }
    }
}

void FileSearchPathListComponent::addPath()
{
    auto start = defaultBrowseTarget;

    if (start == File())
        start = path[0];

    if (start == File())
        start = File::getCurrentWorkingDirectory();

    chooser = std::make_unique<FileChooser> (TRANS ("Add a folder..."), start, "*");
    auto chooserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories;

    chooser->launchAsync (chooserFlags, [this] (const FileChooser& fc)
    {
        if (fc.getResult() == File{})
            return;

        path.add (fc.getResult(), listBox.getSelectedRow());
        changed();
    });
}

void FileSearchPathListComponent::deleteSelected()
{
    deleteKeyPressed (listBox.getSelectedRow());
    changed();
}

void FileSearchPathListComponent::editSelected()
{
    returnKeyPressed (listBox.getSelectedRow());
    changed();
}

void FileSearchPathListComponent::moveSelection (int delta)
{
    jassert (delta == -1 || delta == 1);
    auto currentRow = listBox.getSelectedRow();

    if (isPositiveAndBelow (currentRow, path.getNumPaths()))
    {
        auto newRow = jlimit (0, path.getNumPaths() - 1, currentRow + delta);

        if (currentRow != newRow)
        {
            const auto f = File::createFileWithoutCheckingPath (path.getRawString (currentRow));
            path.remove (currentRow);
            path.add (f, newRow);
            listBox.selectRow (newRow);
            changed();
        }
    }
}


} // namespace juce
