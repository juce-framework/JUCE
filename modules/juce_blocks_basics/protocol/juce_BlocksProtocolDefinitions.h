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


/** This value is incremented when the format of the API changes in a way which
    breaks compatibility.
*/
static constexpr uint32 currentProtocolVersion = 1;

using ProtocolVersion = IntegerWithBitSize<8>;

//==============================================================================
/** A timestamp for a packet, in milliseconds since device boot-up */
using PacketTimestamp = IntegerWithBitSize<32>;

/** This relative timestamp is for use inside a packet, and it represents a
    number of milliseconds that should be added to the packet's timestamp.
*/
using PacketTimestampOffset = IntegerWithBitSize<5>;


//==============================================================================
/** Messages that a device may send to the host. */
enum class MessageFromDevice
{
    deviceTopology          = 0x01,
    packetACK               = 0x02,

    touchStart              = 0x10,
    touchMove               = 0x11,
    touchEnd                = 0x12,

    touchStartWithVelocity  = 0x13,
    touchMoveWithVelocity   = 0x14,
    touchEndWithVelocity    = 0x15,

    controlButtonDown       = 0x20,
    controlButtonUp         = 0x21
};

/** Messages that the host may send to a device. */
enum class MessageFromHost
{
    deviceCommandMessage    = 0x01,
    sharedDataChange        = 0x02,
    programEventMessage     = 0x03
};


/** This is the first item in a BLOCKS message, identifying the message type. */
using MessageType = IntegerWithBitSize<7>;

//==============================================================================
/** This is a type of index identifier used to refer to a block within a group.
    It refers to the index of a device in the list of devices that was most recently
    sent via a topology change message
    (It's not a global UID for a block unit).
    NB: to send a message to all devices, pass the getDeviceIndexForBroadcast() value.
*/
using TopologyIndex     = uint8;

static constexpr int topologyIndexBits = 7;

/** Use this value as the index if you want a message to be sent to all devices in
    the group.
*/
static constexpr TopologyIndex topologyIndexForBroadcast = 63;

using DeviceCount       = IntegerWithBitSize<7>;
using ConnectionCount   = IntegerWithBitSize<8>;

//==============================================================================
/** Battery charge level. */
using BatteryLevel      = IntegerWithBitSize<5>;

/** Battery charger connection flag. */
using BatteryCharging   = IntegerWithBitSize<1>;

//==============================================================================
/** ConnectorPort is an index, starting at 0 for the leftmost port on the
    top edge, and going clockwise.
*/
using ConnectorPort = IntegerWithBitSize<5>;

//==============================================================================
struct BlockSerialNumber
{
    uint8 serial[16];

    bool isValid() const noexcept
    {
        for (auto c : serial)
            if (c == 0)
                return false;

        return isAnyControlBlock() || isPadBlock();
    }

    bool isPadBlock() const noexcept            { return hasPrefix ("LPB"); }
    bool isLiveBlock() const noexcept           { return hasPrefix ("LIC"); }
    bool isLoopBlock() const noexcept           { return hasPrefix ("LOC"); }
    bool isDevCtrlBlock() const noexcept        { return hasPrefix ("DCB"); }

    bool isAnyControlBlock() const noexcept     { return isLiveBlock() || isLoopBlock() || isDevCtrlBlock(); }

    bool hasPrefix (const char* prefix) const noexcept  { return memcmp (serial, prefix, 3) == 0; }
};

struct DeviceStatus
{
    BlockSerialNumber serialNumber;
    TopologyIndex index;
    BatteryLevel batteryLevel;
    BatteryCharging batteryCharging;
};

struct DeviceConnection
{
    TopologyIndex device1, device2;
    ConnectorPort port1, port2;
};


//==============================================================================
/** The coordinates of a touch. */
struct TouchPosition
{
    using Xcoord = IntegerWithBitSize<12>;
    using Ycoord = IntegerWithBitSize<12>;
    using Zcoord = IntegerWithBitSize<8>;

