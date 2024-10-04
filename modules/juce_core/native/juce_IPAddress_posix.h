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

namespace
{
    struct InterfaceInfo
    {
        IPAddress interfaceAddress, broadcastAddress;
    };

    inline bool operator== (const InterfaceInfo& lhs, const InterfaceInfo& rhs)
    {
        return lhs.interfaceAddress == rhs.interfaceAddress
            && lhs.broadcastAddress == rhs.broadcastAddress;
    }

   #if ! JUCE_WASM
    static IPAddress makeAddress (const sockaddr_in6* addr_in)
    {
        if (addr_in == nullptr)
            return {};

        auto addr = addr_in->sin6_addr;

        IPAddressByteUnion temp;
        uint16 arr[8];

        for (int i = 0; i < 8; ++i) // Swap bytes from network to host order
        {
            temp.split[0] = addr.s6_addr[i * 2 + 1];
            temp.split[1] = addr.s6_addr[i * 2];

            arr[i] = temp.combined;
        }

        return IPAddress (arr);
    }

    static IPAddress makeAddress (const sockaddr_in* addr_in)
    {
        if (addr_in->sin_addr.s_addr == INADDR_NONE)
            return {};

        return IPAddress (ntohl (addr_in->sin_addr.s_addr));
    }

    bool populateInterfaceInfo (struct ifaddrs* ifa, InterfaceInfo& interfaceInfo)
    {
        if (ifa->ifa_addr != nullptr)
        {
            if (ifa->ifa_addr->sa_family == AF_INET)
            {
                auto interfaceAddressInfo = unalignedPointerCast<sockaddr_in*> (ifa->ifa_addr);
                auto broadcastAddressInfo = unalignedPointerCast<sockaddr_in*> (ifa->ifa_dstaddr);

                if (interfaceAddressInfo->sin_addr.s_addr != INADDR_NONE)
                {
                    interfaceInfo.interfaceAddress = makeAddress (interfaceAddressInfo);
                    interfaceInfo.broadcastAddress = makeAddress (broadcastAddressInfo);
                    return true;
                }
            }
            else if (ifa->ifa_addr->sa_family == AF_INET6)
            {
                interfaceInfo.interfaceAddress = makeAddress (unalignedPointerCast<sockaddr_in6*> (ifa->ifa_addr));
                interfaceInfo.broadcastAddress = makeAddress (unalignedPointerCast<sockaddr_in6*> (ifa->ifa_dstaddr));
                return true;
            }
        }

        return false;
    }
   #endif

    Array<InterfaceInfo> getAllInterfaceInfo()
    {
        Array<InterfaceInfo> interfaces;

       #if JUCE_WASM
        // TODO
       #else
        struct ifaddrs* ifaddr = nullptr;

        if (getifaddrs (&ifaddr) != -1)
        {
            for (auto* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
            {
                InterfaceInfo i;

                if (populateInterfaceInfo (ifa, i))
                    interfaces.addIfNotAlreadyThere (i);
            }

            freeifaddrs (ifaddr);
        }
       #endif

        return interfaces;
    }
}

void IPAddress::findAllAddresses (Array<IPAddress>& result, bool includeIPv6)
{
    for (auto& i : getAllInterfaceInfo())
        if (includeIPv6 || ! i.interfaceAddress.isIPv6)
            result.addIfNotAlreadyThere (i.interfaceAddress);
}

IPAddress IPAddress::getInterfaceBroadcastAddress (const IPAddress& interfaceAddress)
{
    for (auto& i : getAllInterfaceInfo())
        if (i.interfaceAddress == interfaceAddress)
            return i.broadcastAddress;

    return {};
}

} // namespace juce
