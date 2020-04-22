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

// This file provides interfaces for managing the internal configuration of Blocks
// and synchronises with the connected Block

using namespace BlocksProtocol;

using ConfigType = Block::ConfigMetaData::ConfigType;

/** Manages the configuration of blocks

    @tags{Blocks}
*/
struct BlockConfigManager
{
    /** Structure describing a configuration */
    struct ConfigDescription
    {
        ConfigItemId item;
        int32 value;
        int32 min;
        int32 max;
        bool isActive;
        const char* name;
        ConfigType type;
        const char* optionNames[configMaxOptions];
        const char* group;

        static_assert (configMaxOptions == Block::ConfigMetaData::numOptionNames, "Config options size and config metadata size should be the same");

        Block::ConfigMetaData toConfigMetaData() const
        {
            return Block::ConfigMetaData ((uint32) item, value, { min, max }, isActive, name, type, (const char**) optionNames, group);
        }
    };

    BlockConfigManager (Array<ConfigDescription> defaultConfig)
    {
        for (auto c : defaultConfig)
        {
            uint32 itemIndex;

            if (getIndexForItem (c.item, itemIndex))
                configList[itemIndex] = c;
        }
    }

    void setDeviceIndex (TopologyIndex newDeviceIndex)                       { deviceIndex = newDeviceIndex; }
    void setDeviceComms (PhysicalTopologySource::DeviceConnection* newConn)  { deviceConnection = newConn; }

    static constexpr uint32 numConfigItems = 69;

    static constexpr const char* midiSettingsGroup = "MIDI Settings";
    static constexpr const char* pitchGroup = "Pitch";
    static constexpr const char* playGroup = "Play mode";
    static constexpr const char* sensitivityGroup = "Sensitivity";
    static constexpr const char* rhythmGroup = "Rhythm";
    static constexpr const char* coloursGroup = "Colors";

