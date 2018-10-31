/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "GraphEditorPanel.h"
#include "../Filters/InternalFilters.h"
#include "MainHostWindow.h"
#include "RackRow.h"
#include "../Performer.h"

//==============================================================================
#if JUCE_IOS
 class AUScanner
 {
 public:
     AUScanner (KnownPluginList& list)
         : knownPluginList (list), pool (5)
     {
         knownPluginList.clearBlacklistedFiles();
         paths = formatToScan.getDefaultLocationsToSearch();

         startScan();
     }

 private:
     KnownPluginList& knownPluginList;
     AudioUnitPluginFormat formatToScan;

     std::unique_ptr<PluginDirectoryScanner> scanner;
     FileSearchPath paths;

     ThreadPool pool;

     void startScan()
     {
         auto deadMansPedalFile = getAppProperties().getUserSettings()
                                     ->getFile().getSiblingFile ("RecentlyCrashedPluginsList");

         scanner.reset (new PluginDirectoryScanner (knownPluginList, formatToScan, paths,
                                                    true, deadMansPedalFile, true));

         for (int i = 5; --i >= 0;)
             pool.addJob (new ScanJob (*this), true);
     }

     bool doNextScan()
     {
         String pluginBeingScanned;
         if (scanner->scanNextFile (true, pluginBeingScanned))
             return true;

         return false;
     }

     struct ScanJob  : public ThreadPoolJob
     {
         ScanJob (AUScanner& s)  : ThreadPoolJob ("pluginscan"), scanner (s) {}

         JobStatus runJob()
         {
             while (scanner.doNextScan() && ! shouldExit())
             {}

             return jobHasFinished;
         }

         AUScanner& scanner;

         JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScanJob)
     };

     JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AUScanner)
 };
#endif

//==============================================================================
GraphEditorPanel::GraphEditorPanel (FilterGraph& g)  : graph (g)
{
    graph.addChangeListener (this);
    setOpaque (true);
}

GraphEditorPanel::~GraphEditorPanel()
{
    graph.removeChangeListener (this);
}

void GraphEditorPanel::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void GraphEditorPanel::mouseDown (const MouseEvent& e)
{
    if (isOnTouchDevice())
    {
        originalTouchPos = e.position.toInt();
        startTimer (750);
    }

    if (e.mods.isPopupMenu())
        showPopupMenu (e.position.toInt());
}

void GraphEditorPanel::mouseUp (const MouseEvent&)
{
    if (isOnTouchDevice())
    {
        stopTimer();
        callAfterDelay (250, []() { PopupMenu::dismissAllActiveMenus(); });
    }
}

void GraphEditorPanel::mouseDrag (const MouseEvent& e)
{
    if (isOnTouchDevice() && e.getDistanceFromDragStart() > 5)
        stopTimer();
}

void GraphEditorPanel::createNewPlugin (const PluginDescription& desc, Point<int> position)
{
    graph.addPlugin (desc, position.toDouble() / Point<double> ((double) getWidth(), (double) getHeight()));
}



void GraphEditorPanel::resized()
{
}

void GraphEditorPanel::changeListenerCallback (ChangeBroadcaster*)
{
}


void GraphEditorPanel::showPopupMenu (Point<int> mousePos)
{
    menu.reset (new PopupMenu);

    if (auto* mainWindow = findParentComponentOfClass<MainHostWindow>())
    {
        mainWindow->addPluginsToMenu (*menu);

        menu->showMenuAsync ({},
                             ModalCallbackFunction::create ([this, mousePos] (int r)
                                                            {
                                                                if (auto* mainWindow = findParentComponentOfClass<MainHostWindow>())
                                                                    if (auto* desc = mainWindow->getChosenType (r))
                                                                        createNewPlugin (*desc, mousePos);
                                                            }));
    }
}



void GraphEditorPanel::timerCallback()
{
    // this should only be called on touch devices
    jassert (isOnTouchDevice());

    stopTimer();
    showPopupMenu (originalTouchPos);
}

