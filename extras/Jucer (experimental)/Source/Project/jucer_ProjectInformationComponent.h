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

#ifndef __JUCER_PROJECTINFORMATIONCOMPONENT_H_2F89B0AC__
#define __JUCER_PROJECTINFORMATIONCOMPONENT_H_2F89B0AC__

//[Headers]     -- You can add your own extra header files here --
#include "jucer_ProjectExporter.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    Holds the tabs containing all the project info.
                                                                    //[/Comments]
*/
class ProjectInformationComponent  : public Component,
                                     public ChangeListener,
                                     public Button::Listener
{
public:
    //==============================================================================
    ProjectInformationComponent (Project& project_);
    ~ProjectInformationComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void changeListenerCallback (ChangeBroadcaster*);
    void rebuildConfigTabs();
    //[/UserMethods]

    void resized();
    void buttonClicked (Button* buttonThatWasClicked);
    void paint (Graphics& g);


private:
    //==============================================================================
    //[UserVariables]   -- You can add your own custom variables in this section.
    Project& project;

    var lastProjectType;
    void updateConfigTabs();
    void showConfigMenu();
    void showExporterMenu();
    //[/UserVariables]

    //==============================================================================
    TabbedComponent configTabBox;
    TextButton editConfigsButton;
    TextButton openProjectButton;
    TextButton editExportersButton;
    TextButton saveAndOpenButton;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectInformationComponent);
};


#endif   // __JUCER_PROJECTINFORMATIONCOMPONENT_H_2F89B0AC__
