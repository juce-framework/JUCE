/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

IPAddress::IPAddress() noexcept
{
    address[0] = 0;  address[1] = 0;
    address[2] = 0;  address[3] = 0;
}

IPAddress::IPAddress (const uint8 bytes[4]) noexcept
{
    address[0] = bytes[0];  address[1] = bytes[1];
    address[2] = bytes[2];  address[3] = bytes[3];
}

IPAddress::IPAddress (uint8 a0, uint8 a1, uint8 a2, uint8 a3) noexcept
{
    address[0] = a0;  address[1] = a1;
    address[2] = a2;  address[3] = a3;
}

IPAddress::IPAddress (uint32 n) noexcept
{
    address[0] = (n >> 24);
    address[1] = (n >> 16) & 255;
    address[2] = (n >> 8)  & 255;
    address[3] = (n & 255);
}

IPAddress::IPAddress (const String& adr)
{
    StringArray tokens;
    tokens.addTokens (adr, ".", String::empty);

    for (int i = 0; i < 4; ++i)
        address[i] = (uint8) tokens[i].getIntValue();
}

String IPAddress::toString() const
{
    String s ((int) address[0]);

    for (int i = 1; i < 4; ++i)
        s << '.' << (int) address[i];

    return s;
}

IPAddress IPAddress::any() noexcept           { return IPAddress(); }
IPAddress IPAddress::broadcast() noexcept     { return IPAddress (255, 255, 255, 255); }
IPAddress IPAddress::local() noexcept         { return IPAddress (127, 0, 0, 1); }

bool IPAddress::operator== (const IPAddress& other) const noexcept
{
    return address[0] == other.address[0]
        && address[1] == other.address[1]
        && address[2] == other.address[2]
        && address[3] == other.address[3];
}

bool IPAddress::operator!= (const IPAddress& other) const noexcept
{
    return ! operator== (other);
}

#if ! JUCE_WINDOWS
static void addAddress (const sockaddr_in* addr_in, Array<IPAddress>& result)
{
    in_addr_t addr = addr_in->sin_addr.s_addr;

    if (addr != INADDR_NONE)
        result.addIfNotAlreadyThere (IPAddress (ntohl (addr)));
}

static void findIPAddresses (int sock, Array<IPAddress>& result)
{
    ifconf cfg;
    HeapBlock<char> buffer;
    size_t bufferSize = 1024;

    do
    {
        bufferSize *= 2;
        buffer.calloc (bufferSize);

        cfg.ifc_len = bufferSize;
        cfg.ifc_buf = buffer;

        if (ioctl (sock, SIOCGIFCONF, &cfg) < 0 && errno != EINVAL)
            return;

    } while (bufferSize < cfg.ifc_len + 2 * (IFNAMSIZ + sizeof (struct sockaddr_in6)));

   #if JUCE_MAC || JUCE_IOS
    while (cfg.ifc_len >= (int) (IFNAMSIZ + sizeof (struct sockaddr_in)))
    {
        if (cfg.ifc_req->ifr_addr.sa_family == AF_INET) // Skip non-internet addresses
            addAddress ((const sockaddr_in*) &cfg.ifc_req->ifr_addr, result);

        cfg.ifc_len -= IFNAMSIZ + cfg.ifc_req->ifr_addr.sa_len;
        cfg.ifc_buf += IFNAMSIZ + cfg.ifc_req->ifr_addr.sa_len;
    }
   #else
    for (int i = 0; i < cfg.ifc_len / sizeof (struct ifreq); ++i)
    {
        const ifreq& item = cfg.ifc_req[i];

        if (item.ifr_addr.sa_family == AF_INET)
            addAddress ((const sockaddr_in*) &item.ifr_addr, result);
    }
   #endif
}

void IPAddress::findAllAddresses (Array<IPAddress>& result)
{
    const int sock = socket (AF_INET, SOCK_DGRAM, 0); // a dummy socket to execute the IO control

    if (sock >= 0)
    {
        findIPAddresses (sock, result);
        ::close (sock);
    }
}
#endif
