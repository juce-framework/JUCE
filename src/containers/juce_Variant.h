/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCE_VARIANT_JUCEHEADER__
#define __JUCE_VARIANT_JUCEHEADER__

#include "../text/juce_Identifier.h"
#include "../io/streams/juce_OutputStream.h"
#include "../io/streams/juce_InputStream.h"

#ifndef DOXYGEN
 class ReferenceCountedObject;
 class DynamicObject;
#endif

//==============================================================================
/**
    A variant class, that can be used to hold a range of primitive values.

    A var object can hold a range of simple primitive values, strings, or
    any kind of ReferenceCountedObject. The var class is intended to act like
    the kind of values used in dynamic scripting languages.

    @see DynamicObject
*/
class JUCE_API  var
{
public:
    //==============================================================================
    typedef const var (DynamicObject::*MethodFunction) (const var* arguments, int numArguments);
    typedef Identifier identifier;

    //==============================================================================
    /** Creates a void variant. */
    var() noexcept;

    /** Destructor. */
    ~var() noexcept;

    /** A static var object that can be used where you need an empty variant object. */
    static const var null;

    var (const var& valueToCopy);
    var (int value) noexcept;
    var (int64 value) noexcept;
    var (bool value) noexcept;
    var (double value) noexcept;
    var (const char* value);
    var (const wchar_t* value);
    var (const String& value);
    var (ReferenceCountedObject* object);
    var (MethodFunction method) noexcept;

    const var& operator= (const var& valueToCopy);
    const var& operator= (int value);
    const var& operator= (int64 value);
    const var& operator= (bool value);
    const var& operator= (double value);
    const var& operator= (const char* value);
    const var& operator= (const wchar_t* value);
    const var& operator= (const String& value);
    const var& operator= (ReferenceCountedObject* object);
    const var& operator= (MethodFunction method);

    void swapWith (var& other) noexcept;

    operator int() const noexcept;
    operator int64() const noexcept;
    operator bool() const noexcept;
    operator float() const noexcept;
    operator double() const noexcept;
    operator const String() const;
    const String toString() const;
    ReferenceCountedObject* getObject() const noexcept;
    DynamicObject* getDynamicObject() const noexcept;

    bool isVoid() const noexcept;
    bool isInt() const noexcept;
    bool isInt64() const noexcept;
    bool isBool() const noexcept;
    bool isDouble() const noexcept;
    bool isString() const noexcept;
    bool isObject() const noexcept;
    bool isMethod() const noexcept;

    //==============================================================================
    /** Writes a binary representation of this value to a stream.
        The data can be read back later using readFromStream().
    */
    void writeToStream (OutputStream& output) const;

    /** Reads back a stored binary representation of a value.
        The data in the stream must have been written using writeToStream(), or this
        will have unpredictable results.
    */
    static const var readFromStream (InputStream& input);

    //==============================================================================
    /** If this variant is an object, this returns one of its properties. */
    const var operator[] (const Identifier& propertyName) const;

    //==============================================================================
    /** If this variant is an object, this invokes one of its methods with no arguments. */
    const var call (const Identifier& method) const;
    /** If this variant is an object, this invokes one of its methods with one argument. */
    const var call (const Identifier& method, const var& arg1) const;
    /** If this variant is an object, this invokes one of its methods with 2 arguments. */
    const var call (const Identifier& method, const var& arg1, const var& arg2) const;
    /** If this variant is an object, this invokes one of its methods with 3 arguments. */
    const var call (const Identifier& method, const var& arg1, const var& arg2, const var& arg3);
    /** If this variant is an object, this invokes one of its methods with 4 arguments. */
    const var call (const Identifier& method, const var& arg1, const var& arg2, const var& arg3, const var& arg4) const;
    /** If this variant is an object, this invokes one of its methods with 5 arguments. */
    const var call (const Identifier& method, const var& arg1, const var& arg2, const var& arg3, const var& arg4, const var& arg5) const;

    /** If this variant is an object, this invokes one of its methods with a list of arguments. */
    const var invoke (const Identifier& method, const var* arguments, int numArguments) const;

    //==============================================================================
    /** If this variant is a method pointer, this invokes it on a target object. */
    const var invoke (const var& targetObject, const var* arguments, int numArguments) const;

    //==============================================================================
    /** Returns true if this var has the same value as the one supplied.
        Note that this ignores the type, so a string var "123" and an integer var with the
        value 123 are considered to be equal.
        @see equalsWithSameType
    */
    bool equals (const var& other) const noexcept;

    /** Returns true if this var has the same value and type as the one supplied.
        This differs from equals() because e.g. "123" and 123 will be considered different.
        @see equals
    */
    bool equalsWithSameType (const var& other) const noexcept;

private:
    class VariantType;
    friend class VariantType;
    class VariantType_Void;
    friend class VariantType_Void;
    class VariantType_Int;
    friend class VariantType_Int;
    class VariantType_Int64;
    friend class VariantType_Int64;
    class VariantType_Double;
    friend class VariantType_Double;
    class VariantType_Float;
    friend class VariantType_Float;
    class VariantType_Bool;
    friend class VariantType_Bool;
    class VariantType_String;
    friend class VariantType_String;
    class VariantType_Object;
    friend class VariantType_Object;
    class VariantType_Method;
    friend class VariantType_Method;

    union ValueUnion
    {
        int intValue;
        int64 int64Value;
        bool boolValue;
        double doubleValue;
        String* stringValue;
        ReferenceCountedObject* objectValue;
        MethodFunction methodValue;
    };

    const VariantType* type;
    ValueUnion value;
};

/** Compares the values of two var objects, using the var::equals() comparison. */
bool operator== (const var& v1, const var& v2) noexcept;
/** Compares the values of two var objects, using the var::equals() comparison. */
bool operator!= (const var& v1, const var& v2) noexcept;
bool operator== (const var& v1, const String& v2);
bool operator!= (const var& v1, const String& v2);
bool operator== (const var& v1, const char* v2);
bool operator!= (const var& v1, const char* v2);


#endif   // __JUCE_VARIANT_JUCEHEADER__
