/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#include "../JuceDemoHeader.h"

//==============================================================================
/** Simple message that holds a Colour. */
struct ColourMessage  : public Message
{
    ColourMessage (Colour col)  : colour (col)
    {
    }

    /** Returns the colour of a ColourMessage of white if the message is not a ColourMessage. */
    static Colour getColour (const Message& message)
    {
        if (const ColourMessage* cm = dynamic_cast<const ColourMessage*> (&message))
            return cm->colour;

        return Colours::white;
    }

private:
    Colour colour;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ColourMessage)
};

//==============================================================================
/** Simple component that can be triggered to flash.
    The flash will then fade using a Timer to repaint itself and will send a change
    message once it is finished.
 */
class FlashingComponent   : public Component,
                            public MessageListener,
                            public ChangeBroadcaster,
                            private Timer
{
public:
    FlashingComponent()
        : flashAlpha (0.0f),
          colour (Colours::red)
    {
    }

    void startFlashing()
    {
        flashAlpha = 1.0f;
        startTimerHz (25);
    }

    /** Stops this component flashing without sending a change message. */
    void stopFlashing()
    {
        flashAlpha = 0.0f;
        stopTimer();
        repaint();
    }

    /** Sets the colour of the component. */
    void setFlashColour (const Colour newColour)
    {
        colour = newColour;
        repaint();
    }

    /** Draws our component. */
    void paint (Graphics& g) override
    {
        g.setColour (colour.overlaidWith (Colours::white.withAlpha (flashAlpha)));
        g.fillEllipse (getLocalBounds().toFloat());
    }

    /** Custom mouse handler to trigger a flash. */
    void mouseDown (const MouseEvent&) override
    {
        startFlashing();
    }

    /** Message listener callback used to change our colour */
    void handleMessage (const Message& message) override
    {
        setFlashColour (ColourMessage::getColour (message));
    }

private:
    float flashAlpha;
    Colour colour;

    void timerCallback() override
    {
        // Reduce the alpha level of the flash slightly so it fades out
        flashAlpha -= 0.075f;

        if (flashAlpha < 0.05f)
        {
            stopFlashing();
            sendChangeMessage();
            // Once we've finsihed flashing send a change message to trigger the next component to flash
        }

        repaint();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FlashingComponent)
};

//==============================================================================
class TimersAndEventsDemo   : public Component,
                              private ChangeListener,
                              private Button::Listener
{
public:
    TimersAndEventsDemo()
    {
        setOpaque (true);

        // Create and add our FlashingComponents with some random colours and sizes
        for (int i = 0; i < numFlashingComponents; ++i)
        {
            FlashingComponent* newFlasher = new FlashingComponent();
            flashingComponents.add (newFlasher);

            newFlasher->setFlashColour (getRandomBrightColour());
            newFlasher->addChangeListener (this);

            const int diameter = 25 + random.nextInt (75);
            newFlasher->setSize (diameter, diameter);

            addAndMakeVisible (newFlasher);
        }

        addAndMakeVisible (stopButton);
        stopButton.addListener (this);
        stopButton.setButtonText ("Stop");

        addAndMakeVisible (randomColourButton);
        randomColourButton.addListener (this);
        randomColourButton.setButtonText ("Set Random Colour");

        // lay out our components in a psudo random grid
        Rectangle<int> area (0, 100, 150, 150);

        for (int i = 0; i < flashingComponents.size(); ++i)
        {
            FlashingComponent* comp = flashingComponents.getUnchecked (i);
            Rectangle<int> buttonArea (area.withSize (comp->getWidth(), comp->getHeight()));
            buttonArea.translate (random.nextInt (area.getWidth() - comp->getWidth()),
                                  random.nextInt (area.getHeight() - comp->getHeight()));
            comp->setBounds (buttonArea);

            area.translate (area.getWidth(), 0);

            // if we go off the right start a new row
            if (area.getRight() > (800 - area.getWidth()))
            {
                area.translate (0, area.getWidth());
                area.setX (0);
            }
        }
    }

    ~TimersAndEventsDemo()
    {
        stopButton.removeListener (this);
        randomColourButton.removeListener (this);

        for (int i = flashingComponents.size(); --i >= 0;)
            flashingComponents.getUnchecked (i)->removeChangeListener (this);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::darkgrey);
    }

    void paintOverChildren (Graphics& g) override
    {
        const Rectangle<int> explanationArea (getLocalBounds().removeFromTop (100));

        AttributedString s;
        s.append ("Click on a circle to make it flash. When it has finished flashing it will send a message which causes the next circle to flash");
        s.append (newLine);
        s.append ("Click the \"Set Random Colour\" button to change the colour of one of the circles.");
        s.append (newLine);
        s.setFont (Font (16.0f));
        s.setColour (Colours::lightgrey);
        s.draw (g, explanationArea.reduced (10).toFloat());
    }

    void resized() override
    {
        Rectangle<int> area (getLocalBounds().removeFromBottom (40));
        randomColourButton.setBounds (area.removeFromLeft (166).reduced (8));
        stopButton.setBounds (area.removeFromRight (166).reduced (8));
    }

private:
    enum { numFlashingComponents = 9 };

    OwnedArray<FlashingComponent> flashingComponents;
    TextButton randomColourButton, stopButton;
    Random random;

    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        for (int i = 0; i < flashingComponents.size(); ++i)
            if (source == flashingComponents.getUnchecked (i))
                flashingComponents.getUnchecked ((i + 1) % flashingComponents.size())->startFlashing();
    }

    void buttonClicked (Button* button) override
    {
        if (button == &randomColourButton)
        {
            // Here we post a new ColourMessage with a random colour to a random flashing component.
            // This will send a message to the component asynchronously and trigger its handleMessage callback
            flashingComponents.getUnchecked (random.nextInt (flashingComponents.size()))->postMessage (new ColourMessage (getRandomBrightColour()));
        }
        else if (button == &stopButton)
        {
            for (int i = 0; i < flashingComponents.size(); ++i)
                flashingComponents.getUnchecked (i)->stopFlashing();
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimersAndEventsDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<TimersAndEventsDemo> demo ("40 Timers & Events");
