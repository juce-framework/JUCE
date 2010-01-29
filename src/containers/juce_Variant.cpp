/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_Variant.h"
#include "juce_DynamicObject.h"

//==============================================================================
var::var() throw()
   : type (voidType)
{
    value.doubleValue = 0;
}

var::~var() throw()
{
    if (type == stringType)
        delete value.stringValue;
    else if (type == objectType && value.objectValue != 0)
        value.objectValue->decReferenceCount();
}

const var var::null;

//==============================================================================
var::var (const var& valueToCopy)
   : type (valueToCopy.type),
     value (valueToCopy.value)
{
    if (type == stringType)
        value.stringValue = new String (*(value.stringValue));
    else if (type == objectType && value.objectValue != 0)
        value.objectValue->incReferenceCount();
}

var::var (const int value_) throw()
   : type (intType)
{
    value.intValue = value_;
}

var::var (const bool value_) throw()
   : type (boolType)
{
    value.boolValue = value_;
}

var::var (const double value_) throw()
   : type (doubleType)
{
    value.doubleValue = value_;
}

var::var (const String& value_)
   : type (stringType)
{
    value.stringValue = new String (value_);
}

var::var (const char* const value_)
   : type (stringType)
{
    value.stringValue = new String (value_);
}

var::var (const juce_wchar* const value_)
   : type (stringType)
{
    value.stringValue = new String (value_);
}

var::var (DynamicObject* const object)
   : type (objectType)
{
    value.objectValue = object;

    if (object != 0)
        object->incReferenceCount();
}

var::var (MethodFunction method_) throw()
   : type (methodType)
{
    value.methodValue = method_;
}

//==============================================================================
void var::swapWith (var& other) throw()
{
    swapVariables (type, other.type);
    swapVariables (value, other.value);
}

var& var::operator= (const var& value_)         { var newValue (value_); swapWith (newValue); return *this; }
var& var::operator= (int value_)                { var newValue (value_); swapWith (newValue); return *this; }
var& var::operator= (bool value_)               { var newValue (value_); swapWith (newValue); return *this; }
var& var::operator= (double value_)             { var newValue (value_); swapWith (newValue); return *this; }
var& var::operator= (const char* value_)        { var newValue (value_); swapWith (newValue); return *this; }
var& var::operator= (const juce_wchar* value_)  { var newValue (value_); swapWith (newValue); return *this; }
var& var::operator= (const String& value_)      { var newValue (value_); swapWith (newValue); return *this; }
var& var::operator= (DynamicObject* value_)     { var newValue (value_); swapWith (newValue); return *this; }
var& var::operator= (MethodFunction value_)     { var newValue (value_); swapWith (newValue); return *this; }

//==============================================================================
var::operator int() const
{
    switch (type)
    {
        case voidType:      break;
        case intType:       return value.intValue;
        case boolType:      return value.boolValue ? 1 : 0;
        case doubleType:    return (int) value.doubleValue;
        case stringType:    return value.stringValue->getIntValue();
        case objectType:    break;
        default:            jassertfalse; break;
    }

    return 0;
}

var::operator bool() const
{
    switch (type)
    {
        case voidType:      break;
        case intType:       return value.intValue != 0;
        case boolType:      return value.boolValue;
        case doubleType:    return value.doubleValue != 0;
        case stringType:    return value.stringValue->getIntValue() != 0
                                    || value.stringValue->trim().equalsIgnoreCase (T("true"))
                                    || value.stringValue->trim().equalsIgnoreCase (T("yes"));
        case objectType:    return value.objectValue != 0;
        default:            jassertfalse; break;
    }

    return false;
}

var::operator float() const
{
    return (float) operator double();
}

var::operator double() const
{
    switch (type)
    {
        case voidType:      break;
        case intType:       return value.intValue;
        case boolType:      return value.boolValue ? 1.0 : 0.0;
        case doubleType:    return value.doubleValue;
        case stringType:    return value.stringValue->getDoubleValue();
        case objectType:    break;
        default:            jassertfalse; break;
    }

    return 0.0;
}

const String var::toString() const
{
    switch (type)
    {
        case voidType:      return String::empty;
        case intType:       return String (value.intValue);
        case boolType:      return value.boolValue ? T("1") : T("0");
        case doubleType:    return String (value.doubleValue);
        case stringType:    return *(value.stringValue);
        case objectType:    return "Object 0x" + String::toHexString ((int) (pointer_sized_int) value.objectValue);
        case methodType:    return "Method";
        default:            jassertfalse; break;
    }

    return String::empty;
}

var::operator const String() const
{
    return toString();
}

