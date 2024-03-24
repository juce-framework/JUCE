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

// Implementation notes:
// On macOS, calling AudioUnitInitialize will internally call AudioObjectGetPropertyData, which
// takes a mutex.
// This same mutex is taken on the audio thread, before calling the audio device's IO callback.
// This is a property of the CoreAudio implementation - we can't remove or interact directly
// with these locks in JUCE.
//
// AudioProcessor instances expect that their callback lock will be taken before calling
// processBlock or processBlockBypassed.
// This means that, to avoid deadlocks, we *always* need to make sure that the CoreAudio mutex
// is locked before taking the callback lock.
// Given that we can't interact with the CoreAudio mutex directly, on the main thread we can't
// call any function that might internally interact with CoreAudio while the callback lock is
// taken.
// In particular, be careful not to call `prepareToPlay` on a hosted AudioUnit from the main
// thread while the callback lock is taken.
// The graph implementation currently makes sure to call prepareToPlay on the main thread,
// without taking the graph's callback lock.

namespace juce
{

/*  Provides a comparison function for various types that have an associated NodeID,
    for use with equal_range, lower_bound etc.
*/
class ImplicitNode
{
public:
    using Node           = AudioProcessorGraph::Node;
    using NodeID         = AudioProcessorGraph::NodeID;
    using NodeAndChannel = AudioProcessorGraph::NodeAndChannel;

    ImplicitNode (NodeID x) : node (x) {}
    ImplicitNode (NodeAndChannel x) : ImplicitNode (x.nodeID) {}
    ImplicitNode (const Node* x) : ImplicitNode (x->nodeID) {}
    ImplicitNode (const std::pair<const NodeAndChannel, std::set<NodeAndChannel>>& x) : ImplicitNode (x.first) {}

    /*  This is the comparison function. */
    static bool compare (ImplicitNode a, ImplicitNode b) { return a.node < b.node; }

private:
    NodeID node;
};

//==============================================================================
/*  A copyable type holding all the nodes, and allowing fast lookup by id. */
class Nodes
{
public:
    using Node           = AudioProcessorGraph::Node;
    using NodeID         = AudioProcessorGraph::NodeID;

    const ReferenceCountedArray<Node>& getNodes() const { return array; }

    Node::Ptr getNodeForId (NodeID nodeID) const
    {
        const auto iter = std::lower_bound (array.begin(), array.end(), nodeID, ImplicitNode::compare);
        return iter != array.end() && (*iter)->nodeID == nodeID ? *iter : nullptr;
    }

    Node::Ptr addNode (std::unique_ptr<AudioProcessor> newProcessor, const NodeID nodeID)
    {
        if (newProcessor == nullptr)
        {
            // Cannot add a null audio processor!
            jassertfalse;
            return {};
        }

        if (std::any_of (array.begin(),
                         array.end(),
                         [&] (auto* n) { return n->getProcessor() == newProcessor.get(); }))
        {
            // This audio processor has already been added to the graph!
            jassertfalse;
            return {};
        }

        const auto iter = std::lower_bound (array.begin(), array.end(), nodeID, ImplicitNode::compare);

        if (iter != array.end() && (*iter)->nodeID == nodeID)
        {
            // This nodeID has already been used for a node in the graph!
            jassertfalse;
            return {};
        }

        return array.insert ((int) std::distance (array.begin(), iter),
                             new Node { nodeID, std::move (newProcessor) });
    }

    Node::Ptr removeNode (NodeID nodeID)
    {
        const auto iter = std::lower_bound (array.begin(), array.end(), nodeID, ImplicitNode::compare);
        return iter != array.end() && (*iter)->nodeID == nodeID
             ? array.removeAndReturn ((int) std::distance (array.begin(), iter))
             : nullptr;
    }

    bool operator== (const Nodes& other) const { return array == other.array; }
    bool operator!= (const Nodes& other) const { return array != other.array; }

private:
    ReferenceCountedArray<Node> array;
};

//==============================================================================
/*  A value type holding a full set of graph connections. */
class Connections
{
public:
    using Node           = AudioProcessorGraph::Node;
    using NodeID         = AudioProcessorGraph::NodeID;
    using Connection     = AudioProcessorGraph::Connection;
    using NodeAndChannel = AudioProcessorGraph::NodeAndChannel;

private:
    static auto equalRange (const std::set<NodeAndChannel>& pins, const NodeID node)
    {
        return std::equal_range (pins.cbegin(), pins.cend(), node, ImplicitNode::compare);
    }

    using Map = std::map<NodeAndChannel, std::set<NodeAndChannel>>;

public:
    static constexpr auto midiChannelIndex = AudioProcessorGraph::midiChannelIndex;

    bool addConnection (const Nodes& n, const Connection& c)
    {
        if (! canConnect (n, c))
            return false;

        sourcesForDestination[c.destination].insert (c.source);
        jassert (isConnected (c));
        return true;
    }

    bool removeConnection (const Connection& c)
    {
        const auto iter = sourcesForDestination.find (c.destination);
        return iter != sourcesForDestination.cend() && iter->second.erase (c.source) == 1;
    }

    bool removeIllegalConnections (const Nodes& n)
    {
        auto anyRemoved = false;

        for (auto& dest : sourcesForDestination)
        {
            const auto initialSize = dest.second.size();
            dest.second = removeIllegalConnections (n, std::move (dest.second), dest.first);
            anyRemoved |= (dest.second.size() != initialSize);
        }

        return anyRemoved;
    }

    bool disconnectNode (NodeID n)
    {
        const auto matchingDestinations = getMatchingDestinations (n);
        auto result = matchingDestinations.first != matchingDestinations.second;
        sourcesForDestination.erase (matchingDestinations.first, matchingDestinations.second);

        for (auto& pair : sourcesForDestination)
        {
            const auto range = equalRange (pair.second, n);
            result |= range.first != range.second;
            pair.second.erase (range.first, range.second);
        }

        return result;
    }

    static bool isConnectionLegal (const Nodes& n, Connection c)
    {
        const auto source = n.getNodeForId (c.source     .nodeID);
        const auto dest   = n.getNodeForId (c.destination.nodeID);

        const auto sourceChannel = c.source     .channelIndex;
        const auto destChannel   = c.destination.channelIndex;

        const auto sourceIsMIDI = AudioProcessorGraph::midiChannelIndex == sourceChannel;
        const auto destIsMIDI   = AudioProcessorGraph::midiChannelIndex == destChannel;

        return sourceChannel >= 0
            && destChannel >= 0
            && source != dest
            && sourceIsMIDI == destIsMIDI
            && source != nullptr
            && (sourceIsMIDI
                    ? source->getProcessor()->producesMidi()
                    : sourceChannel < source->getProcessor()->getTotalNumOutputChannels())
            && dest != nullptr
            && (destIsMIDI
                    ? dest->getProcessor()->acceptsMidi()
                    : destChannel < dest->getProcessor()->getTotalNumInputChannels());
    }

    bool canConnect (const Nodes& n, Connection c) const
    {
        return isConnectionLegal (n, c) && ! isConnected (c);
    }

    bool isConnected (Connection c) const
    {
        const auto iter = sourcesForDestination.find (c.destination);

        return iter != sourcesForDestination.cend()
               && iter->second.find (c.source) != iter->second.cend();
    }

    bool isConnected (NodeID srcID, NodeID destID) const
    {
        const auto matchingDestinations = getMatchingDestinations (destID);

        return std::any_of (matchingDestinations.first, matchingDestinations.second, [srcID] (const auto& pair)
        {
            const auto [begin, end] = equalRange (pair.second, srcID);
            return begin != end;
        });
    }

    std::set<NodeID> getSourceNodesForDestination (NodeID destID) const
    {
        const auto matchingDestinations = getMatchingDestinations (destID);

        std::set<NodeID> result;
        std::for_each (matchingDestinations.first, matchingDestinations.second, [&] (const auto& pair)
        {
            for (const auto& source : pair.second)
                result.insert (source.nodeID);
        });
        return result;
    }

    std::set<NodeAndChannel> getSourcesForDestination (const NodeAndChannel& p) const
    {
        const auto iter = sourcesForDestination.find (p);
        return iter != sourcesForDestination.cend() ? iter->second : std::set<NodeAndChannel>{};
    }

    std::vector<Connection> getConnections() const
    {
        std::vector<Connection> result;

        for (auto& pair : sourcesForDestination)
            for (const auto& source : pair.second)
                result.emplace_back (source, pair.first);

        std::sort (result.begin(), result.end());
        result.erase (std::unique (result.begin(), result.end()), result.end());
        return result;
    }

    bool isAnInputTo (NodeID source, NodeID dest) const
    {
        return getConnectedRecursive (source, dest, {}).found;
    }

    bool operator== (const Connections& other) const { return sourcesForDestination == other.sourcesForDestination; }
    bool operator!= (const Connections& other) const { return sourcesForDestination != other.sourcesForDestination; }

    class DestinationsForSources
    {
    public:
        explicit DestinationsForSources (Map m) : map (std::move (m)) {}

        bool isSourceConnectedToDestinationNodeIgnoringChannel (const NodeAndChannel& source, NodeID dest, int channel) const
        {
            if (const auto destIter = map.find (source); destIter != map.cend())
            {
                const auto [begin, end] = equalRange (destIter->second, dest);
                return std::any_of (begin, end, [&] (const NodeAndChannel& nodeAndChannel)
                {
                    return nodeAndChannel != NodeAndChannel { dest, channel };
                });
            }

            return false;
        }

    private:
        Map map;
    };

    /*  Reverses the graph, to allow fast lookup by source.
        This is expensive, don't call this more than necessary!
    */
    auto getDestinationsForSources() const
    {
        Map destinationsForSources;

        for (const auto& [destination, sources] : sourcesForDestination)
            for (const auto& source : sources)
                destinationsForSources[source].insert (destination);

        return DestinationsForSources (std::move (destinationsForSources));
    }

private:
    struct SearchState
    {
        std::set<NodeID> visited;
        bool found = false;
    };

