/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../../core/juce_TargetPlatform.h"
#include "../../../juce_Config.h"

#if JUCE_QUICKTIME && ! (JUCE_64BIT || JUCE_IPHONE)

#if ! JUCE_WINDOWS
 #include <QuickTime/Movies.h>
 #include <QuickTime/QTML.h>
 #include <QuickTime/QuickTimeComponents.h>
 #include <QuickTime/MediaHandlers.h>
 #include <QuickTime/ImageCodec.h>
#else
 #if JUCE_MSVC
  #pragma warning (push)
  #pragma warning (disable : 4100)
 #endif

 /* If you've got an include error here, you probably need to install the QuickTime SDK and
    add its header directory to your include path.

    Alternatively, if you don't need any QuickTime services, just turn off the JUC_QUICKTIME
    flag in juce_Config.h
 */
 #include <Movies.h>
 #include <QTML.h>
 #include <QuickTimeComponents.h>
 #include <MediaHandlers.h>
 #include <ImageCodec.h>

 #if JUCE_MSVC
   #pragma warning (pop)
 #endif
#endif

#include "../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_QuickTimeAudioFormat.h"
#include "../../text/juce_LocalisedStrings.h"
#include "../../threads/juce_Thread.h"
#include "../../io/network/juce_URL.h"

bool juce_OpenQuickTimeMovieFromStream (InputStream* input, Movie& movie, Handle& dataHandle);

static const char* const quickTimeFormatName = "QuickTime file";
static const tchar* const quickTimeExtensions[] =    { T(".mov"), T(".mp3"), T(".mp4"), 0 };

