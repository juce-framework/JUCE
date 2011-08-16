/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created for JUCE version: JUCE v1.53.8

  ------------------------------------------------------------------------------

  JUCE and the Jucer are copyright 2004-10 by Raw Material Software ltd.

  ==============================================================================
*/

//[CppHeaders] You can add your own extra header files here...
#include "jucer_ProjectExporter.h"
#include "jucer_Module.h"
//[/CppHeaders]

#include "jucer_ProjectInformationComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...

//==============================================================================
class PanelBase   : public PropertyPanelWithTooltips
{
public:
    PanelBase (Project& project_) : project (project_) {}

    virtual void rebuildProperties (Array <PropertyComponent*>& props) = 0;

    void visibilityChanged()
    {
        if (isVisible())
            refreshAll();
    }

    void refreshAll()
    {
        getPanel().clear();
        Array <PropertyComponent*> props;
        rebuildProperties (props);
        getPanel().addProperties (props);
    }

protected:
    Project& project;
};

//==============================================================================
class ProjectTab  : public PanelBase
{
public:
    ProjectTab (Project& project_) : PanelBase (project_) {}

    void rebuildProperties (Array <PropertyComponent*>& props)
    {
        project.createPropertyEditors (props);
    }
};

//==============================================================================
class ConfigTab  : public PanelBase
{
public:
    ConfigTab (Project& project_, int configIndex_)
        : PanelBase (project_), configIndex (configIndex_)
    {
    }

    void rebuildProperties (Array <PropertyComponent*>& props)
    {
        project.getConfiguration (configIndex).createPropertyEditors (props);
    }

private:
    int configIndex;
};

//==============================================================================
class ExportTab  : public PanelBase
{
public:
    ExportTab (Project& project_, int exporterIndex_)
        : PanelBase (project_), exporterIndex (exporterIndex_)
    {
    }

    void rebuildProperties (Array <PropertyComponent*>& props)
    {
        ScopedPointer <ProjectExporter> exp (project.createExporter (exporterIndex));

        if (exp != nullptr)
            exp->createPropertyEditors (props);

        for (int i = props.size(); --i >= 0;)
            props.getUnchecked(i)->setPreferredHeight (22);
    }

private:
    int exporterIndex;
};

//==============================================================================
static StringArray getExtraDependenciesNeeded (Project& project, const ModuleList::Module& m)
{
    StringArray dependencies, extraDepsNeeded;
    ModuleList::getInstance().getDependencies (m.uid, dependencies);

    for (int i = 0; i < dependencies.size(); ++i)
        if ((! project.isModuleEnabled (dependencies[i])) && dependencies[i] != m.uid)
            extraDepsNeeded.add (dependencies[i]);

    return extraDepsNeeded;
}

//==============================================================================
class ModuleSettingsPanel  : public PanelBase
{
public:
    ModuleSettingsPanel (Project& project_, const String& moduleID_)
        : PanelBase (project_), moduleID (moduleID_)
    {
        setBounds ("parent.width / 2 + 1, 3, parent.width - 3, parent.height - 3");
    }

