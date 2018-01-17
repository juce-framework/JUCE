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
}

CONTENTCOMPCLASS::~CONTENTCOMPCLASS()
{
    // This shuts down the GL system and stops the rendering calls.
    shutdownOpenGL();
}

//==============================================================================
void CONTENTCOMPCLASS::initialise()
{
    // Initialise GL objects for rendering here.
}

void CONTENTCOMPCLASS::shutdown()
{
    // Free any GL objects created for rendering here.
}

void CONTENTCOMPCLASS::render()
{
    // This clears the context with a black background.
    OpenGLHelpers::clear (Colours::black);

    // Add your rendering code here...
}

//==============================================================================
void CONTENTCOMPCLASS::paint (Graphics& g)
{
    // You can add your component specific drawing code here!
    // This will draw over the top of the openGL background.
}

void CONTENTCOMPCLASS::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
