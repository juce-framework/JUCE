#include "ARASampleProjectAudioProcessorEditor.h"

constexpr int kStatusBarHeight = 20;
constexpr int kMinWidth = 500;
constexpr int kWidth = 1000;
constexpr int kMinHeight = 200;
constexpr int kHeight = 600;

//==============================================================================
ARASampleProjectAudioProcessorEditor::ARASampleProjectAudioProcessorEditor (ARASampleProjectAudioProcessor& p)
    : AudioProcessorEditor (&p),
      AudioProcessorEditorARAExtension (&p)
{
    if (isARAEditorView())
    {
        documentView.reset (new DocumentView (*this));
        // TODO JUCE_ARA hotfix for Unicode chord symbols, see https://forum.juce.com/t/embedding-unicode-string-literals-in-your-cpp-files/12600/7
        documentView->getLookAndFeel().setDefaultSansSerifTypefaceName("Arial Unicode MS");
        documentView->setCurrentPositionInfo (&p.getLastKnownPositionInfo());
        addAndMakeVisible (documentView.get());

        followPlayheadToggleButton.setButtonText ("Viewport follows playhead");
        followPlayheadToggleButton.getToggleStateValue().referTo (documentView->getScrollFollowsPlaybackStateValue());
        addAndMakeVisible (followPlayheadToggleButton);

        // sample zoom functionality
        horizontalZoomLabel.setText ("H:", dontSendNotification);
        verticalZoomLabel.setText ("V:", dontSendNotification);
        verticalZoomInButton.setButtonText("+");
        verticalZoomOutButton.setButtonText("-");
        horizontalZoomInButton.setButtonText("+");
        horizontalZoomOutButton.setButtonText("-");
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

        addAndMakeVisible (horizontalZoomLabel);
        addAndMakeVisible (verticalZoomLabel);
        addAndMakeVisible (verticalZoomInButton);
        addAndMakeVisible (verticalZoomOutButton);
        addAndMakeVisible (horizontalZoomInButton);
        addAndMakeVisible (horizontalZoomOutButton);
        documentView->setIsRulersVisible (true);
        documentView->addListener (this);
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
        followPlayheadToggleButton.setBounds (0, getHeight() - kStatusBarHeight, 200, kStatusBarHeight);
        horizontalZoomInButton.setBounds (getWidth() - kStatusBarHeight, getHeight() - kStatusBarHeight, kStatusBarHeight, kStatusBarHeight);
        horizontalZoomOutButton.setBounds (horizontalZoomInButton.getBounds().translated (-kStatusBarHeight, 0));
        horizontalZoomLabel.setBounds (horizontalZoomOutButton.getBounds().translated (-kStatusBarHeight, 0));
        verticalZoomInButton.setBounds (horizontalZoomLabel.getBounds().translated (-kStatusBarHeight, 0));
        verticalZoomOutButton.setBounds (verticalZoomInButton.getBounds().translated (-kStatusBarHeight, 0));
        verticalZoomLabel.setBounds (verticalZoomOutButton.getBounds().translated (-kStatusBarHeight, 0));
    }
}

void ARASampleProjectAudioProcessorEditor::visibleTimeRangeChanged (Range<double> /*newVisibleTimeRange*/, double /*pixelsPerSecond*/)
{
    horizontalZoomInButton.setEnabled (documentView->isMinimumPixelsPerSecond());
    horizontalZoomOutButton.setEnabled (documentView->isMaximumPixelsPerSecond());
}
