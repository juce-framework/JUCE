/*
  ==============================================================================

    %%filename%%
    Created: %%date%%
    Author:  %%author%%

  ==============================================================================
*/

%%include_juce%%
%%include_corresponding_header%%

//==============================================================================
%%component_class%%::%%component_class%%()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

}

%%component_class%%::~%%component_class%%()
{
}

void %%component_class%%::paint (juce::Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

    g.setColour (juce::Colours::grey);
    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

    g.setColour (juce::Colours::white);
    g.setFont (14.0f);
    g.drawText ("%%component_class%%", getLocalBounds(),
                juce::Justification::centred, true);   // draw some placeholder text
}

void %%component_class%%::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}
