/*
  ==============================================================================

    FILENAME
    Created: DATE
    Author:  AUTHOR

  ==============================================================================
*/

#ifndef HEADERGUARD
#define HEADERGUARD

INCLUDE_JUCE

//==============================================================================
/*
*/
class COMPONENTCLASS    : public Component
{
public:
    COMPONENTCLASS()
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.

    }

    ~COMPONENTCLASS()
    {
    }

    void paint (Graphics& g) override
    {
        /* This demo code just fills the component's background and
           draws some placeholder text to get you started.

           You should replace everything in this method with your own
           drawing code..
        */

        g.fillAll (Colours::white);   // clear the background

        g.setColour (Colours::grey);
        g.drawRect (getLocalBounds(), 1);   // draw an outline around the component

        g.setColour (Colours::lightblue);
        g.setFont (14.0f);
        g.drawText ("COMPONENTCLASS", getLocalBounds(),
                    Justification::centred, true);   // draw some placeholder text
    }

    void resized() override
    {
        // This method is where you should set the bounds of any child
        // components that your component contains..

    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (COMPONENTCLASS)
};


#endif  // HEADERGUARD
