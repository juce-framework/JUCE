/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

INCLUDE_CORRESPONDING_HEADER


//==============================================================================
CONTENTCOMPCLASS::CONTENTCOMPCLASS()
{
    setSize (600, 400);
}

CONTENTCOMPCLASS::~CONTENTCOMPCLASS()
{
}

void CONTENTCOMPCLASS::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    g.setFont (Font (16.0f));
    g.setColour (Colours::white);
    g.drawText ("Hello World!", getLocalBounds(), Justification::centred, true);
}

void CONTENTCOMPCLASS::resized()
{
    // This is called when the CONTENTCOMPCLASS is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