    SearchState getConnectedRecursive (NodeID source, NodeID dest, SearchState state) const
    {
        state.visited.insert (dest);

        for (const auto& s : getSourceNodesForDestination (dest))
        {
            if (state.found || s == source)
                return { std::move (state.visited), true };

            if (state.visited.find (s) == state.visited.cend())
                state = getConnectedRecursive (source, s, std::move (state));
        }

        return state;
    }

    static std::set<NodeAndChannel> removeIllegalConnections (const Nodes& nodes,
                                                              std::set<NodeAndChannel> sources,
                                                              NodeAndChannel destination)
    {
        for (auto source = sources.cbegin(); source != sources.cend();)
        {
            if (! isConnectionLegal (nodes, { *source, destination }))
                source = sources.erase (source);
            else
                ++source;
        }

        return sources;
    }

    std::pair<Map::const_iterator, Map::const_iterator> getMatchingDestinations (NodeID destID) const
    {
        return std::equal_range (sourcesForDestination.cbegin(), sourcesForDestination.cend(), destID, ImplicitNode::compare);
    }

    Map sourcesForDestination;
};

//==============================================================================
/*  Settings used to prepare a node for playback. */
struct PrepareSettings
{
    using ProcessingPrecision = AudioProcessorGraph::ProcessingPrecision;

    ProcessingPrecision precision = ProcessingPrecision::singlePrecision;
    double sampleRate             = 0.0;
    int blockSize                 = 0;

    auto tie() const noexcept { return std::tie (precision, sampleRate, blockSize); }

    bool operator== (const PrepareSettings& other) const { return tie() == other.tie(); }
    bool operator!= (const PrepareSettings& other) const { return tie() != other.tie(); }
};

//==============================================================================
/*  Keeps track of the PrepareSettings applied to each node. */
class NodeStates
{
public:
    using Node           = AudioProcessorGraph::Node;
    using NodeID         = AudioProcessorGraph::NodeID;

    /*  Called from prepareToPlay and releaseResources with the PrepareSettings that should be
        used next time the graph is rebuilt.
    */
    void setState (std::optional<PrepareSettings> newSettings)
    {
        const std::lock_guard<std::mutex> lock (mutex);
        next = newSettings;
    }

    /*  Call from the audio thread only. */
    std::optional<PrepareSettings> getLastRequestedSettings() const { return next; }

    /*  Call from the main thread only!

        Called after updating the graph topology to prepare any currently-unprepared nodes.

        To ensure that all nodes are initialised with the same sample rate, buffer size, etc. as
        the enclosing graph, we must ensure that any operation that uses these details (preparing
        individual nodes) is synchronized with prepare-to-play and release-resources on the
        enclosing graph.

        If the new PrepareSettings are different to the last-seen settings, all nodes will
        be prepared/unprepared as necessary. If the PrepareSettings have not changed, then only
        new nodes will be prepared/unprepared.

        Returns the settings that were applied to the nodes.
    */
    std::optional<PrepareSettings> applySettings (const Nodes& n)
    {
        const auto settingsChanged = [this]
        {
            const std::lock_guard<std::mutex> lock (mutex);
            const auto result = current != next;
            current = next;
            return result;
        }();

        // It may look like releaseResources and prepareToPlay could race with calls to processBlock
        // here, because applySettings is called from the main thread, processBlock is called from
        // the audio thread (normally), and there's no explicit mutex ensuring that the calls don't
        // overlap.
        // However, it is part of the AudioProcessor contract that users shall not call
        // processBlock, prepareToPlay, and/or releaseResources concurrently. That is, there's an
        // implied mutex synchronising these functions on each AudioProcessor.
        //
        // Inside processBlock, we always ensure that the current RenderSequence's PrepareSettings
        // match the graph's settings before attempting to call processBlock on any of the graph
        // nodes; as a result, it's impossible to start calling processBlock on a node on the audio
        // thread while a render sequence rebuild (including prepareToPlay/releaseResources calls)
        // is already in progress here.
        //
        // Due to the implied mutex between prepareToPlay/releaseResources/processBlock, it's also
        // impossible to receive new PrepareSettings and to start a new RenderSequence rebuild while
        // a processBlock call is in progress.

        if (settingsChanged)
        {
            for (const auto& node : n.getNodes())
                node->getProcessor()->releaseResources();

            preparedNodes.clear();
        }

        if (current.has_value())
        {
            for (const auto& node : n.getNodes())
            {
                if (preparedNodes.find (node->nodeID) != preparedNodes.cend())
                    continue;

                preparedNodes.insert (node->nodeID);

                node->getProcessor()->setProcessingPrecision (node->getProcessor()->supportsDoublePrecisionProcessing() ? current->precision
                                                                                                                        : AudioProcessor::singlePrecision);
                node->getProcessor()->setRateAndBufferSizeDetails (current->sampleRate, current->blockSize);
                node->getProcessor()->prepareToPlay               (current->sampleRate, current->blockSize);
            }
        }

        return current;
    }

    /*  Call from the main thread to indicate that a node has been removed from the graph.
    */
    void removeNode (const NodeID n)
    {
        preparedNodes.erase (n);
    }

    /*  Call from the main thread to indicate that all nodes have been removed from the graph.
    */
    void clear()
    {
        preparedNodes.clear();
    }

private:
    std::mutex mutex;
    std::set<NodeID> preparedNodes;
    std::optional<PrepareSettings> current, next;
};

//==============================================================================
template <typename FloatType>
struct GraphRenderSequence
{
    using Node = AudioProcessorGraph::Node;

    struct GlobalIO
    {
        AudioBuffer<FloatType>& audioIn;
        AudioBuffer<FloatType>& audioOut;
        MidiBuffer& midiIn;
        MidiBuffer& midiOut;
    };

    struct Context
    {
        GlobalIO globalIO;
        AudioPlayHead* audioPlayHead;
        int numSamples;
    };

    void perform (AudioBuffer<FloatType>& buffer, MidiBuffer& midiMessages, AudioPlayHead* audioPlayHead)
    {
        auto numSamples = buffer.getNumSamples();
        auto maxSamples = renderingBuffer.getNumSamples();

        if (numSamples > maxSamples)
        {
            // Being asked to render more samples than our buffers have, so divide the buffer into chunks
            int chunkStartSample = 0;
            while (chunkStartSample < numSamples)
            {
                auto chunkSize = jmin (maxSamples, numSamples - chunkStartSample);

                AudioBuffer<FloatType> audioChunk (buffer.getArrayOfWritePointers(), buffer.getNumChannels(), chunkStartSample, chunkSize);
                midiChunk.clear();
                midiChunk.addEvents (midiMessages, chunkStartSample, chunkSize, -chunkStartSample);

                // Splitting up the buffer like this will cause the play head and host time to be
                // invalid for all but the first chunk...
                perform (audioChunk, midiChunk, audioPlayHead);

                chunkStartSample += maxSamples;
            }

            return;
        }

        currentAudioOutputBuffer.setSize (jmax (1, buffer.getNumChannels()), numSamples);
        currentAudioOutputBuffer.clear();
        currentMidiOutputBuffer.clear();

        {
            const Context context { { buffer,
                                      currentAudioOutputBuffer,
                                      midiMessages,
                                      currentMidiOutputBuffer },
                                    audioPlayHead,
                                    numSamples };

            for (const auto& op : renderOps)
                op->process (context);
        }

        for (int i = 0; i < buffer.getNumChannels(); ++i)
            buffer.copyFrom (i, 0, currentAudioOutputBuffer, i, 0, numSamples);

        midiMessages.clear();
        midiMessages.addEvents (currentMidiOutputBuffer, 0, buffer.getNumSamples(), 0);
    }

    JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4661)

    void addClearChannelOp (int index)
    {
        struct ClearOp final : public RenderOp
        {
            explicit ClearOp (int indexIn) : index (indexIn) {}

            void prepare (FloatType* const* renderBuffer, MidiBuffer*) override
            {
                channelBuffer = renderBuffer[index];
            }

            void process (const Context& c) override
            {
                FloatVectorOperations::clear (channelBuffer, c.numSamples);
            }

            FloatType* channelBuffer = nullptr;
            int index = 0;
        };

        renderOps.push_back (std::make_unique<ClearOp> (index));
    }

    void addCopyChannelOp (int srcIndex, int dstIndex)
    {
        struct CopyOp final : public RenderOp
        {
            explicit CopyOp (int fromIn, int toIn) : from (fromIn), to (toIn) {}

            void prepare (FloatType* const* renderBuffer, MidiBuffer*) override
            {
                fromBuffer = renderBuffer[from];
                toBuffer = renderBuffer[to];
            }

            void process (const Context& c) override
            {
                FloatVectorOperations::copy (toBuffer, fromBuffer, c.numSamples);
            }

            FloatType* fromBuffer = nullptr;
            FloatType* toBuffer = nullptr;
            int from = 0, to = 0;
        };

        renderOps.push_back (std::make_unique<CopyOp> (srcIndex, dstIndex));
    }

    void addAddChannelOp (int srcIndex, int dstIndex)
    {
        struct AddOp final : public RenderOp
        {
            explicit AddOp (int fromIn, int toIn) : from (fromIn), to (toIn) {}

            void prepare (FloatType* const* renderBuffer, MidiBuffer*) override
            {
                fromBuffer = renderBuffer[from];
                toBuffer = renderBuffer[to];
            }

            void process (const Context& c) override
            {
                FloatVectorOperations::add (toBuffer, fromBuffer, c.numSamples);
            }

            FloatType* fromBuffer = nullptr;
            FloatType* toBuffer = nullptr;
            int from = 0, to = 0;
        };

        renderOps.push_back (std::make_unique<AddOp> (srcIndex, dstIndex));
    }

