/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

%%include_corresponding_header%%

//==============================================================================
%%content_component_class%%::%%content_component_class%%()
{
    setSize (600, 400);
}

%%content_component_class%%::~%%content_component_class%%()
{
}

//==============================================================================
void %%content_component_class%%::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    g.setFont (Font (16.0f));
    g.setColour (Colours::white);
    g.drawText ("Hello World!", getLocalBounds(), Justification::centred, true);
}

void %%content_component_class%%::resized()
{
    // This is called when the %%content_component_class%% is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
