#include "MusicalContextView.h"
#include "DocumentView.h"

#include "ARA_Library/Utilities/ARAPitchInterpretation.h"
#include "ARA_Library/Utilities/ARATimelineConversion.h"

//==============================================================================
MusicalContextView::MusicalContextView (DocumentView& docView)
    : documentView (docView),
      document (docView.getDocument()),
      musicalContext (nullptr)
{
    document->addListener (this);
    findMusicalContext();
    lastPaintedPosition.resetToDefault();
    setTooltip (juce::String ("Rulers showing playback time in seconds, bars+beats and song chords.") + juce::newLine +
                juce::String ("Double-click to repositon and start host playback (if supported by DAW)."));
    startTimerHz (20);
}

MusicalContextView::~MusicalContextView()
{
    detachFromMusicalContext();
    detachFromDocument();
}

void MusicalContextView::detachFromDocument()
{
    if (document == nullptr)
        return;

    document->removeListener (this);

    document = nullptr;
}

void MusicalContextView::detachFromMusicalContext()
{
    if (musicalContext == nullptr)
        return;

    musicalContext->removeListener (this);

    musicalContext = nullptr;
}

void MusicalContextView::findMusicalContext()
{
    // evaluate selection
    juce::ARAMusicalContext* newMusicalContext = nullptr;
    auto viewSelection = documentView.getARAEditorView()->getViewSelection();
    if (! viewSelection.getRegionSequences().empty())
        newMusicalContext = viewSelection.getRegionSequences<juce::ARARegionSequence>().front()->getMusicalContext();
    else if (! viewSelection.getPlaybackRegions().empty())
        newMusicalContext = viewSelection.getPlaybackRegions<juce::ARAPlaybackRegion>().front()->getRegionSequence()->getMusicalContext();

    // if no context used yet and selection does not yield a new one, use the first musical context in the docment
    if (musicalContext == nullptr && newMusicalContext == nullptr && ! document->getMusicalContexts().empty())
        newMusicalContext = document->getMusicalContexts().front();

    if (newMusicalContext != nullptr && newMusicalContext != musicalContext)
    {
        detachFromMusicalContext();

        musicalContext = newMusicalContext;
        musicalContext->addListener (this);

        repaint();
    }
}

void MusicalContextView::timerCallback()
{
    auto positionInfo = documentView.getPlayHeadPositionInfo();
    if (lastPaintedPosition.ppqLoopStart != positionInfo.ppqLoopStart ||
        lastPaintedPosition.ppqLoopEnd != positionInfo.ppqLoopEnd ||
        lastPaintedPosition.isLooping  != positionInfo.isLooping)
    {
        repaint();
    }
}

