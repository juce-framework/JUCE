#include "ARAPluginDemoAudioProcessorEditor.h"
#include "ARA_Library/Utilities/ARATimelineConversion.h"

static const juce::Identifier showOnlySelectedId = "show_only_selected";
static const juce::Identifier scrollFollowsPlayHeadId = "scroll_follows_playhead";
static juce::ValueTree editorDefaultSettings (JucePlugin_Name "_defaultEditorSettings");

//==============================================================================
ARAPluginDemoAudioProcessorEditor::ARAPluginDemoAudioProcessorEditor (ARAPluginDemoAudioProcessor& p)
    : AudioProcessorEditor (&p),
      AudioProcessorEditorARAExtension (&p),
      tooltip (this)
{
    if (isARAEditorView())
    {
        documentView.reset (new DocumentView (getARAEditorView(), p.getLastKnownPositionInfo()));
        documentView->setShowOnlySelectedRegionSequences (editorDefaultSettings.getProperty (showOnlySelectedId, false));
        documentView->setScrollFollowsPlayHead (editorDefaultSettings.getProperty (scrollFollowsPlayHeadId, documentView->isScrollFollowingPlayHead()));
        // TODO JUCE_ARA hotfix for Unicode chord symbols, see https://forum.juce.com/t/embedding-unicode-string-literals-in-your-cpp-files/12600/7
        documentView->getLookAndFeel().setDefaultSansSerifTypefaceName("Arial Unicode MS");
        addAndMakeVisible (documentView.get());

        onlySelectedTracksButton.setButtonText ("Selected Tracks Only");
        onlySelectedTracksButton.setTooltip ("If enabled, only the track(s) recently selected in the host will be shown.");
        onlySelectedTracksButton.setClickingTogglesState (true);
        onlySelectedTracksButton.setToggleState (documentView->isShowingOnlySelectedRegionSequences(), juce::dontSendNotification);
        onlySelectedTracksButton.onClick = [this]
        {
            const bool isOnlySelected = onlySelectedTracksButton.getToggleState();
            documentView->setShowOnlySelectedRegionSequences (isOnlySelected);
            editorDefaultSettings.setProperty (showOnlySelectedId, onlySelectedTracksButton.getToggleState(), nullptr);
        };
        addAndMakeVisible (onlySelectedTracksButton);

        followPlayHeadButton.setButtonText ("Follow Play-Head");
        followPlayHeadButton.setTooltip ("If enabled, view will scroll automatically when playhead leaves currently visible time range.");
        followPlayHeadButton.setClickingTogglesState (true);
        followPlayHeadButton.setToggleState (documentView->isScrollFollowingPlayHead(), juce::dontSendNotification);
        followPlayHeadButton.onClick = [this]
        {
            documentView->setScrollFollowsPlayHead (followPlayHeadButton.getToggleState());
            editorDefaultSettings.setProperty (scrollFollowsPlayHeadId, followPlayHeadButton.getToggleState(), nullptr);
        };
        addAndMakeVisible (followPlayHeadButton);

        horizontalZoomInButton.setButtonText("+");
        horizontalZoomInButton.setTooltip ("Zoom in horizontally.");
        horizontalZoomOutButton.setButtonText("-");
        horizontalZoomOutButton.setTooltip ("Zoom out horizontally.");
        constexpr static double zoomStepFactor = 1.5; // TODO JUCE_ARA MSVC requires local constexpr to be static, use std::latest to fix
        horizontalZoomInButton.onClick = [this]
        {
            documentView->zoomBy (zoomStepFactor);
        };
        horizontalZoomOutButton.onClick = [this]
        {
            documentView->zoomBy (1.0 / zoomStepFactor);
        };
        addAndMakeVisible (horizontalZoomInButton);
        addAndMakeVisible (horizontalZoomOutButton);

        playheadLinearPositionLabel.setJustificationType (juce::Justification::centred);
        playheadLinearPositionLabel.setTooltip ("Playhead position in hours:minutes:seconds:milliseconds.");
        addAndMakeVisible (playheadLinearPositionLabel);
        playheadMusicalPositionLabel.setJustificationType (juce::Justification::centred);
        playheadMusicalPositionLabel.setTooltip ("Playhead position in bars:beats:ticks.");
        addAndMakeVisible (playheadMusicalPositionLabel);
        startTimerHz (20);
    }

    setSize (1000, 600);
    // for proper view embedding, ARA plug-ins must be resizable
    setResizeLimits (500, 200, 32768, 32768);
    setResizable (true, false);
}

