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

namespace juce
{

std::map<uint32_t, String> VST3ClientExtensions::getCompatibleParameterIds (const InterfaceId&) const
{
    return {};
}

VST3ClientExtensions::InterfaceId VST3ClientExtensions::convertJucePluginId (uint32_t manufacturerCode,
                                                                             uint32_t pluginCode,
                                                                             InterfaceType interfaceType)
{
    const auto word0 = std::invoke ([&]() -> uint32_t
    {
        switch (interfaceType)
        {
            case InterfaceType::ara:            [[fallthrough]];
            case InterfaceType::controller:     [[fallthrough]];
            case InterfaceType::compatibility:  [[fallthrough]];
            case InterfaceType::component:      return 0xABCDEF01;
            case InterfaceType::processor:      return 0x0101ABAB;
        }

        jassertfalse;
        return 0;
    });

    const auto word1 = std::invoke ([&]() -> uint32_t
    {
        switch (interfaceType)
        {
            case InterfaceType::ara:            return 0xA1B2C3D4;
            case InterfaceType::controller:     return 0x1234ABCD;
            case InterfaceType::compatibility:  return 0xC0DEF00D;
            case InterfaceType::component:      return 0x9182FAEB;
            case InterfaceType::processor:      return 0xABCDEF01;
        }

        jassertfalse;
        return 0;
    });

    constexpr auto getByteFromLSB = [] (uint32_t word, int byteIndex)
    {
        jassert (isPositiveAndNotGreaterThan (byteIndex, 3));
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

VST3ClientExtensions::InterfaceId VST3ClientExtensions::convertVST2PluginId (uint32_t pluginCode,
                                                                             const String& pluginName,
                                                                             InterfaceType interfaceType)
{
    VST3ClientExtensions::InterfaceId iid{};

    iid[0] = (std::byte) 'V';
    iid[1] = (std::byte) 'S';
    iid[2] = (std::byte) std::invoke ([&]
    {
        switch (interfaceType)
        {
            case InterfaceType::controller:     return 'E';
            case InterfaceType::component:      return 'T';
            case InterfaceType::ara:            [[fallthrough]];
            case InterfaceType::compatibility:  [[fallthrough]];
            case InterfaceType::processor:      break;
        }

        // A VST2 plugin only has two interfaces
        // - component (the audio effect)
        // - controller (the editor/UI)
        jassertfalse;
        return '\0';
    });
    iid[3] = (std::byte) (pluginCode >> 24);
    iid[4] = (std::byte) (pluginCode >> 16);
    iid[5] = (std::byte) (pluginCode >> 8);
    iid[6] = (std::byte) pluginCode;

    for (const auto [index, character] : enumerate (pluginName, (size_t) 7))
    {
        if (index >= iid.size())
            break;

        iid[index] = (std::byte) CharacterFunctions::toLowerCase (character);
    }

   #if JUCE_WINDOWS
    std::swap (iid[0], iid[3]);
    std::swap (iid[1], iid[2]);
    std::swap (iid[4], iid[5]);
    std::swap (iid[6], iid[7]);
   #endif

    return iid;
}

uint32_t VST3ClientExtensions::convertJuceParameterId (const String& parameterId, bool studioOneCompatible)
{
    auto hash = (uint32_t) (parameterId.hashCode());

    if (studioOneCompatible)
        hash &= 0x7fffffff;

    return hash;
}

VST3ClientExtensions::InterfaceId VST3ClientExtensions::toInterfaceId (const String& interfaceIdString)
{
    jassert (interfaceIdString.length() == 32);
    jassert (interfaceIdString.containsOnly ("0123456789abcdefABCDEF"));

    return { (std::byte) interfaceIdString.substring ( 0,  2).getHexValue32(),
             (std::byte) interfaceIdString.substring ( 2,  4).getHexValue32(),
             (std::byte) interfaceIdString.substring ( 4,  6).getHexValue32(),
             (std::byte) interfaceIdString.substring ( 6,  8).getHexValue32(),
             (std::byte) interfaceIdString.substring ( 8, 10).getHexValue32(),
             (std::byte) interfaceIdString.substring (10, 12).getHexValue32(),
             (std::byte) interfaceIdString.substring (12, 14).getHexValue32(),
             (std::byte) interfaceIdString.substring (14, 16).getHexValue32(),
             (std::byte) interfaceIdString.substring (16, 18).getHexValue32(),
             (std::byte) interfaceIdString.substring (18, 20).getHexValue32(),
             (std::byte) interfaceIdString.substring (20, 22).getHexValue32(),
             (std::byte) interfaceIdString.substring (22, 24).getHexValue32(),
             (std::byte) interfaceIdString.substring (24, 26).getHexValue32(),
             (std::byte) interfaceIdString.substring (26, 28).getHexValue32(),
             (std::byte) interfaceIdString.substring (28, 30).getHexValue32(),
             (std::byte) interfaceIdString.substring (30, 32).getHexValue32() };
}

} // namespace juce
