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

#include "juce_ReferenceCountedObject.h"
#include "juce_OwnedArray.h"
#include "../text/juce_StringArray.h"
#include "../containers/juce_Array.h"
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
    ~var();

    var (const var& valueToCopy) throw();
    var (const int value) throw();
    var (const bool value) throw();
    var (const double value) throw();
    var (const char* const value) throw();
    var (const juce_wchar* const value) throw();
    var (const String& value) throw();
    var (DynamicObject* const object) throw();
    var (MethodFunction method) throw();

    const var& operator= (const var& valueToCopy) throw();
    const var& operator= (const int value) throw();
    const var& operator= (const bool value) throw();
    const var& operator= (const double value) throw();
    const var& operator= (const char* const value) throw();
    const var& operator= (const juce_wchar* const value) throw();
    const var& operator= (const String& value) throw();
    const var& operator= (DynamicObject* const object) throw();
    const var& operator= (MethodFunction method) throw();

    operator int() const throw();
    operator bool() const throw();
    operator float() const throw();
    operator double() const throw();
    operator const String() const throw();
    const String toString() const throw();
    DynamicObject* getObject() const throw();

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
    void writeToStream (OutputStream& output) const throw();

    /** Reads back a stored binary representation of a value.
        The data in the stream must have been written using writeToStream(), or this
        will have unpredictable results.
    */
    static const var readFromStream (InputStream& input) throw();

    //==============================================================================
    class JUCE_API  identifier
    {
    public:
        identifier (const char* const name) throw();
        identifier (const String& name) throw();
        ~identifier() throw();

        bool operator== (const identifier& other) const throw()
        {
            jassert (hashCode != other.hashCode || name == other.name); // check for name hash collisions
            return hashCode == other.hashCode;
        }

        String name;
        int hashCode;
    };

    /** If this variant is an object, this returns one of its properties. */
    const var operator[] (const identifier& propertyName) const throw();

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

    Type type;

    union
    {
        int intValue;
        bool boolValue;
        double doubleValue;
        String* stringValue;
        DynamicObject* objectValue;
        MethodFunction methodValue;
    } value;

    void releaseValue() throw();
};

//==============================================================================
/**
    Represents a dynamically implemented object.

    An instance of this class can be used to store named properties, and
    by subclassing hasMethod() and invokeMethod(), you can give your object
    methods.

    This is intended for use as a wrapper for scripting language objects.
*/
class JUCE_API  DynamicObject  : public ReferenceCountedObject
{
public:
    //==============================================================================
    DynamicObject();

    /** Destructor. */
    virtual ~DynamicObject();

    //==============================================================================
    /** Returns true if the object has a property with this name.
        Note that if the property is actually a method, this will return false.
    */
    virtual bool hasProperty (const var::identifier& propertyName) const;

    /** Returns a named property.

        This returns a void if no such property exists.
    */
    virtual const var getProperty (const var::identifier& propertyName) const;

    /** Sets a named property. */
    virtual void setProperty (const var::identifier& propertyName, const var& newValue);

    /** Removes a named property. */
    virtual void removeProperty (const var::identifier& propertyName);

    //==============================================================================
    /** Checks whether this object has the specified method.

        The default implementation of this just checks whether there's a property
        with this name that's actually a method, but this can be overridden for
        building objects with dynamic invocation.
    */
    virtual bool hasMethod (const var::identifier& methodName) const;

    /** Invokes a named method on this object.

        The default implementation looks up the named property, and if it's a method
        call, then it invokes it.

        This method is virtual to allow more dynamic invocation to used for objects
        where the methods may not already be set as properies.
    */
    virtual const var invokeMethod (const var::identifier& methodName,
                                    const var* parameters,
                                    int numParameters);

    /** Sets up a method.

        This is basically the same as calling setProperty (methodName, (var::MethodFunction) myFunction), but
        helps to avoid accidentally invoking the wrong type of var constructor. It also makes
        the code easier to read,

        The compiler will probably force you to use an explicit cast your method to a (var::MethodFunction), e.g.
        @code
        setMethod ("doSomething", (var::MethodFunction) &MyClass::doSomething);
        @endcode
    */
    void setMethod (const var::identifier& methodName,
                    var::MethodFunction methodFunction);

    //==============================================================================
    /** Removes all properties and methods from the object. */
    void clear();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    Array <int> propertyIds;
    OwnedArray <var> propertyValues;
};



#endif   // __JUCE_VARIANT_JUCEHEADER__
