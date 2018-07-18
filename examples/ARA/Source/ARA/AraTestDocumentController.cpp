//------------------------------------------------------------------------------
//! \file        AraTestDocumentController.cpp
//! \description document controller class for the ARA sample plug-in
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

#include "AraTestDocumentController.h"
#include "AraTestAudioSource.h"
namespace ARA
{
namespace PlugIn
{
/*******************************************************************************/

NoteContentReader::NoteContentReader (const AudioSource* audioSource, const ARAContentTimeRange& range, double timeOffset)
: _lastExportedNote (nullptr)
{
	// since we're assuming a single note, covering the entire audio source
	if ((range.start - timeOffset < audioSource->getDuration ()) &&
		(range.start + range.duration - timeOffset >= 0.0))
	{
		_lastExportedNote = new ARAContentNote;
		_lastExportedNote->frequency = kARAInvalidFrequency;
		_lastExportedNote->pitchNumber = kARAInvalidPitchNumber;
		_lastExportedNote->volume = 1.0f;
		_lastExportedNote->startPosition = 0.0 + timeOffset;
		_lastExportedNote->attackDuration = 0.0;
		_lastExportedNote->noteDuration = audioSource->getDuration();
		_lastExportedNote->signalDuration = audioSource->getDuration();
	}
}

NoteContentReader::~NoteContentReader ()
{
	delete _lastExportedNote;
}

ARAInt32 NoteContentReader::getEventCount ()
{
	return (_lastExportedNote) ? 1 : 0;
}

const void* NoteContentReader::getDataForEvent (ARAInt32 eventIndex)
{
	return _lastExportedNote;
}

/*******************************************************************************/

AudioSourceAnalysisTask::AudioSourceAnalysisTask (AudioSource* audioSource)
: _audioSource (audioSource),
  _analysisResult (nullptr),
  _progress (0),
  _shouldCancel (false)
{
	_future = std::async (std::launch::async, [this] ()
	{
		ARATimePosition analysisStartTime = ARA_GET_CURRENT_TIME ();
		ARATimePosition analysisDuration = _audioSource->getDuration () / FAKE_NOTE_ANALYSIS_SPEED_FACTOR;

		_analysisResult = new TestAudioSourceAnalysisResult ();
		_analysisResult->setSampleRate(_audioSource->getSampleRate());
		_analysisResult->setSampleCount(_audioSource->getSampleCount());
		_analysisResult->setChannelCount(_audioSource->getChannelCount());

		while (!_shouldCancel)
		{
			float progress = (float)(((ARA_GET_CURRENT_TIME () - analysisStartTime) / analysisDuration));
			if (progress > 1.0f)
				progress = 1.0f;

			_progress = progress;
			if (progress >= 1.0f)
				return;

			// actual analysis will do work here and store it in analysisResult, instead of just sleeping
			std::this_thread::sleep_for (std::chrono::milliseconds (50));
		}

		delete _analysisResult;
		_analysisResult = nullptr;
	});
}

bool AudioSourceAnalysisTask::isDone () const
{
	return _future.wait_for (std::chrono::milliseconds (0)) == std::future_status::ready;
}

void AudioSourceAnalysisTask::cancelSynchronously ()
{
	_shouldCancel = true;
	_future.wait ();
}

bool AudioSourceAnalysisTask::isInvalidatedByAudioSourcePropertiesUpdate ()
{
	return ((_audioSource->getSampleRate () != _analysisResult-> getSampleRate ()) ||
			(_audioSource->getSampleCount () != _analysisResult-> getSampleCount ()) ||
			(_audioSource->getChannelCount () != _analysisResult-> getChannelCount ()));
}

const TestAudioSourceAnalysisResult* AudioSourceAnalysisTask::getAnalysisResult ()
{
	ARA_INTERNAL_ASSERT(isDone ());
	return _analysisResult;
}

/*******************************************************************************/

void AraTestDocumentController::_processCompletedAnalysisTasks ()
{
	// in an actual implementation, this would be done on the main thread triggered by a conditon
	// whenever an analysisTask completes on another thread.
	// in this dummy implementation, we rely upon the host polling model updates or analysis completion

	auto it = _activeAnalysisTasks.begin ();
	while (it != _activeAnalysisTasks.end ())
	{
		AudioSourceAnalysisTask* analysisTask = *it;
		if (!analysisTask->isDone ())
		{
			++it;
			continue;
		}

		if (const AudioSourceAnalysisResult* result = analysisTask->getAnalysisResult ())
			analysisTask->getAudioSource ()->setAnalysisResult (result);

		if (getHostInstance ()->getModelUpdateController ())
			_audioSourcesToNotifyComplete.push_back (analysisTask->getAudioSource ());

		delete analysisTask;
		it = _activeAnalysisTasks.erase (it);
	}
}

void AraTestDocumentController::_restartAnalysisForAudioSource (AudioSource* audioSource)
{
	ARA_INTERNAL_ASSERT(audioSource->getAnalysisResult () != nullptr);

	_cancelAnalysisForAudioSource (audioSource);

	audioSource->setAnalysisResult (nullptr);

	doRequestAudioSourceContentAnalysis (audioSource, getAraFactory()->analyzeableContentTypesCount, getAraFactory()->analyzeableContentTypes);
}

void AraTestDocumentController::_cancelAnalysisForAudioSource (AudioSource* audioSource)
{
	for (auto analysisTask : _activeAnalysisTasks)
	{
		if (analysisTask->getAudioSource () == audioSource)
		{
			analysisTask->cancelSynchronously ();
			break;
		}
	}
}

void AraTestDocumentController::doNotifyModelUpdates ()
{
	_processCompletedAnalysisTasks ();

	ModelUpdateController* modelUpdateController = getHostInstance ()->getModelUpdateController ();
	if (!modelUpdateController )
		return;

	for (auto audioSource : _audioSourcesToNotifyStart)
		modelUpdateController->notifyAudioSourceAnalysisProgress (*audioSource, kARAAnalysisProgressStarted, 0.0f);
	_audioSourcesToNotifyStart.clear ();

	for (auto analysisTask : _activeAnalysisTasks)
		modelUpdateController->notifyAudioSourceAnalysisProgress (*analysisTask->getAudioSource (), kARAAnalysisProgressUpdated, analysisTask->getProgress ());

	for (auto audioSource : _audioSourcesToNotifyComplete)
	{
		modelUpdateController->notifyAudioSourceAnalysisProgress (*audioSource, kARAAnalysisProgressCompleted, 1.0f);

		if (audioSource->getAnalysisResult ())
		{
			modelUpdateController->notifyAudioSourceContentChanged (*audioSource, nullptr, kARAContentUpdateEverythingChanged);
			for (auto audioModification : audioSource->getAudioModifications ())
			{
				modelUpdateController->notifyAudioModificationContentChanged (*audioModification, nullptr, kARAContentUpdateEverythingChanged);

				for (auto playbackRegion : audioModification->getPlaybackRegions ())
					modelUpdateController->notifyPlaybackRegionContentChanged (*playbackRegion, nullptr, kARAContentUpdateEverythingChanged);
			}
		}
	}
	_audioSourcesToNotifyComplete.clear ();
}

void AraTestDocumentController::onBeginEditing ()
{
	_disableRendererModelGraphAccess ();
}

void AraTestDocumentController::onEndEditing ()
{
	_enableRendererModelGraphAccess ();
}

bool AraTestDocumentController::doBeginRestoringDocumentFromArchive (HostArchiveReader* archiveReader)
{
	_disableRendererModelGraphAccess ();
	return true;
}

bool AraTestDocumentController::doEndRestoringDocumentFromArchive (HostArchiveReader* archiveReader)
{
	_enableRendererModelGraphAccess ();
	return true;
}

void AraTestDocumentController::onMusicalContextContentUpdated (MusicalContext* musicalContext, const ARAContentTimeRange* range, ARAContentUpdateFlags flags)
{
#if ARA_ENABLE_DEBUG_OUTPUT
	ARA_LOG("musical context updated");
	if ((flags & kARAContentUpdateTimingScopeRemainsUnchanged) == 0)
	{
		HostContentReader<kARAContentTypeTempoEntries> tempoReader (musicalContext);
		if (tempoReader)
		{
			ARA_LOG("tempo map with grade %i:", tempoReader.getGrade ());
			for (ARAInt32 i = 0; i < tempoReader.getEventCount (); ++i)
			{
				const ARAContentTempoEntry * entry = tempoReader.getDataForEvent (i);
				ARA_LOG("quarter %.3f is at second %.3f", entry->quarterPosition, entry->timePosition);
			}
		}
		else
		{
			ARA_LOG("no tempo map provided");
		}
	}
#endif
}

void AraTestDocumentController::onAudioSourcePropertiesUpdated (AudioSource* audioSource, PropertiesPtr<ARAAudioSourceProperties> properties)
{
	for (auto analysisTask : _activeAnalysisTasks)
	{
		if (analysisTask->getAudioSource () == audioSource)
		{
			// if modifying analysis-relevant properites of the given audio source while analyzing,
			// we'll restart the analysis
			if (analysisTask->isInvalidatedByAudioSourcePropertiesUpdate ())
				_restartAnalysisForAudioSource (audioSource);
			break;
		}
	}

	if (const TestAudioSourceAnalysisResult* analysisResult = (TestAudioSourceAnalysisResult*)audioSource->getAnalysisResult())
	{
		if ((audioSource->getSampleRate () != analysisResult-> getSampleRate ()) ||
			(audioSource->getSampleCount () != analysisResult-> getSampleCount ()) ||
			(audioSource->getChannelCount () != analysisResult-> getChannelCount ()))
		{
			// proper plug-ins may be able to create a new result based on the old one, but for
			// test code purposes we simply discard the old result and request a new analysis
			_restartAnalysisForAudioSource (audioSource);
		}
	}
}

void AraTestDocumentController::onAudioSourceContentUpdated (AudioSource* audioSource, const ARAContentTimeRange* range, ARAContentUpdateFlags flags)
{
	if ((flags && kARAContentUpdateSignalScopeRemainsUnchanged) == 0)
	{
		if (audioSource->getAnalysisResult() ||
			doIsAudioSourceContentAnalysisIncomplete (audioSource, getAraFactory()->analyzeableContentTypes[0]))
		{
			// proper plug-ins may be able to create a new result based on the previous analysis by only
			// updateing the changed sample range, but for test code purpose we'll redo a full analysis
			_restartAnalysisForAudioSource (audioSource);
		}
	}
}

void AraTestDocumentController::onDestroyAudioSource (AudioSource* audioSource)
{
	_cancelAnalysisForAudioSource (audioSource);

	_processCompletedAnalysisTasks ();

	erase_remove (_audioSourcesToNotifyStart, audioSource);
	erase_remove (_audioSourcesToNotifyComplete, audioSource);
}

bool AraTestDocumentController::doIsAudioSourceContentAvailable (AudioSource* audioSource, ARAContentType type)
{
	if (type == kARAContentTypeNotes)
	{
		_processCompletedAnalysisTasks ();

		return (audioSource->getAnalysisResult () != nullptr);
	}

	return false;
}

bool AraTestDocumentController::doIsAudioSourceContentAnalysisIncomplete (AudioSource* audioSource, ARAContentType type)
{
	ARA_INTERNAL_ASSERT(type == kARAContentTypeNotes);

	_processCompletedAnalysisTasks ();

	for (auto task : _activeAnalysisTasks)
		if (task->getAudioSource () == audioSource)
			return true;

	return false;
}

void AraTestDocumentController::doRequestAudioSourceContentAnalysis (AudioSource* audioSource, ARASize contentTypesCount, const ARAContentType contentTypes[])
{
	ARA_INTERNAL_ASSERT(contentTypesCount == 1);
	ARA_INTERNAL_ASSERT(contentTypes[0] == kARAContentTypeNotes);

	_processCompletedAnalysisTasks ();

	if (!audioSource->getAnalysisResult ())
	{
		// test if already analyzing
		for (auto task : _activeAnalysisTasks)
			if (task->getAudioSource () == audioSource)
				return;

		if (getHostInstance ()->getModelUpdateController ())
			_audioSourcesToNotifyStart.push_back (audioSource);
		_activeAnalysisTasks.push_back (new AudioSourceAnalysisTask (audioSource));
	}
}

ARAContentGrade AraTestDocumentController::doGetAudioSourceContentGrade (AudioSource* audioSource, ARAContentType type)
{
	if (doIsAudioSourceContentAvailable (audioSource, type))
		return kARAContentGradeDetected;
	return kARAContentGradeInitial;
}

ContentReader* AraTestDocumentController::doCreateAudioSourceContentReader (AudioSource* audioSource, ARAContentType type, const ARAContentTimeRange* range)
{
	if (type == kARAContentTypeNotes)
		return new NoteContentReader (audioSource);
	return nullptr;
}

ContentReader* AraTestDocumentController::doCreateAudioModificationContentReader (AudioModification* audioModification, ARAContentType type, const ARAContentTimeRange* range)
{
	if (type == kARAContentTypeNotes)
		return new NoteContentReader (audioModification);
	return nullptr;
}

ContentReader* AraTestDocumentController::doCreatePlaybackRegionContentReader (PlaybackRegion* playbackRegion, ARAContentType type, const ARAContentTimeRange* range)
{
	if (type == kARAContentTypeNotes)
		return new NoteContentReader (playbackRegion);
	return nullptr;
}

AudioSource* AraTestDocumentController::doCreateAudioSource (Document* document, ARAAudioSourceHostRef hostRef)
{
	return new AraTestAudioSource (document, hostRef);
}

void AraTestDocumentController::doEnableAudioSourceSamplesAccess (AudioSource* audioSource, bool enable)
{
	if (enable)
	{
		// make sure renderers will not access the audio source while its sample buffers are being
		// (re)allocated - if being edited, renderers have already been disabled, otherwise do so now.
		if (getState() == DocumentState::working)
			_disableRendererModelGraphAccess ();
		else
			ARA_INTERNAL_ASSERT(!_renderersCanAccessModelGraph);

		((AraTestAudioSource*)audioSource)->onEnableAudioSampleAccess ();

		if (getState() == DocumentState::working)
			_enableRendererModelGraphAccess ();
	}
}

bool AraTestDocumentController::onRendererBeginsAccessingModelGraph (AraTestPlaybackRenderer* playbackRenderer)
{
	if (!_renderersCanAccessModelGraph)
		return false;

	++_countOfRenderersCurrentlyAccessingModelGraph;
	return true;
}

void AraTestDocumentController::onRendererEndsAccessingModelGraph (AraTestPlaybackRenderer* playbackRenderer)
{
	ARA_INTERNAL_ASSERT(_countOfRenderersCurrentlyAccessingModelGraph > 0);
	--_countOfRenderersCurrentlyAccessingModelGraph;
}

void AraTestDocumentController::_disableRendererModelGraphAccess ()
{
	ARA_INTERNAL_ASSERT(_renderersCanAccessModelGraph);
	_renderersCanAccessModelGraph = false;

	while (_countOfRenderersCurrentlyAccessingModelGraph)
		{};	// spin until all concurrent renderer calls have completed
}

void AraTestDocumentController::_enableRendererModelGraphAccess ()
{
	ARA_INTERNAL_ASSERT(!_renderersCanAccessModelGraph);
	_renderersCanAccessModelGraph = true;
}

/*******************************************************************************/

DocumentController* DocumentController::doCreateDocumentController ()
{
	return new AraTestDocumentController ();
}

}	// namespace PlugIn
}	// namespace Ara
