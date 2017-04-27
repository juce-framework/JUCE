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

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "MainComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
MainContentComponent::MainContentComponent ()
	  : Thread ("listener")
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    addAndMakeVisible (recvTextField = new TextEditor ("Receive Text Field"));
    recvTextField->setMultiLine (true);
    recvTextField->setReturnKeyStartsNewLine (false);
    recvTextField->setReadOnly (true);
    recvTextField->setScrollbarsShown (true);
    recvTextField->setCaretVisible (false);
    recvTextField->setPopupMenuEnabled (false);
    recvTextField->setText (String::empty);

    addAndMakeVisible (serverPortLabel = new Label ("Server Port Label",
                                                    TRANS("Server Port:")));
    serverPortLabel->setFont (Font (15.00f, Font::bold));
    serverPortLabel->setJustificationType (Justification::centredLeft);
    serverPortLabel->setEditable (false, false, false);
    serverPortLabel->setColour (TextEditor::textColourId, Colours::black);
    serverPortLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (svrPortField = new Label ("Server Port Indicator",
                                                 TRANS("error")));
    svrPortField->setFont (Font (15.00f, Font::plain));
    svrPortField->setJustificationType (Justification::centredLeft);
    svrPortField->setEditable (false, false, false);
    svrPortField->setColour (TextEditor::textColourId, Colours::black);
    svrPortField->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (sndTextField = new TextEditor ("Send Text Field"));
    sndTextField->setMultiLine (false);
    sndTextField->setReturnKeyStartsNewLine (false);
    sndTextField->setReadOnly (false);
    sndTextField->setScrollbarsShown (true);
    sndTextField->setCaretVisible (true);
    sndTextField->setPopupMenuEnabled (true);
    sndTextField->setText (TRANS("Hello World!"));

    addAndMakeVisible (MessageLabel = new Label ("Message Label",
                                                 TRANS("Your Message:")));
    MessageLabel->setFont (Font (15.00f, Font::plain));
    MessageLabel->setJustificationType (Justification::centredLeft);
    MessageLabel->setEditable (false, false, false);
    MessageLabel->setColour (TextEditor::textColourId, Colours::black);
    MessageLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (dstPortLabel = new Label ("Destination Port Label",
                                                 TRANS("Port:")));
    dstPortLabel->setFont (Font (15.00f, Font::plain));
    dstPortLabel->setJustificationType (Justification::centredLeft);
    dstPortLabel->setEditable (false, false, false);
    dstPortLabel->setColour (TextEditor::textColourId, Colours::black);
    dstPortLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (dstPortField = new TextEditor ("Destination Port Field"));
    dstPortField->setMultiLine (false);
    dstPortField->setReturnKeyStartsNewLine (false);
    dstPortField->setReadOnly (false);
    dstPortField->setScrollbarsShown (true);
    dstPortField->setCaretVisible (true);
    dstPortField->setPopupMenuEnabled (true);
    dstPortField->setText (String::empty);

    addAndMakeVisible (sndButton = new TextButton ("Send Button"));
    sndButton->setButtonText (TRANS("Send"));
    sndButton->addListener (this);

    addAndMakeVisible (dstAddrLabel = new Label ("Destination Port Label",
                                                 TRANS("Destination Addr:")));
    dstAddrLabel->setFont (Font (15.00f, Font::plain));
    dstAddrLabel->setJustificationType (Justification::centredLeft);
    dstAddrLabel->setEditable (false, false, false);
    dstAddrLabel->setColour (TextEditor::textColourId, Colours::black);
    dstAddrLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    addAndMakeVisible (dstAddrField = new TextEditor ("Destination Port Field"));
    dstAddrField->setMultiLine (false);
    dstAddrField->setReturnKeyStartsNewLine (false);
    dstAddrField->setReadOnly (false);
    dstAddrField->setScrollbarsShown (true);
    dstAddrField->setCaretVisible (true);
    dstAddrField->setPopupMenuEnabled (true);
    dstAddrField->setText (TRANS("127.0.0.1"));


    //[UserPreSize]
    rcvSocket = ScopedPointer<DatagramSocket> (new DatagramSocket());
    if (rcvSocket->bindToPort (0))
    {
       svrPortField->setText (String (rcvSocket->getBoundPort()), dontSendNotification);
    }

    sndSocket = ScopedPointer<DatagramSocket> (new DatagramSocket());
    //[/UserPreSize]

    setSize (600, 400);


    //[Constructor] You can add your own custom stuff here..
    startThread();
    //[/Constructor]
}

MainContentComponent::~MainContentComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    recvTextField = nullptr;
    serverPortLabel = nullptr;
    svrPortField = nullptr;
    sndTextField = nullptr;
    MessageLabel = nullptr;
    dstPortLabel = nullptr;
    dstPortField = nullptr;
    sndButton = nullptr;
    dstAddrLabel = nullptr;
    dstAddrField = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    // deleting the rcvSocket will cancel the read loop and allow the thread to exit
    signalThreadShouldExit();
    rcvSocket = nullptr;
    sndSocket = nullptr;
    waitForThreadToExit (-1);
    //[/Destructor]
}

