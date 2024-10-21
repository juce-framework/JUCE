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

#pragma once


//==============================================================================
class ModuleItem final : public ProjectTreeItemBase
{
public:
    ModuleItem (Project& p, const String& modID)
        : project (p), moduleID (modID)
    {
        missingDependencies = project.getEnabledModules().getExtraDependenciesNeeded (moduleID).size() > 0;
        cppStandardHigherThanProject = project.getEnabledModules().doesModuleHaveHigherCppStandardThanProject (moduleID);

        moduleInfo = project.getEnabledModules().getModuleInfo (moduleID);
    }

    bool canBeSelected() const override       { return true; }
    bool mightContainSubItems() override      { return false; }
    String getUniqueName() const override     { return "module_" + moduleID; }
    String getDisplayName() const override    { return moduleID; }
    String getRenamingName() const override   { return getDisplayName(); }
    void setName (const String&) override     {}
    bool isMissing() const override           { return missingDependencies; }
    bool hasWarnings() const override         { return cppStandardHigherThanProject; }

    void showDocument() override
    {
        showSettingsPage (new ModuleSettingsPanel (project, moduleID, getOwnerView()));
    }

    void deleteItem() override
    {
        closeSettingsPage();
        project.getEnabledModules().removeModule (moduleID);
    }

    Icon getIcon() const override
    {
        auto iconColour = getOwnerView()->findColour (isSelected() ? defaultHighlightedTextColourId
                                                                   : treeIconColourId);

        if (! isSelected())
        {
            if (moduleInfo.isValid() && moduleInfo.getVendor() == "juce")
            {
                if (moduleInfo.getLicense() == "ISC")
                    iconColour = Colours::lightblue;
                else if (moduleInfo.getLicense() == "AGPLv3/Commercial")
                    iconColour = Colours::orange;
            }
        }

        return Icon (getIcons().singleModule, iconColour);
    }

    void showAddMenu (Point<int> p) override
    {
        if (auto* parent = dynamic_cast<EnabledModulesItem*> (getParentItem()))
            parent->showPopupMenu (p);
    }

    void showPopupMenu (Point<int> p) override
    {
        PopupMenu menu;
        menu.addItem (1, "Remove this module");
        launchPopupMenu (menu, p);
    }

    void handlePopupMenuResult (int resultCode) override
    {
        if (resultCode == 1)
            deleteItem();
    }

    bool checkCppStandard()
    {
        auto oldVal = cppStandardHigherThanProject;
        cppStandardHigherThanProject = project.getEnabledModules().doesModuleHaveHigherCppStandardThanProject (moduleID);

        if (oldVal != cppStandardHigherThanProject)
            return true;

        return false;
    }

    Project& project;
    String moduleID;

private:
    ModuleDescription moduleInfo;
    bool missingDependencies = false;
    bool cppStandardHigherThanProject = false;

