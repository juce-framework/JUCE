#include "ARASampleProjectAudioProcessorEditor.h"

constexpr int kStatusBarHeight = 20;
constexpr int kMinWidth = 500;
constexpr int kWidth = 1000;
constexpr int kMinHeight = 200;
constexpr int kHeight = 600;

static const Identifier pixelsPerSecondId = "pixels_per_second";
static const Identifier trackHeightId = "track_height";
static const Identifier trackHeaderWidthId = "track_header_width";
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
        documentView->setTrackHeaderWidth (editorDefaultSettings.getProperty (trackHeaderWidthId, documentView->getTrackHeaderWidth()));
        documentView->setIsTrackHeadersVisible (editorDefaultSettings.getProperty (trackHeadersVisibleId, documentView->isTrackHeadersVisible()));
        documentView->setShowOnlySelectedRegionSequences (editorDefaultSettings.getProperty (showOnlySelectedId, documentView->isShowingOnlySelectedRegionSequences()));
        documentView->setScrollFollowsPlayHead (editorDefaultSettings.getProperty (scrollFollowsPlayHeadId, documentView->isScrollFollowingPlayHead()));
        documentView->setPixelsPerSecond (editorDefaultSettings.getProperty(pixelsPerSecondId, documentView->getPixelsPerSecond()));

        // TODO JUCE_ARA hotfix for Unicode chord symbols, see https://forum.juce.com/t/embedding-unicode-string-literals-in-your-cpp-files/12600/7
        documentView->getLookAndFeel().setDefaultSansSerifTypefaceName("Arial Unicode MS");
        documentView->setIsRulersVisible (true);
        documentView->addListener (this);
        addAndMakeVisible (documentView.get());

        hideTrackHeaderButton.setButtonText ("Hide Track Headers");
        hideTrackHeaderButton.setClickingTogglesState (true);
        hideTrackHeaderButton.setToggleState(! documentView->isTrackHeadersVisible(), dontSendNotification);
        hideTrackHeaderButton.onClick = [this]
        {
            documentView->setIsTrackHeadersVisible (! hideTrackHeaderButton.getToggleState());
            editorDefaultSettings.setProperty (trackHeadersVisibleId,
                                               ! hideTrackHeaderButton.getToggleState(), nullptr);
        };
        addAndMakeVisible (hideTrackHeaderButton);

        onlySelectedTracksButton.setButtonText ("Selected Tracks Only");
        onlySelectedTracksButton.setClickingTogglesState (true);
        onlySelectedTracksButton.setToggleState (documentView->isShowingOnlySelectedRegionSequences(), dontSendNotification);
        onlySelectedTracksButton.onClick = [this]
        {
            documentView->setShowOnlySelectedRegionSequences (onlySelectedTracksButton.getToggleState());
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
        addAndMakeVisible (verticalZoomLabel);

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
            documentView->setTrackHeight (documentView->getTrackHeight() * zoomStepFactor);
        };
        verticalZoomOutButton.onClick = [this, zoomStepFactor]
        {
            documentView->setTrackHeight (documentView->getTrackHeight() / zoomStepFactor);
        };
        addAndMakeVisible (horizontalZoomInButton);
        addAndMakeVisible (horizontalZoomOutButton);
        addAndMakeVisible (verticalZoomInButton);
        addAndMakeVisible (verticalZoomOutButton);

        // show playhead position
        playheadLinearPositionLabel.setJustificationType (Justification::centred);
        playheadMusicalPositionLabel.setJustificationType (Justification::centred);
        addAndMakeVisible (playheadMusicalPositionLabel);
        addAndMakeVisible (playheadLinearPositionLabel);
        startTimerHz (25);
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
        hideTrackHeaderButton.setBounds(0, getHeight() - kStatusBarHeight, 120, kStatusBarHeight);
        onlySelectedTracksButton.setBounds (hideTrackHeaderButton.getRight(), getHeight() - kStatusBarHeight, 120, kStatusBarHeight);
        followPlayHeadButton.setBounds (onlySelectedTracksButton.getRight(), getHeight() - kStatusBarHeight, 120, kStatusBarHeight);
        verticalZoomInButton.setBounds (getWidth() - kStatusBarHeight, getHeight() - kStatusBarHeight, kStatusBarHeight, kStatusBarHeight);
        verticalZoomOutButton.setBounds (verticalZoomInButton.getBounds().translated (-kStatusBarHeight, 0));
        verticalZoomLabel.setBounds (verticalZoomOutButton.getBounds().translated (-kStatusBarHeight, 0));
        horizontalZoomInButton.setBounds (verticalZoomLabel.getBounds().translated (-kStatusBarHeight, 0));
        horizontalZoomOutButton.setBounds (horizontalZoomInButton.getBounds().translated (-kStatusBarHeight, 0));
        horizontalZoomLabel.setBounds (horizontalZoomOutButton.getBounds().translated (-kStatusBarHeight, 0));
        const int kPositionLabelWidth = 100;
        playheadMusicalPositionLabel.setBounds (horizontalZoomLabel.getX() - kPositionLabelWidth, horizontalZoomLabel.getY(), kPositionLabelWidth, kStatusBarHeight);
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

void ARASampleProjectAudioProcessorEditor::timerCallback()
{
    // update position from playhead
    playheadLinearPositionLabel.setText (documentView->getTimecodeAsString(), dontSendNotification);
    playheadMusicalPositionLabel.setText (documentView->getMusicalPositionAsString(), dontSendNotification);
}
