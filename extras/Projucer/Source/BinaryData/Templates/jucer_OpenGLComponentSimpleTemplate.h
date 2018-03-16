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
class %%content_component_class%%   : public OpenGLAppComponent
{
public:
    //==============================================================================
    %%content_component_class%%()
    {
        // Make sure you set the size of the component after
        // you add any child components.
        setSize (800, 600);
    }

    ~%%content_component_class%%()
    {
        // This shuts down the GL system and stops the rendering calls.
        shutdownOpenGL();
    }

    //==============================================================================
    void initialise() override
    {
        // Initialise GL objects for rendering here.
    }

    void shutdown() override
    {
        // Free any GL objects created for rendering here.
    }

    void render() override
    {
        // This clears the context with a black background.
        OpenGLHelpers::clear (Colours::black);

        // Add your rendering code here...
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        // You can add your component specific drawing code here!
        // This will draw over the top of the openGL background.
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


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (%%content_component_class%%)
};
