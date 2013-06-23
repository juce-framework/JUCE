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

NamedValueSet::NamedValue::NamedValue() noexcept
{
}

inline NamedValueSet::NamedValue::NamedValue (const Identifier n, const var& v)
    : name (n), value (v)
{
}

NamedValueSet::NamedValue::NamedValue (const NamedValue& other)
    : name (other.name), value (other.value)
{
}

NamedValueSet::NamedValue& NamedValueSet::NamedValue::operator= (const NamedValueSet::NamedValue& other)
{
    name = other.name;
    value = other.value;
    return *this;
}

#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
NamedValueSet::NamedValue::NamedValue (NamedValue&& other) noexcept
    : nextListItem (static_cast <LinkedListPointer<NamedValue>&&> (other.nextListItem)),
      name (static_cast <Identifier&&> (other.name)),
      value (static_cast <var&&> (other.value))
{
}

inline NamedValueSet::NamedValue::NamedValue (const Identifier n, var&& v)
    : name (n), value (static_cast <var&&> (v))
{
}

NamedValueSet::NamedValue& NamedValueSet::NamedValue::operator= (NamedValue&& other) noexcept
{
    nextListItem = static_cast <LinkedListPointer<NamedValue>&&> (other.nextListItem);
    name = static_cast <Identifier&&> (other.name);
    value = static_cast <var&&> (other.value);
    return *this;
}
#endif

bool NamedValueSet::NamedValue::operator== (const NamedValueSet::NamedValue& other) const noexcept
{
    return name == other.name && value == other.value;
}

//==============================================================================
NamedValueSet::NamedValueSet() noexcept
{
}

NamedValueSet::NamedValueSet (const NamedValueSet& other)
{
    values.addCopyOfList (other.values);
}

NamedValueSet& NamedValueSet::operator= (const NamedValueSet& other)
{
    clear();
    values.addCopyOfList (other.values);
    return *this;
}

#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
NamedValueSet::NamedValueSet (NamedValueSet&& other) noexcept
    : values (static_cast <LinkedListPointer<NamedValue>&&> (other.values))
{
}

NamedValueSet& NamedValueSet::operator= (NamedValueSet&& other) noexcept
{
    other.values.swapWith (values);
    return *this;
}
#endif

NamedValueSet::~NamedValueSet()
{
    clear();
}

void NamedValueSet::clear()
{
    values.deleteAll();
}

bool NamedValueSet::operator== (const NamedValueSet& other) const
{
    const NamedValue* i1 = values;
    const NamedValue* i2 = other.values;

    while (i1 != nullptr && i2 != nullptr)
    {
        if (! (*i1 == *i2))
            return false;

        i1 = i1->nextListItem;
        i2 = i2->nextListItem;
    }

    return true;
}

bool NamedValueSet::operator!= (const NamedValueSet& other) const
{
    return ! operator== (other);
}

int NamedValueSet::size() const noexcept
{
    return values.size();
}

const var& NamedValueSet::operator[] (const Identifier name) const
{
    for (NamedValue* i = values; i != nullptr; i = i->nextListItem)
        if (i->name == name)
            return i->value;

    return var::null;
}

var NamedValueSet::getWithDefault (const Identifier name, const var& defaultReturnValue) const
{
    if (const var* const v = getVarPointer (name))
        return *v;

    return defaultReturnValue;
}

var* NamedValueSet::getVarPointer (const Identifier name) const noexcept
{
    for (NamedValue* i = values; i != nullptr; i = i->nextListItem)
        if (i->name == name)
            return &(i->value);

    return nullptr;
}

#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
bool NamedValueSet::set (const Identifier name, var&& newValue)
{
    LinkedListPointer<NamedValue>* i = &values;

    while (i->get() != nullptr)
    {
        NamedValue* const v = i->get();

        if (v->name == name)
        {
            if (v->value.equalsWithSameType (newValue))
                return false;

            v->value = static_cast <var&&> (newValue);
            return true;
        }

        i = &(v->nextListItem);
    }

    i->insertNext (new NamedValue (name, static_cast <var&&> (newValue)));
    return true;
}
#endif

bool NamedValueSet::set (const Identifier name, const var& newValue)
{
    LinkedListPointer<NamedValue>* i = &values;

    while (i->get() != nullptr)
    {
        NamedValue* const v = i->get();

        if (v->name == name)
        {
            if (v->value.equalsWithSameType (newValue))
                return false;

            v->value = newValue;
            return true;
        }

        i = &(v->nextListItem);
    }

    i->insertNext (new NamedValue (name, newValue));
    return true;
}

bool NamedValueSet::contains (const Identifier name) const
{
    return getVarPointer (name) != nullptr;
}

bool NamedValueSet::remove (const Identifier name)
{
    LinkedListPointer<NamedValue>* i = &values;

    for (;;)
    {
        NamedValue* const v = i->get();

        if (v == nullptr)
            break;

        if (v->name == name)
        {
            delete i->removeNext();
            return true;
        }

        i = &(v->nextListItem);
    }

    return false;
}

const Identifier NamedValueSet::getName (const int index) const
{
    const NamedValue* const v = values[index];
    jassert (v != nullptr);
    return v->name;
}

const var& NamedValueSet::getValueAt (const int index) const
{
    const NamedValue* const v = values[index];
    jassert (v != nullptr);
    return v->value;
}

void NamedValueSet::setFromXmlAttributes (const XmlElement& xml)
{
    clear();
    LinkedListPointer<NamedValue>::Appender appender (values);

    const int numAtts = xml.getNumAttributes(); // xxx inefficient - should write an att iterator..

    for (int i = 0; i < numAtts; ++i)
    {
        const String& name = xml.getAttributeName (i);
        const String& value = xml.getAttributeValue (i);

        if (name.startsWith ("base64:"))
        {
            MemoryBlock mb;

            if (mb.fromBase64Encoding (value))
            {
                appender.append (new NamedValue (name.substring (7), var (mb)));
                continue;
            }
        }

        appender.append (new NamedValue (name, var (value)));
    }
}

void NamedValueSet::copyToXmlAttributes (XmlElement& xml) const
{
    for (NamedValue* i = values; i != nullptr; i = i->nextListItem)
    {
        if (const MemoryBlock* mb = i->value.getBinaryData())
        {
            xml.setAttribute ("base64:" + i->name.toString(),
                              mb->toBase64Encoding());
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