//==============================================================================
class QTAudioReader     : public AudioFormatReader
{
public:
    QTAudioReader (InputStream* const input_, const int trackNum_)
        : AudioFormatReader (input_, TRANS (quickTimeFormatName)),
          ok (false),
          movie (0),
          trackNum (trackNum_),
          extractor (0),
          lastSampleRead (0),
          lastThreadId (0),
          dataHandle (0)
    {
        bufferList.calloc (256, 1);

#ifdef WIN32
        if (InitializeQTML (0) != noErr)
            return;
#endif
        if (EnterMovies() != noErr)
            return;

        bool opened = juce_OpenQuickTimeMovieFromStream (input_, movie, dataHandle);

        if (! opened)
            return;

        {
            const int numTracks = GetMovieTrackCount (movie);
            int trackCount = 0;

            for (int i = 1; i <= numTracks; ++i)
            {
                track = GetMovieIndTrack (movie, i);
                media = GetTrackMedia (track);

                OSType mediaType;
                GetMediaHandlerDescription (media, &mediaType, 0, 0);

                if (mediaType == SoundMediaType
                     && trackCount++ == trackNum_)
                {
                    ok = true;
                    break;
                }
            }
        }

        if (! ok)
            return;

        ok = false;

        lengthInSamples = GetMediaDecodeDuration (media);
        usesFloatingPointData = false;

        samplesPerFrame = (int) (GetMediaDecodeDuration (media) / GetMediaSampleCount (media));

        trackUnitsPerFrame = GetMovieTimeScale (movie) * samplesPerFrame
                                / GetMediaTimeScale (media);

        OSStatus err = MovieAudioExtractionBegin (movie, 0, &extractor);

        unsigned long output_layout_size;
        err = MovieAudioExtractionGetPropertyInfo (extractor,
                                                   kQTPropertyClass_MovieAudioExtraction_Audio,
                                                   kQTMovieAudioExtractionAudioPropertyID_AudioChannelLayout,
                                                   0, &output_layout_size, 0);
        if (err != noErr)
            return;

        HeapBlock <AudioChannelLayout> qt_audio_channel_layout;
        qt_audio_channel_layout.calloc (output_layout_size, 1);

        err = MovieAudioExtractionGetProperty (extractor,
                                               kQTPropertyClass_MovieAudioExtraction_Audio,
                                               kQTMovieAudioExtractionAudioPropertyID_AudioChannelLayout,
                                               output_layout_size, qt_audio_channel_layout, 0);

        qt_audio_channel_layout[0].mChannelLayoutTag = kAudioChannelLayoutTag_Stereo;

        err = MovieAudioExtractionSetProperty (extractor,
                                               kQTPropertyClass_MovieAudioExtraction_Audio,
                                               kQTMovieAudioExtractionAudioPropertyID_AudioChannelLayout,
                                               output_layout_size,
                                               qt_audio_channel_layout);

        err = MovieAudioExtractionGetProperty (extractor,
                                               kQTPropertyClass_MovieAudioExtraction_Audio,
                                               kQTMovieAudioExtractionAudioPropertyID_AudioStreamBasicDescription,
                                               sizeof (inputStreamDesc),
                                               &inputStreamDesc, 0);
        if (err != noErr)
            return;

        inputStreamDesc.mFormatFlags = kAudioFormatFlagIsSignedInteger
                                        | kAudioFormatFlagIsPacked
                                        | kAudioFormatFlagsNativeEndian;
        inputStreamDesc.mBitsPerChannel = sizeof (SInt16) * 8;
        inputStreamDesc.mChannelsPerFrame = jmin ((UInt32) 2, inputStreamDesc.mChannelsPerFrame);
        inputStreamDesc.mBytesPerFrame = sizeof (SInt16) * inputStreamDesc.mChannelsPerFrame;
        inputStreamDesc.mBytesPerPacket = inputStreamDesc.mBytesPerFrame;

        err = MovieAudioExtractionSetProperty (extractor,
                                               kQTPropertyClass_MovieAudioExtraction_Audio,
                                               kQTMovieAudioExtractionAudioPropertyID_AudioStreamBasicDescription,
                                               sizeof (inputStreamDesc),
                                               &inputStreamDesc);
        if (err != noErr)
            return;

        Boolean allChannelsDiscrete = false;
        err = MovieAudioExtractionSetProperty (extractor,
                                               kQTPropertyClass_MovieAudioExtraction_Movie,
                                               kQTMovieAudioExtractionMoviePropertyID_AllChannelsDiscrete,
                                               sizeof (allChannelsDiscrete),
                                               &allChannelsDiscrete);

        if (err != noErr)
            return;

        bufferList->mNumberBuffers = 1;
        bufferList->mBuffers[0].mNumberChannels = inputStreamDesc.mChannelsPerFrame;
        bufferList->mBuffers[0].mDataByteSize = (UInt32) (samplesPerFrame * inputStreamDesc.mBytesPerFrame) + 16;
        bufferList->mBuffers[0].mData = malloc (bufferList->mBuffers[0].mDataByteSize);

        sampleRate = inputStreamDesc.mSampleRate;
        bitsPerSample = 16;
        numChannels = inputStreamDesc.mChannelsPerFrame;

        detachThread();
        ok = true;
    }

    ~QTAudioReader()
    {
        if (dataHandle != 0)
            DisposeHandle (dataHandle);

        if (extractor != 0)
        {
            MovieAudioExtractionEnd (extractor);
            extractor = 0;
        }

        checkThreadIsAttached();
        DisposeMovie (movie);

        juce_free (bufferList->mBuffers[0].mData);

#if JUCE_MAC
        ExitMoviesOnThread ();
#endif
    }

    bool readSamples (int** destSamples, int numDestChannels, int startOffsetInDestBuffer,
                      int64 startSampleInFile, int numSamples)
    {
        checkThreadIsAttached();

        while (numSamples > 0)
        {
            if (! loadFrame ((int) startSampleInFile))
                return false;

            const int numToDo = jmin (numSamples, samplesPerFrame);

            for (int j = numDestChannels; --j >= 0;)
            {
                if (destSamples[j] != 0)
                {
                    const short* const src = ((const short*) bufferList->mBuffers[0].mData) + j;

                    for (int i = 0; i < numToDo; ++i)
                        destSamples[j][startOffsetInDestBuffer + i] = src [i << 1] << 16;
                }
            }

            startOffsetInDestBuffer += numToDo;
            startSampleInFile += numToDo;
            numSamples -= numToDo;
        }

        detachThread();
        return true;
    }

