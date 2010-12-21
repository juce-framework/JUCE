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

#ifndef __JUCE_VARIANT_JUCEHEADER__
#define __JUCE_VARIANT_JUCEHEADER__

#include "../text/juce_Identifier.h"
#include "../io/streams/juce_OutputStream.h"
#include "../io/streams/juce_InputStream.h"

#ifndef DOXYGEN
 class JUCE_API  DynamicObject;
#endif

//==============================================================================
/**
    A variant class, that can be used to hold a range of primitive values.

    A var object can hold a range of simple primitive values, strings, or
    a reference-counted pointer to a DynamicObject. The var class is intended
    to act like the values used in dynamic scripting languages.

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
    var() throw();

    /** Destructor. */
    ~var() throw();

    /** A static var object that can be used where you need an empty variant object. */
    static const var null;

    var (const var& valueToCopy);
    var (int value) throw();
    var (bool value) throw();
    var (double value) throw();
    var (const char* value);
    var (const juce_wchar* value);
    var (const String& value);
    var (DynamicObject* object);
    var (MethodFunction method) throw();

    var& operator= (const var& valueToCopy);
    var& operator= (int value);
    var& operator= (bool value);
    var& operator= (double value);
    var& operator= (const char* value);
    var& operator= (const juce_wchar* value);
    var& operator= (const String& value);
    var& operator= (DynamicObject* object);
    var& operator= (MethodFunction method);

    void swapWith (var& other) throw();

    operator int() const;
    operator bool() const;
    operator float() const;
    operator double() const;
    operator const String() const;
    const String toString() const;
    DynamicObject* getObject() const;

    bool isVoid() const throw();
    bool isInt() const throw();
    bool isBool() const throw();
    bool isDouble() const throw();
    bool isString() const throw();
    bool isObject() const throw();
    bool isMethod() const throw();

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
    /** Returns true if this var has the same value as the one supplied. */
    bool equals (const var& other) const throw();

private:
    class VariantType;
    friend class VariantType;
    class VariantType_Void;
    friend class VariantType_Void;
    class VariantType_Int;
    friend class VariantType_Int;
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
        bool boolValue;
        double doubleValue;
        String* stringValue;
        DynamicObject* objectValue;
        MethodFunction methodValue;
    };

    const VariantType* type;
    ValueUnion value;
};

bool operator== (const var& v1, const var& v2) throw();
bool operator!= (const var& v1, const var& v2) throw();
bool operator== (const var& v1, const String& v2) throw();
bool operator!= (const var& v1, const String& v2) throw();


#endif   // __JUCE_VARIANT_JUCEHEADER__
