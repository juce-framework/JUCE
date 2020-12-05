/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

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

/** Base class for an entity that provides access to a blocks topology.

    @tags{Blocks}
*/
class TopologySource
{
public:
    //==============================================================================
    /** Destructor. */
    virtual ~TopologySource() = default;

    /** Returns the current topology that this object manages. */
    virtual BlockTopology getCurrentTopology() const = 0;

    /** Sets the TopologySource as active, occupying the midi port and trying to connect to the block devices */
    virtual void setActive (bool shouldBeActive) = 0;

    /** Returns true, if the TopologySource is currently trying to connect the block devices */
    virtual bool isActive() const = 0;

    /** Returns true if the topology is locked externally.*/
    virtual bool isLockedFromOutside() const = 0;

    //==============================================================================
    /** Used to receive callbacks for topology changes */
    struct Listener
    {
        virtual ~Listener() = default;

        /** Called for any change in topology - devices changed, connections changed, etc. */
        virtual void topologyChanged() {}

        /** Called when a new block is added to the topology. */
        virtual void blockAdded (const Block::Ptr) {}

        /** Called when a block is removed from the topology. */
        virtual void blockRemoved (const Block::Ptr) {}

        /** Called when a known block is updated.
            This could be because details have been received asynchronously. E.g. Block name.
         */
        virtual void blockUpdated (const Block::Ptr) {}
    };

    void addListener (Listener* l)       { listeners.add (l); }
    void removeListener (Listener* l)    { listeners.remove (l); }

    /** Invoke this to force touches-off on all physical devices. */
    virtual void cancelAllActiveTouches() noexcept {}

    /** Gets blocks from the current topology. */
    Block::Array getBlocks() const { return getCurrentTopology().blocks; }

    /**Gets a block with given uid from the current topology*/
    Block::Ptr getBlockWithUID (Block::UID uid) const { return getCurrentTopology().getBlockWithUID (uid); }

protected:
    //==============================================================================
    ListenerList<Listener> listeners;
};

} // namespace juce
