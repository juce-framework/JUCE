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

    if (!owner.isARAEditorView())
        return false;

    auto findMusicalContextLambda = [] (ARASampleProjectAudioProcessorEditor& editor)
    {
        ARAMusicalContext* musicalContext = nullptr;

        // first try the first selected region sequence
        auto viewSelection = editor.getARAEditorView()->getViewSelection();
        if (!viewSelection.getRegionSequences().empty())
        {
            musicalContext = static_cast<ARAMusicalContext*>(viewSelection.getRegionSequences()[0]->getMusicalContext());
            if (musicalContext != nullptr)
                return musicalContext;
        }

        // then try the first selected playback region's region sequence
        if (!viewSelection.getPlaybackRegions().empty())
        {
            musicalContext = static_cast<ARAMusicalContext*>(viewSelection.getPlaybackRegions()[0]->getRegionSequence()->getMusicalContext());
            if (musicalContext != nullptr)
                return musicalContext;
        }

        // no selection? if we have an editor renderer try the first region sequence / playback region
        if (auto editorRenderer = editor.getAudioProcessorARAExtension()->getARAEditorRenderer())
        {
            musicalContext = static_cast<ARAMusicalContext*>(editorRenderer->getRegionSequences()[0]->getMusicalContext());
            if (musicalContext != nullptr)
                return musicalContext;

            musicalContext = static_cast<ARAMusicalContext*>(editorRenderer->getPlaybackRegions()[0]->getRegionSequence()->getMusicalContext());
            if (musicalContext != nullptr)
                return musicalContext;
        }

        // otherwise if we're a playback renderer try the first playback region
        if (auto playbackRenderer = editor.getAudioProcessorARAExtension()->getARAPlaybackRenderer())
        {
            musicalContext = static_cast<ARAMusicalContext*>(playbackRenderer->getPlaybackRegions()[0]->getRegionSequence()->getMusicalContext());
            if (musicalContext != nullptr)
                return musicalContext;
        }

        // still nothing? try the first musical context in the docment
        musicalContext = static_cast<ARAMusicalContext*>(editor.getARADocumentController()->getDocument()->getMusicalContexts()[0]);
        if (musicalContext != nullptr)
            return musicalContext;

        return static_cast<ARAMusicalContext*>(nullptr);
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

    // tempo ruler
    // use our musical context to read tempo and bar signature data using content readers
    ARA::PlugIn::HostContentReader<ARA::kARAContentTypeTempoEntries> tempoReader (musicalContext);
    ARA::PlugIn::HostContentReader<ARA::kARAContentTypeBarSignatures> barSignatureReader (musicalContext);
    jassert (tempoReader.getEventCount() >= 2 && barSignatureReader.getEventCount() > 0);

    // TODO JUCE_ARA
    // look this up depending on which beat we're rendering
    // to properly handle changes in tempo or bar signature
    double deltaT = (tempoReader.getDataForEvent(1)->timePosition - tempoReader.getDataForEvent(0)->timePosition);
    double deltaQ = (tempoReader.getDataForEvent(1)->quarterPosition - tempoReader.getDataForEvent (0)->quarterPosition);
    double tempoBPM = 60 * deltaQ / deltaT;
    int barSigNumerator = barSignatureReader.getDataForEvent(0)->numerator;

    // TODO JUCE_ARA
    // we should only be doing this on the visible time range
    double timeStart (0), timeEnd (0);
    owner.getTimeRange (timeStart, timeEnd);

    // find the next whole second after time start
    int nextWholeSecond = (int) (timeStart + 0.5);
    int lastWholeSecond = (int) (timeEnd + 0.5);
    double secondsTillWholeSecond = (double) nextWholeSecond - timeStart;
    int pixelStartSeconds = (int) (secondsTillWholeSecond * owner.getPixelsPerSecond());

    // seconds ruler: one tick for each second
    g.setColour (findColour (ColourIds::timeRulerBackgroundColourId));
    g.fillRect (0, 0, bounds.getWidth(), rulerHeight);
    RectangleList <int> timeSecondsRects;
    for (int s = nextWholeSecond; s < lastWholeSecond; s++)
    {
        int secondPixel = (int) (pixelStartSeconds + s * owner.getPixelsPerSecond());
        timeSecondsRects.addWithoutMerging (Rectangle<int>(secondPixel, 0, 2, rulerHeight));
    }
    g.setColour (findColour (ColourIds::timeGridColourId));
    g.fillRectList (timeSecondsRects);

    // convert the time range to beats
    double secondsToBeats = (tempoBPM / 60);
    double beatStart = secondsToBeats * timeStart;
    double beatEnd = secondsToBeats * timeEnd;

    // find the next whole beat
    int nextWholeBeat = (int) (beatStart + 0.5);
    int lastWholeBeat = (int) (beatEnd + 0.5);
    double pixelsPerBeat = owner.getPixelsPerSecond() / secondsToBeats;
    double beatsTillWholeBeat = (double) nextWholeBeat - beatStart;
    int pixelStart = (int) (beatsTillWholeBeat * pixelsPerBeat);

    // tempo ruler: one tick for each beat
    // and thicker ticks for each downbeat
    g.setColour (findColour (ColourIds::musicalRulerBackgroundColourId));
    g.fillRect (0, rulerHeight, bounds.getWidth(), rulerHeight);
    RectangleList <int> musicalRects;
    for (int b = nextWholeBeat; b < lastWholeBeat; b++)
    {
        int tickWidth = (b % barSigNumerator == 0) ? 2 : 1;
        int beatPixel = pixelStart + (int) (pixelsPerBeat * (b - nextWholeBeat));
        musicalRects.addWithoutMerging (Rectangle<int>(beatPixel, rulerHeight, tickWidth, rulerHeight));
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
