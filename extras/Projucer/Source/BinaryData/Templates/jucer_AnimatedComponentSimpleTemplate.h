/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#pragma once

INCLUDE_JUCE

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class CONTENTCOMPCLASS   : public AnimatedAppComponent
{
public:
    //==============================================================================
    CONTENTCOMPCLASS()
    {
        // Make sure you set the size of the component after
        // you add any child components.
        setSize (800, 600);
        setFramesPerSecond (60); // This sets the frequency of the update calls.
    }

    ~CONTENTCOMPCLASS()
    {
    }

    //==============================================================================
    void update() override
    {
        // This function is called at the frequency specified by the setFramesPerSecond() call
        // in the constructor. You can use it to update counters, animate values, etc.
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

        // You can add your drawing code here!
    }

    void resized() override
    {
        // This is called when the MainContentComponent is resized.
        // If you add any child components, this is where you should
        // update their positions.
    }


private:
    //==============================================================================
    // Your private member variables go here...


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CONTENTCOMPCLASS)
};
