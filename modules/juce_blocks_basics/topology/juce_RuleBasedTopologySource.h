/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

/** This topology source holds and applies a set of rules for transforming
    one device topology into another one that may involve virtual and/or
    aggregate devices.

    Given an input PhysicalTopologySource and a set of Rule objects, this class
     will apply the rules and present the resulting topology to clients.

    @tags{Blocks}
*/
class RuleBasedTopologySource  : public TopologySource
{
public:
    /** Creates a RuleBasedTopologySource which wraps another TopologySource
        passed in here.
    */
    RuleBasedTopologySource (TopologySource&);

    /** Destructor. */
    ~RuleBasedTopologySource();

    //==========================================================================
    /** Returns the currently active topology. */
    BlockTopology getCurrentTopology() const override;

    /** A rule that can transform parts of a topology. */
    struct Rule
    {
        virtual ~Rule() {}

        /** Subclasses should implement this method and use it as their opportunity to
            examine the given topology and modify it. For example they may want to substitute
            one or more blocks for more specialised, aggregated Block objects.
        */
        virtual void transformTopology (BlockTopology&) = 0;
    };

    /** Clears the list of active rules.
        Calling this method will cause an asynchronous topology update if the new rule-set
        results in a change to the topology.
    */
    void clearRules();

    /** Adds a rule to the list that will be applied.
        The object passed-in will be owned by this object, so don't keep any references
        to it.
        Calling this method will cause an asynchronous topology update if the new rule-set
        results in a change to the topology.
    */
    void addRule (Rule*);

    /** Sets the TopologySource as active, occupying the midi port and trying to connect to the block devices */
    void setActive (bool shouldBeActive) override;

    /** Returns true, if the TopologySource is currently trying to connect the block devices */
    bool isActive() const override;

private:
    //==========================================================================
    struct Internal;
    std::unique_ptr<Internal> internal;
};

} // namespace juce
