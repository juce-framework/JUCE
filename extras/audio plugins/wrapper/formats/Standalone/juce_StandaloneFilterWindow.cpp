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

#include "juce_StandaloneFilterWindow.h"
#include "../../juce_IncludeCharacteristics.h"


//==============================================================================
StandaloneFilterWindow::StandaloneFilterWindow (const String& title,
                                                const Colour& backgroundColour)
    : DocumentWindow (title, backgroundColour,
                      DocumentWindow::minimiseButton
                       | DocumentWindow::closeButton),
      filter (0),
      deviceManager (0),
      optionsButton (0)
{
    setTitleBarButtonsRequired (DocumentWindow::minimiseButton | DocumentWindow::closeButton, false);

    PropertySet* const globalSettings = getGlobalSettings();

    optionsButton = new TextButton (T("options"));
    Component::addAndMakeVisible (optionsButton);
    optionsButton->addButtonListener (this);
    optionsButton->setTriggeredOnMouseDown (true);

    JUCE_TRY
    {
        filter = createPluginFilter();

        if (filter != 0)
        {
            deviceManager = new AudioFilterStreamingDeviceManager();
            deviceManager->setFilter (filter);

            XmlElement* savedState = 0;

            if (globalSettings != 0)
                savedState = globalSettings->getXmlValue (T("audioSetup"));

            deviceManager->initialise (filter->getNumInputChannels(),
                                       filter->getNumOutputChannels(),
                                       savedState,
                                       true);

            delete savedState;

            if (globalSettings != 0)
            {
                juce::MemoryBlock data;

                if (data.fromBase64Encoding (globalSettings->getValue (T("filterState")))
                     && data.getSize() > 0)
                {
                    filter->setStateInformation (data.getData(), data.getSize());
                }
            }

            setContentComponent (filter->createEditorIfNeeded(), true, true);

            const int x = globalSettings->getIntValue (T("windowX"), -100);
            const int y = globalSettings->getIntValue (T("windowY"), -100);

            if (x != -100 && y != -100)
                setBoundsConstrained (x, y, getWidth(), getHeight());
            else
                centreWithSize (getWidth(), getHeight());
        }
    }
    JUCE_CATCH_ALL

    if (deviceManager == 0)
    {
        jassertfalse    // Your filter didn't create correctly! In a standalone app that's not too great.
        JUCEApplication::quit();
    }
}

StandaloneFilterWindow::~StandaloneFilterWindow()
{
    PropertySet* const globalSettings = getGlobalSettings();

    globalSettings->setValue (T("windowX"), getX());
    globalSettings->setValue (T("windowY"), getY());

    deleteAndZero (optionsButton);

    if (globalSettings != 0 && deviceManager != 0)
    {
        XmlElement* const xml = deviceManager->createStateXml();
        globalSettings->setValue (T("audioSetup"), xml);
        delete xml;
    }

    deleteAndZero (deviceManager);

    if (globalSettings != 0 && filter != 0)
    {
        juce::MemoryBlock data;
        filter->getStateInformation (data);

        globalSettings->setValue (T("filterState"), data.toBase64Encoding());
    }

    deleteFilter();
}

//==============================================================================
void StandaloneFilterWindow::deleteFilter()
{
    if (deviceManager != 0)
        deviceManager->setFilter (0);

    if (filter != 0 && getContentComponent() != 0)
    {
        filter->editorBeingDeleted (dynamic_cast <AudioFilterEditor*> (getContentComponent()));
        setContentComponent (0, true);
    }

    deleteAndZero (filter);
}

void StandaloneFilterWindow::resetFilter()
{
    deleteFilter();

    filter = createPluginFilter();

    if (filter != 0)
    {
        if (deviceManager != 0)
            deviceManager->setFilter (filter);

        setContentComponent (filter->createEditorIfNeeded(), true, true);
    }

    PropertySet* const globalSettings = getGlobalSettings();

    if (globalSettings != 0)
        globalSettings->removeValue (T("filterState"));
}

//==============================================================================
void StandaloneFilterWindow::saveState()
{
    PropertySet* const globalSettings = getGlobalSettings();

    FileChooser fc (TRANS("Save current state"),
                    globalSettings != 0 ? File (globalSettings->getValue (T("lastStateFile")))
                                        : File::nonexistent);

    if (fc.browseForFileToSave (true))
    {
        juce::MemoryBlock data;
        filter->getStateInformation (data);

        if (! fc.getResult().replaceWithData (data.getData(), data.getSize()))
        {
            AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                         TRANS("Error whilst saving"),
                                         TRANS("Couldn't write to the specified file!"));
        }
    }
}

void StandaloneFilterWindow::loadState()
{
    PropertySet* const globalSettings = getGlobalSettings();

    FileChooser fc (TRANS("Load a saved state"),
                    globalSettings != 0 ? File (globalSettings->getValue (T("lastStateFile")))
                                        : File::nonexistent);

    if (fc.browseForFileToOpen())
    {
        juce::MemoryBlock data;

        if (fc.getResult().loadFileAsData (data))
        {
            filter->setStateInformation (data.getData(), data.getSize());
        }
        else
        {
            AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                         TRANS("Error whilst loading"),
                                         TRANS("Couldn't read from the specified file!"));
        }
    }
}

//==============================================================================
PropertySet* StandaloneFilterWindow::getGlobalSettings()
{
    /* If you want this class to store the plugin's settings, you can set up an
       ApplicationProperties object and use this method as it is, or override this
       method to return your own custom PropertySet.

       If using this method without changing it, you'll probably need to call
       ApplicationProperties::setStorageParameters() in your plugin's constructor to
       tell it where to save the file.
    */
    return ApplicationProperties::getInstance()->getUserSettings();
}

void StandaloneFilterWindow::showAudioSettingsDialog()
{
    AudioDeviceSelectorComponent selectorComp (*deviceManager,
                                               filter->getNumInputChannels(),
                                               filter->getNumInputChannels(),
                                               filter->getNumOutputChannels(),
                                               filter->getNumOutputChannels(),
                                               true);

    selectorComp.setSize (500, 350);

    DialogWindow::showModalDialog (TRANS("Audio Settings"), &selectorComp, this, Colours::lightgrey, true, false, false);
}

//==============================================================================
void StandaloneFilterWindow::closeButtonPressed()
{
    JUCEApplication::quit();
}

void StandaloneFilterWindow::resized()
{
    DocumentWindow::resized();

    if (optionsButton != 0)
        optionsButton->setBounds (8, 6, 60, getTitleBarHeight() - 8);
}

void StandaloneFilterWindow::buttonClicked (Button*)
{
    if (filter == 0)
        return;

    PopupMenu m;
    m.addItem (1, TRANS("Audio Settings..."));
    m.addSeparator();
    m.addItem (2, TRANS("Save current state..."));
    m.addItem (3, TRANS("Load a saved state..."));
    m.addSeparator();
    m.addItem (4, TRANS("Reset to default state"));

    switch (m.showAt (optionsButton))
    {
    case 1:
        showAudioSettingsDialog();
        break;

    case 2:
        saveState();
        break;

    case 3:
        loadState();
        break;

    case 4:
        resetFilter();
        break;

    default:
        break;
    }
}
