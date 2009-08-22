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

#include "../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_PluginListComponent.h"
#include "juce_PluginDirectoryScanner.h"
#include "../../gui/components/controls/juce_TextEditor.h"
#include "../../gui/components/windows/juce_AlertWindow.h"
#include "../../gui/graphics/fonts/juce_GlyphArrangement.h"
#include "../../gui/components/filebrowser/juce_FileSearchPathListComponent.h"
#include "../../text/juce_LocalisedStrings.h"
#include "../../events/juce_MessageManager.h"


//==============================================================================
PluginListComponent::PluginListComponent (KnownPluginList& listToEdit,
                                          const File& deadMansPedalFile_,
                                          PropertiesFile* const propertiesToUse_)
    : list (listToEdit),
      deadMansPedalFile (deadMansPedalFile_),
      propertiesToUse (propertiesToUse_)
{
    addAndMakeVisible (listBox = new ListBox (String::empty, this));

    addAndMakeVisible (optionsButton = new TextButton ("Options..."));
    optionsButton->addButtonListener (this);
    optionsButton->setTriggeredOnMouseDown (true);

    setSize (400, 600);
    list.addChangeListener (this);
}

PluginListComponent::~PluginListComponent()
{
    list.removeChangeListener (this);
    deleteAllChildren();
}

void PluginListComponent::resized()
{
    listBox->setBounds (0, 0, getWidth(), getHeight() - 30);
    optionsButton->changeWidthToFitText (24);
    optionsButton->setTopLeftPosition (8, getHeight() - 28);
}

void PluginListComponent::changeListenerCallback (void*)
{
    listBox->updateContent();
    listBox->repaint();
}

int PluginListComponent::getNumRows()
{
    return list.getNumTypes();
}

void PluginListComponent::paintListBoxItem (int row,
                                            Graphics& g,
                                            int width, int height,
                                            bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll (findColour (TextEditor::highlightColourId));

    const PluginDescription* const pd = list.getType (row);

    if (pd != 0)
    {
        GlyphArrangement ga;
        ga.addCurtailedLineOfText (Font (height * 0.7f, Font::bold), pd->name, 8.0f, height * 0.8f, width - 10.0f, true);

        g.setColour (Colours::black);
        ga.draw (g);

        float x, y, r, b;
        ga.getBoundingBox (0, -1, x, y, r, b, false);

        String desc;
        desc << pd->pluginFormatName
             << (pd->isInstrument ? " instrument" : " effect")
             << " - "
             << pd->numInputChannels << (pd->numInputChannels == 1 ? " in" : " ins")
             << " / "
             << pd->numOutputChannels << (pd->numOutputChannels == 1 ? " out" : " outs");

        if (pd->manufacturerName.isNotEmpty())
            desc << " - " << pd->manufacturerName;

        if (pd->version.isNotEmpty())
            desc << " - " << pd->version;

         if (pd->category.isNotEmpty())
            desc << " - category: '" << pd->category << '\'';

        g.setColour (Colours::grey);

        ga.clear();
        ga.addCurtailedLineOfText (Font (height * 0.6f), desc, r + 10.0f, height * 0.8f, width - r - 12.0f, true);
        ga.draw (g);
    }
}

void PluginListComponent::deleteKeyPressed (int lastRowSelected)
{
    list.removeType (lastRowSelected);
}

