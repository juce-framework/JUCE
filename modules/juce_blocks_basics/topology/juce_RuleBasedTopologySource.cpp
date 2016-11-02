/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

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
