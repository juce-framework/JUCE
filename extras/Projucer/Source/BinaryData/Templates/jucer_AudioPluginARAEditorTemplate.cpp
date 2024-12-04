/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

%%editor_cpp_headers%%

//==============================================================================
%%editor_class_name%%::%%editor_class_name%% (%%filter_class_name%%& p)
    : AudioProcessorEditor (&p),
     #if JucePlugin_Enable_ARA
      AudioProcessorEditorARAExtension (&p),
     #endif
      audioProcessor (p)
{
   #if JucePlugin_Enable_ARA
    // ARA plugins must be resizable for proper view embedding
    setResizable (true, false);
   #endif

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
}

%%editor_class_name%%::~%%editor_class_name%%()
{
}

//==============================================================================
void %%editor_class_name%%::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void %%editor_class_name%%::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
