//------------------------------------------------------------------------------
//! \file        ARATestDocumentController.h
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

#pragma once

#include "ARA_Library/PlugIn/ARAPlug.h"

#include <atomic>
#include <unordered_set>
#include <vector>

class TestAnalysisTask;

namespace ARA
{
namespace PlugIn
{

class ARATestAudioSource;
class ARATestPlaybackRenderer;

/*******************************************************************************/
class NoteContentReader: public ContentReader
{
public:
	explicit NoteContentReader (const AudioSource* audioSource, const ARAContentTimeRange* range = nullptr);
	explicit NoteContentReader (const AudioModification* audioModification, const ARAContentTimeRange* range = nullptr);
	explicit NoteContentReader (const PlaybackRegion* playbackRegion, const ARAContentTimeRange* range = nullptr);

	ARAInt32 getEventCount () override;
	const void* getDataForEvent (ARAInt32 eventIndex) override;

private:
	NoteContentReader (const AudioSource* audioSource, const ARAContentTimeRange& range, double timeOffset = 0.0);

private:
	std::vector<ARAContentNote> _exportedNotes;
};

/*******************************************************************************/
class ARATestDocumentController : public DocumentController
{
public:
	ARATestDocumentController ()
	: DocumentController (),
	  _renderersCanAccessModelGraph (true),
	  _countOfRenderersCurrentlyAccessingModelGraph (0)
	{}

protected:
	// Document Management
	void doNotifyModelUpdates () override;

	virtual void doBeginEditing () override;
	virtual void doEndEditing () override;

	virtual bool doRestoreObjectsFromArchive (HostArchiveReader* archiveReader, RestoreObjectsFilter* filter) override;
	virtual bool doStoreObjectsToArchive (HostArchiveWriter* archiveWriter, StoreObjectsFilter* filter) override;

	// Musical Context Management
	virtual void doUpdateMusicalContextContent (MusicalContext* musicalContext, const ARAContentTimeRange* range, ARAContentUpdateFlags flags) override;

	// Audio Source Management
	virtual AudioSource* doCreateAudioSource (Document* document, ARAAudioSourceHostRef hostRef) override;
	virtual void willUpdateAudioSourceProperties (AudioSource* audioSource, PropertiesPtr<ARAAudioSourceProperties> newProperties) override;
	virtual void doUpdateAudioSourceContent (AudioSource* audioSource, const ARAContentTimeRange* range, ARAContentUpdateFlags flags) override;
	virtual void willEnableAudioSourceSamplesAccess (AudioSource* audioSource, bool enable) override;
	virtual void didEnableAudioSourceSamplesAccess (AudioSource* audioSource, bool enable) override;
	virtual void doDeactivateAudioSourceForUndoHistory (AudioSource* audioSource, bool deactivate) override;
	virtual void willDestroyAudioSource (AudioSource* audioSource) override;

	// Content Reader Management
	virtual bool doIsAudioSourceContentAvailable (AudioSource* audioSource, ARAContentType type) override;
	virtual bool doIsAudioSourceContentAnalysisIncomplete (AudioSource* audioSource, ARAContentType type) override;
	virtual void doRequestAudioSourceContentAnalysisWithAlgorithm (AudioSource* audioSource, std::vector<ARAContentType> const& contentTypes, ARAInt32 analysisAlgorithmIndex) override;
	virtual ARAContentGrade doGetAudioSourceContentGrade (AudioSource* audioSource, ARAContentType type) override;
	virtual ContentReader* doCreateAudioSourceContentReader (AudioSource* audioSource, ARAContentType type, const ARAContentTimeRange* range) override;
	virtual ContentReader* doCreateAudioModificationContentReader (AudioModification* audioModification, ARAContentType type, const ARAContentTimeRange* range) override;
	virtual ContentReader* doCreatePlaybackRegionContentReader (PlaybackRegion* playbackRegion, ARAContentType type, const ARAContentTimeRange* range) override;

public:
	// render thread synchronization:
	// this is just a test code implementation of handling the threading - proper code will use a
	// more sophisticated threading implementation, which is needed regardless of ARA.
	// the test code simply blocks renderer access to the model while it is being modified.
	// this includes waiting until concurrent renderer model access has completed before starting modifications.
	bool rendererWillAccessModelGraph (ARATestPlaybackRenderer* playbackRenderer);
	void rendererDidAccessModelGraph (ARATestPlaybackRenderer* playbackRenderer);

private:
	void _disableRendererModelGraphAccess ();
	void _enableRendererModelGraphAccess ();

	void _startOrScheduleAnalysisOfAudioSource (ARATestAudioSource* audioSource);
	void _startAnalysisOfAudioSource (ARATestAudioSource* audioSource);
	void _processCompletedAnalysisTasks ();
	TestAnalysisTask* _getActiveAnalysisTaskForAudioSource (ARATestAudioSource* audioSource);	// returns nullptr if no active analysis for given audio source

private:
	std::unordered_set<ARATestAudioSource*> _audioSourcesScheduledForAnalysis;
	std::unordered_set<ARATestAudioSource*> _audioSourcesToNotifyContentChanged;
	std::vector<ARATestAudioSource*> _audioSourcesToNotifyAnalysisStart;
	std::vector<ARATestAudioSource*> _audioSourcesToNotifyAnalysisCompletion;
	std::vector<TestAnalysisTask*> _activeAnalysisTasks;

	std::atomic<bool> _renderersCanAccessModelGraph;
	std::atomic<unsigned int> _countOfRenderersCurrentlyAccessingModelGraph;
};

}	// namespace PlugIn
}	// namespace ARA
