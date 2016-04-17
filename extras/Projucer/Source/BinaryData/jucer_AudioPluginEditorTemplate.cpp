/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

EDITORCPPHEADERS


//==============================================================================
EDITORCLASSNAME::EDITORCLASSNAME (FILTERCLASSNAME& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
}

EDITORCLASSNAME::~EDITORCLASSNAME()
{
}

//==============================================================================
void EDITORCLASSNAME::paint (Graphics& g)
{
    g.fillAll (Colours::white);

    g.setColour (Colours::black);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), Justification::centred, 1);
}

void EDITORCLASSNAME::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