//==============================================================================
struct GraphDocumentComponent::TooltipBar   : public Component,
                                              private Timer
{
    TooltipBar()
    {
        startTimer (100);
    }

    void paint (Graphics& g) override
    {
        g.setFont (Font (getHeight() * 0.7f, Font::bold));
        g.setColour (Colours::black);
        g.drawFittedText (tip, 10, 0, getWidth() - 12, getHeight(), Justification::centredLeft, 1);
    }

    void timerCallback() override
    {
        String newTip;

        if (auto* underMouse = Desktop::getInstance().getMainMouseSource().getComponentUnderMouse())
            if (auto* ttc = dynamic_cast<TooltipClient*> (underMouse))
                if (! (underMouse->isMouseButtonDown() || underMouse->isCurrentlyBlockedByAnotherModalComponent()))
                    newTip = ttc->getTooltip();

        if (newTip != tip)
        {
            tip = newTip;
            repaint();
        }
    }

    String tip;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TooltipBar)
};

//==============================================================================
class GraphDocumentComponent::TitleBarComponent    : public Component,
                                                     private Button::Listener
{
public:
    TitleBarComponent (GraphDocumentComponent& graphDocumentComponent)
        : owner (graphDocumentComponent)
    {
        static const unsigned char burgerMenuPathData[]
            = { 110,109,0,0,128,64,0,0,32,65,108,0,0,224,65,0,0,32,65,98,254,212,232,65,0,0,32,65,0,0,240,65,252,
                169,17,65,0,0,240,65,0,0,0,65,98,0,0,240,65,8,172,220,64,254,212,232,65,0,0,192,64,0,0,224,65,0,0,
                192,64,108,0,0,128,64,0,0,192,64,98,16,88,57,64,0,0,192,64,0,0,0,64,8,172,220,64,0,0,0,64,0,0,0,65,
                98,0,0,0,64,252,169,17,65,16,88,57,64,0,0,32,65,0,0,128,64,0,0,32,65,99,109,0,0,224,65,0,0,96,65,108,
                0,0,128,64,0,0,96,65,98,16,88,57,64,0,0,96,65,0,0,0,64,4,86,110,65,0,0,0,64,0,0,128,65,98,0,0,0,64,
                254,212,136,65,16,88,57,64,0,0,144,65,0,0,128,64,0,0,144,65,108,0,0,224,65,0,0,144,65,98,254,212,232,
                65,0,0,144,65,0,0,240,65,254,212,136,65,0,0,240,65,0,0,128,65,98,0,0,240,65,4,86,110,65,254,212,232,
                65,0,0,96,65,0,0,224,65,0,0,96,65,99,109,0,0,224,65,0,0,176,65,108,0,0,128,64,0,0,176,65,98,16,88,57,
                64,0,0,176,65,0,0,0,64,2,43,183,65,0,0,0,64,0,0,192,65,98,0,0,0,64,254,212,200,65,16,88,57,64,0,0,208,
                65,0,0,128,64,0,0,208,65,108,0,0,224,65,0,0,208,65,98,254,212,232,65,0,0,208,65,0,0,240,65,254,212,
                200,65,0,0,240,65,0,0,192,65,98,0,0,240,65,2,43,183,65,254,212,232,65,0,0,176,65,0,0,224,65,0,0,176,
                65,99,101,0,0 };

        static const unsigned char pluginListPathData[]
            = { 110,109,193,202,222,64,80,50,21,64,108,0,0,48,65,0,0,0,0,108,160,154,112,65,80,50,21,64,108,0,0,48,65,80,
                50,149,64,108,193,202,222,64,80,50,21,64,99,109,0,0,192,64,251,220,127,64,108,160,154,32,65,165,135,202,
                64,108,160,154,32,65,250,220,47,65,108,0,0,192,64,102,144,10,65,108,0,0,192,64,251,220,127,64,99,109,0,0,
                128,65,251,220,127,64,108,0,0,128,65,103,144,10,65,108,96,101,63,65,251,220,47,65,108,96,101,63,65,166,135,
                202,64,108,0,0,128,65,251,220,127,64,99,109,96,101,79,65,148,76,69,65,108,0,0,136,65,0,0,32,65,108,80,
                77,168,65,148,76,69,65,108,0,0,136,65,40,153,106,65,108,96,101,79,65,148,76,69,65,99,109,0,0,64,65,63,247,
                95,65,108,80,77,128,65,233,161,130,65,108,80,77,128,65,125,238,167,65,108,0,0,64,65,51,72,149,65,108,0,0,64,
                65,63,247,95,65,99,109,0,0,176,65,63,247,95,65,108,0,0,176,65,51,72,149,65,108,176,178,143,65,125,238,167,65,
                108,176,178,143,65,233,161,130,65,108,0,0,176,65,63,247,95,65,99,109,12,86,118,63,148,76,69,65,108,0,0,160,
                64,0,0,32,65,108,159,154,16,65,148,76,69,65,108,0,0,160,64,40,153,106,65,108,12,86,118,63,148,76,69,65,99,
                109,0,0,0,0,63,247,95,65,108,62,53,129,64,233,161,130,65,108,62,53,129,64,125,238,167,65,108,0,0,0,0,51,
                72,149,65,108,0,0,0,0,63,247,95,65,99,109,0,0,32,65,63,247,95,65,108,0,0,32,65,51,72,149,65,108,193,202,190,
                64,125,238,167,65,108,193,202,190,64,233,161,130,65,108,0,0,32,65,63,247,95,65,99,101,0,0 };

        {
            Path p;
            p.loadPathFromData (burgerMenuPathData, sizeof (burgerMenuPathData));
            burgerButton.setShape (p, true, true, false);
        }

        {
            Path p;
            p.loadPathFromData (pluginListPathData, sizeof (pluginListPathData));
            pluginButton.setShape (p, true, true, false);
        }

        burgerButton.addListener (this);
        addAndMakeVisible (burgerButton);

        pluginButton.addListener (this);
        addAndMakeVisible (pluginButton);

        titleLabel.setJustificationType (Justification::centredLeft);
        addAndMakeVisible (titleLabel);

        setOpaque (true);
    }

private:
    void paint (Graphics& g) override
    {
        auto titleBarBackgroundColour = getLookAndFeel().findColour (ResizableWindow::backgroundColourId).darker();

        g.setColour (titleBarBackgroundColour);
        g.fillRect (getLocalBounds());
    }

    void resized() override
    {
        auto r = getLocalBounds();

        burgerButton.setBounds (r.removeFromLeft (40).withSizeKeepingCentre (20, 20));

        pluginButton.setBounds (r.removeFromRight (40).withSizeKeepingCentre (20, 20));

        titleLabel.setFont (Font (static_cast<float> (getHeight()) * 0.5f, Font::plain));
        titleLabel.setBounds (r);
    }

    void buttonClicked (Button* b) override
    {
        owner.showSidePanel (b == &burgerButton);
    }

    GraphDocumentComponent& owner;

    Label titleLabel {"titleLabel", "Plugin Host"};
    ShapeButton burgerButton {"burgerButton", Colours::lightgrey, Colours::lightgrey, Colours::white};
    ShapeButton pluginButton {"pluginButton", Colours::lightgrey, Colours::lightgrey, Colours::white};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TitleBarComponent)
};

