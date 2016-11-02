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
