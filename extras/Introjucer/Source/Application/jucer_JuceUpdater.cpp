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

#include "../jucer_Headers.h"
#include "jucer_JuceUpdater.h"
#include "../Project/jucer_Module.h"


//==============================================================================
JuceUpdater::JuceUpdater (ModuleList& moduleList_, const String& message)
    : moduleList (moduleList_),
      messageLabel (String::empty, message),
      filenameComp ("Juce Folder", ModuleList::getLocalModulesFolder (nullptr),
                    true, true, false, "*", String::empty, "Select your Juce folder"),
      checkNowButton ("Check for available updates on the JUCE website...",
                      "Contacts the website to see if new modules are available"),
      installButton ("Download and install selected modules..."),
      selectAllButton ("Select/Deselect All")
{
    messageLabel.setJustificationType (Justification::centred);
    addAndMakeVisible (&messageLabel);

    addAndMakeVisible (&label);
    addAndMakeVisible (&currentVersionLabel);
    addAndMakeVisible (&filenameComp);
    addAndMakeVisible (&checkNowButton);
    addAndMakeVisible (&installButton);
    addAndMakeVisible (&selectAllButton);
    checkNowButton.addListener (this);
    installButton.addListener (this);
    selectAllButton.addListener (this);
    filenameComp.addListener (this);

    currentVersionLabel.setFont (Font (14.0f, Font::italic));
    label.setFont (Font (12.0f));
    label.setText ("Local modules folder:", false);

    addAndMakeVisible (&availableVersionsList);
    availableVersionsList.setModel (this);

    updateInstallButtonStatus();
    versionsToDownload = ValueTree ("modules");
    versionsToDownload.addListener (this);

    setSize (600, 500);

    checkNow();
}

JuceUpdater::~JuceUpdater()
{
    checkNowButton.removeListener (this);
    filenameComp.removeListener (this);
}

//==============================================================================
class UpdateDialogWindow : public DialogWindow
{
public:
    UpdateDialogWindow (JuceUpdater* updater, Component* componentToCentreAround)
        : DialogWindow ("JUCE Module Updater",
                        Colours::lightgrey, true, true)
    {
        setUsingNativeTitleBar (true);
        setContentOwned (updater, true);
        centreAroundComponent (componentToCentreAround, getWidth(), getHeight());
        setResizable (true, true);
    }

    void closeButtonPressed()
    {
        setVisible (false);
    }

private:
    JUCE_DECLARE_NON_COPYABLE (UpdateDialogWindow);
};

void JuceUpdater::show (ModuleList& moduleList, Component* mainWindow, const String& message)
{
    UpdateDialogWindow w (new JuceUpdater (moduleList, message), mainWindow);
    w.runModalLoop();
}

void JuceUpdater::resized()
{
    messageLabel.setBounds (20, 10, getWidth() - 40, messageLabel.getText().isEmpty() ? 0 : 30);

    filenameComp.setBounds (20, messageLabel.getBottom() + 20, getWidth() - 40, 22);
    label.setBounds (filenameComp.getX(), filenameComp.getY() - 18, filenameComp.getWidth(), 18);
    currentVersionLabel.setBounds (filenameComp.getX(), filenameComp.getBottom(), filenameComp.getWidth(), 25);
    checkNowButton.changeWidthToFitText (22);
    checkNowButton.setCentrePosition (getWidth() / 2, filenameComp.getBottom() + 20);
    availableVersionsList.setBounds (filenameComp.getX(), checkNowButton.getBottom() + 20,
                                     filenameComp.getWidth(),
                                     getHeight() - 30 - (checkNowButton.getBottom() + 20));
    installButton.changeWidthToFitText (22);
    installButton.setTopRightPosition (availableVersionsList.getRight(), getHeight() - 28);
    selectAllButton.setBounds (availableVersionsList.getX(),
                               availableVersionsList.getBottom() + 4,
                               installButton.getX() - availableVersionsList.getX() - 20, 22);
}

void JuceUpdater::paint (Graphics& g)
{
    g.fillAll (Colours::white);
}

void JuceUpdater::buttonClicked (Button* b)
{
    if (b == &installButton)
        install();
    else if (b == &selectAllButton)
        selectAll();
    else
        checkNow();
}

void JuceUpdater::refresh()
{
    availableVersionsList.updateContent();
    availableVersionsList.repaint();
}

