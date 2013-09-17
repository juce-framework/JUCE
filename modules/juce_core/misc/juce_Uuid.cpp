/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/


Uuid::Uuid()
{
    Random r;

    for (size_t i = 0; i < sizeof (uuid); ++i)
        uuid[i] = (uint8) (r.nextInt (256));
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

String Uuid::toString() const
{
    return String::toHexString (uuid, sizeof (uuid), 0);
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
