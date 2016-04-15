/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

struct NamedValueSet::NamedValue
{
    NamedValue() noexcept {}
    NamedValue (const Identifier& n, const var& v)  : name (n), value (v) {}
    NamedValue (const NamedValue& other) : name (other.name), value (other.value) {}

   #if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
    NamedValue (NamedValue&& other) noexcept
        : name (static_cast<Identifier&&> (other.name)),
          value (static_cast<var&&> (other.value))
    {
    }

    NamedValue (Identifier&& n, var&& v) noexcept
        : name (static_cast<Identifier&&> (n)),
          value (static_cast<var&&> (v))
    {
    }

    NamedValue& operator= (NamedValue&& other) noexcept
    {
        name = static_cast<Identifier&&> (other.name);
        value = static_cast<var&&> (other.value);
        return *this;
    }
   #endif

    bool operator== (const NamedValue& other) const noexcept   { return name == other.name && value == other.value; }
    bool operator!= (const NamedValue& other) const noexcept   { return ! operator== (other); }

    Identifier name;
    var value;
};

//==============================================================================
NamedValueSet::NamedValueSet() noexcept
{
}

NamedValueSet::NamedValueSet (const NamedValueSet& other)
   : values (other.values)
{
}

NamedValueSet& NamedValueSet::operator= (const NamedValueSet& other)
{
    clear();
    values = other.values;
    return *this;
}

#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
NamedValueSet::NamedValueSet (NamedValueSet&& other) noexcept
    : values (static_cast<Array<NamedValue>&&> (other.values))
{
}

NamedValueSet& NamedValueSet::operator= (NamedValueSet&& other) noexcept
{
    other.values.swapWith (values);
    return *this;
}
#endif

NamedValueSet::~NamedValueSet() noexcept
{
}

void NamedValueSet::clear()
{
    values.clear();
}

bool NamedValueSet::operator== (const NamedValueSet& other) const
{
    return values == other.values;
}

bool NamedValueSet::operator!= (const NamedValueSet& other) const
{
    return ! operator== (other);
}

int NamedValueSet::size() const noexcept
{
    return values.size();
}

bool NamedValueSet::isEmpty() const noexcept
{
    return values.isEmpty();
}

const var& NamedValueSet::operator[] (const Identifier& name) const noexcept
{
    if (const var* v = getVarPointer (name))
        return *v;

    return var::null;
}

var NamedValueSet::getWithDefault (const Identifier& name, const var& defaultReturnValue) const
{
    if (const var* const v = getVarPointer (name))
        return *v;

    return defaultReturnValue;
}

var* NamedValueSet::getVarPointer (const Identifier& name) const noexcept
{
    for (NamedValue* e = values.end(), *i = values.begin(); i != e; ++i)
        if (i->name == name)
            return &(i->value);

    return nullptr;
}

#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
bool NamedValueSet::set (const Identifier& name, var&& newValue)
{
    if (var* const v = getVarPointer (name))
    {
        if (v->equalsWithSameType (newValue))
            return false;

        *v = static_cast<var&&> (newValue);
        return true;
    }

    values.add (NamedValue (name, static_cast<var&&> (newValue)));
    return true;
}
#endif

bool NamedValueSet::set (const Identifier& name, const var& newValue)
{
    if (var* const v = getVarPointer (name))
    {
        if (v->equalsWithSameType (newValue))
            return false;

        *v = newValue;
        return true;
    }

    values.add (NamedValue (name, newValue));
    return true;
}

bool NamedValueSet::contains (const Identifier& name) const noexcept
{
    return getVarPointer (name) != nullptr;
}

int NamedValueSet::indexOf (const Identifier& name) const noexcept
{
    const int numValues = values.size();

    for (int i = 0; i < numValues; ++i)
        if (values.getReference(i).name == name)
            return i;

    return -1;
}

bool NamedValueSet::remove (const Identifier& name)
{
    const int numValues = values.size();

    for (int i = 0; i < numValues; ++i)
    {
        if (values.getReference(i).name == name)
        {
            values.remove (i);
            return true;
        }
    }

    return false;
}

Identifier NamedValueSet::getName (const int index) const noexcept
{
    if (isPositiveAndBelow (index, values.size()))
        return values.getReference (index).name;

    jassertfalse;
    return Identifier();
}

const var& NamedValueSet::getValueAt (const int index) const noexcept
{
    if (isPositiveAndBelow (index, values.size()))
        return values.getReference (index).value;

    jassertfalse;
    return var::null;
}

var* NamedValueSet::getVarPointerAt (int index) const noexcept
{
    if (isPositiveAndBelow (index, values.size()))
        return &(values.getReference (index).value);

    return nullptr;
}

void NamedValueSet::setFromXmlAttributes (const XmlElement& xml)
{
    values.clearQuick();

    for (const XmlElement::XmlAttributeNode* att = xml.attributes; att != nullptr; att = att->nextListItem)
    {
        if (att->name.toString().startsWith ("base64:"))
        {
            MemoryBlock mb;

            if (mb.fromBase64Encoding (att->value))
            {
                values.add (NamedValue (att->name.toString().substring (7), var (mb)));
                continue;
            }
        }

        values.add (NamedValue (att->name, var (att->value)));
    }
}

void NamedValueSet::copyToXmlAttributes (XmlElement& xml) const
{
    for (NamedValue* e = values.end(), *i = values.begin(); i != e; ++i)
    {
        if (const MemoryBlock* mb = i->value.getBinaryData())
        {
            xml.setAttribute ("base64:" + i->name.toString(), mb->toBase64Encoding());
        }
        else
        {
            // These types can't be stored as XML!
            jassert (! i->value.isObject());
            jassert (! i->value.isMethod());
            jassert (! i->value.isArray());

            xml.setAttribute (i->name.toString(),
                              i->value.toString());
        }
    }
}
