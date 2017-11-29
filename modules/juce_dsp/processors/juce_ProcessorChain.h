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
    struct GetterHelper
    {
        template <typename ProcessorType>
        static auto& get (ProcessorType& a) noexcept    { return GetterHelper<arg - 1>::get (a.processors); }
    };

    template <>
    struct GetterHelper<0>
    {
        template <typename ProcessorType>
        static auto& get (ProcessorType& a) noexcept    { return a.getProcessor(); }
    };

    template <typename Processor, typename Subclass>
    struct ChainBase
    {
        Processor processor;

        Processor& getProcessor() noexcept       { return processor; }
        Subclass& getThis() noexcept             { return *static_cast<Subclass*> (this); }

        template <int arg> auto& get() noexcept  { return GetterHelper<arg>::get (getThis()); }
    };

    template <typename FirstProcessor, typename... SubsequentProcessors>
    struct Chain  : public ChainBase<FirstProcessor, Chain<FirstProcessor, SubsequentProcessors...>>
    {
        using Base = ChainBase<FirstProcessor, Chain<FirstProcessor, SubsequentProcessors...>>;

        void prepare (const ProcessSpec& spec)
        {
            Base::processor.prepare (spec);
            processors.prepare (spec);
        }

        template <typename ProcessContext>
        void process (ProcessContext& context) noexcept
        {
            Base::processor.process (context);
            processors.process (context);
        }

        void reset()
        {
            Base::processor.reset();
            processors.reset();
        }

        Chain<SubsequentProcessors...> processors;
    };

    template <typename ProcessorType>
    struct Chain<ProcessorType>  : public ChainBase<ProcessorType, Chain<ProcessorType>>
    {
        using Base = ChainBase<ProcessorType, Chain<ProcessorType>>;

        template <typename ProcessContext>
        void process (ProcessContext& context) noexcept
        {
            Base::processor.process (context);
        }

        void prepare (const ProcessSpec& spec)
        {
            Base::processor.prepare (spec);
        }

        void reset()
        {
            Base::processor.reset();
        }
    };
}
#endif


//==============================================================================
/**
    This variadically-templated class lets you join together any number of processor
    classes into a single processor which will call process() on them all in sequence.
*/
template <typename... Processors>
using ProcessorChain = ProcessorHelpers::Chain<Processors...>;

} // namespace dsp
} // namespace juce
