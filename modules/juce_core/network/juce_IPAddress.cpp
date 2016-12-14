/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

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
    tokens.addTokens (adr, ".", String());

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
    int bufferSize = 1024;

    do
    {
        bufferSize *= 2;
        buffer.calloc ((size_t) bufferSize);

        cfg.ifc_len = bufferSize;
        cfg.ifc_buf = buffer;

        if (ioctl (sock, SIOCGIFCONF, &cfg) < 0 && errno != EINVAL)
            return;

    } while (bufferSize < cfg.ifc_len + 2 * (int) (IFNAMSIZ + sizeof (struct sockaddr_in6)));

   #if JUCE_MAC || JUCE_IOS
    while (cfg.ifc_len >= (int) (IFNAMSIZ + sizeof (struct sockaddr_in)))
    {
        if (cfg.ifc_req->ifr_addr.sa_family == AF_INET) // Skip non-internet addresses
            addAddress ((const sockaddr_in*) &cfg.ifc_req->ifr_addr, result);

        cfg.ifc_len -= IFNAMSIZ + cfg.ifc_req->ifr_addr.sa_len;
        cfg.ifc_buf += IFNAMSIZ + cfg.ifc_req->ifr_addr.sa_len;
    }
   #else
    for (size_t i = 0; i < (size_t) cfg.ifc_len / (size_t) sizeof (struct ifreq); ++i)
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
