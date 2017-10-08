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
#include "InternalFilters.h"
#include "MainHostWindow.h"
#include "FilterIOConfiguration.h"


//==============================================================================
static Array<PluginWindow*> activePluginWindows;

PluginWindow::PluginWindow (AudioProcessorEditor* pluginEditor, AudioProcessorGraph::Node* o, WindowFormatType t)
    : DocumentWindow (pluginEditor->getName(),
                      LookAndFeel::getDefaultLookAndFeel().findColour (ResizableWindow::backgroundColourId),
                      DocumentWindow::minimiseButton | DocumentWindow::closeButton),
      owner (o),
      type (t)
{
    setSize (400, 300);

    setContentOwned (pluginEditor, true);

    setTopLeftPosition (owner->properties.getWithDefault (getLastXProp (type), Random::getSystemRandom().nextInt (500)),
                        owner->properties.getWithDefault (getLastYProp (type), Random::getSystemRandom().nextInt (500)));

    owner->properties.set (getOpenProp (type), true);

    setVisible (true);

    activePluginWindows.add (this);
}

void PluginWindow::closeCurrentlyOpenWindowsFor (const uint32 nodeId)
{
    for (int i = activePluginWindows.size(); --i >= 0;)
        if (activePluginWindows.getUnchecked(i)->owner->nodeId == nodeId)
            delete activePluginWindows.getUnchecked (i);
}

void PluginWindow::closeAllCurrentlyOpenWindows()
{
    if (activePluginWindows.size() > 0)
    {
        for (int i = activePluginWindows.size(); --i >= 0;)
            delete activePluginWindows.getUnchecked (i);

        Component dummyModalComp;
        dummyModalComp.enterModalState (false);
        MessageManager::getInstance()->runDispatchLoopUntil (50);
    }
}

//==============================================================================
struct ProcessorProgramPropertyComp  : public PropertyComponent,
                                       private AudioProcessorListener
{
    ProcessorProgramPropertyComp (const String& name, AudioProcessor& p)
        : PropertyComponent (name), owner (p)
    {
        owner.addListener (this);
    }

    ~ProcessorProgramPropertyComp()
    {
        owner.removeListener (this);
    }

    void refresh() override {}
    void audioProcessorChanged (AudioProcessor*) override {}
    void audioProcessorParameterChanged (AudioProcessor*, int, float) override {}

    AudioProcessor& owner;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProcessorProgramPropertyComp)
};

struct ProgramAudioProcessorEditor  : public AudioProcessorEditor
{
    ProgramAudioProcessorEditor (AudioProcessor* p)  : AudioProcessorEditor (p)
    {
        jassert (p != nullptr);
        setOpaque (true);

        addAndMakeVisible (panel);

        Array<PropertyComponent*> programs;

        auto numPrograms = p->getNumPrograms();
        int totalHeight = 0;

        for (int i = 0; i < numPrograms; ++i)
        {
            auto name = p->getProgramName (i).trim();

            if (name.isEmpty())
                name = "Unnamed";

            auto pc = new ProcessorProgramPropertyComp (name, *p);
            programs.add (pc);
            totalHeight += pc->getPreferredHeight();
        }

        panel.addProperties (programs);

        setSize (400, jlimit (25, 400, totalHeight));
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::grey);
    }

    void resized() override
    {
        panel.setBounds (getLocalBounds());
    }

    PropertyPanel panel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProgramAudioProcessorEditor)
};

//==============================================================================
PluginWindow* PluginWindow::getWindowFor (AudioProcessorGraph::Node* node, WindowFormatType type)
{
    jassert (node != nullptr);

    for (auto* w : activePluginWindows)
        if (w->owner == node && w->type == type)
            return w;

    auto* processor = node->getProcessor();
    AudioProcessorEditor* ui = nullptr;

    if (auto* pluginInstance = dynamic_cast<AudioPluginInstance*> (processor))
    {
        auto description = pluginInstance->getPluginDescription();

        if (description.pluginFormatName == "Internal")
        {
            getCommandManager().invokeDirectly (CommandIDs::showAudioSettings, false);

            return nullptr;
        }
    }

    if (type == Normal)
    {
        ui = processor->createEditorIfNeeded();

        if (ui == nullptr)
            type = Generic;
    }

    if (ui == nullptr)
    {
        if (type == Generic || type == Parameters)  ui = new GenericAudioProcessorEditor (processor);
        else if (type == Programs)                  ui = new ProgramAudioProcessorEditor (processor);
        else if (type == AudioIO)                   ui = new FilterIOConfigurationWindow (processor);
    }

    if (ui != nullptr)
    {
        if (auto* plugin = dynamic_cast<AudioPluginInstance*> (processor))
            ui->setName (plugin->getName());

        return new PluginWindow (ui, node, type);
    }

    return nullptr;
}

