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

#ifndef DOXYGEN
namespace ProcessorHelpers  // Internal helper classes used in building the ProcessorChain
{
    template <int arg>
    struct AccessHelper
    {
        template <typename ProcessorType>
        static auto& get (ProcessorType& a) noexcept                 { return AccessHelper<arg - 1>::get (a.processors); }

        template <typename ProcessorType>
        static const auto& get (const ProcessorType& a) noexcept     { return AccessHelper<arg - 1>::get (a.processors); }

        template <typename ProcessorType>
        static void setBypassed (ProcessorType& a, bool bypassed)    { AccessHelper<arg - 1>::setBypassed (a.processors, bypassed); }
    };

    template <>
    struct AccessHelper<0>
    {
        template <typename ProcessorType>
        static auto& get (ProcessorType& a) noexcept                 { return a.getProcessor(); }

        template <typename ProcessorType>
        static const auto& get (const ProcessorType& a) noexcept     { return a.getProcessor(); }

        template <typename ProcessorType>
        static void setBypassed (ProcessorType& a, bool bypassed)    { a.isBypassed = bypassed; }
    };

    //==============================================================================
    template <bool isFirst, typename Processor, typename Subclass>
    struct ChainElement
    {
        void prepare (const ProcessSpec& spec)
        {
            processor.prepare (spec);
        }

        template <typename ProcessContext>
        void process (const ProcessContext& context) noexcept
        {
            if (context.usesSeparateInputAndOutputBlocks() && ! isFirst)
            {
                jassert (context.getOutputBlock().getNumChannels() == context.getInputBlock().getNumChannels());
                ProcessContextReplacing<typename ProcessContext::SampleType> replacingContext (context.getOutputBlock());
                replacingContext.isBypassed = (isBypassed || context.isBypassed);

                processor.process (replacingContext);
            }
            else
            {
                ProcessContext contextCopy (context);
                contextCopy.isBypassed = (isBypassed || context.isBypassed);

                processor.process (contextCopy);
            }
        }

        void reset()
        {
            processor.reset();
        }

        bool isBypassed = false;
        Processor processor;

        Processor& getProcessor() noexcept             { return processor; }
        const Processor& getProcessor() const noexcept { return processor; }
        Subclass& getThis() noexcept                   { return *static_cast<Subclass*> (this); }
        const Subclass& getThis() const noexcept       { return *static_cast<const Subclass*> (this); }

        template <int arg> auto& get() noexcept                      { return AccessHelper<arg>::get (getThis()); }
        template <int arg> const auto& get() const noexcept          { return AccessHelper<arg>::get (getThis()); }
        template <int arg> void setBypassed (bool bypassed) noexcept { AccessHelper<arg>::setBypassed (getThis(), bypassed); }
    };

    //==============================================================================
    template <bool isFirst, typename FirstProcessor, typename... SubsequentProcessors>
    struct ChainBase  : public ChainElement<isFirst, FirstProcessor, ChainBase<isFirst, FirstProcessor, SubsequentProcessors...>>
    {
        using Base = ChainElement<isFirst, FirstProcessor, ChainBase<isFirst, FirstProcessor, SubsequentProcessors...>>;

        template <typename ProcessContext>
        void process (const ProcessContext& context) noexcept  { Base::process (context); processors.process (context); }
        void prepare (const ProcessSpec& spec)                 { Base::prepare (spec); processors.prepare (spec); }
        void reset()                                           { Base::reset(); processors.reset(); }

        ChainBase<false, SubsequentProcessors...> processors;
    };

    template <bool isFirst, typename ProcessorType>
    struct ChainBase<isFirst, ProcessorType>  : public ChainElement<isFirst, ProcessorType, ChainBase<isFirst, ProcessorType>> {};
}
#endif


//==============================================================================
/**
    This variadically-templated class lets you join together any number of processor
    classes into a single processor which will call process() on them all in sequence.
*/
template <typename... Processors>
using ProcessorChain = ProcessorHelpers::ChainBase<true, Processors...>;

} // namespace dsp
} // namespace juce
