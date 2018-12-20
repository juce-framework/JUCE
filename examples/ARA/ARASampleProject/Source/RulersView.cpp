#include "RulersView.h"
#include "ARASampleProjectAudioProcessorEditor.h"

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

ARA::ARAContentBarSignature RulersView::findBarSignatureForQuarter (ARA::ARAQuarterPosition quarterPos) const
{
    const HostBarSignatureReader barSignatureReader (musicalContext);

    // search for the bar signature entry for this quarter using std::upper_bound
    auto itBarSig = std::upper_bound (barSignatureReader.begin (), barSignatureReader.end (), quarterPos,
                                      [] (ARA::ARAQuarterPosition quarterPosition, ARA::ARAContentBarSignature barSignature)
    {
        return quarterPosition < barSignature.position;
    });
    if (itBarSig != barSignatureReader.begin ())
        --itBarSig;

    return *itBarSig;
}

double RulersView::findTempoForTime (double timeInSeconds, ARA::ARAContentTempoEntry& leftEntry, ARA::ARAContentTempoEntry& rightEntry) const
{
    const HostTempoEntryReader tempoReader (musicalContext);

    // user std::upper_bound to find the first tempo entry pair after startTime
    // if it's not the first pair, move one step back to find the pair enclosing startTime
    auto itTempo = std::upper_bound (tempoReader.begin (), std::prev (tempoReader.end ()), timeInSeconds,
                                     [] (ARA::ARATimePosition timePosition, ARA::ARAContentTempoEntry tempoEntry)
    {
        return timePosition < tempoEntry.timePosition;
    });
    if (itTempo != tempoReader.begin ())
        --itTempo;

    // find the initial tempo from the first entry pair
    leftEntry = *itTempo;
    rightEntry = *std::next (itTempo);

    return 60 * (rightEntry.quarterPosition - leftEntry.quarterPosition) / (rightEntry.timePosition - leftEntry.timePosition);
}

ARA::ARAContentBarSignature RulersView::findBarSignatureForTime (double timeInSeconds) const
{
    ARA::ARAContentTempoEntry leftEntry, rightEntry;
    double tempo = findTempoForTime (timeInSeconds, leftEntry, rightEntry);

    ARA::ARAContentBarSignature barSignature;
    if (timeInSeconds < rightEntry.timePosition)
    {
        double secondsSinceLastTempoEntry = timeInSeconds - leftEntry.timePosition;
        double quartersSinceLastTempoEntry = secondsToQuarters (secondsSinceLastTempoEntry, tempo);
        barSignature = findBarSignatureForQuarter (leftEntry.quarterPosition + quartersSinceLastTempoEntry);
    }
    else
    {
        double secondsSinceLastTempoEntry = timeInSeconds - rightEntry.timePosition;
        double quartersSinceLastTempoEntry = secondsToQuarters (secondsSinceLastTempoEntry, tempo);
        barSignature = findBarSignatureForQuarter (rightEntry.quarterPosition + quartersSinceLastTempoEntry);
    }

    return barSignature;
}

double RulersView::findTimeOfBeat (ARA::ARAContentBarSignature barSignature, double beat) const
{
    const HostTempoEntryReader tempoReader (musicalContext);

    // find the tempo map entry for the quarter for this beat
    double quarterForBeat = beatsToQuarters (barSignature, beat);
    auto itTempo = std::upper_bound (tempoReader.begin (), std::prev (tempoReader.end ()), quarterForBeat,
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
    if (rightTempoEntry.quarterPosition < quarterForBeat)
    {
        double quartersFromRight = quarterForBeat - rightTempoEntry.quarterPosition;
        double secondsFromRight = quartersToSeconds (quartersFromRight, tempo);
        return rightTempoEntry.timePosition + secondsFromRight;
    }
    else
    {
        double quartersFromLeft = quarterForBeat - leftTempoEntry.quarterPosition;
        double secondsFromLeft = quartersToSeconds (quartersFromLeft, tempo);
        return leftTempoEntry.timePosition + secondsFromLeft;
    }
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
    double startTime, endTime;
    owner.getVisibleTimeRange (startTime, endTime);

    // seconds ruler: one tick for each second
    {
        RectangleList<int> rects;
        const int lastSecond = roundToInt (floor (endTime));
        for (int nextSecond = roundToInt (ceil (startTime)); nextSecond <= lastSecond; ++nextSecond)
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
        // use our musical context to read tempo and bar signature data using content readers
        HostTempoEntryReader tempoReader (musicalContext);
        HostBarSignatureReader barSignatureReader (musicalContext);

        // we must have at least two tempo entries and a bar signature in order to have a proper timing information
        const int tempoEntryCount = tempoReader.getEventCount();
        const int barSigEventCount = barSignatureReader.getEventCount();
        jassert (tempoEntryCount >= 2 && barSigEventCount >= 1);

        RectangleList<int> rects;

        // find the initial tempo from the first entry pair
        ARA::ARAContentTempoEntry leftTempoEntry, rightTempoEntry;
        double tempo = findTempoForTime (startTime, leftTempoEntry, rightTempoEntry);
        
        // if we're starting before the first entry then use startTime to find the starting quarter
        double quarterStart = 0;
        if (startTime < leftTempoEntry.timePosition)
        {
            quarterStart = startTime * tempo / 60;
        }
        // otherwise use the most recent tempo entry's time in seconds to find where we are in beats
        else
        {
            double secondsFromLeft = startTime - leftTempoEntry.timePosition;
            double quartersFromLeft = secondsFromLeft * tempo / 60;
            quarterStart = leftTempoEntry.quarterPosition + quartersFromLeft;
        }

        // find the starting bar signature and its first beat and use it to find the starting beat
        ARA::ARAContentBarSignature barSignature = findBarSignatureForQuarter (quarterStart);
        int currentBeat = (int) quartersToBeats (barSignature, quarterStart);

        // find the second of the starting beat
        double secondForBeat = findTimeOfBeat (barSignature, currentBeat);

        // count the beats from the most recent bar signature change to find downbeats
        int barSigStartingBeat = (int) (quartersToBeats (barSignature, barSignature.position) + 0.5);

        while (secondForBeat < endTime)
        {
            // draw a tick at each beat (thicker ticks for downbeats)
            int beatsSinceBarSigChange = currentBeat - barSigStartingBeat;
            int tickWidth = ((beatsSinceBarSigChange % barSignature.numerator) == 0) ? heavyLineWidth : lightLineWidth;
            int x = owner.getPlaybackRegionsViewsXForTime (secondForBeat);
            rects.addWithoutMerging (Rectangle<int> (x, beatsRulerY, tickWidth, beatsRulerHeight));

            // Find the second for the next beat and see if we have a new bar signature
            // If so update beat position to match this new signature and its starting beat
            secondForBeat = findTimeOfBeat (barSignature, ++currentBeat);
            ARA::ARAContentBarSignature newBarSignature = findBarSignatureForTime (secondForBeat);
            if (newBarSignature.numerator != barSignature.numerator || 
                newBarSignature.denominator != barSignature.denominator)
            {
                currentBeat = (int) (quartersToBeats (newBarSignature, newBarSignature.position) + 0.5);
                barSigStartingBeat = currentBeat;
                barSignature = newBarSignature;
            }
        }

        g.drawText ("beats", bounds, Justification::centredRight);
        g.fillRectList (rects);
    }

    // TODO JUCE_ARA chord ruler
    {
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
