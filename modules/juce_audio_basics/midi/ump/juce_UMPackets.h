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

#ifndef DOXYGEN

namespace juce
{
namespace universal_midi_packets
{

/**
    Holds a collection of Universal MIDI Packets.

    Unlike MidiBuffer, this collection does not store any additional information
    (e.g. timestamps) alongside the raw messages.

    If timestamps are required, these can be added to the container in UMP format,
    as Jitter Reduction Utility messages.

    @tags{Audio}
*/
class Packets
{
public:
    /** Adds a single packet to the collection.

        The View must be valid for this to work. If the view
        points to a malformed message, or if the view points to a region
        too short for the contained message, this call will result in
        undefined behaviour.
    */
    void add (const View& v) { storage.insert (storage.end(), v.cbegin(), v.cend()); }

    void add (const PacketX1& p) { addImpl (p); }
    void add (const PacketX2& p) { addImpl (p); }
    void add (const PacketX3& p) { addImpl (p); }
    void add (const PacketX4& p) { addImpl (p); }

    /** Pre-allocates space for at least `numWords` 32-bit words in this collection. */
    void reserve (size_t numWords)          { storage.reserve (numWords); }

    /** Removes all previously-added packets from this collection. */
    void clear()                            { storage.clear(); }

    /** Gets an iterator pointing to the first packet in this collection. */
    Iterator cbegin() const noexcept     { return Iterator (data(), size()); }
    Iterator begin() const noexcept      { return cbegin(); }

    /** Gets an iterator pointing one-past the last packet in this collection. */
    Iterator cend() const noexcept       { return Iterator (data() + size(), 0); }
    Iterator end() const noexcept        { return cend(); }

    /** Gets a pointer to the contents of the collection as a range of raw 32-bit words. */
    const uint32_t* data() const noexcept   { return storage.data(); }

    /** Returns the number of uint32_t words in storage.

        Note that this is likely to be larger than the number of packets
        currently being stored, as some packets span multiple words.
    */
    size_t size() const noexcept            { return storage.size(); }

private:
    template <size_t numWords>
    void addImpl (const Packet<numWords>& p)
    {
        jassert (Utils::getNumWordsForMessageType (p[0]) == numWords);
        add (View (p.data()));
    }

    std::vector<uint32_t> storage;
};

}
}

#endif
