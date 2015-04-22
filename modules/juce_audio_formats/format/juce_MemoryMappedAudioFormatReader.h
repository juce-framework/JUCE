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

#ifndef JUCE_MEMORYMAPPEDAUDIOFORMATREADER_H_INCLUDED
#define JUCE_MEMORYMAPPEDAUDIOFORMATREADER_H_INCLUDED


//==============================================================================
/**
    A specialised type of AudioFormatReader that uses a MemoryMappedFile to read
    directly from an audio file.

    This allows for incredibly fast random-access to sample data in the mapped
    region of the file, but not all audio formats support it - see
    AudioFormat::createMemoryMappedReader().

    Note that before reading samples from a MemoryMappedAudioFormatReader, you must first
    call mapEntireFile() or mapSectionOfFile() to ensure that the region you want to
    read has been mapped.

    @see AudioFormat::createMemoryMappedReader, AudioFormatReader
*/
class JUCE_API  MemoryMappedAudioFormatReader  : public AudioFormatReader
{
protected:
    //==============================================================================
    /** Creates an MemoryMappedAudioFormatReader object.

        Note that before attempting to read any data, you must call mapEntireFile()
        or mapSectionOfFile() to ensure that the region you want to read has
        been mapped.
    */
    MemoryMappedAudioFormatReader (const File& file, const AudioFormatReader& details,
                                   int64 dataChunkStart, int64 dataChunkLength, int bytesPerFrame);

public:
    /** Returns the file that is being mapped */
    const File& getFile() const noexcept                    { return file; }

    /** Attempts to map the entire file into memory. */
    bool mapEntireFile();

    /** Attempts to map a section of the file into memory. */
    bool mapSectionOfFile (Range<int64> samplesToMap);

    /** Returns the sample range that's currently memory-mapped and available for reading. */
    Range<int64> getMappedSection() const noexcept          { return mappedSection; }

    /** Touches the memory for the given sample, to force it to be loaded into active memory. */
    void touchSample (int64 sample) const noexcept;

    /** Returns the samples for all channels at a given sample position.
        The result array must be large enough to hold a value for each channel
        that this reader contains.
    */
    virtual void getSample (int64 sampleIndex, float* result) const noexcept = 0;

    /** Returns the number of bytes currently being mapped */
    size_t getNumBytesUsed() const                          { return map != nullptr ? map->getSize() : 0; }

protected:
    File file;
    Range<int64> mappedSection;
    ScopedPointer<MemoryMappedFile> map;
    int64 dataChunkStart, dataLength;
    int bytesPerFrame;

    /** Converts a sample index to a byte position in the file. */
    inline int64 sampleToFilePos (int64 sample) const noexcept       { return dataChunkStart + sample * bytesPerFrame; }

    /** Converts a byte position in the file to a sample index. */
    inline int64 filePosToSample (int64 filePos) const noexcept      { return (filePos - dataChunkStart) / bytesPerFrame; }

    /** Converts a sample index to a pointer to the mapped file memory. */
    inline const void* sampleToPointer (int64 sample) const noexcept { return addBytesToPointer (map->getData(), sampleToFilePos (sample) - map->getRange().getStart()); }

    /** Used by AudioFormatReader subclasses to scan for min/max ranges in interleaved data. */
    template <typename SampleType, typename Endianness>
    Range<float> scanMinAndMaxInterleaved (int channel, int64 startSampleInFile, int64 numSamples) const noexcept
    {
        typedef AudioData::Pointer <SampleType, Endianness, AudioData::Interleaved, AudioData::Const> SourceType;

        return SourceType (addBytesToPointer (sampleToPointer (startSampleInFile), ((int) bitsPerSample / 8) * channel), (int) numChannels)
                .findMinAndMax ((size_t) numSamples);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemoryMappedAudioFormatReader)
};


#endif   // JUCE_MEMORYMAPPEDAUDIOFORMATREADER_H_INCLUDED
