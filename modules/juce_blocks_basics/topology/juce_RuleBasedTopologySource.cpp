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
            owner.listeners.call (&TopologySource::Listener::topologyChanged);
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
    internal = new Internal (*this, d);
}

RuleBasedTopologySource::~RuleBasedTopologySource()
{
    internal = nullptr;
}

BlockTopology RuleBasedTopologySource::getCurrentTopology() const             { return internal->topology; }

void RuleBasedTopologySource::clearRules()                                    { internal->clearRules(); }
void RuleBasedTopologySource::addRule (Rule* r)                               { internal->addRule (r); }
