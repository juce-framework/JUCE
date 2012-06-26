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

#include "../Project Saving/jucer_ProjectExporter.h"
#include "jucer_Module.h"
#include "../Application/jucer_JuceUpdater.h"
#include "../Project/jucer_ProjectContentComponent.h"
#include "jucer_ProjectInformationComponent.h"


//==============================================================================
class ModulesPanel  : public PropertyComponent,
                      public FilenameComponentListener,
                      public ButtonListener
{
public:
    ModulesPanel (Project& project_)
        : PropertyComponent ("Modules", 500),
          project (project_),
          modulesLocation ("modules", ModuleList::getLocalModulesFolder (&project),
                           true, true, false, "*", String::empty,
                           "Select a folder containing your JUCE modules..."),
          modulesLabel (String::empty, "Module source folder:"),
          updateModulesButton ("Check for module updates..."),
          moduleListBox (moduleList),
          copyingMessage (project_, moduleList)
    {
        moduleList.rescan (ModuleList::getLocalModulesFolder (&project));

        addAndMakeVisible (&modulesLocation);
        modulesLocation.setBounds ("150, 3, parent.width - 180, 28");
        modulesLocation.addListener (this);

        modulesLabel.attachToComponent (&modulesLocation, true);

        addAndMakeVisible (&updateModulesButton);
        updateModulesButton.setBounds ("parent.width - 175, 3, parent.width - 4, 28");
        updateModulesButton.addListener (this);

        moduleListBox.setOwner (this);
        addAndMakeVisible (&moduleListBox);
        moduleListBox.setBounds ("4, 31, parent.width / 2 - 4, parent.height - 32");

        addAndMakeVisible (&copyingMessage);
        copyingMessage.setBounds ("4, parent.height - 30, parent.width - 4, parent.height - 1");
        copyingMessage.refresh();
    }

    void filenameComponentChanged (FilenameComponent*)
    {
        moduleList.rescan (modulesLocation.getCurrentFile());
        modulesLocation.setCurrentFile (moduleList.getModulesFolder(), false, false);
        ModuleList::setLocalModulesFolder (moduleList.getModulesFolder());
        moduleListBox.refresh();
    }

    void buttonClicked (Button*)
    {
        JuceUpdater::show (moduleList, getTopLevelComponent(), "");

        filenameComponentChanged (nullptr);
    }

    bool isEnabled (const ModuleList::Module* m) const
    {
        return project.isModuleEnabled (m->uid);
    }

    void setEnabled (const ModuleList::Module* m, bool enable)
    {
        if (enable)
            project.addModule (m->uid, true);
        else
            project.removeModule (m->uid);

        refresh();
    }

    bool areDependenciesMissing (const ModuleList::Module* m)
    {
        return moduleList.getExtraDependenciesNeeded (project, *m).size() > 0;
    }

    void selectionChanged (const ModuleList::Module* selectedModule)
    {
        settings = nullptr;

        if (selectedModule != nullptr)
        {
            addAndMakeVisible (settings = new ModuleSettingsPanel (project, moduleList, selectedModule->uid));
            settings->setBounds ("parent.width / 2 + 1, 31, parent.width - 3, parent.height - 32");
        }

        copyingMessage.refresh();
    }

    void refresh()
    {
        moduleListBox.refresh();

        if (settings != nullptr)
            settings->refreshAll();

        copyingMessage.refresh();
    }

    void paint (Graphics& g) // (overridden to avoid drawing the name)
    {
        getLookAndFeel().drawPropertyComponentBackground (g, getWidth(), getHeight(), *this);
    }

    //==============================================================================
    class ModuleSelectionListBox    : public ListBox,
                                      public ListBoxModel
    {
    public:
        ModuleSelectionListBox (ModuleList& list_)
            : list (list_), owner (nullptr)
        {
            setColour (ListBox::backgroundColourId, Colours::white.withAlpha (0.4f));
            setTooltip ("Use this list to select which modules should be included in your app.\n"
                        "Any modules which have missing dependencies will be shown in red.");
        }

        void setOwner (ModulesPanel* owner_)
        {
            owner = owner_;
            setModel (this);
        }

        void refresh()
        {
            updateContent();
            repaint();
        }

        int getNumRows()
        {
            return list.modules.size();
        }

        void paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
        {
            if (rowIsSelected)
                g.fillAll (findColour (TextEditor::highlightColourId));

            const ModuleList::Module* const m = list.modules [rowNumber];

            if (m != nullptr)
            {
                const float tickSize = height * 0.7f;

                getLookAndFeel().drawTickBox (g, *this, (height - tickSize) / 2, (height - tickSize) / 2, tickSize, tickSize,
                                              owner->isEnabled (m), true, false, false);

                if (owner->isEnabled (m) && owner->areDependenciesMissing (m))
                    g.setColour (Colours::red);
                else
                    g.setColour (Colours::black);

                g.setFont (Font (height * 0.7f, Font::bold));
                g.drawFittedText (m->uid, height, 0, 200, height, Justification::centredLeft, 1);

                g.setFont (Font (height * 0.55f, Font::italic));
                g.drawText (m->name, height + 200, 0, width - height - 200, height, Justification::centredLeft, true);
            }
        }

        void listBoxItemClicked (int row, const MouseEvent& e)
        {
            if (e.x < getRowHeight())
                flipRow (row);
        }

        void listBoxItemDoubleClicked (int row, const MouseEvent& e)
        {
            flipRow (row);
        }

        void returnKeyPressed (int row)
        {
            flipRow (row);
        }

        void selectedRowsChanged (int lastRowSelected)
        {
            owner->selectionChanged (list.modules [lastRowSelected]);
        }

        void flipRow (int row)
        {
            const ModuleList::Module* const m = list.modules [row];

            if (m != nullptr)
                owner->setEnabled (m, ! owner->isEnabled (m));
        }

    private:
        ModuleList& list;
        ModulesPanel* owner;
    };

    //==============================================================================
    class ModuleSettingsPanel  : public PropertyPanel
    {
    public:
        ModuleSettingsPanel (Project& project_, ModuleList& moduleList_, const String& moduleID_)
            : project (project_), moduleList (moduleList_), moduleID (moduleID_)
        {
            refreshAll();
        }

        void refreshAll()
        {
            setEnabled (project.isModuleEnabled (moduleID));

            clear();
            PropertyListBuilder props;

            ScopedPointer<LibraryModule> module (moduleList.loadModule (moduleID));

            if (module != nullptr)
            {
                props.add (new ModuleInfoComponent (project, moduleList, moduleID));

                if (project.isModuleEnabled (moduleID))
                {
                    const ModuleList::Module* m = moduleList.findModuleInfo (moduleID);
                    if (m != nullptr && moduleList.getExtraDependenciesNeeded (project, *m).size() > 0)
                        props.add (new MissingDependenciesComponent (project, moduleList, moduleID));
                }

                props.add (new BooleanPropertyComponent (project.shouldShowAllModuleFilesInProject (moduleID),
                                                         "Add source to project", "Make module files browsable in projects"),
                           "If this is enabled, then the entire source tree from this module will be shown inside your project, "
                           "making it easy to browse/edit the module's classes. If disabled, then only the minimum number of files "
                           "required to compile it will appear inside your project.");

                props.add (new BooleanPropertyComponent (project.shouldCopyModuleFilesLocally (moduleID),
                                                         "Create local copy", "Copy the module into the project folder"),
                           "If this is enabled, then a local copy of the entire module will be made inside your project (in the auto-generated JuceLibraryFiles folder), "
                           "so that your project will be self-contained, and won't need to contain any references to files in other folders. "
                           "This also means that you can check the module into your source-control system to make sure it is always in sync with your own code.");

                StringArray possibleValues;
                possibleValues.add ("(Use Default)");
                possibleValues.add ("Enabled");
                possibleValues.add ("Disabled");

                Array<var> mappings;
                mappings.add (Project::configFlagDefault);
                mappings.add (Project::configFlagEnabled);
                mappings.add (Project::configFlagDisabled);

                OwnedArray <Project::ConfigFlag> flags;
                module->getConfigFlags (project, flags);

                for (int i = 0; i < flags.size(); ++i)
                {
                    ChoicePropertyComponent* c = new ChoicePropertyComponent (flags[i]->value, flags[i]->symbol, possibleValues, mappings);
                    c->setTooltip (flags[i]->description);
                    c->setPreferredHeight (22);
                    props.add (c);
                }
            }

            addProperties (props.components);
        }

    private:
        Project& project;
        ModuleList& moduleList;
        String moduleID;

        //==============================================================================
        class ModuleInfoComponent  : public PropertyComponent
        {
        public:
            ModuleInfoComponent (Project& project_, ModuleList& moduleList_, const String& moduleID_)
                : PropertyComponent ("Module", 100), project (project_), moduleList (moduleList_), moduleID (moduleID_)
            {
            }

            void refresh() {}

            void paint (Graphics& g)
            {
                g.setColour (Colours::white.withAlpha (0.4f));
                g.fillRect (0, 0, getWidth(), getHeight() - 1);

                const ModuleList::Module* module = moduleList.findModuleInfo (moduleID);

                if (module != nullptr)
                {
                    String text;
                    text << module->name << newLine << "Version: " << module->version << newLine << newLine
                         << module->description;

                    GlyphArrangement ga;
                    ga.addJustifiedText (Font (13.0f), text, 4.0f, 16.0f, getWidth() - 8.0f, Justification::topLeft);
                    g.setColour (Colours::black);
                    ga.draw (g);
                }
            }

        private:
            Project& project;
            ModuleList& moduleList;
            String moduleID;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModuleInfoComponent);
        };

        //==============================================================================
        class MissingDependenciesComponent  : public PropertyComponent,
                                              public ButtonListener
        {
        public:
            MissingDependenciesComponent (Project& project_, ModuleList& moduleList_, const String& moduleID_)
                : PropertyComponent ("Dependencies", 100),
                  project (project_), moduleList (moduleList_), moduleID (moduleID_),
                  fixButton ("Enable Required Modules")
            {
                const ModuleList::Module* module = moduleList.findModuleInfo (moduleID);

                if (module != nullptr)
                    missingDependencies = moduleList.getExtraDependenciesNeeded (project, *module);

                addAndMakeVisible (&fixButton);
                fixButton.setColour (TextButton::buttonColourId, Colours::red);
                fixButton.setColour (TextButton::textColourOffId, Colours::white);
                fixButton.setBounds ("right - 160, parent.height - 26, parent.width - 8, top + 22");
                fixButton.addListener (this);
            }

            void refresh() {}

            void paint (Graphics& g)
            {
                g.setColour (Colours::white.withAlpha (0.4f));
                g.fillRect (0, 0, getWidth(), getHeight() - 1);

                String text ("This module requires the following dependencies:\n");
                text << missingDependencies.joinIntoString (", ");

                GlyphArrangement ga;
                ga.addJustifiedText (Font (13.0f), text, 4.0f, 16.0f, getWidth() - 8.0f, Justification::topLeft);
                g.setColour (Colours::red);
                ga.draw (g);
            }

            void buttonClicked (Button*)
            {
                bool isModuleCopiedLocally = project.shouldCopyModuleFilesLocally (moduleID).getValue();

                for (int i = missingDependencies.size(); --i >= 0;)
                    project.addModule (missingDependencies[i], isModuleCopiedLocally);

                ModulesPanel* mp = findParentComponentOfClass<ModulesPanel>();
                if (mp != nullptr)
                    mp->refresh();
            }

        private:
            Project& project;
            ModuleList& moduleList;
            String moduleID;
            StringArray missingDependencies;
            TextButton fixButton;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MissingDependenciesComponent);
        };
    };

    //==============================================================================
    class ModuleCopyingInfo  : public Component,
                               public ButtonListener,
                               public Timer
    {
    public:
        ModuleCopyingInfo (Project& project_, ModuleList& list_)
            : project (project_), list (list_),
              copyModeButton ("Set Copying Mode...")
        {
            addAndMakeVisible (&copyModeButton);
            copyModeButton.setBounds ("4, parent.height / 2 - 10, 160, parent.height / 2 + 10");
            copyModeButton.addListener (this);

            startTimer (1500);
        }

        void paint (Graphics& g)
        {
            g.setFont (11.0f);
            g.setColour (Colours::darkred);
            g.drawFittedText (getName(), copyModeButton.getRight() + 10, 0,
                              getWidth() - copyModeButton.getRight() - 16, getHeight(),
                              Justification::centredRight, 4);
        }

        void refresh()
        {
            int numCopied, numNonCopied;
            countCopiedModules (numCopied, numNonCopied);

            String newName;

            if (numCopied > 0 && numNonCopied > 0)
                newName = "Warning! Some of your modules are set to use local copies, and others are using remote references.\n"
                          "This may create problems if some modules expect to share the same parent folder, so you may "
                          "want to make sure that they are all either copied or not.";

            if (project.isAudioPluginModuleMissing())
                newName = "Warning! Your project is an audio plugin, but you haven't enabled the 'juce_audio_plugin_client' module!";

            if (newName != getName())
            {
                setName (newName);
                repaint();
            }
        }

        void countCopiedModules (int& numCopied, int& numNonCopied)
        {
            numCopied = numNonCopied = 0;

            for (int i = list.modules.size(); --i >= 0;)
            {
                const String moduleID (list.modules.getUnchecked(i)->uid);

                if (project.isModuleEnabled (moduleID))
                {
                    if (project.shouldCopyModuleFilesLocally (moduleID).getValue())
                        ++numCopied;
                    else
                        ++numNonCopied;
                }
            }
        }

        void buttonClicked (Button*)
        {
            PopupMenu menu;
            menu.addItem (1, "Enable local copying for all modules");
            menu.addItem (2, "Disable local copying for all modules");

            menu.showMenuAsync (PopupMenu::Options().withTargetComponent (&copyModeButton),
                                ModalCallbackFunction::forComponent (copyMenuItemChosen, this));
        }

        static void copyMenuItemChosen (int resultCode, ModuleCopyingInfo* comp)
        {
            if (resultCode > 0 && comp != nullptr)
                comp->setCopyModeForAllModules (resultCode == 1);
        }

        void setCopyModeForAllModules (bool copyEnabled)
        {
            for (int i = list.modules.size(); --i >= 0;)
                project.shouldCopyModuleFilesLocally (list.modules.getUnchecked(i)->uid) = copyEnabled;

            refresh();
        }

        void timerCallback()
        {
            refresh();
        }

    private:
        Project& project;
        ModuleList& list;
        TextButton copyModeButton;
    };

