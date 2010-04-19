/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  14 Feb 2010 3:06:06 pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
#include "../model/jucer_ProjectExporter.h"
//[/Headers]

#include "jucer_ProjectInformationComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
class PropertiesWithHelpComponent  : public PropertyPanelWithTooltips
{
public:
    PropertiesWithHelpComponent (Project& project_, int tabIndex_)
        : project (project_), tabIndex (tabIndex_)
    {
    }

    ~PropertiesWithHelpComponent()
    {
    }

    void rebuildProperties()
    {
        getPanel()->clear();
        Array <PropertyComponent*> props;

        if (tabIndex == 0)
        {
            // The main project tab...
            project.createPropertyEditors (props);
        }
        else if (tabIndex == 1)
        {
            // The Juce options tab...
            OwnedArray <Project::JuceConfigFlag> flags;
            project.getJuceConfigFlags (flags);

            StringArray possibleValues;
            possibleValues.add ("Enabled");
            possibleValues.add ("Disabled");
            possibleValues.add ("(Use default from juce_Config.h)");

            for (int i = 0; i < flags.size(); ++i)
            {
                if ((int) flags[i]->value.getValue() == 0)
                    flags[i]->value = 3;

                ChoicePropertyComponent* c = new ChoicePropertyComponent (flags[i]->value, flags[i]->symbol, possibleValues);
                c->setTooltip (flags[i]->description);
                c->setPreferredHeight (22);
                props.add (c);
            }
        }
        else if (tabIndex < 2 + project.getNumConfigurations())
        {
            // A config tab..
            project.getConfiguration (tabIndex - 2).createPropertyEditors (props);
        }
        else
        {
            // An export tab..
            ScopedPointer <ProjectExporter> exp (project.createExporter (tabIndex - (2 + project.getNumConfigurations())));

            if (exp != 0)
                exp->createPropertyEditors (props);
        }

        getPanel()->addProperties (props);
    }

    void visibilityChanged()
    {
        if (isVisible())
            rebuildProperties();
    }

private:
    Project& project;
    int tabIndex;
};

//[/MiscUserDefs]

//==============================================================================
ProjectInformationComponent::ProjectInformationComponent (Project& project_)
    : project (project_),
      configTabBox (0),
      editConfigsButton (0),
      openProjectButton (0),
      editExportersButton (0)
{
    addAndMakeVisible (configTabBox = new TabbedComponent (TabbedButtonBar::TabsAtTop));
    configTabBox->setTabBarDepth (30);
    configTabBox->setCurrentTabIndex (-1);

    addAndMakeVisible (editConfigsButton = new TextButton (String::empty));
    editConfigsButton->setButtonText ("Add/Remove Configurations...");
    editConfigsButton->addButtonListener (this);

    addAndMakeVisible (openProjectButton = new TextButton (String::empty));
    openProjectButton->setButtonText ("Open Project in ");
    openProjectButton->addButtonListener (this);

    addAndMakeVisible (editExportersButton = new TextButton (String::empty));
    editExportersButton->setButtonText ("Add/Remove Exporters...");
    editExportersButton->addButtonListener (this);


    //[UserPreSize]
    rebuildConfigTabs();

#if JUCE_MAC || JUCE_WINDOWS
    openProjectButton->setCommandToTrigger (commandManager, CommandIDs::openProjectInIDE, true);
    openProjectButton->setButtonText (commandManager->getNameOfCommand (CommandIDs::openProjectInIDE));
#else
    openProjectButton->setVisible (false);
#endif

    //[/UserPreSize]

    setSize (600, 400);

    //[Constructor] You can add your own custom stuff here..
    configTabBox->setOutline (1);
    configTabBox->setColour (TabbedComponent::outlineColourId, Colours::black);

    editConfigsButton->setTriggeredOnMouseDown (true);

    project.addChangeListener (this);
    //[/Constructor]
}

