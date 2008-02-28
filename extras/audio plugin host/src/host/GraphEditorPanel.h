/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCE_FILTERGRAPHEDITOR_JUCEHEADER__
#define __JUCE_FILTERGRAPHEDITOR_JUCEHEADER__

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
    void changeListenerCallback (void*);
    void updateComponents();

    //==============================================================================
    void beginConnectorDrag (const uint32 sourceFilterID, const int sourceFilterChannel,
                             const uint32 destFilterID, const int destFilterChannel,
                             const MouseEvent& e);
    void dragConnector (const MouseEvent& e);
    void endDraggingConnector (const MouseEvent& e);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    FilterGraph& graph;
    ConnectorComponent* draggingConnector;

    GraphEditorPanel (const GraphEditorPanel&);
    const GraphEditorPanel& operator= (const GraphEditorPanel&);
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
    GraphDocumentComponent (AudioDeviceManager* deviceManager);
    ~GraphDocumentComponent();

    //==============================================================================
    void createNewPlugin (const PluginDescription* desc, int x, int y);

    //==============================================================================
    FilterGraph graph;

    //==============================================================================
    void resized();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    AudioDeviceManager* deviceManager;
    AudioProcessorPlayer graphPlayer;
    MidiKeyboardState keyState;

    GraphEditorPanel* graphPanel;
    Component* keyboardComp;
    Component* statusBar;
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

#endif
