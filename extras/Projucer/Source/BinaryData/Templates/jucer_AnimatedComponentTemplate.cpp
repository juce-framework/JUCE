/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

INCLUDE_CORRESPONDING_HEADER

//==============================================================================
CONTENTCOMPCLASS::CONTENTCOMPCLASS()
{
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (800, 600);
    setFramesPerSecond (60); // This sets the frequency of the update calls.
}

CONTENTCOMPCLASS::~CONTENTCOMPCLASS()
{
}

//==============================================================================
void CONTENTCOMPCLASS::update()
{
    // This function is called at the frequency specified by the setFramesPerSecond() call
    // in the constructor. You can use it to update counters, animate values, etc.
}

//==============================================================================
void CONTENTCOMPCLASS::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void CONTENTCOMPCLASS::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