    JUCE_END_IGNORE_WARNINGS_MSVC

    void addClearMidiBufferOp (int index)
    {
        struct ClearOp final : public RenderOp
        {
            explicit ClearOp (int indexIn) : index (indexIn) {}

            void prepare (FloatType* const*, MidiBuffer* buffers) override
            {
                channelBuffer = buffers + index;
            }

            void process (const Context&) override
            {
                channelBuffer->clear();
            }

            MidiBuffer* channelBuffer = nullptr;
            int index = 0;
        };

        renderOps.push_back (std::make_unique<ClearOp> (index));
    }

    void addCopyMidiBufferOp (int srcIndex, int dstIndex)
    {
        struct CopyOp final : public RenderOp
        {
            explicit CopyOp (int fromIn, int toIn) : from (fromIn), to (toIn) {}

            void prepare (FloatType* const*, MidiBuffer* buffers) override
            {
                fromBuffer = buffers + from;
                toBuffer = buffers + to;
            }

            void process (const Context&) override
            {
                *toBuffer = *fromBuffer;
            }

            MidiBuffer* fromBuffer = nullptr;
            MidiBuffer* toBuffer = nullptr;
            int from = 0, to = 0;
        };

        renderOps.push_back (std::make_unique<CopyOp> (srcIndex, dstIndex));
    }

    void addAddMidiBufferOp (int srcIndex, int dstIndex)
    {
        struct AddOp final : public RenderOp
        {
            explicit AddOp (int fromIn, int toIn) : from (fromIn), to (toIn) {}

            void prepare (FloatType* const*, MidiBuffer* buffers) override
            {
                fromBuffer = buffers + from;
                toBuffer = buffers + to;
            }

            void process (const Context& c) override
            {
                toBuffer->addEvents (*fromBuffer, 0, c.numSamples, 0);
            }

            MidiBuffer* fromBuffer = nullptr;
            MidiBuffer* toBuffer = nullptr;
            int from = 0, to = 0;
        };

        renderOps.push_back (std::make_unique<AddOp> (srcIndex, dstIndex));
    }

    void addDelayChannelOp (int chan, int delaySize)
    {
        struct DelayChannelOp final : public RenderOp
        {
            DelayChannelOp (int chan, int delaySize)
                : buffer ((size_t) (delaySize + 1), (FloatType) 0),
                  channel (chan),
                  writeIndex (delaySize)
            {
            }

            void prepare (FloatType* const* renderBuffer, MidiBuffer*) override
            {
                channelBuffer = renderBuffer[channel];
            }

            void process (const Context& c) override
            {
                auto* data = channelBuffer;

                for (int i = c.numSamples; --i >= 0;)
                {
                    buffer[(size_t) writeIndex] = *data;
                    *data++ = buffer[(size_t) readIndex];

                    if (++readIndex  >= (int) buffer.size()) readIndex = 0;
                    if (++writeIndex >= (int) buffer.size()) writeIndex = 0;
                }
            }

            std::vector<FloatType> buffer;
            FloatType* channelBuffer = nullptr;
            const int channel;
            int readIndex = 0, writeIndex;
        };

        renderOps.push_back (std::make_unique<DelayChannelOp> (chan, delaySize));
    }

    void addProcessOp (const Node::Ptr& node,
                       const Array<int>& audioChannelsUsed,
                       int totalNumChans,
                       int midiBuffer)
    {
        auto op = [&]() -> std::unique_ptr<NodeOp>
        {
            if (auto* ioNode = dynamic_cast<const AudioProcessorGraph::AudioGraphIOProcessor*> (node->getProcessor()))
            {
                switch (ioNode->getType())
                {
                    case AudioProcessorGraph::AudioGraphIOProcessor::audioInputNode:
                        return std::make_unique<AudioInOp> (node, audioChannelsUsed, totalNumChans, midiBuffer);

                    case AudioProcessorGraph::AudioGraphIOProcessor::audioOutputNode:
                        return std::make_unique<AudioOutOp> (node, audioChannelsUsed, totalNumChans, midiBuffer);

                    case AudioProcessorGraph::AudioGraphIOProcessor::midiInputNode:
                        return std::make_unique<MidiInOp> (node, audioChannelsUsed, totalNumChans, midiBuffer);

                    case AudioProcessorGraph::AudioGraphIOProcessor::midiOutputNode:
                        return std::make_unique<MidiOutOp> (node, audioChannelsUsed, totalNumChans, midiBuffer);
                }
            }

            return std::make_unique<ProcessOp> (node, audioChannelsUsed, totalNumChans, midiBuffer);
        }();

        renderOps.push_back (std::move (op));
    }

    void prepareBuffers (int blockSize)
    {
        renderingBuffer.setSize (numBuffersNeeded + 1, blockSize);
        renderingBuffer.clear();
        currentAudioOutputBuffer.setSize (numBuffersNeeded + 1, blockSize);
        currentAudioOutputBuffer.clear();

        currentMidiOutputBuffer.clear();

        midiBuffers.clearQuick();
        midiBuffers.resize (numMidiBuffersNeeded);

        const int defaultMIDIBufferSize = 512;

        midiChunk.ensureSize (defaultMIDIBufferSize);

        for (auto&& m : midiBuffers)
            m.ensureSize (defaultMIDIBufferSize);

        for (const auto& op : renderOps)
            op->prepare (renderingBuffer.getArrayOfWritePointers(), midiBuffers.data());
    }

    int numBuffersNeeded = 0, numMidiBuffersNeeded = 0;

    AudioBuffer<FloatType> renderingBuffer, currentAudioOutputBuffer;

    MidiBuffer currentMidiOutputBuffer;

    Array<MidiBuffer> midiBuffers;
    MidiBuffer midiChunk;

private:
    //==============================================================================
    struct RenderOp
    {
        virtual ~RenderOp() = default;
        virtual void prepare (FloatType* const*, MidiBuffer*) = 0;
        virtual void process (const Context&) = 0;
    };

    struct NodeOp : public RenderOp
    {
        NodeOp (const Node::Ptr& n,
                const Array<int>& audioChannelsUsed,
                int totalNumChans,
                int midiBufferIndex)
            : node (n),
              processor (*n->getProcessor()),
              audioChannelsToUse (audioChannelsUsed),
              audioChannels ((size_t) jmax (1, totalNumChans), nullptr),
              midiBufferToUse (midiBufferIndex)
        {
            while (audioChannelsToUse.size() < (int) audioChannels.size())
                audioChannelsToUse.add (0);
        }

        void prepare (FloatType* const* renderBuffer, MidiBuffer* buffers) final
        {
            for (size_t i = 0; i < audioChannels.size(); ++i)
                audioChannels[i] = renderBuffer[audioChannelsToUse.getUnchecked ((int) i)];

            midiBuffer = buffers + midiBufferToUse;
        }

        void process (const Context& c) final
        {
            processor.setPlayHead (c.audioPlayHead);

            auto numAudioChannels = [this]
            {
                if (const auto* proc = node->getProcessor())
                    if (proc->getTotalNumInputChannels() == 0 && proc->getTotalNumOutputChannels() == 0)
                        return 0;

                return (int) audioChannels.size();
            }();

            AudioBuffer<FloatType> buffer { audioChannels.data(), numAudioChannels, c.numSamples };

            if (processor.isSuspended())
            {
                buffer.clear();
            }
            else
            {
                const auto bypass = node->isBypassed() && processor.getBypassParameter() == nullptr;
                processWithBuffer (c.globalIO, bypass, buffer, *midiBuffer);
            }
        }

        virtual void processWithBuffer (const GlobalIO&, bool bypass, AudioBuffer<FloatType>& audio, MidiBuffer& midi) = 0;

        const Node::Ptr node;
        AudioProcessor& processor;
        MidiBuffer* midiBuffer = nullptr;

        Array<int> audioChannelsToUse;
        std::vector<FloatType*> audioChannels;
        const int midiBufferToUse;
    };

    struct ProcessOp final : public NodeOp
    {
        using NodeOp::NodeOp;

        void processWithBuffer (const GlobalIO&, bool bypass, AudioBuffer<FloatType>& audio, MidiBuffer& midi) final
        {
            callProcess (bypass, audio, midi);
        }

        void callProcess (bool bypass, AudioBuffer<float>& buffer, MidiBuffer& midi)
        {
            if (this->processor.isUsingDoublePrecision())
            {
                tempBufferDouble.makeCopyOf (buffer, true);
                processImpl (bypass, this->processor, tempBufferDouble, midi);
                buffer.makeCopyOf (tempBufferDouble, true);
            }
            else
            {
                processImpl (bypass, this->processor, buffer, midi);
            }
        }

        void callProcess (bool bypass, AudioBuffer<double>& buffer, MidiBuffer& midi)
        {
            if (this->processor.isUsingDoublePrecision())
            {
                processImpl (bypass, this->processor, buffer, midi);
            }
            else
            {
                tempBufferFloat.makeCopyOf (buffer, true);
                processImpl (bypass, this->processor, tempBufferFloat, midi);
                buffer.makeCopyOf (tempBufferFloat, true);
            }
        }

        template <typename Value>
        static void processImpl (bool bypass, AudioProcessor& p, AudioBuffer<Value>& audio, MidiBuffer& midi)
        {
            if (bypass)
                p.processBlockBypassed (audio, midi);
            else
                p.processBlock (audio, midi);
        }

        AudioBuffer<float> tempBufferFloat, tempBufferDouble;
    };

    struct MidiInOp final : public NodeOp
    {
        using NodeOp::NodeOp;

        void processWithBuffer (const GlobalIO& g, bool bypass, AudioBuffer<FloatType>& audio, MidiBuffer& midi) final
        {
            if (! bypass)
                midi.addEvents (g.midiIn, 0, audio.getNumSamples(), 0);
        }
    };

