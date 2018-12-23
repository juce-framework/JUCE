#include "RulersView.h"
#include "ARASampleProjectAudioProcessorEditor.h"
#include "ARA_Library/Utilities/ARAChordAndScaleNames.h"

//==============================================================================
RulersView::RulersView (ARASampleProjectAudioProcessorEditor& owner)
    : owner (owner),
      document (nullptr),
      musicalContext (nullptr)
{
    if (owner.isARAEditorView())
    {
        document = owner.getARADocumentController()->getDocument<ARADocument>();
        document->addListener (this);
        findMusicalContext();
    }
}

RulersView::~RulersView()
{
    detachFromMusicalContext();
    detachFromDocument();
}

void RulersView::detachFromDocument()
{
    if (document == nullptr)
        return;

    document->removeListener (this);

    document = nullptr;
}

void RulersView::detachFromMusicalContext()
{
    if (musicalContext == nullptr)
        return;

    musicalContext->removeListener (this);

    musicalContext = nullptr;
}

void RulersView::findMusicalContext()
{
    if (! owner.isARAEditorView())
        return;

    // evaluate selection
    ARAMusicalContext* newMusicalContext = nullptr;
    auto viewSelection = owner.getARAEditorView()->getViewSelection();
    if (! viewSelection.getRegionSequences().empty())
        newMusicalContext = viewSelection.getRegionSequences().front()->getMusicalContext<ARAMusicalContext>();
    else if (! viewSelection.getPlaybackRegions().empty())
        newMusicalContext = viewSelection.getPlaybackRegions().front()->getRegionSequence()->getMusicalContext<ARAMusicalContext>();

    // if no context used yet and selection does not yield a new one, use the first musical context in the docment
    if (musicalContext == nullptr && newMusicalContext == nullptr &&
        ! owner.getARADocumentController()->getDocument()->getMusicalContexts().empty())
    {
        newMusicalContext = owner.getARADocumentController()->getDocument()->getMusicalContexts<ARAMusicalContext>().front();
    }

    if (newMusicalContext != musicalContext)
    {
        detachFromMusicalContext();

        musicalContext = newMusicalContext;
        musicalContext->addListener (this);

        repaint();
    }
}

//==============================================================================

// TODO JUCE_ARA these two classes should be moved down to the ARA SDK,
// so that both hosts and plug-ins can use it when converting the ARA data to their internal formats.

// TODO JUCE_ARA the caching may be improved by trying the next entry before searching with upper_bounds

template <typename TempoContentReader>
class TempoConverter
{
public:
    TempoConverter (const TempoContentReader& reader)
    : contentReader (reader), leftEntryCache (reader.begin()), rightEntryCache (std::next (leftEntryCache)) {}

    ARA::ARAQuarterPosition getQuarterForTime (ARA::ARATimePosition timePosition) const
    {
        if ((timePosition < leftEntryCache->timePosition && ! isFirst (leftEntryCache)) ||
            (timePosition >= rightEntryCache->timePosition && ! isLast (rightEntryCache)))
        {
            // find the tempo entry after timePosition (or last entry)
            auto itTempo = std::upper_bound (contentReader.begin(), std::prev (contentReader.end()), timePosition,
                                             [] (ARA::ARATimePosition timePosition, ARA::ARAContentTempoEntry tempoEntry)
            {
                return timePosition < tempoEntry.timePosition;
            });

            // pick left and right entry based on found position
            bool isFirstEntry = (itTempo == contentReader.begin());
            leftEntryCache = isFirstEntry ? itTempo : std::prev (itTempo);
            rightEntryCache = isFirstEntry ? std::next (itTempo) : itTempo;
        }

        // linear interpolation of result
        double quartersPerSecond = (rightEntryCache->quarterPosition - leftEntryCache->quarterPosition) / (rightEntryCache->timePosition - leftEntryCache->timePosition);
        return leftEntryCache->quarterPosition + (timePosition - leftEntryCache->timePosition) * quartersPerSecond;
    }

