/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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


class ModulesPanel  : public Component,
                      private TableListBoxModel,
                      private ValueTree::Listener,
                      private Button::Listener
{
public:
    ModulesPanel (Project& p)
        : project (p),
          modulesValueTree (p.getModules().state),
          addWebModuleButton ("Download and add a module..."),
          updateModuleButton ("Install updates to modules..."),
          setCopyModeButton  ("Set copy-mode for all modules..."),
          copyPathButton ("Set paths for all modules...")
    {
        table.getHeader().addColumn ("Module", nameCol, 180, 100, 400, TableHeaderComponent::notSortable);
        table.getHeader().addColumn ("Installed Version", versionCol, 100, 100, 100, TableHeaderComponent::notSortable);
        table.getHeader().addColumn ("Available Version", updateCol, 100, 100, 100, TableHeaderComponent::notSortable);
        table.getHeader().addColumn ("Make Local Copy", copyCol, 100, 100, 100, TableHeaderComponent::notSortable);
        table.getHeader().addColumn ("Paths", pathCol, 250, 100, 600, TableHeaderComponent::notSortable);

        table.setModel (this);
        table.setColour (TableListBox::backgroundColourId, Colours::transparentBlack);
        addAndMakeVisible (table);
        table.updateContent();
        table.setRowHeight (20);

        addAndMakeVisible (addWebModuleButton);
        addAndMakeVisible (updateModuleButton);
        addAndMakeVisible (setCopyModeButton);
        addAndMakeVisible (copyPathButton);
        addWebModuleButton.addListener (this);
        updateModuleButton.addListener (this);
        updateModuleButton.setEnabled (false);
        setCopyModeButton.addListener (this);
        setCopyModeButton.setTriggeredOnMouseDown (true);
        copyPathButton.addListener (this);
        copyPathButton.setTriggeredOnMouseDown (true);

        modulesValueTree.addListener (this);
        lookAndFeelChanged();
    }

    void paint (Graphics& g) override
    {
        if (webUpdateThread == nullptr)
            webUpdateThread = new WebsiteUpdateFetchThread (*this);

        IntrojucerLookAndFeel::fillWithBackgroundTexture (*this, g);
    }

    void resized() override
    {
        Rectangle<int> r (getLocalBounds().reduced (5, 4));

        table.setBounds (r.removeFromTop (table.getRowPosition (getNumRows() - 1, true).getBottom() + 20));

        Rectangle<int> buttonRow (r.removeFromTop (32).removeFromBottom (28));
        addWebModuleButton.setBounds (buttonRow.removeFromLeft (jmin (260, r.getWidth() / 3)));
        buttonRow.removeFromLeft (8);
        updateModuleButton.setBounds (buttonRow.removeFromLeft (jmin (260, r.getWidth() / 3)));
        buttonRow.removeFromLeft (8);

        buttonRow = r.removeFromTop (34).removeFromBottom (28);
        setCopyModeButton.setBounds (buttonRow.removeFromLeft (jmin (260, r.getWidth() / 3)));
        buttonRow.removeFromLeft (8);
        copyPathButton.setBounds (buttonRow.removeFromLeft (jmin (260, r.getWidth() / 3)));
    }

    int getNumRows() override
    {
        return project.getModules().getNumModules();
    }

    void paintRowBackground (Graphics& g, int /*rowNumber*/, int width, int height, bool rowIsSelected) override
    {
        g.setColour (rowIsSelected ? Colours::lightblue.withAlpha (0.4f)
                                   : Colours::white.withAlpha (0.4f));
        g.fillRect (0, 0, width, height - 1);
    }

    void paintCell (Graphics& g, int rowNumber, int columnId, int width, int height, bool /*rowIsSelected*/) override
    {
        String text;
        const String moduleID (project.getModules().getModuleID (rowNumber));

        if (columnId == nameCol)
        {
            text = moduleID;
        }
        else if (columnId == versionCol)
        {
            text = project.getModules().getModuleInfo (moduleID).getVersion();

            if (text.isEmpty())
                text = "?";
        }
        else if (columnId == updateCol)
        {
            if (listFromWebsite != nullptr)
            {
                if (const ModuleDescription* m = listFromWebsite->getModuleWithID (moduleID))
                {
                    if (m->getVersion() != project.getModules().getModuleInfo (moduleID).getVersion())
                        text = m->getVersion() +  " available";
                    else
                        text = "Up-to-date";
                }
                else
                    text = "?";
            }
            else
            {
                text = "-";
            }
        }
        else if (columnId == copyCol)
        {
            text = project.getModules().shouldCopyModuleFilesLocally (moduleID).getValue()
                        ? "Yes" : "No";
        }
        else if (columnId == pathCol)
        {
            StringArray paths;

            for (Project::ExporterIterator exporter (project); exporter.next();)
                paths.addIfNotAlreadyThere (exporter->getPathForModuleString (moduleID).trim());

            text = paths.joinIntoString (", ");
        }

        g.setColour (Colours::black);
        g.setFont (height * 0.65f);
        g.drawText (text, Rectangle<int> (width, height).reduced (4, 0), Justification::centredLeft, true);
    }

    void cellDoubleClicked (int rowNumber, int, const MouseEvent&) override
    {
        const String moduleID (project.getModules().getModuleID (rowNumber));

        if (moduleID.isNotEmpty())
            if (ProjectContentComponent* pcc = findParentComponentOfClass<ProjectContentComponent>())
                pcc->showModule (moduleID);
    }

    void deleteKeyPressed (int row) override
    {
        project.getModules().removeModule (project.getModules().getModuleID (row));
    }

    void webUpdateFinished (const ModuleList& newList)
    {
        listFromWebsite = new ModuleList (newList);

        table.updateContent();
        table.repaint();

        updateModuleButton.setEnabled (getUpdatableModules().size() != 0);
    }

    void buttonClicked (Button* b)
    {
        if (b == &addWebModuleButton)       showAddModuleMenu();
        else if (b == &updateModuleButton)  showUpdateModulesMenu();
        else if (b == &setCopyModeButton)   showCopyModeMenu();
        else if (b == &copyPathButton)      showSetPathsMenu();
    }

private:
    enum
    {
        nameCol = 1,
        versionCol,
        updateCol,
        copyCol,
        pathCol
    };

    Project& project;
    ValueTree modulesValueTree;
    TableListBox table;
    TextButton addWebModuleButton, updateModuleButton, setCopyModeButton, copyPathButton;
    ScopedPointer<ModuleList> listFromWebsite;

    void valueTreePropertyChanged (ValueTree&, const Identifier&) override    { itemChanged(); }
    void valueTreeChildAdded (ValueTree&, ValueTree&) override                { itemChanged(); }
    void valueTreeChildRemoved (ValueTree&, ValueTree&) override              { itemChanged(); }
    void valueTreeChildOrderChanged (ValueTree&) override                     { itemChanged(); }
    void valueTreeParentChanged (ValueTree&) override                         { itemChanged(); }

    void itemChanged()
    {
        table.updateContent();
        resized();
        repaint();
    }

    StringArray getUpdatableModules() const
    {
        StringArray result;

        if (listFromWebsite != nullptr)
        {
            for (int i = 0; i < listFromWebsite->modules.size(); ++i)
            {
                const ModuleDescription* m = listFromWebsite->modules.getUnchecked(i);
                const String v1 (m->getVersion());
                const String v2 (project.getModules().getModuleInfo (m->getID()).getVersion());

                if (v1 != v2 && v1.isNotEmpty() && v2.isNotEmpty())
                    result.add (m->getID());
            }
        }

        return result;
    }

    StringArray getAddableModules() const
    {
        StringArray result;

        if (listFromWebsite != nullptr)
        {
            for (int i = 0; i < listFromWebsite->modules.size(); ++i)
            {
                const ModuleDescription* m = listFromWebsite->modules.getUnchecked(i);

                if (! project.getModules().isModuleEnabled (m->getID()))
                    result.add (m->getID());
            }
        }

        return result;
    }

    void showUpdateModulesMenu()
    {
        StringArray mods (getUpdatableModules());

        PopupMenu m;
        m.addItem (1000, "Update all modules");
        m.addSeparator();

        for (int i = 0; i < mods.size(); ++i)
            m.addItem (1 + i, "Update " + mods[i]);

        int res = m.showAt (&updateModuleButton);

        if (res > 0 && listFromWebsite != nullptr)
        {
            if (res != 1000)
                mods = StringArray (mods[res - 1]);

            Array<ModuleDescription> modsToUpdate;

            for (int i = 0; i < mods.size(); ++i)
            {
                if (const ModuleDescription* md = listFromWebsite->getModuleWithID (mods[i]))
                {
                    ModuleDescription modToUpdate (*md);
                    modToUpdate.manifestFile = project.getModules().getModuleInfo (modToUpdate.getID()).manifestFile;
                    modsToUpdate.add (modToUpdate);
                }
            }

            DownloadAndInstallThread::updateModulesFromWeb (project, modsToUpdate);
        }
    }

    void showAddModuleMenu()
    {
        if (listFromWebsite == nullptr)
        {
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              "Couldn't contact the website!",
                                              "Failed to get the latest module list from juce.com - "
                                              "maybe network or server problems - try again soon!");
            return;
        }

        StringArray mods (getAddableModules());

        if (mods.size() == 0)
        {
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              "No modules to add!",
                                              "Couldn't find any new modules that aren't already in your project!");
            return;
        }

        PopupMenu m;

        for (int i = 0; i < mods.size(); ++i)
            m.addItem (i + 1, "Install " + mods[i]);

        int res = m.showAt (&addWebModuleButton);

        if (res > 0 && listFromWebsite != nullptr)
            if (const ModuleDescription* md = listFromWebsite->getModuleWithID (mods[res - 1]))
                DownloadAndInstallThread::addModuleFromWebsite (project, *md);
    }

    void showCopyModeMenu()
    {
        PopupMenu m;
        m.addItem (1, "Set all modules to copy locally");
        m.addItem (2, "Set all modules to not copy locally");

        int res = m.showAt (&setCopyModeButton);

        if (res != 0)
            project.getModules().setLocalCopyModeForAllModules (res == 1);
    }

    void showSetPathsMenu()
    {
        EnabledModuleList& moduleList = project.getModules();

        const String moduleToCopy (moduleList.getModuleID (table.getSelectedRow()));

        if (moduleToCopy.isNotEmpty())
        {
            PopupMenu m;
            m.addItem (1, "Copy the paths from the module '" + moduleToCopy + "' to all other modules");

            int res = m.showAt (&copyPathButton);

            if (res != 0)
            {
                for (Project::ExporterIterator exporter (project); exporter.next();)
                {
                    for (int i = 0; i < moduleList.getNumModules(); ++i)
                    {
                        String modID = moduleList.getModuleID (i);

                        if (modID != moduleToCopy)
                            exporter->getPathForModuleValue (modID) = exporter->getPathForModuleValue (moduleToCopy).getValue();
                    }
                }
            }

            table.repaint();
        }
        else
        {
            PopupMenu m;
            m.addItem (1, "Copy the paths from the selected module to all other modules", false);

            m.showAt (&copyPathButton);
        }
    }

    struct WebsiteUpdateFetchThread  : private Thread,
                                       private AsyncUpdater
    {
        WebsiteUpdateFetchThread (ModulesPanel& p)  : Thread ("Web Updater"), panel (p)
        {
            startThread (3);
        }

        ~WebsiteUpdateFetchThread()
        {
            stopThread (15000);
        }

        void run() override
        {
            static Time lastDownloadTime;
            static ModuleList lastList;

            if (Time::getCurrentTime() < lastDownloadTime + RelativeTime::minutes (2.0))
            {
                list = lastList;
                triggerAsyncUpdate();
            }
            else
            {
                if (list.loadFromWebsite() && ! threadShouldExit())
                {
                    lastList = list;
                    lastDownloadTime = Time::getCurrentTime();
                    triggerAsyncUpdate();
                }
            }
        }

        void handleAsyncUpdate() override
        {
            panel.webUpdateFinished (list);
        }

    private:
        ModuleList list;
        ModulesPanel& panel;
    };

    ScopedPointer<WebsiteUpdateFetchThread> webUpdateThread;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModulesPanel)
};

