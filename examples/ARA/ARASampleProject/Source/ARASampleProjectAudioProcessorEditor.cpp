#include "ARASampleProjectAudioProcessorEditor.h"
#include "ARA_Library/Utilities/ARATimelineConversion.h"

constexpr int kStatusBarHeight = 20;
constexpr int kPositionLabelWidth = 100;
constexpr int kMinWidth = 500;
constexpr int kWidth = 1000;
constexpr int kMinHeight = 200;
constexpr int kHeight = 600;

static const Identifier pixelsPerSecondId = "pixels_per_second";
static const Identifier trackHeightId = "track_height";
static const Identifier trackHeadersVisibleId = "track_headers_visible";
static const Identifier showOnlySelectedId = "show_only_selected";
static const Identifier scrollFollowsPlayHeadId = "scroll_follows_playhead";

static ValueTree editorDefaultSettings (JucePlugin_Name "_defaultEditorSettings");

//==============================================================================
ARASampleProjectAudioProcessorEditor::ARASampleProjectAudioProcessorEditor (ARASampleProjectAudioProcessor& p)
    : AudioProcessorEditor (&p),
      AudioProcessorEditorARAExtension (&p)
{
    if (isARAEditorView())
    {
        documentView.reset (new DocumentView (*this, p.getLastKnownPositionInfo()));

        // if no defaults yet, construct defaults based on hard-coded defaults from DocumentView
        documentView->setTrackHeight (editorDefaultSettings.getProperty (trackHeightId, documentView->getTrackHeight()));
        documentView->setShowOnlySelectedRegionSequences (editorDefaultSettings.getProperty (showOnlySelectedId, false));
        documentView->setScrollFollowsPlayHead (editorDefaultSettings.getProperty (scrollFollowsPlayHeadId, documentView->isScrollFollowingPlayHead()));
        documentView->setPixelsPerSecond (editorDefaultSettings.getProperty(pixelsPerSecondId, documentView->getPixelsPerSecond()));

        // TODO JUCE_ARA hotfix for Unicode chord symbols, see https://forum.juce.com/t/embedding-unicode-string-literals-in-your-cpp-files/12600/7
        documentView->getLookAndFeel().setDefaultSansSerifTypefaceName("Arial Unicode MS");
        documentView->setIsRulersVisible (true);
        documentView->addListener (this);
        addAndMakeVisible (documentView.get());

        onlySelectedTracksButton.setButtonText ("Selected Tracks Only");
        onlySelectedTracksButton.setClickingTogglesState (true);
        onlySelectedTracksButton.setToggleState (documentView->isShowingOnlySelectedRegionSequences(), dontSendNotification);
        onlySelectedTracksButton.onClick = [this]
        {
            const bool isOnlySelected = onlySelectedTracksButton.getToggleState();
            documentView->setShowOnlySelectedRegionSequences (isOnlySelected);
            verticalZoomLabel.setVisible (! isOnlySelected);
            verticalZoomInButton.setVisible (! isOnlySelected);
            verticalZoomOutButton.setVisible (! isOnlySelected);
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

        horizontalZoomLabel.setText ("H:", dontSendNotification);
        verticalZoomLabel.setText ("V:", dontSendNotification);
        addAndMakeVisible (horizontalZoomLabel);

        horizontalZoomInButton.setButtonText("+");
        horizontalZoomOutButton.setButtonText("-");
        verticalZoomInButton.setButtonText("+");
        verticalZoomOutButton.setButtonText("-");
        constexpr double zoomStepFactor = 1.5;
        horizontalZoomInButton.onClick = [this, zoomStepFactor]
        {
            documentView->setPixelsPerSecond (documentView->getPixelsPerSecond() * zoomStepFactor);
        };
        horizontalZoomOutButton.onClick = [this, zoomStepFactor]
        {
            documentView->setPixelsPerSecond (documentView->getPixelsPerSecond() / zoomStepFactor);
        };
        verticalZoomInButton.onClick = [this, zoomStepFactor]
        {
            documentView->setTrackHeight ((int) (documentView->getTrackHeight() * zoomStepFactor));
        };
        verticalZoomOutButton.onClick = [this, zoomStepFactor]
        {
            documentView->setTrackHeight ((int) (documentView->getTrackHeight () / zoomStepFactor));
        };
        addAndMakeVisible (horizontalZoomInButton);
        addAndMakeVisible (horizontalZoomOutButton);
        if (! documentView->isShowingOnlySelectedRegionSequences())
        {
            addAndMakeVisible (verticalZoomLabel);
            addAndMakeVisible (verticalZoomInButton);
            addAndMakeVisible (verticalZoomOutButton);
        }
        else
        {
            addChildComponent (verticalZoomLabel);
            addChildComponent (verticalZoomInButton);
            addChildComponent (verticalZoomOutButton);
        }

        // show playhead position
        playheadLinearPositionLabel.setJustificationType (Justification::centred);
        playheadMusicalPositionLabel.setJustificationType (Justification::centred);
        addAndMakeVisible (playheadMusicalPositionLabel);
        addAndMakeVisible (playheadLinearPositionLabel);
        startTimerHz (20);
    }

    setSize (kWidth, kHeight);
    setResizeLimits (kMinWidth, kMinHeight, 32768, 32768);
    setResizable (true, false);
}

ARASampleProjectAudioProcessorEditor::~ARASampleProjectAudioProcessorEditor()
{
    if (isARAEditorView())
        documentView->removeListener (this);
}

//==============================================================================
void ARASampleProjectAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    if (! isARAEditorView())
    {
        g.setColour (Colours::white);
        g.setFont (20.0f);
        g.drawFittedText ("Non ARA Instance. Please re-open as ARA2!", getLocalBounds(), Justification::centred, 1);
    }
}

