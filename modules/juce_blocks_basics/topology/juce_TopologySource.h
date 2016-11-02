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


/** Base class for an entity that provides access to a blocks topology. */
class TopologySource
{
public:
    //==========================================================================
    /** Destructor. */
    virtual ~TopologySource() {}

    /** Returns the current topology that this object manages. */
    virtual BlockTopology getCurrentTopology() const = 0;

    //==========================================================================
    struct Listener
    {
        virtual ~Listener() {}
        virtual void topologyChanged() = 0;
    };

    void addListener (Listener* l)       { listeners.add (l); }
    void removeListener (Listener* l)    { listeners.remove (l); }

    /** Invoke this to force touches-off on all physical devices. */
    virtual void cancelAllActiveTouches() noexcept {}

protected:
    //==========================================================================
    juce::ListenerList<Listener> listeners;
};
