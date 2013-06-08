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

#if JUCE_MAC || JUCE_IOS

//==============================================================================
namespace
{
    const char* const coreAudioFormatName = "CoreAudio supported file";

    StringArray findFileExtensionsForCoreAudioCodecs()
    {
        StringArray extensionsArray;
        CFArrayRef extensions = nullptr;
        UInt32 sizeOfArray = sizeof (extensions);

        if (AudioFileGetGlobalInfo (kAudioFileGlobalInfo_AllExtensions, 0, 0, &sizeOfArray, &extensions) == noErr)
        {
            const CFIndex numValues = CFArrayGetCount (extensions);

            for (CFIndex i = 0; i < numValues; ++i)
                extensionsArray.add ("." + String::fromCFString ((CFStringRef) CFArrayGetValueAtIndex (extensions, i)));

            CFRelease (extensions);
        }

        return extensionsArray;
    }
}

//==============================================================================
const char* const CoreAudioFormat::midiDataBase64   = "midiDataBase64";
const char* const CoreAudioFormat::tempo            = "tempo";
const char* const CoreAudioFormat::timeSig          = "time signature";

//==============================================================================
struct CoreAudioFormatMetatdata
{
    static uint32 chunkName (const char* const name) noexcept   { return ByteOrder::bigEndianInt (name); }

    //==============================================================================
    struct FileHeader
    {
        FileHeader (InputStream& input)
        {
            fileType    = input.readIntBigEndian();
            fileVersion = input.readShortBigEndian();
            fileFlags   = input.readShortBigEndian();
        }

        uint32 fileType;
        uint16 fileVersion;
        uint16 fileFlags;
    };

    //==============================================================================
    struct ChunkHeader
    {
        ChunkHeader (InputStream& input)
        {
            chunkType = input.readIntBigEndian();
            chunkSize = input.readInt64BigEndian();
        }

        uint32 chunkType;
        int64 chunkSize;
    };

    //==============================================================================
    struct AudioDescriptionChunk
    {
        AudioDescriptionChunk (InputStream& input)
        {
            sampleRate          = input.readDoubleBigEndian();
            formatID            = input.readIntBigEndian();
            formatFlags         = input.readIntBigEndian();
            bytesPerPacket      = input.readIntBigEndian();
            framesPerPacket     = input.readIntBigEndian();
            channelsPerFrame    = input.readIntBigEndian();
            bitsPerChannel      = input.readIntBigEndian();
        }

        double sampleRate;
        uint32 formatID;
        uint32 formatFlags;
        uint32 bytesPerPacket;
        uint32 framesPerPacket;
        uint32 channelsPerFrame;
        uint32 bitsPerChannel;
    };

    //==============================================================================
    struct UserDefinedChunk
    {
        UserDefinedChunk (InputStream& input, int64 size)
        {
            // a user defined chunk contains 16 bytes of a UUID first
            uuid[1] = input.readInt64BigEndian();
            uuid[0] = input.readInt64BigEndian();

            input.skipNextBytes (size - 16);
        }

        int64 uuid[2];
    };

    //==============================================================================
    static StringPairArray parseMidiChunk (InputStream& input, int64 size)
    {
        const int64 originalPosition = input.getPosition();

        MemoryBlock midiBlock;
        input.readIntoMemoryBlock (midiBlock, (ssize_t) size);
        MemoryInputStream midiInputStream (midiBlock, false);

        StringPairArray midiMetadata;
        MidiFile midiFile;

        if (midiFile.readFrom (midiInputStream))
        {
            midiMetadata.set (CoreAudioFormat::midiDataBase64, midiBlock.toBase64Encoding());

            findTempoEvents (midiFile, midiMetadata);
            findTimeSigEvents (midiFile, midiMetadata);
        }

        input.setPosition (originalPosition + size);
        return midiMetadata;
    }

