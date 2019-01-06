#include "ARASampleProjectAudioProcessorEditor.h"

constexpr int kRulersViewHeight = 3*20;
constexpr int kTrackHeaderWidth = 120;
constexpr int kTrackHeight = 80;
constexpr int kStatusBarHeight = 20;
constexpr int kMinWidth = 3 * kTrackHeaderWidth;
constexpr int kWidth = 1000;
constexpr int kMinHeight = kRulersViewHeight + 1 * kTrackHeight + kStatusBarHeight;
constexpr int kHeight = kMinHeight + 5 * kTrackHeight;

//==============================================================================
ARASampleProjectAudioProcessorEditor::ARASampleProjectAudioProcessorEditor (ARASampleProjectAudioProcessor& p)
    : AudioProcessorEditor (&p),
      documentView (p)
{
    // TODO JUCE_ARA hotfix for Unicode chord symbols, see https://forum.juce.com/t/embedding-unicode-string-literals-in-your-cpp-files/12600/7
    documentView.getLookAndFeel().setDefaultSansSerifTypefaceName("Arial Unicode MS");
    documentView.setCurrentPositionInfo (&p.getLastKnownPositionInfo());
    addAndMakeVisible (documentView);

    setSize (kWidth, kHeight);
    setResizeLimits (kMinWidth, kMinHeight, 32768, 32768);
    setResizable (true, false);
}

//==============================================================================
void ARASampleProjectAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void ARASampleProjectAudioProcessorEditor::resized()
{
    documentView.setBounds (getBounds());
}
