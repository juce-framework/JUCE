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
//[/Headers]

#include "RackRow.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
String FormatKey(int note);
int ParseNote(const char *str);
//[/MiscUserDefs]

//==============================================================================
Forte::Forte ()
{
    //[Constructor_pre] You can add your own custom stuff here..
    m_keyboardState = new MidiKeyboardState();
    //[/Constructor_pre]

    groupComponent.reset (new GroupComponent ("new group",
                                              TRANS("Korg M1")));
    addAndMakeVisible (groupComponent.get());

    groupComponent->setBounds (8, 16, 1000, 80);

    label4.reset (new Label ("new label",
                             TRANS("Transpose")));
    addAndMakeVisible (label4.get());
    label4->setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
    label4->setJustificationType (Justification::centredLeft);
    label4->setEditable (false, false, false);
    label4->setColour (TextEditor::textColourId, Colours::black);
    label4->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    label4->setBounds (536, 0, 80, 24);

    toggleButton.reset (new ToggleButton (String()));
    addAndMakeVisible (toggleButton.get());
    toggleButton->setButtonText (TRANS("Solo"));
    toggleButton->addListener (this);

    toggleButton->setBounds (104, 32, 72, 24);

    toggleButton2.reset (new ToggleButton ("new toggle button"));
    addAndMakeVisible (toggleButton2.get());
    toggleButton2->setButtonText (TRANS("Mute"));
    toggleButton2->addListener (this);

    toggleButton2->setBounds (168, 32, 72, 24);

    slider.reset (new Slider ("new slider"));
    addAndMakeVisible (slider.get());
    slider->setRange (-110, 12, 0.5);
    slider->setSliderStyle (Slider::LinearBar);
    slider->setTextBoxStyle (Slider::TextBoxBelow, false, 80, 20);
    slider->addListener (this);

    slider->setBounds (104, 64, 128, 24);

    comboBox.reset (new ComboBox ("new combo box"));
    addAndMakeVisible (comboBox.get());
    comboBox->setEditableText (false);
    comboBox->setJustificationType (Justification::centredLeft);
    comboBox->setTextWhenNothingSelected (TRANS("None"));
    comboBox->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    comboBox->addItem (TRANS("None"), 1);
    comboBox->addItem (TRANS("Usercard 1"), 2);
    comboBox->addItem (TRANS("Usercard 2"), 3);
    comboBox->addSeparator();
    comboBox->addListener (this);

    comboBox->setBounds (240, 32, 150, 24);

    comboBox2.reset (new ComboBox ("new combo box"));
    addAndMakeVisible (comboBox2.get());
    comboBox2->setEditableText (false);
    comboBox2->setJustificationType (Justification::centredLeft);
    comboBox2->setTextWhenNothingSelected (TRANS("Piano"));
    comboBox2->setTextWhenNoChoicesAvailable (TRANS("(no choices)"));
    comboBox2->addItem (TRANS("Piano"), 1);
    comboBox2->addItem (TRANS("Strings"), 2);
    comboBox2->addSeparator();
    comboBox2->addListener (this);

    comboBox2->setBounds (240, 64, 150, 24);

    label2.reset (new Label ("new label",
                             TRANS("Bank/Program\n")));
    addAndMakeVisible (label2.get());
    label2->setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
    label2->setJustificationType (Justification::centredLeft);
    label2->setEditable (false, false, false);
    label2->setColour (TextEditor::textColourId, Colours::black);
    label2->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    label2->setBounds (264, 0, 104, 24);

    textEditor.reset (new TextEditor ("new text editor"));
    addAndMakeVisible (textEditor.get());
    textEditor->setMultiLine (false);
    textEditor->setReturnKeyStartsNewLine (false);
    textEditor->setReadOnly (false);
    textEditor->setScrollbarsShown (false);
    textEditor->setCaretVisible (true);
    textEditor->setPopupMenuEnabled (true);
    textEditor->setText (String());

    textEditor->setBounds (552, 32, 32, 24);

    label5.reset (new Label ("new label",
                             TRANS("Range")));
    addAndMakeVisible (label5.get());
    label5->setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
    label5->setJustificationType (Justification::centredLeft);
    label5->setEditable (false, false, false);
    label5->setColour (TextEditor::textColourId, Colours::black);
    label5->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    label5->setBounds (640, 0, 56, 24);

    m_to.reset (new Label (String(),
                           TRANS("to")));
    addAndMakeVisible (m_to.get());
    m_to->setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
    m_to->setJustificationType (Justification::centredLeft);
    m_to->setEditable (false, false, false);
    m_to->setColour (TextEditor::textColourId, Colours::black);
    m_to->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    m_to->setBounds (656, 32, 24, 24);

    m_lowKey.reset (new TextEditor (String()));
    addAndMakeVisible (m_lowKey.get());
    m_lowKey->setMultiLine (false);
    m_lowKey->setReturnKeyStartsNewLine (false);
    m_lowKey->setReadOnly (false);
    m_lowKey->setScrollbarsShown (false);
    m_lowKey->setCaretVisible (true);
    m_lowKey->setPopupMenuEnabled (true);
    m_lowKey->setText (TRANS("C -2"));

    m_lowKey->setBounds (624, 32, 32, 24);

    m_highKey.reset (new TextEditor (String()));
    addAndMakeVisible (m_highKey.get());
    m_highKey->setMultiLine (false);
    m_highKey->setReturnKeyStartsNewLine (false);
    m_highKey->setReadOnly (false);
    m_highKey->setScrollbarsShown (false);
    m_highKey->setCaretVisible (true);
    m_highKey->setPopupMenuEnabled (true);
    m_highKey->setText (TRANS("G 8"));

    m_highKey->setBounds (680, 32, 32, 24);

    imageButton.reset (new ImageButton ("new button"));
    addAndMakeVisible (imageButton.get());
    imageButton->addListener (this);

    imageButton->setImages (false, true, true,
                            ImageCache::getFromMemory (truePianos_png, truePianos_pngSize), 1.000f, Colour (0x00000000),
                            Image(), 1.000f, Colour (0x00000000),
                            Image(), 1.000f, Colour (0x00000000));
    imageButton->setBounds (16, 32, 76, 57);

    label.reset (new Label ("new label",
                            TRANS("Songs")));
    addAndMakeVisible (label.get());
    label->setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
    label->setJustificationType (Justification::centredLeft);
    label->setEditable (false, false, false);
    label->setColour (TextEditor::textColourId, Colours::black);
    label->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    label->setBounds (136, 128, 150, 24);

    label7.reset (new Label ("new label",
                             TRANS("SetLists")));
    addAndMakeVisible (label7.get());
    label7->setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
    label7->setJustificationType (Justification::centredLeft);
    label7->setEditable (false, false, false);
    label7->setColour (TextEditor::textColourId, Colours::black);
    label7->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    label7->setBounds (136, 112, 150, 24);

    label9.reset (new Label ("new label",
                             TRANS("Performances")));
    addAndMakeVisible (label9.get());
    label9->setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
    label9->setJustificationType (Justification::centredLeft);
    label9->setEditable (false, false, false);
    label9->setColour (TextEditor::textColourId, Colours::black);
    label9->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    label9->setBounds (136, 144, 150, 24);

    label3.reset (new Label ("new label",
                             TRANS("Volume")));
    addAndMakeVisible (label3.get());
    label3->setFont (Font (15.00f, Font::plain).withTypefaceStyle ("Regular"));
    label3->setJustificationType (Justification::centredLeft);
    label3->setEditable (false, false, false);
    label3->setColour (TextEditor::textColourId, Colours::black);
    label3->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

    label3->setBounds (144, 0, 64, 24);

    m_keyboard.reset (new MidiKeyboardComponent (*m_keyboardState, MidiKeyboardComponent::Orientation::horizontalKeyboard));
    addAndMakeVisible (m_keyboard.get());

    m_keyboard->setBounds (400, 64, 600, 24);

    toggleButton3.reset (new ToggleButton ("new toggle button"));
    addAndMakeVisible (toggleButton3.get());
    toggleButton3->setButtonText (TRANS("Double octave"));
    toggleButton3->addListener (this);

    toggleButton3->setBounds (400, 32, 123, 24);


    //[UserPreSize]
    m_keyboard->setKeyWidth(8.f);
    //[/UserPreSize]

    setSize (1024, 768);


    //[Constructor] You can add your own custom stuff here..
    m_keyboard->addMouseListener(this, false);
                UpdateKeyboard();
    //[/Constructor]
}

