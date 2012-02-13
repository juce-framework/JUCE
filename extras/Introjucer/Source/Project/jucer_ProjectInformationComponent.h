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

#ifndef __JUCER_PROJECTINFORMATIONCOMPONENT_H_30FFCD07__
#define __JUCER_PROJECTINFORMATIONCOMPONENT_H_30FFCD07__

//[Headers]     -- You can add your own extra header files here --
#include "jucer_Project.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    Holds the tabs containing all the project info.
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
    //[UserMethods]
    void changeListenerCallback (ChangeBroadcaster*);
    //[/UserMethods]

    void buttonClicked (Button* buttonThatWasClicked);
    void paint (Graphics& g);

private:
    //==============================================================================
    //[UserVariables]
    Project& project;
    //[/UserVariables]

    //==============================================================================
    Viewport viewport;
    TextButton openProjectButton;
    TextButton saveAndOpenButton;
    RolloverHelpComp rollover;

    void initialiseComponentState();
    static ValueTree getComponentState();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectInformationComponent)
};

#endif   // __JUCER_PROJECTINFORMATIONCOMPONENT_H_30FFCD07__