//==============================================================================
struct GraphDocumentComponent::PluginListBoxModel    : public ListBoxModel,
                                                       public ChangeListener,
                                                       public MouseListener
{
    PluginListBoxModel (ListBox& lb, KnownPluginList& kpl)
        : owner (lb),
          knownPlugins (kpl)
    {
        knownPlugins.addChangeListener (this);
        owner.addMouseListener (this, true);

       #if JUCE_IOS
        scanner.reset (new AUScanner (knownPlugins));
       #endif
    }

    int getNumRows() override
    {
        return knownPlugins.getNumTypes();
    }

    void paintListBoxItem (int rowNumber, Graphics& g,
                           int width, int height, bool rowIsSelected) override
    {
        g.fillAll (rowIsSelected ? Colour (0xff42A2C8)
                                 : Colour (0xff263238));

        g.setColour (rowIsSelected ? Colours::black : Colours::white);

        if (rowNumber < knownPlugins.getNumTypes())
            g.drawFittedText (knownPlugins.getType (rowNumber)->name,
                              { 0, 0, width, height - 2 },
                              Justification::centred,
                              1);

        g.setColour (Colours::black.withAlpha (0.4f));
        g.drawRect (0, height - 1, width, 1);
    }

    var getDragSourceDescription (const SparseSet<int>& selectedRows) override
    {
        if (! isOverSelectedRow)
            return var();

        return String ("PLUGIN: " + String (selectedRows[0]));
    }

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        owner.updateContent();
    }

    void mouseDown (const MouseEvent& e) override
    {
        isOverSelectedRow = owner.getRowPosition (owner.getSelectedRow(), true)
                                 .contains (e.getEventRelativeTo (&owner).getMouseDownPosition());
    }

    ListBox& owner;
    KnownPluginList& knownPlugins;

    bool isOverSelectedRow = false;

   #if JUCE_IOS
    std::unique_ptr<AUScanner> scanner;
   #endif
};