DynamicObject* var::getObject() const
{
    return type == objectType ? value.objectValue : 0;
}

bool var::operator== (const var& other) const throw()
{
    switch (type)
    {
        case voidType:      return other.isVoid();
        case intType:       return value.intValue == (int) other;
        case boolType:      return value.boolValue == (bool) other;
        case doubleType:    return value.doubleValue == (double) other;
        case stringType:    return (*(value.stringValue)) == other.toString();
        case objectType:    return value.objectValue == other.getObject();
        case methodType:    return value.methodValue == other.value.methodValue && other.isMethod();
        default:            jassertfalse; break;
    }

    return false;
}

bool var::operator!= (const var& other) const throw()
{
    return ! operator== (other);
}

void var::writeToStream (OutputStream& output) const
{
    switch (type)
    {
        case voidType:      output.writeCompressedInt (0); break;
        case intType:       output.writeCompressedInt (5); output.writeByte (1); output.writeInt (value.intValue); break;
        case boolType:      output.writeCompressedInt (1); output.writeByte (value.boolValue ? 2 : 3); break;
        case doubleType:    output.writeCompressedInt (9); output.writeByte (4); output.writeDouble (value.doubleValue); break;
        case stringType:
        {
            const int len = value.stringValue->copyToUTF8 (0);
            output.writeCompressedInt (len + 1);
            output.writeByte (5);
            HeapBlock <uint8> temp (len);
            value.stringValue->copyToUTF8 (temp);
            output.write (temp, len);
            break;
        }
        case objectType:
        case methodType:    output.writeCompressedInt (0); jassertfalse; break; // Can't write an object to a stream!
        default:            jassertfalse; break; // Is this a corrupted object?
    }
}

const var var::readFromStream (InputStream& input)
{
    const int numBytes = input.readCompressedInt();

    if (numBytes > 0)
    {
        switch (input.readByte())
        {
            case 1:     return var (input.readInt());
            case 2:     return var (true);
            case 3:     return var (false);
            case 4:     return var (input.readDouble());
            case 5:
            {
                MemoryBlock mb;
                input.readIntoMemoryBlock (mb, numBytes - 1);
                return var (String::fromUTF8 ((const uint8*) mb.getData(), (int) mb.getSize()));
            }

            default:    input.skipNextBytes (numBytes - 1); break;
        }
    }

    return var::null;
}

const var var::operator[] (const var::identifier& propertyName) const
{
    if (type == objectType && value.objectValue != 0)
        return value.objectValue->getProperty (propertyName);

    return var::null;
}

const var var::invoke (const var::identifier& method, const var* arguments, int numArguments) const
{
    if (type == objectType && value.objectValue != 0)
        return value.objectValue->invokeMethod (method, arguments, numArguments);

    return var::null;
}

const var var::invoke (const var& targetObject, const var* arguments, int numArguments) const
{
    if (isMethod())
    {
        DynamicObject* const target = targetObject.getObject();

        if (target != 0)
            return (target->*(value.methodValue)) (arguments, numArguments);
    }

    return var::null;
}

const var var::call (const var::identifier& method) const
{
    return invoke (method, 0, 0);
}

const var var::call (const var::identifier& method, const var& arg1) const
{
    return invoke (method, &arg1, 1);
}

const var var::call (const var::identifier& method, const var& arg1, const var& arg2) const
{
    var args[] = { arg1, arg2 };
    return invoke (method, args, 2);
}

const var var::call (const var::identifier& method, const var& arg1, const var& arg2, const var& arg3)
{
    var args[] = { arg1, arg2, arg3 };
    return invoke (method, args, 3);
}

const var var::call (const var::identifier& method, const var& arg1, const var& arg2, const var& arg3, const var& arg4) const
{
    var args[] = { arg1, arg2, arg3, arg4 };
    return invoke (method, args, 4);
}

const var var::call (const var::identifier& method, const var& arg1, const var& arg2, const var& arg3, const var& arg4, const var& arg5) const
{
    var args[] = { arg1, arg2, arg3, arg4, arg5 };
    return invoke (method, args, 5);
}


//==============================================================================
var::identifier::identifier() throw()
    : hashCode (0)
{
}

var::identifier::identifier (const String& name_)
    : name (name_),
      hashCode (name_.hashCode())
{
    jassert (name_.isNotEmpty());
}

var::identifier::identifier (const char* const name_)
    : name (name_),
      hashCode (name.hashCode())
{
    jassert (name.isNotEmpty());
}

var::identifier::~identifier()
{
}


END_JUCE_NAMESPACE
