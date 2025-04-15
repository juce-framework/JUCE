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

namespace juce
{

/** A helper struct providing functions for managing sorted containers.

    These helper functions simplify common operations on containers that
    are kept in a sorted order.

    @tags{Core}
*/
struct OrderedContainerHelpers
{
    /** Returns true if neither value compares less than the other.
        This is the same definition of equivalence as used by the std containers "set" and "map".
    */
    template <typename A, typename B, typename Less = std::less<>>
    static constexpr bool equivalent (const A& a, const B& b, Less less = {})
    {
        return ! less (a, b) && ! less (b, a);
    }

    //==============================================================================
    /** If the container already contains a value equivalent to the valueToInsert, assigns
        the new value over the old one; otherwise, if no equivalent tag exists, inserts the
        new value preserving the sorted property of the container.
    */
    template <typename OrderedContainer, typename ValueType, typename Less = std::less<>>
    static void insertOrAssign (OrderedContainer& container,
                                const ValueType& valueToInsert,
                                Less less = {})
    {
        // This function won't do the right thing on a container that's not sorted!
        jassert (std::is_sorted (container.begin(), container.end(), less));

        auto iter = std::lower_bound (container.begin(), container.end(), valueToInsert, less);

        if (iter != container.end() && equivalent (*iter, valueToInsert, less))
        {
            *iter = valueToInsert;
        }
        else
        {
            container.insert (iter, valueToInsert);
        }
    }

    /** Removes a specific element from a sorted array, preserving order.

        Searches for an element in the container that compares equivalent to valueToRemove and
        erases it if present, preserving the sorted property of the container.

        Returns true if the array was modified or false otherwise (i.e. the element was not removed).
    */
    template <typename OrderedContainer, typename ValueType, typename Less = std::less<>>
    static bool remove (OrderedContainer& container,
                        const ValueType& valueToRemove,
                        Less less = {})
    {
        // This function won't do the right thing on a container that's not sorted!
        jassert (std::is_sorted (container.begin(), container.end(), less));

        auto iter = std::lower_bound (container.begin(), container.end(), valueToRemove, less);

        if (iter == container.end() || ! equivalent (*iter, valueToRemove, less))
            return false;

        container.erase (iter);
        return true;
    }

    OrderedContainerHelpers() = delete;
};

}
