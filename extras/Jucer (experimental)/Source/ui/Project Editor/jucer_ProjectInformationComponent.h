/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  10 May 2010 7:13:27pm

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

#ifndef __JUCER_HEADER_PROJECTINFORMATIONCOMPONENT_JUCERPROJECTINFORMATIONCOMPONENT_40825900__
#define __JUCER_HEADER_PROJECTINFORMATIONCOMPONENT_JUCERPROJECTINFORMATIONCOMPONENT_40825900__

//[Headers]     -- You can add your own extra header files here --
#include "../../jucer_Headers.h"
#include "../../model/Project/jucer_Project.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class ProjectInformationComponent  : public Component,
                                     public ChangeListener,
                                     public ButtonListener
{
public:
    //==============================================================================
    ProjectInformationComponent (Project& project_);
    ~ProjectInformationComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void changeListenerCallback (void*);
    void rebuildConfigTabs();
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void buttonClicked (Button* buttonThatWasClicked);


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    Project& project;

    var lastProjectType;
    void updateConfigTabs();
    void showConfigMenu();
    void showExporterMenu();
    //[/UserVariables]

    //==============================================================================
    TabbedComponent* configTabBox;
    TextButton* editConfigsButton;
    TextButton* openProjectButton;
    TextButton* editExportersButton;

    //==============================================================================
    // (prevent copy constructor and operator= being generated..)
    ProjectInformationComponent (const ProjectInformationComponent&);
    const ProjectInformationComponent& operator= (const ProjectInformationComponent&);
};


#endif   // __JUCER_HEADER_PROJECTINFORMATIONCOMPONENT_JUCERPROJECTINFORMATIONCOMPONENT_40825900__
