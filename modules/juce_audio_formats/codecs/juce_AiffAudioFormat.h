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
    Reads and Writes AIFF format audio files.

    @see AudioFormat

    @tags{Audio}
*/
class JUCE_API  AiffAudioFormat  : public AudioFormat
{
public:
    //==============================================================================
    /** Creates an format object. */
    AiffAudioFormat();

    /** Destructor. */
    ~AiffAudioFormat() override;

    //==============================================================================
    /** Metadata property name used when reading a aiff file with a basc chunk. */
    static const char* const appleOneShot;
    /** Metadata property name used when reading a aiff file with a basc chunk. */
    static const char* const appleRootSet;
    /** Metadata property name used when reading a aiff file with a basc chunk. */
    static const char* const appleRootNote;
    /** Metadata property name used when reading a aiff file with a basc chunk. */
    static const char* const appleBeats;
    /** Metadata property name used when reading a aiff file with a basc chunk. */
    static const char* const appleDenominator;
    /** Metadata property name used when reading a aiff file with a basc chunk. */
    static const char* const appleNumerator;
    /** Metadata property name used when reading a aiff file with a basc chunk. */
    static const char* const appleTag;
    /** Metadata property name used when reading a aiff file with a basc chunk. */
    static const char* const appleKey;

    //==============================================================================
    Array<int> getPossibleSampleRates() override;
    Array<int> getPossibleBitDepths() override;
    bool canDoStereo() override;
    bool canDoMono() override;

   #if JUCE_MAC
    bool canHandleFile (const File& fileToTest) override;
   #endif

    //==============================================================================
    AudioFormatReader* createReaderFor (InputStream* sourceStream,
                                        bool deleteStreamIfOpeningFails) override;

    MemoryMappedAudioFormatReader* createMemoryMappedReader (const File&)      override;
    MemoryMappedAudioFormatReader* createMemoryMappedReader (FileInputStream*) override;

    std::unique_ptr<AudioFormatWriter> createWriterFor (std::unique_ptr<OutputStream>& streamToWriteTo,
                                                        const AudioFormatWriterOptions& options) override;

    using AudioFormat::createWriterFor;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AiffAudioFormat)
};

} // namespace juce
