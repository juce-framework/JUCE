/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 5.3.2

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library.
  Copyright (c) 2017 - ROLI Ltd.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
#include "../Performer.h"
//[/Headers]

#include "RackRow.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
String FormatKey(int note);
int ParseNote(const char *str);
//[/MiscUserDefs]

//==============================================================================
RackRow::RackRow ()
{
    //[Constructor_pre] You can add your own custom stuff here..
    m_keyboardState = new MidiKeyboardState();
    //[/Constructor_pre]

    m_deviceName.reset (new GroupComponent (String(),
                                            TRANS("Korg M1")));
    addAndMakeVisible (m_deviceName.get());

    m_deviceName->setBounds (0, -2, 816, 80);

    m_solo.reset (new ToggleButton (String()));
    addAndMakeVisible (m_solo.get());
    m_solo->setButtonText (TRANS("Solo"));
    m_solo->addListener (this);

    m_solo->setBounds (96, 14, 72, 24);

    m_mute.reset (new ToggleButton (String()));
    addAndMakeVisible (m_mute.get());
    m_mute->setButtonText (TRANS("Mute"));
    m_mute->addListener (this);

    m_mute->setBounds (160, 14, 72, 24);

    m_volume.reset (new Slider (String()));
    addAndMakeVisible (m_volume.get());
    m_volume->setRange (-110, 12, 0.5);
    m_volume->setSliderStyle (Slider::LinearBar);
    m_volume->setTextBoxStyle (Slider::TextBoxBelow, false, 80, 20);
    m_volume->addListener (this);

    m_volume->setBounds (96, 46, 128, 24);

    m_bank.reset (new ComboBox (String()));
    addAndMakeVisible (m_bank.get());
    m_bank->setEditableText (false);
    m_bank->setJustificationType (Justification::centredLeft);
    m_bank->setTextWhenNothingSelected (TRANS("None"));
    m_bank->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    m_bank->addItem (TRANS("None"), 1);
    m_bank->addItem (TRANS("Usercard 1"), 2);
    m_bank->addItem (TRANS("Usercard 2"), 3);
    m_bank->addSeparator();
    m_bank->addListener (this);

    m_bank->setBounds (232, 14, 150, 24);

    m_program.reset (new ComboBox (String()));
    addAndMakeVisible (m_program.get());
    m_program->setEditableText (false);
    m_program->setJustificationType (Justification::centredLeft);
    m_program->setTextWhenNothingSelected (TRANS("Piano"));
    m_program->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    m_program->addItem (TRANS("Piano"), 1);
    m_program->addItem (TRANS("Strings"), 2);
    m_program->addSeparator();
    m_program->addListener (this);

    m_program->setBounds (232, 46, 150, 24);

    m_transpose.reset (new TextEditor (String()));
    addAndMakeVisible (m_transpose.get());
    m_transpose->setMultiLine (false);
    m_transpose->setReturnKeyStartsNewLine (false);
    m_transpose->setReadOnly (false);
    m_transpose->setScrollbarsShown (false);
    m_transpose->setCaretVisible (true);
    m_transpose->setPopupMenuEnabled (true);
    m_transpose->setText (String());

    m_transpose->setBounds (648, 14, 32, 24);

    m_to.reset (new Label (String(),
                           TRANS("to")));
    addAndMakeVisible (m_to.get());
    m_to->setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
    m_to->setJustificationType (Justification::centredLeft);
    m_to->setEditable (false, false, false);
    m_to->setColour (TextEditor::textColourId, Colours::black);
    m_to->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    m_to->setBounds (744, 14, 24, 24);

    m_lowKey.reset (new TextEditor (String()));
    addAndMakeVisible (m_lowKey.get());
    m_lowKey->setMultiLine (false);
    m_lowKey->setReturnKeyStartsNewLine (false);
    m_lowKey->setReadOnly (false);
    m_lowKey->setScrollbarsShown (false);
    m_lowKey->setCaretVisible (true);
    m_lowKey->setPopupMenuEnabled (true);
    m_lowKey->setText (TRANS("C -2"));

    m_lowKey->setBounds (712, 14, 32, 24);

    m_highKey.reset (new TextEditor (String()));
    addAndMakeVisible (m_highKey.get());
    m_highKey->setMultiLine (false);
    m_highKey->setReturnKeyStartsNewLine (false);
    m_highKey->setReadOnly (false);
    m_highKey->setScrollbarsShown (false);
    m_highKey->setCaretVisible (true);
    m_highKey->setPopupMenuEnabled (true);
    m_highKey->setText (TRANS("G 8"));

    m_highKey->setBounds (768, 14, 32, 24);

    m_deviceSettings.reset (new ImageButton (String()));
    addAndMakeVisible (m_deviceSettings.get());
    m_deviceSettings->setButtonText (TRANS("new button"));
    m_deviceSettings->addListener (this);

    m_deviceSettings->setImages (false, true, true,
                                 Image(), 1.000f, Colour (0x00000000),
                                 Image(), 1.000f, Colour (0x00000000),
                                 Image(), 1.000f, Colour (0x00000000));
    m_deviceSettings->setBounds (8, 14, 76, 57);

    m_keyboard.reset (new MidiKeyboardComponent (*m_keyboardState, MidiKeyboardComponent::Orientation::horizontalKeyboard));
    addAndMakeVisible (m_keyboard.get());

    m_keyboard->setBounds (392, 46, 416, 24);

    m_doubleOctave.reset (new ToggleButton (String()));
    addAndMakeVisible (m_doubleOctave.get());
    m_doubleOctave->setButtonText (TRANS("Double octave"));
    m_doubleOctave->addListener (this);

    m_doubleOctave->setBounds (392, 14, 123, 24);

    m_arpeggiator.reset (new ToggleButton (String()));
    addAndMakeVisible (m_arpeggiator.get());
    m_arpeggiator->setButtonText (TRANS("Arpeggiator"));
    m_arpeggiator->addListener (this);

    m_arpeggiator->setBounds (512, 14, 112, 24);


    //[UserPreSize]
    m_keyboard->setKeyWidth(8.f);
    //[/UserPreSize]

    setSize (816, 76);


    //[Constructor] You can add your own custom stuff here..
    m_keyboard->addMouseListener(this, false);
    m_keyboard->setAvailableRange(21, 21+88-1);
    UpdateKeyboard();
    m_transpose->addListener(this);
    //[/Constructor]
}

