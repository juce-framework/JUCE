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
class CoreAudioReader : public AudioFormatReader
{
public:
    CoreAudioReader (InputStream* const inp)
        : AudioFormatReader (inp, TRANS (coreAudioFormatName)),
          ok (false), lastReadPosition (0)
    {
        usesFloatingPointData = true;
        bitsPerSample = 32;

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
        jassert (destSamples != nullptr);
        const int64 samplesAvailable = lengthInSamples - startSampleInFile;

        if (samplesAvailable < numSamples)
        {
            for (int i = numDestChannels; --i >= 0;)
                if (destSamples[i] != nullptr)
                    zeromem (destSamples[i] + startOffsetInDestBuffer, sizeof (int) * (size_t) numSamples);

            numSamples = (int) samplesAvailable;
        }

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
