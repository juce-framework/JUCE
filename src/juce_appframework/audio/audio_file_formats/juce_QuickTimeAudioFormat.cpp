/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../../../juce_Config.h"

#if JUCE_QUICKTIME_AUDIOFORMAT

#if ! defined (_WIN32)
 #include <Quicktime/Movies.h>
 #include <Quicktime/QTML.h>
 #include <Quicktime/QuickTimeComponents.h>
 #include <Quicktime/MediaHandlers.h>
 #include <Quicktime/ImageCodec.h>
#else
 #ifdef _MSC_VER
  #pragma warning (push)
  #pragma warning (disable : 4100)
 #endif

 #include <Movies.h>
 #include <QTML.h>
 #include <QuickTimeComponents.h>
 #include <MediaHandlers.h>
 #include <ImageCodec.h>

 #ifdef _MSC_VER
   #pragma warning (pop)
 #endif
#endif

#include "../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_QuickTimeAudioFormat.h"
#include "../../../juce_core/text/juce_LocalisedStrings.h"
#include "../../../juce_core/threads/juce_Thread.h"
#include "../../../juce_core/io/files/juce_FileInputStream.h"
#include "../../../juce_core/io/network/juce_URL.h"

#define qtFormatName    TRANS("QuickTime file")
static const tchar* const extensions[] =    { T(".mov"), T(".mp3"), 0 };

//==============================================================================
class QTAudioReader     : public AudioFormatReader
{
public:
    QTAudioReader (InputStream* const input_, const int trackNum_)
        : AudioFormatReader (input_, qtFormatName),
          ok (false),
          movie (0),
          trackNum (trackNum_),
          extractor (0),
          lastSampleRead (0),
          lastThreadId (0)
    {
        bufferList = (AudioBufferList*) juce_calloc (256);

#ifdef WIN32
        if (InitializeQTML (0) != noErr)
            return;
#endif
        if (EnterMovies() != noErr)
            return;

#if JUCE_MAC
        EnterMoviesOnThread (0);
#endif

        if (! openMovie (input_))
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

        AudioChannelLayout* const qt_audio_channel_layout
                = (AudioChannelLayout*) juce_calloc (output_layout_size);

        err = MovieAudioExtractionGetProperty (extractor,
                                               kQTPropertyClass_MovieAudioExtraction_Audio,
                                               kQTMovieAudioExtractionAudioPropertyID_AudioChannelLayout,
                                               output_layout_size, qt_audio_channel_layout, 0);

        qt_audio_channel_layout->mChannelLayoutTag = kAudioChannelLayoutTag_Stereo;

        err = MovieAudioExtractionSetProperty (extractor,
                                               kQTPropertyClass_MovieAudioExtraction_Audio,
                                               kQTMovieAudioExtractionAudioPropertyID_AudioChannelLayout,
                                               sizeof (qt_audio_channel_layout),
                                               qt_audio_channel_layout);

        juce_free (qt_audio_channel_layout);

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
        inputStreamDesc.mChannelsPerFrame = jmin (2, inputStreamDesc.mChannelsPerFrame);
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
        if (extractor != 0)
        {
            MovieAudioExtractionEnd (extractor);
            extractor = 0;
        }

        checkThreadIsAttached();
        DisposeMovie (movie);

        juce_free (bufferList->mBuffers[0].mData);
        juce_free (bufferList);
    }