    struct MidiOutOp final : public NodeOp
    {
        using NodeOp::NodeOp;

        void processWithBuffer (const GlobalIO& g, bool bypass, AudioBuffer<FloatType>& audio, MidiBuffer& midi) final
        {
            if (! bypass)
                g.midiOut.addEvents (midi, 0, audio.getNumSamples(), 0);
        }
    };

    struct AudioInOp final : public NodeOp
    {
        using NodeOp::NodeOp;

        void processWithBuffer (const GlobalIO& g, bool bypass, AudioBuffer<FloatType>& audio, MidiBuffer&) final
        {
            if (bypass)
                return;

            for (int i = jmin (g.audioIn.getNumChannels(), audio.getNumChannels()); --i >= 0;)
                audio.copyFrom (i, 0, g.audioIn, i, 0, audio.getNumSamples());
        }
    };

    struct AudioOutOp final : public NodeOp
    {
        using NodeOp::NodeOp;

        void processWithBuffer (const GlobalIO& g, bool bypass, AudioBuffer<FloatType>& audio, MidiBuffer&) final
        {
            if (bypass)
                return;

            for (int i = jmin (g.audioOut.getNumChannels(), audio.getNumChannels()); --i >= 0;)
                g.audioOut.addFrom (i, 0, audio, i, 0, audio.getNumSamples());
        }
    };

    std::vector<std::unique_ptr<RenderOp>> renderOps;
};

//==============================================================================
struct SequenceAndLatency
{
    using RenderSequenceVariant = std::variant<GraphRenderSequence<float>,
                                               GraphRenderSequence<double>>;

    RenderSequenceVariant sequence;
    int latencySamples = 0;
};

//==============================================================================
class RenderSequenceBuilder
{
public:
    using Node           = AudioProcessorGraph::Node;
    using NodeID         = AudioProcessorGraph::NodeID;
    using Connection     = AudioProcessorGraph::Connection;
    using NodeAndChannel = AudioProcessorGraph::NodeAndChannel;

    static constexpr auto midiChannelIndex = AudioProcessorGraph::midiChannelIndex;

    template <typename FloatType>
    static SequenceAndLatency build (const Nodes& n, const Connections& c)
    {
        GraphRenderSequence<FloatType> sequence;
        const RenderSequenceBuilder builder (n, c, sequence);
        return { std::move (sequence), builder.totalLatency };
    }

private:
    //==============================================================================
    const Array<Node*> orderedNodes;

    struct AssignedBuffer
    {
        NodeAndChannel channel;

        static AssignedBuffer createReadOnlyEmpty() noexcept    { return { { zeroNodeID(), 0 } }; }
        static AssignedBuffer createFree() noexcept             { return { { freeNodeID(), 0 } }; }

        bool isReadOnlyEmpty() const noexcept                   { return channel.nodeID == zeroNodeID(); }
        bool isFree() const noexcept                            { return channel.nodeID == freeNodeID(); }
        bool isAssigned() const noexcept                        { return ! (isReadOnlyEmpty() || isFree()); }

        void setFree() noexcept                                 { channel = { freeNodeID(), 0 }; }
        void setAssignedToNonExistentNode() noexcept            { channel = { anonNodeID(), 0 }; }

    private:
        static NodeID anonNodeID() { return NodeID (0x7ffffffd); }
        static NodeID zeroNodeID() { return NodeID (0x7ffffffe); }
        static NodeID freeNodeID() { return NodeID (0x7fffffff); }
    };

    Array<AssignedBuffer> audioBuffers, midiBuffers;

    enum { readOnlyEmptyBufferIndex = 0 };

    std::unordered_map<uint32, int> delays;
    int totalLatency = 0;

    int getNodeDelay (NodeID nodeID) const noexcept
    {
        const auto iter = delays.find (nodeID.uid);
        return iter != delays.end() ? iter->second : 0;
    }

    int getInputLatencyForNode (const Connections& c, NodeID nodeID) const
    {
        const auto sources = c.getSourceNodesForDestination (nodeID);
        return std::accumulate (sources.cbegin(), sources.cend(), 0, [this] (auto acc, auto source)
        {
            return jmax (acc, this->getNodeDelay (source));
        });
    }

    //==============================================================================
    void getAllParentsOfNode (const NodeID& child,
                              std::set<NodeID>& parents,
                              const std::map<NodeID, std::set<NodeID>>& otherParents,
                              const Connections& c)
    {
        for (const auto& parentNode : c.getSourceNodesForDestination (child))
        {
            if (parentNode == child)
                continue;

            if (parents.insert (parentNode).second)
            {
                const auto parentParents = otherParents.find (parentNode);

                if (parentParents != otherParents.end())
                {
                    parents.insert (parentParents->second.begin(), parentParents->second.end());
                    continue;
                }

                getAllParentsOfNode (parentNode, parents, otherParents, c);
            }
        }
    }

    Array<Node*> createOrderedNodeList (const Nodes& n, const Connections& c)
    {
        Array<Node*> result;

        std::map<NodeID, std::set<NodeID>> nodeParents;

        for (auto& node : n.getNodes())
        {
            const auto nodeID = node->nodeID;
            int insertionIndex = 0;

            for (; insertionIndex < result.size(); ++insertionIndex)
            {
                auto& parents = nodeParents[result.getUnchecked (insertionIndex)->nodeID];

                if (parents.find (nodeID) != parents.end())
                    break;
            }

            result.insert (insertionIndex, node);
            getAllParentsOfNode (nodeID, nodeParents[node->nodeID], nodeParents, c);
        }

        return result;
    }

    //==============================================================================
    template <typename RenderSequence>
    int findBufferForInputAudioChannel (const Connections& c,
                                        const Connections::DestinationsForSources& reversed,
                                        RenderSequence& sequence,
                                        Node& node,
                                        const int inputChan,
                                        const int ourRenderingIndex,
                                        const int maxLatency)
    {
        auto& processor = *node.getProcessor();
        auto numOuts = processor.getTotalNumOutputChannels();

        auto sources = c.getSourcesForDestination ({ node.nodeID, inputChan });

        // Handle an unconnected input channel...
        if (sources.empty())
        {
            if (inputChan >= numOuts)
                return readOnlyEmptyBufferIndex;

            auto index = getFreeBuffer (audioBuffers);
            sequence.addClearChannelOp (index);
            return index;
        }

        // Handle an input from a single source..
        if (sources.size() == 1)
        {
            // channel with a straightforward single input..
            auto src = *sources.begin();

            int bufIndex = getBufferContaining (src);

            if (bufIndex < 0)
            {
                // if not found, this is probably a feedback loop
                bufIndex = readOnlyEmptyBufferIndex;
                jassert (bufIndex >= 0);
            }

            if (inputChan < numOuts && isBufferNeededLater (reversed, ourRenderingIndex, inputChan, src))
            {
                // can't mess up this channel because it's needed later by another node,
                // so we need to use a copy of it..
                auto newFreeBuffer = getFreeBuffer (audioBuffers);
                sequence.addCopyChannelOp (bufIndex, newFreeBuffer);
                bufIndex = newFreeBuffer;
            }

            auto nodeDelay = getNodeDelay (src.nodeID);

            if (nodeDelay < maxLatency)
                sequence.addDelayChannelOp (bufIndex, maxLatency - nodeDelay);

            return bufIndex;
        }

        // Handle a mix of several outputs coming into this input..
        int reusableInputIndex = -1;
        int bufIndex = -1;

        {
            auto i = 0;
            for (const auto& src : sources)
            {
                auto sourceBufIndex = getBufferContaining (src);

                if (sourceBufIndex >= 0 && ! isBufferNeededLater (reversed, ourRenderingIndex, inputChan, src))
                {
                    // we've found one of our input chans that can be re-used..
                    reusableInputIndex = i;
                    bufIndex = sourceBufIndex;

                    auto nodeDelay = getNodeDelay (src.nodeID);

                    if (nodeDelay < maxLatency)
                        sequence.addDelayChannelOp (bufIndex, maxLatency - nodeDelay);

                    break;
                }

                ++i;
            }
        }

        if (reusableInputIndex < 0)
        {
            // can't re-use any of our input chans, so get a new one and copy everything into it..
            bufIndex = getFreeBuffer (audioBuffers);
            jassert (bufIndex != 0);

            audioBuffers.getReference (bufIndex).setAssignedToNonExistentNode();

            auto srcIndex = getBufferContaining (*sources.begin());

            if (srcIndex < 0)
                sequence.addClearChannelOp (bufIndex);  // if not found, this is probably a feedback loop
            else
                sequence.addCopyChannelOp (srcIndex, bufIndex);

            reusableInputIndex = 0;
            auto nodeDelay = getNodeDelay (sources.begin()->nodeID);

            if (nodeDelay < maxLatency)
                sequence.addDelayChannelOp (bufIndex, maxLatency - nodeDelay);
        }

        {
            auto i = 0;
            for (const auto& src : sources)
            {
                if (i != reusableInputIndex)
                {
                    int srcIndex = getBufferContaining (src);

                    if (srcIndex >= 0)
                    {
                        auto nodeDelay = getNodeDelay (src.nodeID);

                        if (nodeDelay < maxLatency)
                        {
                            if (! isBufferNeededLater (reversed, ourRenderingIndex, inputChan, src))
                            {
                                sequence.addDelayChannelOp (srcIndex, maxLatency - nodeDelay);
                            }
                            else // buffer is reused elsewhere, can't be delayed
                            {
                                auto bufferToDelay = getFreeBuffer (audioBuffers);
                                sequence.addCopyChannelOp (srcIndex, bufferToDelay);
                                sequence.addDelayChannelOp (bufferToDelay, maxLatency - nodeDelay);
                                srcIndex = bufferToDelay;
                            }
                        }

                        sequence.addAddChannelOp (srcIndex, bufIndex);
                    }
                }

                ++i;
            }
        }

        return bufIndex;
    }