RackRow::~RackRow()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    delete m_keyboardState;
    //[/Destructor_pre]

    m_deviceName = nullptr;
    m_solo = nullptr;
    m_mute = nullptr;
    m_volume = nullptr;
    m_bank = nullptr;
    m_program = nullptr;
    m_transpose = nullptr;
    m_to = nullptr;
    m_lowKey = nullptr;
    m_highKey = nullptr;
    m_deviceSettings = nullptr;
    m_keyboard = nullptr;
    m_doubleOctave = nullptr;
    m_arpeggiator = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void RackRow::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    //[UserPaint] Add your own custom painting code here..
    g.setColour(Colours::black);
    auto rect = m_deviceSettings->getBounds();
    rect.expand(1, 1);
    g.drawRect(rect);
    if (m_mute->getToggleState())
    {
        g.fillAll(Colour(0x50ffffff));
    }
    //[/UserPaint]
}

void RackRow::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void RackRow::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    /*for (int p = 0; p < m_parameters.size(); ++p)
    {
        auto button = (Button*)m_parameters[p].m_component;
        if (buttonThatWasClicked == button)
        {
            auto b = (bool*)((uint32_t)m_current + m_parameters[p].m_offset);
            *b = button->getToggleState();
        }
    }*/
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == m_solo.get())
    {
        //[UserButtonCode_m_solo] -- add your button handler code here..
        m_current->Solo = buttonThatWasClicked->getToggleState();
        //[/UserButtonCode_m_solo]
    }
    else if (buttonThatWasClicked == m_mute.get())
    {
        //[UserButtonCode_m_mute] -- add your button handler code here..

        // Deactivate plugin here
        m_current->Mute = buttonThatWasClicked->getToggleState();
        repaint();
        //[/UserButtonCode_m_mute]
    }
    else if (buttonThatWasClicked == m_deviceSettings.get())
    {
        //[UserButtonCode_m_deviceSettings] -- add your button handler code here..

        // Open plugin here

        //[/UserButtonCode_m_deviceSettings]
    }
    else if (buttonThatWasClicked == m_doubleOctave.get())
    {
        //[UserButtonCode_m_doubleOctave] -- add your button handler code here..
        m_current->DoubleOctave = buttonThatWasClicked->getToggleState();
        //[/UserButtonCode_m_doubleOctave]
    }
    else if (buttonThatWasClicked == m_arpeggiator.get())
    {
        //[UserButtonCode_m_arpeggiator] -- add your button handler code here..
        //[/UserButtonCode_m_arpeggiator]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}

void RackRow::sliderValueChanged (Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == m_volume.get())
    {
        //[UserSliderCode_m_volume] -- add your slider handling code here..

        m_current->Volume = (float)sliderThatWasMoved->getValue();
        // Change volume of plugin mixer here

        //[/UserSliderCode_m_volume]
    }

    //[UsersliderValueChanged_Post]
    //[/UsersliderValueChanged_Post]
}

