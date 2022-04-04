/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

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
    Acts as a polymorphic base class for processors.
    This exposes the same set of methods that a processor must implement as virtual
    methods, so that you can use the ProcessorWrapper class to wrap an instance of
    a subclass, and then pass that around using ProcessorBase as a base class.
    @see ProcessorWrapper

    @tags{DSP}
*/
struct ProcessorBase
{
    ProcessorBase() = default;
    virtual ~ProcessorBase() = default;

    virtual void prepare (const ProcessSpec&)  = 0;
    virtual void process (const ProcessContextReplacing<float>&) = 0;
    virtual void reset() = 0;
};


//==============================================================================
/**
    Wraps an instance of a given processor class, and exposes it through the
    ProcessorBase interface.
    @see ProcessorBase

    @tags{DSP}
*/
template <typename ProcessorType>
struct ProcessorWrapper  : public ProcessorBase
{
    void prepare (const ProcessSpec& spec) override
    {
        processor.prepare (spec);
    }

    void process (const ProcessContextReplacing<float>& context) override
    {
        processor.process (context);
    }

    void reset() override
    {
        processor.reset();
    }

    ProcessorType processor;
};

} // namespace dsp
} // namespace juce