//==============================================================================
void MusicalContextView::paint (juce::Graphics& g)
{
    const auto bounds = g.getClipBounds();

    g.setColour (juce::Colours::lightslategrey);

    if (musicalContext == nullptr)
    {
        g.setFont (juce::Font (12.0f));
        g.drawText ("No musical context found in ARA document!", bounds, juce::Justification::centred);
        return;
    }

    const auto visibleRange = documentView.getVisibleTimeRange();
    const ARA::PlugIn::HostContentReader<ARA::kARAContentTypeTempoEntries> tempoReader (musicalContext);
    const ARA::TempoConverter<decltype (tempoReader)> tempoConverter (tempoReader);

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
        juce::RectangleList<int> rects;
        const int endTime = juce::roundToInt (floor (visibleRange.getEnd()));
        for (int time = juce::roundToInt (ceil (visibleRange.getStart())); time <= endTime; ++time)
        {
            const int lineWidth = (time % 60 == 0) ? heavyLineWidth : lightLineWidth;
            const int lineHeight = (time % 10 == 0) ? secondsRulerHeight : secondsRulerHeight / 2;
            const int x = documentView.getPlaybackRegionsViewsXForTime (time);
            rects.addWithoutMerging (juce::Rectangle<int> (x - lineWidth / 2, secondsRulerY + secondsRulerHeight - lineHeight, lineWidth, lineHeight));
        }
        g.fillRectList (rects);
    }
    g.drawText ("seconds", bounds.withTrimmedRight (2), juce::Justification::bottomRight);

    // beat ruler: evaluates tempo and bar signatures to draw a line for each beat
    if (tempoReader)
    {
        const ARA::PlugIn::HostContentReader<ARA::kARAContentTypeBarSignatures> barSignaturesReader (musicalContext);
        const ARA::BarSignaturesConverter<decltype (barSignaturesReader)> barSignaturesConverter (barSignaturesReader);
        if (barSignaturesReader)
        {
            juce::RectangleList<int> rects;
            const double beatStart = barSignaturesConverter.getBeatForQuarter (tempoConverter.getQuarterForTime (visibleRange.getStart()));
            const double beatEnd = barSignaturesConverter.getBeatForQuarter (tempoConverter.getQuarterForTime (visibleRange.getEnd()));
            const int endBeat = juce::roundToInt (floor (beatEnd));
            for (int beat = juce::roundToInt (ceil (beatStart)); beat <= endBeat; ++beat)
            {
                const auto quarterPos = barSignaturesConverter.getQuarterForBeat (beat);
                const int x = documentView.getPlaybackRegionsViewsXForTime (tempoConverter.getTimeForQuarter (quarterPos));
                const auto barSignature = barSignaturesConverter.getBarSignatureForQuarter (quarterPos);
                const int lineWidth = (quarterPos == barSignature.position) ? heavyLineWidth : lightLineWidth;
                const int beatsSinceBarStart = juce::roundToInt( barSignaturesConverter.getBeatDistanceFromBarStartForQuarter (quarterPos));
                const int lineHeight = (beatsSinceBarStart == 0) ? beatsRulerHeight : beatsRulerHeight / 2;
                rects.addWithoutMerging (juce::Rectangle<int> (x - lineWidth / 2, beatsRulerY + beatsRulerHeight - lineHeight, lineWidth, lineHeight));
            }
            g.fillRectList (rects);
        }
    }
    g.drawText ("beats", bounds.withTrimmedRight (2).withTrimmedBottom (secondsRulerHeight), juce::Justification::bottomRight);

    // chord ruler: one rect per chord, skipping empty "no chords"
    if (tempoReader)
    {
        juce::RectangleList<int> rects;
        const ARA::ChordInterpreter interpreter (true);
        const ARA::PlugIn::HostContentReader<ARA::kARAContentTypeSheetChords> chordsReader (musicalContext);
        for (auto itChord = chordsReader.begin(); itChord != chordsReader.end(); ++itChord)
        {
            if (interpreter.isNoChord (*itChord))
                continue;

            juce::Rectangle<int> chordRect = bounds;
            chordRect.setVerticalRange (juce::Range<int> (chordRulerY, chordRulerY + chordRulerHeight));
            
            // find the starting position of the chord in pixels
            const auto chordStartTime = (itChord == chordsReader.begin()) ?
                                            documentView.getTimeRange().getStart() : tempoConverter.getTimeForQuarter (itChord->position);
            if (chordStartTime >= visibleRange.getEnd())
                break;
            chordRect.setLeft (documentView.getPlaybackRegionsViewsXForTime (chordStartTime));

            // if we have a chord after this one, use its starting position to end our rect
            if (std::next (itChord) != chordsReader.end())
            {
                const auto nextChordStartTime = tempoConverter.getTimeForQuarter (std::next (itChord)->position);
                if (nextChordStartTime < visibleRange.getStart())
                    continue;
                chordRect.setRight (documentView.getPlaybackRegionsViewsXForTime (nextChordStartTime));
            }

            // draw chord rect and name
            g.drawRect (chordRect);
            g.drawText (juce::convertARAString (interpreter.getNameForChord (*itChord).c_str()), chordRect.withTrimmedLeft (2), juce::Justification::centredLeft);
        }
    }
    g.drawText ("chords", bounds.withTrimmedRight (2).withTrimmedBottom (beatsRulerHeight + secondsRulerHeight), juce::Justification::bottomRight);

    // locators
    {
        lastPaintedPosition = documentView.getPlayHeadPositionInfo();
        const auto startInSeconds = tempoConverter.getTimeForQuarter (lastPaintedPosition.ppqLoopStart);
        const auto endInSeconds = tempoConverter.getTimeForQuarter (lastPaintedPosition.ppqLoopEnd);
        const int startX = documentView.getPlaybackRegionsViewsXForTime (startInSeconds);
        const int endX = documentView.getPlaybackRegionsViewsXForTime (endInSeconds);
        g.setColour (lastPaintedPosition.isLooping ? juce::Colours::skyblue.withAlpha (0.3f) : juce::Colours::grey.withAlpha (0.3f));
        g.fillRect (startX, bounds.getY(), endX - startX, bounds.getHeight());
    }

    // borders
    {
        g.setColour (juce::Colours::darkgrey);
        g.drawLine ((float) bounds.getX(), (float) beatsRulerY, (float) bounds.getRight(), (float) beatsRulerY);
        g.drawLine ((float) bounds.getX(), (float) secondsRulerY, (float) bounds.getRight(), (float) secondsRulerY);
        g.drawRect (bounds);
    }
}

//==============================================================================

void MusicalContextView::mouseDown (const juce::MouseEvent& event)
{
    // use mouse click to set the playhead position in the host (if they provide a playback controller interface)
    auto hostPlaybackController = musicalContext->getDocumentController()->getHostPlaybackController();
    if (hostPlaybackController != nullptr)
        hostPlaybackController->requestSetPlaybackPosition (documentView.getPlaybackRegionsViewsTimeForX (juce::roundToInt (event.position.x)));
}

void MusicalContextView::mouseDoubleClick (const juce::MouseEvent& /*event*/)
{
    // use mouse double click to start host playback (if they provide a playback controller interface)
    auto hostPlaybackController = musicalContext->getDocumentController()->getHostPlaybackController();
    if (hostPlaybackController != nullptr)
        hostPlaybackController->requestStartPlayback();
}

//==============================================================================

void MusicalContextView::onNewSelection (const ARA::PlugIn::ViewSelection& /*viewSelection*/)
{
    findMusicalContext();
}

void MusicalContextView::didEndEditing (juce::ARADocument* /*doc*/)
{
    if (musicalContext == nullptr)
        findMusicalContext();
}

void MusicalContextView::willRemoveMusicalContextFromDocument (juce::ARADocument* /*document*/, juce::ARAMusicalContext* context)
{
    if (musicalContext == context)
        detachFromMusicalContext();     // will restore in didEndEditing()
}

void MusicalContextView::didReorderMusicalContextsInDocument (juce::ARADocument* /*document*/)
{
    if (musicalContext != document->getMusicalContexts().front())
        detachFromMusicalContext();     // will restore in didEndEditing()
}
 void MusicalContextView::willDestroyDocument (juce::ARADocument* /*document*/)
{
    detachFromDocument();
}

void MusicalContextView::doUpdateMusicalContextContent (juce::ARAMusicalContext* /*musicalContext*/, juce::ARAContentUpdateScopes /*scopeFlags*/)
{
    repaint();
}