    template <typename RenderSequence>
    int findBufferForInputMidiChannel (const Connections& c,
                                       const Connections::DestinationsForSources& reversed,
                                       RenderSequence& sequence,
                                       Node& node,
                                       int ourRenderingIndex)
    {
        auto& processor = *node.getProcessor();
        auto sources = c.getSourcesForDestination ({ node.nodeID, midiChannelIndex });

        // No midi inputs..
        if (sources.empty())
        {
            auto midiBufferToUse = getFreeBuffer (midiBuffers); // need to pick a buffer even if the processor doesn't use midi

            if (processor.acceptsMidi() || processor.producesMidi())
                sequence.addClearMidiBufferOp (midiBufferToUse);

            return midiBufferToUse;
        }

        // One midi input..
        if (sources.size() == 1)
        {
            auto src = *sources.begin();
            auto midiBufferToUse = getBufferContaining (src);

            if (midiBufferToUse >= 0)
            {
                if (isBufferNeededLater (reversed, ourRenderingIndex, midiChannelIndex, src))
                {
                    // can't mess up this channel because it's needed later by another node, so we
                    // need to use a copy of it..
                    auto newFreeBuffer = getFreeBuffer (midiBuffers);
                    sequence.addCopyMidiBufferOp (midiBufferToUse, newFreeBuffer);
                    midiBufferToUse = newFreeBuffer;
                }
            }
            else
            {
                // probably a feedback loop, so just use an empty one..
                midiBufferToUse = getFreeBuffer (midiBuffers); // need to pick a buffer even if the processor doesn't use midi
            }

            return midiBufferToUse;
        }

        // Multiple midi inputs..
        int midiBufferToUse = -1;
        int reusableInputIndex = -1;

        {
            auto i = 0;
            for (const auto& src : sources)
            {
                auto sourceBufIndex = getBufferContaining (src);

                if (sourceBufIndex >= 0
                    && ! isBufferNeededLater (reversed, ourRenderingIndex, midiChannelIndex, src))
                {
                    // we've found one of our input buffers that can be re-used..
                    reusableInputIndex = i;
                    midiBufferToUse = sourceBufIndex;
                    break;
                }

                ++i;
            }
        }

        if (reusableInputIndex < 0)
        {
            // can't re-use any of our input buffers, so get a new one and copy everything into it..
            midiBufferToUse = getFreeBuffer (midiBuffers);
            jassert (midiBufferToUse >= 0);

            auto srcIndex = getBufferContaining (*sources.begin());

            if (srcIndex >= 0)
                sequence.addCopyMidiBufferOp (srcIndex, midiBufferToUse);
            else
                sequence.addClearMidiBufferOp (midiBufferToUse);

            reusableInputIndex = 0;
        }

        {
            auto i = 0;
            for (const auto& src : sources)
            {
                if (i != reusableInputIndex)
                {
                    auto srcIndex = getBufferContaining (src);

                    if (srcIndex >= 0)
                        sequence.addAddMidiBufferOp (srcIndex, midiBufferToUse);
                }

                ++i;
            }
        }

        return midiBufferToUse;
    }

    template <typename RenderSequence>
    void createRenderingOpsForNode (const Connections& c,
                                    const Connections::DestinationsForSources& reversed,
                                    RenderSequence& sequence,
                                    Node& node,
                                    const int ourRenderingIndex)
    {
        auto& processor = *node.getProcessor();
        auto numIns  = processor.getTotalNumInputChannels();
        auto numOuts = processor.getTotalNumOutputChannels();
        auto totalChans = jmax (numIns, numOuts);

        Array<int> audioChannelsToUse;
        const auto maxInputLatency = getInputLatencyForNode (c, node.nodeID);

        for (int inputChan = 0; inputChan < numIns; ++inputChan)
        {
            // get a list of all the inputs to this node
            auto index = findBufferForInputAudioChannel (c,
                                                         reversed,
                                                         sequence,
                                                         node,
                                                         inputChan,
                                                         ourRenderingIndex,
                                                         maxInputLatency);
            jassert (index >= 0);

            audioChannelsToUse.add (index);

            if (inputChan < numOuts)
                audioBuffers.getReference (index).channel = { node.nodeID, inputChan };
        }

        for (int outputChan = numIns; outputChan < numOuts; ++outputChan)
        {
            auto index = getFreeBuffer (audioBuffers);
            jassert (index != 0);
            audioChannelsToUse.add (index);

            audioBuffers.getReference (index).channel = { node.nodeID, outputChan };
        }

        auto midiBufferToUse = findBufferForInputMidiChannel (c, reversed, sequence, node, ourRenderingIndex);

        if (processor.producesMidi())
            midiBuffers.getReference (midiBufferToUse).channel = { node.nodeID, midiChannelIndex };

        const auto thisNodeLatency = maxInputLatency + processor.getLatencySamples();
        delays[node.nodeID.uid] = thisNodeLatency;

        if (numOuts == 0)
            totalLatency = jmax (totalLatency, thisNodeLatency);

        sequence.addProcessOp (node, audioChannelsToUse, totalChans, midiBufferToUse);
    }

    //==============================================================================
    static int getFreeBuffer (Array<AssignedBuffer>& buffers)
    {
        for (int i = 1; i < buffers.size(); ++i)
            if (buffers.getReference (i).isFree())
                return i;

        buffers.add (AssignedBuffer::createFree());
        return buffers.size() - 1;
    }

    int getBufferContaining (NodeAndChannel output) const noexcept
    {
        int i = 0;

        for (auto& b : output.isMIDI() ? midiBuffers : audioBuffers)
        {
            if (b.channel == output)
                return i;

            ++i;
        }

        return -1;
    }

    void markAnyUnusedBuffersAsFree (const Connections::DestinationsForSources& c,
                                     Array<AssignedBuffer>& buffers,
                                     const int stepIndex)
    {
        for (auto& b : buffers)
            if (b.isAssigned() && ! isBufferNeededLater (c, stepIndex, -1, b.channel))
                b.setFree();
    }

    bool isBufferNeededLater (const Connections::DestinationsForSources& c,
                              const int stepIndexToSearchFrom,
                              const int inputChannelOfIndexToIgnore,
                              const NodeAndChannel output) const
    {
        if (orderedNodes.size() <= stepIndexToSearchFrom)
            return false;

        if (c.isSourceConnectedToDestinationNodeIgnoringChannel (output,
                                                                 orderedNodes.getUnchecked (stepIndexToSearchFrom)->nodeID,
                                                                 inputChannelOfIndexToIgnore))
        {
            return true;
        }

        return std::any_of (orderedNodes.begin() + stepIndexToSearchFrom + 1, orderedNodes.end(), [&] (const auto* node)
        {
            return c.isSourceConnectedToDestinationNodeIgnoringChannel (output, node->nodeID, -1);
        });
    }

    template <typename RenderSequence>
    RenderSequenceBuilder (const Nodes& n, const Connections& c, RenderSequence& sequence)
        : orderedNodes (createOrderedNodeList (n, c))
    {
        audioBuffers.add (AssignedBuffer::createReadOnlyEmpty()); // first buffer is read-only zeros
        midiBuffers .add (AssignedBuffer::createReadOnlyEmpty());

        const auto reversed = c.getDestinationsForSources();

        for (int i = 0; i < orderedNodes.size(); ++i)
        {
            createRenderingOpsForNode (c, reversed, sequence, *orderedNodes.getUnchecked (i), i);
            markAnyUnusedBuffersAsFree (reversed, audioBuffers, i);
            markAnyUnusedBuffersAsFree (reversed, midiBuffers, i);
        }

        sequence.numBuffersNeeded = audioBuffers.size();
        sequence.numMidiBuffersNeeded = midiBuffers.size();
    }
};

//==============================================================================
/*  A full graph of audio processors, ready to process at a particular sample rate, block size,
    and precision.

    Instances of this class will be created on the main thread, and then passed over to the audio
    thread for processing.
*/
class RenderSequence
{
public:
    using AudioGraphIOProcessor = AudioProcessorGraph::AudioGraphIOProcessor;

    RenderSequence (const PrepareSettings s, const Nodes& n, const Connections& c)
        : RenderSequence (s, s.precision == AudioProcessor::ProcessingPrecision::singlePrecision
                                ? RenderSequenceBuilder::build<float>  (n, c)
                                : RenderSequenceBuilder::build<double> (n, c))
    {
    }

    template <typename FloatType>
    void process (AudioBuffer<FloatType>& audio, MidiBuffer& midi, AudioPlayHead* playHead)
    {
        if (auto* s = std::get_if<GraphRenderSequence<FloatType>> (&sequence.sequence))
            s->perform (audio, midi, playHead);
        else
            jassertfalse; // Not prepared for this audio format!
    }

    int getLatencySamples() const { return sequence.latencySamples; }
    PrepareSettings getSettings() const { return settings; }

private:
    template <typename This, typename Callback>
    static void visitRenderSequence (This& t, Callback&& callback)
    {
        if (auto* sequence = std::get_if<GraphRenderSequence<float>>  (&t.sequence.sequence)) return callback (*sequence);
        if (auto* sequence = std::get_if<GraphRenderSequence<double>> (&t.sequence.sequence)) return callback (*sequence);
        jassertfalse;
    }

    RenderSequence (const PrepareSettings s, SequenceAndLatency&& built)
        : settings (s), sequence (std::move (built))
    {
        visitRenderSequence (*this, [&] (auto& seq) { seq.prepareBuffers (settings.blockSize); });
    }

    PrepareSettings settings;
    SequenceAndLatency sequence;
};

