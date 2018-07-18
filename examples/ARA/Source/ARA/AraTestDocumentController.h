//------------------------------------------------------------------------------
//! \file        AraTestDocumentController.h
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

#pragma once

#include <ARA_Library/PlugIn/AraPlug.h>

#include <future>
#include <atomic>
#include <chrono>

namespace ARA
{
namespace PlugIn
{

class AraTestPlaybackRenderer;

// the test plug-in pretends to be able to do a kARAContentTypeNotes analysis,
// returning a single note with invalid pitch, covering the entire audio source.
// the time consumed by the fake analysis is the duration of the audio source scaled
// down by FAKE_NOTE_ANALYSIS_SPEED_FACTOR.
// if desired, a custom timer can be injected by defining ARA_GET_CURRENT_TIME accordingly.
#if !defined(ARA_GET_CURRENT_TIME)
	#define ARA_GET_CURRENT_TIME() (0.000001 * std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now ().time_since_epoch ()).count ())
#endif

#if !defined(FAKE_NOTE_ANALYSIS_SPEED_FACTOR)
	#define FAKE_NOTE_ANALYSIS_SPEED_FACTOR 20
#endif


/*******************************************************************************/
class NoteContentReader: public ContentReader
{
private:
	NoteContentReader (const AudioSource* audioSource, const ARAContentTimeRange& range, double timeOffset = 0.0);

public:
	explicit NoteContentReader (const AudioSource* audioSource, const ARAContentTimeRange* range = nullptr)
	: NoteContentReader (audioSource, (range) ? *range : ARAContentTimeRange {0.0, audioSource->getDuration ()})
	{}
	explicit NoteContentReader (const AudioModification* audioModification, const ARAContentTimeRange* range = nullptr)
		// actual plug-ins will take the modification data into account instead of simply forwarding to the audio source detection data
		: NoteContentReader (audioModification->getAudioSource (), (range) ? *range : ARAContentTimeRange {0.0, audioModification->getAudioSource ()->getDuration ()})
	{}
	explicit NoteContentReader (const PlaybackRegion* playbackRegion, const ARAContentTimeRange* range = nullptr)
		// actual plug-ins will take the modification data and the full region transformation into account instead of simply forwarding to the audio source detection data
		: NoteContentReader (playbackRegion->getAudioModification ()->getAudioSource (),
		(range) ? *range : ARAContentTimeRange {playbackRegion->getStartInSong (), playbackRegion->getDurationInSong ()},
		playbackRegion->getStartInSong () - playbackRegion->getStartInAudioModification ())
	{}

	virtual ~NoteContentReader ();

	ARAInt32 getEventCount () override;
	const void* getDataForEvent (ARAInt32 eventIndex) override;

private:
	ARAContentNote* _lastExportedNote;
};

/*******************************************************************************/
class TestAudioSourceAnalysisResult : public AudioSourceAnalysisResult
{
public:
	ARASampleRate getSampleRate () const { return _sampleRate; }
	void setSampleRate (ARASampleRate sampleRate) { _sampleRate = sampleRate; }

	ARASampleCount getSampleCount () const { return _sampleCount; }
	void setSampleCount (ARASampleCount sampleCount) { _sampleCount = sampleCount; }

	ARAChannelCount getChannelCount () const { return _channelCount; }
	void setChannelCount (ARAChannelCount channelCount) { _channelCount = channelCount; }

private:
	ARASampleCount _sampleCount;
	ARASampleRate _sampleRate;
	ARAChannelCount _channelCount;
};

/*******************************************************************************/
class AudioSourceAnalysisTask
{
public:
	explicit AudioSourceAnalysisTask (AudioSource* audioSource);

	AudioSource* getAudioSource () const { return _audioSource; }

	float getProgress () const { return _progress; }
	bool isDone () const;
	void cancelSynchronously ();

	bool isInvalidatedByAudioSourcePropertiesUpdate ();

