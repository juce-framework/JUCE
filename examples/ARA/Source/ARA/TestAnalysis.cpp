//------------------------------------------------------------------------------
//! \file        TestAnalysis.cpp
//! \description implementation of audio source analysis for the ARA sample plug-in
//!              actual plug-ins will have an analysis implementation that is independent of ARA
//! \project     ARA SDK, examples
//------------------------------------------------------------------------------
// Copyright (c) 2012-2018, Celemony Software GmbH, All Rights Reserved.
//
// License & Disclaimer:
//
// This Software Development Kit (SDK) may not be distributed in parts or
// its entirety without prior written agreement of Celemony Software GmbH.
//
// This SDK must not be used to modify, adapt, reproduce or transfer any
// software and/or technology used in any Celemony and/or Third-party
// application and/or software module (hereinafter referred as "the Software");
// to modify, translate, reverse engineer, decompile, disassemble or create
// derivative works based on the Software (except to the extent applicable laws
// specifically prohibit such restriction) or to lease, assign, distribute or
// otherwise transfer rights to the Software.
// Neither the name of the Celemony Software GmbH nor the names of its
// contributors may be used to endorse or promote products derived from this
// SDK without specific prior written permission.
//
// This SDK is provided by Celemony Software GmbH "as is" and any express or
// implied warranties, including, but not limited to, the implied warranties of
// non-infringement, merchantability and fitness for a particular purpose are
// disclaimed.
// In no event shall Celemony Software GmbH be liable for any direct, indirect,
// incidental, special, exemplary, or consequential damages (including, but not
// limited to, procurement of substitute goods or services; loss of use, data,
// or profits; or business interruption) however caused and on any theory of
// liability, whether in contract, strict liability, or tort (including
// negligence or otherwise) arising in any way out of the use of this software,
// even if advised of the possibility of such damage.
// Where the liability of Celemony Software GmbH is ruled out or restricted,
// this will also apply for the personal liability of employees, representatives
// and vicarious agents.
// The above restriction of liability will not apply if the damages suffered
// are attributable to willful intent or gross negligence or in the case of
// physical injury.
//------------------------------------------------------------------------------

#include "TestAnalysis.h"
#include "ARATestAudioSource.h"

#include <chrono>


// the test plug-in pretends to be able to do a kARAContentTypeNotes analysis:
// to simulate this, it reads all samples and creates a note with invalid pitch for each range of
// consecutive samples that are not 0. while this is no meaningful algorithm for real-world signals,
// it works nicely with the pulsed sine wave that the test host is using, allowing for automated
// testing of content readers from both host and plug-in side.
// the time consumed by the fake analysis is the duration of the audio source scaled
// down by ARA_FAKE_NOTE_ANALYSIS_SPEED_FACTOR.
// if this is set to 0, the artificial delays are supressed.
#if !defined (ARA_FAKE_NOTE_ANALYSIS_SPEED_FACTOR)
    #define ARA_FAKE_NOTE_ANALYSIS_SPEED_FACTOR 20
#endif

// if desired, a custom timer for calculating the analysis delay can be injected by defining ARA_GET_CURRENT_TIME accordingly.
#if ARA_FAKE_NOTE_ANALYSIS_SPEED_FACTOR != 0
    #if defined (ARA_GET_CURRENT_TIME)
        double ARA_GET_CURRENT_TIME ();    /* declare custom time getter function */
    #else
        #define ARA_GET_CURRENT_TIME() (0.000001 * std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now ().time_since_epoch ()).count ())
    #endif
#endif

#if !defined (ARA_FAKE_NOTE_MAX_COUNT)
    #define ARA_FAKE_NOTE_MAX_COUNT 100
#endif


using namespace ARA;
using namespace PlugIn;


/*******************************************************************************/