void PluginListComponent::buttonClicked (Button* b)
{
    if (optionsButton == b)
    {
        PopupMenu menu;
        menu.addItem (1, TRANS("Clear list"));
        menu.addItem (5, TRANS("Remove selected plugin from list"), listBox->getNumSelectedRows() > 0);
        menu.addItem (6, TRANS("Show folder containing selected plugin"), listBox->getNumSelectedRows() > 0);
        menu.addItem (7, TRANS("Remove any plugins whose files no longer exist"));
        menu.addSeparator();
        menu.addItem (2, TRANS("Sort alphabetically"));
        menu.addItem (3, TRANS("Sort by category"));
        menu.addItem (4, TRANS("Sort by manufacturer"));
        menu.addSeparator();

        for (int i = 0; i < AudioPluginFormatManager::getInstance()->getNumFormats(); ++i)
        {
            AudioPluginFormat* const format = AudioPluginFormatManager::getInstance()->getFormat (i);

            if (format->getDefaultLocationsToSearch().getNumPaths() > 0)
                menu.addItem (10 + i, "Scan for new or updated " + format->getName() + " plugins...");
        }

        const int r = menu.showAt (optionsButton);

        if (r == 1)
        {
            list.clear();
        }
        else if (r == 2)
        {
            list.sort (KnownPluginList::sortAlphabetically);
        }
        else if (r == 3)
        {
            list.sort (KnownPluginList::sortByCategory);
        }
        else if (r == 4)
        {
            list.sort (KnownPluginList::sortByManufacturer);
        }
        else if (r == 5)
        {
            const SparseSet <int> selected (listBox->getSelectedRows());

            for (int i = list.getNumTypes(); --i >= 0;)
                if (selected.contains (i))
                    list.removeType (i);
        }
        else if (r == 6)
        {
            const PluginDescription* const desc = list.getType (listBox->getSelectedRow());

            if (desc != 0)
            {
                if (File (desc->fileOrIdentifier).existsAsFile())
                    File (desc->fileOrIdentifier).getParentDirectory().startAsProcess();
            }
        }
        else if (r == 7)
        {
            for (int i = list.getNumTypes(); --i >= 0;)
            {
                if (! AudioPluginFormatManager::getInstance()->doesPluginStillExist (*list.getType (i)))
                {
                    list.removeType (i);
                }
            }
        }
        else if (r != 0)
        {
            typeToScan = r - 10;
            startTimer (1);
        }
    }
}

void PluginListComponent::timerCallback()
{
    stopTimer();
    scanFor (AudioPluginFormatManager::getInstance()->getFormat (typeToScan));
}

bool PluginListComponent::isInterestedInFileDrag (const StringArray& /*files*/)
{
    return true;
}

void PluginListComponent::filesDropped (const StringArray& files, int, int)
{
    OwnedArray <PluginDescription> typesFound;
    list.scanAndAddDragAndDroppedFiles (files, typesFound);
}

void PluginListComponent::scanFor (AudioPluginFormat* format)
{
    if (format == 0)
        return;

    FileSearchPath path (format->getDefaultLocationsToSearch());

    if (propertiesToUse != 0)
        path = propertiesToUse->getValue ("lastPluginScanPath_" + format->getName(), path.toString());

    {
        AlertWindow aw (TRANS("Select folders to scan..."), String::empty, AlertWindow::NoIcon);
        FileSearchPathListComponent pathList;
        pathList.setSize (500, 300);
        pathList.setPath (path);

        aw.addCustomComponent (&pathList);
        aw.addButton (TRANS("Scan"), 1, KeyPress::returnKey);
        aw.addButton (TRANS("Cancel"), 0, KeyPress (KeyPress::escapeKey));

        if (aw.runModalLoop() == 0)
            return;

        path = pathList.getPath();
    }

    if (propertiesToUse != 0)
    {
        propertiesToUse->setValue ("lastPluginScanPath_" + format->getName(), path.toString());
        propertiesToUse->saveIfNeeded();
    }

    double progress = 0.0;

    AlertWindow aw (TRANS("Scanning for plugins..."),
                    TRANS("Searching for all possible plugin files..."), AlertWindow::NoIcon);

    aw.addButton (TRANS("Cancel"), 0, KeyPress (KeyPress::escapeKey));
    aw.addProgressBarComponent (progress);

    aw.enterModalState();

    MessageManager::getInstance()->runDispatchLoopUntil (300);

    PluginDirectoryScanner scanner (list, *format, path, true, deadMansPedalFile);

    for (;;)
    {
        aw.setMessage (TRANS("Testing:\n\n")
                         + scanner.getNextPluginFileThatWillBeScanned());

        MessageManager::getInstance()->runDispatchLoopUntil (20);

        if (! scanner.scanNextFile (true))
            break;

        if (! aw.isCurrentlyModal())
            break;

        progress = scanner.getProgress();
    }

    if (scanner.getFailedFiles().size() > 0)
    {
        StringArray shortNames;

        for (int i = 0; i < scanner.getFailedFiles().size(); ++i)
            shortNames.add (File (scanner.getFailedFiles()[i]).getFileName());

        AlertWindow::showMessageBox (AlertWindow::InfoIcon,
                                     TRANS("Scan complete"),
                                     TRANS("Note that the following files appeared to be plugin files, but failed to load correctly:\n\n")
                                        + shortNames.joinIntoString (", "));
    }
}


END_JUCE_NAMESPACE
