/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

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
namespace BlocksProtocol
{

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
    firmwareUpdateACK       = 0x03,
    deviceTopologyExtend    = 0x04,
    deviceTopologyEnd       = 0x05,
    deviceVersion           = 0x06,
    deviceName              = 0x07,

    touchStart              = 0x10,
    touchMove               = 0x11,
    touchEnd                = 0x12,

    touchStartWithVelocity  = 0x13,
    touchMoveWithVelocity   = 0x14,
    touchEndWithVelocity    = 0x15,

    configMessage           = 0x18,

    controlButtonDown       = 0x20,
    controlButtonUp         = 0x21,

    programEventMessage     = 0x28,

    logMessage              = 0x30
};

/** Messages that the host may send to a device. */
enum class MessageFromHost
{
    deviceCommandMessage    = 0x01,
    sharedDataChange        = 0x02,
    programEventMessage     = 0x03,
    firmwareUpdatePacket    = 0x04,

    configMessage           = 0x10,
    factoryReset            = 0x11,
    blockReset              = 0x12,

    setName                 = 0x20
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
/** Structure describing a block's serial number

    @tags{Blocks}
*/
struct BlockSerialNumber
{
    uint8 serial[16];

    bool isValid() const noexcept
    {
        for (auto c : serial)
            if (c == 0)
                return false;

        return isAnyControlBlock() || isPadBlock() || isSeaboardBlock();
    }

    bool isPadBlock() const noexcept            { return hasPrefix ("LPB") || hasPrefix ("LPM"); }
    bool isLiveBlock() const noexcept           { return hasPrefix ("LIC"); }
    bool isLoopBlock() const noexcept           { return hasPrefix ("LOC"); }
    bool isDevCtrlBlock() const noexcept        { return hasPrefix ("DCB"); }
    bool isTouchBlock() const noexcept          { return hasPrefix ("TCB"); }
    bool isSeaboardBlock() const noexcept       { return hasPrefix ("SBB"); }

    bool isAnyControlBlock() const noexcept     { return isLiveBlock() || isLoopBlock() || isDevCtrlBlock() || isTouchBlock(); }

    bool hasPrefix (const char* prefix) const noexcept  { return memcmp (serial, prefix, 3) == 0; }
};

//==============================================================================
/** Structure for generic block data

    @tags{Blocks}
 */
template <size_t MaxSize>
struct BlockStringData
{
    uint8 data[MaxSize] = {};
    uint8 length = 0;

    static const size_t maxLength { MaxSize };

    bool isNotEmpty() const
    {
        return length > 0;
    }

    bool operator== (const BlockStringData& other) const
    {
        if (length != other.length)
            return false;

        for (int i = 0; i < length; ++i)
            if (data[i] != other.data[i])
                return false;

        return true;
    }

