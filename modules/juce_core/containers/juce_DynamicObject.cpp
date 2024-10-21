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
        JSONFormatter::writeString (out, properties.getName (i), format.getEncoding());
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
