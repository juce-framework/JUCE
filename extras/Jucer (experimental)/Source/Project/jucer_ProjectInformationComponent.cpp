/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created for JUCE version: JUCE v1.52.97

  ------------------------------------------------------------------------------

  JUCE and the Jucer are copyright 2004-10 by Raw Material Software ltd.

  ==============================================================================
*/

//[CppHeaders] You can add your own extra header files here...
#include "jucer_ProjectExporter.h"
//[/CppHeaders]

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
        getPanel().clear();
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
            possibleValues.add ("(Use default from juce_Config.h)");
            possibleValues.add ("Enabled");
            possibleValues.add ("Disabled");

            Array<var> mappings;
            mappings.add (Project::configFlagDefault);
            mappings.add (Project::configFlagEnabled);
            mappings.add (Project::configFlagDisabled);

            for (int i = 0; i < flags.size(); ++i)
            {
                ChoicePropertyComponent* c = new ChoicePropertyComponent (flags[i]->value, flags[i]->symbol, possibleValues, mappings);
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

            for (int i = props.size(); --i >= 0;)
                props.getUnchecked(i)->setPreferredHeight (22);
        }

        getPanel().addProperties (props);
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
      configTabBox (TabbedButtonBar::TabsAtTop)
{
    addAndMakeVisible (&configTabBox);
    addAndMakeVisible (&editConfigsButton);
    editConfigsButton.setButtonText ("Add/Remove Configurations...");
    editConfigsButton.addListener (this);
    addAndMakeVisible (&openProjectButton);
    openProjectButton.setButtonText ("Open Project in ");
    openProjectButton.addListener (this);
    addAndMakeVisible (&editExportersButton);
    editExportersButton.setButtonText ("Add/Remove Exporters...");
    editExportersButton.addListener (this);
    addAndMakeVisible (&saveAndOpenButton);
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

    setSize (859, 479);

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

    configTabBox.setBounds (Rectangle<int>::leftTopRightBottom (8, 0, (int) ((8.0 + getWidth()) - 16.0),
                                                                (int) ((0.0 + getHeight()) - 36.0)));
    editConfigsButton.setBounds (Rectangle<int>::leftTopRightBottom (8, (int) (getHeight() - 30.0), (int) (8.0 + 192.0),
                                                                     (int) (getHeight() - 30.0 + 22.0)));
    openProjectButton.setBounds (Rectangle<int>::leftTopRightBottom (608, (int) (getHeight() - 30.0), (int) (608.0 + 208.0),
                                                                     (int) (getHeight() - 30.0 + 22.0)));
    editExportersButton.setBounds (Rectangle<int>::leftTopRightBottom (208, (int) (getHeight() - 30.0),
                                                                       (int) (208.0 + 160.0), (int) (getHeight() - 30.0 + 22.0)));
    saveAndOpenButton.setBounds (Rectangle<int>::leftTopRightBottom (391, (int) (getHeight() - 30.0), (int) (391.0 + 208.0),
                                                                     (int) (getHeight() - 30.0 + 22.0)));

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

    int index = 0;
    PropertiesWithHelpComponent* panel = new PropertiesWithHelpComponent (project, index++);
    configTabBox.addTab ("Project Settings", Colours::lightslategrey, panel, true, -1);

    panel = new PropertiesWithHelpComponent (project, index++);
    configTabBox.addTab ("Juce Flags", Colours::lightblue, panel, true, -1);

    int i;
    for (i = 0; i < project.getNumConfigurations(); ++i)
    {
        panel = new PropertiesWithHelpComponent (project, index++);
        Project::BuildConfiguration config (project.getConfiguration (i));
        configTabBox.addTab (config.getName().toString(), Colour::greyLevel (0.65f), panel, true, -1);
    }

    for (i = 0; i < project.getNumExporters(); ++i)
    {
        ScopedPointer <ProjectExporter> exp (project.createExporter (i));

        if (exp != 0)
        {
            panel = new PropertiesWithHelpComponent (project, index++);
            configTabBox.addTab (exp->getName(), Colours::lightsteelblue, panel, true, -1);
        }
    }

    lastProjectType = project.getProjectType().getValue();
}

void ProjectInformationComponent::updateConfigTabs()
{
    if (configTabBox.getNumTabs() != project.getNumConfigurations() + project.getNumExporters() + 2
         || lastProjectType != project.getProjectType().getValue())
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

    const int r = m.showAt (&editExportersButton);

    if (r >= 20000)
        project.deleteExporter (r - 20000);
    else if (r >= 10000)
        project.addNewExporter (r - 10000);
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

<COMPONENT id="tO9EG1a" className="ProjectInformationComponent" width="859"
           height="479" background="f6f9ff" parentClasses="public Component, public ChangeListener"
           constructorParams="Project&amp; project_" memberInitialisers="project (project_)">
  <COMPONENTS>
    <TABBEDCOMPONENT id="962c1575c4142253" memberName="configTabBox" focusOrder="0"
                     position="8, 0, configTabBox.left + parent.right - 16, configTabBox.top + parent.bottom - 36"/>
    <TEXTBUTTON id="b6625dfcdb1f4755" memberName="editConfigsButton" focusOrder="0"
                text="Add/Remove Configurations..." createCallback="1" radioGroup="0"
                connectedLeft="0" connectedRight="0" connectedTop="0" connectedBottom="0"
                backgroundColour="" textColour="" backgroundColourOn="" textColourOn=""
                position="8, parent.bottom - 30, editConfigsButton.left + 192, editConfigsButton.top + 22"/>
    <TEXTBUTTON id="a550a652e2666ee7" memberName="openProjectButton" focusOrder="0"
                text="Open Project in " createCallback="1" radioGroup="0" connectedLeft="0"
                connectedRight="0" connectedTop="0" connectedBottom="0" backgroundColour=""
                textColour="" backgroundColourOn="" textColourOn="" position="608, parent.bottom - 30, openProjectButton.left + 208, openProjectButton.top + 22"/>
    <TEXTBUTTON id="c1f6e5f9811b307e" memberName="editExportersButton" focusOrder="0"
                text="Add/Remove Exporters..." createCallback="1" radioGroup="0"
                connectedLeft="0" connectedRight="0" connectedTop="0" connectedBottom="0"
                backgroundColour="" textColour="" backgroundColourOn="" textColourOn=""
                position="208, parent.bottom - 30, editExportersButton.left + 160, editExportersButton.top + 22"/>
    <TEXTBUTTON id="dRGMyYx" name="" memberName="saveAndOpenButton" position="391, parent.bottom - 30, saveAndOpenButton.left + 208, saveAndOpenButton.top + 22"
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
