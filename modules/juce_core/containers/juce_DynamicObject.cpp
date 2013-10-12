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

DynamicObject::DynamicObject()
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

var DynamicObject::getProperty (const Identifier& propertyName) const
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
    if (var::NativeFunction function = properties [method].getNativeFunction())
        return function (args);

    return var();
}

void DynamicObject::setMethod (Identifier name, var::NativeFunction function)
{
    properties.set (name, var (function));
}

void DynamicObject::clear()
{
    properties.clear();
}

DynamicObject::Ptr DynamicObject::clone()
{
    DynamicObject* newCopy = new DynamicObject();
    newCopy->properties = properties;

    for (LinkedListPointer<NamedValueSet::NamedValue>* i = &(newCopy->properties.values);;)
    {
        if (NamedValueSet::NamedValue* const v = i->get())
        {
            v->value = v->value.clone();
            i = &(v->nextListItem);
        }
        else
            break;
    }

    return newCopy;
}

void DynamicObject::writeAsJSON (OutputStream& out, const int indentLevel, const bool allOnOneLine)
{
    out << '{';
    if (! allOnOneLine)
        out << newLine;

    for (LinkedListPointer<NamedValueSet::NamedValue>* i = &(properties.values);;)
    {
        if (NamedValueSet::NamedValue* const v = i->get())
        {
            if (! allOnOneLine)
                JSONFormatter::writeSpaces (out, indentLevel + JSONFormatter::indentSize);

            out << '"';
            JSONFormatter::writeString (out, v->name);
            out << "\": ";
            JSONFormatter::write (out, v->value, indentLevel + JSONFormatter::indentSize, allOnOneLine);

            if (v->nextListItem.get() != nullptr)
            {
                if (allOnOneLine)
                    out << ", ";
                else
                    out << ',' << newLine;
            }
            else if (! allOnOneLine)
                out << newLine;

            i = &(v->nextListItem);
        }
        else
            break;
    }

    if (! allOnOneLine)
        JSONFormatter::writeSpaces (out, indentLevel);

    out << '}';
}