    //==============================================================================
    class ModuleSettingsPanel final : public Component,
                                      private ValueTree::Listener,
                                      private Value::Listener
    {
    public:
        ModuleSettingsPanel (Project& p, const String& modID, TreeView* tree)
            : group (p.getEnabledModules().getModuleInfo (modID).getID(),
                     Icon (getIcons().singleModule, Colours::transparentBlack)),
              project (p),
              modulesTree (tree),
              moduleID (modID)
        {
            auto& appSettings = getAppSettings();
            appSettings.addProjectDefaultsListener (*this);
            appSettings.addFallbackPathsListener (*this);

            addAndMakeVisible (group);
            refresh();
        }

        ~ModuleSettingsPanel() override
        {
            auto& appSettings = getAppSettings();
            appSettings.removeProjectDefaultsListener (*this);
            appSettings.removeFallbackPathsListener (*this);
        }

        void refresh()
        {
            auto& modules = project.getEnabledModules();

            setEnabled (modules.isModuleEnabled (moduleID));

            PropertyListBuilder props;

            props.add (new ModuleInfoComponent (project, moduleID));

            if (modules.getExtraDependenciesNeeded (moduleID).size() > 0)
                props.add (new MissingDependenciesComponent (project, moduleID));

            if (modules.doesModuleHaveHigherCppStandardThanProject (moduleID))
                props.add (new CppStandardWarningComponent());

            group.clearProperties();
            exporterModulePathValues.clear();

            for (Project::ExporterIterator exporter (project); exporter.next();)
            {
                auto modulePathValue = exporter->getPathForModuleValue (moduleID);
                const auto fallbackPath = getAppSettings().getStoredPath (isJUCEModule (moduleID) ? Ids::defaultJuceModulePath
                                                                                                  : Ids::defaultUserModulePath,
                                                                          exporter->getTargetOSForExporter()).get().toString();

                modulePathValue.setDefault (fallbackPath);
                exporterModulePathValues.add (modulePathValue.getPropertyAsValue());
                exporterModulePathValues.getReference (exporterModulePathValues.size() - 1).addListener (this);

                auto pathComponent = std::make_unique<FilePathPropertyComponent> (modulePathValue,
                                                                                  "Path for " + exporter->getUniqueName().quoted(),
                                                                                  true,
                                                                                  exporter->getTargetOSForExporter() == TargetOS::getThisOS(),
                                                                                  "*",
                                                                                  project.getProjectFolder());

                pathComponent->setEnabled (! modules.shouldUseGlobalPath (moduleID));

                props.add (pathComponent.release(),
                           "A path to the folder that contains the " + moduleID + " module when compiling the "
                           + exporter->getUniqueName().quoted() + " target. "
                           "This can be an absolute path, or relative to the jucer project folder, but it "
                           "must be valid on the filesystem of the target machine that will be performing this build. If this "
                           "is empty then the global path will be used.");
            }

            useGlobalPathValue = modules.shouldUseGlobalPathValue (moduleID);
            useGlobalPathValue.addListener (this);

            auto menuItemString = (TargetOS::getThisOS() == TargetOS::osx ? "\"Projucer->Global Paths...\""
                                                                          : "\"File->Global Paths...\"");

            props.add (new BooleanPropertyComponent (useGlobalPathValue,
                                                     "Use global path", "Use global path for this module"),
                       String ("If this is enabled, then the locally-stored global path (set in the ") + menuItemString + " menu item) "
                       "will be used as the path to this module. "
                       "This means that if this Projucer project is opened on another machine it will use that machine's global path as the path to this module.");

            props.add (new BooleanPropertyComponent (modules.shouldCopyModuleFilesLocallyValue (moduleID),
                                                     "Create local copy", "Copy the module into the project folder"),
                       "If this is enabled, then a local copy of the entire module will be made inside your project (in the auto-generated JuceLibraryFiles folder), "
                       "so that your project will be self-contained, and won't need to contain any references to files in other folders. "
                       "This also means that you can check the module into your source-control system to make sure it is always in sync with your own code.");

            props.add (new BooleanPropertyComponent (modules.shouldShowAllModuleFilesInProjectValue (moduleID),
                                                     "Add source to project", "Make module files browsable in projects"),
                       "If this is enabled, then the entire source tree from this module will be shown inside your project, "
                       "making it easy to browse/edit the module's classes. If disabled, then only the minimum number of files "
                       "required to compile it will appear inside your project.");

            auto info = modules.getModuleInfo (moduleID);

            if (info.isValid())
            {
                configFlags.clear();
                LibraryModule (info).getConfigFlags (project, configFlags);

                for (auto* flag : configFlags)
                {
                    auto* c = new ChoicePropertyComponent (flag->value, flag->symbol);
                    c->setTooltip (flag->description);
                    props.add (c);
                }
            }

            group.setProperties (props);
            parentSizeChanged();
        }

        void parentSizeChanged() override      { updateSize (*this, group); }
        void resized() override                { group.setBounds (getLocalBounds().withTrimmedLeft (12)); }

        String getModuleID() const noexcept    { return moduleID; }

    private:
        void valueTreePropertyChanged (ValueTree&, const Identifier& property) override
        {
            if (property == Ids::defaultJuceModulePath || property == Ids::defaultUserModulePath)
                refresh();
        }

        void valueChanged (Value& v) override
        {
            auto isExporterPathValue = [this, &v]
            {
                for (auto& exporterValue : exporterModulePathValues)
                    if (exporterValue.refersToSameSourceAs (v))
                        return true;

                return false;
            }();

            if (isExporterPathValue)
                project.rescanExporterPathModules();

            refresh();
        }

        //==============================================================================
        Array<Value> exporterModulePathValues;
        Value useGlobalPathValue;

        OwnedArray<Project::ConfigFlag> configFlags;

        PropertyGroupComponent group;
        Project& project;
        SafePointer<TreeView> modulesTree;
        String moduleID;

        //==============================================================================
        class ModuleInfoComponent final  : public PropertyComponent,
                                           private Value::Listener
        {
        public:
            ModuleInfoComponent (Project& p, const String& modID)
                : PropertyComponent ("Module", 150), project (p), moduleID (modID)
            {
                for (Project::ExporterIterator exporter (project); exporter.next();)
                    listeningValues.add (new Value (exporter->getPathForModuleValue (moduleID).getPropertyAsValue()))->addListener (this);

                refresh();
            }

            void refresh() override
            {
                info = project.getEnabledModules().getModuleInfo (moduleID);
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
        class MissingDependenciesComponent final : public PropertyComponent
        {
        public:
            MissingDependenciesComponent (Project& p, const String& modID)
                : PropertyComponent ("Dependencies", 100),
                  project (p), moduleID (modID),
                  missingDependencies (project.getEnabledModules().getExtraDependenciesNeeded (modID))
            {
                addAndMakeVisible (fixButton);
                fixButton.setColour (TextButton::buttonColourId, Colours::red);
                fixButton.setColour (TextButton::textColourOffId, Colours::white);
                fixButton.onClick = [this] { fixDependencies(); };
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

            void fixDependencies()
            {
                auto& enabledModules = project.getEnabledModules();

                if (enabledModules.tryToFixMissingDependencies (moduleID))
                {
                    missingDependencies.clear();
                }
                else
                {
                    missingDependencies = enabledModules.getExtraDependenciesNeeded (moduleID);

                    auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                                     "Adding Missing Dependencies",
                                                                     "Couldn't locate some of these modules - you'll need to find their "
                                                                     "folders manually and add them to the list.");
                    messageBox = AlertWindow::showScopedAsync (options, nullptr);
                }
            }

            void resized() override
            {
                fixButton.setBounds (getWidth() - 168, getHeight() - 26, 160, 22);
            }

        private:
            Project& project;

            String moduleID;
            StringArray missingDependencies;
            TextButton fixButton { "Add Required Modules" };
            ScopedMessageBox messageBox;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MissingDependenciesComponent)
        };

        //==============================================================================
        struct CppStandardWarningComponent final : public PropertyComponent
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
class EnabledModulesItem final : public ProjectTreeItemBase,
                                 private Value::Listener,
                                 private AvailableModulesList::Listener
{
public:
    EnabledModulesItem (Project& p)
        : project (p),
          modulesListTree (project.getEnabledModules().getState())
    {
        modulesListTree.addListener (this);

        projectCppStandardValue.referTo (project.getProjectValue (Ids::cppLanguageStandard));
        projectCppStandardValue.addListener (this);

        ProjucerApplication::getApp().getJUCEPathModulesList().addListener (this);
        ProjucerApplication::getApp().getUserPathsModulesList().addListener (this);

        project.getExporterPathsModulesList().addListener (this);
    }

    ~EnabledModulesItem() override
    {
        ProjucerApplication::getApp().getJUCEPathModulesList().removeListener (this);
        ProjucerApplication::getApp().getUserPathsModulesList().removeListener (this);

        project.getExporterPathsModulesList().removeListener (this);
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
        if (auto* pcc = getProjectContentComponent())
            pcc->setScrollableEditorComponent (std::make_unique<ModulesInformationComponent> (project));
    }

    static File getModuleFolder (const File& draggedFile)
    {
        if (draggedFile.hasFileExtension (headerFileExtensions))
            return draggedFile.getParentDirectory();

        return draggedFile;
    }

    bool isInterestedInFileDrag (const StringArray& files) override
    {
        for (auto i = files.size(); --i >= 0;)
            if (ModuleDescription (getModuleFolder (files[i])).isValid())
                return true;

        return false;
    }

    void filesDropped (const StringArray& files, int /*insertIndex*/) override
    {
        Array<ModuleDescription> modules;

        for (auto f : files)
        {
            ModuleDescription m (getModuleFolder (f));

            if (m.isValid())
                modules.add (m);
        }

        for (int i = 0; i < modules.size(); ++i)
            project.getEnabledModules().addModule (modules.getReference (i).getModuleFolder(),
                                                   project.getEnabledModules().areMostModulesCopiedLocally(),
                                                   project.getEnabledModules().areMostModulesUsingGlobalPath());
    }

    void addSubItems() override
    {
        for (int i = 0; i < project.getEnabledModules().getNumModules(); ++i)
            addSubItem (new ModuleItem (project, project.getEnabledModules().getModuleID (i)));
    }

    void showPopupMenu (Point<int> p) override
    {
        PopupMenu moduleMenus;

        const auto addModulesSubMenu = [&] (const auto& description, const auto& modules, auto rescan)
        {
            PopupMenu menu;

            for (const auto& mod : modules)
            {
                menu.addItem (PopupMenu::Item { mod.first }
                                  .setID (-1)
                                  .setEnabled (! project.getEnabledModules().isModuleEnabled (mod.first))
                                  .setAction ([this, name = mod.first] { project.getEnabledModules().addModuleInteractive (name); }));
            }

            menu.addSeparator();
            menu.addItem (PopupMenu::Item { "Re-scan path" }.setID (-1).setAction (rescan));
            moduleMenus.addSubMenu (description, menu);
        };

        addModulesSubMenu ("Global JUCE modules path",
                           ProjucerApplication::getApp().getJUCEPathModulesList().getAllModules(),
                           [] { ProjucerApplication::getApp().rescanJUCEPathModules(); });

        addModulesSubMenu ("Global user modules path",
                           ProjucerApplication::getApp().getUserPathsModulesList().getAllModules(),
                           [] { ProjucerApplication::getApp().rescanUserPathModules(); });

        addModulesSubMenu ("Exporter paths",
                           project.getExporterPathsModulesList().getAllModules(),
                           [this] { project.rescanExporterPathModules(); });

        PopupMenu menu;
        menu.addSubMenu ("Add a module", moduleMenus);
        menu.addSeparator();
        menu.addItem (PopupMenu::Item { "Add a module from a specified folder..." }
                          .setID (-1)
                          .setAction ([this] { project.getEnabledModules().addModuleFromUserSelectedFile(); }));

        launchPopupMenu (menu, p);
    }

    void handlePopupMenuResult (int resultCode) override
    {
        jassertquiet (resultCode == -1 || resultCode == 0);
    }

    //==============================================================================
    void valueTreeChildAdded (ValueTree& parentTree, ValueTree&) override         { refreshIfNeeded (parentTree); }
    void valueTreeChildRemoved (ValueTree& parentTree, ValueTree&, int) override  { refreshIfNeeded (parentTree); }
    void valueTreeChildOrderChanged (ValueTree& parentTree, int, int) override    { refreshIfNeeded (parentTree); }

    void refreshIfNeeded (ValueTree& changedTree)
    {
        if (changedTree == modulesListTree)
        {
            auto selectedID = getSelectedItemID();

            refreshSubItems();

            if (selectedID.isNotEmpty())
                setSelectedItem (selectedID);
        }
    }

private:
    Project& project;
    ValueTree modulesListTree;
    Value projectCppStandardValue;

    //==============================================================================
    void valueChanged (Value& v) override
    {
        if (v == projectCppStandardValue)
        {
            for (int i = 0; i < getNumSubItems(); ++i)
            {
                if (auto* moduleItem = dynamic_cast<ModuleItem*> (getSubItem (i)))
                {
                    if (moduleItem->checkCppStandard())
                    {
                        refreshSubItems();
                        return;
                    }
                }
            }
        }
    }

    void removeDuplicateModules()
    {
        auto jucePathModulesList = ProjucerApplication::getApp().getJUCEPathModulesList().getAllModules();

        auto& userPathModules = ProjucerApplication::getApp().getUserPathsModulesList();
        userPathModules.removeDuplicates (jucePathModulesList);

        auto& exporterPathModules = project.getExporterPathsModulesList();
        exporterPathModules.removeDuplicates (jucePathModulesList);
        exporterPathModules.removeDuplicates (userPathModules.getAllModules());
    }

    void availableModulesChanged (AvailableModulesList*) override
    {
        removeDuplicateModules();
        refreshSubItems();
    }

    String getSelectedItemID() const
    {
        for (int i = 0; i < getNumSubItems(); ++i)
            if (auto* item = getSubItem (i))
                if (item->isSelected())
                    return item->getUniqueName();

        return {};
    }

    void setSelectedItem (const String& itemID)
    {
        for (int i = 0; i < getNumSubItems(); ++i)
        {
            if (auto* item = getSubItem (i))
            {
                if (item->getUniqueName() == itemID)
                {
                    item->setSelected (true, true);
                    return;
                }
            }
        }
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnabledModulesItem)
};