//==============================================================================
/*  Holds information about the properties of a graph node at the point it was prepared.

    If the bus layout or latency of a given node changes, the graph should be rebuilt so
    that channel connections are ordered correctly, and the graph's internal delay lines have
    the correct delay.
*/
class NodeAttributes
{
    auto tie() const { return std::tie (layout, latencySamples); }

public:
    AudioProcessor::BusesLayout layout;
    int latencySamples = 0;

    bool operator== (const NodeAttributes& other) const { return tie() == other.tie(); }
    bool operator!= (const NodeAttributes& other) const { return tie() != other.tie(); }
};

//==============================================================================
/*  Holds information about a particular graph configuration, without sharing ownership of any
    graph nodes. Can be checked for equality with other RenderSequenceSignature instances to see
    whether two graph configurations match.
*/
class RenderSequenceSignature
{
    auto tie() const { return std::tie (settings, connections, nodes); }

public:
    RenderSequenceSignature (const PrepareSettings s, const Nodes& n, const Connections& c)
        : settings (s), connections (c), nodes (getNodeMap (n)) {}

    bool operator== (const RenderSequenceSignature& other) const { return tie() == other.tie(); }
    bool operator!= (const RenderSequenceSignature& other) const { return tie() != other.tie(); }

private:
    using NodeMap = std::map<AudioProcessorGraph::NodeID, NodeAttributes>;

    static NodeMap getNodeMap (const Nodes& n)
    {
        const auto& nodeRefs = n.getNodes();
        NodeMap result;

        for (const auto& node : nodeRefs)
        {
            auto* proc = node->getProcessor();
            result.emplace (node->nodeID,
                            NodeAttributes { proc->getBusesLayout(),
                                             proc->getLatencySamples() });
        }

        return result;
    }

    PrepareSettings settings;
    Connections connections;
    NodeMap nodes;
};

//==============================================================================
/*  Facilitates wait-free render-sequence updates.

    Topology updates always happen on the main thread (or synchronised with the main thread).
    After updating the graph, the 'baked' graph is passed to RenderSequenceExchange::set.
    At the top of the audio callback, RenderSequenceExchange::updateAudioThreadState will
    attempt to install the most-recently-baked graph, if there's one waiting.
*/
class RenderSequenceExchange final : private Timer
{
public:
    RenderSequenceExchange()
    {
        startTimer (500);
    }

    ~RenderSequenceExchange() override
    {
        stopTimer();
    }

    void set (std::unique_ptr<RenderSequence>&& next)
    {
        const SpinLock::ScopedLockType lock (mutex);
        mainThreadState = std::move (next);
        isNew = true;
    }

    /*  Call from the audio thread only. */
    void updateAudioThreadState()
    {
        const SpinLock::ScopedTryLockType lock (mutex);

        if (lock.isLocked() && isNew)
        {
            // Swap pointers rather than assigning to avoid calling delete here
            std::swap (mainThreadState, audioThreadState);
            isNew = false;
        }
    }

    /*  Call from the audio thread only. */
    RenderSequence* getAudioThreadState() const { return audioThreadState.get(); }

private:
    void timerCallback() override
    {
        const SpinLock::ScopedLockType lock (mutex);

        if (! isNew)
            mainThreadState.reset();
    }

    SpinLock mutex;
    std::unique_ptr<RenderSequence> mainThreadState, audioThreadState;
    bool isNew = false;
};

//==============================================================================
AudioProcessorGraph::Connection::Connection (NodeAndChannel src, NodeAndChannel dst) noexcept
    : source (src), destination (dst)
{
}

bool AudioProcessorGraph::Connection::operator== (const Connection& other) const noexcept
{
    return source == other.source && destination == other.destination;
}

bool AudioProcessorGraph::Connection::operator!= (const Connection& c) const noexcept
{
    return ! operator== (c);
}

bool AudioProcessorGraph::Connection::operator< (const Connection& other) const noexcept
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
class AudioProcessorGraph::Pimpl
{
public:
    explicit Pimpl (AudioProcessorGraph& o) : owner (&o) {}

    const auto& getNodes() const { return nodes.getNodes(); }

    void clear (UpdateKind updateKind)
    {
        if (getNodes().isEmpty())
            return;

        nodes = Nodes{};
        connections = Connections{};
        nodeStates.clear();
        topologyChanged (updateKind);
    }

    auto getNodeForId (NodeID nodeID) const
    {
        return nodes.getNodeForId (nodeID);
    }

    Node::Ptr addNode (std::unique_ptr<AudioProcessor> newProcessor,
                       std::optional<NodeID> nodeID,
                       UpdateKind updateKind)
    {
        if (newProcessor.get() == owner)
        {
            jassertfalse;
            return nullptr;
        }

        const auto idToUse = nodeID.value_or (NodeID { lastNodeID.uid + 1 });

        auto added = nodes.addNode (std::move (newProcessor), idToUse);

        if (added == nullptr)
            return nullptr;

        if (lastNodeID < idToUse)
            lastNodeID = idToUse;

        setParentGraph (added->getProcessor());

        topologyChanged (updateKind);
        return added;
    }

    Node::Ptr removeNode (NodeID nodeID, UpdateKind updateKind)
    {
        connections.disconnectNode (nodeID);
        auto result = nodes.removeNode (nodeID);
        nodeStates.removeNode (nodeID);
        topologyChanged (updateKind);
        return result;
    }

    std::vector<Connection> getConnections() const
    {
        return connections.getConnections();
    }

    bool isConnected (const Connection& c) const
    {
        return connections.isConnected (c);
    }

    bool isConnected (NodeID srcID, NodeID destID) const
    {
        return connections.isConnected (srcID, destID);
    }

    bool isAnInputTo (const Node& src, const Node& dst) const
    {
        return isAnInputTo (src.nodeID, dst.nodeID);
    }

    bool isAnInputTo (NodeID src, NodeID dst) const
    {
        return connections.isAnInputTo (src, dst);
    }

    bool canConnect (const Connection& c) const
    {
        return connections.canConnect (nodes, c);
    }

    bool addConnection (const Connection& c, UpdateKind updateKind)
    {
        if (! connections.addConnection (nodes, c))
            return false;

        jassert (isConnected (c));
        topologyChanged (updateKind);
        return true;
    }

    bool removeConnection (const Connection& c, UpdateKind updateKind)
    {
        if (! connections.removeConnection (c))
            return false;

        topologyChanged (updateKind);
        return true;
    }

    bool disconnectNode (NodeID nodeID, UpdateKind updateKind)
    {
        if (! connections.disconnectNode (nodeID))
            return false;

        topologyChanged (updateKind);
        return true;
    }

    bool isConnectionLegal (const Connection& c) const
    {
        return connections.isConnectionLegal (nodes, c);
    }

    bool removeIllegalConnections (UpdateKind updateKind)
    {
        const auto result = connections.removeIllegalConnections (nodes);
        topologyChanged (updateKind);
        return result;
    }

    //==============================================================================
    void prepareToPlay (double sampleRate, int estimatedSamplesPerBlock)
    {
        owner->setRateAndBufferSizeDetails (sampleRate, estimatedSamplesPerBlock);

        PrepareSettings settings;
        settings.precision  = owner->getProcessingPrecision();
        settings.sampleRate = sampleRate;
        settings.blockSize  = estimatedSamplesPerBlock;

        nodeStates.setState (settings);

        topologyChanged (UpdateKind::sync);
    }

    void releaseResources()
    {
        nodeStates.setState (nullopt);
        topologyChanged (UpdateKind::sync);
    }

    void rebuild (UpdateKind updateKind)
    {
        if (updateKind == UpdateKind::none)
            return;

        if (updateKind == UpdateKind::sync && MessageManager::getInstance()->isThisTheMessageThread())
            handleAsyncUpdate();
        else
            updater.triggerAsyncUpdate();
    }

    void reset()
    {
        for (auto* n : getNodes())
            n->getProcessor()->reset();
    }

    void setNonRealtime (bool isProcessingNonRealtime)
    {
        for (auto* n : getNodes())
            n->getProcessor()->setNonRealtime (isProcessingNonRealtime);
    }

    template <typename Value>
    void processBlock (AudioBuffer<Value>& audio, MidiBuffer& midi, AudioPlayHead* playHead)
    {
        renderSequenceExchange.updateAudioThreadState();

        if (renderSequenceExchange.getAudioThreadState() == nullptr && MessageManager::getInstance()->isThisTheMessageThread())
            handleAsyncUpdate();

        if (owner->isNonRealtime())
        {
            while (renderSequenceExchange.getAudioThreadState() == nullptr)
            {
                Thread::sleep (1);
                renderSequenceExchange.updateAudioThreadState();
            }
        }

        auto* state = renderSequenceExchange.getAudioThreadState();

        // Only process if the graph has the correct blockSize, sampleRate etc.
        if (state != nullptr && state->getSettings() == nodeStates.getLastRequestedSettings())
        {
            state->process (audio, midi, playHead);
        }
        else
        {
            audio.clear();
            midi.clear();
        }
    }

    /*  Call from the audio thread only. */
    auto* getAudioThreadState() const { return renderSequenceExchange.getAudioThreadState(); }

private:
    void setParentGraph (AudioProcessor* p) const
    {
        if (auto* ioProc = dynamic_cast<AudioGraphIOProcessor*> (p))
            ioProc->setParentGraph (owner);
    }

    void topologyChanged (UpdateKind updateKind)
    {
        owner->sendChangeMessage();
        rebuild (updateKind);
    }

    void handleAsyncUpdate()
    {
        if (const auto newSettings = nodeStates.applySettings (nodes))
        {
            for (const auto node : nodes.getNodes())
                setParentGraph (node->getProcessor());

            const RenderSequenceSignature newSignature (*newSettings, nodes, connections);

            if (std::exchange (lastBuiltSequence, newSignature) != newSignature)
            {
                auto sequence = std::make_unique<RenderSequence> (*newSettings, nodes, connections);
                owner->setLatencySamples (sequence->getLatencySamples());
                renderSequenceExchange.set (std::move (sequence));
            }
        }
        else
        {
            lastBuiltSequence.reset();
            renderSequenceExchange.set (nullptr);
        }
    }

