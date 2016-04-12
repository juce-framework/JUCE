/*
  ==============================================================================

   JUCE demo code - use at your own risk!

  ==============================================================================
*/

#include "StringSynthesiser.h"
#include "StringComponent.h"


//==============================================================================
class StringDemoComponent   : public AudioAppComponent
{
public:
    StringDemoComponent()
    {
        createStringComponents();
        setSize (800, 560);

        // specify the number of input and output channels that we want to open
        setAudioChannels (0, 2);
    }

    ~StringDemoComponent()
    {
        shutdownAudio();
    }

    //==============================================================================
    void prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate) override
    {
        generateStringSynths (sampleRate);
    }

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        bufferToFill.clearActiveBufferRegion();

        for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
        {
            float* const channelData = bufferToFill.buffer->getWritePointer (channel, bufferToFill.startSample);

            if (channel == 0)
            {
                for (auto synth : stringSynths)
                    synth->generateAndAddData (channelData, bufferToFill.numSamples);
            }
            else
            {
                memcpy (channelData,
                        bufferToFill.buffer->getReadPointer (0),
                        bufferToFill.numSamples * sizeof (float));
            }
        }
    }

    void releaseResources() override
    {
        stringSynths.clear();
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);
    }

    void resized() override
    {
        int xPos = 20;
        int yPos = 20;
        int yDistance = 50;

        for (auto stringLine : stringLines)
        {
            stringLine->setTopLeftPosition (xPos, yPos);
            yPos += yDistance;
            addAndMakeVisible (stringLine);
        }
    }

private:
    void mouseDown (const MouseEvent& e) override
    {
        mouseDrag (e);
    }

    void mouseDrag (const MouseEvent& e) override
    {
        for (int i = 0; i < stringLines.size(); ++i)
        {
            auto* stringLine = stringLines.getUnchecked(i);

            if (stringLine->getBounds().contains (e.getPosition()))
            {
                float position = (e.position.x - stringLine->getX()) / stringLine->getWidth();

                stringLine->stringPlucked (position);
                stringSynths.getUnchecked(i)->stringPlucked (position);
            }
        }
    }

    //==============================================================================
    struct StringParameters
    {
        StringParameters (int midiNote)
            : frequencyInHz (MidiMessage::getMidiNoteInHertz (midiNote)),
              lengthInPixels ((int) (760 / (frequencyInHz / MidiMessage::getMidiNoteInHertz (42))))
        {
        }

        double frequencyInHz;
        int lengthInPixels;
    };

    static std::vector<StringParameters> getDefaultStringParameters()
    {
        return { 42, 44, 46, 49, 51, 54, 56, 58, 61, 63, 66, 68, 70 };
    }

    void createStringComponents()
    {
        for (auto stringParams : getDefaultStringParameters())
        {
            stringLines.add (new StringComponent (stringParams.lengthInPixels,
                                                  Colour::fromHSV (Random().nextFloat(), 0.6f, 0.9f, 1.0f)));
        }
    }

    void generateStringSynths (double sampleRate)
    {
        stringSynths.clear();

        for (auto stringParams : getDefaultStringParameters())
        {
            stringSynths.add (new StringSynthesiser (sampleRate, stringParams.frequencyInHz));
        }
    }

    //==============================================================================
    OwnedArray<StringComponent> stringLines;
    OwnedArray<StringSynthesiser> stringSynths;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StringDemoComponent)
};