    double getTimeForQuarter (ARA::ARAQuarterPosition quarterPosition) const
    {
        if ((quarterPosition < leftEntryCache->quarterPosition && ! isFirst (leftEntryCache)) ||
            (quarterPosition >= rightEntryCache->quarterPosition && ! isLast (rightEntryCache)))
        {
            // find the tempo entry after timePosition (or last entry)
            auto itTempo = std::upper_bound (contentReader.begin(), std::prev (contentReader.end()), quarterPosition,
                                             [] (ARA::ARAQuarterPosition quarterPosition, ARA::ARAContentTempoEntry tempoEntry)
            {
                return quarterPosition < tempoEntry.quarterPosition;
            });

            // pick left and right entry based on found position
            bool isFirstEntry = (itTempo == contentReader.begin());
            leftEntryCache = isFirstEntry ? itTempo : std::prev (itTempo);
            rightEntryCache = isFirstEntry ? std::next (itTempo) : itTempo;
        }

        // linear interpolation of result
        double secondsPerQuarter = (rightEntryCache->timePosition - leftEntryCache->timePosition) / (rightEntryCache->quarterPosition - leftEntryCache->quarterPosition);
        return leftEntryCache->timePosition + (quarterPosition - leftEntryCache->quarterPosition) * secondsPerQuarter;
    }

private:
    bool isFirst (const typename TempoContentReader::const_iterator& it) const { return it == contentReader.begin(); }
    bool isLast (const typename TempoContentReader::const_iterator& it) const { return std::next (it) == contentReader.end(); }

    const TempoContentReader& contentReader;
    mutable typename TempoContentReader::const_iterator leftEntryCache, rightEntryCache;
};

// TODO JUCE_ARA add caching just as done in the TempoContentReader, but also cache beatAtSigStart.
//               beatAtSigStart can set to -1 to indicate outdated cache.
template <typename BarSignaturesContentReader>
class BarSignaturesConverter
{
public:
    BarSignaturesConverter (const BarSignaturesContentReader& reader) : contentReader (reader) {}

    const auto getBarSignatureIteratorForQuarter (ARA::ARAQuarterPosition quarterPosition) const
    {
        // search for the bar signature entry just after quarterPosition
        auto itBarSig = std::upper_bound (contentReader.begin(), contentReader.end(), quarterPosition,
                                          [] (ARA::ARAQuarterPosition quarterPosition, ARA::ARAContentBarSignature barSignature)
        {
            return quarterPosition < barSignature.position;
        });

        // move one step back, if we can, to find the bar signature for quarterPos
        if (itBarSig != contentReader.begin())
            --itBarSig;

        return itBarSig;
    }

    double getBeatForQuarter (ARA::ARAQuarterPosition quarterPosition) const
    {
        auto itBarSig = contentReader.begin();
        double beatPosition = 0.0;

        if (itBarSig->position < quarterPosition)
        {
            // use each bar signature entry before quarterPosition to count the # of beats
            auto itNextBarSig = std::next (itBarSig);
            while (itNextBarSig != contentReader.end() &&
                   itNextBarSig->position <= quarterPosition)
            {
                beatPosition += quartersToBeats (*itBarSig, itNextBarSig->position - itBarSig->position);
                itBarSig = itNextBarSig++;
            }
        }

        // the last change in quarter position comes after the latest bar signature change
        beatPosition += quartersToBeats (*itBarSig, quarterPosition - itBarSig->position);
        return beatPosition;
    }

    ARA::ARAQuarterPosition getQuarterForBeat (double beatPosition) const
    {
        auto itBarSig = contentReader.begin();
        double currentSigBeat = 0.0;

        if (0.0 < beatPosition)
        {
            // use each bar signature entry before beatPositon to count the # of beats
            auto itNextBarSig = std::next (itBarSig);
            while (itNextBarSig != contentReader.end())
            {
                double beatsDuration = quartersToBeats (*itBarSig, itNextBarSig->position - itBarSig->position);
                double nextSigBeat = currentSigBeat + beatsDuration;
                if (beatPosition < nextSigBeat)
                    break;
                currentSigBeat = nextSigBeat;
                itBarSig = itNextBarSig++;
            }
        }

        // transform the change in beats to quarters using the time signature at beatPosition
        double quarterForBeat = itBarSig->position + beatsToQuarters (*itBarSig, beatPosition - currentSigBeat);
        return quarterForBeat;
    }

private:
    static double quartersToBeats (const ARA::ARAContentBarSignature& barSignature, ARA::ARAQuarterDuration quarterDuration)
    {
        return barSignature.denominator * quarterDuration / 4.0;
    }

    static double beatsToQuarters (ARA::ARAContentBarSignature barSignature, double beatDuration)
    {
        return 4.0 * beatDuration / barSignature.denominator;
    }

private:
    const BarSignaturesContentReader& contentReader;
};

