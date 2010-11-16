/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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
#include "../io/streams/juce_MemoryOutputStream.h"


//==============================================================================
class var::VariantType
{
public:
    VariantType() {}
    virtual ~VariantType() {}

    virtual int toInt (const ValueUnion&) const                 { return 0; }
    virtual double toDouble (const ValueUnion&) const           { return 0; }
    virtual const String toString (const ValueUnion&) const     { return String::empty; }
    virtual bool toBool (const ValueUnion&) const               { return false; }
    virtual DynamicObject* toObject (const ValueUnion&) const   { return 0; }

    virtual bool isVoid() const throw()                         { return false; }
    virtual bool isInt() const throw()                          { return false; }
    virtual bool isBool() const throw()                         { return false; }
    virtual bool isDouble() const throw()                       { return false; }
    virtual bool isString() const throw()                       { return false; }
    virtual bool isObject() const throw()                       { return false; }
    virtual bool isMethod() const throw()                       { return false; }

    virtual void cleanUp (ValueUnion&) const throw()  {}
    virtual void createCopy (ValueUnion& dest, const ValueUnion& source) const      { dest = source; }
    virtual bool equals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) const throw() = 0;
    virtual void writeToStream (const ValueUnion& data, OutputStream& output) const = 0;
};

//==============================================================================
class var::VariantType_Void  : public var::VariantType
{
public:
    VariantType_Void() {}
    static const VariantType_Void instance;

    bool isVoid() const throw()     { return true; }
    bool equals (const ValueUnion&, const ValueUnion&, const VariantType& otherType) const throw()  { return otherType.isVoid(); }
    void writeToStream (const ValueUnion&, OutputStream& output) const   { output.writeCompressedInt (0); }
};

//==============================================================================
class var::VariantType_Int  : public var::VariantType
{
public:
    VariantType_Int() {}
    static const VariantType_Int instance;

    int toInt (const ValueUnion& data) const                { return data.intValue; };
    double toDouble (const ValueUnion& data) const          { return (double) data.intValue; }
    const String toString (const ValueUnion& data) const    { return String (data.intValue); }
    bool toBool (const ValueUnion& data) const              { return data.intValue != 0; }
    bool isInt() const throw()                              { return true; }

    bool equals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) const throw()
    {
        return otherType.toInt (otherData) == data.intValue;
    }

    void writeToStream (const ValueUnion& data, OutputStream& output) const
    {
        output.writeCompressedInt (5);
        output.writeByte (1);
        output.writeInt (data.intValue);
    }
};

//==============================================================================
class var::VariantType_Double   : public var::VariantType
{
public:
    VariantType_Double() {}
    static const VariantType_Double instance;

    int toInt (const ValueUnion& data) const                { return (int) data.doubleValue; };
    double toDouble (const ValueUnion& data) const          { return data.doubleValue; }
    const String toString (const ValueUnion& data) const    { return String (data.doubleValue); }
    bool toBool (const ValueUnion& data) const              { return data.doubleValue != 0; }
    bool isDouble() const throw()                           { return true; }

    bool equals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) const throw()
    {
        return otherType.toDouble (otherData) == data.doubleValue;
    }

    void writeToStream (const ValueUnion& data, OutputStream& output) const
    {
        output.writeCompressedInt (9);
        output.writeByte (4);
        output.writeDouble (data.doubleValue);
    }
};

//==============================================================================
class var::VariantType_Bool   : public var::VariantType
{
public:
    VariantType_Bool() {}
    static const VariantType_Bool instance;

    int toInt (const ValueUnion& data) const                { return data.boolValue ? 1 : 0; };
    double toDouble (const ValueUnion& data) const          { return data.boolValue ? 1.0 : 0.0; }
    const String toString (const ValueUnion& data) const    { return String::charToString (data.boolValue ? '1' : '0'); }
    bool toBool (const ValueUnion& data) const              { return data.boolValue; }
    bool isBool() const throw()                             { return true; }

    bool equals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) const throw()
    {
        return otherType.toBool (otherData) == data.boolValue;
    }

    void writeToStream (const ValueUnion& data, OutputStream& output) const
    {
        output.writeCompressedInt (1);
        output.writeByte (data.boolValue ? 2 : 3);
    }
};

//==============================================================================
class var::VariantType_String   : public var::VariantType
{
public:
    VariantType_String() {}
    static const VariantType_String instance;

