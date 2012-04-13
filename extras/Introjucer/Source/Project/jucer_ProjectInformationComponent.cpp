/*
  ==============================================================================

  This is an automatically generated file!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created for JUCE version: JUCE v2.0.16

  ------------------------------------------------------------------------------

  JUCE is copyright 2004-11 by Raw Material Software ltd.

  ==============================================================================
*/

//[CppHeaders] You can add your own extra header files here...
#include "../Project Saving/jucer_ProjectExporter.h"
#include "jucer_Module.h"
#include "../Application/jucer_JuceUpdater.h"
//[/CppHeaders]

#include "jucer_ProjectInformationComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...


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

                g.setFont (height * 0.7f, Font::bold);
                g.drawFittedText (m->uid, height, 0, 200, height, Justification::centredLeft, 1);

                g.setFont (height * 0.55f, Font::italic);
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
class ProjectSettingsComponent  : public Component
{
public:
    ProjectSettingsComponent (Project& project_)
        : project (project_),
          exporters ("Export Targets", "Add a New Exporter...", true, false)
    {
        addAndMakeVisible (&mainProjectInfoPanel);
        addAndMakeVisible (&modulesPanelGroup);
        addAndMakeVisible (&exporters);

        mainProjectInfoPanel.fillBackground = true;
        modulesPanelGroup.fillBackground = true;
    }

    void updateSize (int width)
    {
        width = jmax (550, width - 6);

        int y = 0;
        y += mainProjectInfoPanel.updateSize (y, width);
        y += modulesPanelGroup.updateSize (y, width);
        y += exporters.updateSize (y, width);

        setSize (width, y);
    }

    void parentSizeChanged()
    {
        updateSize (getParentWidth());
    }

    void visibilityChanged()
    {
        if (isVisible())
            createAllPanels();
    }

    void createModulesPanel()
    {
        PropertyListBuilder props;
        props.add (new ModulesPanel (project));
        modulesPanelGroup.setProperties (props);
        modulesPanelGroup.setName ("Modules");
    }

    void createProjectPanel()
    {
        PropertyListBuilder props;
        project.createPropertyEditors (props);
        mainProjectInfoPanel.setProperties (props);
        mainProjectInfoPanel.setName ("Project Settings");

        lastProjectType = project.getProjectTypeValue().getValue();
    }

    void createExportersPanel()
    {
        exporters.clear();

        for (Project::ExporterIterator exporter (project); exporter.next();)
        {
            PropertyGroup* exporterGroup = exporters.createGroup();
            exporterGroup->fillBackground = true;
            exporterGroup->addDeleteButton ("exporter " + String (exporter.index), "Deletes this export target.");

            PropertyListBuilder props;
            exporter->createPropertyEditors (props);

            PropertyGroupList* configList = new PropertyGroupList ("Configurations", "Add a New Configuration", false, true);
            props.add (configList);
            exporterGroup->setProperties (props);

            configList->createNewButton.setName ("newconfig " + String (exporter.index));

            for (ProjectExporter::ConfigIterator config (*exporter); config.next();)
            {
                PropertyGroup* configGroup = configList->createGroup();

                if (exporter->getNumConfigurations() > 1)
                    configGroup->addDeleteButton ("config " + String (exporter.index) + "/" + String (config.index), "Deletes this configuration.");

                PropertyListBuilder configProps;
                config->createPropertyEditors (configProps);
                configGroup->setProperties (configProps);
            }
        }
    }

    void createAllPanels()
    {
        createProjectPanel();
        createModulesPanel();
        createExportersPanel();
        updateNames();

        updateSize (getWidth());
    }

    bool needsFullUpdate() const
    {
        if (exporters.groups.size() != project.getNumExporters()
             || lastProjectType != project.getProjectTypeValue().getValue())
            return true;

        for (int i = exporters.groups.size(); --i >= 0;)
        {
            ScopedPointer <ProjectExporter> exp (project.createExporter (i));

            jassert (exp != nullptr);
            if (exp != nullptr)
            {
                PropertyGroupList* configList = dynamic_cast <PropertyGroupList*> (exporters.groups.getUnchecked(i)->properties.getLast());

                if (configList != nullptr && configList->groups.size() != exp->getNumConfigurations())
                    return true;
            }
        }

        return false;
    }

