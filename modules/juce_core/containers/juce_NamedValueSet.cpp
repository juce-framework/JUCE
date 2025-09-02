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

NamedValueSet::NamedValue::NamedValue() noexcept {}
NamedValueSet::NamedValue::~NamedValue() noexcept {}

NamedValueSet::NamedValue::NamedValue (const Identifier& n, const var& v)  : name (n), value (v) {}
NamedValueSet::NamedValue::NamedValue (const NamedValue& other) : NamedValue (other.name, other.value) {}

NamedValueSet::NamedValue::NamedValue (NamedValue&& other) noexcept
   : NamedValue (std::move (other.name),
                 std::move (other.value))
{}

NamedValueSet::NamedValue::NamedValue (const Identifier& n, var&& v) noexcept
   : name (n), value (std::move (v))
{
}

NamedValueSet::NamedValue::NamedValue (Identifier&& n, var&& v) noexcept
   : name (std::move (n)),
     value (std::move (v))
{}

NamedValueSet::NamedValue& NamedValueSet::NamedValue::operator= (NamedValue&& other) noexcept
{
    name = std::move (other.name);
    value = std::move (other.value);
    return *this;
}

bool NamedValueSet::NamedValue::operator== (const NamedValue& other) const noexcept   { return name == other.name && value == other.value; }
bool NamedValueSet::NamedValue::operator!= (const NamedValue& other) const noexcept   { return ! operator== (other); }

//==============================================================================
NamedValueSet::NamedValueSet() noexcept {}
NamedValueSet::~NamedValueSet() noexcept {}

NamedValueSet::NamedValueSet (const NamedValueSet& other)  : values (other.values) {}

NamedValueSet::NamedValueSet (NamedValueSet&& other) noexcept
   : values (std::move (other.values)) {}

NamedValueSet::NamedValueSet (std::initializer_list<NamedValue> list)
   : values (std::move (list))
{
}

NamedValueSet& NamedValueSet::operator= (const NamedValueSet& other)
{
    clear();
    values = other.values;
    return *this;
}

NamedValueSet& NamedValueSet::operator= (NamedValueSet&& other) noexcept
{
    other.values.swapWith (values);
    return *this;
}

void NamedValueSet::clear()
{
    values.clear();
}

bool NamedValueSet::operator== (const NamedValueSet& other) const noexcept
{
    auto num = values.size();

    if (num != other.values.size())
        return false;

    for (int i = 0; i < num; ++i)
    {
        // optimise for the case where the keys are in the same order
        if (values.getReference (i).name == other.values.getReference (i).name)
        {
            if (values.getReference (i).value != other.values.getReference (i).value)
                return false;
        }
        else
        {
            // if we encounter keys that are in a different order, search remaining items by brute force..
            for (int j = i; j < num; ++j)
            {
                if (auto* otherVal = other.getVarPointer (values.getReference (j).name))
                    if (values.getReference (j).value == *otherVal)
                        continue;

                return false;
            }

            return true;
        }
    }

    return true;
}

bool NamedValueSet::operator!= (const NamedValueSet& other) const noexcept   { return ! operator== (other); }

int NamedValueSet::size() const noexcept        { return values.size(); }
bool NamedValueSet::isEmpty() const noexcept    { return values.isEmpty(); }

static const var& getNullVarRef() noexcept
{
    static var nullVar;
    return nullVar;
}

const var& NamedValueSet::operator[] (const Identifier& name) const noexcept
{
    if (auto* v = getVarPointer (name))
        return *v;

    return getNullVarRef();
}

var NamedValueSet::getWithDefault (const Identifier& name, const var& defaultReturnValue) const
{
    if (auto* v = getVarPointer (name))
        return *v;

    return defaultReturnValue;
}

var* NamedValueSet::getVarPointer (const Identifier& name) noexcept
{
    for (auto& i : values)
        if (i.name == name)
            return &(i.value);

    return {};
}

const var* NamedValueSet::getVarPointer (const Identifier& name) const noexcept
{
    for (auto& i : values)
        if (i.name == name)
            return &(i.value);

    return {};
}

bool NamedValueSet::set (const Identifier& name, var&& newValue)
{
    if (auto* v = getVarPointer (name))
    {
        if (v->equalsWithSameType (newValue))
            return false;

        *v = std::move (newValue);
        return true;
    }

    values.add ({ name, std::move (newValue) });
    return true;
}

bool NamedValueSet::set (const Identifier& name, const var& newValue)
{
    if (auto* v = getVarPointer (name))
    {
        if (v->equalsWithSameType (newValue))
            return false;

        *v = newValue;
        return true;
    }

    values.add ({ name, newValue });
    return true;
}

bool NamedValueSet::contains (const Identifier& name) const noexcept
{
    return getVarPointer (name) != nullptr;
}

int NamedValueSet::indexOf (const Identifier& name) const noexcept
{
    auto numValues = values.size();

    for (int i = 0; i < numValues; ++i)
        if (values.getReference (i).name == name)
            return i;

    return -1;
}

bool NamedValueSet::remove (const Identifier& name)
{
    auto numValues = values.size();

    for (int i = 0; i < numValues; ++i)
    {
        if (values.getReference (i).name == name)
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
    return {};
}

const var& NamedValueSet::getValueAt (const int index) const noexcept
{
    if (isPositiveAndBelow (index, values.size()))
        return values.getReference (index).value;

    jassertfalse;
    return getNullVarRef();
}

var* NamedValueSet::getVarPointerAt (int index) noexcept
{
    if (isPositiveAndBelow (index, values.size()))
        return &(values.getReference (index).value);

    return {};
}

const var* NamedValueSet::getVarPointerAt (int index) const noexcept
{
    if (isPositiveAndBelow (index, values.size()))
        return &(values.getReference (index).value);

    return {};
}

void NamedValueSet::setFromXmlAttributes (const XmlElement& xml)
{
    values.clearQuick();

    for (const auto& [name, value] : xml.getAttributeIterator())
    {
        if (name.toString().startsWith ("base64:"))
        {
            MemoryBlock mb;

            if (mb.fromBase64Encoding (value))
            {
                values.add ({ name.toString().substring (7), var (mb) });
                continue;
            }
        }

        values.add ({ name, var (value) });
    }
}

void NamedValueSet::copyToXmlAttributes (XmlElement& xml) const
{
    for (auto& i : values)
    {
        if (auto* mb = i.value.getBinaryData())
        {
            xml.setAttribute ("base64:" + i.name.toString(), mb->toBase64Encoding());
        }
        else
        {
            // These types can't be stored as XML!
            jassert (! i.value.isObject());
            jassert (! i.value.isMethod());
            jassert (! i.value.isArray());

            xml.setAttribute (i.name.toString(),
                              i.value.toString());
        }
    }
}

} // namespace juce