//==============================================================================
void MainContentComponent::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colours::white);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void MainContentComponent::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    recvTextField->setBounds (16, 32, proportionOfWidth (0.9151f), getHeight() - 196);
    serverPortLabel->setBounds (8, 8, 88, 24);
    svrPortField->setBounds (104, 8, 88, 24);
    sndTextField->setBounds (32, getHeight() - 125, proportionOfWidth (0.8724f), 24);
    MessageLabel->setBounds (8, getHeight() - 157, 150, 24);
    dstPortLabel->setBounds (184, getHeight() - 92, 150, 23);
    dstPortField->setBounds (192, getHeight() - 68, 88, 24);
    sndButton->setBounds (getWidth() - 168, getHeight() - 37, 150, 24);
    dstAddrLabel->setBounds (24, getHeight() - 92, 150, 24);
    dstAddrField->setBounds (40, getHeight() - 68, 136, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void MainContentComponent::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == sndButton)
    {
        //[UserButtonCode_sndButton] -- add your button handler code here..
        sndSocket->write (dstAddrField->getText(),
                          dstPortField->getText().getIntValue(),
                          sndTextField->getText().getCharPointer(),
                          sndTextField->getText().length());
        //[/UserButtonCode_sndButton]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void MainContentComponent::run()
{
    char buffer[1024];
    String contents;
	int ret;

	while (threadShouldExit() == false)
    {
		while ((ret = rcvSocket->waitUntilReady(true, -1)) == 0)
		{
		}

		if (threadShouldExit() == true)
			return;

        if (ret == 1)
        {
            int len;
            String ip;
            int port;

            if ((len = rcvSocket->read (buffer, 1024, false, ip, port)) >= 0)
            {
                contents += ip + String (":") + String (port)
                               + String (": ") + String (buffer, len)
                               + String ("\n");
            }
            else
            {
                contents += ip + String (":") + String (port)
                               + String (" transfer error!\n");
            }
        } else {
            contents += String ("WaitUntilReady error!\n");
        }

		{
			const MessageManagerLock mmLock;
			recvTextField->setText(contents, false);
		}
	} 
}

int MainContentComponent::getPort() const
{
    return rcvSocket->getBoundPort();
}

void MainContentComponent::setPortLabel (int port)
{
    dstPortField->setText (String (port), true);
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Introjucer information section --

    This is where the Introjucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="MainContentComponent" componentName=""
                 parentClasses="public Component, public Thread" constructorParams=""
                 variableInitialisers="Thread (&quot;listener&quot;)" snapPixels="8"
                 snapActive="1" snapShown="1" overlayOpacity="0.330" fixedSize="0"
                 initialWidth="600" initialHeight="400">
  <BACKGROUND backgroundColour="ffffffff"/>
  <TEXTEDITOR name="Receive Text Field" id="89d723eb3ce61b05" memberName="recvTextField"
              virtualName="" explicitFocusOrder="0" pos="16 32 91.489% 196M"
              initialText="" multiline="1" retKeyStartsLine="0" readonly="1"
              scrollbars="1" caret="0" popupmenu="0"/>
  <LABEL name="Server Port Label" id="fabf5cbebbad8a68" memberName="serverPortLabel"
         virtualName="" explicitFocusOrder="0" pos="8 8 88 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Server Port:" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="1" italic="0" justification="33"/>
  <LABEL name="Server Port Indicator" id="47680129ecf00719" memberName="svrPortField"
         virtualName="" explicitFocusOrder="0" pos="104 8 88 24" edTextCol="ff000000"
         edBkgCol="0" labelText="error" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="33"/>
  <TEXTEDITOR name="Send Text Field" id="6e6ab0fbb17953a3" memberName="sndTextField"
              virtualName="" explicitFocusOrder="0" pos="32 125R 87.234% 24"
              initialText="Hello World!" multiline="0" retKeyStartsLine="0"
              readonly="0" scrollbars="1" caret="1" popupmenu="1"/>
  <LABEL name="Message Label" id="a7ce6a0c44749ada" memberName="MessageLabel"
         virtualName="" explicitFocusOrder="0" pos="8 157R 150 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Your Message:" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <LABEL name="Destination Port Label" id="7540a0e616d5fc71" memberName="dstPortLabel"
         virtualName="" explicitFocusOrder="0" pos="184 92R 150 23" edTextCol="ff000000"
         edBkgCol="0" labelText="Port:" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15"
         bold="0" italic="0" justification="33"/>
  <TEXTEDITOR name="Destination Port Field" id="8ea06e467326aa00" memberName="dstPortField"
              virtualName="" explicitFocusOrder="0" pos="192 68R 88 24" initialText=""
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="1"
              caret="1" popupmenu="1"/>
  <TEXTBUTTON name="Send Button" id="3674f6126ab7e84a" memberName="sndButton"
              virtualName="" explicitFocusOrder="0" pos="168R 37R 150 24" buttonText="Send"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <LABEL name="Destination Port Label" id="119899814d38bed9" memberName="dstAddrLabel"
         virtualName="" explicitFocusOrder="0" pos="24 92R 150 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Destination Addr:" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15" bold="0" italic="0" justification="33"/>
  <TEXTEDITOR name="Destination Port Field" id="49088ae1cdee395" memberName="dstAddrField"
              virtualName="" explicitFocusOrder="0" pos="40 68R 136 24" initialText="127.0.0.1"
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="1"
              caret="1" popupmenu="1"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