    void rebuildProperties (Array <PropertyComponent*>& props)
    {
        ScopedPointer<LibraryModule> module (ModuleList::getInstance().loadModule (moduleID));

        if (module != nullptr)
        {
            props.add (new ModuleInfoComponent (project, moduleID));

            if (project.isModuleEnabled (moduleID))
            {
                const ModuleList::Module* m = ModuleList::getInstance().findModuleInfo (moduleID);
                if (m != nullptr && getExtraDependenciesNeeded (project, *m).size() > 0)
                    props.add (new MissingDependenciesComponent (project, moduleID));
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

        setEnabled (project.isModuleEnabled (moduleID));
    }

private:
    String moduleID;

    //==============================================================================
    class ModuleInfoComponent  : public PropertyComponent
    {
    public:
        ModuleInfoComponent (Project& project_, const String& moduleID_)
            : PropertyComponent ("Module", 100),
              project (project_), moduleID (moduleID_)
        {
        }

        void refresh() {}

        void paint (Graphics& g)
        {
            g.setColour (Colours::white.withAlpha (0.4f));
            g.fillRect (0, 0, getWidth(), getHeight() - 1);

            const ModuleList::Module* module = ModuleList::getInstance().findModuleInfo (moduleID);

            if (module != nullptr)
            {
                String text;
                text << module->name << newLine << newLine
                     << module->description;

                GlyphArrangement ga;
                ga.addJustifiedText (Font (13.0f), text, 4.0f, 16.0f, getWidth() - 8.0f, Justification::topLeft);
                g.setColour (Colours::black);
                ga.draw (g);
            }
        }

    private:
        Project& project;
        String moduleID;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModuleInfoComponent);
    };

    //==============================================================================
    class MissingDependenciesComponent  : public PropertyComponent,
                                          public Button::Listener
    {
    public:
        MissingDependenciesComponent (Project& project_, const String& moduleID_)
            : PropertyComponent ("Dependencies", 100),
              project (project_), moduleID (moduleID_),
              fixButton ("Enable Required Modules")
        {
            const ModuleList::Module* module = ModuleList::getInstance().findModuleInfo (moduleID);

            if (module != nullptr)
                missingDependencies = getExtraDependenciesNeeded (project, *module);

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

        void buttonClicked (Button*);

    private:
        Project& project;
        String moduleID;
        StringArray missingDependencies;
        TextButton fixButton;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MissingDependenciesComponent);
    };
};


//==============================================================================
class ModulesPanel  : public Component,
                      public ListBoxModel
{
public:
    ModulesPanel (Project& project_)
        : project (project_)
    {
        modulesList.setModel (this);
        modulesList.setColour (ListBox::backgroundColourId, Colours::white.withAlpha (0.4f));
        addAndMakeVisible (&modulesList);
        modulesList.setBounds ("4, 3, parent.width / 2 - 4, parent.height - 3");
    }

    int getNumRows()
    {
        return ModuleList::getInstance().modules.size();
    }

    void paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
    {
        if (rowIsSelected)
            g.fillAll (findColour (TextEditor::highlightColourId));

        const ModuleList::Module* const m = ModuleList::getInstance().modules [rowNumber];

        if (m != nullptr)
        {
            const float tickSize = height * 0.7f;

            getLookAndFeel().drawTickBox (g, *this, (height - tickSize) / 2, (height - tickSize) / 2, tickSize, tickSize,
                                          project.isModuleEnabled (m->uid), true, false, false);

            if (project.isModuleEnabled (m->uid) && getExtraDependenciesNeeded (project, *m).size() > 0)
                g.setColour (Colours::red);
            else
                g.setColour (Colours::black);

            g.setFont (height * 0.7f, Font::bold);
            g.drawFittedText (m->uid, height, 0, 200, height, Justification::centredLeft, 1);

            g.setFont (height * 0.55f, Font::italic);
            g.drawText (m->name, height + 200, 0, width - height - 200, height, Justification::centredLeft, true);
        }
    }

    void flipRow (int row)
    {
        const ModuleList::Module* const m = ModuleList::getInstance().modules [row];

        if (m != nullptr)
        {
            if (project.isModuleEnabled (m->uid))
            {
                project.removeModule (m->uid);
            }
            else
            {
                const StringArray extraDepsNeeded (getExtraDependenciesNeeded (project, *m));

/*                if (extraDepsNeeded.size() > 0)
                {
                    if (AlertWindow::showOkCancelBox (AlertWindow::NoIcon,
                                                      "Module Dependencies",
                                                      "The '" + m->uid + "' module requires the following dependencies:\n"
                                                        + extraDepsNeeded.joinIntoString (", ") + "\n\nDo you want to add all these to your project?"))
                    {
                        project.addModule (m->uid);

                        for (int i = extraDepsNeeded.size(); --i >= 0;)
                            project.addModule (extraDepsNeeded[i]);
                    }
                }
                else*/
                {
                    project.addModule (m->uid);
                }
            }
        }

        refresh();
    }

    void listBoxItemClicked (int row, const MouseEvent& e)
    {
        if (e.x < modulesList.getRowHeight())
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
        const ModuleList::Module* const m = ModuleList::getInstance().modules [lastRowSelected];

        settings = nullptr;

        if (m != nullptr)
            addAndMakeVisible (settings = new ModuleSettingsPanel (project, m->uid));
    }

    void refresh()
    {
        modulesList.repaint();

        if (settings != nullptr)
            settings->refreshAll();
    }

private:
    Project& project;
    ListBox modulesList;
    ScopedPointer<ModuleSettingsPanel> settings;
};

void ModuleSettingsPanel::MissingDependenciesComponent::buttonClicked (Button*)
{
    for (int i = missingDependencies.size(); --i >= 0;)
        project.addModule (missingDependencies[i]);

    ModulesPanel* mp = findParentComponentOfClass ((ModulesPanel*) 0);
    if (mp != nullptr)
        mp->refresh();
}

//[/MiscUserDefs]

//==============================================================================
ProjectInformationComponent::ProjectInformationComponent (Project& project_)
    : project (project_),
      configTabBox (TabbedButtonBar::TabsAtTop)
{
    addAndMakeVisible (&configTabBox);
    configTabBox.setBounds ("8, 0, this.left + parent.width - 16, this.top + parent.height - 36");
    addAndMakeVisible (&editConfigsButton);
    editConfigsButton.setBounds ("8, parent.height - 30, this.left + 192, this.top + 22");
    editConfigsButton.setButtonText ("Add/Remove Configurations...");
    editConfigsButton.addListener (this);
    addAndMakeVisible (&openProjectButton);
    openProjectButton.setBounds ("608, parent.height - 30, this.left + 208, this.top + 22");
    openProjectButton.setButtonText ("Open Project in ");
    openProjectButton.addListener (this);
    addAndMakeVisible (&editExportersButton);
    editExportersButton.setBounds ("208, parent.height - 30, this.left + 160, this.top + 22");
    editExportersButton.setButtonText ("Add/Remove Exporters...");
    editExportersButton.addListener (this);
    addAndMakeVisible (&saveAndOpenButton);
    saveAndOpenButton.setBounds ("391, parent.height - 30, this.left + 208, this.top + 22");
    saveAndOpenButton.setButtonText ("Save And Open in");
    saveAndOpenButton.addListener (this);

    //[UserPreSize]
    rebuildConfigTabs();

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

    setSize (836, 427);

    //[Constructor] You can add your own custom stuff here..
    configTabBox.setOutline (1);
    configTabBox.setColour (TabbedComponent::outlineColourId, Colours::black.withAlpha (0.3f));

    editConfigsButton.setTriggeredOnMouseDown (true);

    project.addChangeListener (this);
    //[/Constructor]
}

ProjectInformationComponent::~ProjectInformationComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    project.removeChangeListener (this);
    //[/Destructor_pre]



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

