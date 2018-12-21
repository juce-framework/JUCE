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

// note that these simple helperse won't work in general cases because they 
// aren't examining the entire musical context when performing conversions. 
// Use the RulersView:: member functions below when moving between time spaces
double quartersToSeconds (ARA::ARAQuarterPosition position, double tempo)
{
    return 60. * position / tempo;
}

double secondsToQuarters (ARA::ARATimePosition seconds, double tempo)
{
    return tempo * seconds / 60.;
}

double quartersToBeats (ARA::ARAContentBarSignature barSignature, ARA::ARAQuarterPosition quarterPosition)
{
    return (barSignature.denominator * quarterPosition) / 4.;
}
double beatsToQuarters (ARA::ARAContentBarSignature barSignature, ARA::ARAQuarterPosition beatPosition)
{
    return (4. * beatPosition) / barSignature.denominator;
}

//==============================================================================

ARA::ARAContentBarSignature RulersView::getBarSignatureForQuarter (ARA::ARAQuarterPosition quarterPos) const
{
    // search for the bar signature entry just after this quarter using std::upper_bound
    const HostBarSignatureReader barSignatureReader (musicalContext);
    auto itBarSig = std::upper_bound (barSignatureReader.begin (), barSignatureReader.end (), quarterPos,
                                      [] (ARA::ARAQuarterPosition quarterPosition, ARA::ARAContentBarSignature barSignature)
    {
        return quarterPosition < barSignature.position;
    });

    // move one step back, if we can, to find the bar signature for quarterPos
    if (itBarSig != barSignatureReader.begin ())
        --itBarSig;

    return *itBarSig;
}

double RulersView::getTempoForTime (double timeInSeconds, ARA::ARAContentTempoEntry& leftEntry, ARA::ARAContentTempoEntry& rightEntry) const
{
    // user std::upper_bound to find the first tempo entry pair after timeInSeconds
    const HostTempoEntryReader tempoReader (musicalContext);
    auto itTempo = std::upper_bound (tempoReader.begin (), std::prev (tempoReader.end ()), timeInSeconds,
                                     [] (ARA::ARATimePosition timePosition, ARA::ARAContentTempoEntry tempoEntry)
    {
        return timePosition < tempoEntry.timePosition;
    });

    // if it's not the first pair, move one step back to find the pair enclosing timeInSeconds
    if (itTempo != tempoReader.begin ())
        --itTempo;

    // find the initial tempo from the first entry pair
    leftEntry = *itTempo;
    rightEntry = *std::next (itTempo);

    return 60 * (rightEntry.quarterPosition - leftEntry.quarterPosition) / (rightEntry.timePosition - leftEntry.timePosition);
}

ARA::ARAQuarterPosition RulersView::getQuarterForTime (ARA::ARATimePosition timePosition) const
{
    // find the tempo entries closest to timePosition and use the tempo to find the quarter position
    ARA::ARAContentTempoEntry leftEntry, rightEntry;
    double tempo = getTempoForTime (timePosition, leftEntry, rightEntry);

    double secondsSinceLastTempoEntry = timePosition - leftEntry.timePosition;
    double quartersSinceLastTempoEntry = secondsToQuarters (secondsSinceLastTempoEntry, tempo);
    return leftEntry.quarterPosition + quartersSinceLastTempoEntry;
}

// transform a beat position to a quarter position by examining the host bar signature entries
ARA::ARAQuarterPosition RulersView::getQuarterForBeat (double beatPosition) const
{
    HostBarSignatureReader barSignatureReader (musicalContext);

    auto itBarSig = barSignatureReader.begin ();
    const double firstQuarter = itBarSig->position;
    double currentBeat = 0;

    if (0 < beatPosition)
    {
        // use each bar signature entry before beatPositon to count the # of beats
        for (; std::next (itBarSig) != barSignatureReader.end (); ++itBarSig)
        {
            ARA::ARAQuarterDuration quartersForBarSig = std::next (itBarSig)->position - itBarSig->position;
            double beatsForBarSig = quartersToBeats (*itBarSig, quartersForBarSig);

            // if the starting beat of the next bar signature passes beatPosition exit this loop
            double nextBeat = currentBeat + beatsForBarSig;
            if (beatPosition < nextBeat)
                break;
            currentBeat = nextBeat;
        }
    }

    // transform the change in beats to quarters using the time signature at beatPosition
    double deltaBeats = beatPosition - currentBeat;
    double quarterForBeat = itBarSig->position + beatsToQuarters (*itBarSig, deltaBeats);
    return quarterForBeat;
}