//==============================================================================
void RulersView::paint (juce::Graphics& g)
{
    const auto bounds = g.getClipBounds();

    g.setColour (Colours::lightslategrey);

    if (musicalContext == nullptr)
    {
        g.setFont (Font (12.0f));
        g.drawText ("No musical context found in ARA document!", bounds, Justification::centred);
        return;
    }

    Range<double> visibleRange = owner.getVisibleTimeRange();

    using TempoContentReader = ARA::PlugIn::HostContentReader<ARA::kARAContentTypeTempoEntries>;
    using BarSignaturesContentReader = ARA::PlugIn::HostContentReader<ARA::kARAContentTypeBarSignatures>;
    using ChordsContentReader = ARA::PlugIn::HostContentReader<ARA::kARAContentTypeSheetChords>;
    const TempoContentReader tempoReader (musicalContext);
    const BarSignaturesContentReader barSignaturesReader (musicalContext);
    const ChordsContentReader chordsReader (musicalContext);

    const TempoConverter<TempoContentReader> tempoConverter (tempoReader);

    // we'll draw three rulers: seconds, beats, and chords
    constexpr int lightLineWidth = 1;
    constexpr int heavyLineWidth = 3;
    const int chordRulerY = 0;
    const int chordRulerHeight = getBounds().getHeight() / 3;
    const int beatsRulerY = chordRulerY + chordRulerHeight;
    const int beatsRulerHeight = (getBounds().getHeight() - chordRulerHeight) / 2;
    const int secondsRulerY = beatsRulerY + beatsRulerHeight;
    const int secondsRulerHeight = getBounds().getHeight() - chordRulerHeight - beatsRulerHeight;

    // seconds ruler: one tick for each second
    {
        RectangleList<int> rects;
        const int endTime = roundToInt (floor (visibleRange.getEnd()));
        for (int time = roundToInt (ceil (visibleRange.getStart())); time <= endTime; ++time)
        {
            const int lineWidth = (time % 60 == 0) ? heavyLineWidth : lightLineWidth;
            const int lineHeight = (time % 10 == 0) ? secondsRulerHeight : secondsRulerHeight / 2;
            const int x = owner.getPlaybackRegionsViewsXForTime (time);
            rects.addWithoutMerging (Rectangle<int> (x - lineWidth / 2, secondsRulerY + secondsRulerHeight - lineHeight, lineWidth, lineHeight));
        }
        g.drawText ("seconds", bounds, Justification::bottomRight);
        g.fillRectList (rects);
    }

    // beat ruler: evaluates tempo and bar signatures to draw a line for each beat
    {
        RectangleList<int> rects;

        const BarSignaturesConverter<BarSignaturesContentReader> barSignaturesConverter (barSignaturesReader);

        double beatStart = barSignaturesConverter.getBeatForQuarter (tempoConverter.getQuarterForTime (visibleRange.getStart()));
        double beatEnd = barSignaturesConverter.getBeatForQuarter (tempoConverter.getQuarterForTime (visibleRange.getEnd()));
        int endBeat = roundToInt (floor (beatEnd));
        for (int beat = roundToInt (ceil (beatStart)); beat <= endBeat; ++beat)
        {
            const auto quarterPos = barSignaturesConverter.getQuarterForBeat (beat);
            const auto timePos = tempoConverter.getTimeForQuarter (quarterPos);

            const auto barSignature = *barSignaturesConverter.getBarSignatureIteratorForQuarter (quarterPos);
            const int barSigBeatStart = roundToInt (barSignaturesConverter.getBeatForQuarter (barSignature.position));
            const int beatsSinceBarSigStart = beat - barSigBeatStart;
            const bool isDownBeat = ((beatsSinceBarSigStart % barSignature.numerator) == 0);

            const int lineWidth = isDownBeat ? heavyLineWidth : lightLineWidth;
            const int x = owner.getPlaybackRegionsViewsXForTime (timePos);
            rects.addWithoutMerging (Rectangle<int> (x, beatsRulerY, lineWidth, beatsRulerHeight));
        }
        g.drawText ("beats", bounds, Justification::centredRight);
        g.fillRectList (rects);
    }

    // chord ruler: one rect per chord, skipping empty "no chords"
    {
        RectangleList<int> rects;

        // a chord is considered "no chord" if its intervals are all zero
        auto isNoChord = [] (ARA::ARAContentChord chord)
        {
            return std::all_of (chord.intervals, chord.intervals + sizeof (chord.intervals), [] (ARA::ARAChordIntervalUsage i) { return i == 0; });
        };

        for (auto itChord = chordsReader.begin(); itChord != chordsReader.end(); ++itChord)
        {
            if (isNoChord (*itChord))
                continue;

            Rectangle<int> chordRect = bounds;
            chordRect.setVerticalRange (Range<int> (chordRulerY, chordRulerY + chordRulerHeight));
            
            // find the starting position of the chord in pixels
            const auto chordStartTime = tempoConverter.getTimeForQuarter (itChord->position);
            if (chordStartTime >= visibleRange.getEnd())
                break;
            chordRect.setLeft (owner.getPlaybackRegionsViewsXForTime (chordStartTime));

            // if we have a chord after this one, use its starting position to end our rect
            if (std::next(itChord) != chordsReader.end())
            {
                const auto nextChordStartTime = tempoConverter.getTimeForQuarter (std::next (itChord)->position);
                if (nextChordStartTime < visibleRange.getStart())
                    continue;
                chordRect.setRight (owner.getPlaybackRegionsViewsXForTime (nextChordStartTime));
            }

            // get the chord name
            String chordName = ARA::getNameForChord (*itChord);

            // use the chord name as a hash to pick a random color
            auto& random = Random::getSystemRandom();
            random.setSeed (chordName.hash() + itChord->bass);
            Colour chordColour ((uint8) random.nextInt (256), (uint8) random.nextInt (256), (uint8) random.nextInt (256));

            // draw chord rect and name
            g.setColour (chordColour);
            g.fillRect (chordRect);
            g.setColour (chordColour.contrasting (1.0f));
            g.setFont (Font (12.0f));
            g.drawText (chordName, chordRect, Justification::centred);
        }

        g.setColour (Colours::lightslategrey);
        g.drawText ("chords", bounds, Justification::topRight);
    }

    // borders
    {
        g.setColour (Colours::darkgrey);
        g.drawLine ((float) bounds.getX(), (float) beatsRulerY, (float) bounds.getRight(), (float) beatsRulerY);
        g.drawLine ((float) bounds.getX(), (float) secondsRulerY, (float) bounds.getRight(), (float) secondsRulerY);
        g.drawRect (bounds);
    }
}

