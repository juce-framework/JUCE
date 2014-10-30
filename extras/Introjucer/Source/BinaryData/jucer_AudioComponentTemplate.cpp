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
class MainContentComponent   : public AudioAppComponent
{
public:
    //==============================================================================


    MainContentComponent()
    {
        setSize (500, 400);
        
        // the the input and output channels (currently Mono in and out)
        setAudioChannels (1, 1);
    }

    ~MainContentComponent()
    {
        shutdownAudio();
    }
    
    //=======================================================================
    // HANDLE AUDIO
    
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate)
    {
        // This is called externally with the native paramters when the
        // device is ready.
    }
    
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
    {
        // Process your Audio here.
    }
    
    void releaseResources()
    {
        // This gets automatically called when audio device paramters change
        // or device is restarted.
    }
    
    //=======================================================================
    // HANDLE DRAWING
    
    void paint (Graphics& g)
    {
        // fill background
        g.fillAll (Colours::black);

    }

    void resized()
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


Component* createMainContentComponent() { return new MainContentComponent(); };

#endif  // MAINCOMPONENT_H_INCLUDED