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

struct RuleBasedTopologySource::Internal  : public TopologySource::Listener,
                                            private juce::AsyncUpdater
{
    Internal (RuleBasedTopologySource& da, TopologySource& bd)  : owner (da), detector (bd)
    {
        detector.addListener (this);
    }

    ~Internal()
    {
        detector.removeListener (this);
    }

    void clearRules()
    {
        if (! rules.isEmpty())
        {
            rules.clear();
            triggerAsyncUpdate();
        }
    }

    void addRule (Rule* r)
    {
        if (r != nullptr)
        {
            rules.add (r);
            triggerAsyncUpdate();
        }
    }

    void topologyChanged() override
    {
        cancelPendingUpdate();
        regenerateTopology();
    }

    void handleAsyncUpdate() override
    {
        topologyChanged();
    }

    void regenerateTopology()
    {
        auto newTopology = detector.getCurrentTopology();

        for (auto rule : rules)
            rule->transformTopology (newTopology);

        if (topology != newTopology)
        {
            topology = newTopology;
            owner.listeners.call ([] (TopologySource::Listener& l) { l.topologyChanged(); });
        }
    }

    RuleBasedTopologySource& owner;
    TopologySource& detector;

    BlockTopology topology;
    juce::OwnedArray<Rule> rules;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Internal)
};

RuleBasedTopologySource::RuleBasedTopologySource (TopologySource& d)
{
    internal.reset (new Internal (*this, d));
}

RuleBasedTopologySource::~RuleBasedTopologySource()
{
    internal = nullptr;
}

BlockTopology RuleBasedTopologySource::getCurrentTopology() const             { return internal->topology; }

void RuleBasedTopologySource::clearRules()                                    { internal->clearRules(); }
void RuleBasedTopologySource::addRule (Rule* r)                               { internal->addRule (r); }

} // namespace juce