    if (buttonThatWasClicked == &editConfigsButton)
    {
        //[UserButtonCode_b6625dfcdb1f4755] -- add your button handler code here..
        showConfigMenu();
        //[/UserButtonCode_b6625dfcdb1f4755]
    }
    else if (buttonThatWasClicked == &openProjectButton)
    {
        //[UserButtonCode_a550a652e2666ee7] -- add your button handler code here..
        //[/UserButtonCode_a550a652e2666ee7]
    }
    else if (buttonThatWasClicked == &editExportersButton)
    {
        //[UserButtonCode_c1f6e5f9811b307e] -- add your button handler code here..
        showExporterMenu();
        //[/UserButtonCode_c1f6e5f9811b307e]
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
void ProjectInformationComponent::rebuildConfigTabs()
{
    configTabBox.clearTabs();

    configTabBox.addTab ("Project Settings", Colours::lightslategrey, new ProjectTab (project), true, -1);
    configTabBox.addTab ("Modules", Colours::lightblue, new ModulesPanel (project), true, -1);

    int i;
    for (i = 0; i < project.getNumConfigurations(); ++i)
    {
        Component* panel = new ConfigTab (project, i);
        Project::BuildConfiguration config (project.getConfiguration (i));
        configTabBox.addTab (config.getName().toString(), Colour::greyLevel (0.65f), panel, true, -1);
    }

    for (i = 0; i < project.getNumExporters(); ++i)
    {
        ScopedPointer <ProjectExporter> exp (project.createExporter (i));

        if (exp != nullptr)
        {
            Component* panel = new ExportTab (project, i);
            configTabBox.addTab (exp->getName(), Colours::lightsteelblue, panel, true, -1);
        }
    }

    lastProjectType = project.getProjectTypeValue().getValue();
}

void ProjectInformationComponent::updateConfigTabs()
{
    if (configTabBox.getNumTabs() != project.getNumConfigurations() + project.getNumExporters() + 2
         || lastProjectType != project.getProjectTypeValue().getValue())
    {
        rebuildConfigTabs();
    }
    else
    {
        for (int i = 0; i < project.getNumConfigurations(); ++i)
        {
            Project::BuildConfiguration config (project.getConfiguration (i));
            configTabBox.setTabName (i + 2, config.getName().toString());
        }
    }
}

void ProjectInformationComponent::showConfigMenu()
{
    PopupMenu m;
    m.addItem (1, "Add a new empty configuration");

    PopupMenu createCopyMenu, removeMenu;

    for (int i = 0; i < project.getNumConfigurations(); ++i)
    {
        Project::BuildConfiguration config (project.getConfiguration (i));
        createCopyMenu.addItem (i + 10000, "Create a copy of '" + config.getName().toString() + "'");
        removeMenu.addItem (i + 20000, "Delete configuration '" + config.getName().toString() + "'");
    }

    m.addSubMenu ("Add a copy of an existing configuration", createCopyMenu);
    m.addSubMenu ("Remove configuration", removeMenu);

    const int r = m.showAt (&editConfigsButton);

    if (r >= 20000)
    {
        project.deleteConfiguration (r - 20000);
    }
    else if (r >= 10000)
    {
        Project::BuildConfiguration configToCopy (project.getConfiguration (r - 10000));
        project.addNewConfiguration (&configToCopy);
    }
    else if (r == 1)
    {
        project.addNewConfiguration (nullptr);
    }
}

void ProjectInformationComponent::showExporterMenu()
{
    PopupMenu m;

    PopupMenu createMenu, removeMenu;

    int i;
    for (i = 0; i < project.getNumExporters(); ++i)
    {
        ScopedPointer<ProjectExporter> exp (project.createExporter (i));

        if (exp != nullptr)
            removeMenu.addItem (i + 20000, "Delete " + exp->getName());
    }

    StringArray exporters (ProjectExporter::getExporterNames());

    for (i = 0; i < exporters.size(); ++i)
        createMenu.addItem (i + 10000, "Create a new " + exporters[i] + " target");

    m.addSubMenu ("Create new export target", createMenu);
    m.addSubMenu ("Remove export target", removeMenu);

    const int r = m.showAt (&editExportersButton);

    if (r >= 20000)
        project.deleteExporter (r - 20000);
    else if (r >= 10000)
        project.addNewExporter (exporters [r - 10000]);
}

void ProjectInformationComponent::changeListenerCallback (ChangeBroadcaster*)
{
    updateConfigTabs();
}
//[/MiscUserCode]



//==============================================================================
//=======================  Jucer Information Section  ==========================
//==============================================================================
#if 0
/*  This section stores the Jucer's metadata - edit it at your own risk!

JUCER_COMPONENT_METADATA_START

<COMPONENT id="tO9EG1a" className="ProjectInformationComponent" width="836"
           height="427" background="f6f9ff" parentClasses="public Component, public ChangeListener"
           constructorParams="Project&amp; project_" memberInitialisers="project (project_)">
  <COMPONENTS>
    <TABBEDCOMPONENT id="962c1575c4142253" memberName="configTabBox" focusOrder="0"
                     position="8, 0, this.left + parent.width - 16, this.top + parent.height - 36"/>
    <TEXTBUTTON id="b6625dfcdb1f4755" memberName="editConfigsButton" focusOrder="0"
                text="Add/Remove Configurations..." createCallback="1" radioGroup="0"
                connectedLeft="0" connectedRight="0" connectedTop="0" connectedBottom="0"
                backgroundColour="" textColour="" backgroundColourOn="" textColourOn=""
                position="8, parent.height - 30, this.left + 192, this.top + 22"/>
    <TEXTBUTTON id="a550a652e2666ee7" memberName="openProjectButton" focusOrder="0"
                text="Open Project in " createCallback="1" radioGroup="0" connectedLeft="0"
                connectedRight="0" connectedTop="0" connectedBottom="0" backgroundColour=""
                textColour="" backgroundColourOn="" textColourOn="" position="608, parent.height - 30, this.left + 208, this.top + 22"/>
    <TEXTBUTTON id="c1f6e5f9811b307e" memberName="editExportersButton" focusOrder="0"
                text="Add/Remove Exporters..." createCallback="1" radioGroup="0"
                connectedLeft="0" connectedRight="0" connectedTop="0" connectedBottom="0"
                backgroundColour="" textColour="" backgroundColourOn="" textColourOn=""
                position="208, parent.height - 30, this.left + 160, this.top + 22"/>
    <TEXTBUTTON id="dRGMyYx" name="" memberName="saveAndOpenButton" position="391, parent.height - 30, this.left + 208, this.top + 22"
                text="Save And Open in" createCallback="1" radioGroup="0" connectedLeft="0"
                connectedRight="0" connectedTop="0" connectedBottom="0"/>
  </COMPONENTS>
  <MARKERS_X/>
  <MARKERS_Y/>
  <METHODS/>
</COMPONENT>

JUCER_COMPONENT_METADATA_END
*/
#endif