    void cleanUp (ValueUnion& data) const throw()                        { delete data.stringValue; }
    void createCopy (ValueUnion& dest, const ValueUnion& source) const   { dest.stringValue = new String (*source.stringValue); }

    int toInt (const ValueUnion& data) const                { return data.stringValue->getIntValue(); };
    double toDouble (const ValueUnion& data) const          { return data.stringValue->getDoubleValue(); }
    const String toString (const ValueUnion& data) const    { return *data.stringValue; }
    bool toBool (const ValueUnion& data) const              { return data.stringValue->getIntValue() != 0
                                                                      || data.stringValue->trim().equalsIgnoreCase ("true")
                                                                      || data.stringValue->trim().equalsIgnoreCase ("yes"); }
    bool isString() const throw()                           { return true; }

    bool equals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) const throw()
    {
        return otherType.toString (otherData) == *data.stringValue;
    }

    void writeToStream (const ValueUnion& data, OutputStream& output) const
    {
        const int len = data.stringValue->getNumBytesAsUTF8() + 1;
        output.writeCompressedInt (len + 1);
        output.writeByte (5);
        HeapBlock<char> temp (len);
        data.stringValue->copyToUTF8 (temp, len);
        output.write (temp, len);
    }
};

//==============================================================================
class var::VariantType_Object   : public var::VariantType
{
public:
    VariantType_Object() {}
    static const VariantType_Object instance;

    void cleanUp (ValueUnion& data) const throw()                        { if (data.objectValue != 0) data.objectValue->decReferenceCount(); }
    void createCopy (ValueUnion& dest, const ValueUnion& source) const   { dest.objectValue = source.objectValue; if (dest.objectValue != 0) dest.objectValue->incReferenceCount(); }

    const String toString (const ValueUnion& data) const    { return "Object 0x" + String::toHexString ((int) (pointer_sized_int) data.objectValue); }
    bool toBool (const ValueUnion& data) const              { return data.objectValue != 0; }
    DynamicObject* toObject (const ValueUnion& data) const  { return data.objectValue; }
    bool isObject() const throw()                           { return true; }

    bool equals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) const throw()
    {
        return otherType.toObject (otherData) == data.objectValue;
    }

    void writeToStream (const ValueUnion&, OutputStream& output) const
    {
        jassertfalse; // Can't write an object to a stream!
        output.writeCompressedInt (0);
    }
};

//==============================================================================
class var::VariantType_Method   : public var::VariantType
{
public:
    VariantType_Method() {}
    static const VariantType_Method instance;

    const String toString (const ValueUnion&) const         { return "Method"; }
    bool toBool (const ValueUnion& data) const              { return data.methodValue != 0; }
    bool isMethod() const throw()                           { return true; }

    bool equals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) const throw()
    {
        return otherType.isMethod() && otherData.methodValue == data.methodValue;
    }

    void writeToStream (const ValueUnion&, OutputStream& output) const
    {
        jassertfalse; // Can't write a method to a stream!
        output.writeCompressedInt (0);
    }
};

//==============================================================================
const var::VariantType_Void    var::VariantType_Void::instance;
const var::VariantType_Int     var::VariantType_Int::instance;
const var::VariantType_Bool    var::VariantType_Bool::instance;
const var::VariantType_Double  var::VariantType_Double::instance;
const var::VariantType_String  var::VariantType_String::instance;
const var::VariantType_Object  var::VariantType_Object::instance;
const var::VariantType_Method  var::VariantType_Method::instance;


//==============================================================================
var::var() throw()  : type (&VariantType_Void::instance)
{
}

var::~var() throw()
{
    type->cleanUp (value);
}

const var var::null;

//==============================================================================
var::var (const var& valueToCopy)  : type (valueToCopy.type)
{
    type->createCopy (value, valueToCopy.value);
}

var::var (const int value_) throw() : type (&VariantType_Int::instance)
{
    value.intValue = value_;
}

var::var (const bool value_) throw()  : type (&VariantType_Bool::instance)
{
    value.boolValue = value_;
}

var::var (const double value_) throw()  : type (&VariantType_Double::instance)
{
    value.doubleValue = value_;
}

var::var (const String& value_)  : type (&VariantType_String::instance)
{
    value.stringValue = new String (value_);
}

var::var (const char* const value_)  : type (&VariantType_String::instance)
{
    value.stringValue = new String (value_);
}

var::var (const juce_wchar* const value_)  : type (&VariantType_String::instance)
{
    value.stringValue = new String (value_);
}