    bool operator!= (const BlockStringData& other) const
    {
        return ! ( *this == other );
    }
};

using VersionNumber = BlockStringData<21>;
using BlockName = BlockStringData<33>;

//==============================================================================
/** Structure for the device status

    @tags{Blocks}
*/
struct DeviceStatus
{
    BlockSerialNumber serialNumber;
    TopologyIndex index;
    BatteryLevel batteryLevel;
    BatteryCharging batteryCharging;
};

//==============================================================================
/** Structure for the device connection

    @tags{Blocks}
*/
struct DeviceConnection
{
    TopologyIndex device1, device2;
    ConnectorPort port1, port2;
};

//==============================================================================
/** Structure for the device version

    @tags{Blocks}
*/
struct DeviceVersion
{
    TopologyIndex index;
    VersionNumber version;
};

//==============================================================================
/** Structure used for the device name

    @tags{Blocks}
*/
struct DeviceName
{
    TopologyIndex index;
    BlockName name;
};

static constexpr uint8 maxBlocksInTopologyPacket = 6;
static constexpr uint8 maxConnectionsInTopologyPacket = 24;

//==============================================================================
/** Configuration Item Identifiers. */
enum ConfigItemId
{
    // MIDI
    midiStartChannel    = 0,
    midiEndChannel      = 1,
    midiUseMPE          = 2,
    pitchBendRange      = 3,
    octave              = 4,
    transpose           = 5,
    slideCC             = 6,
    slideMode           = 7,
    octaveTopology      = 8,
    midiChannelRange    = 9,
    MPEZone             = 40,
    // Touch
    velocitySensitivity = 10,
    glideSensitivity    = 11,
    slideSensitivity    = 12,
    pressureSensitivity = 13,
    liftSensitivity     = 14,
    fixedVelocity       = 15,
    fixedVelocityValue  = 16,
    pianoMode           = 17,
    glideLock           = 18,
    glideLockEnable     = 19,
    // Live
    mode                = 20,
    volume              = 21,
    scale               = 22,
    hideMode            = 23,
    chord               = 24,
    arpPattern          = 25,
    tempo               = 26,
    // Tracking
    xTrackingMode       = 30,
    yTrackingMode       = 31,
    zTrackingMode       = 32,
    // Graphics
    gammaCorrection     = 33,
    // User
    user0               = 64,
    user1               = 65,
    user2               = 66,
    user3               = 67,
    user4               = 68,
    user5               = 69,
    user6               = 70,
    user7               = 71,
    user8               = 72,
    user9               = 73,
    user10              = 74,
    user11              = 75,
    user12              = 76,
    user13              = 77,
    user14              = 78,
    user15              = 79,
    user16              = 80,
    user17              = 81,
    user18              = 82,
    user19              = 83,
    user20              = 84,
    user21              = 85,
    user22              = 86,
    user23              = 87,
    user24              = 88,
    user25              = 89,
    user26              = 90,
    user27              = 91,
    user28              = 92,
    user29              = 93,
    user30              = 94,
    user31              = 95
};

static constexpr uint8 numberOfUserConfigs = 32;
static constexpr uint8 maxConfigIndex = uint8 (ConfigItemId::user0) + numberOfUserConfigs;

static constexpr uint8 configUserConfigNameLength = 32;
static constexpr uint8 configMaxOptions = 8;
static constexpr uint8 configOptionNameLength = 16;

//==============================================================================
/** The coordinates of a touch.

    @tags{Blocks}
*/
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

/** The velocities for each dimension of a touch.

    @tags{Blocks}
*/
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
    debugMode                   = 0x04,
    saveProgramAsDefault        = 0x05
};

using DeviceCommand = IntegerWithBitSize<9>;

//==============================================================================
enum ConfigCommands
{
    setConfig                   = 0x00,
    requestConfig               = 0x01, // Request a config update
    requestFactorySync          = 0x02, // Requests all active factory config data
    requestUserSync             = 0x03, // Requests all active user config data
    updateConfig                = 0x04, // Set value, min and max
    updateUserConfig            = 0x05, // As above but contains user config metadata
    setConfigState              = 0x06, // Set config activation state and whether it is saved in flash
    factorySyncEnd              = 0x07,
    clusterConfigSync           = 0x08,
    factorySyncReset            = 0x09
};

using ConfigCommand = IntegerWithBitSize<4>;
using ConfigItemIndex = IntegerWithBitSize<8>;
using ConfigItemValue = IntegerWithBitSize<32>;

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

using FirmwareUpdateACKCode    = IntegerWithBitSize<7>;
using FirmwareUpdateACKDetail  = IntegerWithBitSize<32>;
using FirmwareUpdatePacketSize = IntegerWithBitSize<7>;

static constexpr uint32 numProgramMessageInts = 3;

static constexpr uint32 apiModeHostPingTimeoutMs = 5000;

static constexpr uint32 padBlockProgramAndHeapSize = 7200;
static constexpr uint32 padBlockStackSize = 800;

