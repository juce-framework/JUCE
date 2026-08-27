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

   JUCE End User Licence Agreement: https://juce.com/legal/juce-9-licence/
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

#if JUCE_USE_OPUS || DOXYGEN

//==============================================================================
/**
    Reads and writes the Opus audio format ('.opus' files).

    An Opus stream always decodes at 48000 Hz, so that is the sample rate every
    reader created by this class reports. The sample rate given to the writer
    describes the incoming audio, which is resampled to 48000 Hz while encoding.

    To compile this, you'll need to set the JUCE_USE_OPUS flag.

    @see AudioFormat

    @tags{Audio}
*/
class JUCE_API  OpusAudioFormat  : public AudioFormat
{
public:
    //==============================================================================
    OpusAudioFormat();

    //==============================================================================
    /** Returns a set of sample rates that the format can read and write. The Opus file format
        only ever stores information at 48000 Hz, but the encoder can resample the audio from
        these rates.
    */
    Array<int> getPossibleSampleRates() override;

    Array<int> getPossibleBitDepths() override;
    bool canDoStereo() override;
    bool canDoMono() override;
    bool isChannelLayoutSupported (const AudioChannelSet& channelSet) override;
    bool isCompressed() override;
    StringArray getQualityOptions() override;

    //==============================================================================
    /** Metadata property name used by the opus writer - if you set a string for this
        value, it will be written into the opus file as the name of the encoder app.

        @see createWriterFor
    */
    inline static constexpr const char* encoderName = "encoder";
    inline static constexpr const char* id3title = "id3title";              /**< Metadata key for setting an ID3 title. */
    inline static constexpr const char* id3artist = "id3artist";            /**< Metadata key for setting an ID3 artist name. */
    inline static constexpr const char* id3album = "id3album";              /**< Metadata key for setting an ID3 album. */
    inline static constexpr const char* id3comment = "id3comment";          /**< Metadata key for setting an ID3 comment. */
    inline static constexpr const char* id3date = "id3date";                /**< Metadata key for setting an ID3 date. */
    inline static constexpr const char* id3genre = "id3genre";              /**< Metadata key for setting an ID3 genre. */
    inline static constexpr const char* id3trackNumber = "id3trackNumber";  /**< Metadata key for setting an ID3 track number. */

    //==============================================================================
    AudioFormatReader* createReaderFor (InputStream* sourceStream,
                                        bool deleteStreamIfOpeningFails) override;

    std::unique_ptr<AudioFormatWriter> createWriterFor (std::unique_ptr<OutputStream>& streamToWriteTo,
                                                        const AudioFormatWriterOptions& options) override;

    using AudioFormat::createWriterFor;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpusAudioFormat)
};

#endif

} // namespace juce
