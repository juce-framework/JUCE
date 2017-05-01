/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

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
    bool isMissing() const override           { return hasMissingDependencies(); }

    void showDocument() override
    {
        showSettingsPage (new ModuleSettingsPanel (project, moduleID));
    }

    void deleteItem() override                { project.getModules().removeModule (moduleID); }

    Icon getIcon() const override
    {
        auto iconColour = getOwnerView()->findColour (isSelected() ? defaultHighlightedTextColourId
                                                                   : treeIconColourId);

        if (! isSelected())
        {
            auto info = project.getModules().getModuleInfo (moduleID);
            if (info.isValid() && info.getVendor() == "juce")
            {
                if (info.getLicense() == "ISC")
                    iconColour = Colours::lightblue;
                else if (info.getLicense() == "GPL/Commercial")
                    iconColour = Colours::orange;
            }
        }

        return Icon (getIcons().singleModule, iconColour);
    }

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
            : group (p.getModules().getModuleInfo (modID).getID(), Icon (getIcons().singleModule, Colours::transparentBlack)),
              project (p),
              moduleID (modID)
        {
            addAndMakeVisible (group);
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

        void resized() override { group.setBounds (getLocalBounds().withTrimmedLeft (12)); }

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
                auto bounds = getLocalBounds().reduced (10);
                bounds.removeFromTop (5);

                if (info.isValid())
                {
                    auto topSlice = bounds.removeFromTop (bounds.getHeight() / 3);
                    bounds.removeFromTop (bounds.getHeight() / 6);
                    auto bottomSlice = bounds;

                    g.setColour (findColour (defaultTextColourId));

                    g.drawFittedText (info.getName(),                   topSlice.removeFromTop (topSlice.getHeight() / 3), Justification::centredLeft, 1);
                    g.drawFittedText ("Version: "  + info.getVersion(), topSlice.removeFromTop (topSlice.getHeight() / 2), Justification::centredLeft, 1);
                    g.drawFittedText ("License: " + info.getLicense(),  topSlice.removeFromTop (topSlice.getHeight()),     Justification::centredLeft, 1);

                    g.drawFittedText (info.getDescription(), bottomSlice, Justification::topLeft, 3, 1.0f);
                }
                else
                {
                    g.setColour (Colours::red);
                    g.drawFittedText ("Cannot find this module at the specified path!", bounds, Justification::centred, 1);
                }
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
                String text ("This module has missing dependencies!\n\n"
                             "To build correctly, it requires the following modules to be added:\n");
                text << missingDependencies.joinIntoString (", ");

                g.setColour (Colours::red);
                g.drawFittedText (text, getLocalBounds().reduced (4, 16), Justification::topLeft, 3);
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
    bool isMissing() const override         { return false; }
    Icon getIcon() const override           { return Icon (getIcons().graph, getContentColour (true)); }

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
