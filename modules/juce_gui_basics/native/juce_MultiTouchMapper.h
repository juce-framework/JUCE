/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-as-null-pointer-constant")

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
        TouchInfo (TouchInfo&&) noexcept = default;
        TouchInfo& operator= (TouchInfo&&) noexcept = default;

        IDType touchId;
        ComponentPeer* owner;

        bool operator== (const TouchInfo& o) const noexcept     { return (touchId == o.touchId); }
    };

    //==============================================================================
    Array<TouchInfo> currentTouches;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiTouchMapper)
};

} // namespace juce

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
