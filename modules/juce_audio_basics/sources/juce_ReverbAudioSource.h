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

#ifndef JUCE_REVERBAUDIOSOURCE_H_INCLUDED
#define JUCE_REVERBAUDIOSOURCE_H_INCLUDED


//==============================================================================
/**
    An AudioSource that uses the Reverb class to apply a reverb to another AudioSource.

    @see Reverb
*/
class JUCE_API  ReverbAudioSource   : public AudioSource
{
public:
    /** Creates a ReverbAudioSource to process a given input source.

        @param inputSource              the input source to read from - this must not be null
        @param deleteInputWhenDeleted   if true, the input source will be deleted when
                                        this object is deleted
    */
    ReverbAudioSource (AudioSource* inputSource,
                       bool deleteInputWhenDeleted);

    /** Destructor. */
    ~ReverbAudioSource();

    //==============================================================================
    /** Returns the parameters from the reverb. */
    const Reverb::Parameters& getParameters() const noexcept    { return reverb.getParameters(); }

    /** Changes the reverb's parameters. */
    void setParameters (const Reverb::Parameters& newParams);

    void setBypassed (bool isBypassed) noexcept;
    bool isBypassed() const noexcept                            { return bypass; }

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock (const AudioSourceChannelInfo&) override;

private:
    //==============================================================================
    CriticalSection lock;
    OptionalScopedPointer<AudioSource> input;
    Reverb reverb;
    volatile bool bypass;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbAudioSource)
};


#endif   // JUCE_REVERBAUDIOSOURCE_H_INCLUDED
