/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#ifndef JUCE_TONEGENERATORAUDIOSOURCE_H_INCLUDED
#define JUCE_TONEGENERATORAUDIOSOURCE_H_INCLUDED


//==============================================================================
/**
    A simple AudioSource that generates a sine wave.

*/
class JUCE_API  ToneGeneratorAudioSource  : public AudioSource
{
public:
    //==============================================================================
    /** Creates a ToneGeneratorAudioSource. */
    ToneGeneratorAudioSource();

    /** Destructor. */
    ~ToneGeneratorAudioSource();

    //==============================================================================
    /** Sets the signal's amplitude. */
    void setAmplitude (float newAmplitude);

    /** Sets the signal's frequency. */
    void setFrequency (double newFrequencyHz);


    //==============================================================================
    /** Implementation of the AudioSource method. */
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;

    /** Implementation of the AudioSource method. */
    void releaseResources() override;

    /** Implementation of the AudioSource method. */
    void getNextAudioBlock (const AudioSourceChannelInfo&) override;


private:
    //==============================================================================
    double frequency, sampleRate;
    double currentPhase, phasePerSample;
    float amplitude;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ToneGeneratorAudioSource)
};


#endif   // JUCE_TONEGENERATORAUDIOSOURCE_H_INCLUDED
