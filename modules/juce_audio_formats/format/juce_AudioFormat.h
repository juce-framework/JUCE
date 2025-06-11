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
    Subclasses of AudioFormat are used to read and write different audio
    file formats.

    @see AudioFormatReader, AudioFormatWriter, WavAudioFormat, AiffAudioFormat

    @tags{Audio}
*/
class JUCE_API  AudioFormat
{
public:
    //==============================================================================
    /** Destructor. */
    virtual ~AudioFormat();

    //==============================================================================
    /** Returns the name of this format.
        e.g. "WAV file" or "AIFF file"
    */
    const String& getFormatName() const;

    //==============================================================================
    /** Returns all the file extensions that might apply to a file of this format.
        The first item will be the one that's preferred when creating a new file.
        So for a wav file this might just return ".wav"; for an AIFF file it might
        return two items, ".aif" and ".aiff"
    */
    virtual StringArray getFileExtensions() const;

    /** Returns true if this the given file can be read by this format.
        Subclasses shouldn't do too much work here, just check the extension or
        file type. The base class implementation just checks the file's extension
        against one of the ones that was registered in the constructor.
    */
    virtual bool canHandleFile (const File& fileToTest);

    /** Returns a set of sample rates that the format can read and write. */
    virtual Array<int> getPossibleSampleRates() = 0;

    /** Returns a set of bit depths that the format can read and write. */
    virtual Array<int> getPossibleBitDepths() = 0;

    /** Returns true if the format can do 2-channel audio. */
    virtual bool canDoStereo() = 0;

    /** Returns true if the format can do 1-channel audio. */
    virtual bool canDoMono() = 0;

    /** Returns true if the format uses compressed data. */
    virtual bool isCompressed();

    /** Returns true if the channel layout is supported by this format. */
    virtual bool isChannelLayoutSupported (const AudioChannelSet& channelSet);

    /** Returns a list of different qualities that can be used when writing.

        Non-compressed formats will just return an empty array, but for something
        like Ogg-Vorbis or MP3, it might return a list of bit-rates, etc.

        When calling createWriterFor(), an index from this array is passed in to
        tell the format which option is required.
    */
    virtual StringArray getQualityOptions();

    //==============================================================================
    /** Tries to create an object that can read from a stream containing audio
        data in this format.

        The reader object that is returned can be used to read from the stream, and
        should then be deleted by the caller.

        @param sourceStream                 the stream to read from - the AudioFormatReader object
                                            that is returned will delete this stream when it no longer
                                            needs it.
        @param deleteStreamIfOpeningFails   if no reader can be created, this determines whether this method
                                            should delete the stream object that was passed-in. (If a valid
                                            reader is returned, it will always be in charge of deleting the
                                            stream, so this parameter is ignored)
        @see AudioFormatReader
    */
    virtual AudioFormatReader* createReaderFor (InputStream* sourceStream,
                                                bool deleteStreamIfOpeningFails) = 0;

    /** Attempts to create a MemoryMappedAudioFormatReader, if possible for this format.
        If the format does not support this, the method will return nullptr;
    */
    virtual MemoryMappedAudioFormatReader* createMemoryMappedReader (const File& file);
    virtual MemoryMappedAudioFormatReader* createMemoryMappedReader (FileInputStream* fin);

    /** Tries to create an object that can write to a stream with this audio format.

        If the writer can't be created for some reason (e.g. the parameters passed in
        here aren't suitable), this will return nullptr.

        @param streamToWriteTo  a reference to a unique_ptr that owns the output stream. If creating
                                the writer succeeds, then ownership of the stream will be
                                transferred to the writer, and this argument will be set to nullptr.
                                If creating the writer fails, then streamToWriteTo will remain
                                unchanged, allowing it to be reused to create a writer of a
                                different format.
        @param options          options that specify details of the output file. If the audio format
                                does not support these settings, then this function may return
                                nullptr.

        @see AudioFormatWriterOptions
    */
    virtual std::unique_ptr<AudioFormatWriter> createWriterFor (std::unique_ptr<OutputStream>& streamToWriteTo,
                                                                const AudioFormatWriterOptions& options) = 0;

