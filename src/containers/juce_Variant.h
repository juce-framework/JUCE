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

#ifndef __JUCE_VARIANT_JUCEHEADER__
#define __JUCE_VARIANT_JUCEHEADER__

#include "../io/streams/juce_OutputStream.h"
#include "../io/streams/juce_InputStream.h"

class JUCE_API  DynamicObject;


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

    //==============================================================================
    /** Creates a void variant. */
    var() throw();

    /** Destructor. */
    ~var() throw();

    /** A static var object that can be used where you need an empty variant object. */
    static const var null;

    var (const var& valueToCopy);
    var (const int value) throw();
    var (const bool value) throw();
    var (const double value) throw();
    var (const char* const value);
    var (const juce_wchar* const value);
    var (const String& value);
    var (DynamicObject* const object);
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

    bool isVoid() const throw()         { return type == voidType; }
    bool isInt() const throw()          { return type == intType; }
    bool isBool() const throw()         { return type == boolType; }
    bool isDouble() const throw()       { return type == doubleType; }
    bool isString() const throw()       { return type == stringType; }
    bool isObject() const throw()       { return type == objectType; }
    bool isMethod() const throw()       { return type == methodType; }

    bool operator== (const var& other) const throw();
    bool operator!= (const var& other) const throw();

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
    class JUCE_API  identifier
    {
    public:
        /** Creates a null identifier. */
        identifier() throw();

        /** Creates an identifier with a specified name.
            Because this name may need to be used in contexts such as script variables or XML
            tags, it must only contain ascii letters and digits, or the underscore character.
        */
        identifier (const char* const name);

        /** Creates an identifier with a specified name.
            Because this name may need to be used in contexts such as script variables or XML
            tags, it must only contain ascii letters and digits, or the underscore character.
        */
        identifier (const String& name);

        /** Destructor */
        ~identifier();

        bool operator== (const identifier& other) const throw()
        {
            jassert (hashCode != other.hashCode || name == other.name); // check for name hash collisions
            return hashCode == other.hashCode;
        }

        String name;
        int hashCode;
    };

    /** If this variant is an object, this returns one of its properties. */
    const var operator[] (const identifier& propertyName) const;

    //==============================================================================
    /** If this variant is an object, this invokes one of its methods with no arguments. */
    const var call (const identifier& method) const;
    /** If this variant is an object, this invokes one of its methods with one argument. */
    const var call (const identifier& method, const var& arg1) const;
    /** If this variant is an object, this invokes one of its methods with 2 arguments. */
    const var call (const identifier& method, const var& arg1, const var& arg2) const;
    /** If this variant is an object, this invokes one of its methods with 3 arguments. */
    const var call (const identifier& method, const var& arg1, const var& arg2, const var& arg3);
    /** If this variant is an object, this invokes one of its methods with 4 arguments. */
    const var call (const identifier& method, const var& arg1, const var& arg2, const var& arg3, const var& arg4) const;
    /** If this variant is an object, this invokes one of its methods with 5 arguments. */
    const var call (const identifier& method, const var& arg1, const var& arg2, const var& arg3, const var& arg4, const var& arg5) const;

    /** If this variant is an object, this invokes one of its methods with a list of arguments. */
    const var invoke (const identifier& method, const var* arguments, int numArguments) const;

    //==============================================================================
    /** If this variant is a method pointer, this invokes it on a target object. */
    const var invoke (const var& targetObject, const var* arguments, int numArguments) const;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    enum Type
    {
        voidType = 0,
        intType,
        boolType,
        doubleType,
        stringType,
        objectType,
        methodType
    };

    union ValueUnion
    {
        int intValue;
        bool boolValue;
        double doubleValue;
        String* stringValue;
        DynamicObject* objectValue;
        MethodFunction methodValue;
    };

    Type type;
    ValueUnion value;
};


#endif   // __JUCE_VARIANT_JUCEHEADER__