    void updateNames()
    {
        for (int i = exporters.groups.size(); --i >= 0;)
        {
            PropertyGroup& exporterGroup = *exporters.groups.getUnchecked(i);
            ScopedPointer <ProjectExporter> exp (project.createExporter (i));
            jassert (exp != nullptr);

            if (exp != nullptr)
            {
                exporterGroup.setName (exp->getName());
                exporterGroup.repaint();

                PropertyGroupList* configList = dynamic_cast <PropertyGroupList*> (exporterGroup.properties.getLast());

                if (configList != nullptr)
                {
                    for (int j = configList->groups.size(); --j >= 0;)
                    {
                        PropertyGroup& configGroup = *configList->groups.getUnchecked(j);
                        configGroup.setName ("Configuration: " + exp->getConfiguration (j)->getName().quoted());
                        configGroup.repaint();
                    }
                }
            }
        }
    }

    void update()
    {
        if (needsFullUpdate())
            createAllPanels();
        else
            updateNames();
    }

    void deleteButtonClicked (const String& name)
    {
        if (name.startsWith ("config"))
        {
            int exporterIndex = name.upToLastOccurrenceOf ("/", false, false).getTrailingIntValue();
            int configIndex = name.getTrailingIntValue();

            ScopedPointer<ProjectExporter> exporter (project.createExporter (exporterIndex));
            jassert (exporter != nullptr);

            if (exporter != nullptr)
                exporter->deleteConfiguration (configIndex);
        }
        else
        {
            project.deleteExporter (name.getTrailingIntValue());
        }
    }

    static void newExporterMenuItemChosen (int resultCode, ProjectSettingsComponent* settingsComp)
    {
        if (resultCode > 0 && settingsComp != nullptr)
            settingsComp->project.addNewExporter (ProjectExporter::getExporterNames() [resultCode - 1]);
    }

    void createNewExporter (TextButton& button)
    {
        PopupMenu menu;

        const StringArray exporters (ProjectExporter::getExporterNames());

        for (int i = 0; i < exporters.size(); ++i)
            menu.addItem (i + 1, "Create a new " + exporters[i] + " target");

        menu.showMenuAsync (PopupMenu::Options().withTargetComponent (&button),
                            ModalCallbackFunction::forComponent (newExporterMenuItemChosen, this));
    }

    void createNewConfig (int exporterIndex)
    {
        ScopedPointer<ProjectExporter> exp (project.createExporter (exporterIndex));
        jassert (exp != nullptr);

        if (exp != nullptr)
            exp->addNewConfiguration (nullptr);
    }

