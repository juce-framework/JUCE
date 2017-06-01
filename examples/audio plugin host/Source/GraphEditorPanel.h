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

#pragma once

#include "FilterGraph.h"

struct FilterComponent;
struct ConnectorComponent;
struct PinComponent;


//==============================================================================
/**
    A panel that displays and edits a FilterGraph.
*/
class GraphEditorPanel   : public Component,
                           public ChangeListener
{
public:
    GraphEditorPanel (FilterGraph& graph);
    ~GraphEditorPanel();

    void paint (Graphics& g);
    void mouseDown (const MouseEvent& e);

    void createNewPlugin (const PluginDescription&, Point<int> position);

    FilterComponent* getComponentForFilter (uint32 filterID) const;
    ConnectorComponent* getComponentForConnection (const AudioProcessorGraph::Connection& conn) const;
    PinComponent* findPinAt (Point<float>) const;

    void resized();
    void changeListenerCallback (ChangeBroadcaster*);
    void updateComponents();

    //==============================================================================
    void beginConnectorDrag (uint32 sourceFilterID, int sourceFilterChannel,
                             uint32 destFilterID, int destFilterChannel,
                             const MouseEvent& e);
    void dragConnector (const MouseEvent& e);
    void endDraggingConnector (const MouseEvent& e);

    //==============================================================================
private:
    FilterGraph& graph;
    ScopedPointer<ConnectorComponent> draggingConnector;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphEditorPanel)
};


//==============================================================================
/**
    A panel that embeds a GraphEditorPanel with a midi keyboard at the bottom.

    It also manages the graph itself, and plays it.
*/
class GraphDocumentComponent  : public Component
{
public:
    //==============================================================================
    GraphDocumentComponent (AudioPluginFormatManager& formatManager,
                            AudioDeviceManager& deviceManager);
    ~GraphDocumentComponent();

    //==============================================================================
    void createNewPlugin (const PluginDescription&, Point<int> position);
    void setDoublePrecision (bool doublePrecision);

    //==============================================================================
    ScopedPointer<FilterGraph> graph;

    //==============================================================================
    void resized();

    //==============================================================================
    void unfocusKeyboardComponent();

    //==============================================================================
    void releaseGraph();

private:
    //==============================================================================
    AudioDeviceManager& deviceManager;
    AudioProcessorPlayer graphPlayer;
    MidiKeyboardState keyState;

public:
    GraphEditorPanel* graphPanel;

private:
    Component* keyboardComp;
    Component* statusBar;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphDocumentComponent)
};

//==============================================================================
/** A desktop window containing a plugin's UI. */
class PluginWindow  : public DocumentWindow
{
public:
    enum WindowFormatType
    {
        Normal = 0,
        Generic,
        Programs,
        Parameters,
        AudioIO,
        NumTypes
    };

    PluginWindow (AudioProcessorEditor*, AudioProcessorGraph::Node*, WindowFormatType);
    ~PluginWindow();

    static PluginWindow* getWindowFor (AudioProcessorGraph::Node*, WindowFormatType);

    static void closeCurrentlyOpenWindowsFor (const uint32 nodeId);
    static void closeAllCurrentlyOpenWindows();

    void moved() override;
    void closeButtonPressed() override;

private:
    AudioProcessorGraph::Node* owner;
    WindowFormatType type;

    float getDesktopScaleFactor() const override     { return 1.0f; }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginWindow)
};

inline String toString (PluginWindow::WindowFormatType type)
{
    switch (type)
    {
        case PluginWindow::Normal:     return "Normal";
        case PluginWindow::Generic:    return "Generic";
        case PluginWindow::Programs:   return "Programs";
        case PluginWindow::Parameters: return "Parameters";
        default:                       return String();
    }
}

inline String getLastXProp (PluginWindow::WindowFormatType type)    { return "uiLastX_" + toString (type); }
inline String getLastYProp (PluginWindow::WindowFormatType type)    { return "uiLastY_" + toString (type); }
inline String getOpenProp  (PluginWindow::WindowFormatType type)    { return "uiopen_"  + toString (type); }
