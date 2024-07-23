/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

DynamicObject::DynamicObject()
{
}

DynamicObject::DynamicObject (const DynamicObject& other)
   : ReferenceCountedObject(), properties (other.properties)
{
}

DynamicObject::~DynamicObject()
{
}

bool DynamicObject::hasProperty (const Identifier& propertyName) const
{
    const var* const v = properties.getVarPointer (propertyName);
    return v != nullptr && ! v->isMethod();
}

const var& DynamicObject::getProperty (const Identifier& propertyName) const
{
    return properties [propertyName];
}

void DynamicObject::setProperty (const Identifier& propertyName, const var& newValue)
{
    properties.set (propertyName, newValue);
}

void DynamicObject::removeProperty (const Identifier& propertyName)
{
    properties.remove (propertyName);
}

bool DynamicObject::hasMethod (const Identifier& methodName) const
{
    return getProperty (methodName).isMethod();
}

var DynamicObject::invokeMethod (Identifier method, const var::NativeFunctionArgs& args)
{
    if (auto function = properties [method].getNativeFunction())
        return function (args);

    return {};
}

void DynamicObject::setMethod (Identifier name, var::NativeFunction function)
{
    properties.set (name, var (function));
}

void DynamicObject::clear()
{
    properties.clear();
}

void DynamicObject::cloneAllProperties()
{
    for (int i = properties.size(); --i >= 0;)
        if (auto* v = properties.getVarPointerAt (i))
            *v = v->clone();
}

std::unique_ptr<DynamicObject> DynamicObject::clone() const
{
    auto result = std::make_unique<DynamicObject> (*this);
    result->cloneAllProperties();
    return result;
}

void DynamicObject::writeAsJSON (OutputStream& out, const JSON::FormatOptions& format)
{
    out << '{';
    if (format.getSpacing() == JSON::Spacing::multiLine)
        out << newLine;

    const int numValues = properties.size();

    for (int i = 0; i < numValues; ++i)
    {
        if (format.getSpacing() == JSON::Spacing::multiLine)
            JSONFormatter::writeSpaces (out, format.getIndentLevel() + JSONFormatter::indentSize);

        out << '"';
        JSONFormatter::writeString (out, properties.getName (i));
        out << "\":";

        if (format.getSpacing() != JSON::Spacing::none)
            out << ' ';

        JSON::writeToStream (out,
                             properties.getValueAt (i),
                             format.withIndentLevel (format.getIndentLevel() + JSONFormatter::indentSize));

        if (i < numValues - 1)
        {
            out << ",";

            switch (format.getSpacing())
            {
                case JSON::Spacing::none: break;
                case JSON::Spacing::singleLine: out << ' '; break;
                case JSON::Spacing::multiLine: out << newLine; break;
            }
        }
        else if (format.getSpacing() == JSON::Spacing::multiLine)
            out << newLine;
    }

    if (format.getSpacing() == JSON::Spacing::multiLine)
        JSONFormatter::writeSpaces (out, format.getIndentLevel());

    out << '}';
}

} // namespace juce
