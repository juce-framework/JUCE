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
class CONTENTCOMPCLASS   : public AudioAppComponent
{
public:
    //==============================================================================
    CONTENTCOMPCLASS();
    ~CONTENTCOMPCLASS();

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    // Your private member variables go here...


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CONTENTCOMPCLASS)
};
