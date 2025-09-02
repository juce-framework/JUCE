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

#pragma once

namespace juce
{

/** Useful functions and classes for defining VST3 Interface Ids.

    The classes and functions in this struct are intentionally lightweight,
    requiring almost no JUCE or Steinberg VST3 SDK dependencies.

    @tags{Audio}
*/
struct VST3Interface
{
    /** An enum indicating the various VST3 interface types.

        In most cases users shouldn't need to concern themselves with any interfaces
        other than the component, which is used to report the actual audio effect.
    */
    enum class Type
    {
        ara,
        controller,
        compatibility,
        component,
        processor
    };

    /** A type storing the byte values for a unique VST3 interface identifier. */
    using Id = std::array<std::byte, 16>;

    /** Returns a 16-byte array indicating the VST3 interface ID used for a given
        VST2 plugin.

        Internally JUCE will use this method to assign an ID for the component and
        controller interfaces when JUCE_VST3_CAN_REPLACE_VST2 is enabled.

        @see jucePluginId, hexStringToId
    */
    static Id vst2PluginId (uint32_t pluginCode,
                            const char* pluginName,
                            Type interfaceType = Type::component)
    {
        Id iid{};

        iid[0] = (std::byte) 'V';
        iid[1] = (std::byte) 'S';
        iid[2] = (std::byte) std::invoke ([&]
        {
            switch (interfaceType)
            {
                case Type::controller:     return 'E';
                case Type::component:      return 'T';
                case Type::ara:            [[fallthrough]];
                case Type::compatibility:  [[fallthrough]];
                case Type::processor:      break;
            }

            // A VST2 plugin only has two possible interfaces
            // - component (the audio effect)
            // - controller (the editor/UI)
            jassertfalse;
            return '\0';
        });
        iid[3] = (std::byte) (pluginCode >> 24);
        iid[4] = (std::byte) (pluginCode >> 16);
        iid[5] = (std::byte) (pluginCode >> 8);
        iid[6] = (std::byte) pluginCode;

        for (size_t index = 7; index < iid.size() && *pluginName != 0; ++index)
        {
            iid[index] = (std::byte) std::tolower (*pluginName);
            ++pluginName;
        }

       #if JUCE_WINDOWS
        std::swap (iid[0], iid[3]);
        std::swap (iid[1], iid[2]);
        std::swap (iid[4], iid[5]);
        std::swap (iid[6], iid[7]);
       #endif

        return iid;
    }

    /** Returns a 16-byte array indicating the VST3 interface ID used for a given
        JUCE VST3 plugin.

        Internally this is what JUCE will use to assign an ID to each VST3 interface,
        unless JUCE_VST3_CAN_REPLACE_VST2 is enabled.

        @see vst2PluginId, hexStringToId
    */
    static inline Id jucePluginId (uint32_t manufacturerCode,
                                   uint32_t pluginCode,
                                   Type interfaceType = Type::component)
    {
        const auto word0 = std::invoke ([&]() -> uint32_t
        {
            switch (interfaceType)
            {
                case Type::ara:            [[fallthrough]];
                case Type::controller:     [[fallthrough]];
                case Type::compatibility:  [[fallthrough]];
                case Type::component:      return 0xABCDEF01;
                case Type::processor:      return 0x0101ABAB;
            }

            jassertfalse;
            return 0;
        });

        const auto word1 = std::invoke ([&]() -> uint32_t
        {
            switch (interfaceType)
            {
                case Type::ara:            return 0xA1B2C3D4;
                case Type::controller:     return 0x1234ABCD;
                case Type::compatibility:  return 0xC0DEF00D;
                case Type::component:      return 0x9182FAEB;
                case Type::processor:      return 0xABCDEF01;
            }

            jassertfalse;
            return 0;
        });

        constexpr auto getByteFromLSB = [] (uint32_t word, int byteIndex)
        {
            jassert (0 <= byteIndex && byteIndex <= 3);
            return (std::byte) ((word >> (byteIndex * 8)) & 0xff);
        };

       #if JUCE_WINDOWS
        constexpr auto isWindows = true;
       #else
        constexpr auto isWindows = false;
       #endif

        return {
            getByteFromLSB (word0, isWindows ? 0 : 3),
            getByteFromLSB (word0, isWindows ? 1 : 2),
            getByteFromLSB (word0, isWindows ? 2 : 1),
            getByteFromLSB (word0, isWindows ? 3 : 0),

            getByteFromLSB (word1, isWindows ? 2 : 3),
            getByteFromLSB (word1, isWindows ? 3 : 2),
            getByteFromLSB (word1, isWindows ? 0 : 1),
            getByteFromLSB (word1, isWindows ? 1 : 0),

            getByteFromLSB (manufacturerCode, 3),
            getByteFromLSB (manufacturerCode, 2),
            getByteFromLSB (manufacturerCode, 1),
            getByteFromLSB (manufacturerCode, 0),

            getByteFromLSB (pluginCode, 3),
            getByteFromLSB (pluginCode, 2),
            getByteFromLSB (pluginCode, 1),
            getByteFromLSB (pluginCode, 0)
        };
    }

    /** Converts a 32-character hex notation string to a VST3 interface ID.

        @see jucePluginId, vst2PluginId
     */
    static inline Id hexStringToId (const char* hex)
    {
        jassert (std::strlen (hex) == 32);

        const auto getByteValue = [](const char* str)
        {
            const auto getCharacterValue = [](const char c)
            {
                if (c >= '0' && c <= '9')
                    return (std::byte) (c - '0');

                if (c >= 'A' && c <= 'F')
                    return (std::byte) (c - 'A' + 10);

                if (c >= 'a' && c <= 'f')
                    return (std::byte) (c - 'a' + 10);

                // Invalid hex character!
                jassertfalse;
                return std::byte{};
            };

            return getCharacterValue (str[0]) << 4
                 | getCharacterValue (str[1]);
        };

        return { getByteValue (hex),
                 getByteValue (hex + 2),
                 getByteValue (hex + 4),
                 getByteValue (hex + 6),
                 getByteValue (hex + 8),
                 getByteValue (hex + 10),
                 getByteValue (hex + 12),
                 getByteValue (hex + 14),
                 getByteValue (hex + 16),
                 getByteValue (hex + 18),
                 getByteValue (hex + 20),
                 getByteValue (hex + 22),
                 getByteValue (hex + 24),
                 getByteValue (hex + 26),
                 getByteValue (hex + 28),
                 getByteValue (hex + 30) };
    }

    VST3Interface() = delete;
};

} // namespace juce
