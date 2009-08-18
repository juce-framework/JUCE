/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_FILTERGRAPH_JUCEHEADER__
#define __JUCE_FILTERGRAPH_JUCEHEADER__

class FilterInGraph;
class FilterGraph;

const char* const filenameSuffix = ".filtergraph";
const char* const filenameWildcard = "*.filtergraph";

//==============================================================================
/**
    Represents a connection between two pins in a FilterGraph.
*/
class FilterConnection
{
public:
    //==============================================================================
    FilterConnection (FilterGraph& owner);
    FilterConnection (const FilterConnection& other);
    ~FilterConnection();

    //==============================================================================
    uint32 sourceFilterID;
    int sourceChannel;
    uint32 destFilterID;
    int destChannel;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    FilterGraph& owner;

    const FilterConnection& operator= (const FilterConnection&);
};


//==============================================================================
/**
    Represents one of the filters in a FilterGraph.
*/
/*class FilterInGraph   : public ReferenceCountedObject
{
public:
    //==============================================================================
    FilterInGraph (FilterGraph& owner, AudioPluginInstance* const plugin);
    ~FilterInGraph();

    //==============================================================================
    AudioPluginInstance* const filter;
    uint32 uid;

    //==============================================================================
    void showUI (bool useGenericUI);

    double getX() const throw()                     { return x; }
    double getY() const throw()                     { return y; }
    void setPosition (double x, double y) throw();

    XmlElement* createXml() const;

    static FilterInGraph* createForDescription (FilterGraph& owner,
                                                const PluginDescription& desc,
                                                String& errorMessage);

    static FilterInGraph* createFromXml (FilterGraph& owner, const XmlElement& xml);

    //==============================================================================
    typedef ReferenceCountedObjectPtr <FilterInGraph> Ptr;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    friend class FilterGraphPlayer;
    FilterGraph& owner;
    double x, y;

    friend class PluginWindow;
    Component* activeUI;
    Component* activeGenericUI;
    int lastX, lastY;

    MidiBuffer outputMidi;
    AudioSampleBuffer processedAudio;
    MidiBuffer processedMidi;

    void prepareBuffers (int blockSize);
    void renderBlock (int numSamples,
                      const ReferenceCountedArray <FilterInGraph>& filters,
                      const OwnedArray <FilterConnection>& connections);

    FilterInGraph (const FilterInGraph&);
    const FilterInGraph& operator= (const FilterInGraph&);
};
*/

//==============================================================================
/**
    A collection of filters and some connections between them.
*/
class FilterGraph   : public FileBasedDocument
{
public:
    //==============================================================================
    FilterGraph();
    ~FilterGraph();

    //==============================================================================
    AudioProcessorGraph& getGraph() throw()         { return graph; }

    int getNumFilters() const throw();
    const AudioProcessorGraph::Node::Ptr getNode (const int index) const throw();
    const AudioProcessorGraph::Node::Ptr getNodeForId (const uint32 uid) const throw();

    void addFilter (const PluginDescription* desc, double x, double y);

    void removeFilter (const uint32 filterUID);
    void disconnectFilter (const uint32 filterUID);

    void removeIllegalConnections();

    void setNodePosition (const int nodeId, double x, double y);
    void getNodePosition (const int nodeId, double& x, double& y) const;

    //==============================================================================
    int getNumConnections() const throw();
    const AudioProcessorGraph::Connection* getConnection (const int index) const throw();

    const AudioProcessorGraph::Connection* getConnectionBetween (uint32 sourceFilterUID, int sourceFilterChannel,
                                                                 uint32 destFilterUID, int destFilterChannel) const throw();

    bool canConnect (uint32 sourceFilterUID, int sourceFilterChannel,
                     uint32 destFilterUID, int destFilterChannel) const throw();

    bool addConnection (uint32 sourceFilterUID, int sourceFilterChannel,
                        uint32 destFilterUID, int destFilterChannel);

    void removeConnection (const int index);

    void removeConnection (uint32 sourceFilterUID, int sourceFilterChannel,
                           uint32 destFilterUID, int destFilterChannel);

    void clear();


    //==============================================================================

    XmlElement* createXml() const;
    void restoreFromXml (const XmlElement& xml);

    //==============================================================================
    const String getDocumentTitle();
    const String loadDocument (const File& file);
    const String saveDocument (const File& file);
    const File getLastDocumentOpened();
    void setLastDocumentOpened (const File& file);

    /** The special channel index used to refer to a filter's midi channel.
    */
    static const int midiChannelNumber;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //friend class FilterGraphPlayer;
    //ReferenceCountedArray <FilterInGraph> filters;
    //OwnedArray <FilterConnection> connections;

    AudioProcessorGraph graph;
    AudioProcessorPlayer player;

    uint32 lastUID;
    uint32 getNextUID() throw();

    void createNodeFromXml (const XmlElement& xml);

    FilterGraph (const FilterGraph&);
    const FilterGraph& operator= (const FilterGraph&);
};


//==============================================================================
/**

*/
/*class FilterGraphPlayer   : public AudioIODeviceCallback,
                            public MidiInputCallback,
                            public ChangeListener

{
public:
    //==============================================================================
    FilterGraphPlayer (FilterGraph& graph);
    ~FilterGraphPlayer();

    //==============================================================================
    void setAudioDeviceManager (AudioDeviceManager* dm);
    AudioDeviceManager* getAudioDeviceManager() const throw()   { return deviceManager; }

    //==============================================================================
    void audioDeviceIOCallback (const float** inputChannelData,
                                int totalNumInputChannels,
                                float** outputChannelData,
                                int totalNumOutputChannels,
                                int numSamples);
    void audioDeviceAboutToStart (double sampleRate, int numSamplesPerBlock);
    void audioDeviceStopped();

    void handleIncomingMidiMessage (MidiInput* source, const MidiMessage& message);

    void changeListenerCallback (void*);

    //==============================================================================
    static int compareElements (FilterInGraph* const first, FilterInGraph* const second) throw();

    const float** inputChannelData;
    int totalNumInputChannels;
    float** outputChannelData;
    int totalNumOutputChannels;
    MidiBuffer incomingMidi;

    MidiKeyboardState keyState;
    MidiMessageCollector messageCollector;

    //==============================================================================
    class PlayerAwareFilter
    {
    public:
        virtual void setPlayer (FilterGraphPlayer* newPlayer) = 0;
    };

private:
    FilterGraph& graph;
    CriticalSection processLock;
    double sampleRate;
    int blockSize;
    AudioDeviceManager* deviceManager;

    ReferenceCountedArray <FilterInGraph> filters;
    OwnedArray <FilterConnection> connections;

    void update();

    FilterGraphPlayer (const FilterGraphPlayer&);
    const FilterGraphPlayer& operator= (const FilterGraphPlayer&);
};
*/

#endif
