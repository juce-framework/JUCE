/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

INCLUDE_JUCE

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainContentComponent   : public OpenGLAppComponent
{
public:
    //==============================================================================
    MainContentComponent()
    {
        setSize (800, 600);
    }

    ~MainContentComponent()
    {
        // (Your class must call this in its destructor to stop the
        // rendering callbacks happening after your class has been deleted)
        shutdownOpenGL();
    }

    void initialise() override
    {
    }

    void shutdown() override
    {
    }

    void render() override
    {
        OpenGLHelpers::clear (Colours::lightblue);
    }

    void paint (Graphics& g) override
    {
        // You can add normal 2D drawing code here, which will be
        // drawn over the GL content.
    }

    void resized() override
    {
        // This is called when the MainContentComponent is resized.
        // If you add any child components, this is where you should
        // update their positions.
    }


private:
    //==============================================================================

    // private member variables



    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()    { return new MainContentComponent(); }


#endif  // MAINCOMPONENT_H_INCLUDED
