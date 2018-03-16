/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

%%include_juce%%

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class %%content_component_class%%   : public Component
{
public:
    //==============================================================================
    %%content_component_class%%()
    {
        setSize (600, 400);
    }

    ~%%content_component_class%%()
    {
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

        g.setFont (Font (16.0f));
        g.setColour (Colours::white);
        g.drawText ("Hello World!", getLocalBounds(), Justification::centred, true);
    }

    void resized() override
    {
        // This is called when the %%content_component_class%% is resized.
        // If you add any child components, this is where you should
        // update their positions.
    }


private:
    //==============================================================================
    // Your private member variables go here...


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (%%content_component_class%%)
};
