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


#ifndef SYNTH_H_INCLUDED
#define SYNTH_H_INCLUDED


//==============================================================================
class DemoSynth: public ExpressiveMidiSynthesiserBase
{
public:
    DemoSynth()
        : noiseLevel (0.0f)
    {
    }

    void noteAdded (ExpressiveMidiNote newNote) override
    {
        noiseLevel += 0.05f;
    }

    void noteChanged (ExpressiveMidiNote changedNote) override
    {
        // nothing!
    }

    void noteReleased (ExpressiveMidiNote finishedNote) override
    {
        noiseLevel -= 0.05f;
    }

protected:
    //==========================================================================
    void renderNextSubBlock (AudioBuffer<float>& outputAudio,
                             int startSample,
                             int numSamples) override
    {
        while (--numSamples >= 0)
        {
            for (int i = outputAudio.getNumChannels(); --i >= 0;)
            {
                float randomFloatWithinRange = (2.0f * float (std::rand()) /  float (std::numeric_limits<int>::max())) - 1.0f;
                const float currentSample = noiseLevel * randomFloatWithinRange;

                outputAudio.addSample (i, startSample, currentSample);
            }

            ++startSample;
        }
    }

private:
    //==========================================================================
    float noiseLevel;
};


#endif  // SYNTH_H_INCLUDED