    void newItemButtonClicked (TextButton& button)
    {
        if (button.getName().containsIgnoreCase ("export"))
            createNewExporter (button);
        else if (button.getName().containsIgnoreCase ("newconfig"))
            createNewConfig (button.getName().getTrailingIntValue());
    }

private:
    //==============================================================================
    class PropertyGroup  : public Component,
                           public ButtonListener
    {
    public:
        PropertyGroup()
            : deleteButton ("Delete"), fillBackground (false)
        {
            deleteButton.addListener (this);
        }

        void addDeleteButton (const String& name, const String& tooltip)
        {
            addAndMakeVisible (&deleteButton);
            deleteButton.setBounds ("right - 55, 11, parent.width - 10, 26");
            deleteButton.setColour (TextButton::buttonColourId, Colour (0xa0fcbdbd));
            deleteButton.setColour (TextButton::textColourOffId, Colours::darkred);
            deleteButton.setConnectedEdges (Button::ConnectedOnLeft | Button::ConnectedOnRight);
            deleteButton.setName (name);
            deleteButton.setTooltip (tooltip);
        }

        void setProperties (const PropertyListBuilder& newProps)
        {
            properties.clear();
            properties.addArray (newProps.components);

            for (int i = properties.size(); --i >= 0;)
                addAndMakeVisible (properties.getUnchecked(i));
        }

        int updateSize (int y, int width)
        {
            int height = fillBackground ? 36 : 32;

            for (int i = 0; i < properties.size(); ++i)
            {
                PropertyComponent* pp = properties.getUnchecked(i);
                PropertyGroupList* pgl = dynamic_cast <PropertyGroupList*> (pp);

                if (pgl != nullptr)
                    pgl->updateSize (height, width - 20);

                pp->setBounds (10, height, width - 20, pp->getPreferredHeight());
                height += pp->getHeight();
            }

            height += 16;
            setBounds (0, y, width, height);
            return height;
        }

        void paint (Graphics& g)
        {
            if (fillBackground)
            {
                g.setColour (Colours::white.withAlpha (0.3f));
                g.fillRect (0, 28, getWidth(), getHeight() - 38);

                g.setColour (Colours::black.withAlpha (0.4f));
                g.drawRect (0, 28, getWidth(), getHeight() - 38);
            }

            g.setFont (14.0f, Font::bold);
            g.setColour (Colours::black);
            g.drawFittedText (getName(), 12, 0, getWidth() - 16, 26, Justification::bottomLeft, 1);
        }

        void buttonClicked (Button*)
        {
            ProjectSettingsComponent* psc = findParentComponentOfClass<ProjectSettingsComponent>();
            if (psc != nullptr)
                psc->deleteButtonClicked (deleteButton.getName());
        }

        OwnedArray<PropertyComponent> properties;
        TextButton deleteButton;
        bool fillBackground;
    };

    //==============================================================================
    class PropertyGroupList  : public PropertyComponent,
                               public ButtonListener
    {
    public:
        PropertyGroupList (const String& title, const String& newButtonText,
                           bool triggerOnMouseDown, bool hideNameAndPutButtonAtBottom)
            : PropertyComponent (title), createNewButton (newButtonText),
              dontDisplayName (hideNameAndPutButtonAtBottom)
        {
            addAndMakeVisible (&createNewButton);
            createNewButton.setColour (TextButton::buttonColourId, Colours::lightgreen.withAlpha (0.5f));
            createNewButton.setBounds (hideNameAndPutButtonAtBottom ? "right - 140, parent.height - 25, parent.width - 10, top + 20"
                                                                    : "right - 140, 30, parent.width - 10, top + 20");
            createNewButton.setConnectedEdges (Button::ConnectedOnLeft | Button::ConnectedOnRight);
            createNewButton.addListener (this);
            createNewButton.setTriggeredOnMouseDown (triggerOnMouseDown);
        }

        int updateSize (int ourY, int width)
        {
            int y = dontDisplayName ? 10 : 55;

            for (int i = 0; i < groups.size(); ++i)
                y += groups.getUnchecked(i)->updateSize (y, width);

            y = jmax (y, 100);
            setBounds (0, ourY, width, y);

            if (dontDisplayName)
                y += 25;

            setPreferredHeight (y);
            return y;
        }

        void paint (Graphics& g)
        {
            if (! dontDisplayName)
            {
                g.setFont (17.0f, Font::bold);
                g.setColour (Colours::black);
                g.drawFittedText (getName(), 0, 30, getWidth(), 20, Justification::centred, 1);
            }
        }

        void clear()
        {
            groups.clear();
        }

        void refresh() {}

        PropertyGroup* createGroup()
        {
            PropertyGroup* p = new PropertyGroup();
            groups.add (p);
            addAndMakeVisible (p);
            return p;
        }

        void buttonClicked (Button*)
        {
            ProjectSettingsComponent* psc = findParentComponentOfClass<ProjectSettingsComponent>();
            if (psc != nullptr)
                psc->newItemButtonClicked (createNewButton);
        }

        OwnedArray<PropertyGroup> groups;
        TextButton createNewButton;
        bool dontDisplayName;
    };

    Project& project;
    var lastProjectType;
    PropertyGroup mainProjectInfoPanel, modulesPanelGroup;
    PropertyGroupList exporters;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectSettingsComponent);
};

//[/MiscUserDefs]

