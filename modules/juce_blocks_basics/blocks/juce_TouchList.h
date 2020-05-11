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

/**
    Utility class to hold a list of TouchSurface::Touch objects with different
    indices and blockUIDs, where each touch has a mapping to some kind of
    user-supplied data value.

    The Type template is a user-defined type of object that will be stored for
    each touch element. The type must be default-constructable and copyable.

    @tags{Blocks}
*/
template <typename Type>
class TouchList
{
public:
    /** Creates an empty touch list. */
    TouchList() = default;

    /** Destructor. */
    ~TouchList() = default;

    /** Returns the number of entries in the touch list. */
    int size() const noexcept { return touches.size(); }

    /** Returns the user data object that corresponds to the given touch.
        This will also update the stored state of the TouchEntry::touch value
        for this touch index.
    */
    Type& getValue (const TouchSurface::Touch& touch)
    {
        auto* t = find (touch);

        if (t == nullptr)
        {
            touches.add ({ touch, {} });
            return touches.getReference (touches.size() - 1).value;
        }
        else
        {
            t->touch = touch;
            return t->value;
        }
    }

    /** Returns true if a touch is already in the list. */
    bool contains (const TouchSurface::Touch& touch) const noexcept
    {
        return find (touch) != nullptr;
    }

    /** Updates the entry for the given touch, copying in the new state.
        If no entry with the same index and blockUID exists then a new entry is
        created. If given a touch which is a touch-end, this will *remove* any
        corresponding entries from the list.
    */
    void updateTouch (const TouchSurface::Touch& touch)
    {
        if (touch.isTouchEnd)
        {
            for (int i = touches.size(); --i >= 0;)
                if (matches (touches.getReference(i).touch, touch))
                    touches.remove (i);

            jassert (! contains (touch));
        }
        else
        {
            auto t = find (touch);

            if (t == nullptr)
                touches.add ({ touch, {} });
            else
                t->touch = touch;
        }
    }

    /** Holds the current state of a touch, along with the user-data associated with it. */
    struct TouchEntry
    {
        TouchSurface::Touch touch;
        Type value;
    };

    /** If a touch is in the list, returns a pointer to the TouchEntry.
        Otherwise, returns nullptr.
    */
    const TouchEntry* find (const TouchSurface::Touch& touch) const noexcept
    {
        for (auto& t : touches)
            if (matches (t.touch, touch))
                return &t;

        return nullptr;
    }

    TouchEntry* find (const TouchSurface::Touch& touch) noexcept
    {
        return const_cast<TouchEntry*> (static_cast<const TouchList&> (*this).find (touch));
    }

    /** Allows iterator access to the list of touch entries. */
    TouchEntry* begin() noexcept               { return touches.begin(); }
    const TouchEntry* begin() const noexcept   { return touches.begin(); }

    /** Allows iterator access to the list of touch entries. */
    TouchEntry* end() noexcept                 { return touches.end(); }
    const TouchEntry* end() const noexcept     { return touches.end(); }

    /** Retrieve a reference to particular item in the list of touch entries. */
    TouchEntry& operator[] (const int index)   { return touches.getReference (index); }

    /** Resets all contents, doest not generate any call-backs. */
    void clear() noexcept                      { touches.clear(); }

private:
    //==============================================================================
    static bool matches (const TouchSurface::Touch& t1,
                         const TouchSurface::Touch& t2) noexcept
    {
        return t1.index == t2.index && t1.blockUID == t2.blockUID;
    }

    Array<TouchEntry> touches;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TouchList)
};

} // namespace juce