	const TestAudioSourceAnalysisResult* getAnalysisResult ();

private:
	AudioSource* const _audioSource;
	TestAudioSourceAnalysisResult* _analysisResult;
	std::future<void> _future;
	std::atomic<float> _progress;
	std::atomic<bool> _shouldCancel;
};

/*******************************************************************************/
class AraTestDocumentController : public DocumentController
{
public:
	AraTestDocumentController ()
	: DocumentController (),
	  _renderersCanAccessModelGraph (true),
	  _countOfRenderersCurrentlyAccessingModelGraph (0)
	{}

protected:
	// Document Management
	void doNotifyModelUpdates () override;

	virtual void onBeginEditing () override;
	virtual void onEndEditing () override;
	virtual bool doBeginRestoringDocumentFromArchive (HostArchiveReader* archiveReader) override;
	virtual bool doEndRestoringDocumentFromArchive (HostArchiveReader* archiveReader) override;

	// Musical Context Management
	void onMusicalContextContentUpdated (MusicalContext* musicalContext, const ARAContentTimeRange* range, ARAContentUpdateFlags flags) override;

	// Audio Source Management
	void onAudioSourcePropertiesUpdated (AudioSource* audioSource, PropertiesPtr<ARAAudioSourceProperties> properties) override;
	void onAudioSourceContentUpdated (AudioSource* audioSource, const ARAContentTimeRange* range, ARAContentUpdateFlags flags) override;
	void onDestroyAudioSource (AudioSource* audioSource) override;

	// Content Reader Management
	bool doIsAudioSourceContentAvailable (AudioSource* audioSource, ARAContentType type) override;
	bool doIsAudioSourceContentAnalysisIncomplete (AudioSource* audioSource, ARAContentType type) override;
	void doRequestAudioSourceContentAnalysis (AudioSource* audioSource, ARASize contentTypesCount, const ARAContentType* contentTypes) override;
	ARAContentGrade doGetAudioSourceContentGrade (AudioSource* audioSource, ARAContentType type) override;
	ContentReader* doCreateAudioSourceContentReader (AudioSource* audioSource, ARAContentType type, const ARAContentTimeRange* range) override;
	ContentReader* doCreateAudioModificationContentReader (AudioModification* audioModification, ARAContentType type, const ARAContentTimeRange* range) override;
	ContentReader* doCreatePlaybackRegionContentReader (PlaybackRegion* playbackRegion, ARAContentType type, const ARAContentTimeRange* range) override;

	AudioSource* doCreateAudioSource (Document* document, ARAAudioSourceHostRef hostRef) override;
	void doEnableAudioSourceSamplesAccess (AudioSource* audioSource, bool enable) override;

public:
	// render thread synchronization:
	// this is just a test code implementation of handling the threading - proper code will use a
	// more sophisticated threading implementation, which is needed regardless of ARA.
	// the test code simply blocks renderer access to the model while it is being modified.
	// this includes waiting until concurrent renderer model access has completed before starting modifications.
	bool onRendererBeginsAccessingModelGraph (AraTestPlaybackRenderer* playbackRenderer);
	void onRendererEndsAccessingModelGraph (AraTestPlaybackRenderer* playbackRenderer);

private:
	void _disableRendererModelGraphAccess ();
	void _enableRendererModelGraphAccess ();

	void _processCompletedAnalysisTasks ();
	void _restartAnalysisForAudioSource (AudioSource* audioSource);
	void _cancelAnalysisForAudioSource (AudioSource* audioSource);

private:
	std::vector<AudioSource*> _audioSourcesToNotifyStart;
	std::vector<AudioSource*> _audioSourcesToNotifyComplete;
	std::vector<AudioSourceAnalysisTask*> _activeAnalysisTasks;

	std::atomic<bool> _renderersCanAccessModelGraph;
	std::atomic<unsigned int> _countOfRenderersCurrentlyAccessingModelGraph;
};

}	// namespace PlugIn
}	// namespace Ara