    static void findTempoEvents (MidiFile& midiFile, StringPairArray& midiMetadata)
    {
        MidiMessageSequence tempoEvents;
        midiFile.findAllTempoEvents (tempoEvents);

        const int numTempoEvents = tempoEvents.getNumEvents();
        MemoryOutputStream tempoSequence;

        for (int i = 0; i < numTempoEvents; ++i)
        {
            const double tempo = getTempoFromTempoMetaEvent (tempoEvents.getEventPointer (i));

            if (tempo > 0.0)
            {
                if (i == 0)
                    midiMetadata.set (CoreAudioFormat::tempo, String (tempo));

                if (numTempoEvents > 1)
                    tempoSequence << String (tempo) << ',' << tempoEvents.getEventTime (i) << ';';
            }
        }

        if (tempoSequence.getDataSize() > 0)
            midiMetadata.set ("tempo sequence", tempoSequence.toString());
    }

    static double getTempoFromTempoMetaEvent (MidiMessageSequence::MidiEventHolder* holder)
    {
        if (holder != nullptr)
        {
            const MidiMessage& midiMessage = holder->message;

            if (midiMessage.isTempoMetaEvent())
            {
                const double tempoSecondsPerQuarterNote = midiMessage.getTempoSecondsPerQuarterNote();

                if (tempoSecondsPerQuarterNote > 0.0)
                    return 60.0 / tempoSecondsPerQuarterNote;
            }
        }

        return 0.0;
    }

    static void findTimeSigEvents (MidiFile& midiFile, StringPairArray& midiMetadata)
    {
        MidiMessageSequence timeSigEvents;
        midiFile.findAllTimeSigEvents (timeSigEvents);
        const int numTimeSigEvents = timeSigEvents.getNumEvents();

        MemoryOutputStream timeSigSequence;

        for (int i = 0; i < numTimeSigEvents; ++i)
        {
            int numerator, denominator;
            timeSigEvents.getEventPointer(i)->message.getTimeSignatureInfo (numerator, denominator);

            String timeSigString;
            timeSigString << numerator << '/' << denominator;

            if (i == 0)
                midiMetadata.set (CoreAudioFormat::timeSig, timeSigString);

            if (numTimeSigEvents > 1)
                timeSigSequence << timeSigString << ',' << timeSigEvents.getEventTime (i) << ';';
        }

        if (timeSigSequence.getDataSize() > 0)
            midiMetadata.set ("time signature sequence", timeSigSequence.toString());
    }

    //==============================================================================
    static StringPairArray parseInformationChunk (InputStream& input)
    {
        StringPairArray infoStrings;
        const uint32 numEntries = (uint32) input.readIntBigEndian();

        for (uint32 i = 0; i < numEntries; ++i)
            infoStrings.set (input.readString(), input.readString());

        return infoStrings;
    }

    //==============================================================================
    static bool read (InputStream& input, StringPairArray& metadataValues)
    {
        const int64 originalPos = input.getPosition();

        const FileHeader cafFileHeader (input);
        const bool isCafFile = cafFileHeader.fileType == chunkName ("caff");

        if (isCafFile)
        {
            while (! input.isExhausted())
            {
                const ChunkHeader chunkHeader (input);

                if (chunkHeader.chunkType == chunkName ("desc"))
                {
                    AudioDescriptionChunk audioDescriptionChunk (input);
                }
                else if (chunkHeader.chunkType == chunkName ("uuid"))
                {
                    UserDefinedChunk userDefinedChunk (input, chunkHeader.chunkSize);
                }
                else if (chunkHeader.chunkType == chunkName ("data"))
                {
                    // -1 signifies an unknown data size so the data has to be at the
                    // end of the file so we must have finished the header

                    if (chunkHeader.chunkSize == -1)
                        break;

                    input.skipNextBytes (chunkHeader.chunkSize);
                }
                else if (chunkHeader.chunkType == chunkName ("midi"))
                {
                    metadataValues.addArray (parseMidiChunk (input, chunkHeader.chunkSize));
                }
                else if (chunkHeader.chunkType == chunkName ("info"))
                {
                    metadataValues.addArray (parseInformationChunk (input));
                }
                else
                {
                    // we aren't decoding this chunk yet so just skip over it
                    input.skipNextBytes (chunkHeader.chunkSize);
                }
            }
        }

        input.setPosition (originalPos);

        return isCafFile;
    }
};