PluginWindow::~PluginWindow()
{
    activePluginWindows.removeFirstMatchingValue (this);
    clearContentComponent();
}

void PluginWindow::moved()
{
    owner->properties.set (getLastXProp (type), getX());
    owner->properties.set (getLastYProp (type), getY());
}

void PluginWindow::closeButtonPressed()
{
    owner->properties.set (getOpenProp (type), false);
    delete this;
}

//==============================================================================
struct PinComponent   : public Component,
                        public SettableTooltipClient
{
    PinComponent (FilterGraph& g, uint32 id, int i, bool isIn)
        : graph (g), pluginID (id), index (i), isInput (isIn)
    {
        if (auto node = graph.getNodeForId (pluginID))
        {
            String tip;

            if (index == FilterGraph::midiChannelNumber)
            {
                tip = isInput ? "MIDI Input"
                              : "MIDI Output";
            }
            else
            {
                auto& processor = *node->getProcessor();
                auto channel = processor.getOffsetInBusBufferForAbsoluteChannelIndex (isInput, index, busIdx);

                if (auto* bus = processor.getBus (isInput, busIdx))
                    tip = bus->getName() + ": " + AudioChannelSet::getAbbreviatedChannelTypeName (bus->getCurrentLayout().getTypeOfChannel (channel));
                else
                    tip = (isInput ? "Main Input: "
                                   : "Main Output: ") + String (index + 1);

            }

            setTooltip (tip);
        }

        setSize (16, 16);
    }

    void paint (Graphics& g) override
    {
        const float w = (float) getWidth();
        const float h = (float) getHeight();

        Path p;
        p.addEllipse (w * 0.25f, h * 0.25f, w * 0.5f, h * 0.5f);
        p.addRectangle (w * 0.4f, isInput ? (0.5f * h) : 0.0f, w * 0.2f, h * 0.5f);

        auto colour = (index == FilterGraph::midiChannelNumber ? Colours::red : Colours::green);

        g.setColour (colour.withRotatedHue (static_cast<float> (busIdx) / 5.0f));
        g.fillPath (p);
    }

    void mouseDown (const MouseEvent& e) override
    {
        getGraphPanel()->beginConnectorDrag (isInput ? 0 : pluginID, index,
                                             isInput ? pluginID : 0, index,
                                             e);
    }

    void mouseDrag (const MouseEvent& e) override
    {
        getGraphPanel()->dragConnector (e);
    }

    void mouseUp (const MouseEvent& e) override
    {
        getGraphPanel()->endDraggingConnector (e);
    }

    GraphEditorPanel* getGraphPanel() const noexcept
    {
        return findParentComponentOfClass<GraphEditorPanel>();
    }

    FilterGraph& graph;
    const uint32 pluginID;
    const int index;
    const bool isInput;
    int busIdx = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PinComponent)
};

//==============================================================================
struct FilterComponent   : public Component
{
    FilterComponent (FilterGraph& g, uint32 id)  : graph (g), pluginID (id)
    {
        shadow.setShadowProperties (DropShadow (Colours::black.withAlpha (0.5f), 3, { 0, 1 }));
        setComponentEffect (&shadow);

        setSize (150, 60);
    }

    ~FilterComponent()
    {
        deleteAllChildren();
    }

    FilterComponent (const FilterComponent&) = delete;
    FilterComponent& operator= (const FilterComponent&) = delete;

