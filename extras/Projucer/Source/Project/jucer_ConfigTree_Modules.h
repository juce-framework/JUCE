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

class ModuleItem   : public ConfigTreeItemBase
{
public:
    ModuleItem (Project& p, const String& modID)
        : project (p), moduleID (modID)
    {
    }

    bool canBeSelected() const override       { return true; }
    bool mightContainSubItems() override      { return false; }
    String getUniqueName() const override     { return "module_" + moduleID; }
    String getDisplayName() const override    { return moduleID; }
    String getRenamingName() const override   { return getDisplayName(); }
    void setName (const String&) override     {}
    bool isMissing() override                 { return hasMissingDependencies(); }
    Icon getIcon() const override             { return Icon (getIcons().jigsaw, getContrastingColour (Colours::red, 0.5f)); }
    void showDocument() override              { showSettingsPage (new ModuleSettingsPanel (project, moduleID)); }
    void deleteItem() override                { project.getModules().removeModule (moduleID); }

    void showPopupMenu() override
    {
        PopupMenu menu;
        menu.addItem (1, "Remove this module");
        launchPopupMenu (menu);
    }

    void handlePopupMenuResult (int resultCode) override
    {
        if (resultCode == 1)
            deleteItem();
    }

    Project& project;
    String moduleID;

private:
    bool hasMissingDependencies() const
    {
        return project.getModules().getExtraDependenciesNeeded (moduleID).size() > 0;
    }

    //==============================================================================
    class ModuleSettingsPanel  : public Component
    {
    public:
        ModuleSettingsPanel (Project& p, const String& modID)
            : project (p), moduleID (modID)
        {
            addAndMakeVisible (group);
            group.setName ("Module: " + moduleID);
            refresh();
        }

        void refresh()
        {
            setEnabled (project.getModules().isModuleEnabled (moduleID));

            PropertyListBuilder props;

            props.add (new ModuleInfoComponent (project, moduleID));

            if (project.getModules().getExtraDependenciesNeeded (moduleID).size() > 0)
                props.add (new MissingDependenciesComponent (project, moduleID));

            for (Project::ExporterIterator exporter (project); exporter.next();)
                props.add (new FilePathPropertyComponent (exporter->getPathForModuleValue (moduleID),
                                                          "Path for " + exporter->getName().quoted(),
                                                          true, "*", project.getProjectFolder()),
                           "A path to the folder that contains the " + moduleID + " module when compiling the "
                            + exporter->getName().quoted() + " target. "
                           "This can be an absolute path, or relative to the jucer project folder, but it "
                           "must be valid on the filesystem of the target machine that will be performing this build.");

            props.add (new BooleanPropertyComponent (project.getModules().shouldCopyModuleFilesLocally (moduleID),
                                                     "Create local copy", "Copy the module into the project folder"),
                       "If this is enabled, then a local copy of the entire module will be made inside your project (in the auto-generated JuceLibraryFiles folder), "
                       "so that your project will be self-contained, and won't need to contain any references to files in other folders. "
                       "This also means that you can check the module into your source-control system to make sure it is always in sync with your own code.");

            props.add (new BooleanPropertyComponent (project.getModules().shouldShowAllModuleFilesInProject (moduleID),
                                                     "Add source to project", "Make module files browsable in projects"),
                       "If this is enabled, then the entire source tree from this module will be shown inside your project, "
                       "making it easy to browse/edit the module's classes. If disabled, then only the minimum number of files "
                       "required to compile it will appear inside your project.");

            StringArray possibleValues;
            possibleValues.add ("(Use Default)");
            possibleValues.add ("Enabled");
            possibleValues.add ("Disabled");

            Array<var> mappings;
            mappings.add (Project::configFlagDefault);
            mappings.add (Project::configFlagEnabled);
            mappings.add (Project::configFlagDisabled);

            ModuleDescription info (project.getModules().getModuleInfo (moduleID));

            if (info.isValid())
            {
                OwnedArray <Project::ConfigFlag> configFlags;
                LibraryModule (info).getConfigFlags (project, configFlags);

                for (int i = 0; i < configFlags.size(); ++i)
                {
                    ChoicePropertyComponent* c = new ChoicePropertyComponent (configFlags[i]->value,
                                                                              configFlags[i]->symbol,
                                                                              possibleValues, mappings);
                    c->setTooltip (configFlags[i]->description);
                    props.add (c);
                }
            }

            group.setProperties (props);
            parentSizeChanged();
        }