private:
    Project& project;
    ModuleList moduleList;
    FilenameComponent modulesLocation;
    Label modulesLabel;
    TextButton updateModulesButton;
    ModuleSelectionListBox moduleListBox;
    ModuleCopyingInfo copyingMessage;
    ScopedPointer<ModuleSettingsPanel> settings;
};

//==============================================================================
struct ProjectSettingsTreeClasses
{
    class PropertyGroup  : public Component
    {
    public:
        PropertyGroup()  {}

        void setProperties (const PropertyListBuilder& newProps)
        {
            properties.clear();
            properties.addArray (newProps.components);

            for (int i = properties.size(); --i >= 0;)
                addAndMakeVisible (properties.getUnchecked(i));
        }

        int updateSize (int x, int y, int width)
        {
            int height = 36;

            for (int i = 0; i < properties.size(); ++i)
            {
                PropertyComponent* pp = properties.getUnchecked(i);
                pp->setBounds (10, height, width - 20, pp->getPreferredHeight());
                height += pp->getHeight();
            }

            height += 16;
            setBounds (x, y, width, height);
            return height;
        }

        void paint (Graphics& g)
        {
            g.setColour (Colours::white.withAlpha (0.3f));
            g.fillRect (0, 28, getWidth(), getHeight() - 38);

            g.setColour (Colours::black.withAlpha (0.4f));
            g.drawRect (0, 28, getWidth(), getHeight() - 38);

            g.setFont (Font (14.0f, Font::bold));
            g.setColour (Colours::black);
            g.drawFittedText (getName(), 12, 0, getWidth() - 16, 26, Justification::bottomLeft, 1);
        }

