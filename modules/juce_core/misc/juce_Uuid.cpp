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

Uuid::Uuid()
{
    Random r;

    for (size_t i = 0; i < sizeof (uuid); ++i)
        uuid[i] = (uint8) (r.nextInt (256));

    // To make it RFC 4122 compliant, need to force a few bits...
    uuid[6] = (uuid[6] & 0x0f) | 0x40;
    uuid[8] = (uuid[8] & 0x3f) | 0x80;
}

Uuid::~Uuid() noexcept {}

Uuid::Uuid (const Uuid& other) noexcept
{
    memcpy (uuid, other.uuid, sizeof (uuid));
}

Uuid& Uuid::operator= (const Uuid& other) noexcept
{
    memcpy (uuid, other.uuid, sizeof (uuid));
    return *this;
}

bool Uuid::operator== (const Uuid& other) const noexcept    { return memcmp (uuid, other.uuid, sizeof (uuid)) == 0; }
bool Uuid::operator!= (const Uuid& other) const noexcept    { return ! operator== (other); }

Uuid Uuid::null() noexcept
{
    return Uuid ((const uint8*) nullptr);
}

bool Uuid::isNull() const noexcept
{
    for (size_t i = 0; i < sizeof (uuid); ++i)
        if (uuid[i] != 0)
            return false;

    return true;
}

String Uuid::getHexRegion (int start, int length) const
{
    return String::toHexString (uuid + start, length, 0);
}

String Uuid::toString() const
{
    return getHexRegion (0, 16);
}

String Uuid::toDashedString() const
{
    return getHexRegion (0, 4)
            + "-" + getHexRegion (4, 2)
            + "-" + getHexRegion (6, 2)
            + "-" + getHexRegion (8, 2)
            + "-" + getHexRegion (10, 6);
}

Uuid::Uuid (const String& uuidString)
{
    operator= (uuidString);
}

Uuid& Uuid::operator= (const String& uuidString)
{
    MemoryBlock mb;
    mb.loadFromHexString (uuidString);
    mb.ensureSize (sizeof (uuid), true);
    mb.copyTo (uuid, 0, sizeof (uuid));
    return *this;
}

Uuid::Uuid (const uint8* const rawData) noexcept
{
    operator= (rawData);
}

Uuid& Uuid::operator= (const uint8* const rawData) noexcept
{
    if (rawData != nullptr)
        memcpy (uuid, rawData, sizeof (uuid));
    else
        zeromem (uuid, sizeof (uuid));

    return *this;
}