        void parentSizeChanged() override  { updateSize (*this, group); }

    private:
        PropertyGroupComponent group;
        Project& project;
        String moduleID;

        //==============================================================================
        class ModuleInfoComponent  : public PropertyComponent,
                                     private Value::Listener
        {
        public:
            ModuleInfoComponent (Project& p, const String& modID)
                : PropertyComponent ("Module", 150), project (p), moduleID (modID)
            {
                for (Project::ExporterIterator exporter (project); exporter.next();)
                    listeningValues.add (new Value (exporter->getPathForModuleValue (moduleID)))
                        ->addListener (this);

                refresh();
            }

        private:
            void refresh() override
            {
                info = project.getModules().getModuleInfo (moduleID);
                repaint();
            }

            void paint (Graphics& g) override
            {
                g.setColour (Colours::white.withAlpha (0.4f));
                g.fillRect (getLocalBounds().withTrimmedBottom (1));

                AttributedString s;
                s.setJustification (Justification::topLeft);

                Font f (14.0f);

                if (info.isValid())
                {
                    s.append (info.getName() + "\n\n", f.boldened());
                    s.append ("Version: "  + info.getVersion()
                                + "\nLicense: " + info.getLicense() + "\n", f.italicised());
                    s.append ("\n" + info.getDescription(), f);
                }
                else
                {
                    s.append ("Cannot find this module at the specified path!", f.boldened());
                    s.setColour (Colours::darkred);
                }

                s.draw (g, getLocalBounds().reduced (6, 5).toFloat());
            }

            void valueChanged (Value&) override
            {
                refresh();
            }

            Project& project;
            String moduleID;
            OwnedArray<Value> listeningValues;
            ModuleDescription info;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModuleInfoComponent)
        };

        //==============================================================================
        class MissingDependenciesComponent  : public PropertyComponent,
                                              public ButtonListener
        {
        public:
            MissingDependenciesComponent (Project& p, const String& modID)
                : PropertyComponent ("Dependencies", 100),
                  project (p), moduleID (modID),
                  missingDependencies (project.getModules().getExtraDependenciesNeeded (modID)),
                  fixButton ("Add Required Modules")
            {
                addAndMakeVisible (fixButton);
                fixButton.setColour (TextButton::buttonColourId, Colours::red);
                fixButton.setColour (TextButton::textColourOffId, Colours::white);
                fixButton.addListener (this);
            }

            void refresh() override {}

            void paint (Graphics& g) override
            {
                g.setColour (Colours::white.withAlpha (0.4f));
                g.fillRect (0, 0, getWidth(), getHeight() - 1);

                String text ("This module has missing dependencies!\n\n"
                             "To build correctly, it requires the following modules to be added:\n");
                text << missingDependencies.joinIntoString (", ");

                AttributedString s;
                s.setJustification (Justification::topLeft);
                s.append (text, Font (13.0f), Colours::red.darker());
                s.draw (g, getLocalBounds().reduced (4, 16).toFloat());
            }

            void buttonClicked (Button*) override
            {
                bool anyFailed = false;

                ModuleList list;
                list.scanAllKnownFolders (project);

                for (int i = missingDependencies.size(); --i >= 0;)
                {
                    if (const ModuleDescription* info = list.getModuleWithID (missingDependencies[i]))
                        project.getModules().addModule (info->moduleFolder, project.getModules().areMostModulesCopiedLocally());
                    else
                        anyFailed = true;
                }

                if (ModuleSettingsPanel* p = findParentComponentOfClass<ModuleSettingsPanel>())
                    p->refresh();

                if (anyFailed)
                    AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                                      "Adding Missing Dependencies",
                                                      "Couldn't locate some of these modules - you'll need to find their "
                                                      "folders manually and add them to the list.");
            }