    ConfigDescription configList[numConfigItems] =
    {
        { midiStartChannel,     2,      1,      16,         false,  "MIDI Start Channel",   ConfigType::integer,    {},               midiSettingsGroup },
        { midiEndChannel,       16,     1,      16,         false,  "MIDI End Channel",     ConfigType::integer,    {},               midiSettingsGroup },
        { midiUseMPE,           1,      0,      2,          false,  "MIDI Mode",            ConfigType::options,    { "Multi Channel",
                                                                                                                  "MPE",
                                                                                                                  "Single Channel" }, midiSettingsGroup },
        { pitchBendRange,       48,     1,      96,         false,  "Pitch Bend Range",     ConfigType::integer,    {},               midiSettingsGroup },
        { midiChannelRange,     15,     1,      15,         false,  "No. MIDI Channels",    ConfigType::integer,    {},               midiSettingsGroup },
        { MPEZone,              0,      0,      1,          false,  "MPE Zone",             ConfigType::options,    { "Lower Zone",
                                                                                                                  "Upper Zone"},      midiSettingsGroup },
        { octave,               0,      -4,     6,          false,  "Octave",               ConfigType::integer,    {},               pitchGroup },
        { transpose,            0,      -11,    11,         false,  "Transpose",            ConfigType::integer,    {},               pitchGroup },
        { slideCC,              74,     0,      127,        false,  "Slide CC",             ConfigType::integer,    {},               playGroup },
        { slideMode,            0,      0,      2,          false,  "Slide Mode",           ConfigType::options,    { "Absolute",
                                                                                                                  "Relative Unipolar",
                                                                                                                  "Relative Bipolar" }, playGroup },
        { velocitySensitivity,  100,    0,      127,        false,  "Strike Sensitivity",   ConfigType::integer,    {},               sensitivityGroup },
        { glideSensitivity,     100,    0,      127,        false,  "Glide Sensitivity",    ConfigType::integer,    {},               sensitivityGroup },
        { slideSensitivity,     100,    0,      127,        false,  "Slide Sensitivity",    ConfigType::integer,    {},               sensitivityGroup },
        { pressureSensitivity,  100,    0,      127,        false,  "Pressure Sensitivity", ConfigType::integer,    {},               sensitivityGroup },
        { liftSensitivity,      100,    0,      127,        false,  "Lift Sensitivity",     ConfigType::integer,    {},               sensitivityGroup },
        { fixedVelocity,        0,      0,      1,          false,  "Fixed Velocity",       ConfigType::boolean,    {},               sensitivityGroup },
        { fixedVelocityValue,   127,    1,      127,        false,  "Fixed Velocity Value", ConfigType::integer,    {},               sensitivityGroup },
        { pianoMode,            0,      0,      1,          false,  "Piano Mode",           ConfigType::boolean,    {},               playGroup },
        { glideLock,            0,      0,      127,        false,  "Glide Rate",           ConfigType::integer,    {},               playGroup },
        { glideLockEnable,      0,      0,      1,          false,  "Glide Lock Enable",    ConfigType::boolean,    {},               playGroup },
        { mode,                 4,      1,      5,          false,  "Mode",                 ConfigType::integer,    {},               playGroup },
        { volume,               100,    0,      127,        false,  "Volume",               ConfigType::integer,    {},               playGroup },
        { scale,                0,      0,      18,         false,  "Scale",                ConfigType::integer,    {},               playGroup }, // NOTE: Should be options
        { hideMode,             0,      0,      1,          false,  "Hide Mode",            ConfigType::boolean,    {},               playGroup },
        { chord,                0,      0,      127,        false,  "Chord",                ConfigType::integer,    {},               playGroup }, // NOTE: Should be options
        { arpPattern,           0,      0,      127,        false,  "Arp Pattern",          ConfigType::integer,    {},               playGroup },
        { tempo,                120,    1,      300,        false,  "Tempo",                ConfigType::integer,    {},               rhythmGroup },
        { key,                  0,      0,      11,         false,  "Key",                  ConfigType::options,    { "C", "C#", "D", "D#",
                                                                                                                  "E", "F", "F#", "G",
                                                                                                                  "G#", "A", "A#", "B"}, playGroup },
        { autoTransposeToKey,   0,      0,      1,          false,  "Auto Transpose To Key",ConfigType::boolean,    {},               pitchGroup },
        { xTrackingMode,        1,      1,      4,          false,  "Glide Tracking Mode",  ConfigType::options,    { "Multi-Channel",
                                                                                                                  "Last Played",
                                                                                                                  "Highest",
                                                                                                                  "Lowest",
                                                                                                                  "Disabled" },   playGroup },
        { yTrackingMode,        1,      1,      4,          false,  "Slide Tracking Mode",  ConfigType::options,    { "Multi-Channel",
                                                                                                                  "Last Played",
                                                                                                                  "Highest",
                                                                                                                  "Lowest",
                                                                                                                  "Disabled" },   playGroup },
        { zTrackingMode,        1,      0,      4,          false,  "Pressure Tracking Mode", ConfigType::options, { "Poly Aftertouch",
                                                                                                                  "Last Played",
                                                                                                                  "Highest",
                                                                                                                  "Lowest",
                                                                                                                  "Disabled",
                                                                                                                  "Hardest" },    playGroup },

        { gammaCorrection,      0,         0,         1,    false,  "Gamma Correction",     ConfigType::boolean,    {},             coloursGroup },
        { globalKeyColour, INT32_MIN, INT32_MIN, INT32_MAX, false,  "Global Key Color",     ConfigType::colour,     {},             coloursGroup },
        { rootKeyColour,   INT32_MIN, INT32_MIN, INT32_MAX, false,  "Root Key Color"  ,     ConfigType::colour,     {},             coloursGroup },
        { brightness,           100,    0,    100,          false,  "Brightness",           ConfigType::integer,    {},             coloursGroup },

        // These can be defined for unique usage for a given Littlefoot script
        { user0,                0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user1,                0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user2,                0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user3,                0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user4,                0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user5,                0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user6,                0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user7,                0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user8,                0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user9,                0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user10,               0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user11,               0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user12,               0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user13,               0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user14,               0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user15,               0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user16,               0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user17,               0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user18,               0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user19,               0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user20,               0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user21,               0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user22,               0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user23,               0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user24,               0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user25,               0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user26,               0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user27,               0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user28,               0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user29,               0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user30,               0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} },
        { user31,               0,    0,      127,          false,  {},                     ConfigType::integer,    {},               {} }
    };

    //==============================================================================
    int32 getItemValue (ConfigItemId item)
    {
        uint32 itemIndex;

        if (getIndexForItem (item, itemIndex))
            return configList[itemIndex].value;

        return 0;
    }

    void setItemValue (ConfigItemId item, int32 value)
    {
        uint32 itemIndex;

        if (getIndexForItem (item, itemIndex))
            configList[itemIndex].value = value;

        setBlockConfig (item, value);
    }

    int32 getItemMin (ConfigItemId item)
    {
        uint32 itemIndex;

        if (getIndexForItem (item, itemIndex))
            return configList[itemIndex].min;

        return 0;
    }

    void setItemMin (ConfigItemId item, int32 min)
    {
        uint32 itemIndex;

        if (getIndexForItem (item, itemIndex))
            configList[itemIndex].min = min;
    }

    int32 getItemMax (ConfigItemId item)
    {
        uint32 itemIndex;

        if (getIndexForItem (item, itemIndex))
            return configList[itemIndex].max;

        return 0;
    }

    void setItemMax (ConfigItemId item, int32 max)
    {
        uint32 itemIndex;

        if (getIndexForItem (item, itemIndex))
            configList[itemIndex].max = max;

        // Send updateConfig message to Block
    }

    bool getItemActive (ConfigItemId item)
    {
        uint32 itemIndex;

        if (getIndexForItem (item, itemIndex))
            return configList[itemIndex].isActive;

        return false;
    }

    void setItemActive (ConfigItemId item, bool isActive)
    {
        uint32 itemIndex;

        if (getIndexForItem (item, itemIndex))
            configList[itemIndex].isActive = isActive;

        // Send setConfigState message to Block
    }

    String getOptionName (ConfigItemId item, uint8 optionIndex)
    {
        uint32 itemIndex;

        if (getIndexForItem (item, itemIndex) && optionIndex < configMaxOptions)
            return configList[itemIndex].optionNames[optionIndex];

        return {};
    }

    Block::ConfigMetaData getMetaData (ConfigItemId item)
    {
        uint32 itemIndex;

        if (getIndexForItem (item, itemIndex))
            return configList[itemIndex].toConfigMetaData();

        return { static_cast<juce::uint32> (item) };
    }

    void resetConfigListActiveStatus()
    {
        for (auto& i : configList)
            i.isActive = false;
    }

    //==============================================================================
    // Set Block Configuration
    void setBlockConfig (ConfigItemId item, int32 value)
    {
        buildAndSendPacket ([item, value] (HostPacketBuilder<32>& p) { p.addConfigSetMessage (item, value); });
    }

    void requestBlockConfig (ConfigItemId item)
    {
        buildAndSendPacket ([item] (HostPacketBuilder<32>& p) { p.addRequestMessage (item); });
    }

    void requestFactoryConfigSync()
    {
        buildAndSendPacket ([] (HostPacketBuilder<32>& p) { p.addRequestFactorySyncMessage(); });
    }

    void requestUserConfigSync()
    {
        buildAndSendPacket ([] (HostPacketBuilder<32>& p) { p.addRequestUserSyncMessage(); });
    }

    void handleConfigUpdateMessage (int32 item, int32 value, int32 min, int32 max)
    {
        uint32 index;

        if (getIndexForItem ((ConfigItemId) item, index))
        {
            configList[index].value = value;
            configList[index].min = min;
            configList[index].max = max;
            configList[index].isActive = true;
        }
    }

    void handleConfigSetMessage(int32 item, int32 value)
    {
        uint32 index;

        if (getIndexForItem ((ConfigItemId) item, index))
            configList[index].value = value;
    }

private:
    bool getIndexForItem (ConfigItemId item, uint32& index)
    {
        for (uint32 i = 0; i < numConfigItems; ++i)
        {
            if (configList[i].item == item)
            {
                index = i;
                return true;
            }
        }

        return false;
    }

    template<typename PacketBuildFn>
    void buildAndSendPacket (PacketBuildFn buildFn)
    {
        if (deviceConnection == nullptr)
            return;

        HostPacketBuilder<32> packet;
        packet.writePacketSysexHeaderBytes (deviceIndex);
        buildFn (packet);
        packet.writePacketSysexFooter();
        deviceConnection->sendMessageToDevice (packet.getData(), (size_t) packet.size());
    }

    TopologyIndex deviceIndex {};
    PhysicalTopologySource::DeviceConnection* deviceConnection {};
};

} // namespace juce