    void mouseDown (const MouseEvent& e) override
    {
        originalPos = localPointToGlobal (Point<int>());

        toFront (true);

        if (e.mods.isPopupMenu())
        {
            PopupMenu m;
            m.addItem (1, "Delete this filter");
            m.addItem (2, "Disconnect all pins");
            m.addSeparator();
            m.addItem (3, "Show plugin UI");
            m.addItem (4, "Show all programs");
            m.addItem (5, "Show all parameters");
            m.addSeparator();
            m.addItem (6, "Configure Audio I/O");
            m.addItem (7, "Test state save/load");

            auto r = m.show();

            if (r == 1)
            {
                graph.removeFilter (pluginID);
                return;
            }
            else if (r == 2)
            {
                graph.disconnectFilter (pluginID);
            }
            else
            {
                if (auto f = graph.getNodeForId (pluginID))
                {
                    auto* processor = f->getProcessor();
                    jassert (processor != nullptr);

                    if (r == 7)
                    {
                        MemoryBlock state;
                        processor->getStateInformation (state);
                        processor->setStateInformation (state.getData(), (int) state.getSize());
                    }
                    else
                    {
                        PluginWindow::WindowFormatType type = processor->hasEditor() ? PluginWindow::Normal
                                                                                     : PluginWindow::Generic;

                        switch (r)
                        {
                            case 4: type = PluginWindow::Programs; break;
                            case 5: type = PluginWindow::Parameters; break;
                            case 6: type = PluginWindow::AudioIO; break;

                            default: break;
                        };

                        if (auto* w = PluginWindow::getWindowFor (f, type))
                            w->toFront (true);
                    }
                }
            }
        }
    }

    void mouseDrag (const MouseEvent& e) override
    {
        if (! e.mods.isPopupMenu())
        {
            auto pos = originalPos + e.getOffsetFromDragStart();

            if (getParentComponent() != nullptr)
                pos = getParentComponent()->getLocalPoint (nullptr, pos);

            graph.setNodePosition (pluginID,
                                   (pos.getX() + getWidth() / 2) / (double) getParentWidth(),
                                   (pos.getY() + getHeight() / 2) / (double) getParentHeight());

            getGraphPanel()->updateComponents();
        }
    }

