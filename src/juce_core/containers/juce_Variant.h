/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCE_VARIANT_JUCEHEADER__
#define __JUCE_VARIANT_JUCEHEADER__

#include "juce_ReferenceCountedObject.h"
#include "juce_OwnedArray.h"
#include "../text/juce_StringArray.h"

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
    typedef const var (MethodFunction) (const var& thisObject, const var* arguments, int numArguments);

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
    operator double() const throw();
    const String toString() const throw();
    DynamicObject* getObject() const throw();

    bool isVoid() const throw()         { return type == voidType; }
    bool isInt() const throw()          { return type == intType; }
    bool isBool() const throw()         { return type == boolType; }
    bool isDouble() const throw()       { return type == doubleType; }
    bool isString() const throw()       { return type == stringType; }
    bool isObject() const throw()       { return type == objectType; }
    bool isMethod() const throw()       { return type == methodType; }

    //==============================================================================
    class identifier
    {
    public:
        identifier (const char* const name) throw();
        identifier (const String& name) throw();
        ~identifier() throw();

        bool operator== (const identifier& other) const throw()   { return hashCode == other.hashCode; }

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
        MethodFunction* methodValue;
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
    virtual bool hasProperty (const var::identifier& propertyName) const;
    virtual const var getProperty (const var::identifier& propertyName) const;
    virtual void setProperty (const var::identifier& propertyName, const var& newValue);
    virtual void removeProperty (const var::identifier& propertyName);

    //==============================================================================
    virtual bool hasMethod (const var::identifier& methodName) const;

    virtual const var invokeMethod (const var::identifier& methodName,
                                    const var* parameters,
                                    int numParameters);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    Array <int> propertyIds;
    OwnedArray <var> propertyValues;
};



#endif   // __JUCE_VARIANT_JUCEHEADER__
