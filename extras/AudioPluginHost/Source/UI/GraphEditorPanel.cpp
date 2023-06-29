/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include <JuceHeader.h>
#include "GraphEditorPanel.h"
#include "../Plugins/InternalPlugins.h"
#include "MainHostWindow.h"

//==============================================================================
#if JUCE_IOS
 class AUScanner
 {
 public:
     explicit AUScanner (KnownPluginList& list)
         : knownPluginList (list)
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

     static constexpr auto numJobs = 5;
     ThreadPool pool { ThreadPoolOptions{}.withNumberOfThreads (numJobs) };

     void startScan()
     {
         auto deadMansPedalFile = getAppProperties().getUserSettings()
                                     ->getFile().getSiblingFile ("RecentlyCrashedPluginsList");

         scanner.reset (new PluginDirectoryScanner (knownPluginList, formatToScan, paths,
                                                    true, deadMansPedalFile, true));

         for (int i = numJobs; --i >= 0;)
             pool.addJob (new ScanJob (*this), true);
     }

     bool doNextScan()
     {
         String pluginBeingScanned;
         return scanner->scanNextFile (true, pluginBeingScanned);
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
struct GraphEditorPanel::PinComponent   : public Component,
                                          public SettableTooltipClient
{
    PinComponent (GraphEditorPanel& p, AudioProcessorGraph::NodeAndChannel pinToUse, bool isIn)
        : panel (p), graph (p.graph), pin (pinToUse), isInput (isIn)
    {
        if (auto node = graph.graph.getNodeForId (pin.nodeID))
        {
            String tip;

            if (pin.isMIDI())
            {
                tip = isInput ? "MIDI Input"
                              : "MIDI Output";
            }
            else
            {
                auto& processor = *node->getProcessor();
                auto channel = processor.getOffsetInBusBufferForAbsoluteChannelIndex (isInput, pin.channelIndex, busIdx);

                if (auto* bus = processor.getBus (isInput, busIdx))
                    tip = bus->getName() + ": " + AudioChannelSet::getAbbreviatedChannelTypeName (bus->getCurrentLayout().getTypeOfChannel (channel));
                else
                    tip = (isInput ? "Main Input: "
                                   : "Main Output: ") + String (pin.channelIndex + 1);

            }

            setTooltip (tip);
        }

        setSize (16, 16);
    }

    void paint (Graphics& g) override
    {
        auto w = (float) getWidth();
        auto h = (float) getHeight();

        Path p;
        p.addEllipse (w * 0.25f, h * 0.25f, w * 0.5f, h * 0.5f);
        p.addRectangle (w * 0.4f, isInput ? (0.5f * h) : 0.0f, w * 0.2f, h * 0.5f);

        auto colour = (pin.isMIDI() ? Colours::red : Colours::green);

        g.setColour (colour.withRotatedHue ((float) busIdx / 5.0f));
        g.fillPath (p);
    }

    void mouseDown (const MouseEvent& e) override
    {
        AudioProcessorGraph::NodeAndChannel dummy { {}, 0 };

        panel.beginConnectorDrag (isInput ? dummy : pin,
                                  isInput ? pin : dummy,
                                  e);
    }

    void mouseDrag (const MouseEvent& e) override
    {
        panel.dragConnector (e);
    }

    void mouseUp (const MouseEvent& e) override
    {
        panel.endDraggingConnector (e);
    }

    GraphEditorPanel& panel;
    PluginGraph& graph;
    AudioProcessorGraph::NodeAndChannel pin;
    const bool isInput;
    int busIdx = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PinComponent)
};

//==============================================================================
struct GraphEditorPanel::PluginComponent   : public Component,
                                             public Timer,
                                             private AudioProcessorParameter::Listener,
                                             private AsyncUpdater
{
    PluginComponent (GraphEditorPanel& p, AudioProcessorGraph::NodeID id)  : panel (p), graph (p.graph), pluginID (id)
    {
        shadow.setShadowProperties (DropShadow (Colours::black.withAlpha (0.5f), 3, { 0, 1 }));
        setComponentEffect (&shadow);

        if (auto f = graph.graph.getNodeForId (pluginID))
        {
            if (auto* processor = f->getProcessor())
            {
                if (auto* bypassParam = processor->getBypassParameter())
                    bypassParam->addListener (this);
            }
        }

        setSize (150, 60);
    }

    PluginComponent (const PluginComponent&) = delete;
    PluginComponent& operator= (const PluginComponent&) = delete;

    ~PluginComponent() override
    {
        if (auto f = graph.graph.getNodeForId (pluginID))
        {
            if (auto* processor = f->getProcessor())
            {
                if (auto* bypassParam = processor->getBypassParameter())
                    bypassParam->removeListener (this);
            }
        }
    }

    void mouseDown (const MouseEvent& e) override
    {
        originalPos = localPointToGlobal (Point<int>());

        toFront (true);

        if (isOnTouchDevice())
        {
            startTimer (750);
        }
        else
        {
            if (e.mods.isPopupMenu())
                showPopupMenu();
        }
    }

    void mouseDrag (const MouseEvent& e) override
    {
        if (isOnTouchDevice() && e.getDistanceFromDragStart() > 5)
            stopTimer();

        if (! e.mods.isPopupMenu())
        {
            auto pos = originalPos + e.getOffsetFromDragStart();

            if (getParentComponent() != nullptr)
                pos = getParentComponent()->getLocalPoint (nullptr, pos);

            pos += getLocalBounds().getCentre();

            graph.setNodePosition (pluginID,
                                   { pos.x / (double) getParentWidth(),
                                     pos.y / (double) getParentHeight() });

            panel.updateComponents();
        }
    }

    void mouseUp (const MouseEvent& e) override
    {
        if (isOnTouchDevice())
        {
            stopTimer();
            callAfterDelay (250, []() { PopupMenu::dismissAllActiveMenus(); });
        }

        if (e.mouseWasDraggedSinceMouseDown())
        {
            graph.setChangedFlag (true);
        }
        else if (e.getNumberOfClicks() == 2)
        {
            if (auto f = graph.graph.getNodeForId (pluginID))
                if (auto* w = graph.getOrCreateWindowFor (f, PluginWindow::Type::normal))
                    w->toFront (true);
        }
    }

    bool hitTest (int x, int y) override
    {
        for (auto* child : getChildren())
            if (child->getBounds().contains (x, y))
                return true;

        return x >= 3 && x < getWidth() - 6 && y >= pinSize && y < getHeight() - pinSize;
    }

    void paint (Graphics& g) override
    {
        auto boxArea = getLocalBounds().reduced (4, pinSize);
        bool isBypassed = false;

        if (auto* f = graph.graph.getNodeForId (pluginID))
            isBypassed = f->isBypassed();

        auto boxColour = findColour (TextEditor::backgroundColourId);

        if (isBypassed)
            boxColour = boxColour.brighter();

        g.setColour (boxColour);
        g.fillRect (boxArea.toFloat());

        g.setColour (findColour (TextEditor::textColourId));
        g.setFont (font);
        g.drawFittedText (getName(), boxArea, Justification::centred, 2);
    }

    void resized() override
    {
        if (auto f = graph.graph.getNodeForId (pluginID))
        {
            if (auto* processor = f->getProcessor())
            {
                for (auto* pin : pins)
                {
                    const bool isInput = pin->isInput;
                    auto channelIndex = pin->pin.channelIndex;
                    int busIdx = 0;
                    processor->getOffsetInBusBufferForAbsoluteChannelIndex (isInput, channelIndex, busIdx);

                    const int total = isInput ? numIns : numOuts;
                    const int index = pin->pin.isMIDI() ? (total - 1) : channelIndex;

                    auto totalSpaces = static_cast<float> (total) + (static_cast<float> (jmax (0, processor->getBusCount (isInput) - 1)) * 0.5f);
                    auto indexPos = static_cast<float> (index) + (static_cast<float> (busIdx) * 0.5f);

                    pin->setBounds (proportionOfWidth ((1.0f + indexPos) / (totalSpaces + 1.0f)) - pinSize / 2,
                                    pin->isInput ? 0 : (getHeight() - pinSize),
                                    pinSize, pinSize);
                }
            }
        }
    }

    Point<float> getPinPos (int index, bool isInput) const
    {
        for (auto* pin : pins)
            if (pin->pin.channelIndex == index && isInput == pin->isInput)
                return getPosition().toFloat() + pin->getBounds().getCentre().toFloat();

        return {};
    }

    void update()
    {
        const AudioProcessorGraph::Node::Ptr f (graph.graph.getNodeForId (pluginID));
        jassert (f != nullptr);

        auto& processor = *f->getProcessor();

        numIns = processor.getTotalNumInputChannels();
        if (processor.acceptsMidi())
            ++numIns;

        numOuts = processor.getTotalNumOutputChannels();
        if (processor.producesMidi())
            ++numOuts;

        int w = 100;
        int h = 60;

        w = jmax (w, (jmax (numIns, numOuts) + 1) * 20);

        const int textWidth = font.getStringWidth (processor.getName());
        w = jmax (w, 16 + jmin (textWidth, 300));
        if (textWidth > 300)
            h = 100;

        setSize (w, h);
        setName (processor.getName() + formatSuffix);

        {
            auto p = graph.getNodePosition (pluginID);
            setCentreRelative ((float) p.x, (float) p.y);
        }

        if (numIns != numInputs || numOuts != numOutputs)
        {
            numInputs = numIns;
            numOutputs = numOuts;

            pins.clear();

            for (int i = 0; i < processor.getTotalNumInputChannels(); ++i)
                addAndMakeVisible (pins.add (new PinComponent (panel, { pluginID, i }, true)));

            if (processor.acceptsMidi())
                addAndMakeVisible (pins.add (new PinComponent (panel, { pluginID, AudioProcessorGraph::midiChannelIndex }, true)));

            for (int i = 0; i < processor.getTotalNumOutputChannels(); ++i)
                addAndMakeVisible (pins.add (new PinComponent (panel, { pluginID, i }, false)));

            if (processor.producesMidi())
                addAndMakeVisible (pins.add (new PinComponent (panel, { pluginID, AudioProcessorGraph::midiChannelIndex }, false)));

            resized();
        }
    }

    AudioProcessor* getProcessor() const
    {
        if (auto node = graph.graph.getNodeForId (pluginID))
            return node->getProcessor();

        return {};
    }

    bool isNodeUsingARA() const
    {
        if (auto node = graph.graph.getNodeForId (pluginID))
            return node->properties["useARA"];

        return false;
    }

    void showPopupMenu()
    {
        menu.reset (new PopupMenu);
        menu->addItem ("Delete this filter", [this] { graph.graph.removeNode (pluginID); });
        menu->addItem ("Disconnect all pins", [this] { graph.graph.disconnectNode (pluginID); });
        menu->addItem ("Toggle Bypass", [this]
        {
            if (auto* node = graph.graph.getNodeForId (pluginID))
                node->setBypassed (! node->isBypassed());

            repaint();
        });

        menu->addSeparator();
        if (getProcessor()->hasEditor())
            menu->addItem ("Show plugin GUI", [this] { showWindow (PluginWindow::Type::normal); });

        menu->addItem ("Show all programs", [this] { showWindow (PluginWindow::Type::programs); });
        menu->addItem ("Show all parameters", [this] { showWindow (PluginWindow::Type::generic); });
        menu->addItem ("Show debug log", [this] { showWindow (PluginWindow::Type::debug); });

       #if JUCE_PLUGINHOST_ARA && (JUCE_MAC || JUCE_WINDOWS || JUCE_LINUX)
        if (auto* instance = dynamic_cast<AudioPluginInstance*> (getProcessor()))
            if (instance->getPluginDescription().hasARAExtension && isNodeUsingARA())
                menu->addItem ("Show ARA host controls", [this] { showWindow (PluginWindow::Type::araHost); });
       #endif

        if (autoScaleOptionAvailable)
            addPluginAutoScaleOptionsSubMenu (dynamic_cast<AudioPluginInstance*> (getProcessor()), *menu);

        menu->addSeparator();
        menu->addItem ("Configure Audio I/O", [this] { showWindow (PluginWindow::Type::audioIO); });
        menu->addItem ("Test state save/load", [this] { testStateSaveLoad(); });

       #if ! JUCE_IOS && ! JUCE_ANDROID
        menu->addSeparator();
        menu->addItem ("Save plugin state", [this] { savePluginState(); });
        menu->addItem ("Load plugin state", [this] { loadPluginState(); });
       #endif

        menu->showMenuAsync ({});
    }

    void testStateSaveLoad()
    {
        if (auto* processor = getProcessor())
        {
            MemoryBlock state;
            processor->getStateInformation (state);
            processor->setStateInformation (state.getData(), (int) state.getSize());
        }
    }

    void showWindow (PluginWindow::Type type)
    {
        if (auto node = graph.graph.getNodeForId (pluginID))
            if (auto* w = graph.getOrCreateWindowFor (node, type))
                w->toFront (true);
    }

    void timerCallback() override
    {
        // this should only be called on touch devices
        jassert (isOnTouchDevice());

        stopTimer();
        showPopupMenu();
    }

    void parameterValueChanged (int, float) override
    {
        // Parameter changes might come from the audio thread or elsewhere, but
        // we can only call repaint from the message thread.
        triggerAsyncUpdate();
    }

    void parameterGestureChanged (int, bool) override  {}

    void handleAsyncUpdate() override { repaint(); }

    void savePluginState()
    {
        fileChooser = std::make_unique<FileChooser> ("Save plugin state");

        const auto onChosen = [ref = SafePointer<PluginComponent> (this)] (const FileChooser& chooser)
        {
            if (ref == nullptr)
                return;

            const auto result = chooser.getResult();

            if (result == File())
                return;

            if (auto* node = ref->graph.graph.getNodeForId (ref->pluginID))
            {
                MemoryBlock block;
                node->getProcessor()->getStateInformation (block);
                result.replaceWithData (block.getData(), block.getSize());
            }
        };

        fileChooser->launchAsync (FileBrowserComponent::saveMode | FileBrowserComponent::warnAboutOverwriting, onChosen);
    }

    void loadPluginState()
    {
        fileChooser = std::make_unique<FileChooser> ("Load plugin state");

        const auto onChosen = [ref = SafePointer<PluginComponent> (this)] (const FileChooser& chooser)
        {
            if (ref == nullptr)
                return;

            const auto result = chooser.getResult();

            if (result == File())
                return;

            if (auto* node = ref->graph.graph.getNodeForId (ref->pluginID))
            {
                if (auto stream = result.createInputStream())
                {
                    MemoryBlock block;
                    stream->readIntoMemoryBlock (block);
                    node->getProcessor()->setStateInformation (block.getData(), (int) block.getSize());
                }
            }
        };

        fileChooser->launchAsync (FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles, onChosen);
    }

    GraphEditorPanel& panel;
    PluginGraph& graph;
    const AudioProcessorGraph::NodeID pluginID;
    OwnedArray<PinComponent> pins;
    int numInputs = 0, numOutputs = 0;
    int pinSize = 16;
    Point<int> originalPos;
    Font font { 13.0f, Font::bold };
    int numIns = 0, numOuts = 0;
    DropShadowEffect shadow;
    std::unique_ptr<PopupMenu> menu;
    std::unique_ptr<FileChooser> fileChooser;
    const String formatSuffix = getFormatSuffix (getProcessor());
};


//==============================================================================
struct GraphEditorPanel::ConnectorComponent   : public Component,
                                                public SettableTooltipClient
{
    explicit ConnectorComponent (GraphEditorPanel& p)
        : panel (p), graph (p.graph)
    {
        setAlwaysOnTop (true);
    }

    void setInput (AudioProcessorGraph::NodeAndChannel newSource)
    {
        if (connection.source != newSource)
        {
            connection.source = newSource;
            update();
        }
    }

    void setOutput (AudioProcessorGraph::NodeAndChannel newDest)
    {
        if (connection.destination != newDest)
        {
            connection.destination = newDest;
            update();
        }
    }

    void dragStart (Point<float> pos)
    {
        lastInputPos = pos;
        resizeToFit();
    }

    void dragEnd (Point<float> pos)
    {
        lastOutputPos = pos;
        resizeToFit();
    }

    void update()
    {
        Point<float> p1, p2;
        getPoints (p1, p2);

        if (lastInputPos != p1 || lastOutputPos != p2)
            resizeToFit();
    }

    void resizeToFit()
    {
        Point<float> p1, p2;
        getPoints (p1, p2);

        auto newBounds = Rectangle<float> (p1, p2).expanded (4.0f).getSmallestIntegerContainer();

        if (newBounds != getBounds())
            setBounds (newBounds);
        else
            resized();

        repaint();
    }

    void getPoints (Point<float>& p1, Point<float>& p2) const
    {
        p1 = lastInputPos;
        p2 = lastOutputPos;

        if (auto* src = panel.getComponentForPlugin (connection.source.nodeID))
            p1 = src->getPinPos (connection.source.channelIndex, false);

        if (auto* dest = panel.getComponentForPlugin (connection.destination.nodeID))
            p2 = dest->getPinPos (connection.destination.channelIndex, true);
    }

    void paint (Graphics& g) override
    {
        if (connection.source.isMIDI() || connection.destination.isMIDI())
            g.setColour (Colours::red);
        else
            g.setColour (Colours::green);

        g.fillPath (linePath);
    }

    bool hitTest (int x, int y) override
    {
        auto pos = Point<int> (x, y).toFloat();

        if (hitPath.contains (pos))
        {
            double distanceFromStart, distanceFromEnd;
            getDistancesFromEnds (pos, distanceFromStart, distanceFromEnd);

            // avoid clicking the connector when over a pin
            return distanceFromStart > 7.0 && distanceFromEnd > 7.0;
        }

        return false;
    }

    void mouseDown (const MouseEvent&) override
    {
        dragging = false;
    }

    void mouseDrag (const MouseEvent& e) override
    {
        if (dragging)
        {
            panel.dragConnector (e);
        }
        else if (e.mouseWasDraggedSinceMouseDown())
        {
            dragging = true;

            graph.graph.removeConnection (connection);

            double distanceFromStart, distanceFromEnd;
            getDistancesFromEnds (getPosition().toFloat() + e.position, distanceFromStart, distanceFromEnd);
            const bool isNearerSource = (distanceFromStart < distanceFromEnd);

            AudioProcessorGraph::NodeAndChannel dummy { {}, 0 };

            panel.beginConnectorDrag (isNearerSource ? dummy : connection.source,
                                      isNearerSource ? connection.destination : dummy,
                                      e);
        }
    }

    void mouseUp (const MouseEvent& e) override
    {
        if (dragging)
            panel.endDraggingConnector (e);
    }

    void resized() override
    {
        Point<float> p1, p2;
        getPoints (p1, p2);

        lastInputPos = p1;
        lastOutputPos = p2;

        p1 -= getPosition().toFloat();
        p2 -= getPosition().toFloat();

        linePath.clear();
        linePath.startNewSubPath (p1);
        linePath.cubicTo (p1.x, p1.y + (p2.y - p1.y) * 0.33f,
                          p2.x, p1.y + (p2.y - p1.y) * 0.66f,
                          p2.x, p2.y);

        PathStrokeType wideStroke (8.0f);
        wideStroke.createStrokedPath (hitPath, linePath);

        PathStrokeType stroke (2.5f);
        stroke.createStrokedPath (linePath, linePath);

        auto arrowW = 5.0f;
        auto arrowL = 4.0f;

        Path arrow;
        arrow.addTriangle (-arrowL, arrowW,
                           -arrowL, -arrowW,
                           arrowL, 0.0f);

        arrow.applyTransform (AffineTransform()
                                .rotated (MathConstants<float>::halfPi - (float) atan2 (p2.x - p1.x, p2.y - p1.y))
                                .translated ((p1 + p2) * 0.5f));

        linePath.addPath (arrow);
        linePath.setUsingNonZeroWinding (true);
    }

    void getDistancesFromEnds (Point<float> p, double& distanceFromStart, double& distanceFromEnd) const
    {
        Point<float> p1, p2;
        getPoints (p1, p2);

        distanceFromStart = p1.getDistanceFrom (p);
        distanceFromEnd   = p2.getDistanceFrom (p);
    }

    GraphEditorPanel& panel;
    PluginGraph& graph;
    AudioProcessorGraph::Connection connection { { {}, 0 }, { {}, 0 } };
    Point<float> lastInputPos, lastOutputPos;
    Path linePath, hitPath;
    bool dragging = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConnectorComponent)
};


