//==============================================================================
class %%component_class%%  : public juce::Component
{
public:
    %%component_class%%()
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.

    }

    ~%%component_class%%() override
    {
    }

    void paint (juce::Graphics& g) override
    {
        // You should replace everything in this method with your own drawing code..

        g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));   // clear the background

        g.setColour (juce::Colours::grey);
        g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

        g.setColour (juce::Colours::white);
        g.setFont (14.0f);
        g.drawText ("%%component_class%%", getLocalBounds(),
                    juce::Justification::centred, true);   // draw some placeholder text
    }

    void resized() override
    {
        // This method is where you should set the bounds of any child
        // components that your component contains..

    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (%%component_class%%)
};
