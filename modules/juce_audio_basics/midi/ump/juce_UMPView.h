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

/** @cond */
namespace juce::universal_midi_packets
{

/**
    Points to a single Universal MIDI Packet.

    The packet must be well-formed for member functions to work correctly.

    Specifically, the constructor argument must be the beginning of a region of
    uint32_t that contains at least `getNumWordsForMessageType (*data)` items,
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

} // namespace juce::universal_midi_packets
/** @endcond */
