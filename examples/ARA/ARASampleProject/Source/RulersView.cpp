#include "RulersView.h"
#include "ARASampleProjectAudioProcessorEditor.h"

//==============================================================================
RulersView::RulersView (ARASampleProjectAudioProcessorEditor& owner)
    : owner (owner),
      document (nullptr),
      musicalContext (nullptr)
{
    setColour (borderColourId, Colours::darkgrey);
    setColour (musicalRulerBackgroundColourId, (Colours::green).withAlpha(0.2f));
    setColour (timeRulerBackgroundColourId, (Colours::blue).withAlpha(0.2f));
    setColour (chordsRulerBackgroundColourId, Colours::transparentBlack);
    setColour (musicalGridColourId, Colours::slategrey);
    setColour (timeGridColourId, Colours::slateblue);
    setColour (chordsColourId, Colours::slategrey);

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

// walk through our ARA document to find a suitable musical context
void RulersView::findMusicalContext()
{
    if (! owner.isARAEditorView())
        return;

    auto findMusicalContextLambda = [this] ()
    {
        // first try the first selected region sequence
        auto viewSelection = owner.getARAEditorView()->getViewSelection();
        if (! viewSelection.getRegionSequences().empty())
            return viewSelection.getRegionSequences().front()->getMusicalContext<ARAMusicalContext>();

        // then try the first selected playback region's region sequence
        if (! viewSelection.getPlaybackRegions().empty())
            return viewSelection.getPlaybackRegions().front()->getRegionSequence()->getMusicalContext<ARAMusicalContext>();

        // no selection? if we have an editor renderer try the first region sequence / playback region
        if (auto editorRenderer = owner.getAudioProcessorARAExtension()->getARAEditorRenderer())
        {
            if (! editorRenderer->getRegionSequences().empty())
                return editorRenderer->getRegionSequences().front()->getMusicalContext<ARAMusicalContext>();

            if (! editorRenderer->getPlaybackRegions().empty())
                return editorRenderer->getPlaybackRegions().front()->getRegionSequence()->getMusicalContext<ARAMusicalContext>();
        }

        // otherwise if we're a playback renderer try the first playback region
        if (auto playbackRenderer = owner.getAudioProcessorARAExtension()->getARAPlaybackRenderer())
        {
            if (! playbackRenderer->getPlaybackRegions().empty())
                return playbackRenderer->getPlaybackRegions().front()->getRegionSequence()->getMusicalContext<ARAMusicalContext>();
        }

        // still nothing? try the first musical context in the docment
        if (! owner.getARADocumentController()->getDocument()->getMusicalContexts().empty())
            return owner.getARADocumentController()->getDocument()->getMusicalContexts<ARAMusicalContext>().front();

        return static_cast<ARAMusicalContext*>(nullptr);
    };

    if (auto newMusicalContext = findMusicalContextLambda ())
    {
        if (newMusicalContext != musicalContext)
        {
            detachFromMusicalContext();
            musicalContext = newMusicalContext;
            musicalContext->addListener (this);
        }
    }
}

//==============================================================================
void RulersView::paint (juce::Graphics& g)
{
    const auto bounds = getLocalBounds();
    if (musicalContext == nullptr)
    {
        g.setColour (Colours::darkgrey);
        g.drawRect (bounds, 3);

        g.setColour (Colours::white);
        g.setFont (Font (12.0f));
        g.drawText ("No musical context found in ARA document!", bounds, Justification::centred);
        
        return;
    }

    // we'll draw three rulers: seconds, beats, and chords
    int rulerHeight = bounds.getHeight() / 3;

    // TODO JUCE_ARA
    // we should only be doing this on the visible time range
    double timeStart (0), timeEnd (0);
    owner.getTimeRange (timeStart, timeEnd);

    // find the next whole second after time start
    int nextWholeSecond = (int) (timeStart + 0.5);
    int lastWholeSecond = (int) (timeEnd + 0.5);
    double secondsTillWholeSecond = (double) nextWholeSecond - timeStart;
    double pixelsPerSecond = owner.getPixelsPerSecond();
    int pixelStartSeconds = (int) (secondsTillWholeSecond * pixelsPerSecond);

    // seconds ruler: one tick for each second
    g.setColour (findColour (ColourIds::timeRulerBackgroundColourId));
    g.fillRect (0, 0, bounds.getWidth(), rulerHeight);
    RectangleList <int> timeSecondsRects;
    for (int s = nextWholeSecond; s < lastWholeSecond; s++)
    {
        int secondPixel = (int) (pixelStartSeconds + s * pixelsPerSecond);
        timeSecondsRects.addWithoutMerging (Rectangle<int>(secondPixel, 0, 2, rulerHeight));
    }
    g.setColour (findColour (ColourIds::timeGridColourId));
    g.fillRectList (timeSecondsRects);

    // beat ruler
    // use our musical context to read tempo and bar signature data using content readers
    ARA::PlugIn::HostContentReader<ARA::kARAContentTypeTempoEntries> tempoReader (musicalContext);
    ARA::PlugIn::HostContentReader<ARA::kARAContentTypeBarSignatures> barSignatureReader (musicalContext);

    // we must have at least two tempo entries and a bar signature in order to have a proper musical context
    const int tempoEntryCount = tempoReader.getEventCount();
    const int barSigEventCount = barSignatureReader.getEventCount();
    jassert (tempoEntryCount >= 2 && barSigEventCount >= 1);

    // tempo ruler: one tick for each beat
    g.setColour (findColour (ColourIds::musicalRulerBackgroundColourId));
    g.fillRect (0, rulerHeight, bounds.getWidth(), rulerHeight);
    RectangleList <int> musicalRects;

    // find the first tempo entry for our starting time
    int ixT = 0;
    for (; ixT < tempoEntryCount - 2 && tempoReader.getDataForEvent (ixT + 1)->timePosition < timeStart; ++ixT);

    // use a lambda to update our tempo state while reading the host tempo map
    double tempoBPM (120);
    double secondsToBeats (0), pixelsPerBeat (0);
    double beatEnd (0);
    auto updateTempoState = [&, this] (bool advance)
    {
        if (advance)
            ++ixT;
        
        double deltaT = (tempoReader.getDataForEvent (ixT + 1)->timePosition - tempoReader.getDataForEvent (ixT)->timePosition);
        double deltaQ = (tempoReader.getDataForEvent (ixT + 1)->quarterPosition - tempoReader.getDataForEvent (ixT)->quarterPosition);
        tempoBPM = 60 * deltaQ / deltaT;
        secondsToBeats = (tempoBPM / 60);
        pixelsPerBeat = pixelsPerSecond / secondsToBeats;
        beatEnd = secondsToBeats * timeEnd;
    };

    // update our tempo state using the first two tempo entries
    updateTempoState (false);

    // convert the starting time to beats
    double beatStart = secondsToBeats * timeStart;

    // get the bar signature entry just before beat start (or the last bar signature in the reader)
    int ixB = 0;
    for (; ixB < barSigEventCount - 1 && barSignatureReader.getDataForEvent (ixB + 1)->position < beatStart; ++ixB);
    int barSigNumerator = barSignatureReader.getDataForEvent (ixB)->numerator;

    // find the next whole beat and see if it's in our range
    int nextWholeBeat = roundToInt (ceil (beatStart));
    if (nextWholeBeat < beatEnd)
    {
        // read the tempo map to find the starting beat position in pixels
        double beatPixelPosX = 0;
        double beatsTillWholeBeat = nextWholeBeat - beatStart;
        for (; ixT < tempoEntryCount - 2 && tempoReader.getDataForEvent (ixT + 1)->quarterPosition < nextWholeBeat;)
        {
            beatPixelPosX += pixelsPerBeat * (tempoReader.getDataForEvent (ixT + 1)->quarterPosition - tempoReader.getDataForEvent (ixT)->quarterPosition);
            updateTempoState (true);
        }

        if (tempoReader.getDataForEvent (ixT)->quarterPosition < nextWholeBeat)
            beatPixelPosX += pixelsPerBeat * (nextWholeBeat - tempoReader.getDataForEvent (ixT)->quarterPosition);

        // use a lambda to draw beat markers 
        auto drawBeatRects = [&] (int beatsToDraw)
        {
            // for each beat, advance beat pixel by the current pixelsPerBeat value
            for (int b = 0; b < beatsToDraw; b++)
            {
                int curBeat = nextWholeBeat + b;
                int tickWidth = 1;
                if ((curBeat % barSigNumerator) == 0)
                    tickWidth *= 2;
                musicalRects.addWithoutMerging (Rectangle<int> (roundToInt(beatPixelPosX), rulerHeight, tickWidth, rulerHeight));
                beatPixelPosX += pixelsPerBeat;
            }
        };

        // read tempo entries from the host tempo map until we run out of entries or reach timeEnd
        while (ixT < tempoEntryCount - 2 && tempoReader.getDataForEvent (ixT + 1)->timePosition < timeEnd)
        {
            // draw rects for each whole beat from nextWholeBeat to the next tempo entry
            // keep offsetting pixelStartBeats so we know where to draw the next one

            // draw a beat rect for each beat that's passed since we 
            // drew a beat marker and advance to the next whole beat 
            int beatsToNextTempoEntry = (int) (tempoReader.getDataForEvent (ixT)->quarterPosition - nextWholeBeat);
            drawBeatRects (beatsToNextTempoEntry);
            nextWholeBeat += beatsToNextTempoEntry;

            // find the new tempo
            updateTempoState (true);

            // advance bar signature numerator if our beat position passes the most recent entry
            if (ixB < barSigEventCount - 1 && barSignatureReader.getDataForEvent(ixB)->position < nextWholeBeat)
                barSigNumerator= barSignatureReader.getDataForEvent (++ixB)->numerator;
        }

        // draw the remaining rects until beat end
        int remainingBeats = roundToInt (ceil (beatEnd) - nextWholeBeat);
        drawBeatRects (remainingBeats);
    }

    g.setColour (findColour (ColourIds::musicalGridColourId));
    g.fillRectList (musicalRects);

    // TODO JUCE_ARA chord ruler
    g.setColour (findColour (ColourIds::chordsRulerBackgroundColourId));
    g.fillRect (0, rulerHeight * 2, bounds.getWidth(), rulerHeight);
    g.setColour (findColour (ColourIds::chordsColourId));

    g.setColour (findColour (ColourIds::borderColourId));
    auto borderBounds = g.getClipBounds();
    borderBounds.setHeight (rulerHeight);
    for (int i = 0; i < 3; i++)
    {
        g.drawRect (bounds);
        borderBounds.translate (0, rulerHeight);
    }
}

//==============================================================================

void RulersView::onNewSelection (const ARA::PlugIn::ViewSelection& currentSelection)
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
    {
        detachFromMusicalContext();
        repaint();
    }
}

void RulersView::didReorderMusicalContextsInDocument (ARADocument* doc)
{
    jassert (document == doc);

    if (musicalContext != document->getMusicalContexts().front())
    {
        detachFromMusicalContext();
        repaint();
    }
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
