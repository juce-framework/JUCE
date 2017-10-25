/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

template <typename IDType>
class MultiTouchMapper
{
public:
    MultiTouchMapper() {}

    int getIndexOfTouch (ComponentPeer* peer, IDType touchID)
    {
        jassert (touchID != 0); // need to rethink this if IDs can be 0!
        TouchInfo info {touchID, peer};

        int touchIndex = currentTouches.indexOf (info);

        if (touchIndex < 0)
        {
            auto emptyTouchIndex = currentTouches.indexOf ({});
            touchIndex = (emptyTouchIndex >= 0 ? emptyTouchIndex : currentTouches.size());

            currentTouches.set (touchIndex, info);
        }

        return touchIndex;
    }

    void clear()
    {
        currentTouches.clear();
    }

    void clearTouch (int index)
    {
        currentTouches.set (index, {});
    }

    bool areAnyTouchesActive() const noexcept
    {
        for (auto& t : currentTouches)
            if (t.touchId != 0)
                return true;

        return false;
    }

    void deleteAllTouchesForPeer (ComponentPeer* peer)
    {
        for (auto& t : currentTouches)
            if (t.owner == peer)
                t.touchId = 0;
    }

private:
    //==============================================================================
    struct TouchInfo
    {
        TouchInfo() noexcept  : touchId (0), owner (nullptr) {}
        TouchInfo (IDType idToUse, ComponentPeer* peer) noexcept  : touchId (idToUse), owner (peer) {}

        TouchInfo (const TouchInfo&) = default;
        TouchInfo& operator= (const TouchInfo&) = default;

        // VS2013 can't default move constructors
        TouchInfo (TouchInfo&& other) noexcept  : touchId (other.touchId), owner (other.owner) {}

        // VS2013 can't default move assignments
        TouchInfo& operator= (TouchInfo&& other) noexcept
        {
            touchId = other.touchId;
            owner = other.owner;

            return *this;
        }

        IDType touchId;
        ComponentPeer* owner;

        bool operator== (const TouchInfo& o) const noexcept     { return (touchId == o.touchId); }
    };

    //==============================================================================
    Array<TouchInfo> currentTouches;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiTouchMapper)
};

} // namespace juce
