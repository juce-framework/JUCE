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

StringPairArray::StringPairArray (const bool ignoreCase_)
    : ignoreCase (ignoreCase_)
{
}

StringPairArray::StringPairArray (const StringPairArray& other)
    : keys (other.keys),
      values (other.values),
      ignoreCase (other.ignoreCase)
{
}

StringPairArray::~StringPairArray()
{
}

StringPairArray& StringPairArray::operator= (const StringPairArray& other)
{
    keys = other.keys;
    values = other.values;
    return *this;
}

bool StringPairArray::operator== (const StringPairArray& other) const
{
    for (int i = keys.size(); --i >= 0;)
        if (other [keys[i]] != values[i])
            return false;

    return true;
}

bool StringPairArray::operator!= (const StringPairArray& other) const
{
    return ! operator== (other);
}

const String& StringPairArray::operator[] (StringRef key) const
{
    return values [keys.indexOf (key, ignoreCase)];
}

String StringPairArray::getValue (StringRef key, const String& defaultReturnValue) const
{
    const int i = keys.indexOf (key, ignoreCase);

    if (i >= 0)
        return values[i];

    return defaultReturnValue;
}

bool StringPairArray::containsKey (StringRef key) const noexcept
{
    return keys.contains (key);
}

void StringPairArray::set (const String& key, const String& value)
{
    const int i = keys.indexOf (key, ignoreCase);

    if (i >= 0)
    {
        values.set (i, value);
    }
    else
    {
        keys.add (key);
        values.add (value);
    }
}

void StringPairArray::addArray (const StringPairArray& other)
{
    for (int i = 0; i < other.size(); ++i)
        set (other.keys[i], other.values[i]);
}

void StringPairArray::clear()
{
    keys.clear();
    values.clear();
}

void StringPairArray::remove (StringRef key)
{
    remove (keys.indexOf (key, ignoreCase));
}

void StringPairArray::remove (const int index)
{
    keys.remove (index);
    values.remove (index);
}

void StringPairArray::setIgnoresCase (const bool shouldIgnoreCase)
{
    ignoreCase = shouldIgnoreCase;
}

String StringPairArray::getDescription() const
{
    String s;

    for (int i = 0; i < keys.size(); ++i)
    {
        s << keys[i] << " = " << values[i];
        if (i < keys.size())
            s << ", ";
    }

    return s;
}

void StringPairArray::minimiseStorageOverheads()
{
    keys.minimiseStorageOverheads();
    values.minimiseStorageOverheads();
}
