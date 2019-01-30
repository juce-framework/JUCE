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
#include "GraphEditorPanel.h"
#include "../Filters/FilterGraph.h"
#include "../Filters/InternalFilters.h"
String FormatKey(int note);
int ParseNote(const char *str);
int RackRow::m_tempo = 120;
//[/MiscUserDefs]

//==============================================================================
RackRow::RackRow ()
{
    //[Constructor_pre] You can add your own custom stuff here..
    m_keyboardState = new MidiKeyboardState();
    m_soloMode = false;
    m_current = NULL;
    m_lastNote = -1;
    m_arpeggiatorBeat = 0;
    m_notesDown.reserve(128);
    m_pendingProgram = false;
    m_pendingProgramNames = 0.f;
    m_arpeggiatorTimer = 0.f;
    //[/Constructor_pre]

    m_deviceName.reset (new GroupComponent (String(),
                                            String()));
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

    m_volume->setBounds (96, 43, 128, 24);

    m_bank.reset (new ComboBox (String()));
    addAndMakeVisible (m_bank.get());
    m_bank->setEditableText (false);
    m_bank->setJustificationType (Justification::centredLeft);
    m_bank->setTextWhenNothingSelected (String());
    m_bank->setTextWhenNoChoicesAvailable (String());
    m_bank->addSeparator();
    m_bank->addSeparator();
    m_bank->addListener (this);

    m_bank->setBounds (233, 43, 150, 24);

    m_program.reset (new ComboBox (String()));
    addAndMakeVisible (m_program.get());
    m_program->setEditableText (false);
    m_program->setJustificationType (Justification::centredLeft);
    m_program->setTextWhenNothingSelected (String());
    m_program->setTextWhenNoChoicesAvailable (String());
    m_program->addListener (this);

    m_program->setBounds (233, 16, 150, 24);

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
    m_lowKey->setText (String());

    m_lowKey->setBounds (712, 14, 32, 24);

    m_highKey.reset (new TextEditor (String()));
    addAndMakeVisible (m_highKey.get());
    m_highKey->setMultiLine (false);
    m_highKey->setReturnKeyStartsNewLine (false);
    m_highKey->setReadOnly (false);
    m_highKey->setScrollbarsShown (false);
    m_highKey->setCaretVisible (true);
    m_highKey->setPopupMenuEnabled (true);
    m_highKey->setText (String());

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

    m_keyboard->setBounds (392, 43, 416, 24);

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
    m_lowKey->addListener(this);
    m_highKey->addListener(this);
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
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == m_solo.get())
    {
        //[UserButtonCode_m_solo] -- add your button handler code here..
        m_current->Solo = buttonThatWasClicked->getToggleState();
        panel->SoloChange();
        //[/UserButtonCode_m_solo]
    }
    else if (buttonThatWasClicked == m_mute.get())
    {
        //[UserButtonCode_m_mute] -- add your button handler code here..
        m_current->Mute = buttonThatWasClicked->getToggleState();
        repaint(); // to change background of row
        ((AudioProcessorGraph::Node*)m_current->Device->m_node)->setBypassed(m_current->Mute || (m_soloMode && !m_current->Solo));
        m_program->setEnabled(!m_current->Mute);
        m_bank->setEnabled(!m_current->Mute);
        //[/UserButtonCode_m_mute]
    }
    else if (buttonThatWasClicked == m_deviceSettings.get())
    {
        //[UserButtonCode_m_deviceSettings] -- add your button handler code here..

        if (auto w = graph->getOrCreateWindowFor((AudioProcessorGraph::Node*)m_current->Device->m_node, PluginWindow::Type::normal))
            w->toFront(true);

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
        m_current->Arpeggiator = buttonThatWasClicked->getToggleState();
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
        InternalPluginFormat::SetGain((AudioProcessorGraph::Node *)m_current->Device->m_gainNode, m_current->Volume);
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
        m_current->Program = 0;

        m_pendingProgram = true;

        m_program->clear();

        auto patchFile = File::getCurrentWorkingDirectory().getFullPathName() + "\\" + m_current->Device->PluginName + "_Bank" + String::formatted("%02d_Patches.txt", m_current->Bank);
        if (File(patchFile).exists())
        {
            StringArray lines;
            File(patchFile).readLines(lines);
            if (lines[lines.size() - 1] == "")
                lines.remove(lines.size() - 1);
            for (int i = 0; i < lines.size(); ++i)
                m_program->addItem(lines[i], i + 1);
            m_program->setSelectedId(m_current->Program + 1, false);
        }
        else
        {
            m_pendingProgramNames = 0.1f;
        }
        //[/UserComboBoxCode_m_bank]
    }
    else if (comboBoxThatHasChanged == m_program.get())
    {
        //[UserComboBoxCode_m_program] -- add your combo box handling code here..
        if (m_program->getSelectedId() > 0)
        {
            m_current->Program = m_program->getSelectedId() - 1;
            m_pendingProgram = true;
        }
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
            {
                m_highKey->setText(FormatKey(key));
                m_current->HighKey = key;
            }
            else
            {
                m_lowKey->setText(FormatKey(key));
                m_current->LowKey = key;
            }
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
        UpdateKeyboard();
    }
    else if (&te == m_highKey.get())
    {
        m_current->HighKey = ParseNote(te.getText().getCharPointer());
        UpdateKeyboard();
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

void RackRow::Filter(int samples, int sampleRate, MidiBuffer &midiBuffer)
{
    int arpeggiatorSample = 0;

    if (!midiBuffer.isEmpty())
    {
        MidiMessage midi_message(0xf0);
        MidiBuffer output;
        int sample_number;

        MidiBuffer::Iterator midi_buffer_iter(midiBuffer);
        while (midi_buffer_iter.getNextEvent(midi_message, sample_number))
        {
            midi_message.setChannel(m_current->Device->Channel + 1);
            if (midi_message.isNoteOnOrOff() && midi_message.getNoteNumber() >= m_current->LowKey && midi_message.getNoteNumber() <= m_current->HighKey)
            {
                int note = midi_message.getNoteNumber() + m_current->Transpose;
                if (note >= 0 && note <= 127)
                {
                    if (m_current->Arpeggiator && m_pendingProgramNames <= 0)
                    {
                        // See if new sequence
                        if (m_notesDown.empty() && midi_message.isNoteOn())
                        {
                            m_arpeggiatorBeat = 0;
                            m_arpeggiatorTimer = 0.001f; // 1ms for first one 
                            arpeggiatorSample = sample_number;
                        }

                        // Update m_notesDown with new info
                        int found = -1;
                        for (auto i = 0U; i < m_notesDown.size(); ++i)
                            if (m_notesDown[i] == note)
                                found = i;
                        if (found == -1 && midi_message.isNoteOn())
                        {
                            for (auto i = 0U; i < m_notesDown.size(); ++i)
                            {
                                if (m_notesDown[i] > note)
                                {
                                    found = i;
                                    break;
                                }
                            }
                            if (found == -1)
                                m_notesDown.push_back(note);
                            else
                                m_notesDown.insert(m_notesDown.begin() + found, note);
                            m_arpeggiatorBeat = 0; // Any change in notes resets arpeggiator position
                        }
                        if (found != -1 && midi_message.isNoteOff())
                        {
                            swap(m_notesDown[found], m_notesDown[m_notesDown.size() - 1]);
                            m_notesDown.pop_back();
                            m_arpeggiatorBeat = 0; // Any change in notes resets arpeggiator position
                        }

                        // are we ending?
                        if (midi_message.isNoteOff() && m_notesDown.empty())
                        {
                            // cancel last note
                            if (m_lastNote >= 0)
                            {
                                output.addEvent(MidiMessage::noteOff(m_current->Device->Channel, m_lastNote), sample_number);
                                m_lastNote = -1;
                            }
                            m_arpeggiatorBeat = 0.f;
                        }
                    }
                    else
                    {
                        midi_message.setNoteNumber(note); // transposed note
                        output.addEvent(midi_message, sample_number);
                        if (m_current->DoubleOctave && note < 128 - 12) // double octave
                        {
                            midi_message.setNoteNumber(note+12);
                            output.addEvent(midi_message, sample_number);
                        }
                    }
                }
            }
        }
        midiBuffer = output;
    }

    if (m_pendingProgram)
    {
        m_pendingProgram = false;

        if (m_current->Device->PluginName == "M1" && m_current->Device->Channel == 1) // if Channel 2's then should come later
        {
            midiBuffer.addEvent(MidiMessage(0xBF, 0x00, 0), 0);
            midiBuffer.addEvent(MidiMessage(0xBF, 0x20, 22), 0);
            midiBuffer.addEvent(MidiMessage(0xCF, 49), 0); // Use MIDI channel 16 to put into two part mode (have to use this Combi mode since no way to Sysex it to Program mode with KLC)
        }

        if (m_bank->isVisible())
        {
            midiBuffer.addEvent(MidiMessage(0xB0 + m_current->Device->Channel - 1, 0x00, 0), 0);
            midiBuffer.addEvent(MidiMessage(0xB0 + m_current->Device->Channel - 1, 0x20, m_current->Bank), 0);
        }

        midiBuffer.addEvent(MidiMessage(0xC0 + m_current->Device->Channel - 1, m_current->Program), 0); // I think this is needed to trigger the bank change too
    }

    if (m_pendingProgramNames > 0)
    {
        m_pendingProgramNames -= samples / (float)sampleRate;
        if (m_pendingProgramNames <= 0)
        {
            m_pendingProgramNames = 0;
            auto processor = ((AudioProcessorGraph::Node*)m_current->Device->m_node)->getProcessor();
            MessageManagerLock lock;
            for (int i = 0; i < processor->getNumPrograms(); ++i)
                m_program->addItem(processor->getProgramName(i), i + 1);
            m_program->setSelectedId(m_current->Program + 1, false);
        }
    }
    

    if (m_arpeggiatorTimer > 0) // arpeggiator active
    {
        while (1)
        {
            auto nextarpeggiatorSample = int(arpeggiatorSample + m_arpeggiatorTimer * sampleRate);
            if (nextarpeggiatorSample < samples) // beat in this block
            {
                arpeggiatorSample = nextarpeggiatorSample; // in case we can fit more in

                m_arpeggiatorTimer = 15.f / m_tempo; // reset timer

                // cancel last note
                if (m_lastNote >= 0)
                {
                    midiBuffer.addEvent(MidiMessage::noteOff(m_current->Device->Channel, m_lastNote), nextarpeggiatorSample);
                    m_lastNote = -1;
                }

                // need new note
                if (!m_notesDown.empty())
                {
                    m_lastNote = m_notesDown[m_arpeggiatorBeat % m_notesDown.size()] + 12 * ((m_arpeggiatorBeat / m_notesDown.size()) % 3);
                    if (m_lastNote < 128)
                        midiBuffer.addEvent(MidiMessage::noteOn(m_current->Device->Channel, m_lastNote, 1.0f), nextarpeggiatorSample);
                    m_arpeggiatorBeat++;
                }
            }
            else
                break;
        }

        // reduce timer
        m_arpeggiatorTimer -= (samples - arpeggiatorSample) / (float)sampleRate;
        assert(m_arpeggiatorTimer > 0); // shouldnt be possible to get to 0 here
    }
}

void RackRow::Setup(Device &device, FilterGraph &filterGraph, GraphEditorPanel &GraphEditorPanel)
{
    graph = &filterGraph;
    panel = &GraphEditorPanel;

    m_deviceName->setText(device.Name);
    auto image = ImageFileFormat::loadFrom(File::getCurrentWorkingDirectory().getFullPathName() + "\\" + String(device.Name + ".png"));
    m_deviceSettings->setImages(false, false, false, image, 1.0f, Colours::transparentBlack, image, 1.0f, Colours::transparentBlack, image, 1.0f, Colours::transparentBlack);

    InternalPluginFormat::SetFilterCallback((AudioProcessorGraph::Node*)device.m_midiFilterNode, this);

    m_id = device.ID;

    auto bankFile = File::getCurrentWorkingDirectory().getFullPathName() + "\\" + String(device.PluginName + "_Banks.txt");
    if (File(bankFile).exists())
    {
        StringArray lines;
        File(bankFile).readLines(lines);
        for (int i = 0; i < lines.size(); ++i)
            m_bank->addItem(lines[i], i + 1);
    }
    else
        m_bank->setVisible(false);

    auto programFile = File::getCurrentWorkingDirectory().getFullPathName() + "\\" + String(device.Name + ".txt");
    if (File(programFile).exists())
    {
        StringArray lines;
        File(programFile).readLines(lines);
        for (int i = 0; i<lines.size(); ++i)
            m_program->addItem(lines[i], i + 1);
    }
    else if (!m_bank->isVisible())
    {
        auto processor = ((AudioProcessorGraph::Node*)device.m_node)->getProcessor();

        for (int i = 0; i < processor->getNumPrograms(); ++i)
            m_program->addItem(processor->getProgramName(i), i + 1);
    }
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

    if (m_bank->isVisible())
        m_bank->setSelectedId(zone->Bank + 1);
    else
        m_program->setSelectedId(zone->Program + 1);

    UpdateKeyboard();
}

void RackRow::SetSoloMode(bool mode)
{
    m_soloMode = mode;
    ((AudioProcessorGraph::Node*)m_current->Device->m_node)->setBypassed(m_current->Mute || (m_soloMode && !m_current->Solo)); // Do this here again. Can't rely on Toggle because only works if changed
}

//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="RackRow" componentName=""
                 parentClasses="public Component, public TextEditor::Listener, public MidiFilterCallback"
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
                  explicitFocusOrder="0" pos="0 -2 816 80" title=""/>
  <TOGGLEBUTTON name="" id="2b62ef4a67b701f3" memberName="m_solo" virtualName=""
                explicitFocusOrder="0" pos="96 14 72 24" buttonText="Solo" connectedEdges="0"
                needsCallback="1" radioGroupId="0" state="0"/>
  <TOGGLEBUTTON name="" id="6ad6011b41b23475" memberName="m_mute" virtualName=""
                explicitFocusOrder="0" pos="160 14 72 24" buttonText="Mute" connectedEdges="0"
                needsCallback="1" radioGroupId="0" state="0"/>
  <SLIDER name="" id="a0e2bc5a61933c6d" memberName="m_volume" virtualName=""
          explicitFocusOrder="0" pos="96 43 128 24" min="-110.00000000000000000000"
          max="12.00000000000000000000" int="0.50000000000000000000" style="LinearBar"
          textBoxPos="TextBoxBelow" textBoxEditable="1" textBoxWidth="80"
          textBoxHeight="20" skewFactor="1.00000000000000000000" needsCallback="1"/>
  <COMBOBOX name="" id="90d63ca95a92a112" memberName="m_bank" virtualName=""
            explicitFocusOrder="0" pos="233 43 150 24" editable="0" layout="33"
            items="&#10;" textWhenNonSelected="" textWhenNoItems=""/>
  <COMBOBOX name="" id="9de3cb5469378fa1" memberName="m_program" virtualName=""
            explicitFocusOrder="0" pos="233 16 150 24" editable="0" layout="33"
            items="" textWhenNonSelected="" textWhenNoItems=""/>
  <TEXTEDITOR name="" id="b6e30577b79a003a" memberName="m_transpose" virtualName=""
              explicitFocusOrder="0" pos="648 14 32 24" initialText="" multiline="0"
              retKeyStartsLine="0" readonly="0" scrollbars="0" caret="1" popupmenu="1"/>
  <LABEL name="" id="72d9777463cc6a85" memberName="m_to" virtualName=""
         explicitFocusOrder="0" pos="744 14 24 24" edTextCol="ff000000"
         edBkgCol="0" labelText="to" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15.00000000000000000000"
         kerning="0.00000000000000000000" bold="0" italic="0" justification="33"/>
  <TEXTEDITOR name="" id="3d470180923a3d6f" memberName="m_lowKey" virtualName=""
              explicitFocusOrder="0" pos="712 14 32 24" initialText="" multiline="0"
              retKeyStartsLine="0" readonly="0" scrollbars="0" caret="1" popupmenu="1"/>
  <TEXTEDITOR name="" id="5f3abd7bbb50678c" memberName="m_highKey" virtualName=""
              explicitFocusOrder="0" pos="768 14 32 24" initialText="" multiline="0"
              retKeyStartsLine="0" readonly="0" scrollbars="0" caret="1" popupmenu="1"/>
  <IMAGEBUTTON name="" id="31b2ae44720b5f47" memberName="m_deviceSettings" virtualName=""
               explicitFocusOrder="0" pos="8 14 76 57" buttonText="new button"
               connectedEdges="0" needsCallback="1" radioGroupId="0" keepProportions="1"
               resourceNormal="" opacityNormal="1.00000000000000000000" colourNormal="0"
               resourceOver="" opacityOver="1.00000000000000000000" colourOver="0"
               resourceDown="" opacityDown="1.00000000000000000000" colourDown="0"/>
  <GENERICCOMPONENT name="" id="3a433662794e0409" memberName="m_keyboard" virtualName="MidiKeyboardComponent"
                    explicitFocusOrder="0" pos="392 43 416 24" class="unknown" params="*m_keyboardState, MidiKeyboardComponent::Orientation::horizontalKeyboard"/>
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
