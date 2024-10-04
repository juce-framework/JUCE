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
    An AudioSource that takes the audio from another source, and re-maps its
    input and output channels to a different arrangement.

    You can use this to increase or decrease the number of channels that an
    audio source uses, or to re-order those channels.

    Call the reset() method before using it to set up a default mapping, and then
    the setInputChannelMapping() and setOutputChannelMapping() methods to
    create an appropriate mapping, otherwise no channels will be connected and
    it'll produce silence.

    @see AudioSource

    @tags{Audio}
*/
class ChannelRemappingAudioSource  : public AudioSource
{
public:
    //==============================================================================
    /** Creates a remapping source that will pass on audio from the given input.

        @param source       the input source to use. Make sure that this doesn't
                            get deleted before the ChannelRemappingAudioSource object
        @param deleteSourceWhenDeleted  if true, the input source will be deleted
                            when this object is deleted, if false, the caller is
                            responsible for its deletion
    */
    ChannelRemappingAudioSource (AudioSource* source,
                                 bool deleteSourceWhenDeleted);

    /** Destructor. */
    ~ChannelRemappingAudioSource() override;

    //==============================================================================
    /** Specifies a number of channels that this audio source must produce from its
        getNextAudioBlock() callback.
    */
    void setNumberOfChannelsToProduce (int requiredNumberOfChannels);

    /** Clears any mapped channels.

        After this, no channels are mapped, so this object will produce silence. Create
        some mappings with setInputChannelMapping() and setOutputChannelMapping().
    */
    void clearAllMappings();

    /** Creates an input channel mapping.

        When the getNextAudioBlock() method is called, the data in channel sourceChannelIndex of the incoming
        data will be sent to destChannelIndex of our input source.

        @param destChannelIndex     the index of an input channel in our input audio source (i.e. the
                                    source specified when this object was created).
        @param sourceChannelIndex   the index of the input channel in the incoming audio data buffer
                                    during our getNextAudioBlock() callback
    */
    void setInputChannelMapping (int destChannelIndex,
                                 int sourceChannelIndex);

    /** Creates an output channel mapping.

        When the getNextAudioBlock() method is called, the data returned in channel sourceChannelIndex by
        our input audio source will be copied to channel destChannelIndex of the final buffer.

        @param sourceChannelIndex   the index of an output channel coming from our input audio source
                                    (i.e. the source specified when this object was created).
        @param destChannelIndex     the index of the output channel in the incoming audio data buffer
                                    during our getNextAudioBlock() callback
    */
    void setOutputChannelMapping (int sourceChannelIndex,
                                  int destChannelIndex);

    /** Returns the channel from our input that will be sent to channel inputChannelIndex of
        our input audio source.
    */
    int getRemappedInputChannel (int inputChannelIndex) const;

    /** Returns the output channel to which channel outputChannelIndex of our input audio
        source will be sent to.
    */
    int getRemappedOutputChannel (int outputChannelIndex) const;


    //==============================================================================
    /** Returns an XML object to encapsulate the state of the mappings.
        @see restoreFromXml
    */
    std::unique_ptr<XmlElement> createXml() const;

    /** Restores the mappings from an XML object created by createXML().
        @see createXml
    */
    void restoreFromXml (const XmlElement&);

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock (const AudioSourceChannelInfo&) override;


private:
    //==============================================================================
    OptionalScopedPointer<AudioSource> source;
    Array<int> remappedInputs, remappedOutputs;
    int requiredNumberOfChannels;

    AudioBuffer<float> buffer;
    AudioSourceChannelInfo remappedInfo;
    CriticalSection lock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelRemappingAudioSource)
};

} // namespace juce
