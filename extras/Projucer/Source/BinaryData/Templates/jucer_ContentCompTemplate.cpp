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
void %%content_component_class%%::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setFont (juce::FontOptions (16.0f));
    g.setColour (juce::Colours::white);
    g.drawText ("Hello World!", getLocalBounds(), juce::Justification::centred, true);
}

void %%content_component_class%%::resized()
{
    // This is called when the %%content_component_class%% is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
