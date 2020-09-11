#include "ARAPluginDemoAudioProcessorEditor.h"
#include "ARA_Library/Utilities/ARATimelineConversion.h"

static const Identifier showOnlySelectedId = "show_only_selected";
static const Identifier scrollFollowsPlayHeadId = "scroll_follows_playhead";
static ValueTree editorDefaultSettings (JucePlugin_Name "_defaultEditorSettings");

//==============================================================================
ARAPluginDemoAudioProcessorEditor::ARAPluginDemoAudioProcessorEditor (ARAPluginDemoAudioProcessor& p)
    : AudioProcessorEditor (&p),
      AudioProcessorEditorARAExtension (&p)
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
        onlySelectedTracksButton.setClickingTogglesState (true);
        onlySelectedTracksButton.setToggleState (documentView->isShowingOnlySelectedRegionSequences(), dontSendNotification);
        onlySelectedTracksButton.onClick = [this]
        {
            const bool isOnlySelected = onlySelectedTracksButton.getToggleState();
            documentView->setShowOnlySelectedRegionSequences (isOnlySelected);
            editorDefaultSettings.setProperty (showOnlySelectedId, onlySelectedTracksButton.getToggleState(), nullptr);
        };
        addAndMakeVisible (onlySelectedTracksButton);

        followPlayHeadButton.setButtonText ("Follow Play-Head");
        followPlayHeadButton.setClickingTogglesState (true);
        followPlayHeadButton.setToggleState (documentView->isScrollFollowingPlayHead(), dontSendNotification);
        followPlayHeadButton.onClick = [this]
        {
            documentView->setScrollFollowsPlayHead (followPlayHeadButton.getToggleState());
            editorDefaultSettings.setProperty (scrollFollowsPlayHeadId, followPlayHeadButton.getToggleState(), nullptr);
        };
        addAndMakeVisible (followPlayHeadButton);

        horizontalZoomInButton.setButtonText("+");
        horizontalZoomOutButton.setButtonText("-");
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

        playheadLinearPositionLabel.setJustificationType (Justification::centred);
        playheadMusicalPositionLabel.setJustificationType (Justification::centred);
        addAndMakeVisible (playheadMusicalPositionLabel);
        addAndMakeVisible (playheadLinearPositionLabel);
        startTimerHz (20);
    }

    setSize (1000, 600);
    setResizeLimits (500, 200, 32768, 32768);
    setResizable (true, false);
}

//==============================================================================
void ARAPluginDemoAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    if (! isARAEditorView())
    {
        g.setColour (Colours::white);
        g.setFont (20.0f);
        g.drawFittedText ("Non ARA Instance. Please re-open as ARA2!", getLocalBounds(), Justification::centred, 1);
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
String timeToTimecodeString (double seconds)
{
    auto millisecs = roundToInt (seconds * 1000.0);
    auto absMillisecs = std::abs (millisecs);

    return String::formatted ("%02dh:%02dm:%02ds.%03dms",
                              millisecs / 3600000,
                              (absMillisecs / 60000) % 60,
                              (absMillisecs / 1000)  % 60,
                              absMillisecs % 1000);
}

void ARAPluginDemoAudioProcessorEditor::timerCallback()
{
    const auto timePosition = documentView->getPlayHeadPositionInfo().timeInSeconds;
    playheadLinearPositionLabel.setText (timeToTimecodeString (timePosition), dontSendNotification);

    String musicalPosition;
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
            const auto tickIndex = roundToInt ((beatDistance - beatIndex) * quartersPerBeat * 960.0);
            musicalPosition = String::formatted ("bar %d | beat %d | tick %03d", (barIndex >= 0) ? barIndex + 1 : barIndex, beatIndex + 1, tickIndex + 1);
        }
    }
    playheadMusicalPositionLabel.setText (musicalPosition, dontSendNotification);
}
