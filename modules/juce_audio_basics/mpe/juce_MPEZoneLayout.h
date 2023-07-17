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

namespace juce
{

//==============================================================================
/**
    This struct represents an MPE zone.

    It can either be a lower or an upper zone, where:
      - A lower zone encompasses master channel 1 and an arbitrary number of ascending
        MIDI channels, increasing from channel 2.
      - An upper zone encompasses master channel 16 and an arbitrary number of descending
        MIDI channels, decreasing from channel 15.

    It also defines a pitchbend range (in semitones) to be applied for per-note pitchbends and
    master pitchbends, respectively.

    @tags{Audio}
*/
struct MPEZone
{
    enum class Type { lower, upper };

    MPEZone() = default;

    MPEZone (Type type, int memberChannels = 0, int perNotePitchbend = 48, int masterPitchbend = 2)
        : zoneType (type),
          numMemberChannels (memberChannels),
          perNotePitchbendRange (perNotePitchbend),
          masterPitchbendRange (masterPitchbend)
    {}

    bool isLowerZone() const noexcept             { return zoneType == Type::lower; }
    bool isUpperZone() const noexcept             { return zoneType == Type::upper; }

    bool isActive() const noexcept                { return numMemberChannels > 0; }

    int getMasterChannel() const noexcept         { return isLowerZone() ? lowerZoneMasterChannel : upperZoneMasterChannel; }
    int getFirstMemberChannel() const noexcept    { return isLowerZone() ? lowerZoneMasterChannel + 1 : upperZoneMasterChannel - 1; }
    int getLastMemberChannel() const noexcept     { return isLowerZone() ? (lowerZoneMasterChannel + numMemberChannels)
                                                                         : (upperZoneMasterChannel - numMemberChannels); }

    bool isUsingChannelAsMemberChannel (int channel) const noexcept
    {
        return isLowerZone() ? (lowerZoneMasterChannel < channel && channel <= getLastMemberChannel())
                             : (channel < upperZoneMasterChannel && getLastMemberChannel() <= channel);
    }

    bool isUsing (int channel) const noexcept
    {
        return isUsingChannelAsMemberChannel (channel) || channel == getMasterChannel();
    }

    static auto tie (const MPEZone& z)
    {
        return std::tie (z.zoneType,
                         z.numMemberChannels,
                         z.perNotePitchbendRange,
                         z.masterPitchbendRange);
    }

    bool operator== (const MPEZone& other) const
    {
        return tie (*this) == tie (other);
    }

    bool operator!= (const MPEZone& other) const
    {
        return tie (*this) != tie (other);
    }

    //==============================================================================
    static constexpr int lowerZoneMasterChannel = 1,
                         upperZoneMasterChannel = 16;

    Type zoneType = Type::lower;

    int numMemberChannels     = 0;
    int perNotePitchbendRange = 48;
    int masterPitchbendRange  = 2;
};

//==============================================================================
/**
    This class represents the current MPE zone layout of a device capable of handling MPE.

    An MPE device can have up to two zones: a lower zone with master channel 1 and
    allocated MIDI channels increasing from channel 2, and an upper zone with master
    channel 16 and allocated MIDI channels decreasing from channel 15. MPE mode is
    enabled on a device when one of these zones is active and disabled when both
    are inactive.

    Use the MPEMessages helper class to convert the zone layout represented
    by this object to MIDI message sequences that you can send to an Expressive
    MIDI device to set its zone layout, add zones etc.

    @see MPEInstrument

    @tags{Audio}
*/
class JUCE_API  MPEZoneLayout
{
public:
    //==============================================================================
    /** Creates a layout with inactive upper and lower zones. */
    MPEZoneLayout() = default;

    /** Creates a layout with the given upper and lower zones. */
    MPEZoneLayout (MPEZone lower, MPEZone upper);

    /** Creates a layout with a single upper or lower zone, leaving the other zone uninitialised. */
    MPEZoneLayout (MPEZone singleZone);

    MPEZoneLayout (const MPEZoneLayout& other);
    MPEZoneLayout& operator= (const MPEZoneLayout& other);

    bool operator== (const MPEZoneLayout& other) const { return lowerZone == other.lowerZone && upperZone == other.upperZone; }
    bool operator!= (const MPEZoneLayout& other) const { return ! operator== (other); }

    //==============================================================================
    /** Returns a struct representing the lower MPE zone. */
    MPEZone getLowerZone() const noexcept    { return lowerZone; }

    /** Returns a struct representing the upper MPE zone. */
    MPEZone getUpperZone() const noexcept    { return upperZone; }

    /** Sets the lower zone of this layout. */
    void setLowerZone (int numMemberChannels = 0,
                       int perNotePitchbendRange = 48,
                       int masterPitchbendRange = 2) noexcept;

    /** Sets the upper zone of this layout. */
    void setUpperZone (int numMemberChannels = 0,
                       int perNotePitchbendRange = 48,
                       int masterPitchbendRange = 2) noexcept;

    /** Clears the lower and upper zones of this layout, making them both inactive
        and disabling MPE mode.
    */
    void clearAllZones();

    /** Returns true if either of the zones are active. */
    bool isActive() const  { return lowerZone.isActive() || upperZone.isActive(); }

    //==============================================================================
    /** Pass incoming MIDI messages to an object of this class if you want the
        zone layout to properly react to MPE RPN messages like an
        MPE device.

        MPEMessages::rpnNumber will add or remove zones; RPN 0 will
        set the per-note or master pitchbend ranges.

        Any other MIDI messages will be ignored by this class.

        @see MPEMessages
    */
    void processNextMidiEvent (const MidiMessage& message);

    /** Pass incoming MIDI buffers to an object of this class if you want the
        zone layout to properly react to MPE RPN messages like an
        MPE device.

        MPEMessages::rpnNumber will add or remove zones; RPN 0 will
        set the per-note or master pitchbend ranges.

        Any other MIDI messages will be ignored by this class.

        @see MPEMessages
     */
    void processNextMidiBuffer (const MidiBuffer& buffer);

    //==============================================================================
    /** Listener class. Derive from this class to allow your class to be
        notified about changes to the zone layout.
    */
    class Listener
    {
    public:
        /** Destructor. */
        virtual ~Listener() = default;

        /** Implement this callback to be notified about any changes to this
            MPEZoneLayout. Will be called whenever a zone is added, zones are
            removed, or any zone's master or note pitchbend ranges change.
        */
        virtual void zoneLayoutChanged (const MPEZoneLayout& layout) = 0;
    };

    //==============================================================================
    /** Adds a listener. */
    void addListener (Listener* const listenerToAdd) noexcept;

    /** Removes a listener. */
    void removeListener (Listener* const listenerToRemove) noexcept;

   #ifndef DOXYGEN
    using Zone = MPEZone;
   #endif

private:
    //==============================================================================
    MPEZone lowerZone { MPEZone::Type::lower, 0 };
    MPEZone upperZone { MPEZone::Type::upper, 0 };

    MidiRPNDetector rpnDetector;
    ListenerList<Listener> listeners;

    //==============================================================================
    void setZone (bool, int, int, int) noexcept;

    void processRpnMessage (MidiRPNMessage);
    void processZoneLayoutRpnMessage (MidiRPNMessage);
    void processPitchbendRangeRpnMessage (MidiRPNMessage);

    void updateMasterPitchbend (MPEZone&, int);
    void updatePerNotePitchbendRange (MPEZone&, int);

    void sendLayoutChangeMessage();
    void checkAndLimitZoneParameters (int, int, int&) noexcept;
};

} // namespace juce
