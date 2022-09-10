/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

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

#ifndef DOXYGEN

namespace juce
{
namespace universal_midi_packets
{

/**
    Points to a single Universal MIDI Packet.

    The packet must be well-formed for member functions to work correctly.

    Specifically, the constructor argument must be the beginning of a region of
    uint32_t that contains at least `getNumWordsForMessageType(*data)` items,
    where `data` is the constructor argument.

    NOTE: Instances of this class do not own the memory that they point to!
    If you need to store a packet pointed-to by a View for later use, copy
    the view contents to a Packets collection, or use the Utils::PacketX types.

    @tags{Audio}
*/
class View
{
public:
    /** Create an invalid view. */
    View() noexcept = default;

    /** Create a view of the packet starting at address `d`. */
    explicit View (const uint32_t* data) noexcept : ptr (data) {}

    /** Get a pointer to the first word in the Universal MIDI Packet currently
        pointed-to by this view.
    */
    const uint32_t* data() const noexcept { return ptr; }

    /** Get the number of 32-words (between 1 and 4 inclusive) in the Universal
        MIDI Packet currently pointed-to by this view.
    */
    uint32_t size() const noexcept;

    /** Get a specific word from this packet.

        Passing an `index` that is greater than or equal to the result of `size`
        will cause undefined behaviour.
    */
    const uint32_t& operator[] (size_t index) const noexcept { return ptr[index]; }

    /** Get an iterator pointing to the first word in the packet. */
    const uint32_t* begin() const noexcept { return ptr; }
    const uint32_t* cbegin() const noexcept { return ptr; }

    /** Get an iterator pointing one-past the last word in the packet. */
    const uint32_t* end() const noexcept { return ptr + size(); }
    const uint32_t* cend() const noexcept { return ptr + size(); }

    /** Return true if this view is pointing to the same address as another view. */
    bool operator== (const View& other) const noexcept { return ptr == other.ptr; }

    /** Return false if this view is pointing to the same address as another view. */
    bool operator!= (const View& other) const noexcept { return ! operator== (other); }

private:
    const uint32_t* ptr = nullptr;
};

}
}

#endif
