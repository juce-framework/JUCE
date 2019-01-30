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

#pragma once

//[Headers]     -- You can add your own extra header files here --
#include "../../JuceLibraryCode/JuceHeader.h"
#include "../Filters/InternalFilters.h"
class Device;
class Zone;
class FilterGraph;
class GraphEditorPanel;
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Projucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class RackRow  : public Component,
                 public TextEditor::Listener,
                 public MidiFilterCallback,
                 public Button::Listener,
                 public Slider::Listener,
                 public ComboBox::Listener
{
public:
    //==============================================================================
    RackRow ();
    ~RackRow();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void UpdateKeyboard();
    void Setup(Device &device, FilterGraph &filterGraph, GraphEditorPanel &panel);
    void Assign(Zone *zone);
    int ID() { return m_id; }
    void textEditorTextChanged(TextEditor&) override;
    void SetSoloMode(bool mode);
    bool IsSolo() { return m_solo->getToggleState(); }
    void Filter(int samples, int sampleRate, MidiBuffer &midiBuffer) override;
    static void SetTempo(int tempo) { m_tempo = tempo; }
    //[/UserMethods]

    void paint (Graphics& g) override;
    void resized() override;
    void buttonClicked (Button* buttonThatWasClicked) override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override;
    void mouseDown (const MouseEvent& e) override;
    void mouseDrag (const MouseEvent& e) override;
    void mouseUp (const MouseEvent& e) override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    MidiKeyboardState *m_keyboardState;
    Zone *m_current;
    int m_id;
    FilterGraph* graph;
    GraphEditorPanel* panel;
    bool m_soloMode;
    bool m_pendingProgram;
    float m_pendingProgramNames;

    vector<int> m_notesDown;
    bool m_anyNotesDown;
    int m_arpeggiatorBeat;
    int m_lastNote;
    float m_arpeggiatorTimer;
    static int m_tempo;
    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<GroupComponent> m_deviceName;
    std::unique_ptr<ToggleButton> m_solo;
    std::unique_ptr<ToggleButton> m_mute;
    std::unique_ptr<Slider> m_volume;
    std::unique_ptr<ComboBox> m_bank;
    std::unique_ptr<ComboBox> m_program;
    std::unique_ptr<TextEditor> m_transpose;
    std::unique_ptr<Label> m_to;
    std::unique_ptr<TextEditor> m_lowKey;
    std::unique_ptr<TextEditor> m_highKey;
    std::unique_ptr<ImageButton> m_deviceSettings;
    std::unique_ptr<MidiKeyboardComponent> m_keyboard;
    std::unique_ptr<ToggleButton> m_doubleOctave;
    std::unique_ptr<ToggleButton> m_arpeggiator;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RackRow)
};

//[EndFile] You can add extra defines here...
//[/EndFile]