            void resized() override
            {
                fixButton.setBounds (getWidth() - 168, getHeight() - 26, 160, 22);
            }

        private:
            Project& project;
            String moduleID;
            StringArray missingDependencies;
            TextButton fixButton;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MissingDependenciesComponent)
        };
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModuleItem)
};

//==============================================================================
class EnabledModulesItem   : public ConfigTreeItemBase
{
public:
    EnabledModulesItem (Project& p)
        : project (p),
          moduleListTree (p.getModules().state)
    {
        moduleListTree.addListener (this);
    }

    int getItemHeight() const override      { return 22; }
    bool isModulesList() const override     { return true; }
    bool canBeSelected() const override     { return true; }
    bool mightContainSubItems() override    { return true; }
    String getUniqueName() const override   { return "modules"; }
    String getRenamingName() const override { return getDisplayName(); }
    String getDisplayName() const override  { return "Modules"; }
    void setName (const String&) override   {}
    bool isMissing() override               { return false; }
    Icon getIcon() const override           { return Icon (getIcons().graph, getContrastingColour (Colours::red, 0.5f)); }

    void showDocument() override
    {
        if (ProjectContentComponent* pcc = getProjectContentComponent())
            pcc->setEditorComponent (new ModulesPanel (project), nullptr);
    }

    static File getModuleFolder (const File& draggedFile)
    {
        if (draggedFile.hasFileExtension (headerFileExtensions))
            return draggedFile.getParentDirectory();

        return draggedFile;
    }

    bool isInterestedInFileDrag (const StringArray& files) override
    {
        for (int i = files.size(); --i >= 0;)
            if (ModuleDescription (getModuleFolder (files[i])).isValid())
                return true;

        return false;
    }

    void filesDropped (const StringArray& files, int /*insertIndex*/) override
    {
        Array<ModuleDescription> modules;

        for (int i = files.size(); --i >= 0;)
        {
            ModuleDescription m (getModuleFolder (files[i]));

            if (m.isValid())
                modules.add (m);
        }

        for (int i = 0; i < modules.size(); ++i)
            project.getModules().addModule (modules.getReference(i).moduleFolder,
                                            project.getModules().areMostModulesCopiedLocally());
    }

    void addSubItems() override
    {
        for (int i = 0; i < project.getModules().getNumModules(); ++i)
            addSubItem (new ModuleItem (project, project.getModules().getModuleID (i)));
    }

    void showPopupMenu() override
    {
        PopupMenu menu, knownModules, copyModeMenu;

        const StringArray modules (getAvailableModules());
        for (int i = 0; i < modules.size(); ++i)
            knownModules.addItem (1 + i, modules[i], ! project.getModules().isModuleEnabled (modules[i]));

        menu.addSubMenu ("Add a module", knownModules);
        menu.addSeparator();
        menu.addItem (1001, "Add a module from a specified folder...");

        launchPopupMenu (menu);
    }

    void handlePopupMenuResult (int resultCode) override
    {
        if (resultCode == 1001)
            project.getModules().addModuleFromUserSelectedFile();
        else if (resultCode > 0)
            project.getModules().addModuleInteractive (getAvailableModules() [resultCode - 1]);
    }

    StringArray getAvailableModules()
    {
        ModuleList list;
        list.scanAllKnownFolders (project);
        return list.getIDs();
    }

    //==============================================================================
    void valueTreeChildAdded (ValueTree& parentTree, ValueTree&) override         { refreshIfNeeded (parentTree); }
    void valueTreeChildRemoved (ValueTree& parentTree, ValueTree&, int) override  { refreshIfNeeded (parentTree); }
    void valueTreeChildOrderChanged (ValueTree& parentTree, int, int) override    { refreshIfNeeded (parentTree); }

    void refreshIfNeeded (ValueTree& changedTree)
    {
        if (changedTree == moduleListTree)
            refreshSubItems();
    }

private:
    Project& project;
    ValueTree moduleListTree;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnabledModulesItem)
};
