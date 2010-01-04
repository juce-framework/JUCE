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

#include "../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_AudioProcessorGraph.h"
#include "../../events/juce_MessageManager.h"


const int AudioProcessorGraph::midiChannelIndex = 0x1000;

//==============================================================================
AudioProcessorGraph::Node::Node (const uint32 id_, AudioProcessor* const processor_)
    : id (id_),
      processor (processor_),
      isPrepared (false)
{
    jassert (processor_ != 0);
}

AudioProcessorGraph::Node::~Node()
{
    delete processor;
}

void AudioProcessorGraph::Node::prepare (const double sampleRate, const int blockSize,
                                         AudioProcessorGraph* const graph)
{
    if (! isPrepared)
    {
        isPrepared = true;

        AudioProcessorGraph::AudioGraphIOProcessor* const ioProc
            = dynamic_cast <AudioProcessorGraph::AudioGraphIOProcessor*> (processor);

        if (ioProc != 0)
            ioProc->setParentGraph (graph);

        processor->setPlayConfigDetails (processor->getNumInputChannels(),
                                         processor->getNumOutputChannels(),
                                         sampleRate, blockSize);

        processor->prepareToPlay (sampleRate, blockSize);
    }
}

void AudioProcessorGraph::Node::unprepare()
{
    if (isPrepared)
    {
        isPrepared = false;
        processor->releaseResources();
    }
}

//==============================================================================
AudioProcessorGraph::AudioProcessorGraph()
    : lastNodeId (0),
      renderingBuffers (1, 1),
      currentAudioOutputBuffer (1, 1)
{
}

AudioProcessorGraph::~AudioProcessorGraph()
{
    clearRenderingSequence();
    clear();
}

const String AudioProcessorGraph::getName() const
{
    return "Audio Graph";
}

//==============================================================================
void AudioProcessorGraph::clear()
{
    nodes.clear();
    connections.clear();
    triggerAsyncUpdate();
}

AudioProcessorGraph::Node* AudioProcessorGraph::getNodeForId (const uint32 nodeId) const
{
    for (int i = nodes.size(); --i >= 0;)
        if (nodes.getUnchecked(i)->id == nodeId)
            return nodes.getUnchecked(i);

    return 0;
}

AudioProcessorGraph::Node* AudioProcessorGraph::addNode (AudioProcessor* const newProcessor,
                                                         uint32 nodeId)
{
    if (newProcessor == 0)
    {
        jassertfalse
        return 0;
    }

    if (nodeId == 0)
    {
        nodeId = ++lastNodeId;
    }
    else
    {
        // you can't add a node with an id that already exists in the graph..
        jassert (getNodeForId (nodeId) == 0);
        removeNode (nodeId);
    }

    lastNodeId = nodeId;

    Node* const n = new Node (nodeId, newProcessor);
    nodes.add (n);
    triggerAsyncUpdate();

    AudioProcessorGraph::AudioGraphIOProcessor* const ioProc
        = dynamic_cast <AudioProcessorGraph::AudioGraphIOProcessor*> (n->processor);

    if (ioProc != 0)
        ioProc->setParentGraph (this);

    return n;
}

bool AudioProcessorGraph::removeNode (const uint32 nodeId)
{
    disconnectNode (nodeId);

    for (int i = nodes.size(); --i >= 0;)
    {
        if (nodes.getUnchecked(i)->id == nodeId)
        {
            AudioProcessorGraph::AudioGraphIOProcessor* const ioProc
                = dynamic_cast <AudioProcessorGraph::AudioGraphIOProcessor*> (nodes.getUnchecked(i)->processor);

            if (ioProc != 0)
                ioProc->setParentGraph (0);

            nodes.remove (i);
            triggerAsyncUpdate();

            return true;
        }
    }

    return false;
}

//==============================================================================
const AudioProcessorGraph::Connection* AudioProcessorGraph::getConnectionBetween (const uint32 sourceNodeId,
                                                                                  const int sourceChannelIndex,
                                                                                  const uint32 destNodeId,
                                                                                  const int destChannelIndex) const
{
    for (int i = connections.size(); --i >= 0;)
    {
        const Connection* const c = connections.getUnchecked(i);

        if (c->sourceNodeId == sourceNodeId
             && c->destNodeId == destNodeId
             && c->sourceChannelIndex == sourceChannelIndex
             && c->destChannelIndex == destChannelIndex)
        {
            return c;
        }
    }

    return 0;
}

bool AudioProcessorGraph::isConnected (const uint32 possibleSourceNodeId,
                                       const uint32 possibleDestNodeId) const
{
    for (int i = connections.size(); --i >= 0;)
    {
        const Connection* const c = connections.getUnchecked(i);

        if (c->sourceNodeId == possibleSourceNodeId
             && c->destNodeId == possibleDestNodeId)
        {
            return true;
        }
    }

    return false;
}

