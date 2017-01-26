/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/


/** This topology source holds and applies a set of rules for transforming
    one device topology into another one that may involve virtual and/or
    aggregate devices.

    Given an input PhysicalTopologySource and a set of Rule objects, this class
     will apply the rules and present the resulting topology to clients.
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
    BlockTopology getCurrentTopology() const;

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

private:
    //==========================================================================
    struct Internal;
    juce::ScopedPointer<Internal> internal;
};