    void mouseUp (const MouseEvent& e) override
    {
        if (e.mouseWasDraggedSinceMouseDown())
        {
            graph.setChangedFlag (true);
        }
        else if (e.getNumberOfClicks() == 2)
        {
            if (auto f = graph.getNodeForId (pluginID))
                if (auto* w = PluginWindow::getWindowFor (f, PluginWindow::Normal))
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
        g.setColour (findColour (TextEditor::backgroundColourId));

        const int x = 4;
        const int y = pinSize;
        const int w = getWidth() - x * 2;
        const int h = getHeight() - pinSize * 2;

        g.fillRect (x, y, w, h);

        g.setColour (findColour (TextEditor::textColourId));
        g.setFont (font);
        g.drawFittedText (getName(), getLocalBounds().reduced (4, 2), Justification::centred, 2);
    }

    void resized() override
    {
        if (auto f = graph.getNodeForId (pluginID))
        {
            if (auto* processor = f->getProcessor())
            {
                for (auto* child : getChildren())
                {
                    if (auto* pin = dynamic_cast<PinComponent*> (child))
                    {
                        const bool isInput = pin->isInput;
                        int busIdx = 0;
                        processor->getOffsetInBusBufferForAbsoluteChannelIndex (isInput, pin->index, busIdx);

                        const int total = isInput ? numIns : numOuts;
                        const int index = pin->index == FilterGraph::midiChannelNumber ? (total - 1) : pin->index;

                        auto totalSpaces = static_cast<float> (total) + (static_cast<float> (jmax (0, processor->getBusCount (isInput) - 1)) * 0.5f);
                        auto indexPos = static_cast<float> (index) + (static_cast<float> (busIdx) * 0.5f);

                        pin->setBounds (proportionOfWidth ((1.0f + indexPos) / (totalSpaces + 1.0f)) - pinSize / 2,
                                        pin->isInput ? 0 : (getHeight() - pinSize),
                                        pinSize, pinSize);
                    }
                }
            }
        }
    }

    Point<float> getPinPos (int index, bool isInput) const
    {
        for (auto* child : getChildren())
            if (auto* pin = dynamic_cast<PinComponent*> (child))
                if (pin->index == index && isInput == pin->isInput)
                    return getPosition().toFloat() + pin->getBounds().getCentre().toFloat();

        return {};
    }

    void update()
    {
        const AudioProcessorGraph::Node::Ptr f (graph.getNodeForId (pluginID));

        if (f == nullptr)
        {
            delete this;
            return;
        }

        numIns = f->getProcessor()->getTotalNumInputChannels();
        if (f->getProcessor()->acceptsMidi())
            ++numIns;

        numOuts = f->getProcessor()->getTotalNumOutputChannels();
        if (f->getProcessor()->producesMidi())
            ++numOuts;

        int w = 100;
        int h = 60;

        w = jmax (w, (jmax (numIns, numOuts) + 1) * 20);

        const int textWidth = font.getStringWidth (f->getProcessor()->getName());
        w = jmax (w, 16 + jmin (textWidth, 300));
        if (textWidth > 300)
            h = 100;

        setSize (w, h);

        setName (f->getProcessor()->getName());

        {
            Point<double> p = graph.getNodePosition (pluginID);
            setCentreRelative ((float) p.x, (float) p.y);
        }

        if (numIns != numInputs || numOuts != numOutputs)
        {
            numInputs = numIns;
            numOutputs = numOuts;

            deleteAllChildren();

            int i;
            for (i = 0; i < f->getProcessor()->getTotalNumInputChannels(); ++i)
                addAndMakeVisible (new PinComponent (graph, pluginID, i, true));

            if (f->getProcessor()->acceptsMidi())
                addAndMakeVisible (new PinComponent (graph, pluginID, FilterGraph::midiChannelNumber, true));

            for (i = 0; i < f->getProcessor()->getTotalNumOutputChannels(); ++i)
                addAndMakeVisible (new PinComponent (graph, pluginID, i, false));

            if (f->getProcessor()->producesMidi())
                addAndMakeVisible (new PinComponent (graph, pluginID, FilterGraph::midiChannelNumber, false));

            resized();
        }
    }

    GraphEditorPanel* getGraphPanel() const noexcept
    {
        return findParentComponentOfClass<GraphEditorPanel>();
    }

    FilterGraph& graph;
    const uint32 pluginID;
    int numInputs = 0, numOutputs = 0;
    int pinSize = 16;
    Point<int> originalPos;
    Font font { 13.0f, Font::bold };
    int numIns = 0, numOuts = 0;
    DropShadowEffect shadow;
};


//==============================================================================
struct ConnectorComponent   : public Component,
                              public SettableTooltipClient
{
    ConnectorComponent (FilterGraph& g) : graph (g)
    {
        setAlwaysOnTop (true);
    }

    void setInput (uint32 newSourceID, int newSourceChannel)
    {
        if (sourceFilterID != newSourceID || sourceFilterChannel != newSourceChannel)
        {
            sourceFilterID = newSourceID;
            sourceFilterChannel = newSourceChannel;
            update();
        }
    }

    void setOutput (uint32 newDestID, int newDestChannel)
    {
        if (destFilterID != newDestID || destFilterChannel != newDestChannel)
        {
            destFilterID = newDestID;
            destFilterChannel = newDestChannel;
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

        if (auto* hostPanel = getGraphPanel())
        {
            if (auto* src = hostPanel->getComponentForFilter (sourceFilterID))
                p1 = src->getPinPos (sourceFilterChannel, false);

            if (auto* dest = hostPanel->getComponentForFilter (destFilterID))
                p2 = dest->getPinPos (destFilterChannel, true);
        }
    }

    void paint (Graphics& g) override
    {
        if (sourceFilterChannel == FilterGraph::midiChannelNumber
             || destFilterChannel == FilterGraph::midiChannelNumber)
        {
            g.setColour (Colours::red);
        }
        else
        {
            g.setColour (Colours::green);
        }

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
            getGraphPanel()->dragConnector (e);
        }
        else if (e.mouseWasDraggedSinceMouseDown())
        {
            dragging = true;

            graph.removeConnection (sourceFilterID, sourceFilterChannel, destFilterID, destFilterChannel);

            double distanceFromStart, distanceFromEnd;
            getDistancesFromEnds (e.position, distanceFromStart, distanceFromEnd);
            const bool isNearerSource = (distanceFromStart < distanceFromEnd);

            getGraphPanel()->beginConnectorDrag (isNearerSource ? 0 : sourceFilterID,
                                                 sourceFilterChannel,
                                                 isNearerSource ? destFilterID : 0,
                                                 destFilterChannel,
                                                 e);
        }
    }

    void mouseUp (const MouseEvent& e) override
    {
        if (dragging)
            getGraphPanel()->endDraggingConnector (e);
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
                                .rotated (float_Pi * 0.5f - (float) atan2 (p2.x - p1.x, p2.y - p1.y))
                                .translated ((p1 + p2) * 0.5f));

        linePath.addPath (arrow);
        linePath.setUsingNonZeroWinding (true);
    }

