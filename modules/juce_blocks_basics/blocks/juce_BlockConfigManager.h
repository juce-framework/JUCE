/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/

namespace juce
{

// This file provides interfaces for managing the internal configuration of Blocks
// and synchronises with the connected Block

using namespace BlocksProtocol;

struct BlockConfigManager
{
    void setDeviceIndex (TopologyIndex newDeviceIndex)                       { deviceIndex = newDeviceIndex; }
    void setDeviceComms (PhysicalTopologySource::DeviceConnection* newConn)  { deviceConnection = newConn; }

    enum ConfigType
    {
        integer,
        floating,
        boolean,
        colour,
        options
    };

    static constexpr uint32 numConfigItems = 61;

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
            return Block::ConfigMetaData ((uint32) item, value, { min, max }, isActive, name, (uint32) type, (const char**) optionNames, group);
        }
    };

    ConfigDescription configList[numConfigItems] =
    {
        { midiStartChannel,     2,      1,      16,     false,  "MIDI Start Channel",   ConfigType::integer,    {},               "MIDI Settings" },
        { midiEndChannel,       16,     1,      16,     false,  "MIDI End Channel",     ConfigType::integer,    {},               "MIDI Settings" },
        { midiUseMPE,           1,      0,      1,      false,  "Use MPE",              ConfigType::boolean,    {},               "MIDI Settings" },
        { pitchBendRange,       48,     1,      96,     false,  "Pitch Bend Range",     ConfigType::integer,    {},               "MIDI Settings" },
        { octave,               0,      -4,     6,      false,  "Octave",               ConfigType::integer,    {},               "Pitch" },
        { transpose,            0,      -11,    11,     false,  "Transpose",            ConfigType::integer,    {},               "Pitch" },
        { slideCC,              74,     0,      127,    false,  "Slide CC",             ConfigType::integer,    {},               "Play mode" },
        { slideMode,            0,      0,      2,      false,  "Slide Mode",           ConfigType::options,    { "Absolute",
                                                                                                                  "Relative Unipolar",
                                                                                                                  "Relative Bipolar" }, "Play mode" },
        { velocitySensitivity,  100,    0,      127,    false,  "Strike Sensitivity",   ConfigType::integer,    {},               "5D Touch" },
        { glideSensitivity,     100,    0,      127,    false,  "Glide Sensitivity",    ConfigType::integer,    {},               "5D Touch" },
        { slideSensitivity,     100,    0,      127,    false,  "Slide Sensitivity",    ConfigType::integer,    {},               "5D Touch" },
        { pressureSensitivity,  100,    0,      127,    false,  "Pressure Sensitivity", ConfigType::integer,    {},               "5D Touch" },
        { liftSensitivity,      100,    0,      127,    false,  "Lift Sensitivity",     ConfigType::integer,    {},               "5D Touch" },
        { fixedVelocity,        0,      0,      1,      false,  "Fixed Velocity",       ConfigType::boolean,    {},               "5D Touch" },
        { fixedVelocityValue,   127,    1,      127,    false,  "Fixed Velocity Value", ConfigType::integer,    {},               "5D Touch" },
        { pianoMode,            0,      0,      1,      false,  "Piano Mode",           ConfigType::boolean,    {},               "Play mode" },
        { glideLock,            0,      0,      127,    false,  "Glide Rate",           ConfigType::integer,    {},               "Play mode" },
        { glideLockEnable,      0,      0,      1,      false,  "Glidelock Enable",     ConfigType::boolean,    {},               "Play mode" },
        { mode,                 4,      1,      5,      false,  "Mode",                 ConfigType::integer,    {},               "Play mode" },
        { volume,               100,    0,      127,    false,  "Volume",               ConfigType::integer,    {},               "Play mode" },
        { scale,                0,      0,      18,     false,  "Scale",                ConfigType::integer,    {},               "Play mode" }, // NOTE: Should be options
        { hideMode,             0,      0,      1,      false,  "Hide Mode",            ConfigType::boolean,    {},               "Play mode" },
        { chord,                0,      0,      127,    false,  "Chord",                ConfigType::integer,    {},               "Play mode" }, // NOTE: Should be options
        { arpPattern,           0,      0,      127,    false,  "Arp Pattern",          ConfigType::integer,    {},               "Play mode" },
        { tempo,                120,    1,      300,    false,  "Tempo",                ConfigType::integer,    {},               "Rhythm" },
        { xTrackingMode,        1,      0,      4,      false,  "Glide Tracking Mode",  ConfigType::options,    { "Multi-Channel",
                                                                                                                  "Last Played",
                                                                                                                  "Highest",
                                                                                                                  "Lowest",
                                                                                                                  "Disabled" },   "Play mode" },
        { yTrackingMode,        1,      0,      4,      false,  "Slide Tracking Mode",  ConfigType::options,    { "Multi-Channel",
                                                                                                                  "Last Played",
                                                                                                                  "Highest",
                                                                                                                  "Lowest",
                                                                                                                  "Disabled" },   "Play mode" },
        { zTrackingMode,        1,      0,      4,      false,  "Pressure Tracking Mode",  ConfigType::options, { "Multi-Channel",
                                                                                                                  "Last Played",
                                                                                                                  "Highest",
                                                                                                                  "Lowest",
                                                                                                                  "Disabled",
                                                                                                                  "Hardest" },    "Play mode" },

        { gammaCorrection,      0,      0,      1,      false,  "Gamma Correction",     ConfigType::boolean,    {},               {} },

        // These can be defined for unique usage for a given Littlefoot script
        { user0,                0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user1,                0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user2,                0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user3,                0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user4,                0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user5,                0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user6,                0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user7,                0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user8,                0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user9,                0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user10,               0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user11,               0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user12,               0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user13,               0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user14,               0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user15,               0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user16,               0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user17,               0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user18,               0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user19,               0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user20,               0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user21,               0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user22,               0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user23,               0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user24,               0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user25,               0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user26,               0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user27,               0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user28,               0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user29,               0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user30,               0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} },
        { user31,               0,    0,      127,      false,  {},                     ConfigType::integer,    {},               {} }
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

    juce::String getOptionName (ConfigItemId item, uint8 optionIndex)
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

        return {};
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
        HostPacketBuilder<32> packet;

        packet.writePacketSysexHeaderBytes (deviceIndex);
        packet.addConfigSetMessage (item, value);
        packet.writePacketSysexFooter();

        if (deviceConnection != nullptr)
            deviceConnection->sendMessageToDevice (packet.getData(), (size_t) packet.size());
    }

    void requestBlockConfig (ConfigItemId item)
    {
        HostPacketBuilder<32> packet;

        packet.writePacketSysexHeaderBytes (deviceIndex);
        packet.addRequestMessage (item);
        packet.writePacketSysexFooter();

        if (deviceConnection != nullptr)
            deviceConnection->sendMessageToDevice(packet.getData(), (size_t) packet.size());
    }

    void requestFactoryConfigSync()
    {
        HostPacketBuilder<32> packet;

        packet.writePacketSysexHeaderBytes(deviceIndex);
        packet.addRequestFactorySyncMessage();
        packet.writePacketSysexFooter();

        if (deviceConnection != nullptr)
            deviceConnection->sendMessageToDevice(packet.getData(), (size_t) packet.size());
    }

    void requestUserConfigSync()
    {
        HostPacketBuilder<32> packet;

        packet.writePacketSysexHeaderBytes(deviceIndex);
        packet.addRequestUserSyncMessage();
        packet.writePacketSysexFooter();

        if (deviceConnection != nullptr)
            deviceConnection->sendMessageToDevice(packet.getData(), (size_t) packet.size());
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

    TopologyIndex deviceIndex;
    PhysicalTopologySource::DeviceConnection* deviceConnection;
};

} // namespace juce