//==============================================================================
GraphDocumentComponent::GraphDocumentComponent (AudioPluginFormatManager& fm,
                                                AudioDeviceManager& dm,
                                                KnownPluginList& kpl)
    : graph (new FilterGraph (fm)),
      deviceManager (dm),
      pluginList (kpl),
      graphPlayer (getAppProperties().getUserSettings()->getBoolValue ("doublePrecisionProcessing", false))
{
    init();

    deviceManager.addChangeListener (graphPanel.get());
    deviceManager.addAudioCallback (&graphPlayer);
    deviceManager.addMidiInputCallback (String(), &graphPlayer.getMidiMessageCollector());
}

void GraphDocumentComponent::init()
{
    graphPanel.reset (new GraphEditorPanel (*graph));
    addAndMakeVisible (graphPanel.get());
    graphPlayer.setProcessor (&graph->graph);

    m_volumeColumn.reset(new Label(String(),TRANS("Volume")));
    m_volumeColumn->setFont(Font(15.00f, Font::plain).withTypefaceStyle("Regular"));
    m_volumeColumn->setJustificationType(Justification::centredLeft);
    m_volumeColumn->setEditable(false, false, false);
    m_volumeColumn->setColour(TextEditor::textColourId, Colours::black);
    m_volumeColumn->setColour(TextEditor::backgroundColourId, Colour(0x00000000));
    m_volumeColumn->setBounds(144, 0, 64, 24);

    m_rangeColumn.reset(new Label(String(),TRANS("Range")));
    m_rangeColumn->setFont(Font(15.00f, Font::plain).withTypefaceStyle("Regular"));
    m_rangeColumn->setJustificationType(Justification::centredLeft);
    m_rangeColumn->setEditable(false, false, false);
    m_rangeColumn->setColour(TextEditor::textColourId, Colours::black);
    m_rangeColumn->setColour(TextEditor::backgroundColourId, Colour(0x00000000));
    m_rangeColumn->setBounds(736, 0, 56, 24);

    m_bankProgramColumn.reset(new Label(String(),TRANS("Bank/Program\n")));
    m_bankProgramColumn->setFont(Font(15.00f, Font::plain).withTypefaceStyle("Regular"));
    m_bankProgramColumn->setJustificationType(Justification::centredLeft);
    m_bankProgramColumn->setEditable(false, false, false);
    m_bankProgramColumn->setColour(TextEditor::textColourId, Colours::black);
    m_bankProgramColumn->setColour(TextEditor::backgroundColourId, Colour(0x00000000));
    m_bankProgramColumn->setBounds(264, 0, 104, 24);

    m_transposeColumn.reset(new Label(String(),TRANS("Transpose")));
    m_transposeColumn->setFont(Font(15.00f, Font::plain).withTypefaceStyle("Regular"));
    m_transposeColumn->setJustificationType(Justification::centredLeft);
    m_transposeColumn->setEditable(false, false, false);
    m_transposeColumn->setColour(TextEditor::textColourId, Colours::black);
    m_transposeColumn->setColour(TextEditor::backgroundColourId, Colour(0x00000000));
    m_transposeColumn->setBounds(632, 0, 80, 24);


    m_rackTopUI.reset(new Component());
    m_rackTopUI->addAndMakeVisible(m_bankProgramColumn.get());
    m_rackTopUI->addAndMakeVisible(m_transposeColumn.get());
    m_rackTopUI->addAndMakeVisible(m_rangeColumn.get());
    m_rackTopUI->addAndMakeVisible(m_volumeColumn.get());

    m_rackUI.reset(new Component());
    m_rackUI->addAndMakeVisible(m_rackTopUI.get());

    auto m_performer = new Performer();

    int devicesOnScreen = m_performer->m_current.Rack.size();
    int deviceWidth = 100;
    int deviceHeight = 20;
    int titleHeight = m_volumeColumn->getHeight() - 4;
    for (int i = 0; i < devicesOnScreen; ++i)
    {
        m_rackDevice.push_back(std::make_unique<RackRow>());
        auto newRackRow = m_rackDevice.back().get();

        deviceWidth = newRackRow->getWidth() + 2;
        deviceHeight = newRackRow->getHeight();
        m_rackUI->addAndMakeVisible(newRackRow);
        newRackRow->setBounds(0, i*deviceHeight + titleHeight, deviceWidth, deviceHeight);

    }
    m_rackTopUI->setBounds(0, 0, deviceWidth, titleHeight);


    m_tabs.reset(new TabbedComponent(TabbedButtonBar::TabsAtTop));
    m_tabs->setTabBarDepth(30);
    m_tabs->addTab(TRANS("SetLists"), Colours::darkblue, 0, false);
    m_tabs->addTab(TRANS("Songs"), Colours::darkgreen, 0, false);
    m_tabs->addTab(TRANS("Performances"), Colours::darkkhaki, 0, false);
    m_tabs->addTab(TRANS("Rack"), Colours::darkgrey, m_rackUI.get(), false);
    m_tabs->setCurrentTabIndex(3);
    m_tabs->setBounds(10,10, deviceWidth, deviceHeight * devicesOnScreen + titleHeight + 40); // include tab bar

    addAndMakeVisible(m_tabs.get());


    keyState.addListener (&graphPlayer.getMidiMessageCollector());

    keyboardComp.reset (new MidiKeyboardComponent (keyState, MidiKeyboardComponent::horizontalKeyboard));
    keyboardComp->setAvailableRange(21, 21 + 88 - 1);

    addAndMakeVisible (keyboardComp.get());
    statusBar.reset (new TooltipBar());
    addAndMakeVisible (statusBar.get());

    if (isOnTouchDevice())
    {
        if (isOnTouchDevice())
        {
            titleBarComponent.reset (new TitleBarComponent (*this));
            addAndMakeVisible (titleBarComponent.get());
        }

        pluginListBoxModel.reset (new PluginListBoxModel (pluginListBox, pluginList));

        pluginListBox.setModel (pluginListBoxModel.get());
        pluginListBox.setRowHeight (40);

        pluginListSidePanel.setContent (&pluginListBox, false);

        mobileSettingsSidePanel.setContent (new AudioDeviceSelectorComponent (deviceManager,
                                                                              0, 2, 0, 2,
                                                                              true, true, true, false));

        if (isOnTouchDevice())
        {
            addAndMakeVisible (pluginListSidePanel);
            addAndMakeVisible (mobileSettingsSidePanel);
        }
    }
}

