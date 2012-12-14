/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_MULTITOUCHMAPPER_JUCEHEADER__
#define __JUCE_MULTITOUCHMAPPER_JUCEHEADER__

template <typename IDType>
class MultiTouchMapper
{
public:
    MultiTouchMapper() {}

    int getIndexOfTouch (IDType touchID)
    {
        jassert (touchID != 0); // need to rethink this if IDs can be 0!

        int touchIndex = currentTouches.indexOf (touchID);

        if (touchIndex < 0)
        {
            for (touchIndex = 0; touchIndex < currentTouches.size(); ++touchIndex)
                if (currentTouches.getUnchecked (touchIndex) == 0)
                    break;

            currentTouches.set (touchIndex, touchID);
        }

        return touchIndex;
    }

    void clear()
    {
        currentTouches.clear();
    }

    void clearTouch (int index)
    {
        currentTouches.set (index, 0);
    }

    bool areAnyTouchesActive() const noexcept
    {
        for (int i = currentTouches.size(); --i >= 0;)
            if (currentTouches.getUnchecked(i) != 0)
                return true;

        return false;
    }

private:
    Array<IDType> currentTouches;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MultiTouchMapper)
};

#endif   // __JUCE_MULTITOUCHMAPPER_JUCEHEADER__