Forte::~Forte()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    groupComponent = nullptr;
    label4 = nullptr;
    toggleButton = nullptr;
    toggleButton2 = nullptr;
    slider = nullptr;
    comboBox = nullptr;
    comboBox2 = nullptr;
    label2 = nullptr;
    textEditor = nullptr;
    label5 = nullptr;
    m_to = nullptr;
    m_lowKey = nullptr;
    m_highKey = nullptr;
    imageButton = nullptr;
    label = nullptr;
    label7 = nullptr;
    label9 = nullptr;
    label3 = nullptr;
    m_keyboard = nullptr;
    toggleButton3 = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void Forte::paint (Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (Colour (0xff323e44));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void Forte::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void Forte::buttonClicked (Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == toggleButton.get())
    {
        //[UserButtonCode_toggleButton] -- add your button handler code here..
        //[/UserButtonCode_toggleButton]
    }
    else if (buttonThatWasClicked == toggleButton2.get())
    {
        //[UserButtonCode_toggleButton2] -- add your button handler code here..
        //[/UserButtonCode_toggleButton2]
    }
    else if (buttonThatWasClicked == imageButton.get())
    {
        //[UserButtonCode_imageButton] -- add your button handler code here..
        //[/UserButtonCode_imageButton]
    }
    else if (buttonThatWasClicked == toggleButton3.get())
    {
        //[UserButtonCode_toggleButton3] -- add your button handler code here..
        //[/UserButtonCode_toggleButton3]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}

void Forte::sliderValueChanged (Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == slider.get())
    {
        //[UserSliderCode_slider] -- add your slider handling code here..
        //[/UserSliderCode_slider]
    }

    //[UsersliderValueChanged_Post]
    //[/UsersliderValueChanged_Post]
}

void Forte::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
    //[UsercomboBoxChanged_Pre]
    //[/UsercomboBoxChanged_Pre]

    if (comboBoxThatHasChanged == comboBox.get())
    {
        //[UserComboBoxCode_comboBox] -- add your combo box handling code here..
        //[/UserComboBoxCode_comboBox]
    }
    else if (comboBoxThatHasChanged == comboBox2.get())
    {
        //[UserComboBoxCode_comboBox2] -- add your combo box handling code here..
        //[/UserComboBoxCode_comboBox2]
    }

    //[UsercomboBoxChanged_Post]
    //[/UsercomboBoxChanged_Post]
}

void Forte::mouseDown (const MouseEvent& e)
{
    //[UserCode_mouseDown] -- Add your code here...
    mouseDrag(e);
    //[/UserCode_mouseDown]
}

void Forte::mouseDrag (const MouseEvent& e)
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