//==============================================================================
ProjectInformationComponent::ProjectInformationComponent (Project& project_)
    : project (project_)
{
    //[Constructor_pre]
    //[/Constructor_pre]

    addChildAndSetID (&viewport, "ykdBpb");
    addChildAndSetID (&openProjectButton, "a550a652e2666ee7");
    addChildAndSetID (&saveAndOpenButton, "dRGMyYx");
    addChildAndSetID (&rollover, "QqLJBF");

    initialiseComponentState();
    openProjectButton.addListener (this);
    saveAndOpenButton.addListener (this);

    //[UserPreSize]
    viewport.setViewedComponent (new ProjectSettingsComponent (project), true);

   #if JUCE_MAC || JUCE_WINDOWS
    openProjectButton.setCommandToTrigger (commandManager, CommandIDs::openInIDE, true);
    openProjectButton.setButtonText (commandManager->getNameOfCommand (CommandIDs::openInIDE));

    saveAndOpenButton.setCommandToTrigger (commandManager, CommandIDs::saveAndOpenInIDE, true);
    saveAndOpenButton.setButtonText (commandManager->getNameOfCommand (CommandIDs::saveAndOpenInIDE));
   #else
    openProjectButton.setVisible (false);
    saveAndOpenButton.setVisible (false);
   #endif
    //[/UserPreSize]

    setSize (808, 638);

    //[Constructor]
    project.addChangeListener (this);
    //[/Constructor]
}

ProjectInformationComponent::~ProjectInformationComponent()
{
    //[Destructor]
    project.removeChangeListener (this);
    //[/Destructor]
}

