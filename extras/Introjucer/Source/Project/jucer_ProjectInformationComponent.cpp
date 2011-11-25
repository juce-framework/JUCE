/*
  ==============================================================================

  This is an automatically generated file!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created for JUCE version: JUCE v2.0.9

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
          moduleListBox (moduleList)
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
        moduleListBox.setBounds ("4, 31, parent.width / 2 - 4, parent.height - 3");
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
            addAndMakeVisible (settings = new ModuleSettingsPanel (project, moduleList, selectedModule->uid));
    }

    void refresh()
    {
        moduleListBox.refresh();

        if (settings != nullptr)
            settings->refreshAll();
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
            setBounds ("parent.width / 2 + 1, 31, parent.width - 3, parent.height - 3");
            refreshAll();
        }

        void refreshAll()
        {
            setEnabled (project.isModuleEnabled (moduleID));

            clear();
            Array <PropertyComponent*> props;

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
                                                         "Add source to project", "Make module files browsable in projects"));
                props.getLast()->setTooltip ("If this is enabled, then the entire source tree from this module will be shown inside your project, "
                                             "making it easy to browse/edit the module's classes. If disabled, then only the minimum number of files "
                                             "required to compile it will appear inside your project.");

                props.add (new BooleanPropertyComponent (project.shouldCopyModuleFilesLocally (moduleID),
                                                         "Create local copy", "Copy the module into the project folder"));
                props.getLast()->setTooltip ("If this is enabled, then a local copy of the entire module will be made inside your project (in the auto-generated JuceLibraryFiles folder), "
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

            addProperties (props);
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

                ModulesPanel* mp = findParentComponentOfClass ((ModulesPanel*) nullptr);
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

private:
    Project& project;
    ModuleList moduleList;
    FilenameComponent modulesLocation;
    Label modulesLabel;
    TextButton updateModulesButton;
    ModuleSelectionListBox moduleListBox;
    ScopedPointer<ModuleSettingsPanel> settings;
};


//==============================================================================
class ProjectSettingsComponent  : public Component
{
public:
    ProjectSettingsComponent (Project& project_)
        : project (project_),
          configs ("Configurations", "Add a New Configuration", false),
          exporters ("Export Targets", "Add a New Exporter...", true)
    {
        addAndMakeVisible (&mainProjectInfoPanel);
        addAndMakeVisible (&modulesPanelGroup);
        addAndMakeVisible (&configs);
        addAndMakeVisible (&exporters);

        Array<PropertyComponent*> props;
        props.add (new ModulesPanel (project));
        modulesPanelGroup.setProperties (props);
        modulesPanelGroup.setName ("Modules");

        createItems();
    }

    void updateSize (int width)
    {
        width = jmax (550, width);

        int y = 0;
        y += mainProjectInfoPanel.updateSize (y, width);
        y += modulesPanelGroup.updateSize (y, width);
        y += configs.updateSize (y, width);
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
            refreshAll();
    }

    void refreshAll()
    {
        {
            Array <PropertyComponent*> props;
            project.createPropertyEditors (props);
            mainProjectInfoPanel.setProperties (props);
            mainProjectInfoPanel.setName ("Project Settings");
        }

        int i;
        for (i = configs.groups.size(); --i >= 0;)
        {
            PropertyGroup& pp = *configs.groups.getUnchecked(i);

            Array <PropertyComponent*> props;
            project.getConfiguration (i).createPropertyEditors (props);
            pp.setProperties (props);
        }

        for (i = exporters.groups.size(); --i >= 0;)
        {
            PropertyGroup& pp = *exporters.groups.getUnchecked(i);
            Array <PropertyComponent*> props;
            ScopedPointer <ProjectExporter> exp (project.createExporter (i));

            jassert (exp != nullptr);
            if (exp != nullptr)
            {
                exp->createPropertyEditors (props);

                for (int j = props.size(); --j >= 0;)
                    props.getUnchecked(j)->setPreferredHeight (22);

                pp.setProperties (props);
            }
        }

        refreshSectionNames();

        updateSize (getWidth());
    }

    void refreshSectionNames()
    {
        int i;
        for (i = configs.groups.size(); --i >= 0;)
        {
            PropertyGroup& pp = *configs.groups.getUnchecked(i);
            pp.setName (project.getConfiguration (i).getName().toString().quoted());
            pp.repaint();
        }

        for (i = exporters.groups.size(); --i >= 0;)
        {
            PropertyGroup& pp = *exporters.groups.getUnchecked(i);
            ScopedPointer <ProjectExporter> exp (project.createExporter (i));

            jassert (exp != nullptr);
            if (exp != nullptr)
                pp.setName (exp->getName());

            pp.repaint();
        }
    }

    void createItems()
    {
        configs.clear();
        exporters.clear();

        int i;
        for (i = 0; i < project.getNumConfigurations(); ++i)
        {
            PropertyGroup* p = configs.createGroup();

            if (project.getNumConfigurations() > 1)
                p->addDeleteButton ("config " + String (i), "Deletes this configuration.");
        }

        for (i = 0; i < project.getNumExporters(); ++i)
        {
            PropertyGroup* p = exporters.createGroup();
            p->addDeleteButton ("exporter " + String (i), "Deletes this export target.");
        }

        lastProjectType = project.getProjectTypeValue().getValue();
        refreshAll();
    }

    void update()
    {
        if (configs.groups.size() != project.getNumConfigurations()
             || exporters.groups.size() != project.getNumExporters()
             || lastProjectType != project.getProjectTypeValue().getValue())
        {
            createItems();
        }

        refreshSectionNames();
    }

    void deleteButtonClicked (const String& name)
    {
        if (name.startsWith ("config"))
            project.deleteConfiguration (name.getTrailingIntValue());
        else
            project.deleteExporter (name.getTrailingIntValue());
    }

    void createNewExporter (TextButton& button)
    {
        StringArray exporters (ProjectExporter::getExporterNames());
        PopupMenu menu;

        for (int i = 0; i < exporters.size(); ++i)
            menu.addItem (i + 1, "Create a new " + exporters[i] + " target");

        const int r = menu.showAt (&button);

        if (r > 0)
            project.addNewExporter (exporters [r - 1]);
    }

    void createNewConfig()
    {
        project.addNewConfiguration (nullptr);
    }

    void newItemButtonClicked (TextButton& button)
    {
        if (button.getName().containsIgnoreCase ("export"))
            createNewExporter (button);
        else
            createNewConfig();
    }

private:
    //==============================================================================
    class PropertyGroup  : public Component,
                           public ButtonListener
    {
    public:
        PropertyGroup()
            : deleteButton ("Delete"), preferredHeight (0)
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

        void setProperties (const Array<PropertyComponent*>& newProps)
        {
            properties.clear();
            properties.addArray (newProps);

            preferredHeight = 32;
            for (int i = properties.size(); --i >= 0;)
            {
                addAndMakeVisible (properties.getUnchecked(i));
                preferredHeight += properties.getUnchecked(i)->getPreferredHeight();
            }
        }

        int getPreferredHeight() const
        {
            return preferredHeight;
        }

        int updateSize (int y, int width)
        {
            setBounds (0, y, width, preferredHeight);

            y = 30;
            for (int i = 0; i < properties.size(); ++i)
            {
                PropertyComponent* pp = properties.getUnchecked(i);
                pp->setBounds (10, y, width - 20, pp->getPreferredHeight());
                y += pp->getHeight();
            }

            return preferredHeight;
        }

        void paint (Graphics& g)
        {
            g.setFont (14.0f, Font::bold);
            g.setColour (Colours::black);
            g.drawFittedText (getName(), 12, 0, getWidth() - 16, 28, Justification::bottomLeft, 1);
        }

        void buttonClicked (Button*)
        {
            ProjectSettingsComponent* psc = findParentComponentOfClass ((ProjectSettingsComponent*) nullptr);
            if (psc != nullptr)
                psc->deleteButtonClicked (deleteButton.getName());
        }

    private:
        OwnedArray<PropertyComponent> properties;
        TextButton deleteButton;
        int preferredHeight;
    };

    //==============================================================================
    class PropertyGroupList  : public Component,
                               public ButtonListener
    {
    public:
        PropertyGroupList (const String& title, const String& newButtonText, bool triggerOnMouseDown)
            : Component (title), createNewButton (newButtonText)
        {
            addAndMakeVisible (&createNewButton);
            createNewButton.setColour (TextButton::buttonColourId, Colours::lightgreen.withAlpha (0.5f));
            createNewButton.setBounds ("right - 140, 30, parent.width - 10, top + 20");
            createNewButton.setConnectedEdges (Button::ConnectedOnLeft | Button::ConnectedOnRight);
            createNewButton.addListener (this);
            createNewButton.setTriggeredOnMouseDown (triggerOnMouseDown);
        }

        int updateSize (int ourY, int width)
        {
            int y = 55;

            for (int i = 0; i < groups.size(); ++i)
                y += groups.getUnchecked(i)->updateSize (y, width);

            y = jmax (y, 100);
            setBounds (0, ourY, width, y);
            return y;
        }

        void paint (Graphics& g)
        {
            g.setFont (17.0f, Font::bold);
            g.setColour (Colours::black);
            g.drawFittedText (getName(), 0, 30, getWidth(), 20, Justification::centred, 1);
        }

        void clear()
        {
            groups.clear();
        }

        PropertyGroup* createGroup()
        {
            PropertyGroup* p = new PropertyGroup();
            groups.add (p);
            addAndMakeVisible (p);
            return p;
        }

        void buttonClicked (Button*)
        {
            ProjectSettingsComponent* psc = findParentComponentOfClass ((ProjectSettingsComponent*) nullptr);
            if (psc != nullptr)
                psc->newItemButtonClicked (createNewButton);
        }

        OwnedArray<PropertyGroup> groups;
        TextButton createNewButton;
    };

    Project& project;
    var lastProjectType;
    PropertyGroup mainProjectInfoPanel, modulesPanelGroup;
    PropertyGroupList configs, exporters;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectSettingsComponent);
};

//==============================================================================
class ProjectInformationComponent::RolloverHelpComp   : public Component,
                                                        private Timer
{
public:
    RolloverHelpComp()
        : lastComp (nullptr)
    {
        startTimer (150);
    }

    void paint (Graphics& g)
    {
        AttributedString s;
        s.setJustification (Justification::centredLeft);
        s.append (lastTip, Font (14.0f), Colour::greyLevel (0.15f));

        TextLayout tl;
        tl.createLayoutWithBalancedLineLengths (s, getWidth() - 10.0f);
        if (tl.getNumLines() > 3)
            tl.createLayout (s, getWidth() - 10.0f);

        tl.draw (g, getLocalBounds().toFloat());
    }

    void timerCallback()
    {
        Component* newComp = Desktop::getInstance().getMainMouseSource().getComponentUnderMouse();

        if (newComp != nullptr
             && (newComp->getTopLevelComponent() != getTopLevelComponent()
                  || newComp->isCurrentlyBlockedByAnotherModalComponent()))
            newComp = nullptr;

        if (newComp != lastComp)
        {
            lastComp = newComp;

            String newTip (findTip (newComp));

            if (newTip != lastTip)
            {
                lastTip = newTip;
                repaint();
            }
        }
    }

private:
    static String findTip (Component* c)
    {
        while (c != nullptr)
        {
            TooltipClient* const tc = dynamic_cast <TooltipClient*> (c);
            if (tc != nullptr)
            {
                const String tip (tc->getTooltip());

                if (tip.isNotEmpty())
                    return tip;
            }

            c = c->getParentComponent();
        }

        return String::empty;
    }

    Component* lastComp;
    String lastTip;
};

//[/MiscUserDefs]

//==============================================================================
ProjectInformationComponent::ProjectInformationComponent (Project& project_)
    : project (project_)
{
    addAndMakeVisible (&viewport);
    viewport.setComponentID ("ykdBpb");
    viewport.setBounds (RelativeRectangle ("8, 8, parent.width - 8, parent.height - 74"));
    viewport.setScrollBarThickness (16);
    addAndMakeVisible (&openProjectButton);
    openProjectButton.setComponentID ("a550a652e2666ee7");
    openProjectButton.setBounds (RelativeRectangle ("8, parent.height - 34, left + 227, top + 24"));
    openProjectButton.setButtonText ("Open Project in ");
    openProjectButton.addListener (this);
    openProjectButton.setColour (TextButton::buttonColourId, Colour (0xffddddff));
    addAndMakeVisible (&saveAndOpenButton);
    saveAndOpenButton.setComponentID ("dRGMyYx");
    saveAndOpenButton.setBounds (RelativeRectangle ("8, parent.height - 65, left + 227, top + 24"));
    saveAndOpenButton.setButtonText ("Save And Open in");
    saveAndOpenButton.addListener (this);
    saveAndOpenButton.setColour (TextButton::buttonColourId, Colour (0xffddddff));
    addAndMakeVisible (rollover = new RolloverHelpComp());
    rollover->setComponentID ("QqLJBF");
    rollover->setBounds (RelativeRectangle ("246, parent.height - 68, parent.width - 8, parent.height - 4"));

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


    //[Constructor] You can add your own custom stuff here..
    project.addChangeListener (this);
    //[/Constructor]
}

ProjectInformationComponent::~ProjectInformationComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    project.removeChangeListener (this);
    //[/Destructor_pre]

    rollover = 0;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void ProjectInformationComponent::resized()
{
    //[Userresized_Pre]
    //[/Userresized_Pre]



    //[Userresized_Post]
    //[/Userresized_Post]
}

void ProjectInformationComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == &openProjectButton)
    {
        //[UserButtonCode_a550a652e2666ee7] -- add your button handler code here..
        //[/UserButtonCode_a550a652e2666ee7]
    }
    else if (buttonThatWasClicked == &saveAndOpenButton)
    {
        //[UserButtonCode_dRGMyYx] -- add your button handler code here..
        //[/UserButtonCode_dRGMyYx]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}

void ProjectInformationComponent::paint (Graphics& g)
{
    //[UserPaint] Add your own custom painting code here..
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
                      class="RolloverHelpComp"/>
  </COMPONENTS>
  <MARKERS_X/>
  <MARKERS_Y/>
  <METHODS/>
</COMPONENT>

JUCER_COMPONENT_METADATA_END
*/
#endif