double RulersView::getBeatForQuarter (ARA::ARAQuarterPosition quarterPosition) const
{
    HostBarSignatureReader barSignatureReader (musicalContext);
    auto itBarSig = barSignatureReader.begin ();
    double beatPosition = 0;

    if (itBarSig->position < quarterPosition)
    {
        // use each bar signature entry before quarterPosition to count the # of beats
        for (; std::next (itBarSig) != barSignatureReader.end (); ++itBarSig)
        {
            auto itNextBarSig = std::next (itBarSig);
            if (quarterPosition < itNextBarSig->position)
                break;
            ARA::ARAQuarterDuration deltaQ = itNextBarSig->position - itBarSig->position;
            beatPosition += quartersToBeats (*itBarSig, deltaQ);
        }
    }

    // the last change in quarter position comes after the latest bar signature change
    ARA::ARAQuarterDuration deltaQ = quarterPosition - itBarSig->position;
    beatPosition += quartersToBeats (*itBarSig, deltaQ);
    return beatPosition;
}

double RulersView::getTimeForQuarter (ARA::ARAQuarterPosition quarterPosition) const
{
    const HostTempoEntryReader tempoReader (musicalContext);

    auto itTempo = std::upper_bound (tempoReader.begin (), std::prev (tempoReader.end ()), quarterPosition,
                                     [] (ARA::ARAQuarterPosition beatPosition, ARA::ARAContentTempoEntry tempoEntry)
    {
        return beatPosition < tempoEntry.quarterPosition;
    });
    if (itTempo != tempoReader.begin ())
        --itTempo;

    // find the tempo for this beat and convert to seconds
    ARA::ARAContentTempoEntry leftTempoEntry = *itTempo;
    ARA::ARAContentTempoEntry rightTempoEntry = *std::next (itTempo);
    double tempo = 60 * (rightTempoEntry.quarterPosition - leftTempoEntry.quarterPosition) / (rightTempoEntry.timePosition - leftTempoEntry.timePosition);

    // find difference in quarters from last known quarter position, convert to seconds
    double quartersFromLeft = quarterPosition - leftTempoEntry.quarterPosition;
    double secondsFromLeft = quartersToSeconds (quartersFromLeft, tempo);
    return leftTempoEntry.timePosition + secondsFromLeft;
}

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

    // we'll draw three rulers: seconds, beats, and chords
    constexpr int lightLineWidth = 1;
    constexpr int heavyLineWidth = 3;
    const int chordRulerY = 0;
    const int chordRulerHeight = getBounds().getHeight() / 3;
    const int beatsRulerY = chordRulerY + chordRulerHeight;
    const int beatsRulerHeight = (getBounds().getHeight() - chordRulerHeight) / 2;
    const int secondsRulerY = beatsRulerY + beatsRulerHeight;
    const int secondsRulerHeight = getBounds().getHeight() - chordRulerHeight - beatsRulerHeight;

    // we should only be doing this on the visible time range
    Range<double> visibleRange = owner.getVisibleTimeRange ();

    // seconds ruler: one tick for each second
    {
        RectangleList<int> rects;
        const int lastSecond = roundToInt (floor (visibleRange.getEnd()));
        for (int nextSecond = roundToInt (ceil (visibleRange.getStart())); nextSecond <= lastSecond; ++nextSecond)
        {
            const int lineWidth = (nextSecond % 60 == 0) ? heavyLineWidth : lightLineWidth;
            const int lineHeight = (nextSecond % 10 == 0) ? secondsRulerHeight : secondsRulerHeight / 2;
            const int x = owner.getPlaybackRegionsViewsXForTime (nextSecond);
            rects.addWithoutMerging (Rectangle<int> (x - lineWidth / 2, secondsRulerY + secondsRulerHeight - lineHeight, lineWidth, lineHeight));
        }
        g.drawText ("seconds", bounds, Justification::bottomRight);
        g.fillRectList (rects);
    }

    // beat ruler: evaluates tempo and bar signatures to draw a line for each beat
    {
        RectangleList<int> rects;

        // find the time in beats from the first time signature entry to our starting position
        ARA::ARAQuarterPosition quarterStart = getQuarterForTime (visibleRange.getStart ());
        ARA::ARAQuarterPosition quarterEnd = getQuarterForTime (visibleRange.getEnd ());

        // find the starting beat as well as the next visible whole beat
        double beatStart = getBeatForQuarter (quarterStart);
        int nextWholeBeat = roundToInt (ceil (beatStart));

        // do the same with the last whole beat
        double beatEnd = getBeatForQuarter (quarterEnd);
        int lastWholeBeat = roundToInt (ceil (beatEnd));

        for (int b = nextWholeBeat; b < lastWholeBeat; b++)
        {
            // find the quarter and time position for this beat
            ARA::ARAQuarterPosition quarterPos = getQuarterForBeat (b);
            ARA::ARATimePosition timePos = getTimeForQuarter (quarterPos);

            // update bar signature each beat so we can find downbeats
            ARA::ARAContentBarSignature barSignature = getBarSignatureForQuarter (quarterPos);
            int barSigBeatStart = roundToInt (ceil (getBeatForQuarter (barSignature.position)));
            int beatsSinceBarSigStart = b - barSigBeatStart;
            bool isDownBeat = ((beatsSinceBarSigStart % barSignature.numerator) == 0);

            // add a tick rect, increasing the thickness for downbeats
            int tickWidth = isDownBeat ? heavyLineWidth : lightLineWidth;
            int x = owner.getPlaybackRegionsViewsXForTime (timePos);
            rects.addWithoutMerging (Rectangle<int> (x, beatsRulerY, tickWidth, beatsRulerHeight));
        }
        g.drawText ("beats", bounds, Justification::centredRight);
        g.fillRectList (rects);
    }

    // TODO JUCE_ARA chord ruler
    {
        RectangleList<int> rects;

        // A chord is consiered "blank" if its intervals are all zero
        auto isChordBlank = [] (ARA::ARAContentChord chord)
        {
            return std::all_of (chord.intervals, chord.intervals + sizeof (chord.intervals), [] (ARA::ARAChordIntervalUsage i) { return i == 0; });
        };

        HostChordReader chordReader (musicalContext);

        for (auto itChord = chordReader.begin (); itChord != chordReader.end (); ++itChord)
        {
            if (isChordBlank (*itChord))
                continue;

            Rectangle<int> chordRect = bounds;
            chordRect.setVerticalRange (Range<int> (chordRulerY, chordRulerY + chordRulerHeight));
            
            // find the starting position of the chord in pixels
            ARA::ARATimePosition chordStartSecond = getTimeForQuarter (itChord->position);
            int chordRectStart = owner.getPlaybackRegionsViewsXForTime (chordStartSecond);
            chordRect.setLeft (chordRectStart);

            // get the chord name
            String chordName = ARA::getNameForChord (*itChord);

            // if we have a chord after this one, use its starting position to end our rect
            if (std::next(itChord) != chordReader.end ())
            {
                ARA::ARATimePosition nextChordStartSecond = getTimeForQuarter (std::next (itChord)->position);
                int chordRectEnd = owner.getPlaybackRegionsViewsXForTime (nextChordStartSecond);
                chordRect.setRight (chordRectEnd);
            }

            // use the chord name as a hash to pick a random color
            auto& random = Random::getSystemRandom ();
            random.setSeed (chordName.hash () + itChord->bass);
            Colour chordColour ((uint8) random.nextInt (256), (uint8) random.nextInt (256), (uint8) random.nextInt (256));

            // draw chord rect and name
            g.setColour (chordColour);
            g.fillRect (chordRect);

            g.setColour (chordColour.contrasting (1.0f));
            g.setFont (Font (12.0f));
            g.drawText (chordName, chordRect, Justification::centred);
        }

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

void RulersView::mouseDown (const MouseEvent& event)
{
    // use mouse double click to set the playhead position in the host
    // (if they provide a playback controller interface)
    auto playbackController = musicalContext->getDocument ()->getDocumentController ()->getHostInstance ()->getPlaybackController ();
    if (playbackController != nullptr)
    {
        playbackController->requestSetPlaybackPosition (owner.getPlaybackRegionsViewsTimeForX (roundToInt (event.position.x)));
    }
}

void RulersView::mouseDoubleClick (const MouseEvent& /*event*/)
{
    // use mouse double click to set the playhead position in the host
    // (if they provide a playback controller interface)
    auto playbackController = musicalContext->getDocument ()->getDocumentController ()->getHostInstance ()->getPlaybackController ();
    if (playbackController != nullptr)
    {
        playbackController->requestStartPlayback ();
    }
}
