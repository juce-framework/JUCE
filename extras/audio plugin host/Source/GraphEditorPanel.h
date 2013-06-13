/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef __GRAPHEDITORPANEL_JUCEHEADER__
#define __GRAPHEDITORPANEL_JUCEHEADER__

#include "FilterGraph.h"

class FilterComponent;
class ConnectorComponent;
class PinComponent;


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

    void createNewPlugin (const PluginDescription* desc, int x, int y);

    FilterComponent* getComponentForFilter (const uint32 filterID) const;
    ConnectorComponent* getComponentForConnection (const AudioProcessorGraph::Connection& conn) const;
    PinComponent* findPinAt (const int x, const int y) const;

    void resized();
    void changeListenerCallback (ChangeBroadcaster*);
    void updateComponents();

    //==============================================================================
    void beginConnectorDrag (const uint32 sourceFilterID, const int sourceFilterChannel,
                             const uint32 destFilterID, const int destFilterChannel,
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
                            AudioDeviceManager* deviceManager);
    ~GraphDocumentComponent();

    //==============================================================================
    void createNewPlugin (const PluginDescription* desc, int x, int y);

    //==============================================================================
    FilterGraph graph;

    //==============================================================================
    void resized();

private:
    //==============================================================================
    AudioDeviceManager* deviceManager;
    AudioProcessorPlayer graphPlayer;
    MidiKeyboardState keyState;

    GraphEditorPanel* graphPanel;
    Component* keyboardComp;
    Component* statusBar;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphDocumentComponent)
};

//==============================================================================
/** A desktop window containing a plugin's UI.
*/
class PluginWindow  : public DocumentWindow
{
    PluginWindow (Component* const uiComp,
                  AudioProcessorGraph::Node* owner_,
                  const bool isGeneric_);

public:
    static PluginWindow* getWindowFor (AudioProcessorGraph::Node* node,
                                       bool useGenericView);

    static void closeCurrentlyOpenWindowsFor (const uint32 nodeId);

    static void closeAllCurrentlyOpenWindows();

    ~PluginWindow();

    void moved();
    void closeButtonPressed();

private:
    AudioProcessorGraph::Node* owner;
    bool isGeneric;
};

#endif   // __GRAPHEDITORPANEL_JUCEHEADER__
