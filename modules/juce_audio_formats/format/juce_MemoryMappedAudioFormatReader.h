/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_MEMORYMAPPEDAUDIOFORMATREADER_JUCEHEADER__
#define __JUCE_MEMORYMAPPEDAUDIOFORMATREADER_JUCEHEADER__

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
    bool mapSectionOfFile (const Range<int64>& samplesToMap);

    /** Returns the sample range that's currently memory-mapped and available for reading. */
    const Range<int64>& getMappedSection() const noexcept   { return mappedSection; }

    /** Touches the memory for the given sample, to force it to be loaded into active memory. */
    void touchSample (int64 sample) const noexcept;

    /** Returns the number of bytes currently being mapped */
    size_t getNumBytesUsed() const           { return map != nullptr ? map->getSize() : 0; }

protected:
    File file;
    Range<int64> mappedSection;
    ScopedPointer<MemoryMappedFile> map;
    int64 dataChunkStart, dataLength;
    int bytesPerFrame;

    inline int64 sampleToFilePos (int64 sample) const noexcept       { return dataChunkStart + sample * bytesPerFrame; }
    inline int64 filePosToSample (int64 filePos) const noexcept      { return (filePos - dataChunkStart) / bytesPerFrame; }
    inline const void* sampleToPointer (int64 sample) const noexcept { return addBytesToPointer (map->getData(), sampleToFilePos (sample) - map->getRange().getStart()); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MemoryMappedAudioFormatReader)
};


#endif   // __JUCE_MEMORYMAPPEDAUDIOFORMATREADER_JUCEHEADER__