    bool loadFrame (const int sampleNum)
    {
        if (lastSampleRead != sampleNum)
        {
            TimeRecord time;
            time.scale = (TimeScale) inputStreamDesc.mSampleRate;
            time.base = 0;
            time.value.hi = 0;
            time.value.lo = (UInt32) sampleNum;

            OSStatus err = MovieAudioExtractionSetProperty (extractor,
                                                            kQTPropertyClass_MovieAudioExtraction_Movie,
                                                            kQTMovieAudioExtractionMoviePropertyID_CurrentTime,
                                                            sizeof (time), &time);

            if (err != noErr)
                return false;
        }

        bufferList->mBuffers[0].mDataByteSize = inputStreamDesc.mBytesPerFrame * samplesPerFrame;

        UInt32 outFlags = 0;
        UInt32 actualNumSamples = samplesPerFrame;
        OSStatus err = MovieAudioExtractionFillBuffer (extractor, &actualNumSamples,
                                                       bufferList, &outFlags);

        lastSampleRead = sampleNum + samplesPerFrame;

        return err == noErr;
    }

    juce_UseDebuggingNewOperator

    bool ok;

private:
    Movie movie;
    Media media;
    Track track;
    const int trackNum;
    double trackUnitsPerFrame;
    int samplesPerFrame;
    int lastSampleRead;
    Thread::ThreadID lastThreadId;
    MovieAudioExtractionRef extractor;
    AudioStreamBasicDescription inputStreamDesc;
    HeapBlock <AudioBufferList> bufferList;
    Handle dataHandle;

    /*OSErr readMovieStream (long offset, long size, void* dataPtr)
    {
        input->setPosition (offset);
        input->read (dataPtr, size);
        return noErr;
    }

    static OSErr readMovieStreamProc (long offset, long size, void* dataPtr, void* userRef)
    {
        return ((QTAudioReader*) userRef)->readMovieStream (offset, size, dataPtr);
    }*/

    //==============================================================================
    void checkThreadIsAttached()
    {
#if JUCE_MAC
        if (Thread::getCurrentThreadId() != lastThreadId)
            EnterMoviesOnThread (0);
        AttachMovieToCurrentThread (movie);
#endif
    }

    void detachThread()
    {
#if JUCE_MAC
        DetachMovieFromCurrentThread (movie);
#endif
    }
};


//==============================================================================
QuickTimeAudioFormat::QuickTimeAudioFormat()
    : AudioFormat (TRANS (quickTimeFormatName), (const tchar**) quickTimeExtensions)
{
}

QuickTimeAudioFormat::~QuickTimeAudioFormat()
{
}

const Array <int> QuickTimeAudioFormat::getPossibleSampleRates()
{
    return Array<int>();
}

const Array <int> QuickTimeAudioFormat::getPossibleBitDepths()
{
    return Array<int>();
}

bool QuickTimeAudioFormat::canDoStereo()
{
    return true;
}

bool QuickTimeAudioFormat::canDoMono()
{
    return true;
}

//==============================================================================
AudioFormatReader* QuickTimeAudioFormat::createReaderFor (InputStream* sourceStream,
                                                          const bool deleteStreamIfOpeningFails)
{
    ScopedPointer <QTAudioReader> r (new QTAudioReader (sourceStream, 0));

    if (r->ok)
        return r.release();

    if (! deleteStreamIfOpeningFails)
        r->input = 0;

    return 0;
}

AudioFormatWriter* QuickTimeAudioFormat::createWriterFor (OutputStream* /*streamToWriteTo*/,
                                                          double /*sampleRateToUse*/,
                                                          unsigned int /*numberOfChannels*/,
                                                          int /*bitsPerSample*/,
                                                          const StringPairArray& /*metadataValues*/,
                                                          int /*qualityOptionIndex*/)
{
    jassertfalse // not yet implemented!
    return 0;
}

END_JUCE_NAMESPACE

#endif