//==============================================================================
class CoreAudioReader : public AudioFormatReader
{
public:
    CoreAudioReader (InputStream* const inp)
        : AudioFormatReader (inp, TRANS (coreAudioFormatName)),
          ok (false), lastReadPosition (0)
    {
        usesFloatingPointData = true;
        bitsPerSample = 32;

        if (input != nullptr)
            CoreAudioFormatMetatdata::read (*input, metadataValues);

        OSStatus status = AudioFileOpenWithCallbacks (this,
                                                      &readCallback,
                                                      nullptr,  // write needs to be null to avoid permisisions errors
                                                      &getSizeCallback,
                                                      nullptr,  // setSize needs to be null to avoid permisisions errors
                                                      0,        // AudioFileTypeID inFileTypeHint
                                                      &audioFileID);
        if (status == noErr)
        {
            status = ExtAudioFileWrapAudioFileID (audioFileID, false, &audioFileRef);

            if (status == noErr)
            {
                AudioStreamBasicDescription sourceAudioFormat;
                UInt32 audioStreamBasicDescriptionSize = sizeof (AudioStreamBasicDescription);
                ExtAudioFileGetProperty (audioFileRef,
                                         kExtAudioFileProperty_FileDataFormat,
                                         &audioStreamBasicDescriptionSize,
                                         &sourceAudioFormat);

                numChannels = sourceAudioFormat.mChannelsPerFrame;
                sampleRate  = sourceAudioFormat.mSampleRate;

                UInt32 sizeOfLengthProperty = sizeof (int64);
                ExtAudioFileGetProperty (audioFileRef,
                                         kExtAudioFileProperty_FileLengthFrames,
                                         &sizeOfLengthProperty,
                                         &lengthInSamples);

                destinationAudioFormat.mSampleRate       = sampleRate;
                destinationAudioFormat.mFormatID         = kAudioFormatLinearPCM;
                destinationAudioFormat.mFormatFlags      = kLinearPCMFormatFlagIsFloat | kLinearPCMFormatFlagIsNonInterleaved | kAudioFormatFlagsNativeEndian;
                destinationAudioFormat.mBitsPerChannel   = sizeof (float) * 8;
                destinationAudioFormat.mChannelsPerFrame = numChannels;
                destinationAudioFormat.mBytesPerFrame    = sizeof (float);
                destinationAudioFormat.mFramesPerPacket  = 1;
                destinationAudioFormat.mBytesPerPacket   = destinationAudioFormat.mFramesPerPacket * destinationAudioFormat.mBytesPerFrame;

                status = ExtAudioFileSetProperty (audioFileRef,
                                                  kExtAudioFileProperty_ClientDataFormat,
                                                  sizeof (AudioStreamBasicDescription),
                                                  &destinationAudioFormat);
                if (status == noErr)
                {
                    bufferList.malloc (1, sizeof (AudioBufferList) + numChannels * sizeof (AudioBuffer));
                    bufferList->mNumberBuffers = numChannels;
                    ok = true;
                }
            }
        }
    }

    ~CoreAudioReader()
    {
        ExtAudioFileDispose (audioFileRef);
        AudioFileClose (audioFileID);
    }