void ARASampleProjectAudioProcessorEditor::resized()
{
    if (isARAEditorView())
    {
        documentView->setBounds (0, 0, getWidth(), getHeight() - kStatusBarHeight);
        onlySelectedTracksButton.setBounds (0, getHeight() - kStatusBarHeight, 120, kStatusBarHeight);
        followPlayHeadButton.setBounds (onlySelectedTracksButton.getRight(), getHeight() - kStatusBarHeight, 120, kStatusBarHeight);
        verticalZoomInButton.setBounds (getWidth() - kStatusBarHeight, getHeight() - kStatusBarHeight, kStatusBarHeight, kStatusBarHeight);
        verticalZoomOutButton.setBounds (verticalZoomInButton.getBounds().translated (-kStatusBarHeight, 0));
        verticalZoomLabel.setBounds (verticalZoomOutButton.getBounds().translated (-kStatusBarHeight, 0));
        horizontalZoomInButton.setBounds (verticalZoomLabel.getBounds().translated (-kStatusBarHeight, 0));
        horizontalZoomOutButton.setBounds (horizontalZoomInButton.getBounds().translated (-kStatusBarHeight, 0));
        horizontalZoomLabel.setBounds (horizontalZoomOutButton.getBounds().translated (-kStatusBarHeight, 0));
        playheadMusicalPositionLabel.setBounds ((horizontalZoomLabel.getX() + followPlayHeadButton.getRight()) / 2, horizontalZoomLabel.getY(), kPositionLabelWidth, kStatusBarHeight);
        playheadLinearPositionLabel.setBounds (playheadMusicalPositionLabel.getBounds().translated (-kPositionLabelWidth, 0));
    }
}

void ARASampleProjectAudioProcessorEditor::visibleTimeRangeChanged (Range<double> /*newVisibleTimeRange*/, double pixelsPerSecond)
{
    horizontalZoomInButton.setEnabled (documentView->isMinimumPixelsPerSecond());
    horizontalZoomOutButton.setEnabled (documentView->isMaximumPixelsPerSecond());
    editorDefaultSettings.setProperty (pixelsPerSecondId, pixelsPerSecond, nullptr);
}

void ARASampleProjectAudioProcessorEditor::trackHeightChanged (int newTrackHeight)
{
    editorDefaultSettings.setProperty (trackHeightId, newTrackHeight, nullptr);
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

void ARASampleProjectAudioProcessorEditor::timerCallback()
{
    const auto timePosition = documentView->getPlayHeadPositionInfo().timeInSeconds;
    playheadLinearPositionLabel.setText (timeToTimecodeString (timePosition), dontSendNotification);

    String musicalPosition;
    const auto musicalContext = documentView->getCurrentMusicalContext();
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