bool AudioProcessorGraph::canConnect (const uint32 sourceNodeId,
                                      const int sourceChannelIndex,
                                      const uint32 destNodeId,
                                      const int destChannelIndex) const
{
    if (sourceChannelIndex < 0
         || destChannelIndex < 0
         || sourceNodeId == destNodeId
         || (destChannelIndex == midiChannelIndex) != (sourceChannelIndex == midiChannelIndex))
        return false;

    const Node* const source = getNodeForId (sourceNodeId);

    if (source == 0
         || (sourceChannelIndex != midiChannelIndex && sourceChannelIndex >= source->processor->getNumOutputChannels())
         || (sourceChannelIndex == midiChannelIndex && ! source->processor->producesMidi()))
        return false;

    const Node* const dest = getNodeForId (destNodeId);

    if (dest == 0
         || (destChannelIndex != midiChannelIndex && destChannelIndex >= dest->processor->getNumInputChannels())
         || (destChannelIndex == midiChannelIndex && ! dest->processor->acceptsMidi()))
        return false;

    return getConnectionBetween (sourceNodeId, sourceChannelIndex,
                                 destNodeId, destChannelIndex) == 0;
}

bool AudioProcessorGraph::addConnection (const uint32 sourceNodeId,
                                         const int sourceChannelIndex,
                                         const uint32 destNodeId,
                                         const int destChannelIndex)
{
    if (! canConnect (sourceNodeId, sourceChannelIndex, destNodeId, destChannelIndex))
        return false;

    Connection* const c = new Connection();
    c->sourceNodeId = sourceNodeId;
    c->sourceChannelIndex = sourceChannelIndex;
    c->destNodeId = destNodeId;
    c->destChannelIndex = destChannelIndex;

    connections.add (c);
    triggerAsyncUpdate();

    return true;
}

void AudioProcessorGraph::removeConnection (const int index)
{
    connections.remove (index);
    triggerAsyncUpdate();
}

bool AudioProcessorGraph::removeConnection (const uint32 sourceNodeId, const int sourceChannelIndex,
                                            const uint32 destNodeId, const int destChannelIndex)
{
    bool doneAnything = false;

    for (int i = connections.size(); --i >= 0;)
    {
        const Connection* const c = connections.getUnchecked(i);

        if (c->sourceNodeId == sourceNodeId
             && c->destNodeId == destNodeId
             && c->sourceChannelIndex == sourceChannelIndex
             && c->destChannelIndex == destChannelIndex)
        {
            removeConnection (i);
            doneAnything = true;
            triggerAsyncUpdate();
        }
    }

    return doneAnything;
}

bool AudioProcessorGraph::disconnectNode (const uint32 nodeId)
{
    bool doneAnything = false;

    for (int i = connections.size(); --i >= 0;)
    {
        const Connection* const c = connections.getUnchecked(i);

        if (c->sourceNodeId == nodeId || c->destNodeId == nodeId)
        {
            removeConnection (i);
            doneAnything = true;
            triggerAsyncUpdate();
        }
    }

    return doneAnything;
}

bool AudioProcessorGraph::removeIllegalConnections()
{
    bool doneAnything = false;

    for (int i = connections.size(); --i >= 0;)
    {
        const Connection* const c = connections.getUnchecked(i);

        const Node* const source = getNodeForId (c->sourceNodeId);
        const Node* const dest = getNodeForId (c->destNodeId);

        if (source == 0 || dest == 0
             || (c->sourceChannelIndex != midiChannelIndex
                  && (((unsigned int) c->sourceChannelIndex) >= (unsigned int) source->processor->getNumOutputChannels()))
             || (c->sourceChannelIndex == midiChannelIndex
                  && ! source->processor->producesMidi())
             || (c->destChannelIndex != midiChannelIndex
                   && (((unsigned int) c->destChannelIndex) >= (unsigned int) dest->processor->getNumInputChannels()))
             || (c->destChannelIndex == midiChannelIndex
                   && ! dest->processor->acceptsMidi()))
        {
            removeConnection (i);
            doneAnything = true;
            triggerAsyncUpdate();
        }
    }

    return doneAnything;
}

//==============================================================================
namespace GraphRenderingOps
{

//==============================================================================
class AudioGraphRenderingOp
{
public:
    AudioGraphRenderingOp() {}
    virtual ~AudioGraphRenderingOp()  {}

    virtual void perform (AudioSampleBuffer& sharedBufferChans,
                          const OwnedArray <MidiBuffer>& sharedMidiBuffers,
                          const int numSamples) = 0;

    juce_UseDebuggingNewOperator
};

//==============================================================================
class ClearChannelOp : public AudioGraphRenderingOp
{
public:
    ClearChannelOp (const int channelNum_)
        : channelNum (channelNum_)
    {}

    ~ClearChannelOp()        {}

    void perform (AudioSampleBuffer& sharedBufferChans, const OwnedArray <MidiBuffer>&, const int numSamples)
    {
        sharedBufferChans.clear (channelNum, 0, numSamples);
    }

private:
    const int channelNum;

    ClearChannelOp (const ClearChannelOp&);
    const ClearChannelOp& operator= (const ClearChannelOp&);
};

//==============================================================================
class CopyChannelOp : public AudioGraphRenderingOp
{
public:
    CopyChannelOp (const int srcChannelNum_, const int dstChannelNum_)
        : srcChannelNum (srcChannelNum_),
          dstChannelNum (dstChannelNum_)
    {}

    ~CopyChannelOp()        {}