static constexpr uint32 controlBlockProgramAndHeapSize = 3000;
static constexpr uint32 controlBlockStackSize = 800;


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

    firmwareUpdateACK        = MessageType::bits + FirmwareUpdateACKCode::bits + FirmwareUpdateACKDetail::bits,

    controlButtonMessage     = typeDeviceAndTime + ControlButtonID::bits,

    configSetMessage         = MessageType::bits + ConfigCommand::bits + ConfigItemIndex::bits + ConfigItemValue::bits,
    configRespMessage        = MessageType::bits + ConfigCommand::bits + ConfigItemIndex::bits + (ConfigItemValue::bits * 3),
    configSyncEndMessage     = MessageType::bits + ConfigCommand::bits,
};

//==============================================================================
// These are the littlefoot functions provided for use in BLOCKS programs
static constexpr const char* ledProgramLittleFootFunctions[] =
{
    "min/iii",
    "min/fff",
    "max/iii",
    "max/fff",
    "clamp/iiii",
    "clamp/ffff",
    "abs/ii",
    "abs/ff",
    "map/ffffff",
    "map/ffff",
    "mod/iii",
    "getRandomFloat/f",
    "getRandomInt/ii",
    "log/vi",
    "logHex/vi",
    "getMillisecondCounter/i",
    "getFirmwareVersion/i",
    "getTimeInCurrentFunctionCall/i",
    "getBatteryLevel/f",
    "isBatteryCharging/b",
    "isMasterBlock/b",
    "isConnectedToHost/b",
    "setStatusOverlayActive/vb",
    "getNumBlocksInTopology/i",
    "getBlockIDForIndex/ii",
    "getBlockIDOnPort/ii",
    "getPortToMaster/i",
    "getBlockTypeForID/ii",
    "sendMessageToBlock/viiii",
    "sendMessageToHost/viii",
    "getHorizontalDistFromMaster/i",
    "getVerticalDistFromMaster/i",
    "getAngleFromMaster/i",
    "setAutoRotate/vb",
    "getClusterIndex/i",
    "getClusterWidth/i",
    "getClusterHeight/i",
    "getClusterXpos/i",
    "getClusterYpos/i",
    "getNumBlocksInCurrentCluster/i",
    "getBlockIdForBlockInCluster/ii",
    "isMasterInCurrentCluster/b",
    "setClusteringActive/vb",
    "makeARGB/iiiii",
    "blendARGB/iii",
    "fillPixel/viii",
    "blendPixel/viii",
    "fillRect/viiiii",
    "blendRect/viiiii",
    "blendGradientRect/viiiiiiii",
    "blendCircle/vifffb",
    "addPressurePoint/vifff",
    "drawPressureMap/v",
    "fadePressureMap/v",
    "drawNumber/viiii",
    "clearDisplay/v",
    "clearDisplay/vi",
    "displayBatteryLevel/v",
    "sendMIDI/vi",
    "sendMIDI/vii",
    "sendMIDI/viii",
    "sendNoteOn/viii",
    "sendNoteOff/viii",
    "sendAftertouch/viii",
    "sendCC/viii",
    "sendPitchBend/vii",
    "sendPitchBend/viii",
    "sendChannelPressure/vii",
    "addPitchCorrectionPad/viiffff",
    "setPitchCorrectionEnabled/vb",
    "getPitchCorrectionPitchBend/iii",
    "setChannelRange/vbii",
    "assignChannel/ii",
    "deassignChannel/vii",
    "getControlChannel/i",
    "useMPEDuplicateFilter/vb",
    "getSensorValue/iii",
    "handleTouchAsSeaboard/vi",
    "setPowerSavingEnabled/vb",
    "getLocalConfig/ii",
    "setLocalConfig/vii",
    "requestRemoteConfig/vii",
    "setRemoteConfig/viii",
    "setLocalConfigItemRange/viii",
    "setLocalConfigActiveState/vibb",
    "linkBlockIDtoController/vi",
    "repaintControl/v",
    "onControlPress/vi",
    "onControlRelease/vi",
    "initControl/viiiiiiiii",
    "setButtonMode/vii",
    "setButtonType/viii",
    "setButtonMinMaxDefault/viiii",
    "setButtonColours/viii",
    "setButtonTriState/vii",
    nullptr
};

} // namespace BlocksProtocol
} // namespace juce