TestAnalysisTask::TestAnalysisTask (ARATestAudioSource* audioSource)
: _audioSource (audioSource),
  _hostAudioReader (new HostAudioReader (audioSource)),        // create audio reader on the main thread, before dispatching to analysis thread
  _analysisResult (nullptr),
  _progress (0),
  _shouldCancel (false)
{
    _future = std::async (std::launch::async, [this] ()
    {
        // helper variables to artificially slow down analysis as indicated by ARA_FAKE_NOTE_ANALYSIS_SPEED_FACTOR
#if ARA_FAKE_NOTE_ANALYSIS_SPEED_FACTOR != 0
        ARATimePosition analysisStartTime = ARA_GET_CURRENT_TIME ();
        ARATimeDuration analysisTargetDuration = _audioSource->getDuration () / ARA_FAKE_NOTE_ANALYSIS_SPEED_FACTOR;
#endif

        // setup buffers and audio reader for reading samples
        const size_t blockSize = 64;
        const ARAChannelCount channelCount = _audioSource->getChannelCount ();
        std::vector<float> buffer (channelCount * blockSize);
        std::vector<void*> dataPointers (channelCount);
        for (ARAChannelCount c = 0; c < channelCount; ++c)
            dataPointers[c] = &buffer[c * blockSize];

        // search the audio for silence and treat each region between silence as a note
        ARASamplePosition blockStartIndex = 0;
        ARASamplePosition lastNoteStartIndex = 0;
        bool wasZero = true;    // samples before the start of the file are 0
        std::vector<TestAnalysisNote> foundNotes;
        while (true)
        {
            // check cancel
            if (_shouldCancel)
                return;

            // calculate size of current block and check if done
            ARASampleCount count = std::min ((ARASampleCount) blockSize, _audioSource->getSampleCount () - blockStartIndex);
            if (count <= 0)
                break;

            // read samples - note that this test code ignores any errors that the reader might return here!
            _hostAudioReader->readAudioSamples (blockStartIndex, count, dataPointers.data ());

            // analyze current block
            for (ARASamplePosition i = 0; i < count && foundNotes.size () < ARA_FAKE_NOTE_MAX_COUNT; ++i)
            {
                // check if current sample is zero on all channels
                bool isZero = true;
                for (ARAChannelCount c = 0; c < _audioSource->getChannelCount (); ++c)
                    isZero &= (buffer[(size_t) i + c * blockSize] == 0.0f);

                // check if consecutive range of (non)zero samples ends
                if (isZero != wasZero)
                {
                    wasZero = isZero;
                    ARASamplePosition index = blockStartIndex + i;
                    if (isZero)
                    {
                        // found end of note - construct note
                        double noteStartTime = lastNoteStartIndex / _audioSource->getSampleRate ();
                        double noteDuration = (index - lastNoteStartIndex) / _audioSource->getSampleRate ();

                        // construct note
                        TestAnalysisNote foundNote;
                        foundNote.setFrequency (kARAInvalidFrequency);
                        foundNote.setVolume (1.0f);
                        foundNote.setStartTime (noteStartTime);
                        foundNote.setDuration (noteDuration);
                        foundNotes.push_back (foundNote);
                    }
                    else
                    {
                        // found start of note - store start index
                        lastNoteStartIndex = index;
                    }
                }
            }

            // go to next block and set progress
            // (in the progress calculation, we're scaling by 0.999 to account for the time needed
            // to store the result after this loop has completed)
            blockStartIndex += count;
            float progress = 0.999f * (float) blockStartIndex / (float) _audioSource->getSampleCount ();
            _progress = progress;

            // for testing purposes only, sleep here until dummy analysis time has elapsed -
            // actual plug-ins will process as fast as possible, without arbitrary sleeping
#if ARA_FAKE_NOTE_ANALYSIS_SPEED_FACTOR != 0
            ARATimeDuration analysisTargetTime = analysisStartTime + progress * analysisTargetDuration;
            ARATimeDuration timeToSleep = analysisTargetTime - ARA_GET_CURRENT_TIME ();
            if (timeToSleep > 0.0)
                std::this_thread::sleep_for (std::chrono::milliseconds ((long long) (timeToSleep * 1000000 + 0.5)));
#endif
        }

        if (!wasZero)
        {
            // last note continued until the end of the audio source - construct last note
            double noteStartTime = lastNoteStartIndex / _audioSource->getSampleRate ();
            double noteDuration = (_audioSource->getSampleCount () - lastNoteStartIndex) / _audioSource->getSampleRate ();

            // construct note
            TestAnalysisNote foundNote;
            foundNote.setFrequency (kARAInvalidFrequency);
            foundNote.setVolume (1.0f);
            foundNote.setStartTime (noteStartTime);
            foundNote.setDuration (noteDuration);
            foundNotes.push_back (foundNote);
        }

        // store result
        _analysisResult = new TestAnalysisResult ();
        _analysisResult->setNotes (foundNotes);
        _progress = 1.0f;
    });
}

TestAnalysisTask::~TestAnalysisTask ()
{
    delete _hostAudioReader;
}

bool TestAnalysisTask::isDone () const
{
    return _future.wait_for (std::chrono::milliseconds (0)) == std::future_status::ready;
}

void TestAnalysisTask::cancelSynchronously ()
{
    _shouldCancel = true;
    _future.wait ();
    delete _analysisResult;    // delete here in case our future completed before recognizing the cancel
}

const TestAnalysisResult* TestAnalysisTask::getAnalysisResult ()
{
    ARA_INTERNAL_ASSERT (isDone ());
    return _analysisResult;
}