//==============================================================================
void ProjectInformationComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == &openProjectButton)
    {
        //[UserButtonCode_openProjectButton] -- add your button handler code here..
        //[/UserButtonCode_openProjectButton]
    }
    else if (buttonThatWasClicked == &saveAndOpenButton)
    {
        //[UserButtonCode_saveAndOpenButton] -- add your button handler code here..
        //[/UserButtonCode_saveAndOpenButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}

void ProjectInformationComponent::paint (Graphics& g)
{
    //[UserPaint]
    g.setTiledImageFill (ImageCache::getFromMemory (BinaryData::brushed_aluminium_png, BinaryData::brushed_aluminium_pngSize),
                         0, 0, 1.0f);
    g.fillAll();
    drawRecessedShadows (g, getWidth(), getHeight(), 14);
    //[/UserPaint]
}

//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void ProjectInformationComponent::changeListenerCallback (ChangeBroadcaster*)
{
    dynamic_cast<ProjectSettingsComponent*> (viewport.getViewedComponent())->update();
}
//[/MiscUserCode]

//==============================================================================
//=======================  Jucer Information Section  ==========================
//==============================================================================
#if 0
/*  This section stores the metadata for this component - edit it at your own risk!

JUCER_COMPONENT_METADATA_START

<COMPONENT id="tO9EG1a" className="ProjectInformationComponent" width="808"
           height="638" background="f6f9ff" parentClasses="public Component, public ChangeListener"
           constructorParams="Project&amp; project_" memberInitialisers="project (project_)">
  <COMPONENTS>
    <VIEWPORT id="ykdBpb" memberName="viewport" position="8, 8, parent.width - 8, parent.height - 74"
              scrollBarV="1" scrollBarH="1" scrollbarWidth="16"/>
    <TEXTBUTTON id="a550a652e2666ee7" memberName="openProjectButton" focusOrder="0"
                text="Open Project in " createCallback="1" radioGroup="0" connectedLeft="0"
                connectedRight="0" connectedTop="0" connectedBottom="0" backgroundColour="FFDDDDFF"
                textColour="" backgroundColourOn="" textColourOn="" position="8, parent.height - 34, left + 227, top + 24"/>
    <TEXTBUTTON id="dRGMyYx" name="" memberName="saveAndOpenButton" position="8, parent.height - 65, left + 227, top + 24"
                text="Save And Open in" createCallback="1" radioGroup="0" connectedLeft="0"
                connectedRight="0" connectedTop="0" connectedBottom="0" backgroundColour="FFDDDDFF"/>
    <GENERICCOMPONENT id="QqLJBF" memberName="rollover" position="246, parent.height - 68, parent.width - 8, parent.height - 4"
                      class="RolloverHelpComp" canBeAggregated="1" constructorParams=""/>
  </COMPONENTS>
  <MARKERS_X/>
  <MARKERS_Y/>
  <METHODS paint="1"/>
</COMPONENT>

JUCER_COMPONENT_METADATA_END
*/
#endif

void ProjectInformationComponent::initialiseComponentState()
{

    BinaryData::ImageProvider imageProvider;
    ComponentBuilder::initialiseFromValueTree (*this, getComponentState(), &imageProvider);
}

ValueTree ProjectInformationComponent::getComponentState()
{

    const unsigned char data[] =
        "COMPONENT\0\x01\x08id\0\x01\t\x05tO9EG1a\0""className\0\x01\x1d\x05ProjectInformationComponent\0width\0\x01\x05\x05""808\0height\0\x01\x05\x05""638\0""background\0\x01\x08\x05""f6f9ff\0parentClasses\0\x01)\x05public Component, public ChangeListener\0"
        "constructorParams\0\x01\x13\x05Project& project_\0memberInitialisers\0\x01\x14\x05project (project_)\0\x01\x04""COMPONENTS\0\0\x01\x04VIEWPORT\0\x01\x06id\0\x01\x08\x05ykdBpb\0memberName\0\x01\n\x05viewport\0position\0\x01,\x05""8, 8, parent.width - "
        "8, parent.height - 74\0scrollBarV\0\x01\x03\x05""1\0scrollBarH\0\x01\x03\x05""1\0scrollbarWidth\0\x01\x04\x05""16\0\0TEXTBUTTON\0\x01\x0fid\0\x01\x12\x05""a550a652e2666ee7\0memberName\0\x01\x13\x05openProjectButton\0""focusOrder\0\x01\x03\x05""0\0tex"
        "t\0\x01\x12\x05Open Project in \0""createCallback\0\x01\x03\x05""1\0radioGroup\0\x01\x03\x05""0\0""connectedLeft\0\x01\x03\x05""0\0""connectedRight\0\x01\x03\x05""0\0""connectedTop\0\x01\x03\x05""0\0""connectedBottom\0\x01\x03\x05""0\0""backgroundCol"
        "our\0\x01\n\x05""FFDDDDFF\0textColour\0\x01\x02\x05\0""backgroundColourOn\0\x01\x02\x05\0textColourOn\0\x01\x02\x05\0position\0\x01-\x05""8, parent.height - 34, left + 227, top + 24\0\0TEXTBUTTON\0\x01\x0cid\0\x01\t\x05""dRGMyYx\0name\0\x01\x02\x05\0"
        "memberName\0\x01\x13\x05saveAndOpenButton\0position\0\x01-\x05""8, parent.height - 65, left + 227, top + 24\0text\0\x01\x12\x05Save And Open in\0""createCallback\0\x01\x03\x05""1\0radioGroup\0\x01\x03\x05""0\0""connectedLeft\0\x01\x03\x05""0\0""conne"
        "ctedRight\0\x01\x03\x05""0\0""connectedTop\0\x01\x03\x05""0\0""connectedBottom\0\x01\x03\x05""0\0""backgroundColour\0\x01\n\x05""FFDDDDFF\0\0GENERICCOMPONENT\0\x01\x06id\0\x01\x08\x05QqLJBF\0memberName\0\x01\n\x05rollover\0position\0\x01>\x05""246, p"
        "arent.height - 68, parent.width - 8, parent.height - 4\0""class\0\x01\x12\x05RolloverHelpComp\0""canBeAggregated\0\x01\x03\x05""1\0""constructorParams\0\x01\x02\x05\0\0MARKERS_X\0\0\0MARKERS_Y\0\0\0METHODS\0\x01\x01paint\0\x01\x03\x05""1\0\0";

    return ValueTree::readFromData (data, sizeof (data));
}