    /** Tries to create an object that can write to a stream with this audio format.

        The writer object that is returned can be used to write to the stream, and
        should then be deleted by the caller.

        If the stream can't be created for some reason (e.g. the parameters passed in
        here aren't suitable), this will return nullptr.

        @param streamToWriteTo      the stream that the data will go to - this will be
                                    deleted by the AudioFormatWriter object when it's no longer
                                    needed. If no AudioFormatWriter can be created by this method,
                                    the stream will NOT be deleted, so that the caller can re-use it
                                    to try to open a different format, etc
        @param sampleRateToUse      the sample rate for the file, which must be one of the ones
                                    returned by getPossibleSampleRates()
        @param numberOfChannels     the number of channels
        @param bitsPerSample        the bits per sample to use - this must be one of the values
                                    returned by getPossibleBitDepths()
        @param metadataValues       a set of metadata values that the writer should try to write
                                    to the stream. Exactly what these are depends on the format,
                                    and the subclass doesn't actually have to do anything with
                                    them if it doesn't want to. Have a look at the specific format
                                    implementation classes to see possible values that can be
                                    used
        @param qualityOptionIndex   the index of one of compression qualities returned by the
                                    getQualityOptions() method. If there aren't any quality options
                                    for this format, just pass 0 in this parameter, as it'll be
                                    ignored
        @see AudioFormatWriter
    */
    [[deprecated ("Use the function taking an AudioFormatWriterOptions instead.")]]
    AudioFormatWriter* createWriterFor (OutputStream* streamToWriteTo,
                                        double sampleRateToUse,
                                        unsigned int numberOfChannels,
                                        int bitsPerSample,
                                        const StringPairArray& metadataValues,
                                        int qualityOptionIndex);

    /** Tries to create an object that can write to a stream with this audio format.

        The writer object that is returned can be used to write to the stream, and
        should then be deleted by the caller.

        If the stream can't be created for some reason (e.g. the parameters passed in
        here aren't suitable), this will return nullptr.

        @param streamToWriteTo      the stream that the data will go to - this will be
                                    deleted by the AudioFormatWriter object when it's no longer
                                    needed. If no AudioFormatWriter can be created by this method,
                                    the stream will NOT be deleted, so that the caller can re-use it
                                    to try to open a different format, etc
        @param sampleRateToUse      the sample rate for the file, which must be one of the ones
                                    returned by getPossibleSampleRates()
        @param channelLayout        the channel layout for the file. Use isChannelLayoutSupported
                                    to check if the writer supports this layout.
        @param bitsPerSample        the bits per sample to use - this must be one of the values
                                    returned by getPossibleBitDepths()
        @param metadataValues       a set of metadata values that the writer should try to write
                                    to the stream. Exactly what these are depends on the format,
                                    and the subclass doesn't actually have to do anything with
                                    them if it doesn't want to. Have a look at the specific format
                                    implementation classes to see possible values that can be
                                    used
        @param qualityOptionIndex   the index of one of compression qualities returned by the
                                    getQualityOptions() method. If there aren't any quality options
                                    for this format, just pass 0 in this parameter, as it'll be
                                    ignored
        @see AudioFormatWriter
    */
    [[deprecated ("Use the function taking an AudioFormatWriterOptions instead.")]]
    AudioFormatWriter* createWriterFor (OutputStream* streamToWriteTo,
                                        double sampleRateToUse,
                                        const AudioChannelSet& channelLayout,
                                        int bitsPerSample,
                                        const StringPairArray& metadataValues,
                                        int qualityOptionIndex);

protected:
    /** Creates an AudioFormat object.

        @param formatName       this sets the value that will be returned by getFormatName()
        @param fileExtensions   an array of file extensions - these will be returned by getFileExtensions()
    */
    AudioFormat (String formatName, StringArray fileExtensions);

    /** Creates an AudioFormat object.

        @param formatName       this sets the value that will be returned by getFormatName()
        @param fileExtensions   a whitespace-separated list of file extensions - these will
                                be returned by getFileExtensions()
    */
    AudioFormat (StringRef formatName, StringRef fileExtensions);

private:
    AudioFormatWriter* createWriterForRawPtr (OutputStream*, const AudioFormatWriterOptions&);

    //==============================================================================
    String formatName;
    StringArray fileExtensions;
};

} // namespace juce
