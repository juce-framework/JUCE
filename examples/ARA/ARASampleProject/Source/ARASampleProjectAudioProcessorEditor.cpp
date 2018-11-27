#include "ARASampleProjectAudioProcessor.h"
#include "ARASampleProjectAudioProcessorEditor.h"


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
        getARAEditorView ()->addListener (this);

        for (auto regionSequence : document->getRegionSequences())
        {
            if (ARA::contains (getARAEditorView()->getHiddenRegionSequences(), regionSequence))
                continue;

            static_cast<ARARegionSequence*> (regionSequence)->addListener (this);
            regionSequenceViews.add (new RegionSequenceView (this, static_cast<ARARegionSequence*> (regionSequence)));
            regionSequenceListView.addAndMakeVisible (regionSequenceViews.getLast());
        }

        rebuildView();
    }
}

ARASampleProjectAudioProcessorEditor::~ARASampleProjectAudioProcessorEditor()
{
    if (isARAEditorView())
    {
        auto document = static_cast<ARADocument*> (getARADocumentController()->getDocument());
        document->removeListener (this);
        getARAEditorView ()->removeListener (this);

        for (auto regionSequence : document->getRegionSequences())
            static_cast<ARARegionSequence*> (regionSequence)->removeListener (this);
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

        double normalizedEnd = endInSeconds / kVisibleSeconds;
        v->setBounds (0, kRegionSequenceHeight * i, kTrackHeaderWidth + (int) (width * normalizedEnd), kRegionSequenceHeight);

        maxRegionSequenceLength = jmax (maxRegionSequenceLength, endInSeconds);
        i++;
    }

    // normalized width = view width in terms of kVisibleSeconds
    // size this to ensure we can see one second beyond the longest region sequnce
    const double normalizedWidth = (maxRegionSequenceLength + 1) / kVisibleSeconds;
    regionSequenceListView.setBounds (0, 0, kTrackHeaderWidth + (int) (normalizedWidth * width), kRegionSequenceHeight * i);
    regionSequenceViewPort.setBounds (0, 0, getWidth(), getHeight());
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

void ARASampleProjectAudioProcessorEditor::onHideRegionSequences (std::vector<ARARegionSequence*> const& regionSequences)
{
    std::vector<RegionSequenceView*> viewsToRemove;
    for (auto v : regionSequenceViews)
        if (ARA::contains (regionSequences, v->getRegionSequence ()))
            viewsToRemove.push_back (v);

    if (viewsToRemove.empty ())
        return;

    for (auto v : viewsToRemove)
        regionSequenceViews.removeObject (v, true);

    repaint ();
}

void ARASampleProjectAudioProcessorEditor::doEndEditing (ARADocument* document)
{
    for (auto regionSequence : document->getRegionSequences())
    {
        if (ARA::contains (getARAEditorView()->getHiddenRegionSequences(), regionSequence))
            continue;

        // TODO JUCE_ARA
        // we need a proper callback for when a region sequence is created
        // so we know when to make new views / subscribe to callbacks
        static_cast<ARARegionSequence*> (regionSequence)->addListener (this);

        // See if we need to make a new view - ideally we'd know when a new
        // region sequence is created and use that hook to make the view
        bool makeNewView = true;
        for (int i = 0; i < regionSequenceViews.size() && makeNewView; i++)
        {
            if (regionSequenceViews[i]->getRegionSequence() == regionSequence)
                makeNewView = false;
        }
        if (makeNewView)
        {
            regionSequenceViews.add (new RegionSequenceView (this, static_cast<ARARegionSequence*> (regionSequence)));
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
