/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

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

//==============================================================================
#ifndef DOXYGEN
/** The contents of this namespace are used to implement ProcessorChain and should
    not be used elsewhere. Their interfaces (and existence) are liable to change!
*/
namespace detail
{
    template <typename Fn, typename Tuple, size_t... Ix>
    constexpr void forEachInTuple (Fn&& fn, Tuple&& tuple, std::index_sequence<Ix...>)
    {
        (void) std::initializer_list<int> { ((void) fn (std::get<Ix> (tuple), std::integral_constant<size_t, Ix>()), 0)... };
    }

    template <typename T>
    using TupleIndexSequence = std::make_index_sequence<std::tuple_size<std::remove_cv_t<std::remove_reference_t<T>>>::value>;

    template <typename Fn, typename Tuple>
    constexpr void forEachInTuple (Fn&& fn, Tuple&& tuple)
    {
        forEachInTuple (std::forward<Fn> (fn), std::forward<Tuple> (tuple), TupleIndexSequence<Tuple>{});
    }

    // This could be a template variable, but that code causes an internal compiler error in MSVC 19.00.24215
    template <typename Context, size_t Ix>
    struct UseContextDirectly
    {
        static constexpr auto value = ! Context::usesSeparateInputAndOutputBlocks() || Ix == 0;
    };
}
#endif

/** This variadically-templated class lets you join together any number of processor
    classes into a single processor which will call process() on them all in sequence.

    @tags{DSP}
*/
template <typename... Processors>
class ProcessorChain
{
public:
    /** Get a reference to the processor at index `Index`. */
    template <int Index>       auto& get()       noexcept { return std::get<Index> (processors); }

    /** Get a reference to the processor at index `Index`. */
    template <int Index> const auto& get() const noexcept { return std::get<Index> (processors); }

    /** Set the processor at index `Index` to be bypassed or enabled. */
    template <int Index>
    void setBypassed (bool b) noexcept  { bypassed[(size_t) Index] = b; }

    /** Query whether the processor at index `Index` is bypassed. */
    template <int Index>
    bool isBypassed() const noexcept    { return bypassed[(size_t) Index]; }

    /** Prepare all inner processors with the provided `ProcessSpec`. */
    void prepare (const ProcessSpec& spec)
    {
        detail::forEachInTuple ([&] (auto& proc, auto) { proc.prepare (spec); }, processors);
    }

    /** Reset all inner processors. */
    void reset()
    {
        detail::forEachInTuple ([] (auto& proc, auto) { proc.reset(); }, processors);
    }

    /** Process `context` through all inner processors in sequence. */
    template <typename ProcessContext>
    void process (const ProcessContext& context) noexcept
    {
        detail::forEachInTuple ([this, &context] (auto& proc, auto index) noexcept { this->processOne (context, proc, index); },
                                processors);
    }

private:
    template <typename Context, typename Proc, size_t Ix, std::enable_if_t<! detail::UseContextDirectly<Context, Ix>::value, int> = 0>
    void processOne (const Context& context, Proc& proc, std::integral_constant<size_t, Ix>) noexcept
    {
        jassert (context.getOutputBlock().getNumChannels() == context.getInputBlock().getNumChannels());
        ProcessContextReplacing<typename Context::SampleType> replacingContext (context.getOutputBlock());
        replacingContext.isBypassed = (bypassed[Ix] || context.isBypassed);

        proc.process (replacingContext);
    }

    template <typename Context, typename Proc, size_t Ix, std::enable_if_t<detail::UseContextDirectly<Context, Ix>::value, int> = 0>
    void processOne (const Context& context, Proc& proc, std::integral_constant<size_t, Ix>) noexcept
    {
        auto contextCopy = context;
        contextCopy.isBypassed = (bypassed[Ix] || context.isBypassed);

        proc.process (contextCopy);
    }

    std::tuple<Processors...> processors;
    std::array<bool, sizeof...(Processors)> bypassed { {} };
};

/** Non-member equivalent of ProcessorChain::get which avoids awkward
    member template syntax.
*/
template <int Index, typename... Processors>
inline auto& get (ProcessorChain<Processors...>& chain) noexcept
{
    return chain.template get<Index>();
}

/** Non-member equivalent of ProcessorChain::get which avoids awkward
    member template syntax.
*/
template <int Index, typename... Processors>
inline auto& get (const ProcessorChain<Processors...>& chain) noexcept
{
    return chain.template get<Index>();
}

/** Non-member equivalent of ProcessorChain::setBypassed which avoids awkward
    member template syntax.
*/
template <int Index, typename... Processors>
inline void setBypassed (ProcessorChain<Processors...>& chain, bool bypassed) noexcept
{
    chain.template setBypassed<Index> (bypassed);
}

/** Non-member equivalent of ProcessorChain::isBypassed which avoids awkward
    member template syntax.
*/
template <int Index, typename... Processors>
inline bool isBypassed (const ProcessorChain<Processors...>& chain) noexcept
{
    return chain.template isBypassed<Index>();
}

} // namespace dsp
} // namespace juce

#ifndef DOXYGEN
namespace std
{

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wmismatched-tags")

/** Adds support for C++17 structured bindings. */
template <typename... Processors>
struct tuple_size<::juce::dsp::ProcessorChain<Processors...>> : integral_constant<size_t, sizeof... (Processors)> {};

/** Adds support for C++17 structured bindings. */
template <size_t I, typename... Processors>
struct tuple_element<I, ::juce::dsp::ProcessorChain<Processors...>> : tuple_element<I, tuple<Processors...>> {};

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

} // namespace std
#endif
