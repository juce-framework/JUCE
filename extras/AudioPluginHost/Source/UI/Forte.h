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
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Projucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class Forte  : public Component,
               public Button::Listener,
               public Slider::Listener,
               public ComboBox::Listener
{
public:
    //==============================================================================
    Forte ();
    ~Forte();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (Graphics& g) override;
    void resized() override;
    void buttonClicked (Button* buttonThatWasClicked) override;
    void sliderValueChanged (Slider* sliderThatWasMoved) override;
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override;

    // Binary resources:
    static const char* keys_png;
    static const int keys_pngSize;
    static const char* truePianos_png;
    static const int truePianos_pngSize;
    static const char* piano_png;
    static const int piano_pngSize;


private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<GroupComponent> groupComponent;
    std::unique_ptr<Label> label4;
    std::unique_ptr<ToggleButton> toggleButton;
    std::unique_ptr<ToggleButton> toggleButton2;
    std::unique_ptr<Slider> slider;
    std::unique_ptr<ComboBox> comboBox;
    std::unique_ptr<ComboBox> comboBox2;
    std::unique_ptr<Label> label2;
    std::unique_ptr<TextEditor> textEditor;
    std::unique_ptr<Label> label5;
    std::unique_ptr<Label> label8;
    std::unique_ptr<TextEditor> textEditor2;
    std::unique_ptr<TextEditor> textEditor3;
    std::unique_ptr<ImageButton> imageButton;
    std::unique_ptr<Label> label;
    std::unique_ptr<Label> label7;
    std::unique_ptr<Label> label9;
    std::unique_ptr<TextEditor> textEditor7;
    std::unique_ptr<Label> label3;
    std::unique_ptr<ComboBox> comboBox3;
    std::unique_ptr<Label> label6;
    std::unique_ptr<Label> label10;
    std::unique_ptr<unknown> component;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Forte)
};

//[EndFile] You can add extra defines here...
//[/EndFile]
