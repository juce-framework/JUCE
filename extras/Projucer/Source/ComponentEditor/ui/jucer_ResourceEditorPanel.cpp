/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#include "../../jucer_Headers.h"
#include "jucer_ResourceEditorPanel.h"


//==============================================================================
class ResourceListButton  : public Component,
                            private ButtonListener
{
public:
    ResourceListButton (JucerDocument& doc)
        : document (doc), reloadButton ("Reload"), row (0)
    {
        setInterceptsMouseClicks (false, true);
        addAndMakeVisible (reloadButton);
        reloadButton.addListener (this);
    }

    void update (int newRow)
    {
        row = newRow;
        reloadButton.setVisible (document.getResources() [row] != nullptr);
    }

    void resized()
    {
        reloadButton.setBoundsInset (BorderSize<int> (2));
    }

    void buttonClicked (Button*)
    {
        if (const BinaryResources::BinaryResource* const r = document.getResources() [row])
            document.getResources()
                .browseForResource ("Select a file to replace this resource",
                                    "*",
                                    File (r->originalFilename),
                                    r->name);
    }

private:
    JucerDocument& document;
    TextButton reloadButton;
    int row;
};


//==============================================================================
ResourceEditorPanel::ResourceEditorPanel (JucerDocument& doc)
    : document (doc),
      addButton ("Add new resource..."),
      reloadAllButton ("Reload all resources"),
      delButton ("Delete selected resources")
{
    addAndMakeVisible (addButton);
    addButton.addListener (this);

    addAndMakeVisible (reloadAllButton);
    reloadAllButton.addListener (this);

    addAndMakeVisible (delButton);
    delButton.addListener (this);
    delButton.setEnabled (false);

    addAndMakeVisible (listBox = new TableListBox (String(), this));
    listBox->getHeader().addColumn ("name", 1, 150, 80, 400);
    listBox->getHeader().addColumn ("original file", 2, 350, 80, 800);
    listBox->getHeader().addColumn ("size", 3, 100, 40, 150);
    listBox->getHeader().addColumn ("reload", 4, 100, 100, 100, TableHeaderComponent::notResizableOrSortable);
    listBox->getHeader().setStretchToFitActive (true);

    listBox->setColour (ListBox::backgroundColourId, Colours::lightgrey);
    listBox->setColour (ListBox::outlineColourId, Colours::darkgrey);
    listBox->setOutlineThickness (1);
    listBox->updateContent();

    document.addChangeListener (this);
    handleCommandMessage (1);
}

ResourceEditorPanel::~ResourceEditorPanel()
{
    document.removeChangeListener (this);
}

int ResourceEditorPanel::getNumRows()
{
    return document.getResources().size();
}

void ResourceEditorPanel::paintRowBackground (Graphics& g, int /*rowNumber*/,
                                              int /*width*/, int /*height*/, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll (findColour (TextEditor::highlightColourId));
}

void ResourceEditorPanel::paintCell (Graphics& g, int rowNumber, int columnId, int width, int height,
                                     bool /*rowIsSelected*/)
{
    if (const BinaryResources::BinaryResource* const r = document.getResources() [rowNumber])
    {
        String text;

        if (columnId == 1)
            text = r->name;
        else if (columnId == 2)
            text = r->originalFilename;
        else if (columnId == 3)
            text = File::descriptionOfSizeInBytes ((int64) r->data.getSize());

        g.setFont (13.0f);
        g.drawText (text, 4, 0, width - 6, height, Justification::centredLeft, true);
    }
}

Component* ResourceEditorPanel::refreshComponentForCell (int rowNumber, int columnId, bool /*isRowSelected*/,
                                                         Component* existingComponentToUpdate)
{
    if (columnId != 4)
        return nullptr;

    if (existingComponentToUpdate == nullptr)
        existingComponentToUpdate = new ResourceListButton (document);

    ((ResourceListButton*) existingComponentToUpdate)->update (rowNumber);

    return existingComponentToUpdate;
}

int ResourceEditorPanel::getColumnAutoSizeWidth (int columnId)
{
    if (columnId == 4)
        return 0;

    Font f (13.0f);
    int widest = 40;

    for (int i = document.getResources().size(); --i >= 0;)
    {
        const BinaryResources::BinaryResource* const r = document.getResources() [i];
        jassert (r != nullptr);
        String text;

        if (columnId == 1)
            text = r->name;
        else if (columnId == 2)
            text = r->originalFilename;
        else if (columnId == 3)
            text = File::descriptionOfSizeInBytes ((int64) r->data.getSize());

        widest = jmax (widest, f.getStringWidth (text));
    }

    return widest + 10;
}

//==============================================================================
class ResourceSorter
{
public:
    ResourceSorter (const int columnId_, const bool forwards)
        : columnId (columnId_),
          direction (forwards ? 1 : -1)
    {
    }

    int compareElements (BinaryResources::BinaryResource* first, BinaryResources::BinaryResource* second)
    {
        if (columnId == 1)  return direction * first->name.compare (second->name);
        if (columnId == 2)  return direction * first->originalFilename.compare (second->originalFilename);
        if (columnId == 3)  return direction * (int) first->data.getSize() - (int) second->data.getSize();

        return 0;
    }

private:
    const int columnId, direction;
    ResourceSorter (const ResourceSorter&);
    ResourceSorter& operator= (const ResourceSorter&);
};

void ResourceEditorPanel::sortOrderChanged (int newSortColumnId, const bool isForwards)
{
    ResourceSorter sorter (newSortColumnId, isForwards);
    document.getResources().sort (sorter);
}

//==============================================================================
void ResourceEditorPanel::selectedRowsChanged (int /*lastRowSelected*/)
{
    delButton.setEnabled (listBox->getNumSelectedRows() > 0);
}

void ResourceEditorPanel::resized()
{
    listBox->setBounds (6, 4, getWidth() - 12, getHeight() - 38);

    addButton.changeWidthToFitText (22);
    addButton.setTopLeftPosition (8, getHeight() - 30);

    reloadAllButton.changeWidthToFitText (22);
    reloadAllButton.setTopLeftPosition (addButton.getRight() + 10, getHeight() - 30);

    delButton.changeWidthToFitText (22);
    delButton.setTopRightPosition (getWidth() - 8, getHeight() - 30);
}

void ResourceEditorPanel::visibilityChanged()
{
    if (isVisible())
        listBox->updateContent();
}

void ResourceEditorPanel::changeListenerCallback (ChangeBroadcaster*)
{
    if (isVisible())
        listBox->updateContent();
}

void ResourceEditorPanel::buttonClicked (Button* b)
{
    if (b == &addButton)
    {
        document.getResources()
            .browseForResource ("Select a file to add as a resource",
                                "*",
                                File(),
                                String());
    }
    else if (b == &delButton)
    {
        document.getResources().remove (listBox->getSelectedRow (0));
    }
    else if (b == &reloadAllButton)
    {
        StringArray failed;

        for (int i = 0; i < document.getResources().size(); ++i)
        {
            if (! document.getResources().reload (i))
                failed.add (document.getResources().getResourceNames() [i]);
        }

        if (failed.size() > 0)
        {
            AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                         TRANS("Reloading resources"),
                                         TRANS("The following resources couldn't be reloaded from their original files:\n\n")
                                            + failed.joinIntoString (", "));
        }
    }
}