void RackRow::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    //[UsercomboBoxChanged_Pre]
    //[/UsercomboBoxChanged_Pre]

    if (comboBoxThatHasChanged == m_bank.get())
    {
        //[UserComboBoxCode_m_bank] -- add your combo box handling code here..
        m_current->Bank = m_bank->getSelectedId() - 1;


        // send bank change here
        // update program dropdown

        //[/UserComboBoxCode_m_bank]
    }
    else if (comboBoxThatHasChanged == m_program.get())
    {
        //[UserComboBoxCode_m_program] -- add your combo box handling code here..
        m_current->Program = m_program->getSelectedId() - 1;


        // send program change here

        //[/UserComboBoxCode_m_program]
    }

    //[UsercomboBoxChanged_Post]
    //[/UsercomboBoxChanged_Post]
}

void RackRow::mouseDown (const MouseEvent& e)
{
    //[UserCode_mouseDown] -- Add your code here...
    mouseDrag(e);
    //[/UserCode_mouseDown]
}

void RackRow::mouseDrag (const MouseEvent& e)
{
    //[UserCode_mouseDrag] -- Add your code here...
    if (e.eventComponent == m_keyboard.get())
    {
        auto key = m_keyboard->getNoteAtPosition(e.position);
        if (key != -1)
        {
            auto lowkey = ParseNote(m_lowKey->getTextValue().toString().getCharPointer());
            auto highkey = ParseNote(m_highKey->getTextValue().toString().getCharPointer());
            if (abs(key - lowkey) > abs(key - highkey))
                m_highKey->setText(FormatKey(key));
            else
                m_lowKey->setText(FormatKey(key));
            UpdateKeyboard();
        }
    }
    //[/UserCode_mouseDrag]
}