//==============================================================================

void RulersView::mouseDown (const MouseEvent& event)
{
    // use mouse click to set the playhead position in the host (if they provide a playback controller interface)
    auto playbackController = musicalContext->getDocument()->getDocumentController()->getHostInstance()->getPlaybackController();
    if (playbackController != nullptr)
        playbackController->requestSetPlaybackPosition (owner.getPlaybackRegionsViewsTimeForX (roundToInt (event.position.x)));
}

void RulersView::mouseDoubleClick (const MouseEvent& /*event*/)
{
    // use mouse double click to start host playback (if they provide a playback controller interface)
    auto playbackController = musicalContext->getDocument()->getDocumentController()->getHostInstance()->getPlaybackController();
    if (playbackController != nullptr)
        playbackController->requestStartPlayback();
}

//==============================================================================

void RulersView::onNewSelection (const ARA::PlugIn::ViewSelection& /*currentSelection*/)
{
    findMusicalContext();
}

void RulersView::didEndEditing (ARADocument* /*doc*/)
{
    if (musicalContext == nullptr)
        findMusicalContext();
}

void RulersView::willRemoveMusicalContextFromDocument (ARADocument* doc, ARAMusicalContext* context)
{
    jassert (document == doc);

    if (musicalContext == context)
        detachFromMusicalContext();     // will restore in didEndEditing()
}

void RulersView::didReorderMusicalContextsInDocument (ARADocument* doc)
{
    jassert (document == doc);

    if (musicalContext != document->getMusicalContexts().front())
        detachFromMusicalContext();     // will restore in didEndEditing()
}
 void RulersView::willDestroyDocument (ARADocument* doc)
{
    jassert (document == doc);

    detachFromDocument();
}

void RulersView::doUpdateMusicalContextContent (ARAMusicalContext* context, ARAContentUpdateScopes /*scopeFlags*/)
{
    jassert (musicalContext == context);

    repaint();
}