//==============================================================================
GraphEditorPanel::GraphEditorPanel (PluginGraph& g)  : graph (g)
{
    graph.addChangeListener (this);
    setOpaque (true);
}

GraphEditorPanel::~GraphEditorPanel()
{
    graph.removeChangeListener (this);
    draggingConnector = nullptr;
    nodes.clear();
    connectors.clear();
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

void GraphEditorPanel::createNewPlugin (const PluginDescriptionAndPreference& desc, Point<int> position)
{
    graph.addPlugin (desc, position.toDouble() / Point<double> ((double) getWidth(), (double) getHeight()));
}

GraphEditorPanel::PluginComponent* GraphEditorPanel::getComponentForPlugin (AudioProcessorGraph::NodeID nodeID) const
{
    for (auto* fc : nodes)
       if (fc->pluginID == nodeID)
            return fc;

    return nullptr;
}

GraphEditorPanel::ConnectorComponent* GraphEditorPanel::getComponentForConnection (const AudioProcessorGraph::Connection& conn) const
{
    for (auto* cc : connectors)
        if (cc->connection == conn)
            return cc;

    return nullptr;
}

GraphEditorPanel::PinComponent* GraphEditorPanel::findPinAt (Point<float> pos) const
{
    for (auto* fc : nodes)
    {
        // NB: A Visual Studio optimiser error means we have to put this Component* in a local
        // variable before trying to cast it, or it gets mysteriously optimised away..
        auto* comp = fc->getComponentAt (pos.toInt() - fc->getPosition());

        if (auto* pin = dynamic_cast<PinComponent*> (comp))
            return pin;
    }

    return nullptr;
}

void GraphEditorPanel::resized()
{
    updateComponents();
}

void GraphEditorPanel::changeListenerCallback (ChangeBroadcaster*)
{
    updateComponents();
}

void GraphEditorPanel::updateComponents()
{
    for (int i = nodes.size(); --i >= 0;)
        if (graph.graph.getNodeForId (nodes.getUnchecked(i)->pluginID) == nullptr)
            nodes.remove (i);

    for (int i = connectors.size(); --i >= 0;)
        if (! graph.graph.isConnected (connectors.getUnchecked(i)->connection))
            connectors.remove (i);

    for (auto* fc : nodes)
        fc->update();

    for (auto* cc : connectors)
        cc->update();

    for (auto* f : graph.graph.getNodes())
    {
        if (getComponentForPlugin (f->nodeID) == nullptr)
        {
            auto* comp = nodes.add (new PluginComponent (*this, f->nodeID));
            addAndMakeVisible (comp);
            comp->update();
        }
    }

    for (auto& c : graph.graph.getConnections())
    {
        if (getComponentForConnection (c) == nullptr)
        {
            auto* comp = connectors.add (new ConnectorComponent (*this));
            addAndMakeVisible (comp);

            comp->setInput (c.source);
            comp->setOutput (c.destination);
        }
    }
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
                                                                if (auto* mainWin = findParentComponentOfClass<MainHostWindow>())
                                                                    if (const auto chosen = mainWin->getChosenType (r))
                                                                        createNewPlugin (*chosen, mousePos);
                                                            }));
    }
}

