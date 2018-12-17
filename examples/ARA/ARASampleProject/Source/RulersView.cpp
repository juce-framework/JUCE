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
    if (musicalContext == nullptr)
    {
        g.setColour (Colours::darkgrey);
        g.drawRect (getLocalBounds(), 3);

        g.setColour (Colours::white);
        g.setFont (Font (12.0f));
        g.drawText ("Not implemented yet: showing seconds, bars with beat division, chords", getLocalBounds(), Justification::centred);
        
        return;
    }

    // we'll draw three rulers: seconds, beats, and chords
    int rulerHeight = getHeight() / 3;
    g.setColour (Colours::slategrey);

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
    for (int s = nextWholeSecond; s < lastWholeSecond; s++)
    {
        int secondPixel = (int) (pixelStartSeconds + s * owner.getPixelsPerSecond());
        Rectangle<int> lineRect (secondPixel, 0, 2, rulerHeight);
        g.fillRect (lineRect);
    }

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
    for (int b = nextWholeBeat; b < lastWholeBeat; b++)
    {
        int tickWidth = (b % barSigNumerator == 0) ? 2 : 1;
        int beatPixel = pixelStart + (int) (pixelsPerBeat * (b - nextWholeBeat));
        Rectangle<int> lineRect (beatPixel, rulerHeight, tickWidth, rulerHeight);
        g.fillRect (lineRect);
    }

    // TODO JUCE_ARA chord ruler

    g.setColour (Colours::black);
    auto bounds = g.getClipBounds();
    bounds.setHeight (rulerHeight);
    for (int i = 0; i < 3; i++)
    {
        g.drawRect (bounds);
        bounds.translate (0, rulerHeight);
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