void RackRow::mouseUp (const MouseEvent& e)
{
    //[UserCode_mouseUp] -- Add your code here...
    mouseDrag(e);
    //[/UserCode_mouseUp]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...

void RackRow::textEditorTextChanged(TextEditor&te)
{
    if (&te == m_transpose.get())
    {
        m_current->Transpose = atoi(te.getText().getCharPointer());
    }
    else if (&te == m_lowKey.get())
    {
        m_current->LowKey = ParseNote(te.getText().getCharPointer());
    }
    else if (&te == m_highKey.get())
    {
        m_current->HighKey = ParseNote(te.getText().getCharPointer());
    }
}

String FormatKey(int note)
{
    const char *notenames[] = { "C ","C#","D ","D#","E ","F ","F#","G ","G#","A ","A#","B " };
    return String::formatted("%S%d", notenames[note % 12], note / 12 - 2);
}

int ParseNote(const char *str)
{
    char notename[256];

    char nospace[256];
    memset(nospace, 0, sizeof(nospace));
    for (int c = 0; c<(int)strlen(str); ++c)
    {
        if (str[c] != ' ')
            nospace[strlen(nospace)] = str[c];
    }
    const char *notenames[] = { "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" };
    for (int note = 0; note <= 127; ++note)
    {
        sprintf(notename, "%s%d", notenames[note % 12], note / 12 - 2);
        if (stricmp(notename, nospace) == 0)
            return note;
    }
    return 0;
}

void RackRow::UpdateKeyboard()
{
    auto lowkey = ParseNote(m_lowKey->getTextValue().toString().getCharPointer());
    auto highkey = ParseNote(m_highKey->getTextValue().toString().getCharPointer());
    m_keyboardState->allNotesOff(1);
    for(int i=lowkey;i<=highkey;++i)
        m_keyboardState->noteOn(1, i, 1.0f);
}

void RackRow::Setup(Device &device)
{
 /*   Zone dummyZone;
    m_parameters.push_back(Parameter(m_solo.get(), (uint32_t)&dummyZone.Solo - (uint32_t)&dummyZone));
    m_parameters.push_back(Parameter(m_mute.get(), (uint32_t)&dummyZone.Mute - (uint32_t)&dummyZone));
    m_parameters.push_back(Parameter(m_volume.get(), (uint32_t)&dummyZone.Volume - (uint32_t)&dummyZone));
    m_parameters.push_back(Parameter(m_bank.get(), (uint32_t)&dummyZone.Bank - (uint32_t)&dummyZone));
    m_parameters.push_back(Parameter(m_program.get(), (uint32_t)&dummyZone.Program - (uint32_t)&dummyZone));
    m_parameters.push_back(Parameter(m_transpose.get(), (uint32_t)&dummyZone.Transpose - (uint32_t)&dummyZone));
    m_parameters.push_back(Parameter(m_lowKey.get(), (uint32_t)&dummyZone.LowKey - (uint32_t)&dummyZone));
    m_parameters.push_back(Parameter(m_highKey.get(), (uint32_t)&dummyZone.HighKey - (uint32_t)&dummyZone));
    m_parameters.push_back(Parameter(m_doubleOctave.get(), (uint32_t)&dummyZone.DoubleOctave - (uint32_t)&dummyZone));
    m_parameters.push_back(Parameter(m_arpeggiator.get(), (uint32_t)&dummyZone.Arpeggiator - (uint32_t)&dummyZone));*/

    m_deviceName->setText(device.Name);
    auto image = ImageFileFormat::loadFrom(File::getCurrentWorkingDirectory().getFullPathName() + "\\" + String(device.Name + ".png"));
    m_deviceSettings->setImages(false, false, false, image, 1.0f, Colours::transparentBlack, image, 1.0f, Colours::transparentBlack, image, 1.0f, Colours::transparentBlack);

    m_id = device.ID;
}

void RackRow::Assign(Zone *zone)
{
    m_current = zone;
    m_volume->setValue(zone->Volume);
    m_solo->setToggleState(zone->Solo, true);
    m_mute->setToggleState(zone->Mute, true);
    m_doubleOctave->setToggleState(zone->DoubleOctave, true);
    m_arpeggiator->setToggleState(zone->Arpeggiator, true);
    m_lowKey->setText(FormatKey(zone->LowKey));
    m_highKey->setText(FormatKey(zone->HighKey));
    m_transpose->setText(String(zone->Transpose));
    m_bank->setSelectedId(zone->Bank);
    m_program->setSelectedId(zone->Program);

    UpdateKeyboard();
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="RackRow" componentName=""
                 parentClasses="public Component, public TextEditor::Listener"
                 constructorParams="" variableInitialisers="" snapPixels="8" snapActive="1"
                 snapShown="1" overlayOpacity="0.330" fixedSize="1" initialWidth="816"
                 initialHeight="76">
  <METHODS>
    <METHOD name="mouseDown (const MouseEvent&amp; e)"/>
    <METHOD name="mouseDrag (const MouseEvent&amp; e)"/>
    <METHOD name="mouseUp (const MouseEvent&amp; e)"/>
  </METHODS>
  <BACKGROUND backgroundColour="0"/>
  <GROUPCOMPONENT name="" id="85efcbef1342dec0" memberName="m_deviceName" virtualName=""
                  explicitFocusOrder="0" pos="0 -2 816 80" title="Korg M1"/>
  <TOGGLEBUTTON name="" id="2b62ef4a67b701f3" memberName="m_solo" virtualName=""
                explicitFocusOrder="0" pos="96 14 72 24" buttonText="Solo" connectedEdges="0"
                needsCallback="1" radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="" id="6ad6011b41b23475" memberName="m_mute" virtualName=""
                explicitFocusOrder="0" pos="160 14 72 24" buttonText="Mute" connectedEdges="0"
                needsCallback="1" radioGroupId="0" state="0"/>
  <SLIDER name="" id="a0e2bc5a61933c6d" memberName="m_volume" virtualName=""
          explicitFocusOrder="0" pos="96 46 128 24" min="-110.00000000000000000000"
          max="12.00000000000000000000" int="0.50000000000000000000" style="LinearBar"
          textBoxPos="TextBoxBelow" textBoxEditable="1" textBoxWidth="80"
          textBoxHeight="20" skewFactor="1.00000000000000000000" needsCallback="1"/>
  <COMBOBOX name="" id="90d63ca95a92a112" memberName="m_bank" virtualName=""
            explicitFocusOrder="0" pos="232 14 150 24" editable="0" layout="33"
            items="None&#10;Usercard 1&#10;Usercard 2&#10;" textWhenNonSelected="None"
            textWhenNoItems="(no choices)"/>
  <COMBOBOX name="" id="9de3cb5469378fa1" memberName="m_program" virtualName=""
            explicitFocusOrder="0" pos="232 46 150 24" editable="0" layout="33"
            items="Piano&#10;Strings&#10;" textWhenNonSelected="Piano" textWhenNoItems="(no choices)"/>
  <TEXTEDITOR name="" id="b6e30577b79a003a" memberName="m_transpose" virtualName=""
              explicitFocusOrder="0" pos="648 14 32 24" initialText="" multiline="0"
              retKeyStartsLine="0" readonly="0" scrollbars="0" caret="1" popupmenu="1"/>
  <LABEL name="" id="72d9777463cc6a85" memberName="m_to" virtualName=""
         explicitFocusOrder="0" pos="744 14 24 24" edTextCol="ff000000"
         edBkgCol="0" labelText="to" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15.00000000000000000000"
         kerning="0.00000000000000000000" bold="0" italic="0" justification="33"/>
  <TEXTEDITOR name="" id="3d470180923a3d6f" memberName="m_lowKey" virtualName=""
              explicitFocusOrder="0" pos="712 14 32 24" initialText="C -2"
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="0"
              caret="1" popupmenu="1"/>
  <TEXTEDITOR name="" id="5f3abd7bbb50678c" memberName="m_highKey" virtualName=""
              explicitFocusOrder="0" pos="768 14 32 24" initialText="G 8" multiline="0"
              retKeyStartsLine="0" readonly="0" scrollbars="0" caret="1" popupmenu="1"/>
  <IMAGEBUTTON name="" id="31b2ae44720b5f47" memberName="m_deviceSettings" virtualName=""
               explicitFocusOrder="0" pos="8 14 76 57" buttonText="new button"
               connectedEdges="0" needsCallback="1" radioGroupId="0" keepProportions="1"
               resourceNormal="" opacityNormal="1.00000000000000000000" colourNormal="0"
               resourceOver="" opacityOver="1.00000000000000000000" colourOver="0"
               resourceDown="" opacityDown="1.00000000000000000000" colourDown="0"/>
  <GENERICCOMPONENT name="" id="3a433662794e0409" memberName="m_keyboard" virtualName="MidiKeyboardComponent"
                    explicitFocusOrder="0" pos="392 46 416 24" class="unknown" params="*m_keyboardState, MidiKeyboardComponent::Orientation::horizontalKeyboard"/>
  <TOGGLEBUTTON name="" id="7a9e84b485ffe060" memberName="m_doubleOctave" virtualName=""
                explicitFocusOrder="0" pos="392 14 123 24" buttonText="Double octave"
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="" id="82787edffe0c1be4" memberName="m_arpeggiator" virtualName=""
                explicitFocusOrder="0" pos="512 14 112 24" buttonText="Arpeggiator"
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]