    AudioProcessorGraph* owner = nullptr;
    Nodes nodes;
    Connections connections;
    NodeStates nodeStates;
    RenderSequenceExchange renderSequenceExchange;
    NodeID lastNodeID;
    std::optional<RenderSequenceSignature> lastBuiltSequence;
    LockingAsyncUpdater updater { [this] { handleAsyncUpdate(); } };
};

//==============================================================================
AudioProcessorGraph::AudioProcessorGraph() : pimpl (std::make_unique<Pimpl> (*this)) {}
AudioProcessorGraph::~AudioProcessorGraph() = default;

const String AudioProcessorGraph::getName() const                   { return "Audio Graph"; }
bool AudioProcessorGraph::supportsDoublePrecisionProcessing() const { return true; }
double AudioProcessorGraph::getTailLengthSeconds() const            { return 0; }
bool AudioProcessorGraph::acceptsMidi() const                       { return true; }
bool AudioProcessorGraph::producesMidi() const                      { return true; }
void AudioProcessorGraph::getStateInformation (MemoryBlock&)        {}
void AudioProcessorGraph::setStateInformation (const void*, int)    {}

void AudioProcessorGraph::processBlock (AudioBuffer<float>&  audio, MidiBuffer& midi)                       { return pimpl->processBlock (audio, midi, getPlayHead()); }
void AudioProcessorGraph::processBlock (AudioBuffer<double>& audio, MidiBuffer& midi)                       { return pimpl->processBlock (audio, midi, getPlayHead()); }
std::vector<AudioProcessorGraph::Connection> AudioProcessorGraph::getConnections() const                    { return pimpl->getConnections(); }
bool AudioProcessorGraph::addConnection (const Connection& c, UpdateKind updateKind)                        { return pimpl->addConnection (c, updateKind); }
bool AudioProcessorGraph::removeConnection (const Connection& c, UpdateKind updateKind)                     { return pimpl->removeConnection (c, updateKind); }
void AudioProcessorGraph::prepareToPlay (double sampleRate, int estimatedSamplesPerBlock)                   { return pimpl->prepareToPlay (sampleRate, estimatedSamplesPerBlock); }
void AudioProcessorGraph::clear (UpdateKind updateKind)                                                     { return pimpl->clear (updateKind); }
const ReferenceCountedArray<AudioProcessorGraph::Node>& AudioProcessorGraph::getNodes() const noexcept      { return pimpl->getNodes(); }
AudioProcessorGraph::Node* AudioProcessorGraph::getNodeForId (NodeID x) const                               { return pimpl->getNodeForId (x).get(); }
bool AudioProcessorGraph::disconnectNode (NodeID nodeID, UpdateKind updateKind)                             { return pimpl->disconnectNode (nodeID, updateKind); }
void AudioProcessorGraph::releaseResources()                                                                { return pimpl->releaseResources(); }
bool AudioProcessorGraph::removeIllegalConnections (UpdateKind updateKind)                                  { return pimpl->removeIllegalConnections (updateKind); }
void AudioProcessorGraph::rebuild()                                                                         { return pimpl->rebuild (UpdateKind::sync); }
void AudioProcessorGraph::reset()                                                                           { return pimpl->reset(); }
bool AudioProcessorGraph::canConnect (const Connection& c) const                                            { return pimpl->canConnect (c); }
bool AudioProcessorGraph::isConnected (const Connection& c) const noexcept                                  { return pimpl->isConnected (c); }
bool AudioProcessorGraph::isConnected (NodeID a, NodeID b) const noexcept                                   { return pimpl->isConnected (a, b); }
bool AudioProcessorGraph::isConnectionLegal (const Connection& c) const                                     { return pimpl->isConnectionLegal (c); }
bool AudioProcessorGraph::isAnInputTo (const Node& source, const Node& destination) const noexcept          { return pimpl->isAnInputTo (source, destination); }
bool AudioProcessorGraph::isAnInputTo (NodeID source, NodeID destination) const noexcept                    { return pimpl->isAnInputTo (source, destination); }

AudioProcessorGraph::Node::Ptr AudioProcessorGraph::addNode (std::unique_ptr<AudioProcessor> newProcessor,
                                                             std::optional<NodeID> nodeId,
                                                             UpdateKind updateKind)
{
    return pimpl->addNode (std::move (newProcessor), nodeId, updateKind);
}

void AudioProcessorGraph::setNonRealtime (bool isProcessingNonRealtime) noexcept
{
    AudioProcessor::setNonRealtime (isProcessingNonRealtime);
    pimpl->setNonRealtime (isProcessingNonRealtime);
}

AudioProcessorGraph::Node::Ptr AudioProcessorGraph::removeNode (NodeID nodeID, UpdateKind updateKind)
{
    return pimpl->removeNode (nodeID, updateKind);
}

AudioProcessorGraph::Node::Ptr AudioProcessorGraph::removeNode (Node* node, UpdateKind updateKind)
{
    if (node != nullptr)
        return removeNode (node->nodeID, updateKind);

    jassertfalse;
    return {};
}

//==============================================================================
AudioProcessorGraph::AudioGraphIOProcessor::AudioGraphIOProcessor (const IODeviceType deviceType)
    : type (deviceType)
{
}

AudioProcessorGraph::AudioGraphIOProcessor::~AudioGraphIOProcessor() = default;

const String AudioProcessorGraph::AudioGraphIOProcessor::getName() const
{
    switch (type)
    {
        case audioOutputNode:   return "Audio Output";
        case audioInputNode:    return "Audio Input";
        case midiOutputNode:    return "MIDI Output";
        case midiInputNode:     return "MIDI Input";
        default:                break;
    }

    return {};
}

void AudioProcessorGraph::AudioGraphIOProcessor::fillInPluginDescription (PluginDescription& d) const
{
    d.name = getName();
    d.category = "I/O devices";
    d.pluginFormatName = "Internal";
    d.manufacturerName = "JUCE";
    d.version = "1.0";
    d.isInstrument = false;

    d.deprecatedUid = d.uniqueId = d.name.hashCode();

    d.numInputChannels = getTotalNumInputChannels();

    if (type == audioOutputNode && graph != nullptr)
        d.numInputChannels = graph->getTotalNumInputChannels();

    d.numOutputChannels = getTotalNumOutputChannels();

    if (type == audioInputNode && graph != nullptr)
        d.numOutputChannels = graph->getTotalNumOutputChannels();
}

void AudioProcessorGraph::AudioGraphIOProcessor::prepareToPlay (double, int)
{
    jassert (graph != nullptr);
}

void AudioProcessorGraph::AudioGraphIOProcessor::releaseResources()
{
}

bool AudioProcessorGraph::AudioGraphIOProcessor::supportsDoublePrecisionProcessing() const
{
    return true;
}

void AudioProcessorGraph::AudioGraphIOProcessor::processBlock (AudioBuffer<float>&, MidiBuffer&)
{
    // The graph should never call this!
    jassertfalse;
}

void AudioProcessorGraph::AudioGraphIOProcessor::processBlock (AudioBuffer<double>&, MidiBuffer&)
{
    // The graph should never call this!
    jassertfalse;
}

double AudioProcessorGraph::AudioGraphIOProcessor::getTailLengthSeconds() const
{
    return 0;
}

bool AudioProcessorGraph::AudioGraphIOProcessor::acceptsMidi() const
{
    return type == midiOutputNode;
}

bool AudioProcessorGraph::AudioGraphIOProcessor::producesMidi() const
{
    return type == midiInputNode;
}

bool AudioProcessorGraph::AudioGraphIOProcessor::isInput() const noexcept           { return type == audioInputNode  || type == midiInputNode; }
bool AudioProcessorGraph::AudioGraphIOProcessor::isOutput() const noexcept          { return type == audioOutputNode || type == midiOutputNode; }

bool AudioProcessorGraph::AudioGraphIOProcessor::hasEditor() const                  { return false; }
AudioProcessorEditor* AudioProcessorGraph::AudioGraphIOProcessor::createEditor()    { return nullptr; }

int AudioProcessorGraph::AudioGraphIOProcessor::getNumPrograms()                    { return 0; }
int AudioProcessorGraph::AudioGraphIOProcessor::getCurrentProgram()                 { return 0; }
void AudioProcessorGraph::AudioGraphIOProcessor::setCurrentProgram (int)            { }

const String AudioProcessorGraph::AudioGraphIOProcessor::getProgramName (int)       { return {}; }
void AudioProcessorGraph::AudioGraphIOProcessor::changeProgramName (int, const String&) {}

void AudioProcessorGraph::AudioGraphIOProcessor::getStateInformation (MemoryBlock&)     {}
void AudioProcessorGraph::AudioGraphIOProcessor::setStateInformation (const void*, int) {}

void AudioProcessorGraph::AudioGraphIOProcessor::setParentGraph (AudioProcessorGraph* const newGraph)
{
    graph = newGraph;

    if (graph == nullptr)
        return;

    setPlayConfigDetails (type == audioOutputNode ? newGraph->getTotalNumOutputChannels() : 0,
                          type == audioInputNode  ? newGraph->getTotalNumInputChannels()  : 0,
                          getSampleRate(),
                          getBlockSize());

    updateHostDisplay();
}

//==============================================================================
//==============================================================================
#if JUCE_UNIT_TESTS

class AudioProcessorGraphTests final : public UnitTest
{
public:
    AudioProcessorGraphTests()
        : UnitTest ("AudioProcessorGraph", UnitTestCategories::audioProcessors) {}

