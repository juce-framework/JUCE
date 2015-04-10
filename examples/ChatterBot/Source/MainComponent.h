/*
  ==============================================================================

  This is an automatically generated GUI class created by the Introjucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Introjucer version: 3.1.1

  ------------------------------------------------------------------------------

  The Introjucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-13 by Raw Material Software Ltd.

  ==============================================================================
*/

#ifndef __JUCE_HEADER_CB87035FA43504BB__
#define __JUCE_HEADER_CB87035FA43504BB__

//[Headers]     -- You can add your own extra header files here --
#include "JuceHeader.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Introjucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class MainContentComponent  : public Component,
                              public Thread,
                              public ButtonListener
{
public:
    //==============================================================================
    MainContentComponent ();
    ~MainContentComponent();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void run();
    int getPort() const;
    void setPortLabel (int port);
    //[/UserMethods]

    void paint (Graphics& g);
    void resized();
    void buttonClicked (Button* buttonThatWasClicked);



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    ScopedPointer<DatagramSocket> rcvSocket;
    ScopedPointer<DatagramSocket> sndSocket;
    //[/UserVariables]

    //==============================================================================
    ScopedPointer<TextEditor> recvTextField;
    ScopedPointer<Label> serverPortLabel;
    ScopedPointer<Label> svrPortField;
    ScopedPointer<TextEditor> sndTextField;
    ScopedPointer<Label> MessageLabel;
    ScopedPointer<Label> dstPortLabel;
    ScopedPointer<TextEditor> dstPortField;
    ScopedPointer<TextButton> sndButton;
    ScopedPointer<Label> dstAddrLabel;
    ScopedPointer<TextEditor> dstAddrField;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

#endif   // __JUCE_HEADER_CB87035FA43504BB__