class WebsiteContacterThread  : public Thread,
                                private AsyncUpdater
{
public:
    WebsiteContacterThread (JuceUpdater& owner_, const ModuleList& latestList)
        : Thread ("Module updater"),
          owner (owner_),
          downloaded (latestList)
    {
        startThread();
    }

    ~WebsiteContacterThread()
    {
        stopThread (10000);
    }

    void run()
    {
        if (downloaded.loadFromWebsite())
            triggerAsyncUpdate();
        else
            AlertWindow::showMessageBox (AlertWindow::InfoIcon,
                                         "Module Update",
                                         "Couldn't connect to the JUCE webserver!");
    }

    void handleAsyncUpdate()
    {
        owner.backgroundUpdateComplete (downloaded);
    }

private:
    JuceUpdater& owner;
    ModuleList downloaded;
};

void JuceUpdater::checkNow()
{
    websiteContacterThread = nullptr;
    websiteContacterThread = new WebsiteContacterThread (*this, latestList);
}

void JuceUpdater::backgroundUpdateComplete (const ModuleList& newList)
{
    latestList = newList;
    websiteContacterThread = nullptr;

    if (latestList == moduleList)
        AlertWindow::showMessageBox (AlertWindow::InfoIcon,
                                     "Module Update",
                                     "No new modules are available");
    refresh();
}

int JuceUpdater::getNumCheckedModules() const
{
    int numChecked = 0;

    for (int i = latestList.modules.size(); --i >= 0;)
        if (versionsToDownload [latestList.modules.getUnchecked(i)->uid])
            ++numChecked;

    return numChecked;
}

bool JuceUpdater::isLatestVersion (const String& moduleID) const
{
    const ModuleList::Module* m1 = moduleList.findModuleInfo (moduleID);
    const ModuleList::Module* m2 = latestList.findModuleInfo (moduleID);

    return m1 != nullptr && m2 != nullptr && m1->version == m2->version;
}

void JuceUpdater::updateInstallButtonStatus()
{
    const int numChecked = getNumCheckedModules();
    installButton.setEnabled (numChecked > 0);
    selectAllButton.setToggleState (numChecked > latestList.modules.size() / 2, false);
}

void JuceUpdater::filenameComponentChanged (FilenameComponent*)
{
    moduleList.rescan (filenameComp.getCurrentFile());
    filenameComp.setCurrentFile (moduleList.getModulesFolder(), true, false);

    if (! ModuleList::isModulesFolder (moduleList.getModulesFolder()))
        currentVersionLabel.setText ("(Not a Juce folder)", false);
    else
        currentVersionLabel.setText (String::empty, false);

    refresh();
}

void JuceUpdater::selectAll()
{
    bool enable = getNumCheckedModules() < latestList.modules.size() / 2;

    versionsToDownload.removeAllProperties (nullptr);

    if (enable)
    {
        for (int i = latestList.modules.size(); --i >= 0;)
            if (! isLatestVersion (latestList.modules.getUnchecked(i)->uid))
                versionsToDownload.setProperty (latestList.modules.getUnchecked(i)->uid, true, nullptr);
    }
}

//==============================================================================
int JuceUpdater::getNumRows()
{
    return latestList.modules.size();
}

void JuceUpdater::paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll (findColour (TextEditor::highlightColourId));
}

