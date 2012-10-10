/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

INCLUDE_CORRESPONDING_HEADER


//==============================================================================
CONTENTCOMPCLASS::CONTENTCOMPCLASS()
{
    setSize (500, 400);
}

CONTENTCOMPCLASS::~CONTENTCOMPCLASS()
{
}

void CONTENTCOMPCLASS::paint (Graphics& g)
{
    g.fillAll (Colour (0xffeeddff));

    g.setFont (Font (16.0f));
    g.setColour (Colours::black);
    g.drawText ("Hello World!", getLocalBounds(), Justification::centred, true);
}

void CONTENTCOMPCLASS::resized()
{
    // This is called when the CONTENTCOMPCLASS is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
