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
    bool hasWarnings() const override         { return hasHigherCppStandardThanProject(); }

    void showDocument() override
    {
        showSettingsPage (new ModuleSettingsPanel (project, moduleID, getOwnerView()));
    }

    void deleteItem() override
    {
        closeSettingsPage();
        project.getModules().removeModule (moduleID);
    }

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

    bool hasHigherCppStandardThanProject() const
    {
        return project.getModules().doesModuleHaveHigherCppStandardThanProject (moduleID);
    }

    //==============================================================================
    class ModuleSettingsPanel  : public Component,
                                 private Value::Listener
    {
    public:
        ModuleSettingsPanel (Project& p, const String& modID, TreeView* tree)
            : group (p.getModules().getModuleInfo (modID).getID(),
                     Icon (getIcons().singleModule, Colours::transparentBlack)),
              project (p),
              modulesTree (tree),
              moduleID (modID)
        {
            defaultJuceModulePathValue.referTo (getAppSettings().getStoredPath (Ids::defaultJuceModulePath));
            defaultUserModulePathValue.referTo (getAppSettings().getStoredPath (Ids::defaultUserModulePath));

            defaultJuceModulePathValue.addListener (this);
            defaultUserModulePathValue.addListener (this);

            addAndMakeVisible (group);
            refresh();
        }

        void refresh()
        {
            auto& modules = project.getModules();

            setEnabled (modules.isModuleEnabled (moduleID));

            PropertyListBuilder props;

            props.add (new ModuleInfoComponent (project, moduleID));

            if (modules.getExtraDependenciesNeeded (moduleID).size() > 0)
                props.add (new MissingDependenciesComponent (project, moduleID));

            if (modules.doesModuleHaveHigherCppStandardThanProject (moduleID))
                props.add (new CppStandardWarningComponent());

            modulePathValueSources.clear();

            for (Project::ExporterIterator exporter (project); exporter.next();)
            {
                auto key = modules.isJuceModule (moduleID) ? Ids::defaultJuceModulePath
                                                           : Ids::defaultUserModulePath;

                Value src (modulePathValueSources.add (new DependencyPathValueSource (exporter->getPathForModuleValue (moduleID),
                                                                                      key, exporter->getTargetOSForExporter())));

                auto* pathComponent = new DependencyFilePathPropertyComponent (src, "Path for " + exporter->getName().quoted(),
                                                                               true, "*", project.getProjectFolder());

                props.add (pathComponent,
                           "A path to the folder that contains the " + moduleID + " module when compiling the "
                           + exporter->getName().quoted() + " target. "
                           "This can be an absolute path, or relative to the jucer project folder, but it "
                           "must be valid on the filesystem of the target machine that will be performing this build. If this "
                           "is empty then the global path will be used.");

                pathComponent->setEnabled (! modules.shouldUseGlobalPath (moduleID));
                pathComponent->getValue().addListener (this);
            }

            globalPathValue.referTo (modules.getShouldUseGlobalPathValue (moduleID));

            auto menuItemString = (TargetOS::getThisOS() == TargetOS::osx ? "\"Projucer->Global Search Paths...\""
                                                                          : "\"File->Global Search Paths...\"");

            props.add (new BooleanPropertyComponent (globalPathValue,
                                                     "Use global path", "Use global path for this module"),
                       String ("If this is enabled, then the locally-stored global path (set in the ") + menuItemString + " menu item) "
                       "will be used as the path to this module. "
                       "This means that if this Projucer project is opened on another machine it will use that machine's global path as the path to this module.");
            globalPathValue.addListener (this);

            props.add (new BooleanPropertyComponent (modules.shouldCopyModuleFilesLocally (moduleID),
                                                     "Create local copy", "Copy the module into the project folder"),
                       "If this is enabled, then a local copy of the entire module will be made inside your project (in the auto-generated JuceLibraryFiles folder), "
                       "so that your project will be self-contained, and won't need to contain any references to files in other folders. "
                       "This also means that you can check the module into your source-control system to make sure it is always in sync with your own code.");

            props.add (new BooleanPropertyComponent (modules.shouldShowAllModuleFilesInProject (moduleID),
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

            ModuleDescription info (modules.getModuleInfo (moduleID));

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
        SafePointer<TreeView> modulesTree;
        String moduleID;
        Value globalPathValue;
        Value defaultJuceModulePathValue, defaultUserModulePathValue;

        ReferenceCountedArray<Value::ValueSource> modulePathValueSources;

        //==============================================================================
        void valueChanged (Value& v) override
        {
            if (v == globalPathValue)
            {
                auto useGlobalPath =  globalPathValue.getValue();

                for (auto prop : group.properties)
                {
                    if (auto* pathPropertyComponent = dynamic_cast<DependencyFilePathPropertyComponent*> (prop))
                        pathPropertyComponent->setEnabled (! useGlobalPath);
                }
            }

            if (auto* moduleInfo = dynamic_cast<ModuleInfoComponent*> (group.properties.getUnchecked (0)))
                moduleInfo->refresh();
        }

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

            void refresh() override
            {
                info = project.getModules().getModuleInfo (moduleID);
                repaint();
            }

        private:
            void paint (Graphics& g) override
            {
                auto bounds = getLocalBounds().reduced (10);
                bounds.removeFromTop (5);

                if (info.isValid())
                {
                    auto topSlice = bounds.removeFromTop (bounds.getHeight() / 2);
                    bounds.removeFromTop (bounds.getHeight() / 6);
                    auto bottomSlice = bounds;

                    g.setColour (findColour (defaultTextColourId));

                    g.drawFittedText (info.getName(),                   topSlice.removeFromTop (topSlice.getHeight() / 4), Justification::centredLeft, 1);
                    g.drawFittedText ("Version: "  + info.getVersion(), topSlice.removeFromTop (topSlice.getHeight() / 3), Justification::centredLeft, 1);
                    g.drawFittedText ("License: "  + info.getLicense(), topSlice.removeFromTop (topSlice.getHeight() / 2), Justification::centredLeft, 1);
                    g.drawFittedText ("Location: " + info.getFolder().getParentDirectory().getFullPathName(),
                                      topSlice.removeFromTop (topSlice.getHeight()), Justification::centredLeft, 1);

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
                                              public Button::Listener
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
                g.drawFittedText (text, getLocalBounds().reduced (10), Justification::topLeft, 3);
            }

            void buttonClicked (Button*) override
            {
                ModuleList list;

                list.scanGlobalJuceModulePath();

                if (! tryToFix (list))
                {
                    list.scanGlobalUserModulePath();

                    if (! tryToFix (list))
                    {
                        list.scanProjectExporterModulePaths (project);

                        if (! tryToFix (list))
                        {
                            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                                              "Adding Missing Dependencies",
                                                              "Couldn't locate some of these modules - you'll need to find their "
                                                              "folders manually and add them to the list.");

                            return;
                        }
                    }
                }

                refreshAndReselectItem();
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

            bool tryToFix (ModuleList& list)
            {
                auto& modules      = project.getModules();
                auto copyLocally   = modules.areMostModulesCopiedLocally();
                auto useGlobalPath = modules.areMostModulesUsingGlobalPath();

                StringArray missing;

                for (auto missingModule : missingDependencies)
                {
                    if (auto* info = list.getModuleWithID (missingModule))
                        modules.addModule (info->moduleFolder, copyLocally, useGlobalPath);
                    else
                        missing.add (missingModule);
                }

                missingDependencies.swapWith (missing);
                return (missingDependencies.size() == 0);
            }

            void refreshAndReselectItem()
            {
                if (auto* settingsPanel = findParentComponentOfClass<ModuleSettingsPanel>())
                {
                    if (settingsPanel->modulesTree == nullptr)
                        return;

                    auto* rootItem = settingsPanel->modulesTree->getRootItem();

                    if (rootItem == nullptr)
                        return;

                    for (auto i = 0; i < rootItem->getNumSubItems(); ++i)
                    {
                        if (auto* subItem = dynamic_cast<ConfigTreeItemBase*> (rootItem->getSubItem (i)))
                        {
                            if (subItem->getDisplayName() == moduleID)
                            {
                                subItem->setSelected (true, true);
                                return;
                            }
                        }
                    }
                }
            }

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MissingDependenciesComponent)
        };

        //==============================================================================
        struct CppStandardWarningComponent    : public PropertyComponent
        {
            CppStandardWarningComponent()
                : PropertyComponent ("CppStandard", 100)
            {
            }

            void refresh() override {}

            void paint (Graphics& g) override
            {
                auto text = String ("This module has a higher C++ language standard requirement than your project!\n\n"
                                    "To use this module you need to increase the C++ standard of the project.\n");

                g.setColour (findColour (defaultHighlightColourId));
                g.drawFittedText (text, getLocalBounds().reduced (10), Justification::topLeft, 3);
            }

            StringArray configsToWarnAbout;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CppStandardWarningComponent)
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
                                            project.getModules().areMostModulesCopiedLocally(),
                                            project.getModules().areMostModulesUsingGlobalPath());
    }

    void addSubItems() override
    {
        for (int i = 0; i < project.getModules().getNumModules(); ++i)
            addSubItem (new ModuleItem (project, project.getModules().getModuleID (i)));
    }

    void showPopupMenu() override
    {
        auto& modules = project.getModules();
        PopupMenu knownModules, jucePathModules, userPathModules, exporterPathsModules;

        auto index = 100;
        for (auto m : getAvailableModulesInGlobalJucePath())
            jucePathModules.addItem (index++, m, ! modules.isModuleEnabled (m));

        knownModules.addSubMenu ("Global JUCE modules path", jucePathModules);

        index = 200;
        for (auto m : getAvailableModulesInGlobalUserPath())
            userPathModules.addItem (index++, m, ! modules.isModuleEnabled (m));

        knownModules.addSubMenu ("Global user modules path", userPathModules);

        index = 300;
        for (auto m : getAvailableModulesInExporterPaths())
            exporterPathsModules.addItem (index++, m, ! modules.isModuleEnabled (m));

        knownModules.addSubMenu ("Exporter paths", exporterPathsModules);

        PopupMenu menu;
        menu.addSubMenu ("Add a module", knownModules);
        menu.addSeparator();
        menu.addItem (1001, "Add a module from a specified folder...");

        launchPopupMenu (menu);
    }

    void handlePopupMenuResult (int resultCode) override
    {
        auto& modules = project.getModules();

        if (resultCode == 1001)
        {
            modules.addModuleFromUserSelectedFile();
        }
        else if (resultCode > 0)
        {
            if (resultCode < 200)
                modules.addModuleInteractive (getAvailableModulesInGlobalJucePath() [resultCode - 100]);
            else if (resultCode < 300)
                modules.addModuleInteractive (getAvailableModulesInGlobalUserPath() [resultCode - 200]);
            else if (resultCode < 400)
                modules.addModuleInteractive (getAvailableModulesInExporterPaths() [resultCode - 300]);
        }
    }

    StringArray getAvailableModulesInGlobalJucePath()
    {
        ModuleList list;
        list.addAllModulesInFolder ({ getAppSettings().getStoredPath (Ids::defaultJuceModulePath).toString() });

        return list.getIDs();
    }

    StringArray getAvailableModulesInGlobalUserPath()
    {
        ModuleList list;
        auto paths = StringArray::fromTokens (getAppSettings().getStoredPath (Ids::defaultUserModulePath).toString(), ";", {});

        for (auto p : paths)
        {
            auto f = File::createFileWithoutCheckingPath (p.trim());
            if (f.exists())
                list.addAllModulesInFolder (f);
        }

        auto ids = list.getIDs();

        for (auto m : getAvailableModulesInGlobalJucePath())
            ids.removeString (m);

        return ids;
    }

    StringArray getAvailableModulesInExporterPaths()
    {
        ModuleList list;
        list.scanProjectExporterModulePaths (project);

        auto ids = list.getIDs();

        for (auto m : getAvailableModulesInGlobalJucePath())
            ids.removeString (m);

        for (auto m : getAvailableModulesInGlobalUserPath())
            ids.removeString (m);

        return ids;
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