    void perform (AudioSampleBuffer& sharedBufferChans, const OwnedArray <MidiBuffer>&, const int numSamples)
    {
        sharedBufferChans.copyFrom (dstChannelNum, 0, sharedBufferChans, srcChannelNum, 0, numSamples);
    }

private:
    const int srcChannelNum, dstChannelNum;

    CopyChannelOp (const CopyChannelOp&);
    const CopyChannelOp& operator= (const CopyChannelOp&);
};

//==============================================================================
class AddChannelOp : public AudioGraphRenderingOp
{
public:
    AddChannelOp (const int srcChannelNum_, const int dstChannelNum_)
        : srcChannelNum (srcChannelNum_),
          dstChannelNum (dstChannelNum_)
    {}

    ~AddChannelOp()        {}

    void perform (AudioSampleBuffer& sharedBufferChans, const OwnedArray <MidiBuffer>&, const int numSamples)
    {
        sharedBufferChans.addFrom (dstChannelNum, 0, sharedBufferChans, srcChannelNum, 0, numSamples);
    }

private:
    const int srcChannelNum, dstChannelNum;

    AddChannelOp (const AddChannelOp&);
    const AddChannelOp& operator= (const AddChannelOp&);
};

//==============================================================================
class ClearMidiBufferOp : public AudioGraphRenderingOp
{
public:
    ClearMidiBufferOp (const int bufferNum_)
        : bufferNum (bufferNum_)
    {}

    ~ClearMidiBufferOp()        {}

    void perform (AudioSampleBuffer&, const OwnedArray <MidiBuffer>& sharedMidiBuffers, const int)
    {
        sharedMidiBuffers.getUnchecked (bufferNum)->clear();
    }

private:
    const int bufferNum;

    ClearMidiBufferOp (const ClearMidiBufferOp&);
    const ClearMidiBufferOp& operator= (const ClearMidiBufferOp&);
};

//==============================================================================
class CopyMidiBufferOp : public AudioGraphRenderingOp
{
public:
    CopyMidiBufferOp (const int srcBufferNum_, const int dstBufferNum_)
        : srcBufferNum (srcBufferNum_),
          dstBufferNum (dstBufferNum_)
    {}

    ~CopyMidiBufferOp()        {}

    void perform (AudioSampleBuffer&, const OwnedArray <MidiBuffer>& sharedMidiBuffers, const int)
    {
        *sharedMidiBuffers.getUnchecked (dstBufferNum) = *sharedMidiBuffers.getUnchecked (srcBufferNum);
    }

private:
    const int srcBufferNum, dstBufferNum;

    CopyMidiBufferOp (const CopyMidiBufferOp&);
    const CopyMidiBufferOp& operator= (const CopyMidiBufferOp&);
};

//==============================================================================
class AddMidiBufferOp : public AudioGraphRenderingOp
{
public:
    AddMidiBufferOp (const int srcBufferNum_, const int dstBufferNum_)
        : srcBufferNum (srcBufferNum_),
          dstBufferNum (dstBufferNum_)
    {}

    ~AddMidiBufferOp()        {}

    void perform (AudioSampleBuffer&, const OwnedArray <MidiBuffer>& sharedMidiBuffers, const int numSamples)
    {
        sharedMidiBuffers.getUnchecked (dstBufferNum)
            ->addEvents (*sharedMidiBuffers.getUnchecked (srcBufferNum), 0, numSamples, 0);
    }

private:
    const int srcBufferNum, dstBufferNum;

    AddMidiBufferOp (const AddMidiBufferOp&);
    const AddMidiBufferOp& operator= (const AddMidiBufferOp&);
};

//==============================================================================
class ProcessBufferOp : public AudioGraphRenderingOp
{
public:
    ProcessBufferOp (const AudioProcessorGraph::Node::Ptr& node_,
                     const Array <int>& audioChannelsToUse_,
                     const int totalChans_,
                     const int midiBufferToUse_)
        : node (node_),
          processor (node_->processor),
          audioChannelsToUse (audioChannelsToUse_),
          totalChans (jmax (1, totalChans_)),
          midiBufferToUse (midiBufferToUse_)
    {
        channels.calloc (totalChans);

        while (audioChannelsToUse.size() < totalChans)
            audioChannelsToUse.add (0);
    }

    ~ProcessBufferOp()
    {
    }

    void perform (AudioSampleBuffer& sharedBufferChans, const OwnedArray <MidiBuffer>& sharedMidiBuffers, const int numSamples)
    {
        for (int i = totalChans; --i >= 0;)
            channels[i] = sharedBufferChans.getSampleData (audioChannelsToUse.getUnchecked (i), 0);

        AudioSampleBuffer buffer (channels, totalChans, numSamples);

        processor->processBlock (buffer, *sharedMidiBuffers.getUnchecked (midiBufferToUse));
    }

    const AudioProcessorGraph::Node::Ptr node;
    AudioProcessor* const processor;

private:
    Array <int> audioChannelsToUse;
    HeapBlock <float*> channels;
    int totalChans;
    int midiBufferToUse;

    ProcessBufferOp (const ProcessBufferOp&);
    const ProcessBufferOp& operator= (const ProcessBufferOp&);
};

//==============================================================================
/** Used to calculate the correct sequence of rendering ops needed, based on
    the best re-use of shared buffers at each stage.
*/
class RenderingOpSequenceCalculator
{
public:
    //==============================================================================
    RenderingOpSequenceCalculator (AudioProcessorGraph& graph_,
                                   const VoidArray& orderedNodes_,
                                   VoidArray& renderingOps)
        : graph (graph_),
          orderedNodes (orderedNodes_)
    {
        nodeIds.add (-2); // first buffer is read-only zeros
        channels.add (0);

        midiNodeIds.add (-2);

        for (int i = 0; i < orderedNodes.size(); ++i)
        {
            createRenderingOpsForNode ((AudioProcessorGraph::Node*) orderedNodes.getUnchecked(i),
                                       renderingOps, i);

            markAnyUnusedBuffersAsFree (i);
        }
    }

    int getNumBuffersNeeded() const         { return nodeIds.size(); }

    int getNumMidiBuffersNeeded() const     { return midiNodeIds.size(); }


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    AudioProcessorGraph& graph;
    const VoidArray& orderedNodes;
    Array <int> nodeIds, channels, midiNodeIds;

    //==============================================================================
    void createRenderingOpsForNode (AudioProcessorGraph::Node* const node,
                                    VoidArray& renderingOps,
                                    const int ourRenderingIndex)
    {
        const int numIns = node->processor->getNumInputChannels();
        const int numOuts = node->processor->getNumOutputChannels();
        const int totalChans = jmax (numIns, numOuts);

        Array <int> audioChannelsToUse;
        int midiBufferToUse = -1;

        for (int inputChan = 0; inputChan < numIns; ++inputChan)
        {
            // get a list of all the inputs to this node
            Array <int> sourceNodes, sourceOutputChans;

            for (int i = graph.getNumConnections(); --i >= 0;)
            {
                const AudioProcessorGraph::Connection* const c = graph.getConnection (i);

                if (c->destNodeId == node->id && c->destChannelIndex == inputChan)
                {
                    sourceNodes.add (c->sourceNodeId);
                    sourceOutputChans.add (c->sourceChannelIndex);
                }
            }

            int bufIndex = -1;

            if (sourceNodes.size() == 0)
            {
                // unconnected input channel

                if (inputChan >= numOuts)
                {
                    bufIndex = getReadOnlyEmptyBuffer();
                    jassert (bufIndex >= 0);
                }
                else
                {
                    bufIndex = getFreeBuffer (false);
                    renderingOps.add (new ClearChannelOp (bufIndex));
                }
            }
            else if (sourceNodes.size() == 1)
            {
                // channel with a straightforward single input..
                const int srcNode = sourceNodes.getUnchecked(0);
                const int srcChan = sourceOutputChans.getUnchecked(0);

                bufIndex = getBufferContaining (srcNode, srcChan);

                if (bufIndex < 0)
                {
                    // if not found, this is probably a feedback loop
                    bufIndex = getReadOnlyEmptyBuffer();
                    jassert (bufIndex >= 0);
                }

                if (inputChan < numOuts
                     && isBufferNeededLater (ourRenderingIndex,
                                             inputChan,
                                             srcNode, srcChan))
                {
                    // can't mess up this channel because it's needed later by another node, so we
                    // need to use a copy of it..
                    const int newFreeBuffer = getFreeBuffer (false);

                    renderingOps.add (new CopyChannelOp (bufIndex, newFreeBuffer));

                    bufIndex = newFreeBuffer;
                }
            }
            else
            {
                // channel with a mix of several inputs..

                // try to find a re-usable channel from our inputs..
                int reusableInputIndex = -1;

                for (int i = 0; i < sourceNodes.size(); ++i)
                {
                    const int sourceBufIndex = getBufferContaining (sourceNodes.getUnchecked(i),
                                                                    sourceOutputChans.getUnchecked(i));

                    if (sourceBufIndex >= 0
                        && ! isBufferNeededLater (ourRenderingIndex,
                                                  inputChan,
                                                  sourceNodes.getUnchecked(i),
                                                  sourceOutputChans.getUnchecked(i)))
                    {
                        // we've found one of our input chans that can be re-used..
                        reusableInputIndex = i;
                        bufIndex = sourceBufIndex;
                        break;
                    }
                }

                if (reusableInputIndex < 0)
                {
                    // can't re-use any of our input chans, so get a new one and copy everything into it..
                    bufIndex = getFreeBuffer (false);
                    jassert (bufIndex != 0);

                    const int srcIndex = getBufferContaining (sourceNodes.getUnchecked (0),
                                                              sourceOutputChans.getUnchecked (0));
                    if (srcIndex < 0)
                    {
                        // if not found, this is probably a feedback loop
                        renderingOps.add (new ClearChannelOp (bufIndex));
                    }
                    else
                    {
                        renderingOps.add (new CopyChannelOp (srcIndex, bufIndex));
                    }

                    reusableInputIndex = 0;
                }

                for (int j = 0; j < sourceNodes.size(); ++j)
                {
                    if (j != reusableInputIndex)
                    {
                        const int srcIndex = getBufferContaining (sourceNodes.getUnchecked(j),
                                                                  sourceOutputChans.getUnchecked(j));
                        if (srcIndex >= 0)
                            renderingOps.add (new AddChannelOp (srcIndex, bufIndex));
                    }
                }
            }

            jassert (bufIndex >= 0);
            audioChannelsToUse.add (bufIndex);

            if (inputChan < numOuts)
                markBufferAsContaining (bufIndex, node->id, inputChan);
        }

        for (int outputChan = numIns; outputChan < numOuts; ++outputChan)
        {
            const int bufIndex = getFreeBuffer (false);
            jassert (bufIndex != 0);
            audioChannelsToUse.add (bufIndex);

            markBufferAsContaining (bufIndex, node->id, outputChan);
        }

        // Now the same thing for midi..
        Array <int> midiSourceNodes;

        for (int i = graph.getNumConnections(); --i >= 0;)
        {
            const AudioProcessorGraph::Connection* const c = graph.getConnection (i);

            if (c->destNodeId == node->id && c->destChannelIndex == AudioProcessorGraph::midiChannelIndex)
                midiSourceNodes.add (c->sourceNodeId);
        }

        if (midiSourceNodes.size() == 0)
        {
            // No midi inputs..
            midiBufferToUse = getFreeBuffer (true); // need to pick a buffer even if the processor doesn't use midi

            if (node->processor->acceptsMidi() || node->processor->producesMidi())
                renderingOps.add (new ClearMidiBufferOp (midiBufferToUse));
        }
        else if (midiSourceNodes.size() == 1)
        {
            // One midi input..
            midiBufferToUse = getBufferContaining (midiSourceNodes.getUnchecked(0),
                                                   AudioProcessorGraph::midiChannelIndex);

            if (midiBufferToUse >= 0)
            {
                if (isBufferNeededLater (ourRenderingIndex,
                                         AudioProcessorGraph::midiChannelIndex,
                                         midiSourceNodes.getUnchecked(0),
                                         AudioProcessorGraph::midiChannelIndex))
                {
                    // can't mess up this channel because it's needed later by another node, so we
                    // need to use a copy of it..
                    const int newFreeBuffer = getFreeBuffer (true);
                    renderingOps.add (new CopyMidiBufferOp (midiBufferToUse, newFreeBuffer));
                    midiBufferToUse = newFreeBuffer;
                }
            }
            else
            {
                 // probably a feedback loop, so just use an empty one..
                 midiBufferToUse = getFreeBuffer (true); // need to pick a buffer even if the processor doesn't use midi
            }
        }
        else
        {
            // More than one midi input being mixed..
            int reusableInputIndex = -1;

            for (int i = 0; i < midiSourceNodes.size(); ++i)
            {
                const int sourceBufIndex = getBufferContaining (midiSourceNodes.getUnchecked(i),
                                                                AudioProcessorGraph::midiChannelIndex);

                if (sourceBufIndex >= 0
                     && ! isBufferNeededLater (ourRenderingIndex,
                                               AudioProcessorGraph::midiChannelIndex,
                                               midiSourceNodes.getUnchecked(i),
                                               AudioProcessorGraph::midiChannelIndex))
                {
                    // we've found one of our input buffers that can be re-used..
                    reusableInputIndex = i;
                    midiBufferToUse = sourceBufIndex;
                    break;
                }
            }

            if (reusableInputIndex < 0)
            {
                // can't re-use any of our input buffers, so get a new one and copy everything into it..
                midiBufferToUse = getFreeBuffer (true);
                jassert (midiBufferToUse >= 0);

                const int srcIndex = getBufferContaining (midiSourceNodes.getUnchecked(0),
                                                          AudioProcessorGraph::midiChannelIndex);
                if (srcIndex >= 0)
                    renderingOps.add (new CopyMidiBufferOp (srcIndex, midiBufferToUse));
                else
                    renderingOps.add (new ClearMidiBufferOp (midiBufferToUse));

                reusableInputIndex = 0;
            }

            for (int j = 0; j < midiSourceNodes.size(); ++j)
            {
                if (j != reusableInputIndex)
                {
                    const int srcIndex = getBufferContaining (midiSourceNodes.getUnchecked(j),
                                                              AudioProcessorGraph::midiChannelIndex);
                    if (srcIndex >= 0)
                        renderingOps.add (new AddMidiBufferOp (srcIndex, midiBufferToUse));
                }
            }
        }

        if (node->processor->producesMidi())
            markBufferAsContaining (midiBufferToUse, node->id,
                                    AudioProcessorGraph::midiChannelIndex);

        renderingOps.add (new ProcessBufferOp (node, audioChannelsToUse,
                                               totalChans, midiBufferToUse));
    }

    //==============================================================================
    int getFreeBuffer (const bool forMidi)
    {
        if (forMidi)
        {
            for (int i = 1; i < midiNodeIds.size(); ++i)
                if (midiNodeIds.getUnchecked(i) < 0)
                    return i;

            midiNodeIds.add (-1);
            return midiNodeIds.size() - 1;
        }
        else
        {
            for (int i = 1; i < nodeIds.size(); ++i)
                if (nodeIds.getUnchecked(i) < 0)
                    return i;

            nodeIds.add (-1);
            channels.add (0);
            return nodeIds.size() - 1;
        }
    }

    int getReadOnlyEmptyBuffer() const
    {
        return 0;
    }

    int getBufferContaining (const int nodeId, const int outputChannel) const
    {
        if (outputChannel == AudioProcessorGraph::midiChannelIndex)
        {
            for (int i = midiNodeIds.size(); --i >= 0;)
                if (midiNodeIds.getUnchecked(i) == nodeId)
                    return i;
        }
        else
        {
            for (int i = nodeIds.size(); --i >= 0;)
                if (nodeIds.getUnchecked(i) == nodeId
                     && channels.getUnchecked(i) == outputChannel)
                    return i;
        }

        return -1;
    }

    void markAnyUnusedBuffersAsFree (const int stepIndex)
    {
        int i;
        for (i = 0; i < nodeIds.size(); ++i)
        {
            if (nodeIds.getUnchecked(i) >= 0
                 && ! isBufferNeededLater (stepIndex, -1,
                                           nodeIds.getUnchecked(i),
                                           channels.getUnchecked(i)))
            {
                nodeIds.set (i, -1);
            }
        }

        for (i = 0; i < midiNodeIds.size(); ++i)
        {
            if (midiNodeIds.getUnchecked(i) >= 0
                 && ! isBufferNeededLater (stepIndex, -1,
                                           midiNodeIds.getUnchecked(i),
                                           AudioProcessorGraph::midiChannelIndex))
            {
                midiNodeIds.set (i, -1);
            }
        }
    }

    bool isBufferNeededLater (int stepIndexToSearchFrom,
                              int inputChannelOfIndexToIgnore,
                              const int nodeId,
                              const int outputChanIndex) const
    {
        while (stepIndexToSearchFrom < orderedNodes.size())
        {
            const AudioProcessorGraph::Node* const node = (const AudioProcessorGraph::Node*) orderedNodes.getUnchecked (stepIndexToSearchFrom);

            if (outputChanIndex == AudioProcessorGraph::midiChannelIndex)
            {
                if (inputChannelOfIndexToIgnore != AudioProcessorGraph::midiChannelIndex
                     && graph.getConnectionBetween (nodeId, AudioProcessorGraph::midiChannelIndex,
                                                    node->id, AudioProcessorGraph::midiChannelIndex) != 0)
                    return true;
            }
            else
            {
                for (int i = 0; i < node->processor->getNumInputChannels(); ++i)
                    if (i != inputChannelOfIndexToIgnore
                         && graph.getConnectionBetween (nodeId, outputChanIndex,
                                                        node->id, i) != 0)
                        return true;
            }

            inputChannelOfIndexToIgnore = -1;
            ++stepIndexToSearchFrom;
        }

        return false;
    }

    void markBufferAsContaining (int bufferNum, int nodeId, int outputIndex)
    {
        if (outputIndex == AudioProcessorGraph::midiChannelIndex)
        {
            jassert (bufferNum > 0 && bufferNum < midiNodeIds.size());

            midiNodeIds.set (bufferNum, nodeId);
        }
        else
        {
            jassert (bufferNum >= 0 && bufferNum < nodeIds.size());

            nodeIds.set (bufferNum, nodeId);
            channels.set (bufferNum, outputIndex);
        }
    }

    RenderingOpSequenceCalculator (const RenderingOpSequenceCalculator&);
    const RenderingOpSequenceCalculator& operator= (const RenderingOpSequenceCalculator&);
};

}

//==============================================================================
void AudioProcessorGraph::clearRenderingSequence()
{
    const ScopedLock sl (renderLock);

    for (int i = renderingOps.size(); --i >= 0;)
    {
        GraphRenderingOps::AudioGraphRenderingOp* const r
            = (GraphRenderingOps::AudioGraphRenderingOp*) renderingOps.getUnchecked(i);

        renderingOps.remove (i);
        delete r;
    }
}

bool AudioProcessorGraph::isAnInputTo (const uint32 possibleInputId,
                                       const uint32 possibleDestinationId,
                                       const int recursionCheck) const
{
    if (recursionCheck > 0)
    {
        for (int i = connections.size(); --i >= 0;)
        {
            const AudioProcessorGraph::Connection* const c = connections.getUnchecked (i);

            if (c->destNodeId == possibleDestinationId
                 && (c->sourceNodeId == possibleInputId
                      || isAnInputTo (possibleInputId, c->sourceNodeId, recursionCheck - 1)))
                return true;
        }
    }

    return false;
}

void AudioProcessorGraph::buildRenderingSequence()
{
    VoidArray newRenderingOps;
    int numRenderingBuffersNeeded = 2;
    int numMidiBuffersNeeded = 1;

    {
        MessageManagerLock mml;

        VoidArray orderedNodes;

        int i;
        for (i = 0; i < nodes.size(); ++i)
        {
            Node* const node = nodes.getUnchecked(i);

            node->prepare (getSampleRate(), getBlockSize(), this);

            int j = 0;
            for (; j < orderedNodes.size(); ++j)
                if (isAnInputTo (node->id,
                                 ((Node*) orderedNodes.getUnchecked (j))->id,
                                 nodes.size() + 1))
                    break;

            orderedNodes.insert (j, node);
        }

        GraphRenderingOps::RenderingOpSequenceCalculator calculator (*this, orderedNodes, newRenderingOps);

        numRenderingBuffersNeeded = calculator.getNumBuffersNeeded();
        numMidiBuffersNeeded = calculator.getNumMidiBuffersNeeded();
    }

    VoidArray oldRenderingOps (renderingOps);

    {
        // swap over to the new rendering sequence..
        const ScopedLock sl (renderLock);

        renderingBuffers.setSize (numRenderingBuffersNeeded, getBlockSize());
        renderingBuffers.clear();

        for (int i = midiBuffers.size(); --i >= 0;)
            midiBuffers.getUnchecked(i)->clear();

        while (midiBuffers.size() < numMidiBuffersNeeded)
            midiBuffers.add (new MidiBuffer());

        renderingOps = newRenderingOps;
    }

    for (int i = oldRenderingOps.size(); --i >= 0;)
        delete (GraphRenderingOps::AudioGraphRenderingOp*) oldRenderingOps.getUnchecked(i);
}

void AudioProcessorGraph::handleAsyncUpdate()
{
    buildRenderingSequence();
}

//==============================================================================
void AudioProcessorGraph::prepareToPlay (double /*sampleRate*/, int estimatedSamplesPerBlock)
{
    currentAudioInputBuffer = 0;
    currentAudioOutputBuffer.setSize (jmax (1, getNumOutputChannels()), estimatedSamplesPerBlock);
    currentMidiInputBuffer = 0;
    currentMidiOutputBuffer.clear();

    clearRenderingSequence();
    buildRenderingSequence();
}

void AudioProcessorGraph::releaseResources()
{
    for (int i = 0; i < nodes.size(); ++i)
        nodes.getUnchecked(i)->unprepare();

    renderingBuffers.setSize (1, 1);
    midiBuffers.clear();

    currentAudioInputBuffer = 0;
    currentAudioOutputBuffer.setSize (1, 1);
    currentMidiInputBuffer = 0;
    currentMidiOutputBuffer.clear();
}

void AudioProcessorGraph::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    const int numSamples = buffer.getNumSamples();

    const ScopedLock sl (renderLock);

    currentAudioInputBuffer = &buffer;
    currentAudioOutputBuffer.setSize (jmax (1, buffer.getNumChannels()), numSamples);
    currentAudioOutputBuffer.clear();
    currentMidiInputBuffer = &midiMessages;
    currentMidiOutputBuffer.clear();

    int i;
    for (i = 0; i < renderingOps.size(); ++i)
    {
        GraphRenderingOps::AudioGraphRenderingOp* const op
            = (GraphRenderingOps::AudioGraphRenderingOp*) renderingOps.getUnchecked(i);

        op->perform (renderingBuffers, midiBuffers, numSamples);
    }

    for (i = 0; i < buffer.getNumChannels(); ++i)
        buffer.copyFrom (i, 0, currentAudioOutputBuffer, i, 0, numSamples);

    midiMessages.clear();
    midiMessages.addEvents (currentMidiOutputBuffer, 0, buffer.getNumSamples(), 0);
}

const String AudioProcessorGraph::getInputChannelName (const int channelIndex) const
{
    return "Input " + String (channelIndex + 1);
}

const String AudioProcessorGraph::getOutputChannelName (const int channelIndex) const
{
    return "Output " + String (channelIndex + 1);
}

bool AudioProcessorGraph::isInputChannelStereoPair (int /*index*/) const
{
    return true;
}

bool AudioProcessorGraph::isOutputChannelStereoPair (int /*index*/) const
{
    return true;
}

bool AudioProcessorGraph::acceptsMidi() const
{
    return true;
}

bool AudioProcessorGraph::producesMidi() const
{
    return true;
}

void AudioProcessorGraph::getStateInformation (JUCE_NAMESPACE::MemoryBlock& /*destData*/)
{
}

void AudioProcessorGraph::setStateInformation (const void* /*data*/, int /*sizeInBytes*/)
{
}


//==============================================================================
AudioProcessorGraph::AudioGraphIOProcessor::AudioGraphIOProcessor (const IODeviceType type_)
    : type (type_),
      graph (0)
{
}

AudioProcessorGraph::AudioGraphIOProcessor::~AudioGraphIOProcessor()
{
}

const String AudioProcessorGraph::AudioGraphIOProcessor::getName() const
{
    switch (type)
    {
    case audioOutputNode:
        return "Audio Output";
    case audioInputNode:
        return "Audio Input";
    case midiOutputNode:
        return "Midi Output";
    case midiInputNode:
        return "Midi Input";
    default:
        break;
    }

    return String::empty;
}

void AudioProcessorGraph::AudioGraphIOProcessor::fillInPluginDescription (PluginDescription& d) const
{
    d.name = getName();
    d.uid = d.name.hashCode();
    d.category = "I/O devices";
    d.pluginFormatName = "Internal";
    d.manufacturerName = "Raw Material Software";
    d.version = "1.0";
    d.isInstrument = false;

    d.numInputChannels = getNumInputChannels();
    if (type == audioOutputNode && graph != 0)
        d.numInputChannels = graph->getNumInputChannels();

    d.numOutputChannels = getNumOutputChannels();
    if (type == audioInputNode && graph != 0)
        d.numOutputChannels = graph->getNumOutputChannels();
}

void AudioProcessorGraph::AudioGraphIOProcessor::prepareToPlay (double, int)
{
    jassert (graph != 0);
}

void AudioProcessorGraph::AudioGraphIOProcessor::releaseResources()
{
}

void AudioProcessorGraph::AudioGraphIOProcessor::processBlock (AudioSampleBuffer& buffer,
                                                               MidiBuffer& midiMessages)
{
    jassert (graph != 0);

    switch (type)
    {
    case audioOutputNode:
    {
        for (int i = jmin (graph->currentAudioOutputBuffer.getNumChannels(),
                           buffer.getNumChannels()); --i >= 0;)
        {
            graph->currentAudioOutputBuffer.addFrom (i, 0, buffer, i, 0, buffer.getNumSamples());
        }

        break;
    }

    case audioInputNode:
    {
        for (int i = jmin (graph->currentAudioInputBuffer->getNumChannels(),
                           buffer.getNumChannels()); --i >= 0;)
        {
            buffer.copyFrom (i, 0, *graph->currentAudioInputBuffer, i, 0, buffer.getNumSamples());
        }

        break;
    }

    case midiOutputNode:
        graph->currentMidiOutputBuffer.addEvents (midiMessages, 0, buffer.getNumSamples(), 0);
        break;

    case midiInputNode:
        midiMessages.addEvents (*graph->currentMidiInputBuffer, 0, buffer.getNumSamples(), 0);
        break;

    default:
        break;
    }
}

bool AudioProcessorGraph::AudioGraphIOProcessor::acceptsMidi() const
{
    return type == midiOutputNode;
}

bool AudioProcessorGraph::AudioGraphIOProcessor::producesMidi() const
{
    return type == midiInputNode;
}

const String AudioProcessorGraph::AudioGraphIOProcessor::getInputChannelName (const int channelIndex) const
{
    switch (type)
    {
    case audioOutputNode:
        return "Output " + String (channelIndex + 1);
    case midiOutputNode:
        return "Midi Output";
    default:
        break;
    }

    return String::empty;
}

const String AudioProcessorGraph::AudioGraphIOProcessor::getOutputChannelName (const int channelIndex) const
{
    switch (type)
    {
    case audioInputNode:
        return "Input " + String (channelIndex + 1);
    case midiInputNode:
        return "Midi Input";
    default:
        break;
    }

    return String::empty;
}

bool AudioProcessorGraph::AudioGraphIOProcessor::isInputChannelStereoPair (int /*index*/) const
{
    return type == audioInputNode || type == audioOutputNode;
}

bool AudioProcessorGraph::AudioGraphIOProcessor::isOutputChannelStereoPair (int index) const
{
    return isInputChannelStereoPair (index);
}

bool AudioProcessorGraph::AudioGraphIOProcessor::isInput() const
{
    return type == audioInputNode || type == midiInputNode;
}

bool AudioProcessorGraph::AudioGraphIOProcessor::isOutput() const
{
    return type == audioOutputNode || type == midiOutputNode;
}

AudioProcessorEditor* AudioProcessorGraph::AudioGraphIOProcessor::createEditor()
{
    return 0;
}

int AudioProcessorGraph::AudioGraphIOProcessor::getNumParameters()                  { return 0; }
const String AudioProcessorGraph::AudioGraphIOProcessor::getParameterName (int)     { return String::empty; }

float AudioProcessorGraph::AudioGraphIOProcessor::getParameter (int)                { return 0.0f; }
const String AudioProcessorGraph::AudioGraphIOProcessor::getParameterText (int)     { return String::empty; }
void AudioProcessorGraph::AudioGraphIOProcessor::setParameter (int, float)          { }

int AudioProcessorGraph::AudioGraphIOProcessor::getNumPrograms()                    { return 0; }
int AudioProcessorGraph::AudioGraphIOProcessor::getCurrentProgram()                 { return 0; }
void AudioProcessorGraph::AudioGraphIOProcessor::setCurrentProgram (int)            { }

const String AudioProcessorGraph::AudioGraphIOProcessor::getProgramName (int)       { return String::empty; }
void AudioProcessorGraph::AudioGraphIOProcessor::changeProgramName (int, const String&)  { }

void AudioProcessorGraph::AudioGraphIOProcessor::getStateInformation (JUCE_NAMESPACE::MemoryBlock&)
{
}

void AudioProcessorGraph::AudioGraphIOProcessor::setStateInformation (const void*, int)
{
}

void AudioProcessorGraph::AudioGraphIOProcessor::setParentGraph (AudioProcessorGraph* const newGraph)
{
    graph = newGraph;

    if (graph != 0)
    {
        setPlayConfigDetails (type == audioOutputNode ? graph->getNumOutputChannels() : 0,
                              type == audioInputNode ? graph->getNumInputChannels() : 0,
                              getSampleRate(),
                              getBlockSize());

        updateHostDisplay();
    }
}


END_JUCE_NAMESPACE
