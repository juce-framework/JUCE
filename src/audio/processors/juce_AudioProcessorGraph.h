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

#ifndef __JUCE_AUDIOPROCESSORGRAPH_JUCEHEADER__
#define __JUCE_AUDIOPROCESSORGRAPH_JUCEHEADER__

#include "juce_AudioProcessor.h"
#include "../plugins/juce_AudioPluginFormatManager.h"
#include "../plugins/juce_KnownPluginList.h"
#include "../../containers/juce_ReferenceCountedArray.h"


//==============================================================================
/**
    A type of AudioProcessor which plays back a graph of other AudioProcessors.

    Use one of these objects if you want to wire-up a set of AudioProcessors
    and play back the result.

    Processors can be added to the graph as "nodes" using addNode(), and once
    added, you can connect any of their input or output channels to other
    nodes using addConnection().

    To play back a graph through an audio device, you might want to use an
    AudioProcessorPlayer object.
*/
class JUCE_API  AudioProcessorGraph   : public AudioProcessor,
                                        public AsyncUpdater
{
public:
    //==============================================================================
    /** Creates an empty graph.
    */
    AudioProcessorGraph();

    /** Destructor.

        Any processor objects that have been added to the graph will also be deleted.
    */
    ~AudioProcessorGraph();

    //==============================================================================
    /** Represents one of the nodes, or processors, in an AudioProcessorGraph.

        To create a node, call AudioProcessorGraph::addNode().
    */
    class JUCE_API  Node   : public ReferenceCountedObject
    {
    public:
        /** Destructor.
        */
        ~Node();

        //==============================================================================
        /** The ID number assigned to this node.

            This is assigned by the graph that owns it, and can't be changed.
        */
        const uint32 id;

        /** The actual processor object that this node represents.
        */
        AudioProcessor* const processor;

        /** A set of user-definable properties that are associated with this node.

            This can be used to attach values to the node for whatever purpose seems
            useful. For example, you might store an x and y position if your application
            is displaying the nodes on-screen.
        */
        PropertySet properties;

        //==============================================================================
        /** A convenient typedef for referring to a pointer to a node object.
        */
        typedef ReferenceCountedObjectPtr <Node> Ptr;

        //==============================================================================
        juce_UseDebuggingNewOperator

    private:
        friend class AudioProcessorGraph;

        bool isPrepared;

        Node (const uint32 id, AudioProcessor* const processor);

        void prepare (const double sampleRate, const int blockSize, AudioProcessorGraph* const graph);
        void unprepare();

        Node (const Node&);
        const Node& operator= (const Node&);
    };

    //==============================================================================
    /** Represents a connection between two channels of two nodes in an AudioProcessorGraph.

        To create a connection, use AudioProcessorGraph::addConnection().
    */
    struct JUCE_API  Connection
    {
    public:
        //==============================================================================
        /** The ID number of the node which is the input source for this connection.
            @see AudioProcessorGraph::getNodeForId
        */
        uint32 sourceNodeId;

        /** The index of the output channel of the source node from which this
            connection takes its data.

            If this value is the special number AudioProcessorGraph::midiChannelIndex, then
            it is referring to the source node's midi output. Otherwise, it is the zero-based
            index of an audio output channel in the source node.
        */
        int sourceChannelIndex;

        /** The ID number of the node which is the destination for this connection.
            @see AudioProcessorGraph::getNodeForId
        */
        uint32 destNodeId;

        /** The index of the input channel of the destination node to which this
            connection delivers its data.

            If this value is the special number AudioProcessorGraph::midiChannelIndex, then
            it is referring to the destination node's midi input. Otherwise, it is the zero-based
            index of an audio input channel in the destination node.
        */
        int destChannelIndex;

        //==============================================================================
        juce_UseDebuggingNewOperator

    private:
    };

    //==============================================================================
    /** Deletes all nodes and connections from this graph.

        Any processor objects in the graph will be deleted.
    */
    void clear();

    /** Returns the number of nodes in the graph. */
    int getNumNodes() const                                         { return nodes.size(); }

    /** Returns a pointer to one of the nodes in the graph.

        This will return 0 if the index is out of range.
        @see getNodeForId
    */
    Node* getNode (const int index) const                           { return nodes [index]; }

    /** Searches the graph for a node with the given ID number and returns it.

        If no such node was found, this returns 0.
        @see getNode
    */
    Node* getNodeForId (const uint32 nodeId) const;

    /** Adds a node to the graph.

        This creates a new node in the graph, for the specified processor. Once you have
        added a processor to the graph, the graph owns it and will delete it later when
        it is no longer needed.

        The optional nodeId parameter lets you specify an ID to use for the node, but
        if the value is already in use, this new node will overwrite the old one.

        If this succeeds, it returns a pointer to the newly-created node.
    */
    Node* addNode (AudioProcessor* const newProcessor,
                   uint32 nodeId = 0);

    /** Deletes a node within the graph which has the specified ID.

        This will also delete any connections that are attached to this node.
    */
    bool removeNode (const uint32 nodeId);

    //==============================================================================
    /** Returns the number of connections in the graph. */
    int getNumConnections() const                                       { return connections.size(); }

    /** Returns a pointer to one of the connections in the graph. */
    const Connection* getConnection (const int index) const             { return connections [index]; }

    /** Searches for a connection between some specified channels.

        If no such connection is found, this returns 0.
    */
    const Connection* getConnectionBetween (const uint32 sourceNodeId,
                                            const int sourceChannelIndex,
                                            const uint32 destNodeId,
                                            const int destChannelIndex) const;

    /** Returns true if there is a connection between any of the channels of
        two specified nodes.
    */
    bool isConnected (const uint32 possibleSourceNodeId,
                      const uint32 possibleDestNodeId) const;

    /** Returns true if it would be legal to connect the specified points.
    */
    bool canConnect (const uint32 sourceNodeId, const int sourceChannelIndex,
                     const uint32 destNodeId, const int destChannelIndex) const;

    /** Attempts to connect two specified channels of two nodes.

        If this isn't allowed (e.g. because you're trying to connect a midi channel
        to an audio one or other such nonsense), then it'll return false.
    */
    bool addConnection (const uint32 sourceNodeId, const int sourceChannelIndex,
                        const uint32 destNodeId, const int destChannelIndex);

    /** Deletes the connection with the specified index.

        Returns true if a connection was actually deleted.
    */
    void removeConnection (const int index);

    /** Deletes any connection between two specified points.

        Returns true if a connection was actually deleted.
    */
    bool removeConnection (const uint32 sourceNodeId, const int sourceChannelIndex,
                           const uint32 destNodeId, const int destChannelIndex);

    /** Removes all connections from the specified node.
    */
    bool disconnectNode (const uint32 nodeId);

    /** Performs a sanity checks of all the connections.

        This might be useful if some of the processors are doing things like changing
        their channel counts, which could render some connections obsolete.
    */
    bool removeIllegalConnections();

    //==============================================================================
    /** A special number that represents the midi channel of a node.

        This is used as a channel index value if you want to refer to the midi input
        or output instead of an audio channel.
    */
    static const int midiChannelIndex;


    //==============================================================================
    /** A special type of AudioProcessor that can live inside an AudioProcessorGraph
        in order to use the audio that comes into and out of the graph itself.

        If you create an AudioGraphIOProcessor in "input" mode, it will act as a
        node in the graph which delivers the audio that is coming into the parent
        graph. This allows you to stream the data to other nodes and process the
        incoming audio.

        Likewise, one of these in "output" mode can be sent data which it will add to
        the sum of data being sent to the graph's output.

        @see AudioProcessorGraph
    */
    class JUCE_API  AudioGraphIOProcessor     : public AudioPluginInstance
    {
    public:
        /** Specifies the mode in which this processor will operate.
        */
        enum IODeviceType
        {
            audioInputNode,     /**< In this mode, the processor has output channels
                                     representing all the audio input channels that are
                                     coming into its parent audio graph. */
            audioOutputNode,    /**< In this mode, the processor has input channels
                                     representing all the audio output channels that are
                                     going out of its parent audio graph. */
            midiInputNode,      /**< In this mode, the processor has a midi output which
                                     delivers the same midi data that is arriving at its
                                     parent graph. */
            midiOutputNode      /**< In this mode, the processor has a midi input and
                                     any data sent to it will be passed out of the parent
                                     graph. */
        };

        //==============================================================================
        /** Returns the mode of this processor. */
        IODeviceType getType() const                                { return type; }

        /** Returns the parent graph to which this processor belongs, or 0 if it
            hasn't yet been added to one. */
        AudioProcessorGraph* getParentGraph() const                 { return graph; }

        /** True if this is an audio or midi input. */
        bool isInput() const;
        /** True if this is an audio or midi output. */
        bool isOutput() const;

        //==============================================================================
        AudioGraphIOProcessor (const IODeviceType type);
        ~AudioGraphIOProcessor();

        const String getName() const;
        void fillInPluginDescription (PluginDescription& d) const;

        void prepareToPlay (double sampleRate, int estimatedSamplesPerBlock);
        void releaseResources();
        void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages);

        const String getInputChannelName (const int channelIndex) const;
        const String getOutputChannelName (const int channelIndex) const;
        bool isInputChannelStereoPair (int index) const;
        bool isOutputChannelStereoPair (int index) const;
        bool acceptsMidi() const;
        bool producesMidi() const;

        AudioProcessorEditor* createEditor();

        int getNumParameters();
        const String getParameterName (int);
        float getParameter (int);
        const String getParameterText (int);
        void setParameter (int, float);

        int getNumPrograms();
        int getCurrentProgram();
        void setCurrentProgram (int);
        const String getProgramName (int);
        void changeProgramName (int, const String&);

        void getStateInformation (JUCE_NAMESPACE::MemoryBlock& destData);
        void setStateInformation (const void* data, int sizeInBytes);

        /** @internal */
        void setParentGraph (AudioProcessorGraph* const graph);

        juce_UseDebuggingNewOperator

    private:
        const IODeviceType type;
        AudioProcessorGraph* graph;

        AudioGraphIOProcessor (const AudioGraphIOProcessor&);
        const AudioGraphIOProcessor& operator= (const AudioGraphIOProcessor&);
    };

    //==============================================================================
    // AudioProcessor methods:

    const String getName() const;

    void prepareToPlay (double sampleRate, int estimatedSamplesPerBlock);
    void releaseResources();
    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages);

    const String getInputChannelName (const int channelIndex) const;
    const String getOutputChannelName (const int channelIndex) const;
    bool isInputChannelStereoPair (int index) const;
    bool isOutputChannelStereoPair (int index) const;

    bool acceptsMidi() const;
    bool producesMidi() const;

    AudioProcessorEditor* createEditor()            { return 0; }

    int getNumParameters()                          { return 0; }
    const String getParameterName (int)             { return String::empty; }
    float getParameter (int)                        { return 0; }
    const String getParameterText (int)             { return String::empty; }
    void setParameter (int, float)                  { }

    int getNumPrograms()                            { return 0; }
    int getCurrentProgram()                         { return 0; }
    void setCurrentProgram (int)                    { }
    const String getProgramName (int)               { return String::empty; }
    void changeProgramName (int, const String&)     { }

    void getStateInformation (JUCE_NAMESPACE::MemoryBlock& destData);
    void setStateInformation (const void* data, int sizeInBytes);

    /** @internal */
    void handleAsyncUpdate();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    ReferenceCountedArray <Node> nodes;
    OwnedArray <Connection> connections;
    int lastNodeId;
    AudioSampleBuffer renderingBuffers;
    OwnedArray <MidiBuffer> midiBuffers;

    CriticalSection renderLock;
    VoidArray renderingOps;

    friend class AudioGraphIOProcessor;
    AudioSampleBuffer* currentAudioInputBuffer;
    AudioSampleBuffer currentAudioOutputBuffer;
    MidiBuffer* currentMidiInputBuffer;
    MidiBuffer currentMidiOutputBuffer;

    void clearRenderingSequence();
    void buildRenderingSequence();

    bool isAnInputTo (const uint32 possibleInputId,
                      const uint32 possibleDestinationId,
                      const int recursionCheck) const;

    AudioProcessorGraph (const AudioProcessorGraph&);
    const AudioProcessorGraph& operator= (const AudioProcessorGraph&);
};


#endif   // __JUCE_AUDIOPROCESSORGRAPH_JUCEHEADER__
