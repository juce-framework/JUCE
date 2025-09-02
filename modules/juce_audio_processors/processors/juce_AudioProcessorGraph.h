/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

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

    @tags{Audio}
*/
class JUCE_API  AudioProcessorGraph   : public AudioProcessor,
                                        public ChangeBroadcaster
{
public:
    //==============================================================================
    /** Creates an empty graph. */
    AudioProcessorGraph();

    /** Destructor.
        Any processor objects that have been added to the graph will also be deleted.
    */
    ~AudioProcessorGraph() override;

    /** Each node in the graph has a UID of this type. */
    struct NodeID
    {
        constexpr NodeID() = default;
        explicit constexpr NodeID (uint32 i) : uid (i) {}

        uint32 uid = 0;

        constexpr bool operator== (const NodeID& other) const noexcept    { return uid == other.uid; }
        constexpr bool operator!= (const NodeID& other) const noexcept    { return uid != other.uid; }
        constexpr bool operator<  (const NodeID& other) const noexcept    { return uid <  other.uid; }
    };

    //==============================================================================
    /** A special index that represents the midi channel of a node.

        This is used as a channel index value if you want to refer to the midi input
        or output instead of an audio channel.
    */
    enum { midiChannelIndex = 0x1000 };

    //==============================================================================
    /**
        Represents an input or output channel of a node in an AudioProcessorGraph.
    */
    class NodeAndChannel
    {
        constexpr auto tie() const { return std::tie (nodeID, channelIndex); }

    public:
        NodeID nodeID;
        int channelIndex;

        constexpr bool isMIDI() const noexcept                                    { return channelIndex == midiChannelIndex; }

        constexpr bool operator== (const NodeAndChannel& other) const noexcept    { return tie() == other.tie(); }
        constexpr bool operator!= (const NodeAndChannel& other) const noexcept    { return tie() != other.tie(); }
        constexpr bool operator<  (const NodeAndChannel& other) const noexcept    { return tie() <  other.tie(); }
    };

    //==============================================================================
    /** Represents one of the nodes, or processors, in an AudioProcessorGraph.

        To create a node, call AudioProcessorGraph::addNode().
    */
    class JUCE_API  Node   : public ReferenceCountedObject
    {
    public:
        //==============================================================================
        /** The ID number assigned to this node.
            This is assigned by the graph that owns it, and can't be changed.
        */
        const NodeID nodeID;

        /** The actual processor object that this node represents. */
        AudioProcessor* getProcessor() const noexcept           { return processor.get(); }

        /** A set of user-definable properties that are associated with this node.

            This can be used to attach values to the node for whatever purpose seems
            useful. For example, you might store an x and y position if your application
            is displaying the nodes on-screen.
        */
        NamedValueSet properties;

        //==============================================================================
        /** Returns if the node is bypassed or not. */
        bool isBypassed() const noexcept
        {
            if (processor != nullptr)
            {
                if (auto* bypassParam = processor->getBypassParameter())
                    return ! approximatelyEqual (bypassParam->getValue(), 0.0f);
            }

            return bypassed;
        }

        /** Tell this node to bypass processing. */
        void setBypassed (bool shouldBeBypassed) noexcept
        {
            if (processor != nullptr)
            {
                if (auto* bypassParam = processor->getBypassParameter())
                    bypassParam->setValueNotifyingHost (shouldBeBypassed ? 1.0f : 0.0f);
            }

            bypassed = shouldBeBypassed;
        }

        //==============================================================================
        /** A convenient typedef for referring to a pointer to a node object. */
        using Ptr = ReferenceCountedObjectPtr<Node>;

        /** @internal

            Returns true if setBypassed (true) was called on this node.
            This behaviour is different from isBypassed(), which may additionally return true if
            the node has a bypass parameter that is not set to 0.
        */
        bool userRequestedBypass() const { return bypassed; }

        /** @internal

            To create a new node, use AudioProcessorGraph::addNode.
        */
        Node (NodeID n, std::unique_ptr<AudioProcessor> p) noexcept
            : nodeID (n), processor (std::move (p))
        {
            jassert (processor != nullptr);
        }

    private:
        //==============================================================================
        std::unique_ptr<AudioProcessor> processor;
        std::atomic<bool> bypassed { false };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Node)
    };

    //==============================================================================
    /** Represents a connection between two channels of two nodes in an AudioProcessorGraph.

        To create a connection, use AudioProcessorGraph::addConnection().
    */
    struct JUCE_API  Connection
    {
        //==============================================================================
        constexpr Connection() = default;
        constexpr Connection (NodeAndChannel sourceIn, NodeAndChannel destinationIn) noexcept
            : source (sourceIn), destination (destinationIn) {}

        constexpr Connection (const Connection&) = default;
        constexpr Connection& operator= (const Connection&) = default;

        constexpr bool operator== (const Connection& other) const noexcept
        {
            return source == other.source && destination == other.destination;
        }

        constexpr bool operator!= (const Connection& other) const noexcept
        {
            return ! operator== (other);
        }

        constexpr bool operator<  (const Connection& other) const noexcept
        {
            const auto tie = [] (auto& x)
            {
                return std::tie (x.source.nodeID,
                                 x.destination.nodeID,
                                 x.source.channelIndex,
                                 x.destination.channelIndex);
            };
            return tie (*this) < tie (other);
        }

        //==============================================================================
        /** The channel and node which is the input source for this connection. */
        NodeAndChannel source { {}, 0 };

        /** The channel and node which is the input source for this connection. */
        NodeAndChannel destination { {}, 0 };
    };

    //==============================================================================
    /** Indicates how the graph should be updated after a change.

        If you need to make lots of changes to a graph (e.g. lots of separate calls
        to addNode, addConnection etc.) you can avoid rebuilding the graph on each
        change by using the async update kind.
    */
    enum class UpdateKind
    {
        sync,   ///< Graph should be rebuilt immediately after modification.
        async,  ///< Graph rebuild should be delayed. If you make several changes to the graph
                ///< inside the same call stack, these changes will be applied in one go.
        none    ///< Graph should not be rebuilt automatically. Use rebuild() to trigger a graph
                ///< rebuild.
    };

    //==============================================================================
    /** Deletes all nodes and connections from this graph.
        Any processor objects in the graph will be deleted.
    */
    void clear (UpdateKind = UpdateKind::sync);

    /** Returns the array of nodes in the graph. */
    const ReferenceCountedArray<Node>& getNodes() const noexcept;

    /** Returns the number of nodes in the graph. */
    int getNumNodes() const noexcept                                { return getNodes().size(); }

    /** Returns a pointer to one of the nodes in the graph.
        This will return nullptr if the index is out of range.
        @see getNodeForId
    */
    Node::Ptr getNode (int index) const noexcept                    { return getNodes()[index]; }

    /** Searches the graph for a node with the given ID number and returns it.
        If no such node was found, this returns nullptr.
        @see getNode
    */
    Node* getNodeForId (NodeID) const;

    /** Adds a node to the graph.

        This creates a new node in the graph, for the specified processor. Once you have
        added a processor to the graph, the graph owns it and will delete it later when
        it is no longer needed.

        The optional nodeId parameter lets you specify a unique ID to use for the node.
        If the value is already in use, this method will fail and return an empty node.

        If this succeeds, it returns a pointer to the newly-created node.
    */
    Node::Ptr addNode (std::unique_ptr<AudioProcessor> newProcessor, std::optional<NodeID> nodeId = std::nullopt, UpdateKind = UpdateKind::sync);

    /** Deletes a node within the graph which has the specified ID.
        This will also delete any connections that are attached to this node.
    */
    Node::Ptr removeNode (NodeID, UpdateKind = UpdateKind::sync);

    /** Deletes a node within the graph.
        This will also delete any connections that are attached to this node.
    */
    Node::Ptr removeNode (Node*, UpdateKind = UpdateKind::sync);

    /** Returns the list of connections in the graph. */
    std::vector<Connection> getConnections() const;

    /** Returns true if the given connection exists. */
    bool isConnected (const Connection&) const noexcept;

    /** Returns true if there is a direct connection between any of the channels of
        two specified nodes.
    */
    bool isConnected (NodeID possibleSourceNodeID, NodeID possibleDestNodeID) const noexcept;

    /** Does a recursive check to see if there's a direct or indirect series of connections
        between these two nodes.
    */
    bool isAnInputTo (const Node& source, const Node& destination) const noexcept;

    /** Does a recursive check to see if there's a direct or indirect series of connections
        between these two nodes.
    */
    bool isAnInputTo (NodeID source, NodeID destination) const noexcept;

    /** Returns true if it would be legal to connect the specified points. */
    bool canConnect (const Connection&) const;

    /** Attempts to connect two specified channels of two nodes.

        If this isn't allowed (e.g. because you're trying to connect a midi channel
        to an audio one or other such nonsense), then it'll return false.
    */
    bool addConnection (const Connection&, UpdateKind = UpdateKind::sync);

    /** Deletes the given connection. */
    bool removeConnection (const Connection&, UpdateKind = UpdateKind::sync);

    /** Removes all connections from the specified node. */
    bool disconnectNode (NodeID, UpdateKind = UpdateKind::sync);

    /** Returns true if the given connection's channel numbers map on to valid
        channels at each end.
        Even if a connection is valid when created, its status could change if
        a node changes its channel config.
    */
    bool isConnectionLegal (const Connection&) const;

    /** Performs a sanity checks of all the connections.

        This might be useful if some of the processors are doing things like changing
        their channel counts, which could render some connections obsolete.
    */
    bool removeIllegalConnections (UpdateKind = UpdateKind::sync);

    /** Rebuilds the graph if necessary.

        This function will only ever rebuild the graph on the main thread. If this function is
        called from another thread, the rebuild request will be dispatched asynchronously to the
        main thread.
    */
    void rebuild();

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
        IODeviceType getType() const noexcept                       { return type; }

        /** Returns the parent graph to which this processor belongs, or nullptr if it
            hasn't yet been added to one. */
        AudioProcessorGraph* getParentGraph() const noexcept        { return graph; }

        /** True if this is an audio or midi input. */
        bool isInput() const noexcept;
        /** True if this is an audio or midi output. */
        bool isOutput() const noexcept;

        //==============================================================================
        AudioGraphIOProcessor (IODeviceType);
        ~AudioGraphIOProcessor() override;

        const String getName() const override;
        void fillInPluginDescription (PluginDescription&) const override;
        void prepareToPlay (double newSampleRate, int estimatedSamplesPerBlock) override;
        void releaseResources() override;
        void processBlock (AudioBuffer<float>& , MidiBuffer&) override;
        void processBlock (AudioBuffer<double>&, MidiBuffer&) override;
        bool supportsDoublePrecisionProcessing() const override;

        double getTailLengthSeconds() const override;
        bool acceptsMidi() const override;
        bool producesMidi() const override;

        bool hasEditor() const override;
        AudioProcessorEditor* createEditor() override;

        int getNumPrograms() override;
        int getCurrentProgram() override;
        void setCurrentProgram (int) override;
        const String getProgramName (int) override;
        void changeProgramName (int, const String&) override;

        void getStateInformation (juce::MemoryBlock& destData) override;
        void setStateInformation (const void* data, int sizeInBytes) override;

        /** @internal */
        void setParentGraph (AudioProcessorGraph*);

    private:
        const IODeviceType type;
        AudioProcessorGraph* graph = nullptr;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioGraphIOProcessor)
    };

    //==============================================================================
    const String getName() const override;
    void prepareToPlay (double, int) override;
    void releaseResources() override;
    void processBlock (AudioBuffer<float>&,  MidiBuffer&) override;
    void processBlock (AudioBuffer<double>&, MidiBuffer&) override;
    bool supportsDoublePrecisionProcessing() const override;

    void reset() override;
    void setNonRealtime (bool) noexcept override;

    double getTailLengthSeconds() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;

    bool hasEditor() const override                         { return false; }
    AudioProcessorEditor* createEditor() override           { return nullptr; }
    int getNumPrograms() override                           { return 0; }
    int getCurrentProgram() override                        { return 0; }
    void setCurrentProgram (int) override                   { }
    const String getProgramName (int) override              { return {}; }
    void changeProgramName (int, const String&) override    { }
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    class Pimpl;
    std::unique_ptr<Pimpl> pimpl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorGraph)
};

} // namespace juce
