/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    An AudioSource that uses the Reverb class to apply a reverb to another AudioSource.

    @see Reverb

    @tags{Audio}
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
    ~ReverbAudioSource() override;

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
    std::atomic<bool> bypass;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbAudioSource)
};

} // namespace juce