Component* JuceUpdater::refreshComponentForRow (int rowNumber, bool isRowSelected, Component* existingComponentToUpdate)
{
    class UpdateListComponent  : public Component
    {
    public:
        UpdateListComponent (JuceUpdater& updater_)
            : updater (updater_)
        {
            addChildComponent (&toggle);
            toggle.setWantsKeyboardFocus (false);
            setInterceptsMouseClicks (false, true);
        }

        void setModule (const ModuleList::Module* newModule,
                        const ModuleList::Module* existingModule,
                        const Value& value)
        {
            if (newModule != nullptr)
            {
                toggle.getToggleStateValue().referTo (value);
                toggle.setVisible (true);
                toggle.setEnabled (true);

                name = newModule->uid;
                status = String::empty;

                if (existingModule == nullptr)
                {
                    status << " (not currently installed)";
                }
                else if (existingModule->version != newModule->version)
                {
                    status << " installed: " << existingModule->version
                           << ", available: " << newModule->version;
                }
                else
                {
                    status << " (latest version already installed: " << existingModule->version << ")";
                    toggle.setEnabled (false);
                }
            }
            else
            {
                name = status = String::empty;
                toggle.setVisible (false);
            }
        }

        void paint (Graphics& g)
        {
            g.setColour (Colours::green.withAlpha (0.12f));

            g.fillRect (0, 1, getWidth(), getHeight() - 2);
            g.setColour (Colours::black);
            g.setFont (getHeight() * 0.7f);

            g.drawText (name, toggle.getRight() + 4, 0, getWidth() / 2 - toggle.getRight() - 4, getHeight(),
                        Justification::centredLeft, true);

            g.drawText (status, getWidth() / 2, 0, getWidth() / 2, getHeight(),
                        Justification::centredLeft, true);
        }

        void resized()
        {
            toggle.setBounds (getLocalBounds().reduced (2));
        }

    private:
        JuceUpdater& updater;
        ToggleButton toggle;
        String name, status;
    };

    UpdateListComponent* c = dynamic_cast <UpdateListComponent*> (existingComponentToUpdate);
    if (c == nullptr)
        c = new UpdateListComponent (*this);

    ModuleList::Module* m = latestList.modules [rowNumber];

    if (m != nullptr)
        c->setModule (m,
                      moduleList.findModuleInfo (m->uid),
                      versionsToDownload.getPropertyAsValue (m->uid, nullptr));
    else
        c->setModule (nullptr, nullptr, Value());

    return c;
}

//==============================================================================
class InstallThread   : public ThreadWithProgressWindow
{
public:
    InstallThread (const ModuleList& targetList_,
                   const ModuleList& list_, const StringArray& itemsToInstall_)
        : ThreadWithProgressWindow ("Installing New Modules", true, true),
          result (Result::ok()),
          targetList (targetList_),
          list (list_),
          itemsToInstall (itemsToInstall_)
    {
    }

    void run()
    {
        for (int i = 0; i < itemsToInstall.size(); ++i)
        {
            const ModuleList::Module* m = list.findModuleInfo (itemsToInstall[i]);

            jassert (m != nullptr);
            if (m != nullptr)
            {
                setProgress (i / (double) itemsToInstall.size());

                MemoryBlock downloaded;
                result = download (*m, downloaded);

                if (result.failed())
                    break;

                if (threadShouldExit())
                    break;

                result = unzip (*m, downloaded);

                if (result.failed())
                    break;
            }

            if (threadShouldExit())
                break;
        }
    }

    Result download (const ModuleList::Module& m, MemoryBlock& dest)
    {
        setStatusMessage ("Downloading " + m.uid + "...");

        if (m.url.readEntireBinaryStream (dest, false))
            return Result::ok();

        return Result::fail ("Failed to download from: " + m.url.toString (false));
    }

    Result unzip (const ModuleList::Module& m, const MemoryBlock& data)
    {
        setStatusMessage ("Installing " + m.uid + "...");

        MemoryInputStream input (data, false);
        ZipFile zip (input);

        if (zip.getNumEntries() == 0)
            return Result::fail ("The downloaded file wasn't a valid module file!");

        return zip.uncompressTo (targetList.getModulesFolder(), true);
    }

    Result result;

private:
    ModuleList targetList, list;
    StringArray itemsToInstall;
};

void JuceUpdater::install()
{
    if (! moduleList.getModulesFolder().createDirectory())
    {
        AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                     "Module Update",
                                     "Couldn't create the target folder!");
        return;
    }

    StringArray itemsWanted;

    for (int i = latestList.modules.size(); --i >= 0;)
        if (versionsToDownload [latestList.modules.getUnchecked(i)->uid])
            itemsWanted.add (latestList.modules.getUnchecked(i)->uid);

    {
        InstallThread thread (moduleList, latestList, itemsWanted);
        thread.runThread();
    }

    moduleList.rescan();
    refresh();
}

void JuceUpdater::valueTreePropertyChanged (ValueTree&, const Identifier&) { updateInstallButtonStatus(); }
void JuceUpdater::valueTreeChildAdded (ValueTree&, ValueTree&) {}
void JuceUpdater::valueTreeChildRemoved (ValueTree&, ValueTree&) {}
void JuceUpdater::valueTreeChildOrderChanged (ValueTree&) {}
void JuceUpdater::valueTreeParentChanged (ValueTree&) {}
