#include "ARASampleProjectAudioProcessor.h"
#include "ARASampleProjectAudioProcessorEditor.h"

static const int kVisibleSeconds = 10;
static const int kMinWidth = 500;
static const int kWidth = 1000;
static const int kRegionSequenceHeight = 80;
static const int kMinHeight = 1 * kRegionSequenceHeight;
static const int kHeight = 5 * kRegionSequenceHeight;

//==============================================================================
ARASampleProjectAudioProcessorEditor::ARASampleProjectAudioProcessorEditor (ARASampleProjectAudioProcessor& p)
: AudioProcessorEditor (&p),
  AudioProcessorEditorARAExtension (&p),
  isViewDirty (false)
{
    // init viewport and region sequence list view
    regionSequenceViewPort.setScrollBarsShown (true, true);
    regionSequenceListView.setBounds (0, 0, kWidth - regionSequenceViewPort.getScrollBarThickness(), kHeight);
    regionSequenceViewPort.setViewedComponent (&regionSequenceListView, false);
    addAndMakeVisible (regionSequenceViewPort);
    
    setSize (kWidth, kHeight);
    setResizeLimits (kMinWidth, kMinHeight, 32768, 32768);
    setResizable (true, false);

    if (isARAEditorView())
    {
        auto document = static_cast<ARADocument*> (getARADocumentController()->getDocument());
        document->addListener (this);
        getARAEditorView()->addSelectionListener (this);

        for (auto regionSequence : document->getRegionSequences())
        {
            static_cast<ARARegionSequence*>(regionSequence)->addListener (this);
            regionSequenceViews.add (new RegionSequenceView (this, static_cast<ARARegionSequence*>(regionSequence)));
            regionSequenceListView.addAndMakeVisible (regionSequenceViews.getLast());
        }

        rebuildView();

        // manually invoke the onNewSelection callback to refresh our UI with the current selection
        // TODO JUCE_ARA should we rename the function that recreates the view?
        onNewSelection (getARAEditorView()->getViewSelection());
    }
}

ARASampleProjectAudioProcessorEditor::~ARASampleProjectAudioProcessorEditor()
{
    if (isARAEditorView())
    {
        auto document = static_cast<ARADocument*> (getARADocumentController()->getDocument());
        document->removeListener (this);
        getARAEditorView()->removeSelectionListener (this);

        for (auto regionSequence : document->getRegionSequences())
            static_cast<ARARegionSequence*>(regionSequence)->removeListener (this);
    }
}

//==============================================================================
void ARASampleProjectAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
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
    double maxRegionSequenceLength = 0.0;
    int i = 0;
    const int width = getWidth();

    // compute region sequence view bounds in terms of kVisibleSeconds and kRegionSequenceHeight
    // and determine the length in seconds of the longest ARA region sequence
    for (auto v : regionSequenceViews)
    {
        double startInSeconds (0), endInSeconds (0);
        v->getTimeRange (startInSeconds, endInSeconds);
        double lengthInSeconds = endInSeconds - startInSeconds;

        maxRegionSequenceLength = jmax (maxRegionSequenceLength, startInSeconds + lengthInSeconds);

        double normalizedStartPos = startInSeconds / kVisibleSeconds;
        double normalizedLength = lengthInSeconds / kVisibleSeconds;
        v->setBounds ((int) (width * normalizedStartPos), kRegionSequenceHeight * i, (int) (width * normalizedLength), kRegionSequenceHeight);
        i++;
    }

    // normalized width = view width in terms of kVisibleSeconds
    // size this to ensure we can see one second beyond the longest region sequnce
    const double normalizedWidth = (maxRegionSequenceLength + 1) / kVisibleSeconds;
    regionSequenceListView.setBounds (0, 0, (int) (normalizedWidth * width), kRegionSequenceHeight * i);
    regionSequenceViewPort.setBounds (0, 0, getWidth(), getHeight());
}

// rebuild our region sequence views and display selection state
void ARASampleProjectAudioProcessorEditor::onNewSelection (const ARA::PlugIn::ViewSelection& currentSelection)
{
    // flag the region as selected if it's a part of the current selection
    for (RegionSequenceView* regionSequenceView : regionSequenceViews)
    {
        bool isSelected = ARA::contains (currentSelection.getRegionSequences(), regionSequenceView->getRegionSequence());
        regionSequenceView->setIsSelected (isSelected);
    }
}

void ARASampleProjectAudioProcessorEditor::rebuildView()
{
    std::sort(regionSequenceViews.begin(), regionSequenceViews.end(),[] (RegionSequenceView* a, RegionSequenceView* b)
    {
        int orderA = a->getRegionSequence()->getOrderIndex();
        int orderB = b->getRegionSequence()->getOrderIndex();
        return std::less<int>() (orderA, orderB);
    });
    
    resized();
}

void ARASampleProjectAudioProcessorEditor::doEndEditing (ARADocument* document)
{
    for (auto regionSequence : document->getRegionSequences())
    {
        // TODO JUCE_ARA
        // we need a proper callback for when a region sequence is created
        // so we know when to make new views / subscribe to callbacks
        static_cast<ARARegionSequence*>(regionSequence)->addListener (this);

        // See if we need to make a new view - ideally we'd know when a new
        // region sequence is created and use that hook to make the view
        bool makeNewView = true;
        for (int i = 0; i < regionSequenceViews.size() && makeNewView; i++)
            if (regionSequenceViews[i]->getRegionSequence() == regionSequence)
                makeNewView = false;
        if (makeNewView)
        {
            regionSequenceViews.add (new RegionSequenceView (this, static_cast<ARARegionSequence*>(regionSequence)));
            regionSequenceListView.addAndMakeVisible (regionSequenceViews.getLast());
            setDirty();
        }
    }

    if (isViewDirty)
    {
        rebuildView();
        isViewDirty = false;
    }
}

void ARASampleProjectAudioProcessorEditor::didUpdateRegionSequenceProperties (ARARegionSequence* /*regionSequence*/)
{
    setDirty();
}

void ARASampleProjectAudioProcessorEditor::willDestroyRegionSequence (ARARegionSequence* regionSequence)
{
    for (int i = 0; i < regionSequenceViews.size(); i++)
    {
        if (regionSequenceViews[i]->getRegionSequence() == regionSequence)
        {
            regionSequenceViews.remove (i, true);
            return;
        }
    }
}