    Xcoord x;
    Ycoord y;
    Zcoord z;

    enum { bits = Xcoord::bits + Ycoord::bits + Zcoord::bits };
};

/** The velocities for each dimension of a touch. */
struct TouchVelocity
{
    using VXcoord = IntegerWithBitSize<8>;
    using VYcoord = IntegerWithBitSize<8>;
    using VZcoord = IntegerWithBitSize<8>;

    VXcoord vx;
    VYcoord vy;
    VZcoord vz;

    enum { bits = VXcoord::bits + VYcoord::bits + VZcoord::bits };
};

/** The index of a touch, i.e. finger number. */
using TouchIndex = IntegerWithBitSize<5>;

using PacketCounter = IntegerWithBitSize<10>;

//==============================================================================
enum DeviceCommands
{
    beginAPIMode                = 0x00,
    requestTopologyMessage      = 0x01,
    endAPIMode                  = 0x02,
    ping                        = 0x03,
    debugMode                   = 0x04
};

using DeviceCommand = IntegerWithBitSize<9>;

//==============================================================================
/** An ID for a control-block button type */
using ControlButtonID = IntegerWithBitSize<12>;

//==============================================================================
using RotaryDialIndex = IntegerWithBitSize<7>;
using RotaryDialAngle = IntegerWithBitSize<14>;
using RotaryDialDelta = IntegerWithBitSize<14>;

//==============================================================================
enum DataChangeCommands
{
    endOfPacket                 = 0,
    endOfChanges                = 1,
    skipBytesFew                = 2,
    skipBytesMany               = 3,
    setSequenceOfBytes          = 4,
    setFewBytesWithValue        = 5,
    setFewBytesWithLastValue    = 6,
    setManyBytesWithValue       = 7
};

using PacketIndex            = IntegerWithBitSize<16>;
using DataChangeCommand      = IntegerWithBitSize<3>;
using ByteCountFew           = IntegerWithBitSize<4>;
using ByteCountMany          = IntegerWithBitSize<8>;
using ByteValue              = IntegerWithBitSize<8>;
using ByteSequenceContinues  = IntegerWithBitSize<1>;

static constexpr uint32 numProgramMessageInts = 2;

static constexpr uint32 apiModeHostPingTimeoutMs = 5000;

static constexpr uint32 padBlockProgramAndHeapSize = 3200;
static constexpr uint32 padBlockStackSize = 800;

static constexpr uint32 controlBlockProgramAndHeapSize = 1500;
static constexpr uint32 controlBlockStackSize = 500;


//==============================================================================
/** Contains the number of bits required to encode various items in the packets */
enum BitSizes
{
    topologyMessageHeader    = MessageType::bits + ProtocolVersion::bits + DeviceCount::bits + ConnectionCount::bits,
    topologyDeviceInfo       = sizeof (BlockSerialNumber) * 7 + BatteryLevel::bits + BatteryCharging::bits,
    topologyConnectionInfo   = topologyIndexBits + ConnectorPort::bits + topologyIndexBits + ConnectorPort::bits,

    typeDeviceAndTime        = MessageType::bits + PacketTimestampOffset::bits,

    touchMessage             = typeDeviceAndTime + TouchIndex::bits + TouchPosition::bits,
    touchMessageWithVelocity = touchMessage + TouchVelocity::bits,

    programEventMessage      = MessageType::bits + 32 * numProgramMessageInts,
    packetACK                = MessageType::bits + PacketCounter::bits,

    controlButtonMessage     = typeDeviceAndTime + ControlButtonID::bits,
};

//==============================================================================
// These are the littlefoot functions provided for use in BLOCKS programs
static constexpr const char* ledProgramLittleFootFunctions[] =
{
    "makeARGB/iiiii",
    "blendARGB/iii",
    "setLED/viii",
    "blendLED/viii",
    "fillRect/viiiii",
    "enableDebug/viii",
    nullptr
};