    bool read (int** destSamples,
               int64 startSample,
               int numSamples)
    {
        checkThreadIsAttached();
        int done = 0;

        while (numSamples > 0)
        {
            if (! loadFrame ((int) startSample))
                return false;

            const int numToDo = jmin (numSamples, samplesPerFrame);

            for (unsigned int j = 0; j < inputStreamDesc.mChannelsPerFrame; ++j)
            {
                if (destSamples[j] != 0)
                {
                    const short* const src = ((const short*) bufferList->mBuffers[0].mData) + j;

                    for (int i = 0; i < numToDo; ++i)
                        destSamples[j][done + i] = src [i << 1] << 16;
                }
            }

            done += numToDo;
            startSample += numToDo;
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
    int lastSampleRead, lastThreadId;
    MovieAudioExtractionRef extractor;
    AudioStreamBasicDescription inputStreamDesc;
    AudioBufferList* bufferList;

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

    static Handle createHandleDataRef (Handle dataHandle, const char* fileName)
    {
        Handle dataRef = 0;
        OSStatus err = PtrToHand (&dataHandle, &dataRef, sizeof (Handle));
        if (err == noErr)
        {
            Str255 suffix;
#if JUCE_WIN32
            strcpy_s ((char*) suffix, 128, fileName);
#else
            strcpy ((char*) suffix, fileName);
#endif
            StringPtr name = suffix;
            err = PtrAndHand (name, dataRef, name[0]+1);

            if (err == noErr)
            {
                long atoms[3];
                atoms[0] = EndianU32_NtoB (3 * sizeof (long));
                atoms[1] = EndianU32_NtoB (kDataRefExtensionMacOSFileType);
                atoms[2] = EndianU32_NtoB (MovieFileType);

                err = PtrAndHand (atoms, dataRef, 3 * sizeof (long));

                if (err == noErr)
                    return dataRef;
            }

            DisposeHandle (dataRef);
        }

        return 0;
    }

    static CFStringRef juceStringToCFString (const String& s)
    {
        const int len = s.length();
        const juce_wchar* const t = (const juce_wchar*) s;

        UniChar* temp = (UniChar*) juce_malloc (sizeof (UniChar) * len + 4);

        for (int i = 0; i <= len; ++i)
            temp[i] = t[i];

        CFStringRef result = CFStringCreateWithCharacters (kCFAllocatorDefault, temp, len);
        juce_free (temp);

        return result;
    }

    //==============================================================================
    bool openMovie (InputStream* const input)
    {
        bool ok = false;

        QTNewMoviePropertyElement props[5];
        zeromem (props, sizeof (props));
        int prop = 0;

        FileInputStream* fin = dynamic_cast <FileInputStream*> (input);

        if (fin != 0)
        {
            String path (fin->getFile().getFullPathName().replaceCharacter (T('\\'), T('/')));
            if (path.startsWithChar (T('/')))
                path = path.substring (1);

            CFStringRef pathString = juceStringToCFString (T("file://") + URL::addEscapeChars (path));
            CFURLRef urlRef = CFURLCreateWithString (kCFAllocatorDefault, pathString, 0);
	        CFRelease (pathString);

            props[prop].propClass = kQTPropertyClass_DataLocation;
            props[prop].propID = kQTDataLocationPropertyID_CFURL;
            props[prop].propValueSize = sizeof (urlRef);
            props[prop].propValueAddress = &urlRef;
            ++prop;

            ok = openMovie (props, prop);
        }
        else
        {
            // sanity-check because this currently needs to load the whole stream into memory..
            jassert (input->getTotalLength() < 50 * 1024 * 1024);

            Handle dataHandle = NewHandle ((Size) input->getTotalLength());
            HLock (dataHandle);
            // read the entire stream into memory - this is a pain, but can't get it to work
            // properly using a custom callback to supply the data.
            input->read (*dataHandle, (int) input->getTotalLength());
            HUnlock (dataHandle);

            // different types to get QT to try. (We should really be a bit smarter here by
            // working out in advance which one the stream contains, rather than just trying
            // each one)
            const char* const suffixesToTry[] = { "\04.mov", "\04.mp3", 
                                                  "\04.avi", "\04.m4a" };

            for (int i = 0; i < numElementsInArray (suffixesToTry) && ! ok; ++i)
            {
                Handle dataRef = createHandleDataRef (dataHandle, suffixesToTry [i]);

                /*  // this fails for some bizarre reason - it can be bodged to work with
                    // movies, but can't seem to do it for other file types..
                QTNewMovieUserProcRecord procInfo;
                procInfo.getMovieUserProc = NewGetMovieUPP (readMovieStreamProc);
                procInfo.getMovieUserProcRefcon = this;
                procInfo.defaultDataRef.dataRef = dataRef;
                procInfo.defaultDataRef.dataRefType = HandleDataHandlerSubType;

                props[prop].propClass = kQTPropertyClass_DataLocation;
                props[prop].propID = kQTDataLocationPropertyID_MovieUserProc;
                props[prop].propValueSize = sizeof (procInfo);
                props[prop].propValueAddress = (void*) &procInfo;
                ++prop; */

                DataReferenceRecord dr;
                dr.dataRef = dataRef;
                dr.dataRefType = HandleDataHandlerSubType;
                props[prop].propClass = kQTPropertyClass_DataLocation;
                props[prop].propID = kQTDataLocationPropertyID_DataReference;
                props[prop].propValueSize = sizeof (dr);
                props[prop].propValueAddress = (void*) &dr;
                ++prop;

                ok = openMovie (props, prop);

                DisposeHandle (dataRef);
            }

            DisposeHandle (dataHandle);
        }

        return ok;
    }

    bool openMovie (QTNewMoviePropertyElement* props, int prop)
    {
        Boolean trueBool = true;
        props[prop].propClass = kQTPropertyClass_MovieInstantiation;
        props[prop].propID = kQTMovieInstantiationPropertyID_DontResolveDataRefs;
        props[prop].propValueSize = sizeof (trueBool);
        props[prop].propValueAddress = &trueBool;
        ++prop;

        Boolean isActive = true;
        props[prop].propClass = kQTPropertyClass_NewMovieProperty;
        props[prop].propID = kQTNewMoviePropertyID_Active;
        props[prop].propValueSize = sizeof (isActive);
        props[prop].propValueAddress = &isActive;
        ++prop;

#if JUCE_MAC
        SetPort (0);
#else
        MacSetPort (0);
#endif

        return NewMovieFromProperties (prop, props, 0, 0, &movie) == noErr;
    }

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
    : AudioFormat (qtFormatName, (const tchar**) extensions)
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
    QTAudioReader* r = new QTAudioReader (sourceStream, 0);

    if (! r->ok)
    {
        if (! deleteStreamIfOpeningFails)
            r->input = 0;

        deleteAndZero (r);
    }

    return r;
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