    GraphEditorPanel* getGraphPanel() const noexcept
    {
        return findParentComponentOfClass<GraphEditorPanel>();
    }

    void getDistancesFromEnds (Point<float> p, double& distanceFromStart, double& distanceFromEnd) const
    {
        Point<float> p1, p2;
        getPoints (p1, p2);

        distanceFromStart = p1.getDistanceFrom (p);
        distanceFromEnd   = p2.getDistanceFrom (p);
    }

    FilterGraph& graph;
    uint32 sourceFilterID = 0, destFilterID = 0;
    int sourceFilterChannel = 0, destFilterChannel = 0;
    Point<float> lastInputPos, lastOutputPos;
    Path linePath, hitPath;
    bool dragging = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConnectorComponent)
};


//==============================================================================
GraphEditorPanel::GraphEditorPanel (FilterGraph& g)  : graph (g)
{
    graph.addChangeListener (this);
    setOpaque (true);
}

GraphEditorPanel::~GraphEditorPanel()
{
    graph.removeChangeListener (this);
    draggingConnector = nullptr;
    deleteAllChildren();
}

void GraphEditorPanel::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void GraphEditorPanel::mouseDown (const MouseEvent& e)
{
    if (e.mods.isPopupMenu())
    {
        PopupMenu m;

        if (auto* mainWindow = findParentComponentOfClass<MainHostWindow>())
        {
            mainWindow->addPluginsToMenu (m);

            auto r = m.show();

            if (auto* desc = mainWindow->getChosenType (r))
                createNewPlugin (*desc, e.position.toInt());
        }
    }
}

void GraphEditorPanel::createNewPlugin (const PluginDescription& desc, Point<int> position)
{
    graph.addFilter (desc, position.toDouble() / Point<double> ((double) getWidth(), (double) getHeight()));
}

FilterComponent* GraphEditorPanel::getComponentForFilter (const uint32 filterID) const
{
    for (auto* child : getChildren())
        if (auto* fc = dynamic_cast<FilterComponent*> (child))
            if (fc->pluginID == filterID)
                return fc;

    return nullptr;
}

ConnectorComponent* GraphEditorPanel::getComponentForConnection (const AudioProcessorGraph::Connection& conn) const
{
    for (auto* child : getChildren())
    {
        if (auto* c = dynamic_cast<ConnectorComponent*> (child))
            if (c->sourceFilterID == conn.sourceNodeId
                 && c->destFilterID == conn.destNodeId
                 && c->sourceFilterChannel == conn.sourceChannelIndex
                 && c->destFilterChannel == conn.destChannelIndex)
                return c;
    }

    return nullptr;
}

PinComponent* GraphEditorPanel::findPinAt (Point<float> pos) const
{
    for (auto* child : getChildren())
        if (auto* fc = dynamic_cast<FilterComponent*> (child))
            if (auto* pin = dynamic_cast<PinComponent*> (fc->getComponentAt (pos.toInt() - fc->getPosition())))
                return pin;

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
    auto children = getChildren();
    for (auto child : children)
        if (auto* fc = dynamic_cast<FilterComponent*> (static_cast<Component*> (child)))
            fc->update();

    for (int i = getNumChildComponents(); --i >= 0;)
    {
        auto* cc = dynamic_cast<ConnectorComponent*> (getChildComponent (i));

        if (cc != nullptr && cc != draggingConnector)
        {
            if (graph.getConnectionBetween (cc->sourceFilterID, cc->sourceFilterChannel,
                                            cc->destFilterID, cc->destFilterChannel) == nullptr)
            {
                delete cc;
            }
            else
            {
                cc->update();
            }
        }
    }

    for (int i = graph.getNumFilters(); --i >= 0;)
    {
        auto f = graph.getNode (i);

        if (getComponentForFilter (f->nodeId) == 0)
        {
            auto* comp = new FilterComponent (graph, f->nodeId);
            addAndMakeVisible (comp);
            comp->update();
        }
    }

    for (int i = graph.getNumConnections(); --i >= 0;)
    {
        auto* c = graph.getConnection (i);

        if (getComponentForConnection (*c) == 0)
        {
            auto* comp = new ConnectorComponent (graph);
            addAndMakeVisible (comp);

            comp->setInput (c->sourceNodeId, c->sourceChannelIndex);
            comp->setOutput (c->destNodeId, c->destChannelIndex);
        }
    }
}