//==============================================================================
class DownloadAndInstallThread   : public ThreadWithProgressWindow
{
public:
    DownloadAndInstallThread (const Array<ModuleDescription>& modulesToInstall)
        : ThreadWithProgressWindow ("Installing New Modules", true, true),
          result (Result::ok()),
          modules (modulesToInstall)
    {
    }

    static void updateModulesFromWeb (Project& project, const Array<ModuleDescription>& mods)
    {
        DownloadAndInstallThread d (mods);

        if (d.runThread())
        {
            if (d.result.failed())
            {
                AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                                  "Module Install Failed",
                                                  d.result.getErrorMessage());
            }
            else
            {
                for (int i = 0; i < d.modules.size(); ++i)
                    project.getModules().addModule (d.modules.getReference(i).manifestFile,
                                                    project.getModules().areMostModulesCopiedLocally());
            }
        }
    }

    static void addModuleFromWebsite (Project& project, const ModuleDescription& module)
    {
        Array<ModuleDescription> mods;
        mods.add (module);

        static File lastLocation (EnabledModuleList::findDefaultModulesFolder (project));

        FileChooser fc ("Select the parent folder for the new module...", lastLocation, String::empty, false);

        if (fc.browseForDirectory())
        {
            lastLocation = fc.getResult();

            if (lastLocation.getChildFile (ModuleDescription::getManifestFileName()).exists())
            {
                AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                                  "Adding Module",
                                                  "You chose a folder that appears to be a module.\n\n"
                                                  "You need to select the *parent* folder inside which the new modules will be created.");
                return;
            }

            for (int i = 0; i < mods.size(); ++i)
                mods.getReference(i).manifestFile = lastLocation.getChildFile (mods.getReference(i).getID())
                                                                .getChildFile (ModuleDescription::getManifestFileName());

            updateModulesFromWeb (project, mods);
        }
    }

    void run() override
    {
        for (int i = 0; i < modules.size(); ++i)
        {
            const ModuleDescription& m = modules.getReference(i);

            setProgress (i / (double) modules.size());

            MemoryBlock downloaded;
            result = download (m, downloaded);

            if (result.failed() || threadShouldExit())
                break;

            result = unzip (m, downloaded);

            if (result.failed() || threadShouldExit())
                break;
        }
    }

    Result download (const ModuleDescription& m, MemoryBlock& dest)
    {
        setStatusMessage ("Downloading " + m.getID() + "...");

        const ScopedPointer<InputStream> in (m.url.createInputStream (false, nullptr, nullptr, String::empty, 10000));

        if (in != nullptr && in->readIntoMemoryBlock (dest))
            return Result::ok();

        return Result::fail ("Failed to download from: " + m.url.toString (false));
    }

    Result unzip (const ModuleDescription& m, const MemoryBlock& data)
    {
        setStatusMessage ("Installing " + m.getID() + "...");

        MemoryInputStream input (data, false);
        ZipFile zip (input);

        if (zip.getNumEntries() == 0)
            return Result::fail ("The downloaded file wasn't a valid module file!");

        if (! m.getFolder().deleteRecursively())
            return Result::fail ("Couldn't delete the existing folder:\n" + m.getFolder().getFullPathName());

        return zip.uncompressTo (m.getFolder().getParentDirectory(), true);
    }

    Result result;
    Array<ModuleDescription> modules;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DownloadAndInstallThread)
};
