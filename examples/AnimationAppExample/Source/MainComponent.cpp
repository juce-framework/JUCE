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
        setSize (800, 600);
        setFramesPerSecond (60);
    }

    void update() override
    {
        // This function is called at the frequency specified by the setFramesPerSecond() call
        // in the constructor. You can use it to update counters, animate values, etc.
    }

    void paint (Graphics& g) override
    {
        // (Our component is opaque, so we must completely fill the background with a solid colour)
        g.fillAll (Colours::black);

        g.setColour (Colours::white);
        const int fishLength = 15;

        Path spinePath;

        for (int i = 0; i < fishLength; ++i)
        {
            const float radius = 100 + 10 * std::sin (getFrameCounter() * 0.1f + i * 0.5f);

            Point<float> p (getWidth()  / 2.0f + 1.5f * radius * std::sin (getFrameCounter() * 0.02f + i * 0.12f),
                            getHeight() / 2.0f + 1.0f * radius * std::cos (getFrameCounter() * 0.04f + i * 0.12f));

            // draw the circles along the fish
            g.fillEllipse (p.x - i, p.y - i, 2.0f + 2.0f * i, 2.0f + 2.0f * i);

            if (i == 0)
                spinePath.startNewSubPath (p);  // if this is the first point, start a new path..
            else
                spinePath.lineTo (p);           // ...otherwise add the next point
        }

        // draw an outline around the path that we have created
        g.strokePath (spinePath, PathStrokeType (4.0f));
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


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()    { return new MainContentComponent(); }


#endif  // MAINCOMPONENT_H_INCLUDED
