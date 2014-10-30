/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainContentComponent   : public AudioAppComponent
{
public:
    //==============================================================================


    MainContentComponent()   : phase (0.0f),
                            delta (0.0f),
                            frequency (5000.0f),
                            amplitude (0.2f),
                            sampleRate (0.0)
    
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
    
    void prepareToPlay (int samplesPerBlockExpected, double newSampleRate) override
    {
        sampleRate = newSampleRate;
    }
    
    
    /* This is where the audio is created. In this example we 
     fill the audio buffer with a sine wave whose frequency is
     controlled by the mouse Y position and whose volume is 
     controlled by the mouse X potition.
     */
    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
    {
        bufferToFill.clearActiveBufferRegion();
        
        // iterate over each sample of the sample buffer
        for (int i = bufferToFill.startSample; i < bufferToFill.numSamples + bufferToFill.startSample; ++i)
        {
            bufferToFill.buffer->getWritePointer (0)[i] = amplitude * sinf (phase);
            
            // increment the phase step for the next sample
            phase += delta;
            
            // reset the phase when it reaches 2PI to avoid large numbers
            while (phase >= 2.0f * float_Pi) phase -= 2.0f * float_Pi;
        }
    }
    
    void releaseResources() override
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
        
        // Set the drawing colour to white
        g.setColour (Colours::white);
        
        // Draw an ellipse based on the mouse position and audio volume
        int radius = amplitude * 200;
        g.fillEllipse  (mouse.x - radius/2, mouse.y - radius/2, radius, radius);
        
        // draw a representative sinewave
        Path wave;
        for (int i = 0; i < getWidth(); i++)
        {
            if (i == 0) wave.startNewSubPath (0, getHeight()/2);
            else wave.lineTo (i, getHeight()/2 + amplitude * getHeight() * 2.0f * sin (i*frequency*0.0001f));
        }
        g.strokePath (wave, PathStrokeType (2));

    }
    
    // Mouse handling
    void mouseUp(const MouseEvent& e) override
    {
        amplitude = 0.0f;
    }
    
    void mouseDown (const MouseEvent& e) override
    {
        mouseDrag (e);
    }

    void mouseDrag (const MouseEvent& e) override
    {
        // Update the mouse position variable
        mouse.setXY (e.x, e.y);
        repaint();
        
        frequency = (getHeight() - e.y) * 10.0f;
        amplitude = e.x/float(getWidth()) * 0.2f;
        
        delta = 2.0f * float_Pi * frequency / sampleRate;
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
    
    float phase;
    float delta;
    float frequency;
    float amplitude;
    
    double sampleRate;
                        
    Point<int> mouse;
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


Component* createMainContentComponent() { return new MainContentComponent(); };

#endif  // MAINCOMPONENT_H_INCLUDED