GraphDocumentComponent::~GraphDocumentComponent()
{
    releaseGraph();

    keyState.removeListener (&graphPlayer.getMidiMessageCollector());
}

void GraphDocumentComponent::resized()
{
    auto r = getLocalBounds();
    const int titleBarHeight = 40;
    const int keysHeight = 60;
    const int statusHeight = 20;

    if (isOnTouchDevice())
        titleBarComponent->setBounds (r.removeFromTop(titleBarHeight));

    keyboardComp->setBounds (r.removeFromBottom (keysHeight));
    statusBar->setBounds (r.removeFromBottom (statusHeight));
    graphPanel->setBounds (r);

    checkAvailableWidth();
}

void GraphDocumentComponent::createNewPlugin (const PluginDescription& desc, Point<int> pos)
{
    graphPanel->createNewPlugin (desc, pos);
}

void GraphDocumentComponent::unfocusKeyboardComponent()
{
    keyboardComp->unfocusAllComponents();
}

void GraphDocumentComponent::releaseGraph()
{
    deviceManager.removeAudioCallback (&graphPlayer);
    deviceManager.removeMidiInputCallback (String(), &graphPlayer.getMidiMessageCollector());

    if (graphPanel != nullptr)
    {
        deviceManager.removeChangeListener (graphPanel.get());
        graphPanel = nullptr;
    }

    keyboardComp = nullptr;
    statusBar = nullptr;

    graphPlayer.setProcessor (nullptr);
    graph = nullptr;
}

