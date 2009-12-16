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

#include "juce_FileSearchPathListComponent.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../filebrowser/juce_FileChooser.h"
#include "../../../text/juce_LocalisedStrings.h"
#include "../buttons/juce_TextButton.h"
#include "../buttons/juce_DrawableButton.h"
#include "../../graphics/drawables/juce_DrawablePath.h"


//==============================================================================
FileSearchPathListComponent::FileSearchPathListComponent()
{
    addAndMakeVisible (listBox = new ListBox (String::empty, this));
    listBox->setColour (ListBox::backgroundColourId, Colours::black.withAlpha (0.02f));
    listBox->setColour (ListBox::outlineColourId, Colours::black.withAlpha (0.1f));
    listBox->setOutlineThickness (1);

    addAndMakeVisible (addButton = new TextButton ("+"));
    addButton->addButtonListener (this);
    addButton->setConnectedEdges (Button::ConnectedOnLeft | Button::ConnectedOnRight | Button::ConnectedOnBottom | Button::ConnectedOnTop);

    addAndMakeVisible (removeButton = new TextButton ("-"));
    removeButton->addButtonListener (this);
    removeButton->setConnectedEdges (Button::ConnectedOnLeft | Button::ConnectedOnRight | Button::ConnectedOnBottom | Button::ConnectedOnTop);

    addAndMakeVisible (changeButton = new TextButton (TRANS("change...")));
    changeButton->addButtonListener (this);

    addAndMakeVisible (upButton = new DrawableButton (String::empty, DrawableButton::ImageOnButtonBackground));
    upButton->addButtonListener (this);

    {
        Path arrowPath;
        arrowPath.addArrow (50.0f, 100.0f, 50.0f, 0.0f, 40.0f, 100.0f, 50.0f);
        DrawablePath arrowImage;
        arrowImage.setFill (Colours::black.withAlpha (0.4f));
        arrowImage.setPath (arrowPath);

        ((DrawableButton*) upButton)->setImages (&arrowImage);
    }

    addAndMakeVisible (downButton = new DrawableButton (String::empty, DrawableButton::ImageOnButtonBackground));
    downButton->addButtonListener (this);

    {
        Path arrowPath;
        arrowPath.addArrow (50.0f, 0.0f, 50.0f, 100.0f, 40.0f, 100.0f, 50.0f);
        DrawablePath arrowImage;
        arrowImage.setFill (Colours::black.withAlpha (0.4f));
        arrowImage.setPath (arrowPath);

        ((DrawableButton*) downButton)->setImages (&arrowImage);
    }

    updateButtons();
}

FileSearchPathListComponent::~FileSearchPathListComponent()
{
    deleteAllChildren();
}

void FileSearchPathListComponent::updateButtons() throw()
{
    const bool anythingSelected = listBox->getNumSelectedRows() > 0;

    removeButton->setEnabled (anythingSelected);
    changeButton->setEnabled (anythingSelected);
    upButton->setEnabled (anythingSelected);
    downButton->setEnabled (anythingSelected);
}

void FileSearchPathListComponent::changed() throw()
{
    listBox->updateContent();
    listBox->repaint();
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

void FileSearchPathListComponent::setDefaultBrowseTarget (const File& newDefaultDirectory) throw()
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
    Font f (height * 0.7f);
    f.setHorizontalScale (0.9f);
    g.setFont (f);

    g.drawText (path [rowNumber].getFullPathName(),
                4, 0, width - 6, height,
                Justification::centredLeft, true);
}

void FileSearchPathListComponent::deleteKeyPressed (int row)
{
    if (((unsigned int) row) < (unsigned int) path.getNumPaths())
    {
        path.remove (row);
        changed();
    }
}

void FileSearchPathListComponent::returnKeyPressed (int row)
{
    FileChooser chooser (TRANS("Change folder..."), path [row], T("*"));

    if (chooser.browseForDirectory())
    {
        path.remove (row);
        path.add (chooser.getResult(), row);
        changed();
    }
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
    listBox->setBounds (2, 2, getWidth() - 4, buttonY - 5);

    addButton->setBounds (2, buttonY, buttonH, buttonH);
    removeButton->setBounds (addButton->getRight(), buttonY, buttonH, buttonH);

    ((TextButton*) changeButton)->changeWidthToFitText (buttonH);
    downButton->setSize (buttonH * 2, buttonH);
    upButton->setSize (buttonH * 2, buttonH);

    downButton->setTopRightPosition (getWidth() - 2, buttonY);
    upButton->setTopRightPosition (downButton->getX() - 4, buttonY);
    changeButton->setTopRightPosition (upButton->getX() - 8, buttonY);
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
            const int row = listBox->getRowContainingPosition (0, mouseY - listBox->getY());
            path.add (f, row);
            changed();
        }
    }
}

void FileSearchPathListComponent::buttonClicked (Button* button)
{
    const int currentRow = listBox->getSelectedRow();

    if (button == removeButton)
    {
        deleteKeyPressed (currentRow);
    }
    else if (button == addButton)
    {
        File start (defaultBrowseTarget);

        if (start == File::nonexistent)
            start = path [0];

        if (start == File::nonexistent)
            start = File::getCurrentWorkingDirectory();

        FileChooser chooser (TRANS("Add a folder..."), start, T("*"));

        if (chooser.browseForDirectory())
        {
            path.add (chooser.getResult(), currentRow);
        }
    }
    else if (button == changeButton)
    {
        returnKeyPressed (currentRow);
    }
    else if (button == upButton)
    {
        if (currentRow > 0 && currentRow < path.getNumPaths())
        {
            const File f (path[currentRow]);
            path.remove (currentRow);
            path.add (f, currentRow - 1);
            listBox->selectRow (currentRow - 1);
        }
    }
    else if (button == downButton)
    {
        if (currentRow >= 0 && currentRow < path.getNumPaths() - 1)
        {
            const File f (path[currentRow]);
            path.remove (currentRow);
            path.add (f, currentRow + 1);
            listBox->selectRow (currentRow + 1);
        }
    }

    changed();
}


END_JUCE_NAMESPACE