ProjectInformationComponent::~ProjectInformationComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    project.removeChangeListener (this);
    //[/Destructor_pre]

    deleteAndZero (configTabBox);
    deleteAndZero (editConfigsButton);
    deleteAndZero (openProjectButton);
    deleteAndZero (editExportersButton);

    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void ProjectInformationComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void ProjectInformationComponent::resized()
{
    configTabBox->setBounds (8, 0, getWidth() - 16, getHeight() - 36);
    editConfigsButton->setBounds (8, getHeight() - 26, 192, 22);
    openProjectButton->setBounds (384, getHeight() - 26, 208, 22);
    editExportersButton->setBounds (208, getHeight() - 26, 160, 22);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void ProjectInformationComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == editConfigsButton)
    {
        //[UserButtonCode_editConfigsButton] -- add your button handler code here..
        showConfigMenu();
        //[/UserButtonCode_editConfigsButton]
    }
    else if (buttonThatWasClicked == openProjectButton)
    {
        //[UserButtonCode_openProjectButton] -- add your button handler code here..
        //[/UserButtonCode_openProjectButton]
    }
    else if (buttonThatWasClicked == editExportersButton)
    {
        //[UserButtonCode_editExportersButton] -- add your button handler code here..
        showExporterMenu();
        //[/UserButtonCode_editExportersButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void ProjectInformationComponent::rebuildConfigTabs()
{
    configTabBox->clearTabs();

    int index = 0;
    PropertiesWithHelpComponent* panel = new PropertiesWithHelpComponent (project, index++);
    configTabBox->addTab ("Project Settings", Colours::lightslategrey, panel, true, -1);

    panel = new PropertiesWithHelpComponent (project, index++);
    configTabBox->addTab ("Juce Flags", Colours::lightblue, panel, true, -1);

    int i;
    for (i = 0; i < project.getNumConfigurations(); ++i)
    {
        panel = new PropertiesWithHelpComponent (project, index++);
        Project::BuildConfiguration config (project.getConfiguration (i));
        configTabBox->addTab (config.getName().toString(), Colour::greyLevel (0.65f), panel, true, -1);
    }

    for (i = 0; i < project.getNumExporters(); ++i)
    {
        ScopedPointer <ProjectExporter> exp (project.createExporter (i));

        if (exp != 0)
        {
            panel = new PropertiesWithHelpComponent (project, index++);
            configTabBox->addTab (exp->getName(), Colours::lightsteelblue, panel, true, -1);
        }
    }

    lastProjectType = (Project::ProjectType) (int) project.getProjectType().getValue();
}

void ProjectInformationComponent::updateConfigTabs()
{
    if (configTabBox->getNumTabs() != project.getNumConfigurations() + project.getNumExporters() + 2
         || lastProjectType != (Project::ProjectType) (int) project.getProjectType().getValue())
    {
        rebuildConfigTabs();
    }
    else
    {
        for (int i = 0; i < project.getNumConfigurations(); ++i)
        {
            Project::BuildConfiguration config (project.getConfiguration (i));
            configTabBox->setTabName (i + 2, config.getName().toString());
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

    const int r = m.showAt (editConfigsButton);

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
        project.addNewConfiguration (0);
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

        if (exp != 0)
            removeMenu.addItem (i + 20000, "Delete " + exp->getName());
    }

    StringArray exporters (ProjectExporter::getExporterNames());

    for (i = 0; i < exporters.size(); ++i)
        createMenu.addItem (i + 10000, "Create a new " + exporters[i] + " target");

    m.addSubMenu ("Create new export target", createMenu);
    m.addSubMenu ("Remove export target", removeMenu);

    const int r = m.showAt (editExportersButton);

    if (r >= 20000)
        project.deleteExporter (r - 20000);
    else if (r >= 10000)
        project.addNewExporter (r - 10000);
}

void ProjectInformationComponent::changeListenerCallback (void*)
{
    updateConfigTabs();
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Jucer information section --

    This is where the Jucer puts all of its metadata, so don't change anything in here!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="ProjectInformationComponent"
                 componentName="" parentClasses="public Component, public ChangeListener"
                 constructorParams="Project&amp; project_" variableInitialisers="project (project_)"
                 snapPixels="8" snapActive="1" snapShown="0" overlayOpacity="0.330000013"
                 fixedSize="0" initialWidth="600" initialHeight="400">
  <BACKGROUND backgroundColour="f6f9ff"/>
  <TABBEDCOMPONENT name="" id="962c1575c4142253" memberName="configTabBox" virtualName=""
                   explicitFocusOrder="0" pos="8 0 16M 36M" orientation="top" tabBarDepth="30"
                   initialTab="-1"/>
  <TEXTBUTTON name="" id="b6625dfcdb1f4755" memberName="editConfigsButton"
              virtualName="" explicitFocusOrder="0" pos="8 26R 192 22" buttonText="Add/Remove Configurations..."
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="a550a652e2666ee7" memberName="openProjectButton"
              virtualName="" explicitFocusOrder="0" pos="384 26R 208 22" buttonText="Open Project in "
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="c1f6e5f9811b307e" memberName="editExportersButton"
              virtualName="" explicitFocusOrder="0" pos="208 26R 160 22" buttonText="Add/Remove Exporters..."
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif
