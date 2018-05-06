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

namespace juce
{
namespace dsp
{

/**
    Converts a mono processor class into a multi-channel version by duplicating it
    and applying multichannel buffers across an array of instances.

    When the prepare method is called, it uses the specified number of channels to
    instantiate the appropriate number of instances, which it then uses in its
    process() method.

    @tags{DSP}
*/
template <typename MonoProcessorType, typename StateType>
struct ProcessorDuplicator
{
    ProcessorDuplicator() : state (new StateType()) {}
    ProcessorDuplicator (StateType* stateToUse) : state (stateToUse) {}
    ProcessorDuplicator (const ProcessorDuplicator&) = default;
    ProcessorDuplicator (ProcessorDuplicator&&) = default;

    void prepare (const ProcessSpec& spec)
    {
        processors.removeRange ((int) spec.numChannels, processors.size());

        while (static_cast<size_t> (processors.size()) < spec.numChannels)
            processors.add (new MonoProcessorType (state));

        auto monoSpec = spec;
        monoSpec.numChannels = 1;

        for (auto* p : processors)
            p->prepare (monoSpec);
    }

    void reset() noexcept      { for (auto* p : processors) p->reset(); }

    template<typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        jassert ((int) context.getInputBlock().getNumChannels()  <= processors.size());
        jassert ((int) context.getOutputBlock().getNumChannels() <= processors.size());

        auto numChannels = static_cast<size_t> (jmin (context.getInputBlock().getNumChannels(),
                                                      context.getOutputBlock().getNumChannels()));

        for (size_t chan = 0; chan < numChannels; ++chan)
            processors[(int) chan]->process (MonoProcessContext<ProcessContext> (context, chan));
    }

    typename StateType::Ptr state;

private:
    template <typename ProcessContext>
    struct MonoProcessContext : public ProcessContext
    {
        MonoProcessContext (const ProcessContext& multiChannelContext, size_t channelToUse)
            : ProcessContext (multiChannelContext), channel (channelToUse)
        {}

        size_t channel;

        typename ProcessContext::AudioBlockType getInputBlock()  const noexcept       { return ProcessContext::getInputBlock().getSingleChannelBlock (channel); }
        typename ProcessContext::AudioBlockType getOutputBlock() const noexcept       { return ProcessContext::getOutputBlock().getSingleChannelBlock (channel); }
    };

    juce::OwnedArray<MonoProcessorType> processors;
};

} // namespace dsp
} // namespace juce