bool GraphDocumentComponent::isInterestedInDragSource (const SourceDetails& details)
{
    return ((dynamic_cast<ListBox*> (details.sourceComponent.get()) != nullptr)
            && details.description.toString().startsWith ("PLUGIN"));
}

void GraphDocumentComponent::itemDropped (const SourceDetails& details)
{
    // don't allow items to be dropped behind the sidebar
    if (pluginListSidePanel.getBounds().contains (details.localPosition))
        return;

    auto pluginTypeIndex = details.description.toString()
                                 .fromFirstOccurrenceOf ("PLUGIN: ", false, false)
                                 .getIntValue();

    // must be a valid index!
    jassert (isPositiveAndBelow (pluginTypeIndex, pluginList.getNumTypes()));

    createNewPlugin (*pluginList.getType (pluginTypeIndex), details.localPosition);
}

void GraphDocumentComponent::showSidePanel (bool showSettingsPanel)
{
    if (showSettingsPanel)
        mobileSettingsSidePanel.showOrHide (true);
    else
        pluginListSidePanel.showOrHide (true);

    checkAvailableWidth();

    lastOpenedSidePanel = showSettingsPanel ? &mobileSettingsSidePanel
                                            : &pluginListSidePanel;
}

void GraphDocumentComponent::hideLastSidePanel()
{
    if (lastOpenedSidePanel != nullptr)
        lastOpenedSidePanel->showOrHide (false);

    if      (mobileSettingsSidePanel.isPanelShowing())    lastOpenedSidePanel = &mobileSettingsSidePanel;
    else if (pluginListSidePanel.isPanelShowing())        lastOpenedSidePanel = &pluginListSidePanel;
    else                                                  lastOpenedSidePanel = nullptr;
}

void GraphDocumentComponent::checkAvailableWidth()
{
    if (mobileSettingsSidePanel.isPanelShowing() && pluginListSidePanel.isPanelShowing())
    {
        if (getWidth() - (mobileSettingsSidePanel.getWidth() + pluginListSidePanel.getWidth()) < 150)
            hideLastSidePanel();
    }
}

void GraphDocumentComponent::setDoublePrecision (bool doublePrecision)
{
    graphPlayer.setDoublePrecisionProcessing (doublePrecision);
}

bool GraphDocumentComponent::closeAnyOpenPluginWindows()
{
    return graphPanel->graph.closeAnyOpenPluginWindows();
}
