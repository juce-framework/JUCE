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
class MainContentComponent   : public AnimatedAppComponent
{
public:
    //==============================================================================


    MainContentComponent()
    {
        setSize (500, 400);
        setFramesPerSecond (60);
    }

    ~MainContentComponent()
    {
    }

    void update()
    {
        
    }
    
    void paint (Graphics& g)
    {
        // fill background
        g.fillAll (Colours::black);
        
        int fishLength = 15;
        
        // set the drawing colour
        g.setColour (Colours::white);
        
        // Create a new path object for the spine
        Path p;
        
        // 
        for (int i = 0; i < fishLength; ++i)
        {
            float radius = 100 + 10 * sin (getFrameCounter() * 0.1 + i * 0.5f);
            float x = getWidth()/2 + 1.5f * radius * sin (getFrameCounter() * 0.02f + i * 0.12f);
            float y = getHeight()/2 + radius * cos (getFrameCounter() * 0.04f + i * 0.12f);
            
            // draw the ellipses of the fish
            g.fillEllipse(x - i, y - i, 2 + 2*i, 2 + 2*i);
            
            // start a new path at the beginning otherwise add the next point
            if (i == 0)
                p.startNewSubPath(x, y);
            else
                p.lineTo (x, y);
        }
        
        // stroke the path that we have created
        g.strokePath (p, PathStrokeType (4));

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