        OwnedArray<PropertyComponent> properties;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyGroup);
    };

    //==============================================================================
    class PropertyPanelViewport  : public Component
    {
    public:
        PropertyPanelViewport (Component* content)
        {
            addAndMakeVisible (&viewport);
            addAndMakeVisible (&rolloverHelp);
            viewport.setViewedComponent (content, true);
        }

        void paint (Graphics& g)
        {
            g.setTiledImageFill (ImageCache::getFromMemory (BinaryData::brushed_aluminium_png,
                                                            BinaryData::brushed_aluminium_pngSize),
                                 0, 0, 1.0f);
            g.fillAll();
            drawRecessedShadows (g, getWidth(), getHeight(), 14);
        }

        void resized()
        {
            Rectangle<int> r (getLocalBounds());
            rolloverHelp.setBounds (r.removeFromBottom (70).reduced (10, 0));
            viewport.setBounds (r);
        }

        Viewport viewport;
        RolloverHelpComp rolloverHelp;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyPanelViewport);
    };

    //==============================================================================
    class SettingsItemBase  : public JucerTreeViewBase,
                              public ValueTree::Listener
    {
    public:
        SettingsItemBase() {}

        void showSettingsPage (Component* content)
        {
            content->setComponentID (getUniqueName());

            ScopedPointer<Component> comp (content);
            ProjectContentComponent* pcc = getProjectContentComponent();

            if (pcc != nullptr)
                pcc->setEditorComponent (new PropertyPanelViewport (comp.release()), nullptr);
        }

        void closeSettingsPage()
        {
            ProjectContentComponent* pcc = getProjectContentComponent();

            if (pcc != nullptr)
            {
                PropertyPanelViewport* ppv = dynamic_cast<PropertyPanelViewport*> (pcc->getEditorComponent());

                if (ppv != nullptr && ppv->viewport.getViewedComponent()->getComponentID() == getUniqueName())
                    pcc->hideEditor();
            }
        }

        void deleteAllSelectedItems()
        {
            TreeView* const tree = getOwnerView();
            jassert (tree->getNumSelectedItems() <= 1); // multi-select should be disabled

            if (SettingsItemBase* s = dynamic_cast <SettingsItemBase*> (tree->getSelectedItem (0)))
                s->deleteItem();
        }

        void itemOpennessChanged (bool isNowOpen)
        {
            if (isNowOpen)
               refreshSubItems();
        }

        void valueTreePropertyChanged (ValueTree&, const Identifier&) {}
        void valueTreeChildAdded (ValueTree&, ValueTree&) {}
        void valueTreeChildRemoved (ValueTree&, ValueTree&) {}
        void valueTreeChildOrderChanged (ValueTree&) {}
        void valueTreeParentChanged (ValueTree&) {}

        static void updateSizes (Component& comp, PropertyGroup* groups, int numGroups)
        {
            const int width = jmax (550, comp.getParentWidth() - 20);

            int y = 0;
            for (int i = 0; i < numGroups; ++i)
                y += groups[i].updateSize (12, y, width - 12);

            comp.setSize (width, y);
        }
    };

    //==============================================================================
    class ConfigItem   : public SettingsItemBase
    {
    public:
        ConfigItem (const ProjectExporter::BuildConfiguration::Ptr& config_, const String& exporterName_)
            : config (config_), exporterName (exporterName_), configTree (config->config)
        {
            jassert (config != nullptr);
            configTree.addListener (this);
        }

        bool isRoot() const                     { return false; }
        bool isMissing()                        { return false; }
        bool canBeSelected() const              { return true; }
        bool mightContainSubItems()             { return false; }
        String getUniqueName() const            { return config->project.getProjectUID() + "_config_" + config->getName(); }
        String getRenamingName() const          { return getDisplayName(); }
        String getDisplayName() const           { return config->getName(); }
        void setName (const String&)            {}
        const Drawable* getIcon() const         { return StoredSettings::getInstance()->getCogIcon(); }

        void showDocument()                     { showSettingsPage (new SettingsComp (config, exporterName)); }
        void itemOpennessChanged (bool)         {}

        void deleteItem()
        {
            if (AlertWindow::showOkCancelBox (AlertWindow::WarningIcon, "Delete Configuration",
                                              "Are you sure you want to delete this configuration?"))
            {
                closeSettingsPage();
                config->removeFromExporter();
            }
        }

        void showPopupMenu()
        {
            PopupMenu menu;
            menu.addItem (1, "Create a copy of this configuration");
            menu.addSeparator();
            menu.addItem (2, "Delete this configuration");

            launchPopupMenu (menu);
        }

        void handlePopupMenuResult (int resultCode)
        {
            if (resultCode == 2)
            {
                deleteAllSelectedItems();
            }
            else if (resultCode == 1)
            {
                for (Project::ExporterIterator exporter (config->project); exporter.next();)
                {
                    if (config->config.isAChildOf (exporter.exporter->settings))
                    {
                        exporter.exporter->addNewConfiguration (config);
                        break;
                    }
                }
            }
        }

        var getDragSourceDescription()
        {
            return getParentItem()->getUniqueName() + "||" + config->getName();
        }

        void valueTreePropertyChanged (ValueTree&, const Identifier&)  { repaintItem(); }

    private:
        ProjectExporter::BuildConfiguration::Ptr config;
        String exporterName;
        ValueTree configTree;

        //==============================================================================
        class SettingsComp  : public Component
        {
        public:
            SettingsComp (ProjectExporter::BuildConfiguration* config, const String& exporterName)
            {
                addAndMakeVisible (&group);

                PropertyListBuilder props;
                config->createPropertyEditors (props);
                group.setProperties (props);
                group.setName (exporterName + " / " + config->getName());
                parentSizeChanged();
            }

            void parentSizeChanged()  { updateSizes (*this, &group, 1); }

        private:
            PropertyGroup group;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsComp);
        };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConfigItem);
    };

    //==============================================================================
    class ExporterItem   : public SettingsItemBase
    {
    public:
        ExporterItem (Project& project_, ProjectExporter* exporter_, int exporterIndex_)
            : project (project_), exporter (exporter_), configListTree (exporter->getConfigurations()),
              exporterIndex (exporterIndex_)
        {
            configListTree.addListener (this);
            jassert (exporter != nullptr);
        }

        bool isRoot() const                     { return false; }
        bool canBeSelected() const              { return true; }
        bool mightContainSubItems()             { return exporter->getNumConfigurations() > 0; }
        String getUniqueName() const            { return project.getProjectUID() + "_exporter_" + String (exporterIndex); }
        String getRenamingName() const          { return getDisplayName(); }
        String getDisplayName() const           { return exporter->getName(); }
        void setName (const String&)            {}
        bool isMissing()                        { return false; }
        const Drawable* getIcon() const         { return LookAndFeel::getDefaultLookAndFeel().getDefaultDocumentFileImage(); }
        void showDocument()                     { showSettingsPage (new SettingsComp (exporter)); }

        void deleteItem()
        {
            if (AlertWindow::showOkCancelBox (AlertWindow::WarningIcon, "Delete Exporter",
                                              "Are you sure you want to delete this export target?"))
            {
                closeSettingsPage();
                ValueTree parent (exporter->settings.getParent());
                parent.removeChild (exporter->settings, project.getUndoManagerFor (parent));
            }
        }

        void addSubItems()
        {
            for (ProjectExporter::ConfigIterator config (*exporter); config.next();)
                addSubItem (new ConfigItem (config.config, exporter->getName()));
        }

        void showPopupMenu()
        {
            PopupMenu menu;
            menu.addItem (1, "Add a new configuration");
            menu.addSeparator();
            menu.addItem (2, "Delete this exporter");

            launchPopupMenu (menu);
        }

        void handlePopupMenuResult (int resultCode)
        {
            if (resultCode == 2)
                deleteAllSelectedItems();
            else if (resultCode == 1)
                exporter->addNewConfiguration (nullptr);
        }

        var getDragSourceDescription()
        {
            return getParentItem()->getUniqueName() + "/" + String (exporterIndex);
        }

        bool isInterestedInDragSource (const DragAndDropTarget::SourceDetails& dragSourceDetails)
        {
            return dragSourceDetails.description.toString().startsWith (getUniqueName());
        }

        void itemDropped (const DragAndDropTarget::SourceDetails& dragSourceDetails, int insertIndex)
        {
            const int oldIndex = indexOfConfig (dragSourceDetails.description.toString().fromLastOccurrenceOf ("||", false, false));

            if (oldIndex >= 0)
                configListTree.moveChild (oldIndex, insertIndex, project.getUndoManagerFor (configListTree));
        }

        int indexOfConfig (const String& configName)
        {
            int i = 0;
            for (ProjectExporter::ConfigIterator config (*exporter); config.next(); ++i)
                if (config->getName() == configName)
                    return i;

            return -1;
        }

        //==============================================================================
        void valueTreeChildAdded (ValueTree& parentTree, ValueTree&)    { refreshIfNeeded (parentTree); }
        void valueTreeChildRemoved (ValueTree& parentTree, ValueTree&)  { refreshIfNeeded (parentTree); }
        void valueTreeChildOrderChanged (ValueTree& parentTree)         { refreshIfNeeded (parentTree); }

        void refreshIfNeeded (ValueTree& changedTree)
        {
            if (changedTree == configListTree)
                refreshSubItems();
        }

    private:
        Project& project;
        ScopedPointer<ProjectExporter> exporter;
        ValueTree configListTree;
        int exporterIndex;

        //==============================================================================
        class SettingsComp  : public Component
        {
        public:
            SettingsComp (ProjectExporter* exporter)
            {
                addAndMakeVisible (&group);

                PropertyListBuilder props;
                exporter->createPropertyEditors (props);
                group.setProperties (props);
                group.setName ("Export target: " + exporter->getName());
                parentSizeChanged();
            }

            void parentSizeChanged()  { updateSizes (*this, &group, 1); }

        private:
            PropertyGroup group;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsComp);
        };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExporterItem);
    };

    //==============================================================================
    class RootItem   : public SettingsItemBase
    {
    public:
        RootItem (Project& project_)
            : project (project_), exportersTree (project_.getExporters())
        {
            exportersTree.addListener (this);
        }

        bool isRoot() const                     { return true; }
        String getRenamingName() const          { return getDisplayName(); }
        String getDisplayName() const           { return project.getTitle(); }
        void setName (const String&)            {}
        bool isMissing()                        { return false; }
        const Drawable* getIcon() const         { return project.getMainGroup().getIcon(); }
        void showDocument()                     { showSettingsPage (new SettingsComp (project)); }
        bool canBeSelected() const              { return true; }
        bool mightContainSubItems()             { return project.getNumExporters() > 0; }
        String getUniqueName() const            { return project.getProjectUID() + "_config_root"; }

        void addSubItems()
        {
            int i = 0;
            for (Project::ExporterIterator exporter (project); exporter.next(); ++i)
                addSubItem (new ExporterItem (project, exporter.exporter.release(), i));
        }

        void showPopupMenu()
        {
            PopupMenu menu;

            const StringArray exporters (ProjectExporter::getExporterNames());

            for (int i = 0; i < exporters.size(); ++i)
                menu.addItem (i + 1, "Create a new " + exporters[i] + " target");

            launchPopupMenu (menu);
        }

        void handlePopupMenuResult (int resultCode)
        {
            if (resultCode > 0)
            {
                String exporterName (ProjectExporter::getExporterNames() [resultCode - 1]);

                if (exporterName.isNotEmpty())
                    project.addNewExporter (exporterName);
            }
        }

        bool isInterestedInDragSource (const DragAndDropTarget::SourceDetails& dragSourceDetails)
        {
            return dragSourceDetails.description.toString().startsWith (getUniqueName());
        }

        void itemDropped (const DragAndDropTarget::SourceDetails& dragSourceDetails, int insertIndex)
        {
            int oldIndex = dragSourceDetails.description.toString().getTrailingIntValue();
            exportersTree.moveChild (oldIndex, insertIndex, project.getUndoManagerFor (exportersTree));
        }

        //==============================================================================
        void valueTreeChildAdded (ValueTree& parentTree, ValueTree&)    { refreshIfNeeded (parentTree); }
        void valueTreeChildRemoved (ValueTree& parentTree, ValueTree&)  { refreshIfNeeded (parentTree); }
        void valueTreeChildOrderChanged (ValueTree& parentTree)         { refreshIfNeeded (parentTree); }

        void refreshIfNeeded (ValueTree& changedTree)
        {
            if (changedTree == exportersTree)
                refreshSubItems();
        }

    private:
        Project& project;
        ValueTree exportersTree;

        //==============================================================================
        class SettingsComp  : public Component,
                              private ChangeListener
        {
        public:
            SettingsComp (Project& project_)
                : project (project_)
            {
                addAndMakeVisible (&groups[0]);
                addAndMakeVisible (&groups[1]);

                createAllPanels();
                project.addChangeListener (this);
            }

            ~SettingsComp()
            {
                project.removeChangeListener (this);
            }

            void parentSizeChanged()
            {
                updateSizes (*this, groups, 2);
            }

            void createAllPanels()
            {
                {
                    PropertyListBuilder props;
                    project.createPropertyEditors (props);
                    groups[0].setProperties (props);
                    groups[0].setName ("Project Settings");

                    lastProjectType = project.getProjectTypeValue().getValue();
                }

                PropertyListBuilder props;
                props.add (new ModulesPanel (project));
                groups[1].setProperties (props);
                groups[1].setName ("Modules");

                parentSizeChanged();
            }

            void changeListenerCallback (ChangeBroadcaster*)
            {
                if (lastProjectType != project.getProjectTypeValue().getValue())
                    createAllPanels();
            }

        private:
            Project& project;
            var lastProjectType;
            PropertyGroup groups[2];

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsComp);
        };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RootItem);
    };
};

JucerTreeViewBase* createProjectConfigTreeViewRoot (Project& project)
{
    return new ProjectSettingsTreeClasses::RootItem (project);
}
