#include "juce_ARAModelObjects.h"

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wunused-function")
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4505)
#undef JUCE_VST3HEADERS_INCLUDE_HEADERS_ONLY
#define JUCE_VST3HEADERS_INCLUDE_HEADERS_ONLY 1
#include <juce_audio_processors/format_types/juce_VST3Headers.h>
#include <juce_audio_processors/format_types/juce_VST3Common.h>
JUCE_END_IGNORE_WARNINGS_MSVC
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#if JUCE_MAC || JUCE_IOS
#include <juce_audio_basics/native/juce_mac_CoreAudioLayouts.h>
#endif

namespace juce
{

//==============================================================================

ARADocument::ARADocument (ARADocumentController* documentController)
    : ARA::PlugIn::Document (documentController)
{}

//==============================================================================

ARAMusicalContext::ARAMusicalContext (ARADocument* document, ARA::ARAMusicalContextHostRef hostRef)
    : ARA::PlugIn::MusicalContext (document, hostRef)
{}

//==============================================================================

ARARegionSequence::ARARegionSequence (ARADocument* document, ARA::ARARegionSequenceHostRef hostRef)
    : ARA::PlugIn::RegionSequence (document, hostRef)
{}

Range<double> ARARegionSequence::getTimeRange (bool includeHeadAndTail) const
{
    if (getPlaybackRegions().empty())
        return {};

    auto startTime = std::numeric_limits<double>::max();
    auto endTime = std::numeric_limits<double>::lowest();
    for (const auto& playbackRegion : getPlaybackRegions())
    {
        const auto regionTimeRange = playbackRegion->getTimeRange (includeHeadAndTail);
        startTime = jmin (startTime, regionTimeRange.getStart());
        endTime = jmax (endTime, regionTimeRange.getEnd());
    }
    return { startTime, endTime };
}

double ARARegionSequence::getCommonSampleRate() const
{
    auto commonSampleRate = 0.0;
    for (const auto& playbackRegion : getPlaybackRegions())
    {
        const auto sampleRate = playbackRegion->getAudioModification()->getAudioSource()->getSampleRate();
        if (commonSampleRate == 0.0)
            commonSampleRate = sampleRate;
        if (commonSampleRate != sampleRate)
            return 0.0;
    }
    return commonSampleRate;
}

//==============================================================================

ARAAudioSource::ARAAudioSource (ARADocument* document, ARA::ARAAudioSourceHostRef hostRef)
    : ARA::PlugIn::AudioSource (document, hostRef)
{}

void ARAAudioSource::doUpdateChannelArrangement (const ARA::ChannelArrangement& channelArrangement) noexcept
{
    switch (channelArrangement.getChannelArrangementDataType())
    {
        default:
            jassertfalse;   // fall through to kARAChannelArrangementUndefined here
        case ARA::kARAChannelArrangementUndefined:
            if (getChannelCount() == 1)
                layout = AudioChannelSet::mono();
            else if (getChannelCount() == 2)
                layout = AudioChannelSet::stereo();
            else
                layout = AudioChannelSet::discreteChannels (getChannelCount());
            break;
        case ARA::kARAChannelArrangementVST3SpeakerArrangement:
            layout = getChannelSetForSpeakerArrangement (*static_cast<const Steinberg::Vst::SpeakerArrangement*> (channelArrangement.getChannelArrangement()));
            break;
#if JUCE_MAC || JUCE_IOS
        case ARA::kARAChannelArrangementCoreAudioChannelLayout:
            layout = CoreAudioLayouts::fromCoreAudio (*static_cast<const AudioChannelLayout*> (channelArrangement.getChannelArrangement()));
            break;
#endif
// TODO JUCE_ARA add this when adding ARA AAX support
//        case ARA::kARAChannelArrangementAAXStemFormat:
//            //layout = channelSetFromStemFormat (*static_cast<const AAX_EStemFormat*> (channelArrangement.getChannelArrangement()), false);
//            break;
    }
}

void ARAAudioSource::notifyAnalysisProgressStarted()
{
    getDocumentController()->internalNotifyAudioSourceAnalysisProgressStarted (this);
}

void ARAAudioSource::notifyAnalysisProgressUpdated (float progress)
{
    getDocumentController()->internalNotifyAudioSourceAnalysisProgressUpdated (this, progress);
}

void ARAAudioSource::notifyAnalysisProgressCompleted()
{
    getDocumentController()->internalNotifyAudioSourceAnalysisProgressCompleted (this);
}

void ARAAudioSource::notifyContentChanged (ARAContentUpdateScopes scopeFlags, bool notifyARAHost)
{
    getDocumentController()->internalNotifyAudioSourceContentChanged (this, scopeFlags, notifyARAHost);
}

//==============================================================================

ARAAudioModification::ARAAudioModification (ARAAudioSource* audioSource, ARA::ARAAudioModificationHostRef hostRef, const ARAAudioModification* optionalModificationToClone)
    : ARA::PlugIn::AudioModification (audioSource, hostRef, optionalModificationToClone)
{}

void ARAAudioModification::notifyContentChanged (ARAContentUpdateScopes scopeFlags, bool notifyARAHost)
{
    getDocumentController()->internalNotifyAudioModificationContentChanged (this, scopeFlags, notifyARAHost);
}

//==============================================================================

ARAPlaybackRegion::ARAPlaybackRegion (ARAAudioModification* audioModification, ARA::ARAPlaybackRegionHostRef hostRef)
    : ARA::PlugIn::PlaybackRegion (audioModification, hostRef)
{}

Range<double> ARAPlaybackRegion::getTimeRange (bool includeHeadAndTail) const
{
    auto startTime = getStartInPlaybackTime();
    auto endTime = getEndInPlaybackTime();
    
    if (includeHeadAndTail)
    {
        ARA::ARATimeDuration headTime {}, tailTime {};
        getDocumentController()->getPlaybackRegionHeadAndTailTime (toRef (this), &headTime, &tailTime);
        startTime -= headTime;
        endTime += tailTime;
    }
    
    return { startTime, endTime };
}

Range<int64> ARAPlaybackRegion::getSampleRange (double sampleRate, bool includeHeadAndTail) const
{
    const auto timeRange = getTimeRange (includeHeadAndTail);
    return { ARA::samplePositionAtTime (timeRange.getStart(), sampleRate), ARA::samplePositionAtTime (timeRange.getEnd(), sampleRate) };
}

double ARAPlaybackRegion::getHeadTime() const
{
    ARA::ARATimeDuration headTime {}, tailTime {};
    getDocumentController()->getPlaybackRegionHeadAndTailTime (toRef (this), &headTime, &tailTime);
    return headTime;
}

double ARAPlaybackRegion::getTailTime() const
{
    
    ARA::ARATimeDuration headTime {}, tailTime {};
    getDocumentController()->getPlaybackRegionHeadAndTailTime (toRef (this), &headTime, &tailTime);
    return tailTime;
}

void ARAPlaybackRegion::notifyContentChanged (ARAContentUpdateScopes scopeFlags, bool notifyARAHost)
{
    getDocumentController()->internalNotifyPlaybackRegionContentChanged (this, scopeFlags, notifyARAHost);
}

} // namespace juce
