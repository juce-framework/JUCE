/*
  ==============================================================================

    ToggleLightComponent.h
    Created: 11 Feb 2015 9:56:51am
    Author:  Felix Faire

  ==============================================================================
*/

#ifndef TOGGLELIGHTCOMPONENT_H_INCLUDED
#define TOGGLELIGHTCOMPONENT_H_INCLUDED

/**
    This class represents one of the individual lights in our grid.
*/
class ToggleLightComponent  : public Component
{
public:
    ToggleLightComponent (String name = "light")
        : Component (name),
          isOn (false)
    {
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);

        // Only shows the red ellipse when the button is on.
        if (isOn)
        {
            g.setColour (Colours::red);
            g.fillEllipse (getLocalBounds().toFloat());
        }
    }

    void mouseEnter (const MouseEvent&) override
    {
        // button toggles state on mouse over.
        isOn = ! isOn;
        repaint();
    }

private:
    // member variables for the Component
    bool isOn;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToggleLightComponent)
};


#endif  // TOGGLELIGHTCOMPONENT_H_INCLUDED
