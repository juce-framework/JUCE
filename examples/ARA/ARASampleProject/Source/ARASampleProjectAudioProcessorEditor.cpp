#include "ARASampleProjectAudioProcessorEditor.h"
#include "ARASampleProjectDocumentController.h"

constexpr int kStatusBarHeight = 20;
constexpr int kMinWidth = 500;
constexpr int kWidth = 1000;
constexpr int kMinHeight = 200;
constexpr int kHeight = 600;

static const Identifier pixelsPerSecondId = "pixels_per_second";
static const Identifier trackHeightId = "track_height";
static const Identifier trackHeaderViewId = " track_header_view";
static const Identifier trackHeadersVisibleId = "track_headers_visible";
static const Identifier showOnlySelectedId = "show_only_selected";
static const Identifier scrollFollowsPlaybackId = "scroll_follows_playback";

//==============================================================================
ARASampleProjectAudioProcessorEditor::ARASampleProjectAudioProcessorEditor (ARASampleProjectAudioProcessor& p)
    : AudioProcessorEditor (&p),
      AudioProcessorEditorARAExtension (&p)
{
    if (isARAEditorView())
    {
        documentView.reset (new DocumentView (*this, p.getLastKnownPositionInfo()));
        // TODO JUCE_ARA hotfix for Unicode chord symbols, see https://forum.juce.com/t/embedding-unicode-string-literals-in-your-cpp-files/12600/7
        documentView->getLookAndFeel().setDefaultSansSerifTypefaceName("Arial Unicode MS");
        documentView->setIsRulersVisible (true);
        documentView->setIsTrackHeadersVisible (true);
        documentView->setShowOnlySelectedRegionSequences (false);
        documentView->addListener (this);
        addAndMakeVisible (documentView.get());

        auto documentController = getARADocumentController<ARASampleProjectDocumentController>();
        globalSettings = &documentController->getGlobalEditorSettings();
        globalSettings->addListener (this);

        hideTrackHeaderButton.setButtonText ("Hide Track Headers");
        hideTrackHeaderButton.setClickingTogglesState (true);
        hideTrackHeaderButton.setToggleState(! documentView->isTrackHeadersVisible(), dontSendNotification);
        hideTrackHeaderButton.onClick = [this]
        {
            globalSettings->setProperty (trackHeadersVisibleId, ! hideTrackHeaderButton.getToggleState(), nullptr);
        };
        addAndMakeVisible (hideTrackHeaderButton);

        onlySelectedTracksButton.setButtonText ("Selected Tracks Only");
        onlySelectedTracksButton.setClickingTogglesState (true);
        onlySelectedTracksButton.setToggleState(documentView->isShowingOnlySelectedRegionSequences(), dontSendNotification);
        onlySelectedTracksButton.onClick = [this]
        {
            globalSettings->setProperty (showOnlySelectedId, onlySelectedTracksButton.getToggleState(), nullptr);
        };
        addAndMakeVisible (onlySelectedTracksButton);
        followPlayheadButton.setButtonText ("Follow Playhead");
        followPlayheadButton.setClickingTogglesState (true);
        followPlayheadButton.onClick = [this]
        {
            globalSettings->setProperty (scrollFollowsPlaybackId, followPlayheadButton.getToggleState(), nullptr);
        };
        addAndMakeVisible (followPlayheadButton);

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
            globalSettings->setProperty (trackHeightId, documentView->getTrackHeight() * zoomStepFactor, nullptr);
        };
        verticalZoomOutButton.onClick = [this, zoomStepFactor]
        {
            globalSettings->setProperty (trackHeightId, documentView->getTrackHeight() / zoomStepFactor, nullptr);
        };
        addAndMakeVisible (horizontalZoomInButton);
        addAndMakeVisible (horizontalZoomOutButton);
        addAndMakeVisible (verticalZoomInButton);
        addAndMakeVisible (verticalZoomOutButton);

        // first time try to load globalSettings if any..
        // a cleaner approach would be to add another redundant function to be called from
        // valueTreePropertyChanged. (instead of passing bogus identifier).
        valueTreePropertyChanged (*globalSettings, "null");
    }

    setSize (kWidth, kHeight);
    setResizeLimits (kMinWidth, kMinHeight, 32768, 32768);
    setResizable (true, false);
}

ARASampleProjectAudioProcessorEditor::~ARASampleProjectAudioProcessorEditor()
{
    if (isARAEditorView())
    {
        globalSettings->removeListener (this);
        globalSettings = nullptr;
        documentView->removeListener (this);
    }
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
        followPlayheadButton.setBounds (onlySelectedTracksButton.getRight(), getHeight() - kStatusBarHeight, 120, kStatusBarHeight);
        verticalZoomInButton.setBounds (getWidth() - kStatusBarHeight, getHeight() - kStatusBarHeight, kStatusBarHeight, kStatusBarHeight);
        verticalZoomOutButton.setBounds (verticalZoomInButton.getBounds().translated (-kStatusBarHeight, 0));
        verticalZoomLabel.setBounds (verticalZoomOutButton.getBounds().translated (-kStatusBarHeight, 0));
        horizontalZoomInButton.setBounds (verticalZoomLabel.getBounds().translated (-kStatusBarHeight, 0));
        horizontalZoomOutButton.setBounds (horizontalZoomInButton.getBounds().translated (-kStatusBarHeight, 0));
        horizontalZoomLabel.setBounds (horizontalZoomOutButton.getBounds().translated (-kStatusBarHeight, 0));
    }
}

void ARASampleProjectAudioProcessorEditor::visibleTimeRangeChanged (Range<double> /*newVisibleTimeRange*/, double pixelsPerSecond)
{
    // on async callback it can be null...
    if (globalSettings)
        globalSettings->setProperty (pixelsPerSecondId, pixelsPerSecond, nullptr);

    horizontalZoomInButton.setEnabled (documentView->isMinimumPixelsPerSecond());
    horizontalZoomOutButton.setEnabled (documentView->isMaximumPixelsPerSecond());
}

void ARASampleProjectAudioProcessorEditor::valueTreePropertyChanged(juce::ValueTree &tree, const juce::Identifier &property)
{
    // something is wrong, this should only listens when ARADocumentController exists!
    jassert (documentView != nullptr);

    documentView->setTrackHeight (tree.getProperty (trackHeightId, documentView->getTrackHeight()));
    documentView->setTrackHeaderWidth (tree.getProperty (trackHeaderViewId, documentView->getTrackHeaderWidth()));
    documentView->setIsTrackHeadersVisible (tree.getProperty (trackHeadersVisibleId, documentView->isTrackHeadersVisible()));
    documentView->setShowOnlySelectedRegionSequences (tree.getProperty (showOnlySelectedId, documentView->isShowingOnlySelectedRegionSequences()));
    documentView->setScrollFollowsPlaybackState (tree.getProperty (scrollFollowsPlaybackId, documentView->getScrollFollowPlaybackState()));
    documentView->setPixelsPerSecond (tree.getProperty(pixelsPerSecondId, documentView->getPixelsPerSecond()));
}
