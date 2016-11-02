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


/**
    Utility class to hold a list of TouchSurface::Touch objects with different
    indices and blockUIDs, where each touch has a mapping to some kind of
    user-supplied data value.

    The Type template is a user-defined type of object that will be stored for
    each touch element. The type must be default-constructable and copyable.
*/
template <typename Type>
class TouchList
{
public:
    /** Creates an empty touch list. */
    TouchList() {}

    /** Destructor. */
    ~TouchList() {}

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
    TouchEntry* find (const TouchSurface::Touch& touch) const noexcept
    {
        for (auto& t : touches)
            if (matches (t.touch, touch))
                return &t;

        return nullptr;
    }

    /** Allows iterator access to the list of touch entries. */
    TouchEntry* begin() const noexcept      { return touches.begin(); }

    /** Allows iterator access to the list of touch entries. */
    TouchEntry* end() const noexcept        { return touches.end(); }

    /** Retrieve a reference to particular item in the list of touch entires. */
    TouchEntry& operator[] (const int index) { return touches.getReference (index); }

    /** Resets all contents, doest not generate any call-backs. */
    void clear() noexcept                   { touches.clear(); }

private:
    //==========================================================================
    static bool matches (const TouchSurface::Touch& t1,
                         const TouchSurface::Touch& t2) noexcept
    {
        return t1.index == t2.index && t1.blockUID == t2.blockUID;
    }

    juce::Array<TouchEntry> touches;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TouchList)
};