void Forte::mouseUp (const MouseEvent& e)
{
    //[UserCode_mouseUp] -- Add your code here...
    mouseDrag(e);
    //[/UserCode_mouseUp]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...

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

void Forte::UpdateKeyboard()
{
    auto lowkey = ParseNote(m_lowKey->getTextValue().toString().getCharPointer());
    auto highkey = ParseNote(m_highKey->getTextValue().toString().getCharPointer());
    m_keyboardState->allNotesOff(1);
    for(int i=lowkey;i<=highkey;++i)
        m_keyboardState->noteOn(1, i, 1.0f);
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="Forte" componentName="" parentClasses="public Component"
                 constructorParams="" variableInitialisers="" snapPixels="8" snapActive="1"
                 snapShown="1" overlayOpacity="0.330" fixedSize="1" initialWidth="1024"
                 initialHeight="768">
  <METHODS>
    <METHOD name="mouseDown (const MouseEvent&amp; e)"/>
    <METHOD name="mouseDrag (const MouseEvent&amp; e)"/>
    <METHOD name="mouseUp (const MouseEvent&amp; e)"/>
  </METHODS>
  <BACKGROUND backgroundColour="ff323e44"/>
  <GROUPCOMPONENT name="new group" id="85efcbef1342dec0" memberName="groupComponent"
                  virtualName="" explicitFocusOrder="0" pos="8 16 1000 80" title="Korg M1"/>
  <LABEL name="new label" id="1b2a0908338bf229" memberName="label4" virtualName=""
         explicitFocusOrder="0" pos="536 0 80 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Transpose" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15.00000000000000000000"
         kerning="0.00000000000000000000" bold="0" italic="0" justification="33"/>
  <TOGGLEBUTTON name="" id="2b62ef4a67b701f3" memberName="toggleButton" virtualName=""
                explicitFocusOrder="0" pos="104 32 72 24" buttonText="Solo" connectedEdges="0"
                needsCallback="1" radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="new toggle button" id="6ad6011b41b23475" memberName="toggleButton2"
                virtualName="" explicitFocusOrder="0" pos="168 32 72 24" buttonText="Mute"
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
  <SLIDER name="new slider" id="a0e2bc5a61933c6d" memberName="slider" virtualName=""
          explicitFocusOrder="0" pos="104 64 128 24" min="-110.00000000000000000000"
          max="12.00000000000000000000" int="0.50000000000000000000" style="LinearBar"
          textBoxPos="TextBoxBelow" textBoxEditable="1" textBoxWidth="80"
          textBoxHeight="20" skewFactor="1.00000000000000000000" needsCallback="1"/>
  <COMBOBOX name="new combo box" id="90d63ca95a92a112" memberName="comboBox"
            virtualName="" explicitFocusOrder="0" pos="240 32 150 24" editable="0"
            layout="33" items="None&#10;Usercard 1&#10;Usercard 2&#10;" textWhenNonSelected="None"
            textWhenNoItems="(no choices)"/>
  <COMBOBOX name="new combo box" id="9de3cb5469378fa1" memberName="comboBox2"
            virtualName="" explicitFocusOrder="0" pos="240 64 150 24" editable="0"
            layout="33" items="Piano&#10;Strings&#10;" textWhenNonSelected="Piano"
            textWhenNoItems="(no choices)"/>
  <LABEL name="new label" id="c2703f80f1b7b4ae" memberName="label2" virtualName=""
         explicitFocusOrder="0" pos="264 0 104 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Bank/Program&#10;" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.00000000000000000000" kerning="0.00000000000000000000"
         bold="0" italic="0" justification="33"/>
  <TEXTEDITOR name="new text editor" id="b6e30577b79a003a" memberName="textEditor"
              virtualName="" explicitFocusOrder="0" pos="552 32 32 24" initialText=""
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="0"
              caret="1" popupmenu="1"/>
  <LABEL name="new label" id="c3e6077209440ad9" memberName="label5" virtualName=""
         explicitFocusOrder="0" pos="640 0 56 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Range" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15.00000000000000000000"
         kerning="0.00000000000000000000" bold="0" italic="0" justification="33"/>
  <LABEL name="" id="72d9777463cc6a85" memberName="m_to" virtualName=""
         explicitFocusOrder="0" pos="656 32 24 24" edTextCol="ff000000"
         edBkgCol="0" labelText="to" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15.00000000000000000000"
         kerning="0.00000000000000000000" bold="0" italic="0" justification="33"/>
  <TEXTEDITOR name="" id="3d470180923a3d6f" memberName="m_lowKey" virtualName=""
              explicitFocusOrder="0" pos="624 32 32 24" initialText="C -2"
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="0"
              caret="1" popupmenu="1"/>
  <TEXTEDITOR name="" id="5f3abd7bbb50678c" memberName="m_highKey" virtualName=""
              explicitFocusOrder="0" pos="680 32 32 24" initialText="G 8" multiline="0"
              retKeyStartsLine="0" readonly="0" scrollbars="0" caret="1" popupmenu="1"/>
  <IMAGEBUTTON name="new button" id="31b2ae44720b5f47" memberName="imageButton"
               virtualName="" explicitFocusOrder="0" pos="16 32 76 57" buttonText="new button"
               connectedEdges="0" needsCallback="1" radioGroupId="0" keepProportions="1"
               resourceNormal="truePianos_png" opacityNormal="1.00000000000000000000"
               colourNormal="0" resourceOver="" opacityOver="1.00000000000000000000"
               colourOver="0" resourceDown="" opacityDown="1.00000000000000000000"
               colourDown="0"/>
  <LABEL name="new label" id="35b886c1bcdf396b" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="136 128 150 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Songs" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15.00000000000000000000"
         kerning="0.00000000000000000000" bold="0" italic="0" justification="33"/>
  <LABEL name="new label" id="b82268e8804bf625" memberName="label7" virtualName=""
         explicitFocusOrder="0" pos="136 112 150 24" edTextCol="ff000000"
         edBkgCol="0" labelText="SetLists" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15.00000000000000000000"
         kerning="0.00000000000000000000" bold="0" italic="0" justification="33"/>
  <LABEL name="new label" id="71e88b4332596fb1" memberName="label9" virtualName=""
         explicitFocusOrder="0" pos="136 144 150 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Performances" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.00000000000000000000" kerning="0.00000000000000000000"
         bold="0" italic="0" justification="33"/>
  <LABEL name="new label" id="f934b675a0c74566" memberName="label3" virtualName=""
         explicitFocusOrder="0" pos="144 0 64 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Volume" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15.00000000000000000000"
         kerning="0.00000000000000000000" bold="0" italic="0" justification="33"/>
  <GENERICCOMPONENT name="" id="3a433662794e0409" memberName="m_keyboard" virtualName="MidiKeyboardComponent"
                    explicitFocusOrder="0" pos="400 64 600 24" class="unknown" params="*m_keyboardState, MidiKeyboardComponent::Orientation::horizontalKeyboard"/>
  <TOGGLEBUTTON name="new toggle button" id="7a9e84b485ffe060" memberName="toggleButton3"
                virtualName="" explicitFocusOrder="0" pos="400 32 123 24" buttonText="Double octave"
                connectedEdges="0" needsCallback="1" radioGroupId="0" state="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif

//==============================================================================
// Binary resources - be careful not to edit any of these sections!

// JUCER_RESOURCE: truePianos_png, 9333, "../m1.png"
static const unsigned char resource_Forte_truePianos_png[] = { 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,76,0,0,0,57,8,2,0,0,0,19,26,90,14,0,0,0,9,112,72,89,115,0,0,11,19,0,0,11,19,1,0,154,156,
24,0,0,10,79,105,67,67,80,80,104,111,116,111,115,104,111,112,32,73,67,67,32,112,114,111,102,105,108,101,0,0,120,218,157,83,103,84,83,233,22,61,247,222,244,66,75,136,128,148,75,111,82,21,8,32,82,66,139,
128,20,145,38,42,33,9,16,74,136,33,161,217,21,81,193,17,69,69,4,27,200,160,136,3,142,142,128,140,21,81,44,12,138,10,216,7,228,33,162,142,131,163,136,138,202,251,225,123,163,107,214,188,247,230,205,254,
181,215,62,231,172,243,157,179,207,7,192,8,12,150,72,51,81,53,128,12,169,66,30,17,224,131,199,196,198,225,228,46,64,129,10,36,112,0,16,8,179,100,33,115,253,35,1,0,248,126,60,60,43,34,192,7,190,0,1,120,
211,11,8,0,192,77,155,192,48,28,135,255,15,234,66,153,92,1,128,132,1,192,116,145,56,75,8,128,20,0,64,122,142,66,166,0,64,70,1,128,157,152,38,83,0,160,4,0,96,203,99,98,227,0,80,45,0,96,39,127,230,211,0,
128,157,248,153,123,1,0,91,148,33,21,1,160,145,0,32,19,101,136,68,0,104,59,0,172,207,86,138,69,0,88,48,0,20,102,75,196,57,0,216,45,0,48,73,87,102,72,0,176,183,0,192,206,16,11,178,0,8,12,0,48,81,136,133,
41,0,4,123,0,96,200,35,35,120,0,132,153,0,20,70,242,87,60,241,43,174,16,231,42,0,0,120,153,178,60,185,36,57,69,129,91,8,45,113,7,87,87,46,30,40,206,73,23,43,20,54,97,2,97,154,64,46,194,121,153,25,50,129,
52,15,224,243,204,0,0,160,145,21,17,224,131,243,253,120,206,14,174,206,206,54,142,182,14,95,45,234,191,6,255,34,98,98,227,254,229,207,171,112,64,0,0,225,116,126,209,254,44,47,179,26,128,59,6,128,109,254,
162,37,238,4,104,94,11,160,117,247,139,102,178,15,64,181,0,160,233,218,87,243,112,248,126,60,60,69,161,144,185,217,217,229,228,228,216,74,196,66,91,97,202,87,125,254,103,194,95,192,87,253,108,249,126,
60,252,247,245,224,190,226,36,129,50,93,129,71,4,248,224,194,204,244,76,165,28,207,146,9,132,98,220,230,143,71,252,183,11,255,252,29,211,34,196,73,98,185,88,42,20,227,81,18,113,142,68,154,140,243,50,165,
34,137,66,146,41,197,37,210,255,100,226,223,44,251,3,62,223,53,0,176,106,62,1,123,145,45,168,93,99,3,246,75,39,16,88,116,192,226,247,0,0,242,187,111,193,212,40,8,3,128,104,131,225,207,119,255,239,63,253,
71,160,37,0,128,102,73,146,113,0,0,94,68,36,46,84,202,179,63,199,8,0,0,68,160,129,42,176,65,27,244,193,24,44,192,6,28,193,5,220,193,11,252,96,54,132,66,36,196,194,66,16,66,10,100,128,28,114,96,41,172,
130,66,40,134,205,176,29,42,96,47,212,64,29,52,192,81,104,134,147,112,14,46,194,85,184,14,61,112,15,250,97,8,158,193,40,188,129,9,4,65,200,8,19,97,33,218,136,1,98,138,88,35,142,8,23,153,133,248,33,193,
72,4,18,139,36,32,201,136,20,81,34,75,145,53,72,49,82,138,84,32,85,72,29,242,61,114,2,57,135,92,70,186,145,59,200,0,50,130,252,134,188,71,49,148,129,178,81,61,212,12,181,67,185,168,55,26,132,70,162,11,
208,100,116,49,154,143,22,160,155,208,114,180,26,61,140,54,161,231,208,171,104,15,218,143,62,67,199,48,192,232,24,7,51,196,108,48,46,198,195,66,177,56,44,9,147,99,203,177,34,172,12,171,198,26,176,86,172,
3,187,137,245,99,207,177,119,4,18,129,69,192,9,54,4,119,66,32,97,30,65,72,88,76,88,78,216,72,168,32,28,36,52,17,218,9,55,9,3,132,81,194,39,34,147,168,75,180,38,186,17,249,196,24,98,50,49,135,88,72,44,
35,214,18,143,19,47,16,123,136,67,196,55,36,18,137,67,50,39,185,144,2,73,177,164,84,210,18,210,70,210,110,82,35,233,44,169,155,52,72,26,35,147,201,218,100,107,178,7,57,148,44,32,43,200,133,228,157,228,
195,228,51,228,27,228,33,242,91,10,157,98,64,113,164,248,83,226,40,82,202,106,74,25,229,16,229,52,229,6,101,152,50,65,85,163,154,82,221,168,161,84,17,53,143,90,66,173,161,182,82,175,81,135,168,19,52,117,
154,57,205,131,22,73,75,165,173,162,149,211,26,104,23,104,247,105,175,232,116,186,17,221,149,30,78,151,208,87,210,203,233,71,232,151,232,3,244,119,12,13,134,21,131,199,136,103,40,25,155,24,7,24,103,25,
119,24,175,152,76,166,25,211,139,25,199,84,48,55,49,235,152,231,153,15,153,111,85,88,42,182,42,124,21,145,202,10,149,74,149,38,149,27,42,47,84,169,170,166,170,222,170,11,85,243,85,203,84,143,169,94,83,
125,174,70,85,51,83,227,169,9,212,150,171,85,170,157,80,235,83,27,83,103,169,59,168,135,170,103,168,111,84,63,164,126,89,253,137,6,89,195,76,195,79,67,164,81,160,177,95,227,188,198,32,11,99,25,179,120,
44,33,107,13,171,134,117,129,53,196,38,177,205,217,124,118,42,187,152,253,29,187,139,61,170,169,161,57,67,51,74,51,87,179,82,243,148,102,63,7,227,152,113,248,156,116,78,9,231,40,167,151,243,126,138,222,
20,239,41,226,41,27,166,52,76,185,49,101,92,107,170,150,151,150,88,171,72,171,81,171,71,235,189,54,174,237,167,157,166,189,69,187,89,251,129,14,65,199,74,39,92,39,71,103,143,206,5,157,231,83,217,83,221,
167,10,167,22,77,61,58,245,174,46,170,107,165,27,161,187,68,119,191,110,167,238,152,158,190,94,128,158,76,111,167,222,121,189,231,250,28,125,47,253,84,253,109,250,167,245,71,12,88,6,179,12,36,6,219,12,
206,24,60,197,53,113,111,60,29,47,199,219,241,81,67,93,195,64,67,165,97,149,97,151,225,132,145,185,209,60,163,213,70,141,70,15,140,105,198,92,227,36,227,109,198,109,198,163,38,6,38,33,38,75,77,234,77,
238,154,82,77,185,166,41,166,59,76,59,76,199,205,204,205,162,205,214,153,53,155,61,49,215,50,231,155,231,155,215,155,223,183,96,90,120,90,44,182,168,182,184,101,73,178,228,90,166,89,238,182,188,110,133,
90,57,89,165,88,85,90,93,179,70,173,157,173,37,214,187,173,187,167,17,167,185,78,147,78,171,158,214,103,195,176,241,182,201,182,169,183,25,176,229,216,6,219,174,182,109,182,125,97,103,98,23,103,183,197,
174,195,238,147,189,147,125,186,125,141,253,61,7,13,135,217,14,171,29,90,29,126,115,180,114,20,58,86,58,222,154,206,156,238,63,125,197,244,150,233,47,103,88,207,16,207,216,51,227,182,19,203,41,196,105,
157,83,155,211,71,103,23,103,185,115,131,243,136,139,137,75,130,203,46,151,62,46,155,27,198,221,200,189,228,74,116,245,113,93,225,122,210,245,157,155,179,155,194,237,168,219,175,238,54,238,105,238,135,
220,159,204,52,159,41,158,89,51,115,208,195,200,67,224,81,229,209,63,11,159,149,48,107,223,172,126,79,67,79,129,103,181,231,35,47,99,47,145,87,173,215,176,183,165,119,170,247,97,239,23,62,246,62,114,159,
227,62,227,60,55,222,50,222,89,95,204,55,192,183,200,183,203,79,195,111,158,95,133,223,67,127,35,255,100,255,122,255,209,0,167,128,37,1,103,3,137,129,65,129,91,2,251,248,122,124,33,191,142,63,58,219,101,
246,178,217,237,65,140,160,185,65,21,65,143,130,173,130,229,193,173,33,104,200,236,144,173,33,247,231,152,206,145,206,105,14,133,80,126,232,214,208,7,97,230,97,139,195,126,12,39,133,135,133,87,134,63,
142,112,136,88,26,209,49,151,53,119,209,220,67,115,223,68,250,68,150,68,222,155,103,49,79,57,175,45,74,53,42,62,170,46,106,60,218,55,186,52,186,63,198,46,102,89,204,213,88,157,88,73,108,75,28,57,46,42,
174,54,110,108,190,223,252,237,243,135,226,157,226,11,227,123,23,152,47,200,93,112,121,161,206,194,244,133,167,22,169,46,18,44,58,150,64,76,136,78,56,148,240,65,16,42,168,22,140,37,242,19,119,37,142,10,
121,194,29,194,103,34,47,209,54,209,136,216,67,92,42,30,78,242,72,42,77,122,146,236,145,188,53,121,36,197,51,165,44,229,185,132,39,169,144,188,76,13,76,221,155,58,158,22,154,118,32,109,50,61,58,189,49,
131,146,145,144,113,66,170,33,77,147,182,103,234,103,230,102,118,203,172,101,133,178,254,197,110,139,183,47,30,149,7,201,107,179,144,172,5,89,45,10,182,66,166,232,84,90,40,215,42,7,178,103,101,87,102,
191,205,137,202,57,150,171,158,43,205,237,204,179,202,219,144,55,156,239,159,255,237,18,194,18,225,146,182,165,134,75,87,45,29,88,230,189,172,106,57,178,60,113,121,219,10,227,21,5,43,134,86,6,172,60,184,
138,182,42,109,213,79,171,237,87,151,174,126,189,38,122,77,107,129,94,193,202,130,193,181,1,107,235,11,85,10,229,133,125,235,220,215,237,93,79,88,47,89,223,181,97,250,134,157,27,62,21,137,138,174,20,219,
23,151,21,127,216,40,220,120,229,27,135,111,202,191,153,220,148,180,169,171,196,185,100,207,102,210,102,233,230,222,45,158,91,14,150,170,151,230,151,14,110,13,217,218,180,13,223,86,180,237,245,246,69,
219,47,151,205,40,219,187,131,182,67,185,163,191,60,184,188,101,167,201,206,205,59,63,84,164,84,244,84,250,84,54,238,210,221,181,97,215,248,110,209,238,27,123,188,246,52,236,213,219,91,188,247,253,62,
201,190,219,85,1,85,77,213,102,213,101,251,73,251,179,247,63,174,137,170,233,248,150,251,109,93,173,78,109,113,237,199,3,210,3,253,7,35,14,182,215,185,212,213,29,210,61,84,82,143,214,43,235,71,14,199,
31,190,254,157,239,119,45,13,54,13,85,141,156,198,226,35,112,68,121,228,233,247,9,223,247,30,13,58,218,118,140,123,172,225,7,211,31,118,29,103,29,47,106,66,154,242,154,70,155,83,154,251,91,98,91,186,79,
204,62,209,214,234,222,122,252,71,219,31,15,156,52,60,89,121,74,243,84,201,105,218,233,130,211,147,103,242,207,140,157,149,157,125,126,46,249,220,96,219,162,182,123,231,99,206,223,106,15,111,239,186,16,
116,225,210,69,255,139,231,59,188,59,206,92,242,184,116,242,178,219,229,19,87,184,87,154,175,58,95,109,234,116,234,60,254,147,211,79,199,187,156,187,154,174,185,92,107,185,238,122,189,181,123,102,247,
233,27,158,55,206,221,244,189,121,241,22,255,214,213,158,57,61,221,189,243,122,111,247,197,247,245,223,22,221,126,114,39,253,206,203,187,217,119,39,238,173,188,79,188,95,244,64,237,65,217,67,221,135,213,
63,91,254,220,216,239,220,127,106,192,119,160,243,209,220,71,247,6,133,131,207,254,145,245,143,15,67,5,143,153,143,203,134,13,134,235,158,56,62,57,57,226,63,114,253,233,252,167,67,207,100,207,38,158,23,
254,162,254,203,174,23,22,47,126,248,213,235,215,206,209,152,209,161,151,242,151,147,191,109,124,165,253,234,192,235,25,175,219,198,194,198,30,190,201,120,51,49,94,244,86,251,237,193,119,220,119,29,239,
163,223,15,79,228,124,32,127,40,255,104,249,177,245,83,208,167,251,147,25,147,147,255,4,3,152,243,252,99,51,45,219,0,0,0,32,99,72,82,77,0,0,122,37,0,0,128,131,0,0,249,255,0,0,128,233,0,0,117,48,0,0,234,
96,0,0,58,152,0,0,23,111,146,95,197,70,0,0,25,160,73,68,65,84,120,218,148,122,121,152,100,85,149,231,57,247,222,183,196,139,125,207,200,204,218,161,168,162,128,66,132,66,10,24,215,161,177,161,249,148,
102,81,176,154,210,233,198,129,161,149,229,67,28,1,81,10,74,4,157,79,160,63,253,160,5,165,65,5,41,28,28,101,233,111,80,246,42,182,41,160,64,10,106,35,51,171,42,247,53,34,94,68,188,229,222,51,127,188,136,
200,23,153,89,133,190,136,63,222,114,223,125,247,236,191,115,206,197,149,71,29,9,179,135,2,144,193,25,17,33,162,146,12,17,219,143,17,145,136,66,227,33,184,12,143,105,143,4,0,64,5,64,0,160,20,33,2,67,78,
132,0,128,161,95,120,60,17,32,2,34,34,34,17,176,214,180,193,77,0,96,12,1,48,244,57,4,64,32,194,230,107,208,252,44,34,34,50,198,16,9,17,5,73,63,180,178,230,215,184,16,156,115,34,34,193,164,239,43,34,34,
34,165,40,88,9,209,236,234,2,26,40,116,17,38,21,128,66,247,36,53,7,6,28,12,206,58,94,12,93,34,34,80,248,237,128,0,10,141,106,51,26,24,99,1,195,219,228,7,99,2,206,8,80,178,181,30,112,60,223,243,0,16,116,
29,12,67,32,34,99,210,243,60,223,247,132,208,0,136,148,106,174,27,17,40,96,98,231,226,176,41,158,214,109,6,192,66,151,10,81,53,63,22,188,222,38,34,180,40,34,2,34,0,36,2,108,81,21,200,54,172,74,68,32,165,
47,149,12,232,82,164,16,25,2,42,165,4,231,129,96,56,19,156,11,225,7,250,6,132,8,82,41,9,64,82,57,85,167,92,41,3,0,128,71,74,9,33,98,177,88,83,93,3,45,84,4,0,164,8,219,203,234,92,107,235,14,1,32,205,14,
160,89,13,104,174,126,118,134,64,116,193,72,162,166,16,169,101,35,45,187,224,208,154,194,117,221,197,75,151,45,95,190,220,182,237,90,173,150,76,38,135,135,134,129,200,48,205,225,225,65,33,184,174,235,
118,213,174,148,43,130,148,106,207,165,105,66,232,77,30,183,196,68,164,20,34,42,165,230,216,94,123,53,48,239,80,74,133,70,82,248,149,249,227,3,242,194,143,22,52,251,214,185,106,127,34,145,136,159,254,
95,62,155,201,22,251,250,62,204,100,178,66,136,98,247,68,87,87,161,94,175,245,247,247,21,10,121,199,113,94,219,246,210,228,244,180,104,173,30,136,154,214,22,118,30,72,16,94,88,235,49,181,216,25,62,9,203,
178,115,162,78,122,22,164,112,33,122,22,112,105,45,67,38,34,50,13,243,245,55,94,69,174,113,198,106,245,210,216,216,88,60,145,168,216,182,235,52,44,203,226,66,159,24,26,217,127,96,63,231,28,23,245,22,59,
190,138,29,159,199,5,228,180,192,90,23,228,122,219,212,195,207,155,246,216,26,214,158,103,14,73,33,253,156,67,36,159,245,252,74,249,74,42,82,24,120,111,6,136,26,71,29,16,116,77,51,76,221,182,109,37,61,
68,152,85,215,182,123,109,205,2,74,17,118,46,186,205,245,5,181,116,33,70,204,151,12,155,47,177,176,101,30,158,119,109,117,13,76,86,48,78,138,1,0,10,84,68,72,128,202,5,32,95,58,94,67,49,68,142,140,20,137,
64,189,88,235,27,129,246,182,201,166,78,185,41,245,17,180,29,86,170,115,86,121,72,69,157,127,217,33,240,150,27,166,166,151,11,252,176,2,64,194,112,112,99,18,32,248,128,8,66,151,130,80,12,232,252,70,32,
82,248,91,142,249,218,24,246,159,225,80,49,103,228,71,74,178,211,242,161,211,183,53,93,119,248,81,112,42,84,104,118,236,80,140,118,92,198,217,247,14,97,138,115,99,58,52,25,170,136,218,58,75,225,165,146,
106,138,113,14,138,152,111,171,115,239,31,214,71,16,44,232,18,68,251,245,182,114,134,7,52,67,85,135,255,88,72,9,103,141,186,101,180,237,48,216,249,110,219,157,52,63,212,25,54,1,231,58,172,185,195,90,218,
222,134,129,243,68,2,243,1,166,152,7,145,16,14,67,80,11,139,30,74,163,90,18,8,71,29,58,20,226,253,235,92,215,194,38,221,114,144,56,11,31,195,216,175,115,126,65,72,11,72,228,80,86,209,194,100,11,68,122,
104,177,150,112,22,239,97,120,89,11,123,148,185,196,32,40,9,210,247,25,3,69,74,8,17,60,69,152,135,92,17,16,24,1,1,74,2,64,226,68,29,43,66,68,206,152,47,37,183,98,102,160,167,173,199,8,127,251,209,180,
174,144,98,19,53,163,81,128,213,230,11,115,97,178,49,192,250,252,196,147,62,113,234,233,167,151,74,221,7,246,31,144,126,128,159,136,136,92,79,18,160,84,10,16,25,67,34,84,36,1,3,144,137,237,175,132,77,
154,136,4,40,252,91,195,192,161,252,231,66,227,41,28,87,255,154,232,218,112,188,83,79,251,204,231,254,235,153,142,84,133,82,245,213,173,175,59,142,143,12,165,244,20,209,177,39,156,124,228,17,71,72,223,
123,101,219,214,241,209,17,198,80,211,77,33,88,205,182,1,169,141,93,230,184,79,177,160,232,62,114,53,97,53,91,16,142,54,135,17,205,177,225,192,32,112,54,81,12,131,111,116,93,111,233,210,101,159,252,228,
167,247,236,235,143,37,19,110,205,110,212,29,198,184,235,186,186,105,126,254,204,191,91,125,204,137,74,202,136,161,239,217,181,103,104,240,160,97,234,103,254,253,63,44,238,237,29,28,60,240,199,63,252,
222,243,220,0,108,204,9,176,236,175,87,200,240,49,239,102,0,125,155,147,30,10,244,205,119,42,97,6,249,190,103,152,145,51,206,60,171,94,175,191,246,202,214,152,161,149,167,198,60,191,230,184,78,38,91,184,
120,195,215,150,44,93,185,107,215,110,134,236,131,157,59,63,120,255,61,157,51,33,68,182,80,140,39,210,141,134,116,28,119,238,42,91,7,107,38,215,179,255,185,241,175,153,220,117,138,164,77,89,107,149,42,
248,43,37,21,169,57,176,105,86,107,1,144,0,9,16,88,224,162,90,115,16,17,248,62,156,246,201,207,245,244,46,249,207,167,159,246,221,70,42,22,157,24,29,171,214,106,171,86,173,222,248,213,175,145,132,39,254,
240,84,54,155,243,61,231,197,23,158,85,158,163,60,111,201,210,101,137,100,122,114,106,250,249,231,159,37,82,173,60,123,174,102,49,34,21,100,145,45,43,237,56,102,169,71,68,68,32,64,10,192,83,19,93,0,42,
100,4,72,4,42,248,7,46,56,240,61,42,136,57,164,0,72,42,170,55,92,233,122,202,151,74,41,82,1,30,99,8,0,32,93,79,174,89,115,226,250,245,159,221,190,253,237,29,111,111,239,237,41,42,226,35,227,149,83,63,
117,198,121,23,94,188,119,239,135,15,62,240,203,99,86,173,236,233,238,125,237,149,109,67,7,251,24,195,88,60,115,234,233,159,225,154,190,109,219,139,83,147,163,156,49,0,214,90,103,115,133,109,155,252,8,
205,58,4,180,233,72,109,15,25,54,91,182,41,149,210,140,200,199,78,92,55,83,153,154,152,152,176,43,101,167,81,147,210,103,74,112,142,132,202,208,205,99,142,89,57,58,248,225,11,127,122,90,231,208,149,207,
75,233,159,118,218,41,165,66,250,185,63,63,243,204,159,254,124,252,241,39,172,253,248,186,129,189,187,95,127,101,155,38,116,199,245,62,249,217,245,93,165,174,189,123,118,239,120,235,77,93,19,243,75,35,
109,245,17,109,56,217,137,87,23,14,223,115,198,44,152,4,206,197,171,0,4,224,251,244,233,207,125,234,19,167,127,186,214,112,235,245,106,173,90,46,79,79,76,77,78,76,142,142,78,78,142,207,204,76,55,26,141,
45,143,61,2,10,164,239,35,194,174,93,187,18,153,130,227,58,143,62,242,244,95,118,126,208,221,221,115,214,217,255,224,73,245,252,159,255,175,219,168,3,192,146,165,43,142,59,225,227,141,186,253,242,11,207,
123,174,171,107,44,128,159,184,208,146,4,0,50,118,24,63,161,58,69,218,89,25,232,76,166,194,56,187,69,42,2,144,231,249,199,173,61,97,221,41,167,165,51,197,158,104,210,182,43,210,247,42,229,25,207,115,128,
124,215,105,216,213,74,165,50,61,57,49,49,62,54,54,53,49,89,158,158,222,185,243,131,29,59,119,58,174,203,164,226,66,51,76,237,131,157,239,142,77,76,244,13,236,225,130,9,110,158,113,198,231,211,137,244,
139,47,189,240,225,222,221,166,206,23,174,21,2,16,17,99,12,11,133,220,130,65,172,5,217,85,167,36,67,174,95,17,50,80,160,22,82,214,150,45,19,2,40,165,84,174,216,157,239,94,92,44,45,90,220,219,219,211,211,
147,202,100,166,103,102,98,137,68,60,150,172,215,234,36,93,187,90,150,210,241,124,223,117,92,175,102,219,118,117,98,114,120,98,106,114,124,108,102,106,114,114,114,106,178,94,173,250,190,139,92,106,92,
143,89,233,181,107,79,72,36,83,219,94,125,185,50,51,161,9,6,0,4,140,0,144,228,188,36,29,177,152,207,55,145,129,34,68,32,36,34,106,21,159,8,104,78,150,21,148,204,96,14,128,10,49,34,0,55,109,10,155,195,
164,84,158,47,1,80,8,97,89,86,58,147,41,22,139,221,221,61,197,222,149,133,98,49,151,203,155,17,83,73,197,56,52,234,53,223,113,60,215,117,221,186,35,253,105,88,62,13,61,147,206,76,99,248,160,176,223,80,
83,131,251,251,7,70,71,134,43,229,105,34,41,56,227,28,195,160,127,78,100,34,16,136,128,165,174,146,82,10,32,72,151,17,144,2,103,8,1,181,179,228,80,40,53,89,216,221,132,136,4,0,118,136,49,168,148,82,74,
145,82,4,32,244,104,36,98,101,178,217,98,169,187,84,42,149,122,151,100,179,249,76,46,27,141,197,125,199,125,239,224,228,176,118,74,141,119,19,186,57,89,253,251,147,172,238,148,87,41,207,56,246,228,19,
255,251,183,15,62,240,115,206,217,2,232,183,67,146,92,74,95,112,206,25,103,68,132,74,133,99,61,41,130,150,59,238,204,40,145,254,198,28,122,110,212,98,140,177,160,20,129,140,26,13,219,62,80,25,219,187,
107,135,16,58,19,102,34,153,201,228,11,197,238,158,98,87,87,180,176,154,103,84,66,84,1,220,163,151,71,11,217,184,237,84,141,88,94,48,246,250,235,219,65,42,38,56,132,204,111,62,169,210,119,24,71,65,45,
79,203,24,107,41,36,151,210,111,198,243,118,237,109,22,46,81,155,121,243,139,107,179,62,156,104,65,192,24,212,237,91,117,29,64,36,37,125,203,138,46,95,190,108,100,100,204,50,116,187,58,254,225,248,193,
93,239,110,71,46,162,209,100,60,115,111,188,184,60,87,88,89,58,249,216,189,238,145,185,174,66,34,155,184,247,161,255,120,235,173,237,241,136,118,232,34,75,179,210,133,136,68,10,123,22,245,2,145,84,10,
16,64,1,2,7,80,173,24,163,136,72,74,217,188,194,118,161,152,14,77,100,139,54,194,195,39,52,193,120,41,105,213,170,213,66,51,234,141,198,204,76,185,84,42,88,166,81,171,213,164,84,149,74,185,50,105,219,
149,113,219,119,148,138,50,189,28,183,242,249,194,138,222,69,221,239,188,179,189,86,157,98,92,33,181,203,41,56,63,122,41,165,16,185,38,56,246,44,234,149,82,54,11,144,128,65,81,188,149,4,6,177,62,48,101,
165,148,34,82,68,42,84,218,87,200,230,212,239,66,68,6,173,132,195,68,39,162,21,43,142,50,12,83,1,8,46,132,166,151,203,229,209,209,145,98,161,24,137,152,134,110,144,162,190,254,247,75,165,146,93,241,43,
213,137,234,140,107,219,101,73,190,97,234,140,161,10,18,40,90,32,114,4,213,231,224,142,97,24,92,55,116,203,178,0,81,250,126,171,36,60,27,229,168,93,168,7,12,44,137,33,11,163,220,64,253,90,93,167,16,58,
71,192,112,215,106,161,60,6,136,172,104,204,52,77,233,251,3,253,3,81,43,162,9,225,185,110,58,149,50,116,195,113,156,134,211,152,154,42,91,86,210,48,141,100,42,183,98,229,138,120,60,58,53,61,217,74,163,
233,35,115,0,68,148,82,114,215,115,211,233,116,79,119,119,34,145,64,64,207,243,194,146,105,243,166,213,69,11,252,103,64,47,34,178,102,213,143,16,8,105,214,176,219,124,197,195,39,107,83,83,101,203,138,
77,76,76,118,149,74,140,49,206,49,151,203,217,118,109,247,238,221,169,84,10,0,108,219,206,102,179,0,36,4,175,215,107,125,125,125,158,231,206,169,114,29,134,194,224,146,147,82,185,108,198,247,252,88,212,
74,38,147,181,90,163,86,179,57,231,1,15,8,230,52,42,16,16,41,176,78,108,31,172,73,27,33,129,106,223,157,95,103,152,87,14,71,206,120,165,82,38,82,137,120,50,106,89,66,240,221,187,119,233,186,110,89,22,
231,130,8,122,123,23,217,182,29,172,187,92,158,30,31,31,103,172,89,37,8,183,34,195,36,205,2,157,214,29,113,255,125,63,239,93,212,251,202,43,175,60,254,248,227,74,169,114,121,102,114,114,82,215,181,82,
169,219,178,172,106,205,110,52,26,115,26,50,45,109,1,106,149,216,131,73,131,208,24,174,80,135,91,38,11,186,65,68,95,17,56,142,247,193,7,239,48,38,12,211,148,82,149,74,165,116,58,35,37,41,69,53,187,225,
56,174,105,234,166,105,236,222,61,212,178,158,160,210,33,67,26,55,203,208,214,215,103,221,59,127,252,247,143,155,166,254,208,67,15,109,221,186,213,247,124,69,100,219,182,148,50,18,177,242,249,188,208,
180,153,233,25,223,247,57,227,64,65,187,170,35,37,9,52,182,5,0,154,78,10,145,49,198,155,158,12,2,191,199,130,14,239,108,123,120,182,21,173,16,65,104,12,144,124,223,39,160,209,145,209,131,7,15,76,79,79,
121,158,111,234,70,58,157,138,39,227,59,223,251,139,93,171,114,38,66,157,5,104,11,144,58,19,93,198,24,50,54,27,255,182,255,191,87,255,245,27,255,227,229,151,223,136,199,162,186,110,86,109,123,229,202,
149,66,136,98,177,216,215,223,231,186,222,254,253,7,60,215,137,88,209,82,169,36,4,171,85,171,245,70,67,41,197,24,3,108,183,79,130,52,18,145,160,163,201,135,12,90,90,64,68,115,66,54,17,33,169,67,217,149,
148,82,41,69,68,154,166,25,134,225,251,190,148,18,145,117,192,18,92,168,200,16,72,162,205,80,4,254,167,103,158,126,123,199,251,169,116,44,149,201,207,204,84,191,241,141,43,30,121,228,225,76,38,177,229,
177,223,14,15,15,42,169,106,53,155,72,73,223,143,199,162,217,108,54,147,201,36,18,9,33,132,244,101,179,102,208,66,115,48,219,168,104,49,24,59,80,78,96,39,29,29,171,67,23,202,2,91,231,156,3,128,148,62,
145,2,96,33,175,222,132,157,115,34,36,182,90,235,225,92,129,87,42,229,76,38,149,72,166,135,71,198,174,188,250,234,219,126,176,249,129,7,238,191,230,218,171,135,135,135,53,141,35,10,207,243,226,241,120,
62,159,95,186,116,217,200,200,112,173,86,215,117,45,145,72,120,158,91,46,151,137,136,115,14,64,42,104,203,83,39,78,69,22,118,9,109,104,21,28,65,69,188,77,240,124,183,52,223,135,205,153,16,91,53,11,0,192,
102,223,126,1,77,198,98,41,111,69,98,195,195,35,215,93,247,173,27,111,184,254,214,205,155,54,111,222,44,125,153,76,37,83,169,212,224,193,145,111,95,247,237,243,206,63,223,105,52,26,142,179,99,199,219,
219,182,109,125,247,221,119,171,213,234,196,196,196,248,248,4,17,68,162,86,38,147,97,140,26,13,71,122,126,155,175,65,116,165,133,234,90,179,99,72,181,93,69,11,163,224,130,77,203,80,59,61,52,3,2,133,179,
243,86,183,126,142,38,11,221,74,12,14,141,222,116,253,141,87,95,117,197,149,223,188,252,167,247,252,59,103,152,205,230,147,137,204,192,254,253,215,92,125,229,181,215,94,195,24,115,93,183,92,46,127,252,
227,39,0,208,179,207,62,39,165,244,92,143,154,85,54,95,8,158,201,164,28,199,29,29,25,245,61,47,144,18,133,100,212,130,145,129,199,163,142,180,40,48,239,192,13,114,30,228,40,179,3,218,85,64,196,195,212,
123,195,125,151,176,38,55,123,33,227,195,227,183,110,218,244,47,255,252,213,141,27,55,62,252,219,199,116,75,203,101,10,177,104,98,96,96,224,95,175,184,226,134,239,92,239,122,174,231,249,141,70,163,187,
103,209,163,143,62,244,157,239,92,59,83,174,34,50,221,176,2,54,42,223,19,140,170,229,138,244,161,102,215,107,181,154,16,60,18,137,68,34,150,82,228,75,41,21,53,183,238,32,107,98,135,22,133,138,24,50,30,
66,192,138,49,46,132,70,4,74,74,5,68,72,237,36,47,72,116,17,195,123,89,48,236,86,59,52,57,92,254,248,225,109,155,47,56,239,220,127,60,247,139,207,252,233,207,150,101,100,11,121,43,146,232,239,235,191,
252,178,255,254,189,239,221,232,186,174,175,164,227,56,221,61,139,126,241,139,251,191,249,205,203,29,199,49,12,45,159,47,77,207,84,63,245,169,207,156,182,126,253,226,37,221,35,35,195,247,221,119,191,231,
249,142,227,0,128,239,75,207,243,11,133,152,97,88,142,235,86,171,85,223,247,137,72,145,66,80,237,102,51,54,119,37,64,88,195,3,212,142,8,200,152,96,12,25,74,41,3,116,29,108,123,233,196,57,170,25,181,148,
68,228,128,208,214,154,14,77,222,187,111,207,151,46,60,255,245,215,223,140,197,140,92,46,167,155,209,129,129,3,95,255,151,175,111,222,188,201,247,28,41,85,163,209,40,117,247,222,125,247,157,223,250,246,
117,36,125,77,231,165,174,222,233,233,74,119,119,239,150,45,143,29,113,196,178,109,91,95,220,176,97,195,193,131,7,16,177,225,200,182,70,229,243,249,116,58,107,152,198,196,248,196,196,196,132,16,66,51,
116,206,177,157,15,180,180,90,181,65,47,54,133,220,110,31,181,56,209,66,161,68,82,74,217,118,75,179,249,157,82,12,121,199,102,173,48,145,171,86,173,124,255,253,93,177,120,36,159,207,27,134,209,223,127,
224,171,27,191,118,199,237,183,251,126,67,74,207,113,101,87,87,215,143,126,252,227,235,111,184,30,17,133,224,221,165,238,186,221,208,141,200,239,30,251,221,218,227,143,125,254,217,103,47,217,184,97,104,
232,160,16,220,52,163,102,36,54,54,54,198,57,95,181,106,85,169,84,218,247,225,135,53,219,158,158,158,174,84,42,0,128,140,197,226,177,88,44,38,101,176,7,202,15,229,208,72,68,36,37,103,216,218,6,68,128,
140,160,195,15,113,142,129,163,82,161,20,191,101,179,65,11,136,230,183,46,197,251,239,239,138,199,226,185,124,94,211,181,190,254,254,141,255,116,201,29,183,223,238,251,158,148,158,235,214,186,186,150,
108,254,193,15,110,186,233,187,200,152,166,107,165,82,143,227,120,158,84,191,254,197,47,215,30,127,236,11,207,61,247,79,151,92,52,52,60,172,9,102,89,241,98,161,167,111,160,239,198,27,111,60,235,172,179,
34,145,200,248,248,248,85,87,93,53,57,49,225,186,110,155,223,138,200,178,44,211,52,149,82,245,122,221,182,171,174,235,250,190,7,64,92,8,198,49,4,0,131,52,38,216,24,213,140,70,74,17,145,10,48,77,160,8,
190,239,75,73,225,78,118,139,41,42,152,138,148,196,68,34,145,203,229,53,77,239,31,24,184,248,162,139,239,252,201,79,148,114,60,223,113,29,183,80,40,109,186,229,230,77,183,220,194,4,55,116,163,171,187,
91,249,114,106,98,250,151,191,124,224,156,115,206,126,225,249,23,190,178,225,162,193,193,131,186,206,44,43,209,219,189,104,215,238,125,23,93,244,229,187,238,190,139,115,94,179,171,27,55,108,248,195,147,
79,106,6,15,118,239,5,106,6,8,185,92,54,22,139,71,44,75,215,52,64,229,186,110,189,214,168,213,235,78,195,105,135,56,198,88,96,127,109,164,17,168,107,7,210,104,214,43,144,49,166,148,106,107,114,27,154,
7,246,73,74,225,210,21,43,116,161,239,31,24,184,240,194,11,238,190,235,223,72,41,207,175,57,94,35,159,43,125,239,123,183,108,190,237,102,77,211,116,93,239,234,234,102,136,131,131,7,239,190,235,223,54,
110,220,248,242,203,91,191,178,225,75,7,7,247,107,66,139,68,162,189,61,75,62,252,176,239,148,83,214,63,252,235,223,112,77,248,210,187,226,178,75,183,60,252,104,36,166,53,164,204,164,138,66,232,83,83,99,
154,166,173,95,127,26,145,122,231,157,29,182,93,51,77,195,178,204,136,21,213,132,174,20,213,107,141,90,163,86,107,212,149,148,65,85,13,25,4,142,42,144,94,91,45,219,165,137,16,114,158,171,201,237,237,42,
136,40,116,161,15,12,244,255,227,185,95,188,243,39,119,18,185,158,95,119,28,149,207,247,124,247,166,235,111,187,237,135,134,105,112,206,187,186,74,156,243,254,254,254,91,54,109,222,184,113,227,107,175,
189,254,149,175,92,220,63,176,223,52,185,105,88,189,61,139,134,134,134,150,45,91,122,207,61,247,106,186,161,200,187,254,250,235,30,126,228,209,100,76,115,37,228,50,165,108,54,191,119,239,190,187,238,186,
123,221,186,147,117,221,168,84,202,59,119,190,251,242,214,23,183,110,221,58,60,52,50,57,57,21,137,68,163,209,152,174,233,113,61,238,249,110,165,94,211,52,93,8,78,4,82,130,148,10,49,240,181,32,165,39,165,
12,116,53,200,22,136,20,0,63,148,38,55,179,8,51,18,253,226,23,206,249,217,207,126,202,0,61,89,119,26,213,66,126,233,77,55,221,124,235,109,155,204,8,112,110,21,139,93,134,97,246,125,216,119,213,149,87,
223,122,235,166,55,223,122,231,252,243,191,184,119,207,94,221,196,72,36,182,184,119,233,204,204,52,17,252,254,247,255,103,245,170,163,21,193,205,223,191,225,246,59,238,136,197,184,148,42,149,44,245,244,
46,217,177,227,173,43,174,184,226,150,91,111,117,234,13,165,36,99,120,112,240,128,224,184,103,239,158,253,251,135,94,122,233,165,237,219,223,24,25,25,145,146,132,198,26,78,67,74,95,169,96,173,92,211,140,
0,139,182,28,141,108,231,186,129,173,114,206,25,99,135,215,100,56,255,130,11,167,166,166,202,51,83,19,227,195,131,7,250,60,183,113,227,13,215,3,128,110,104,150,101,44,93,190,108,245,154,163,141,72,228,
210,175,95,78,68,59,223,223,117,244,154,213,0,160,25,44,145,74,172,57,230,216,149,43,143,74,166,82,79,61,245,159,174,235,215,235,206,15,111,255,177,166,177,68,92,183,44,94,232,202,159,116,210,250,84,50,
123,230,231,207,156,158,153,26,31,31,31,30,30,169,148,167,246,237,221,213,223,183,175,81,171,85,43,149,177,177,177,61,123,246,60,243,204,51,95,248,194,57,186,33,172,168,30,137,233,166,37,130,191,97,105,
166,165,71,162,102,44,17,205,228,82,217,124,58,157,77,37,211,137,120,50,158,204,36,19,169,120,42,147,76,103,147,233,108,50,149,73,166,179,169,76,46,157,206,166,82,233,68,42,147,76,103,211,233,108,38,147,
203,166,179,25,254,210,139,47,11,198,125,223,115,27,181,98,161,251,214,91,239,248,254,166,239,107,58,10,161,21,11,139,173,168,217,223,223,127,206,217,231,252,252,190,127,223,191,127,240,75,95,186,96,199,
219,59,116,147,25,166,217,219,179,12,136,247,247,247,221,121,231,93,231,158,123,46,17,61,248,224,3,215,94,247,173,136,169,129,82,145,72,98,241,146,163,134,134,134,210,233,228,111,30,126,200,178,76,167,
225,154,134,86,175,87,61,215,47,22,186,165,66,4,174,235,70,52,22,45,22,139,93,93,93,187,118,237,28,28,26,66,54,139,75,89,179,76,68,82,121,158,235,122,190,7,4,130,115,46,56,131,102,195,16,145,49,38,24,
11,202,168,10,17,25,231,129,210,6,137,11,34,10,198,152,231,121,141,154,211,85,236,189,227,246,219,191,119,243,141,154,96,156,105,249,66,151,21,53,15,12,236,61,245,228,245,247,223,119,255,196,216,248,37,
27,190,252,230,155,111,152,81,131,33,239,237,94,34,4,255,96,231,251,223,249,159,223,222,120,201,70,165,224,137,39,255,120,245,53,87,1,184,190,212,12,45,186,100,233,10,187,82,177,171,211,191,126,232,241,
174,226,162,106,185,172,11,78,228,218,229,233,82,105,57,145,64,80,132,190,82,64,4,209,104,124,237,218,19,236,170,39,125,98,136,24,108,13,14,161,179,160,214,2,4,190,87,151,190,203,152,0,96,154,110,112,
206,3,179,12,42,170,92,136,192,141,19,1,162,18,130,55,45,115,98,124,244,192,254,126,223,117,238,184,237,7,28,65,8,48,77,125,201,226,197,107,142,57,38,157,201,157,120,210,186,145,145,209,169,169,153,51,
62,119,6,2,68,44,61,18,141,172,56,242,136,181,31,59,62,98,69,255,219,215,254,217,173,187,190,71,207,63,247,114,161,80,100,2,35,81,61,158,136,30,119,220,113,39,156,112,162,161,89,247,222,123,175,235,186,
163,35,163,227,99,227,51,211,19,251,118,255,101,102,98,168,81,181,171,229,134,93,169,85,171,149,74,165,90,169,216,82,170,203,46,187,28,0,35,150,110,90,34,98,105,102,68,4,255,136,165,5,151,193,73,196,226,
177,184,17,141,25,145,136,30,177,140,104,220,138,39,99,217,124,58,155,79,103,114,233,84,38,153,72,197,147,153,100,50,157,200,100,211,185,66,166,208,149,43,150,10,112,96,255,62,167,81,253,95,63,254,33,
3,16,12,116,67,95,180,168,119,205,154,53,133,66,241,168,163,142,222,187,175,191,222,112,207,59,239,124,0,136,8,140,68,244,101,71,172,88,251,177,227,227,201,196,25,103,254,221,204,204,140,244,228,59,59,
118,30,177,98,21,0,152,81,35,26,183,142,62,122,245,186,117,235,12,195,188,242,27,87,122,174,59,62,54,54,62,54,82,158,158,28,216,183,123,116,104,127,163,94,177,43,21,187,108,219,21,219,174,84,42,101,91,
41,122,242,201,39,99,177,168,105,154,17,75,15,83,216,166,51,68,164,22,139,155,17,75,179,44,61,158,140,38,211,209,72,76,51,34,220,140,106,177,132,149,206,36,51,185,116,38,155,73,101,210,201,116,42,149,
73,103,114,185,92,33,143,94,163,246,211,159,253,236,170,171,175,225,156,49,46,114,249,66,50,153,152,154,156,230,76,60,249,196,19,199,28,123,204,198,141,27,255,227,193,7,12,157,35,96,87,119,111,50,157,
62,120,240,64,119,119,247,31,255,248,135,82,177,107,120,112,228,252,11,190,252,234,107,175,26,166,0,84,139,22,47,201,165,179,239,189,247,222,201,39,159,252,216,163,91,164,146,82,122,154,64,187,106,75,
95,22,138,69,79,17,2,144,10,122,221,138,105,102,173,102,159,125,246,89,111,188,254,170,110,104,138,40,192,69,225,237,33,97,60,222,220,254,79,20,206,203,90,240,205,71,228,97,77,110,111,109,224,221,133,
210,149,87,93,13,200,185,16,185,92,33,158,76,85,171,182,227,56,191,249,205,175,78,254,196,186,107,174,184,234,158,251,238,209,4,0,227,185,98,111,50,149,154,154,152,226,130,111,217,242,219,229,43,150,205,
204,204,92,118,233,101,207,62,247,124,58,157,17,26,43,20,242,185,108,126,240,224,96,42,149,122,228,145,71,162,209,152,231,122,140,1,73,175,86,183,187,10,93,74,33,33,11,54,113,6,81,220,136,88,63,250,209,
143,126,245,171,95,69,44,35,168,190,49,198,66,177,30,231,116,138,56,23,74,201,240,142,238,80,6,3,156,11,0,80,82,41,37,131,191,16,12,17,64,48,193,80,8,205,40,149,122,142,58,106,245,138,21,71,166,211,217,
45,91,126,71,68,223,189,241,187,0,160,107,76,211,121,207,162,69,171,142,61,126,249,145,71,166,210,233,39,159,122,138,136,102,102,166,191,126,233,165,0,16,137,152,209,104,108,201,210,229,39,157,252,137,
213,71,175,201,102,115,207,61,251,172,235,54,198,70,135,166,198,71,103,38,70,251,247,188,95,43,79,186,13,187,110,87,107,53,187,86,173,214,170,213,90,165,42,93,239,149,87,94,43,20,242,145,136,17,141,154,
17,75,152,17,97,152,220,48,121,232,132,7,74,107,89,70,60,110,37,18,49,43,106,90,150,126,24,77,142,70,141,100,58,158,47,166,99,73,211,136,240,72,76,255,255,3,0,243,192,112,123,52,109,249,238,0,0,0,0,73,
69,78,68,174,66,96,130,0,0};

const char* Forte::truePianos_png = (const char*) resource_Forte_truePianos_png;
const int Forte::truePianos_pngSize = 9333;


//[EndFile] You can add extra defines here...
//[/EndFile]