//==============================================================================
void ARAPluginDemoAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    if (! isARAEditorView())
    {
        g.setColour (juce::Colours::white);
        g.setFont (20.0f);
        g.drawFittedText ("Non ARA Instance. Please re-open as ARA2!", getLocalBounds(), juce::Justification::centred, 1);
    }
}

void ARAPluginDemoAudioProcessorEditor::resized()
{
    if (isARAEditorView())
    {
        constexpr int kStatusBarHeight = 20;
        constexpr int kPositionLabelWidth = 100;
        documentView->setBounds (0, 0, getWidth(), getHeight() - kStatusBarHeight);
        onlySelectedTracksButton.setBounds (0, getHeight() - kStatusBarHeight, 120, kStatusBarHeight);
        followPlayHeadButton.setBounds (onlySelectedTracksButton.getRight(), getHeight() - kStatusBarHeight, 120, kStatusBarHeight);
        horizontalZoomInButton.setBounds (getWidth() - kStatusBarHeight, getHeight() - kStatusBarHeight, kStatusBarHeight, kStatusBarHeight);
        horizontalZoomOutButton.setBounds (horizontalZoomInButton.getBounds().translated (-kStatusBarHeight, 0));
        playheadMusicalPositionLabel.setBounds ((horizontalZoomOutButton.getX() + followPlayHeadButton.getRight()) / 2, horizontalZoomOutButton.getY(), kPositionLabelWidth, kStatusBarHeight);
        playheadLinearPositionLabel.setBounds (playheadMusicalPositionLabel.getBounds().translated (-kPositionLabelWidth, 0));
    }
}

//==============================================================================

// copied from AudioPluginDemo.h: quick-and-dirty function to format a timecode string
juce::String timeToTimecodeString (double seconds)
{
    auto millisecs = juce::roundToInt (seconds * 1000.0);
    auto absMillisecs = std::abs (millisecs);

    return juce::String::formatted ("%02dh:%02dm:%02ds.%03dms",
                                    millisecs / 3600000,
                                    (absMillisecs / 60000) % 60,
                                    (absMillisecs / 1000)  % 60,
                                    absMillisecs % 1000);
}

void ARAPluginDemoAudioProcessorEditor::timerCallback()
{
    const auto timePosition = documentView->getPlayHeadPositionInfo().timeInSeconds;
    playheadLinearPositionLabel.setText (timeToTimecodeString (timePosition), juce::dontSendNotification);

    juce::String musicalPosition;
    const auto musicalContext = documentView->getMusicalContextView().getCurrentMusicalContext();
    if (musicalContext != nullptr)
    {
        const ARA::PlugIn::HostContentReader<ARA::kARAContentTypeTempoEntries> tempoReader (musicalContext);
        const ARA::PlugIn::HostContentReader<ARA::kARAContentTypeBarSignatures> barSignaturesReader (musicalContext);
        if (tempoReader && barSignaturesReader)
        {
            const ARA::TempoConverter<decltype (tempoReader)> tempoConverter (tempoReader);
            const ARA::BarSignaturesConverter<decltype (barSignaturesReader)> barSignaturesConverter (barSignaturesReader);
            const auto quarterPosition = tempoConverter.getQuarterForTime (timePosition);
            const auto barIndex = barSignaturesConverter.getBarIndexForQuarter (quarterPosition);
            const auto beatDistance = barSignaturesConverter.getBeatDistanceFromBarStartForQuarter (quarterPosition);
            const auto quartersPerBeat = 4.0 / (double) barSignaturesConverter.getBarSignatureForQuarter (quarterPosition).denominator;
            const auto beatIndex = (int) beatDistance;
            const auto tickIndex = juce::roundToInt ((beatDistance - beatIndex) * quartersPerBeat * 960.0);
            musicalPosition = juce::String::formatted ("bar %d | beat %d | tick %03d", (barIndex >= 0) ? barIndex + 1 : barIndex, beatIndex + 1, tickIndex + 1);
        }
    }
    playheadMusicalPositionLabel.setText (musicalPosition, juce::dontSendNotification);
}
