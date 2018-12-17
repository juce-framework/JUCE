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
        document = static_cast<ARADocument*>(owner.getARADocumentController()->getDocument());
        document->addListener (this);
        if (findMusicalContext())
            musicalContext->addListener (this);
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
bool RulersView::findMusicalContext()
{
    detachFromMusicalContext();

    if (! owner.isARAEditorView())
        return false;

    auto findMusicalContextLambda = [] (ARASampleProjectAudioProcessorEditor& editor)
    {
        ARAMusicalContext* musicalContext = nullptr;

        // first try the first selected region sequence
        auto viewSelection = editor.getARAEditorView()->getViewSelection();
        if (! viewSelection.getRegionSequences().empty())
        {
            musicalContext = static_cast<ARAMusicalContext*>(viewSelection.getRegionSequences()[0]->getMusicalContext());
            if (musicalContext != nullptr)
                return musicalContext;
        }

        // then try the first selected playback region's region sequence
        if (! viewSelection.getPlaybackRegions().empty())
        {
            musicalContext = static_cast<ARAMusicalContext*>(viewSelection.getPlaybackRegions()[0]->getRegionSequence()->getMusicalContext());
            if (musicalContext != nullptr)
                return musicalContext;
        }

        // no selection? if we have an editor renderer try the first region sequence / playback region
        if (auto editorRenderer = editor.getAudioProcessorARAExtension()->getARAEditorRenderer())
        {
            if (! editorRenderer->getRegionSequences().empty())
            {
                musicalContext = static_cast<ARAMusicalContext*>(editorRenderer->getRegionSequences()[0]->getMusicalContext());
                if (musicalContext != nullptr)
                    return musicalContext;
            }

            if (! editorRenderer->getPlaybackRegions().empty())
            {
                musicalContext = static_cast<ARAMusicalContext*>(editorRenderer->getPlaybackRegions()[0]->getRegionSequence()->getMusicalContext());
                if (musicalContext != nullptr)
                    return musicalContext;
            }
        }

        // otherwise if we're a playback renderer try the first playback region
        if (auto playbackRenderer = editor.getAudioProcessorARAExtension()->getARAPlaybackRenderer())
        {
            if (! playbackRenderer->getPlaybackRegions().empty())
            {
                musicalContext = static_cast<ARAMusicalContext*>(playbackRenderer->getPlaybackRegions()[0]->getRegionSequence()->getMusicalContext());
                if (musicalContext != nullptr)
                    return musicalContext;
            }
        }

        // still nothing? try the first musical context in the docment
        auto document = editor.getARADocumentController()->getDocument();
        jassert (! document->getMusicalContexts().empty());
        return static_cast<ARAMusicalContext*>(editor.getARADocumentController()->getDocument()->getMusicalContexts()[0]);
    };

    musicalContext = findMusicalContextLambda (owner);
    if (musicalContext != nullptr)
    {
        musicalContext->addListener (this);
        return true;
    }

    return false;
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

    // use a lambda to update our tempo state while reading the host tempo map
    int ixT = 0;
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
        // if so, start drawing tick marks at each whole (integer) beat
        double beatsTillWholeBeat = nextWholeBeat - beatStart;

        // use a lambda to draw beat markers 
        double beatPixelPosX = pixelsPerBeat * beatsTillWholeBeat;
        auto drawBeatRects = [&] (int beatsToDraw)
        {
            // for each beat, advance beat pixel by the current pixelsPerBeat value
            for (int b = 0; b < beatsToDraw; b++, beatPixelPosX += pixelsPerBeat)
            {
                int curBeat = nextWholeBeat + b;
                int tickWidth = 1;
                if ((curBeat % barSigNumerator) == 0)
                    tickWidth *= 2;
                musicalRects.addWithoutMerging (Rectangle<int> (roundToInt(beatPixelPosX), rulerHeight, tickWidth, rulerHeight));
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

void RulersView::didEndEditing (ARADocument* /*doc*/)
{
    if (findMusicalContext())
        musicalContext->addListener (this);
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
