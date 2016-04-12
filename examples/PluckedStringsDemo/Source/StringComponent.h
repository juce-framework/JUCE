/*
  ==============================================================================

   JUCE demo code - use at your own risk!

  ==============================================================================
*/


/*
    This component represents a horizontal vibrating musical string of fixed height
    and variable length. The string can be excited by calling stringPlucked().
*/
class StringComponent   : public Component,
                          private Timer
{
public:
    StringComponent (int lengthInPixels, Colour stringColour)
        : length (lengthInPixels), colour (stringColour)
    {
        // ignore mouse-clicks so that our parent can get them instead.
        setInterceptsMouseClicks (false, false);
        setSize (length, height);
        startTimerHz (60);
    }

    //==============================================================================
    void stringPlucked (float pluckPositionRelative)
    {
        amplitude = maxAmplitude * std::sin (pluckPositionRelative * float_Pi);
        phase = float_Pi;
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.setColour (colour);
        g.strokePath (generateStringPath(), PathStrokeType (2.0f));
    }

    Path generateStringPath() const
    {
        const float y = height / 2.0f;

        Path stringPath;
        stringPath.startNewSubPath (0, y);
        stringPath.quadraticTo (length / 2.0f, y + (std::sin (phase) * amplitude), (float) length, y);
        return stringPath;
    }

    //==============================================================================
    void timerCallback() override
    {
        updateAmplitude();
        updatePhase();
        repaint();
    }

    void updateAmplitude()
    {
        // this determines the decay of the visible string vibration.
        amplitude *= 0.99f;
    }

    void updatePhase()
    {
        // this determines the visible vibration frequency.
        // just an arbitrary number chosen to look OK:
        const float phaseStep = 400.0f / length;

        phase += phaseStep;

        if (phase > float_Pi)
            phase -= 2.0f * float_Pi;
    }

private:
    //==============================================================================
    int length;
    Colour colour;

    int height = 20;
    float amplitude = 0.0f;
    const float maxAmplitude = 12.0f;
    float phase = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StringComponent)
};