    //==============================================================================
    bool readSamples (int** destSamples, int numDestChannels, int startOffsetInDestBuffer,
                      int64 startSampleInFile, int numSamples)
    {
        clearSamplesBeyondAvailableLength (destSamples, numDestChannels, startOffsetInDestBuffer,
                                           startSampleInFile, numSamples, lengthInSamples);

        if (numSamples <= 0)
            return true;

        if (lastReadPosition != startSampleInFile)
        {
            OSStatus status = ExtAudioFileSeek (audioFileRef, startSampleInFile);
            if (status != noErr)
                return false;

            lastReadPosition = startSampleInFile;
        }

        while (numSamples > 0)
        {
            const int numThisTime = jmin (8192, numSamples);
            const size_t numBytes = sizeof (float) * (size_t) numThisTime;

            audioDataBlock.ensureSize (numBytes * numChannels, false);
            float* data = static_cast<float*> (audioDataBlock.getData());

            for (int j = (int) numChannels; --j >= 0;)
            {
                bufferList->mBuffers[j].mNumberChannels = 1;
                bufferList->mBuffers[j].mDataByteSize = (UInt32) numBytes;
                bufferList->mBuffers[j].mData = data;
                data += numThisTime;
            }

            UInt32 numFramesToRead = (UInt32) numThisTime;
            OSStatus status = ExtAudioFileRead (audioFileRef, &numFramesToRead, bufferList);
            if (status != noErr)
                return false;

            for (int i = numDestChannels; --i >= 0;)
            {
                if (destSamples[i] != nullptr)
                {
                    if (i < (int) numChannels)
                        memcpy (destSamples[i] + startOffsetInDestBuffer, bufferList->mBuffers[i].mData, numBytes);
                    else
                        zeromem (destSamples[i] + startOffsetInDestBuffer, numBytes);
                }
            }

            startOffsetInDestBuffer += numThisTime;
            numSamples -= numThisTime;
            lastReadPosition += numThisTime;
        }

        return true;
    }

    bool ok;

private:
    AudioFileID audioFileID;
    ExtAudioFileRef audioFileRef;
    AudioStreamBasicDescription destinationAudioFormat;
    MemoryBlock audioDataBlock;
    HeapBlock<AudioBufferList> bufferList;
    int64 lastReadPosition;

    static SInt64 getSizeCallback (void* inClientData)
    {
        return static_cast<CoreAudioReader*> (inClientData)->input->getTotalLength();
    }

    static OSStatus readCallback (void* inClientData,
                                  SInt64 inPosition,
                                  UInt32 requestCount,
                                  void* buffer,
                                  UInt32* actualCount)
    {
        CoreAudioReader* const reader = static_cast<CoreAudioReader*> (inClientData);

        reader->input->setPosition (inPosition);
        *actualCount = (UInt32) reader->input->read (buffer, (int) requestCount);

        return noErr;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoreAudioReader)
};

//==============================================================================
CoreAudioFormat::CoreAudioFormat()
    : AudioFormat (TRANS (coreAudioFormatName), findFileExtensionsForCoreAudioCodecs())
{
}

CoreAudioFormat::~CoreAudioFormat() {}

Array<int> CoreAudioFormat::getPossibleSampleRates()    { return Array<int>(); }
Array<int> CoreAudioFormat::getPossibleBitDepths()      { return Array<int>(); }

bool CoreAudioFormat::canDoStereo()     { return true; }
bool CoreAudioFormat::canDoMono()       { return true; }

//==============================================================================
AudioFormatReader* CoreAudioFormat::createReaderFor (InputStream* sourceStream,
                                                     bool deleteStreamIfOpeningFails)
{
    ScopedPointer<CoreAudioReader> r (new CoreAudioReader (sourceStream));

    if (r->ok)
        return r.release();

    if (! deleteStreamIfOpeningFails)
        r->input = nullptr;

    return nullptr;
}

AudioFormatWriter* CoreAudioFormat::createWriterFor (OutputStream* streamToWriteTo,
                                                     double sampleRateToUse,
                                                     unsigned int numberOfChannels,
                                                     int bitsPerSample,
                                                     const StringPairArray& metadataValues,
                                                     int qualityOptionIndex)
{
    jassertfalse; // not yet implemented!
    return nullptr;
}

#endif