void GraphEditorPanel::beginConnectorDrag (AudioProcessorGraph::NodeAndChannel source,
                                           AudioProcessorGraph::NodeAndChannel dest,
                                           const MouseEvent& e)
{
    auto* c = dynamic_cast<ConnectorComponent*> (e.originalComponent);
    connectors.removeObject (c, false);
    draggingConnector.reset (c);

    if (draggingConnector == nullptr)
        draggingConnector.reset (new ConnectorComponent (*this));

    draggingConnector->setInput (source);
    draggingConnector->setOutput (dest);

    addAndMakeVisible (draggingConnector.get());
    draggingConnector->toFront (false);

    dragConnector (e);
}

void GraphEditorPanel::dragConnector (const MouseEvent& e)
{
    auto e2 = e.getEventRelativeTo (this);

    if (draggingConnector != nullptr)
    {
        draggingConnector->setTooltip ({});

        auto pos = e2.position;

        if (auto* pin = findPinAt (pos))
        {
            auto connection = draggingConnector->connection;

            if (connection.source.nodeID == AudioProcessorGraph::NodeID() && ! pin->isInput)
            {
                connection.source = pin->pin;
            }
            else if (connection.destination.nodeID == AudioProcessorGraph::NodeID() && pin->isInput)
            {
                connection.destination = pin->pin;
            }

            if (graph.graph.canConnect (connection))
            {
                pos = (pin->getParentComponent()->getPosition() + pin->getBounds().getCentre()).toFloat();
                draggingConnector->setTooltip (pin->getTooltip());
            }
        }

        if (draggingConnector->connection.source.nodeID == AudioProcessorGraph::NodeID())
            draggingConnector->dragStart (pos);
        else
            draggingConnector->dragEnd (pos);
    }
}

