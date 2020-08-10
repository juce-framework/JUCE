/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../Plugins/PluginGraph.h"

class MainHostWindow;

//==============================================================================
/**
    A panel that displays and edits a PluginGraph.
*/
class GraphEditorPanel   : public Component,
                           public ChangeListener,
                           private Timer
{
public:
    GraphEditorPanel (PluginGraph& graph);
    ~GraphEditorPanel() override;

    void createNewPlugin (const PluginDescription&, Point<int> position);

    void paint (Graphics&) override;
    void resized() override;

    void mouseDown (const MouseEvent&) override;
    void mouseUp   (const MouseEvent&) override;
    void mouseDrag (const MouseEvent&) override;

    void changeListenerCallback (ChangeBroadcaster*) override;

    //==============================================================================
    void updateComponents();

    //==============================================================================
    void showPopupMenu (Point<int> position);

    //==============================================================================
    void beginConnectorDrag (AudioProcessorGraph::NodeAndChannel source,
                             AudioProcessorGraph::NodeAndChannel dest,
                             const MouseEvent&);
    void dragConnector (const MouseEvent&);
    void endDraggingConnector (const MouseEvent&);

    //==============================================================================
    PluginGraph& graph;

private:
    struct PluginComponent;
    struct ConnectorComponent;
    struct PinComponent;

    OwnedArray<PluginComponent> nodes;
    OwnedArray<ConnectorComponent> connectors;
    std::unique_ptr<ConnectorComponent> draggingConnector;
    std::unique_ptr<PopupMenu> menu;

    PluginComponent* getComponentForPlugin (AudioProcessorGraph::NodeID) const;
    ConnectorComponent* getComponentForConnection (const AudioProcessorGraph::Connection&) const;
    PinComponent* findPinAt (Point<float>) const;

    //==============================================================================
    Point<int> originalTouchPos;

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphEditorPanel)
};


//==============================================================================
/**
    A panel that embeds a GraphEditorPanel with a midi keyboard at the bottom.

    It also manages the graph itself, and plays it.
*/
class GraphDocumentComponent  : public Component,
                                public DragAndDropTarget,
                                public DragAndDropContainer,
                                private ChangeListener
{
public:
    GraphDocumentComponent (AudioPluginFormatManager& formatManager,
                            AudioDeviceManager& deviceManager,
                            KnownPluginList& pluginList);

    ~GraphDocumentComponent() override;

    //==============================================================================
    void createNewPlugin (const PluginDescription&, Point<int> position);
    void setDoublePrecision (bool doublePrecision);
    bool closeAnyOpenPluginWindows();

    //==============================================================================
    std::unique_ptr<PluginGraph> graph;

    void resized() override;
    void unfocusKeyboardComponent();
    void releaseGraph();

    //==============================================================================
    bool isInterestedInDragSource (const SourceDetails&) override;
    void itemDropped (const SourceDetails&) override;

    //==============================================================================
    std::unique_ptr<GraphEditorPanel> graphPanel;
    std::unique_ptr<MidiKeyboardComponent> keyboardComp;

    //==============================================================================
    void showSidePanel (bool isSettingsPanel);
    void hideLastSidePanel();

    BurgerMenuComponent burgerMenu;

private:
    //==============================================================================
    AudioDeviceManager& deviceManager;
    KnownPluginList& pluginList;

    AudioProcessorPlayer graphPlayer;
    MidiKeyboardState keyState;
    MidiOutput* midiOutput = nullptr;

    struct TooltipBar;
    std::unique_ptr<TooltipBar> statusBar;

    class TitleBarComponent;
    std::unique_ptr<TitleBarComponent> titleBarComponent;

    //==============================================================================
    struct PluginListBoxModel;
    std::unique_ptr<PluginListBoxModel> pluginListBoxModel;

    ListBox pluginListBox;

    SidePanel mobileSettingsSidePanel { "Settings", 300, true };
    SidePanel pluginListSidePanel    { "Plugins", 250, false };
    SidePanel* lastOpenedSidePanel = nullptr;

    //==============================================================================
    void changeListenerCallback (ChangeBroadcaster*) override;

    void init();
    void checkAvailableWidth();
    void updateMidiOutput();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphDocumentComponent)
};