    void runTest() override
    {
        const auto midiChannel = AudioProcessorGraph::midiChannelIndex;

        beginTest ("isConnected returns true when two nodes are connected");
        {
            AudioProcessorGraph graph;
            const auto nodeA = graph.addNode (BasicProcessor::make ({}, MidiIn::no, MidiOut::yes))->nodeID;
            const auto nodeB = graph.addNode (BasicProcessor::make ({}, MidiIn::yes, MidiOut::no))->nodeID;

            expect (graph.canConnect ({ { nodeA, midiChannel }, { nodeB, midiChannel } }));
            expect (! graph.canConnect ({ { nodeB, midiChannel }, { nodeA, midiChannel } }));
            expect (! graph.canConnect ({ { nodeA, midiChannel }, { nodeA, midiChannel } }));
            expect (! graph.canConnect ({ { nodeB, midiChannel }, { nodeB, midiChannel } }));

            expect (graph.getConnections().empty());
            expect (! graph.isConnected ({ { nodeA, midiChannel }, { nodeB, midiChannel } }));
            expect (! graph.isConnected (nodeA, nodeB));

            expect (graph.addConnection ({ { nodeA, midiChannel }, { nodeB, midiChannel } }));

            expect (graph.getConnections().size() == 1);
            expect (graph.isConnected ({ { nodeA, midiChannel }, { nodeB, midiChannel } }));
            expect (graph.isConnected (nodeA, nodeB));

            expect (graph.disconnectNode (nodeA));

            expect (graph.getConnections().empty());
            expect (! graph.isConnected ({ { nodeA, midiChannel }, { nodeB, midiChannel } }));
            expect (! graph.isConnected (nodeA, nodeB));
        }

        beginTest ("graph lookups work with a large number of connections");
        {
            AudioProcessorGraph graph;

            std::vector<AudioProcessorGraph::NodeID> nodeIDs;

            constexpr auto numNodes = 100;

            for (auto i = 0; i < numNodes; ++i)
            {
                nodeIDs.push_back (graph.addNode (BasicProcessor::make (BasicProcessor::getStereoProperties(),
                                                                        MidiIn::yes,
                                                                        MidiOut::yes))->nodeID);
            }

            for (auto it = nodeIDs.begin(); it != std::prev (nodeIDs.end()); ++it)
            {
                expect (graph.addConnection ({ { it[0], 0 }, { it[1], 0 } }));
                expect (graph.addConnection ({ { it[0], 1 }, { it[1], 1 } }));
            }

            // Check whether isConnected reports correct results when called
            // with both connections and nodes
            for (auto it = nodeIDs.begin(); it != std::prev (nodeIDs.end()); ++it)
            {
                expect (graph.isConnected ({ { it[0], 0 }, { it[1], 0 } }));
                expect (graph.isConnected ({ { it[0], 1 }, { it[1], 1 } }));
                expect (graph.isConnected (it[0], it[1]));
            }

            const auto& nodes = graph.getNodes();

            expect (! graph.isAnInputTo (*nodes[0], *nodes[0]));

            // Check whether isAnInputTo behaves correctly for a non-cyclic graph
            for (auto it = std::next (nodes.begin()); it != std::prev (nodes.end()); ++it)
            {
                expect (! graph.isAnInputTo (**it, **it));

                expect (graph.isAnInputTo (*nodes[0], **it));
                expect (! graph.isAnInputTo (**it, *nodes[0]));

                expect (graph.isAnInputTo (**it, *nodes[nodes.size() - 1]));
                expect (! graph.isAnInputTo (*nodes[nodes.size() - 1], **it));
            }

            // Make the graph cyclic
            graph.addConnection ({ { nodeIDs.back(), 0 }, { nodeIDs.front(), 0 } });
            graph.addConnection ({ { nodeIDs.back(), 1 }, { nodeIDs.front(), 1 } });

            // Check whether isAnInputTo behaves correctly for a cyclic graph
            for (const auto* node : graph.getNodes())
            {
                expect (graph.isAnInputTo (*node, *node));

                expect (graph.isAnInputTo (*nodes[0], *node));
                expect (graph.isAnInputTo (*node, *nodes[0]));

                expect (graph.isAnInputTo (*node, *nodes[nodes.size() - 1]));
                expect (graph.isAnInputTo (*nodes[nodes.size() - 1], *node));
            }
        }

        beginTest ("rebuilding the graph recalculates overall latency");
        {
            AudioProcessorGraph graph;

            const auto nodeA = graph.addNode (BasicProcessor::make (BasicProcessor::getStereoProperties(), MidiIn::no, MidiOut::no))->nodeID;
            const auto nodeB = graph.addNode (BasicProcessor::make (BasicProcessor::getStereoProperties(), MidiIn::no, MidiOut::no))->nodeID;
            const auto final = graph.addNode (BasicProcessor::make (BasicProcessor::getInputOnlyProperties(), MidiIn::no, MidiOut::no))->nodeID;

            expect (graph.addConnection ({ { nodeA, 0 }, { nodeB, 0 } }));
            expect (graph.addConnection ({ { nodeA, 1 }, { nodeB, 1 } }));
            expect (graph.addConnection ({ { nodeB, 0 }, { final, 0 } }));
            expect (graph.addConnection ({ { nodeB, 1 }, { final, 1 } }));

            expect (graph.getLatencySamples() == 0);

            // Graph isn't built, latency is 0 if prepareToPlay hasn't been called yet
            const auto nodeALatency = 100;
            graph.getNodeForId (nodeA)->getProcessor()->setLatencySamples (nodeALatency);
            graph.rebuild();
            expect (graph.getLatencySamples() == 0);

            graph.prepareToPlay (44100, 512);

            expect (graph.getLatencySamples() == nodeALatency);

            const auto nodeBLatency = 200;
            graph.getNodeForId (nodeB)->getProcessor()->setLatencySamples (nodeBLatency);
            graph.rebuild();
            expect (graph.getLatencySamples() == nodeALatency + nodeBLatency);

            const auto finalLatency = 300;
            graph.getNodeForId (final)->getProcessor()->setLatencySamples (finalLatency);
            graph.rebuild();
            expect (graph.getLatencySamples() == nodeALatency + nodeBLatency + finalLatency);
        }

        beginTest ("large render sequence can be built");
        {
            AudioProcessorGraph graph;

            std::vector<AudioProcessorGraph::NodeID> nodeIDs;

            constexpr auto numNodes = 1000;
            constexpr auto numChannels = 100;

            for (auto i = 0; i < numNodes; ++i)
            {
                nodeIDs.push_back (graph.addNode (BasicProcessor::make (BasicProcessor::getMultichannelProperties (numChannels),
                                                                        MidiIn::yes,
                                                                        MidiOut::yes))->nodeID);
            }

            for (auto it = nodeIDs.begin(); it != std::prev (nodeIDs.end()); ++it)
                for (auto channel = 0; channel < numChannels; ++channel)
                    expect (graph.addConnection ({ { it[0], channel }, { it[1], channel } }));

            const auto b = std::chrono::steady_clock::now();
            graph.prepareToPlay (44100.0, 512);
            const auto e = std::chrono::steady_clock::now();
            const auto duration = std::chrono::duration_cast<std::chrono::milliseconds> (e - b).count();

            // No test here, but older versions of the graph would take forever to complete building
            // this graph, so we just want to make sure that we finish the test without timing out.
            logMessage ("render sequence built in " + String (duration) + " ms");
        }
    }

private:
    enum class MidiIn  { no, yes };
    enum class MidiOut { no, yes };

    class BasicProcessor final : public AudioProcessor
    {
    public:
        explicit BasicProcessor (const AudioProcessor::BusesProperties& layout, MidiIn mIn, MidiOut mOut)
            : AudioProcessor (layout), midiIn (mIn), midiOut (mOut) {}

        const String getName() const override                         { return "Basic Processor"; }
        double getTailLengthSeconds() const override                  { return {}; }
        bool acceptsMidi() const override                             { return midiIn  == MidiIn ::yes; }
        bool producesMidi() const override                            { return midiOut == MidiOut::yes; }
        AudioProcessorEditor* createEditor() override                 { return {}; }
        bool hasEditor() const override                               { return {}; }
        int getNumPrograms() override                                 { return 1; }
        int getCurrentProgram() override                              { return {}; }
        void setCurrentProgram (int) override                         {}
        const String getProgramName (int) override                    { return {}; }
        void changeProgramName (int, const String&) override          {}
        void getStateInformation (juce::MemoryBlock&) override        {}
        void setStateInformation (const void*, int) override          {}
        void prepareToPlay (double, int) override                     {}
        void releaseResources() override                              {}
        void processBlock (AudioBuffer<float>&, MidiBuffer&) override {}
        bool supportsDoublePrecisionProcessing() const override       { return true; }
        bool isMidiEffect() const override                            { return {}; }
        void reset() override                                         {}
        void setNonRealtime (bool) noexcept override                  {}

        using AudioProcessor::processBlock;

        static std::unique_ptr<AudioProcessor> make (const BusesProperties& layout,
                                                     MidiIn midiIn,
                                                     MidiOut midiOut)
        {
            return std::make_unique<BasicProcessor> (layout, midiIn, midiOut);
        }

        static BusesProperties getInputOnlyProperties()
        {
            return BusesProperties().withInput  ("in", AudioChannelSet::stereo());
        }

        static BusesProperties getStereoProperties()
        {
            return BusesProperties().withInput  ("in",  AudioChannelSet::stereo())
                                    .withOutput ("out", AudioChannelSet::stereo());
        }

        static BusesProperties getMultichannelProperties (int numChannels)
        {
            return BusesProperties().withInput  ("in",  AudioChannelSet::discreteChannels (numChannels))
                                    .withOutput ("out", AudioChannelSet::discreteChannels (numChannels));
        }

    private:
        MidiIn midiIn;
        MidiOut midiOut;
    };
};

static AudioProcessorGraphTests audioProcessorGraphTests;

#endif

} // namespace juce