void GraphEditorPanel::endDraggingConnector (const MouseEvent& e)
{
    if (draggingConnector == nullptr)
        return;

    draggingConnector->setTooltip ({});

    auto e2 = e.getEventRelativeTo (this);
    auto connection = draggingConnector->connection;

    draggingConnector = nullptr;

    if (auto* pin = findPinAt (e2.position))
    {
        if (connection.source.nodeID == AudioProcessorGraph::NodeID())
        {
            if (pin->isInput)
                return;

            connection.source = pin->pin;
        }
        else
        {
            if (! pin->isInput)
                return;

            connection.destination = pin->pin;
        }

        graph.graph.addConnection (connection);
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
        g.setFont (Font ((float) getHeight() * 0.7f, Font::bold));
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
    explicit TitleBarComponent (GraphDocumentComponent& graphDocumentComponent)
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
            g.drawFittedText (knownPlugins.getTypes()[rowNumber].name, { 0, 0, width, height - 2 }, Justification::centred, 1);

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

    JUCE_DECLARE_NON_COPYABLE (PluginListBoxModel)
};

//==============================================================================
GraphDocumentComponent::GraphDocumentComponent (AudioPluginFormatManager& fm,
                                                AudioDeviceManager& dm,
                                                KnownPluginList& kpl)
    : graph (new PluginGraph (fm, kpl)),
      deviceManager (dm),
      pluginList (kpl),
      graphPlayer (getAppProperties().getUserSettings()->getBoolValue ("doublePrecisionProcessing", false))
{
    init();

    deviceManager.addChangeListener (graphPanel.get());
    deviceManager.addAudioCallback (&graphPlayer);
    deviceManager.addMidiInputDeviceCallback ({}, &graphPlayer.getMidiMessageCollector());
    deviceManager.addChangeListener (this);
}

void GraphDocumentComponent::init()
{
    updateMidiOutput();

    graphPanel.reset (new GraphEditorPanel (*graph));
    addAndMakeVisible (graphPanel.get());
    graphPlayer.setProcessor (&graph->graph);

    keyState.addListener (&graphPlayer.getMidiMessageCollector());

    keyboardComp.reset (new MidiKeyboardComponent (keyState, MidiKeyboardComponent::horizontalKeyboard));
    addAndMakeVisible (keyboardComp.get());
    statusBar.reset (new TooltipBar());
    addAndMakeVisible (statusBar.get());

    graphPanel->updateComponents();

    if (isOnTouchDevice())
    {
        titleBarComponent.reset (new TitleBarComponent (*this));
        addAndMakeVisible (titleBarComponent.get());

        pluginListBoxModel.reset (new PluginListBoxModel (pluginListBox, pluginList));

        pluginListBox.setModel (pluginListBoxModel.get());
        pluginListBox.setRowHeight (40);

        pluginListSidePanel.setContent (&pluginListBox, false);

        mobileSettingsSidePanel.setContent (new AudioDeviceSelectorComponent (deviceManager,
                                                                              0, 2, 0, 2,
                                                                              true, true, true, false));

        addAndMakeVisible (pluginListSidePanel);
        addAndMakeVisible (mobileSettingsSidePanel);
    }
}

GraphDocumentComponent::~GraphDocumentComponent()
{
    if (midiOutput != nullptr)
        midiOutput->stopBackgroundThread();

    releaseGraph();

    keyState.removeListener (&graphPlayer.getMidiMessageCollector());
}

void GraphDocumentComponent::resized()
{
    auto r = [this]
    {
        auto bounds = getLocalBounds();

        if (auto* display = Desktop::getInstance().getDisplays().getDisplayForRect (getScreenBounds()))
            return display->safeAreaInsets.subtractedFrom (bounds);

        return bounds;
    }();

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

void GraphDocumentComponent::createNewPlugin (const PluginDescriptionAndPreference& desc, Point<int> pos)
{
    graphPanel->createNewPlugin (desc, pos);
}

void GraphDocumentComponent::releaseGraph()
{
    deviceManager.removeAudioCallback (&graphPlayer);
    deviceManager.removeMidiInputDeviceCallback ({}, &graphPlayer.getMidiMessageCollector());

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

    createNewPlugin (PluginDescriptionAndPreference { pluginList.getTypes()[pluginTypeIndex] },
                     details.localPosition);
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

void GraphDocumentComponent::changeListenerCallback (ChangeBroadcaster*)
{
    updateMidiOutput();
}

void GraphDocumentComponent::updateMidiOutput()
{
    auto* defaultMidiOutput = deviceManager.getDefaultMidiOutput();

    if (midiOutput != defaultMidiOutput)
    {
        midiOutput = defaultMidiOutput;

        if (midiOutput != nullptr)
            midiOutput->startBackgroundThread();

        graphPlayer.setMidiOutput (midiOutput);
    }
}