var::var (DynamicObject* const object)  : type (&VariantType_Object::instance)
{
    value.objectValue = object;

    if (object != 0)
        object->incReferenceCount();
}

var::var (MethodFunction method_) throw()  : type (&VariantType_Method::instance)
{
    value.methodValue = method_;
}

//==============================================================================
bool var::isVoid() const throw()                { return type->isVoid(); }
bool var::isInt() const throw()                 { return type->isInt(); }
bool var::isBool() const throw()                { return type->isBool(); }
bool var::isDouble() const throw()              { return type->isDouble(); }
bool var::isString() const throw()              { return type->isString(); }
bool var::isObject() const throw()              { return type->isObject(); }
bool var::isMethod() const throw()              { return type->isMethod(); }

var::operator int() const                       { return type->toInt (value); }
var::operator bool() const                      { return type->toBool (value); }
var::operator float() const                     { return (float) type->toDouble (value); }
var::operator double() const                    { return type->toDouble (value); }
const String var::toString() const              { return type->toString (value); }
var::operator const String() const              { return type->toString (value); }
DynamicObject* var::getObject() const           { return type->toObject (value); }

//==============================================================================
void var::swapWith (var& other) throw()
{
    swapVariables (type, other.type);
    swapVariables (value, other.value);
}

var& var::operator= (const var& newValue)         { type->cleanUp (value); type = newValue.type; type->createCopy (value, newValue.value); return *this; }
var& var::operator= (int newValue)                { var v (newValue); swapWith (v); return *this; }
var& var::operator= (bool newValue)               { var v (newValue); swapWith (v); return *this; }
var& var::operator= (double newValue)             { var v (newValue); swapWith (v); return *this; }
var& var::operator= (const char* newValue)        { var v (newValue); swapWith (v); return *this; }
var& var::operator= (const juce_wchar* newValue)  { var v (newValue); swapWith (v); return *this; }
var& var::operator= (const String& newValue)      { var v (newValue); swapWith (v); return *this; }
var& var::operator= (DynamicObject* newValue)     { var v (newValue); swapWith (v); return *this; }
var& var::operator= (MethodFunction newValue)     { var v (newValue); swapWith (v); return *this; }

//==============================================================================
bool var::equals (const var& other) const throw()
{
    return type->equals (value, other.value, *other.type);
}

bool operator== (const var& v1, const var& v2) throw()      { return v1.equals (v2); }
bool operator!= (const var& v1, const var& v2) throw()      { return ! v1.equals (v2); }
bool operator== (const var& v1, const String& v2) throw()   { return v1.toString() == v2; }
bool operator!= (const var& v1, const String& v2) throw()   { return v1.toString() != v2; }

//==============================================================================
void var::writeToStream (OutputStream& output) const
{
    type->writeToStream (value, output);
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
                MemoryOutputStream mo;
                mo.writeFromInputStream (input, numBytes - 1);
                return var (mo.toUTF8());
            }

            default:    input.skipNextBytes (numBytes - 1); break;
        }
    }

    return var::null;
}

const var var::operator[] (const Identifier& propertyName) const
{
    DynamicObject* const o = getObject();
    return o != 0 ? o->getProperty (propertyName) : var::null;
}

const var var::invoke (const Identifier& method, const var* arguments, int numArguments) const
{
    DynamicObject* const o = getObject();
    return o != 0 ? o->invokeMethod (method, arguments, numArguments) : var::null;
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

const var var::call (const Identifier& method) const
{
    return invoke (method, 0, 0);
}

const var var::call (const Identifier& method, const var& arg1) const
{
    return invoke (method, &arg1, 1);
}

const var var::call (const Identifier& method, const var& arg1, const var& arg2) const
{
    var args[] = { arg1, arg2 };
    return invoke (method, args, 2);
}

const var var::call (const Identifier& method, const var& arg1, const var& arg2, const var& arg3)
{
    var args[] = { arg1, arg2, arg3 };
    return invoke (method, args, 3);
}

const var var::call (const Identifier& method, const var& arg1, const var& arg2, const var& arg3, const var& arg4) const
{
    var args[] = { arg1, arg2, arg3, arg4 };
    return invoke (method, args, 4);
}

const var var::call (const Identifier& method, const var& arg1, const var& arg2, const var& arg3, const var& arg4, const var& arg5) const
{
    var args[] = { arg1, arg2, arg3, arg4, arg5 };
    return invoke (method, args, 5);
}


END_JUCE_NAMESPACE