void GraphEditorPanel::beginConnectorDrag (const uint32 sourceFilterID, const int sourceFilterChannel,
                                           const uint32 destFilterID, const int destFilterChannel,
                                           const MouseEvent& e)
{
    draggingConnector = dynamic_cast<ConnectorComponent*> (e.originalComponent);

    if (draggingConnector == nullptr)
        draggingConnector = new ConnectorComponent (graph);

    draggingConnector->setInput (sourceFilterID, sourceFilterChannel);
    draggingConnector->setOutput (destFilterID, destFilterChannel);

    addAndMakeVisible (draggingConnector);
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
            auto srcFilter  = draggingConnector->sourceFilterID;
            auto srcChannel = draggingConnector->sourceFilterChannel;
            auto dstFilter  = draggingConnector->destFilterID;
            auto dstChannel = draggingConnector->destFilterChannel;

            if (srcFilter == 0 && ! pin->isInput)
            {
                srcFilter = pin->pluginID;
                srcChannel = pin->index;
            }
            else if (dstFilter == 0 && pin->isInput)
            {
                dstFilter = pin->pluginID;
                dstChannel = pin->index;
            }

            if (graph.canConnect (srcFilter, srcChannel, dstFilter, dstChannel))
            {
                pos = (pin->getParentComponent()->getPosition() + pin->getBounds().getCentre()).toFloat();
                draggingConnector->setTooltip (pin->getTooltip());
            }
        }

        if (draggingConnector->sourceFilterID == 0)
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

    auto srcFilter  = draggingConnector->sourceFilterID;
    auto srcChannel = draggingConnector->sourceFilterChannel;
    auto dstFilter  = draggingConnector->destFilterID;
    auto dstChannel = draggingConnector->destFilterChannel;

    draggingConnector = nullptr;

    if (auto* pin = findPinAt (e2.position))
    {
        if (srcFilter == 0)
        {
            if (pin->isInput)
                return;

            srcFilter = pin->pluginID;
            srcChannel = pin->index;
        }
        else
        {
            if (! pin->isInput)
                return;

            dstFilter = pin->pluginID;
            dstChannel = pin->index;
        }

        graph.addConnection (srcFilter, srcChannel, dstFilter, dstChannel);
    }
}


//==============================================================================
struct TooltipBar   : public Component,
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
GraphDocumentComponent::GraphDocumentComponent (AudioPluginFormatManager& fm, AudioDeviceManager& dm)
    : graph (new FilterGraph (fm)), deviceManager (dm),
      graphPlayer (getAppProperties().getUserSettings()->getBoolValue ("doublePrecisionProcessing", false))
{
    addAndMakeVisible (graphPanel = new GraphEditorPanel (*graph));

    deviceManager.addChangeListener (graphPanel);

    graphPlayer.setProcessor (&graph->getGraph());

    keyState.addListener (&graphPlayer.getMidiMessageCollector());

    addAndMakeVisible (keyboardComp = new MidiKeyboardComponent (keyState,
                                                                 MidiKeyboardComponent::horizontalKeyboard));

    addAndMakeVisible (statusBar = new TooltipBar());

    deviceManager.addAudioCallback (&graphPlayer);
    deviceManager.addMidiInputCallback (String(), &graphPlayer.getMidiMessageCollector());

    graphPanel->updateComponents();
}

GraphDocumentComponent::~GraphDocumentComponent()
{
    releaseGraph();

    keyState.removeListener (&graphPlayer.getMidiMessageCollector());
}

void GraphDocumentComponent::resized()
{
    const int keysHeight = 60;
    const int statusHeight = 20;

    graphPanel->setBounds (0, 0, getWidth(), getHeight() - keysHeight);
    statusBar->setBounds (0, getHeight() - keysHeight - statusHeight, getWidth(), statusHeight);
    keyboardComp->setBounds (0, getHeight() - keysHeight, getWidth(), keysHeight);
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
    deviceManager.removeChangeListener (graphPanel);

    deleteAllChildren();

    graphPlayer.setProcessor (nullptr);
    graph = nullptr;
}

void GraphDocumentComponent::setDoublePrecision (bool doublePrecision)
{
    graphPlayer.setDoublePrecisionProcessing (doublePrecision);
}
