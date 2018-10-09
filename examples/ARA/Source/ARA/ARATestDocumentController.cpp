//------------------------------------------------------------------------------
//! \file        ARATestDocumentController.cpp
//! \description document controller implementation for the ARA sample plug-in
//!              customizes the document controller and related classes of the ARA library
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

#include "ARATestDocumentController.h"
#include "ARATestAudioSource.h"
#include "ARATestPlaybackRenderer.h"
#include "TestAnalysis.h"
#include "TestPersistency.h"

#include <map>


// by default, the test plug-in only analyzes audio sources when explicitly requested by the host.
// the define below allows to always trigger audio source analysis when a new audio source instance
// is created, which allows for testing analysis and related notifications in hosts that never
// request audio source analysis.
#if !defined (ARA_ALWAYS_PERFORM_ANALYSIS)
    #define ARA_ALWAYS_PERFORM_ANALYSIS 0
#endif


namespace ARA
{
namespace PlugIn
{

/*******************************************************************************/

NoteContentReader::NoteContentReader (const AudioSource* audioSource, const ARAContentTimeRange* range)
: NoteContentReader (audioSource, (range) ? *range : ARAContentTimeRange {0.0, audioSource->getDuration ()})
{}

NoteContentReader::NoteContentReader (const AudioModification* audioModification, const ARAContentTimeRange* range)
    // actual plug-ins will take the modification data into account instead of simply forwarding to the audio source detection data
    : NoteContentReader (audioModification->getAudioSource (), (range) ? *range : ARAContentTimeRange {0.0, audioModification->getAudioSource ()->getDuration ()})
{}

NoteContentReader::NoteContentReader (const PlaybackRegion* playbackRegion, const ARAContentTimeRange* range)
    // actual plug-ins will take the modification data and the full region transformation into account instead of simply forwarding to the audio source detection data
    : NoteContentReader (playbackRegion->getAudioModification ()->getAudioSource (),
    (range) ? *range : ARAContentTimeRange {playbackRegion->getStartInPlaybackTime (), playbackRegion->getDurationInPlaybackTime ()},
    playbackRegion->getStartInPlaybackTime () - playbackRegion->getStartInAudioModificationTime ())
{}

NoteContentReader::NoteContentReader (const AudioSource* audioSource, const ARAContentTimeRange& range, double timeOffset)
{
    //! \todo is there any elegant way to avoid all those up-casts from AudioSource* to ARATestAudioSource* in this file?
    ARATestAudioSource* testAudioSource = (ARATestAudioSource*)audioSource;
    ARA_INTERNAL_ASSERT (testAudioSource->getAnalysisResult ());
    for (auto note : testAudioSource->getAnalysisResult ()->getNotes ())
    {
        if ((range.start - timeOffset < note.getStartTime () + note.getDuration ()) &&
            (range.start + range.duration - timeOffset >= note.getStartTime ()))
        {
            ARAContentNote adjustedNote;
            adjustedNote.frequency = note.getFrequency ();
            adjustedNote.pitchNumber = kARAInvalidPitchNumber;
            adjustedNote.volume = note.getVolume ();
            adjustedNote.startPosition = note.getStartTime () + timeOffset;
            adjustedNote.attackDuration = 0.0;
            adjustedNote.noteDuration = note.getDuration ();
            adjustedNote.signalDuration = note.getDuration ();
            _exportedNotes.push_back (adjustedNote);
        }
    }
}

ARAInt32 NoteContentReader::getEventCount ()
{
    return (ARAInt32)_exportedNotes.size ();
}

const void* NoteContentReader::getDataForEvent (ARAInt32 eventIndex)
{
    return &_exportedNotes[eventIndex];
}

/*******************************************************************************/

void ARATestDocumentController::_startOrScheduleAnalysisOfAudioSource (ARATestAudioSource* audioSource)
{
    // test if already analyzing
    if (_getActiveAnalysisTaskForAudioSource (audioSource) != nullptr)
        return;

    // postpone if host is currently editing or access is not enabled yet, otherwise start immediately
    if (isHostEditingDocument () || !audioSource->isSampleAccessEnabled ())
        _audioSourcesScheduledForAnalysis.insert (audioSource);
    else
        _startAnalysisOfAudioSource (audioSource);
}

void ARATestDocumentController::_startAnalysisOfAudioSource (ARATestAudioSource* audioSource)
{
    ARA_INTERNAL_ASSERT (audioSource->isSampleAccessEnabled ());

    if (getHostInstance ()->getModelUpdateController ())
        _audioSourcesToNotifyAnalysisStart.push_back (audioSource);

    _activeAnalysisTasks.push_back (new TestAnalysisTask (audioSource));
}

void ARATestDocumentController::_processCompletedAnalysisTasks ()
{
    // in an actual implementation, this would be done on the main thread triggered by a conditon
    // whenever an analysisTask completes on another thread.
    // in this dummy implementation, we rely upon the host polling model updates or analysis completion

    auto it = _activeAnalysisTasks.begin ();
    while (it != _activeAnalysisTasks.end ())
    {
        TestAnalysisTask* analysisTask = *it;
        if (!analysisTask->isDone ())
        {
            ++it;
            continue;
        }

        if (const TestAnalysisResult* result = analysisTask->getAnalysisResult ())
        {
            analysisTask->getAudioSource ()->setAnalysisResult (result);
            if (getHostInstance ()->getModelUpdateController ())
                _audioSourcesToNotifyContentChanged.insert (analysisTask->getAudioSource ());
        }

        if (getHostInstance ()->getModelUpdateController ())
            _audioSourcesToNotifyAnalysisCompletion.push_back (analysisTask->getAudioSource ());

        delete analysisTask;
        it = _activeAnalysisTasks.erase (it);
    }
}

TestAnalysisTask* ARATestDocumentController::_getActiveAnalysisTaskForAudioSource (ARATestAudioSource* audioSource)
{
    for (auto analysisTask : _activeAnalysisTasks)
    {
        if (analysisTask->getAudioSource () == audioSource)
            return analysisTask;
    }
    return nullptr;
}

/*******************************************************************************/

void ARATestDocumentController::doNotifyModelUpdates ()
{
    _processCompletedAnalysisTasks ();

    ModelUpdateController* modelUpdateController = getHostInstance ()->getModelUpdateController ();
    if (!modelUpdateController )
        return;

    // report analysis progress
    for (auto audioSource : _audioSourcesToNotifyAnalysisStart)
        modelUpdateController->notifyAudioSourceAnalysisProgress (audioSource->getHostRef (), kARAAnalysisProgressStarted, 0.0f);
    _audioSourcesToNotifyAnalysisStart.clear ();

    for (auto analysisTask : _activeAnalysisTasks)
        modelUpdateController->notifyAudioSourceAnalysisProgress (analysisTask->getAudioSource ()->getHostRef (), kARAAnalysisProgressUpdated, analysisTask->getProgress ());

    for (auto audioSource : _audioSourcesToNotifyAnalysisCompletion)
        modelUpdateController->notifyAudioSourceAnalysisProgress (audioSource->getHostRef (), kARAAnalysisProgressCompleted, 1.0f);
    _audioSourcesToNotifyAnalysisCompletion.clear ();

    // report content changed
    for (auto audioSource : _audioSourcesToNotifyContentChanged)
    {
        modelUpdateController->notifyAudioSourceContentChanged (audioSource->getHostRef (), nullptr, kARAContentUpdateEverythingChanged);
        // in an actual plug-in, changing the analysis will typically change the modification data
        // as well, so we notify for all modifications of the affected source here
        for (auto audioModification : audioSource->getAudioModifications ())
        {
            modelUpdateController->notifyAudioModificationContentChanged (audioModification->getHostRef (), nullptr, kARAContentUpdateEverythingChanged);

            // in an actual plug-in, changing the modification data will typically change the region data
            // as well, so we notify for all regions of the affected modification here
            for (auto playbackRegion : audioModification->getPlaybackRegions ())
                modelUpdateController->notifyPlaybackRegionContentChanged (playbackRegion->getHostRef (), nullptr, kARAContentUpdateEverythingChanged);
        }
    }
    _audioSourcesToNotifyContentChanged.clear ();
}

void ARATestDocumentController::doBeginEditing ()
{
    _disableRendererModelGraphAccess ();
}

void ARATestDocumentController::doEndEditing ()
{
    _enableRendererModelGraphAccess ();

    auto it = _audioSourcesScheduledForAnalysis.begin ();
    while (it != _audioSourcesScheduledForAnalysis.end ())
    {
        ARATestAudioSource* audioSource = *it;
        if (!audioSource->isSampleAccessEnabled ())
        {
            ++it;
            continue;
        }

        _startAnalysisOfAudioSource (audioSource);
        it = _audioSourcesScheduledForAnalysis.erase (it);
    }
}

/*******************************************************************************/

bool ARATestDocumentController::doRestoreObjectsFromArchive (HostArchiveReader* archiveReader, RestoreObjectsFilter* filter)
{
    // this dummy implementation only deals with audio source states
    if (!filter->shouldRestoreAnyAudioSourceFromArchive ())
        return true;

    // make a map of audio source analysis results keyed by the audio source's persistent ID
    AudioSourceIDMap audioSourcesByID = getAudioSourcesByID ();

    TestUnarchiver unarchiver (archiveReader);

    // start reading data from the archive, starting with the number of audio sources in the archive
    size_t numAudioSources = unarchiver.readSize ();

    // use the persisted audio source ID to find the existing audio source and
    // assign its analysis result using the data stored in the archive
    for (size_t i = 0; i < numAudioSources; ++i)
    {
        float progressVal = float (i) / float (numAudioSources);
        archiveReader->notifyDocumentUnarchivingProgress (progressVal);

        // read persistent ID from archive
        std::string persistentID = unarchiver.readString ();
        if (!unarchiver.didSucceed ())
            break;

        // verify that this audio source should be restored in the case of partial persistency
        if (!filter->shouldRestoreAudioSourceFromArchive (persistentID.c_str ()))
            continue;

        // update persistent ID if host overrides ID pointer
        const char* mappedID = archiveReader->overrideAudioSourceArchivePersistentID (persistentID.c_str ());
        if (mappedID != persistentID.c_str ())    
            persistentID = mappedID;

        // find matching audio source in model graph
        if (audioSourcesByID.count (persistentID.c_str ()) == 0)
            continue;

        ARATestAudioSource* testAudioSource = (ARATestAudioSource*) audioSourcesByID[persistentID.c_str ()];

        // read analysis result
        size_t numNotes = unarchiver.readSize ();
        std::vector<TestAnalysisNote> persistedNotes (numNotes);
        for (TestAnalysisNote& persistedNote : persistedNotes)
        {
            persistedNote.setFrequency ((float) unarchiver.readDouble ());
            persistedNote.setVolume ((float) unarchiver.readDouble ());
            persistedNote.setStartTime (unarchiver.readDouble ());
            persistedNote.setDuration (unarchiver.readDouble ());
        }

        if (!unarchiver.didSucceed ())
        {
            // on failure, notify content change since the host expects the content to be properly restored
            if (getHostInstance ()->getModelUpdateController ())
                _audioSourcesToNotifyContentChanged.insert (testAudioSource);
            break;
        }

        // abort any currently running or scheduled analysis of the audio source
        if (TestAnalysisTask* analysisTask = _getActiveAnalysisTaskForAudioSource (testAudioSource))
            analysisTask->cancelSynchronously ();
        _audioSourcesScheduledForAnalysis.erase (testAudioSource);

        // save restored result in model
        TestAnalysisResult* analysisResult = new TestAnalysisResult ();
        analysisResult->setNotes (persistedNotes);
        testAudioSource->setAnalysisResult (analysisResult);

        // clear any pending content change notification, since the host expects content to match the restored state
        if (getHostInstance ()->getModelUpdateController ())
            _audioSourcesToNotifyContentChanged.erase (testAudioSource);
    }
    archiveReader->notifyDocumentUnarchivingProgress (1.0f);

    return unarchiver.didSucceed ();
}

bool ARATestDocumentController::doStoreObjectsToArchive (HostArchiveWriter* archiveWriter, StoreObjectsFilter* filter)
{
    // this dummy implementation only deals with audio source states
    std::vector<ARATestAudioSource*> audioSourcesToPersist;

    if (filter->shouldStoreAnyAudioSourceToArchive ())
    {
        // make sure to capture any pending analysis result
        _processCompletedAnalysisTasks ();

        // collect all audio sources with actual data (i.e audio source analysis results) to store
        audioSourcesToPersist.reserve (getDocument ()->getAudioSources ().size ());
        for (AudioSource* audioSource : getDocument ()->getAudioSources ())
        {
            // Apply filter
            if (!filter->shouldStoreAudioSourceToArchive (audioSource))
                continue;

            ARATestAudioSource* testAudioSource = (ARATestAudioSource*)audioSource;
            if (testAudioSource->getAnalysisResult ())
                audioSourcesToPersist.push_back (testAudioSource);
        }
    }

    // create archiver
    TestArchiver archiver (archiveWriter);

    // start writing data, beginning with the number of audio sources we are persisting
    ARASize numAudioSources = audioSourcesToPersist.size ();
    archiver.writeSize (numAudioSources);

    // for each audio source to persist, persist its ID followed by its analysis result
    for (ARASize i = 0; i < numAudioSources; ++i)
    {
        float progressVal = float (i) / float (numAudioSources);
        archiveWriter->notifyDocumentArchivingProgress (progressVal);

        // write persistent ID
        archiver.writeString (audioSourcesToPersist[i]->getPersistentID ());

        // write analysis result
        const TestAnalysisResult* analysisResult = audioSourcesToPersist[i]->getAnalysisResult ();

        size_t numNotes = analysisResult->getNotes ().size ();
        archiver.writeSize (numNotes);
        for (auto noteToPersist : analysisResult->getNotes ())
        {
            archiver.writeDouble (noteToPersist.getFrequency ());
            archiver.writeDouble (noteToPersist.getVolume ());
            archiver.writeDouble (noteToPersist.getStartTime ());
            archiver.writeDouble (noteToPersist.getDuration ());
        }
    }
    archiveWriter->notifyDocumentArchivingProgress (1.0f);

    return archiver.didSucceed ();
}

/*******************************************************************************/

void ARATestDocumentController::doUpdateMusicalContextContent (MusicalContext* musicalContext, const ARAContentTimeRange* range, ARAContentUpdateFlags flags)
{
#if ARA_ENABLE_DEBUG_OUTPUT
    ARA_LOG ("musical context updated");
    if ((flags & kARAContentUpdateTimingScopeRemainsUnchanged) == 0)
    {
        HostContentReader<kARAContentTypeTempoEntries> tempoReader (musicalContext);
        if (tempoReader)
        {
            ARA_LOG ("tempo map with grade %i:", tempoReader.getGrade ());
            for (ARAInt32 i = 0; i < tempoReader.getEventCount (); ++i)
            {
                const ARAContentTempoEntry * entry = tempoReader.getDataForEvent (i);
                ARA_LOG ("quarter %.3f is at second %.3f", entry->quarterPosition, entry->timePosition);
            }
        }
        else
        {
            ARA_LOG ("no tempo map provided");
        }
    }
#endif
}

/*******************************************************************************/

AudioSource* ARATestDocumentController::doCreateAudioSource (Document* document, ARAAudioSourceHostRef hostRef)
{
#if ARA_ALWAYS_PERFORM_ANALYSIS
    ARATestAudioSource* testAudioSource = new ARATestAudioSource (document, hostRef);
    _startOrScheduleAnalysisOfAudioSource (testAudioSource);
    return testAudioSource;
#else
    return new ARATestAudioSource (document, hostRef);
#endif
}

void ARATestDocumentController::willUpdateAudioSourceProperties (AudioSource* audioSource, PropertiesPtr<ARAAudioSourceProperties> newProperties)
{
    if ((audioSource->getSampleRate () != newProperties->sampleRate) ||
        (audioSource->getSampleCount () != newProperties->sampleCount) ||
        (audioSource->getChannelCount () != newProperties->channelCount))
    {
        // no need to trigger updateRenderSampleCache () here, since host is required to
        // disable sample access when changing channel or sample count, which will always update the cache.
        // any potential analysis of the audio source also would have be cancelled already when disabling acces.

        ARATestAudioSource* testAudioSource = (ARATestAudioSource*)audioSource;
        if (const TestAnalysisResult* analysisResult = testAudioSource->getAnalysisResult ())
        {
            // actual plug-ins may be able to create a new result based on the old one, but for
            // test code purposes we simply discard the old result and request a new analysis
            testAudioSource->setAnalysisResult (nullptr);
            if (getHostInstance ()->getModelUpdateController ())
                _audioSourcesToNotifyContentChanged.insert (testAudioSource);

            _startOrScheduleAnalysisOfAudioSource (testAudioSource);
        }
    }
}

void ARATestDocumentController::doUpdateAudioSourceContent (AudioSource* audioSource, const ARAContentTimeRange* range, ARAContentUpdateFlags flags)
{
    if ((flags && kARAContentUpdateSignalScopeRemainsUnchanged) == 0)
    {
        ARATestAudioSource* testAudioSource = (ARATestAudioSource*)audioSource;

        if (testAudioSource->isSampleAccessEnabled ())
            testAudioSource->updateRenderSampleCache ();

        // if modifying sample data of the given audio source while analyzing,
        // we'll abort and restart the analysis in onEndEditing ()
        if (TestAnalysisTask* analysisTask = _getActiveAnalysisTaskForAudioSource (testAudioSource))
        {
            analysisTask->cancelSynchronously ();
            _startOrScheduleAnalysisOfAudioSource (testAudioSource);
        }

        if (const TestAnalysisResult* analysisResult = testAudioSource->getAnalysisResult ())
        {
            // actual plug-ins may be able to create a new result based on the old one, but for
            // test code purposes we simply discard the old result and request a new analysis
            testAudioSource->setAnalysisResult (nullptr);
            if (getHostInstance ()->getModelUpdateController ())
                _audioSourcesToNotifyContentChanged.insert (testAudioSource);

            _startOrScheduleAnalysisOfAudioSource (testAudioSource);
        }
    }
}

void ARATestDocumentController::willEnableAudioSourceSamplesAccess (AudioSource* audioSource, bool enable)
{
    // if disabling access to the given audio source while analyzing,
    // we'll abort and restart the analysis when re-enabling access
    if (!enable)
    {
        ARATestAudioSource* testAudioSource = (ARATestAudioSource*)audioSource;
        if (TestAnalysisTask* analysisTask = _getActiveAnalysisTaskForAudioSource (testAudioSource))
        {
            analysisTask->cancelSynchronously ();
            _audioSourcesScheduledForAnalysis.insert (testAudioSource);
        }
    }

    // make sure renderers will not access the audio source while its state changes -
    // if being edited, renderers have already been disabled, otherwise do so now.
    if (!isHostEditingDocument ())
        _disableRendererModelGraphAccess ();
}

void ARATestDocumentController::didEnableAudioSourceSamplesAccess (AudioSource* audioSource, bool enable)
{
    ARATestAudioSource* testAudioSource = (ARATestAudioSource*)audioSource;

    if (enable)
        testAudioSource->updateRenderSampleCache ();

    if (!isHostEditingDocument ())
        _enableRendererModelGraphAccess ();

    // if enabling access, restart any pending analysis if host is not currently editing
    // the document (otherwise done in onEndEditing ())
    if (!isHostEditingDocument () && enable)
    {
        if (_audioSourcesScheduledForAnalysis.count (testAudioSource) != 0)
        {
            _startAnalysisOfAudioSource (testAudioSource);
            _audioSourcesScheduledForAnalysis.erase (testAudioSource);
        }
    }
}

void ARATestDocumentController::doDeactivateAudioSourceForUndoHistory (AudioSource* audioSource, bool deactivate)
{
    ARATestAudioSource* testAudioSource = (ARATestAudioSource*)audioSource;
    if (deactivate)
    {
        testAudioSource->destroyRenderSampleCache ();
    }
    else
    {
        if (testAudioSource->isSampleAccessEnabled ())
            testAudioSource->updateRenderSampleCache ();
    }
}

void ARATestDocumentController::willDestroyAudioSource (AudioSource* audioSource)
{
    ARATestAudioSource* testAudioSource = (ARATestAudioSource*)audioSource;

    if (TestAnalysisTask* analysisTask = _getActiveAnalysisTaskForAudioSource (testAudioSource))
        analysisTask->cancelSynchronously ();
    _processCompletedAnalysisTasks ();    // flush any pending analysis results for the audio source

    _audioSourcesScheduledForAnalysis.erase (testAudioSource);
    _audioSourcesToNotifyContentChanged.erase (testAudioSource);
    erase_remove (_audioSourcesToNotifyAnalysisStart, testAudioSource);
    erase_remove (_audioSourcesToNotifyAnalysisCompletion, testAudioSource);
}

/*******************************************************************************/

bool ARATestDocumentController::doIsAudioSourceContentAvailable (AudioSource* audioSource, ARAContentType type)
{
    if (type == kARAContentTypeNotes)
    {
        _processCompletedAnalysisTasks ();

        return (((ARATestAudioSource*)audioSource)->getAnalysisResult () != nullptr);
    }

    return false;
}

bool ARATestDocumentController::doIsAudioSourceContentAnalysisIncomplete (AudioSource* audioSource, ARAContentType type)
{
    ARA_INTERNAL_ASSERT (type == kARAContentTypeNotes);

    _processCompletedAnalysisTasks ();

    return (_getActiveAnalysisTaskForAudioSource ((ARATestAudioSource*)audioSource) != nullptr);
}

void ARATestDocumentController::doRequestAudioSourceContentAnalysisWithAlgorithm (AudioSource* audioSource, std::vector<ARAContentType> const& contentTypes, ARAInt32 analysisAlgorithmIndex)
{
    ARA_INTERNAL_ASSERT (contentTypes.size () == 1);
    ARA_INTERNAL_ASSERT (contentTypes[0] == kARAContentTypeNotes);

    _processCompletedAnalysisTasks ();

    ARATestAudioSource* testAudioSource = (ARATestAudioSource*)audioSource;
    if (!testAudioSource->getAnalysisResult ())
        _startOrScheduleAnalysisOfAudioSource (testAudioSource);
}

ARAContentGrade ARATestDocumentController::doGetAudioSourceContentGrade (AudioSource* audioSource, ARAContentType type)
{
    if (doIsAudioSourceContentAvailable (audioSource, type))
        return kARAContentGradeDetected;
    return kARAContentGradeInitial;
}

ContentReader* ARATestDocumentController::doCreateAudioSourceContentReader (AudioSource* audioSource, ARAContentType type, const ARAContentTimeRange* range)
{
    if (type == kARAContentTypeNotes)
        return new NoteContentReader (audioSource, range);
    return nullptr;
}

ContentReader* ARATestDocumentController::doCreateAudioModificationContentReader (AudioModification* audioModification, ARAContentType type, const ARAContentTimeRange* range)
{
    if (type == kARAContentTypeNotes)
        return new NoteContentReader (audioModification, range);
    return nullptr;
}

ContentReader* ARATestDocumentController::doCreatePlaybackRegionContentReader (PlaybackRegion* playbackRegion, ARAContentType type, const ARAContentTimeRange* range)
{
    if (type == kARAContentTypeNotes)
        return new NoteContentReader (playbackRegion, range);
    return nullptr;
}

PlaybackRenderer* ARATestDocumentController::doCreatePlaybackRenderer ()
{
    return new ARATestPlaybackRenderer (this);
}

/*******************************************************************************/

bool ARATestDocumentController::rendererWillAccessModelGraph (ARATestPlaybackRenderer* playbackRenderer)
{
    if (!_renderersCanAccessModelGraph)
        return false;

    ++_countOfRenderersCurrentlyAccessingModelGraph;
    return true;
}

void ARATestDocumentController::rendererDidAccessModelGraph (ARATestPlaybackRenderer* playbackRenderer)
{
    ARA_INTERNAL_ASSERT (_countOfRenderersCurrentlyAccessingModelGraph > 0);
    --_countOfRenderersCurrentlyAccessingModelGraph;
}

void ARATestDocumentController::_disableRendererModelGraphAccess ()
{
    ARA_INTERNAL_ASSERT (_renderersCanAccessModelGraph);
    _renderersCanAccessModelGraph = false;

    while (_countOfRenderersCurrentlyAccessingModelGraph)
        {};    // spin until all concurrent renderer calls have completed
}

void ARATestDocumentController::_enableRendererModelGraphAccess ()
{
    ARA_INTERNAL_ASSERT (!_renderersCanAccessModelGraph);
    _renderersCanAccessModelGraph = true;
}

/*******************************************************************************/

DocumentController* DocumentController::doCreateDocumentController ()
{
    return new ARATestDocumentController ();
}

#if !JucePlugin_Enable_ARA
const ARAFactory* DocumentController::getARAFactory ()
{
    static const ARAContentType analyzeableContentTypes[] = { kARAContentTypeNotes };

#if defined (__cpp_template_auto)
    static const SizedStruct<&ARAFactory::supportedPlaybackTransformationFlags> factory =
#else
    static const SizedStruct<ARA_MEMBER_PTR_ARGS (ARAFactory, supportedPlaybackTransformationFlags)> factory =
#endif
    {

#if ARA_SUPPORT_VERSION_1
        kARAAPIGeneration_1_0_Final,
#else
        kARAAPIGeneration_2_0_Draft,
#endif
        kARAAPIGeneration_2_0_Final,
        "com.arademocompany.testplugin.arafactory",
        &ARAInitialize, &ARAUninitialize,
        "ARATestPlugIn", "ARA Demo Company", "http://www.arademocompany.com", "1.0.0",
        &ARACreateDocumentControllerWithDocumentInstance,
        "com.arademocompany.testplugin.aradocumentarchive.version1", (ARASize)0, nullptr,
        sizeof (analyzeableContentTypes)/sizeof (ARAContentType), analyzeableContentTypes,
        kARAPlaybackTransformationNoChanges
    };
    return &factory;
}
#endif

}    // namespace PlugIn
}    // namespace ARA
