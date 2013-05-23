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
#include "../streams/juce_OutputStream.h"
#include "../streams/juce_InputStream.h"
#include "../containers/juce_Array.h"

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

    You can save/load var objects either in a small, proprietary binary format
    using writeToStream()/readFromStream(), or as JSON by using the JSON class.

    @see JSON, DynamicObject
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
    var (const Array<var>& value);
    var (ReferenceCountedObject* object);
    var (MethodFunction method) noexcept;
    var (const void* binaryData, size_t dataSize);
    var (const MemoryBlock& binaryData);

    var& operator= (const var& valueToCopy);
    var& operator= (int value);
    var& operator= (int64 value);
    var& operator= (bool value);
    var& operator= (double value);
    var& operator= (const char* value);
    var& operator= (const wchar_t* value);
    var& operator= (const String& value);
    var& operator= (const Array<var>& value);
    var& operator= (ReferenceCountedObject* object);
    var& operator= (MethodFunction method);

   #if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
    var (var&& other) noexcept;
    var (String&& value);
    var (MemoryBlock&& binaryData);
    var& operator= (var&& other) noexcept;
    var& operator= (String&& value);
   #endif

    void swapWith (var& other) noexcept;

    //==============================================================================
    operator int() const noexcept;
    operator int64() const noexcept;
    operator bool() const noexcept;
    operator float() const noexcept;
    operator double() const noexcept;
    operator String() const;
    String toString() const;

    /** If this variant holds an array, this provides access to it.
        NOTE: Beware when you use this - the array pointer is only valid for the lifetime
        of the variant that returned it, so be very careful not to call this method on temporary
        var objects that are the return-value of a function, and which may go out of scope before
        you use the array!
    */
    Array<var>* getArray() const noexcept;

    /** If this variant holds a memory block, this provides access to it.
        NOTE: Beware when you use this - the MemoryBlock pointer is only valid for the lifetime
        of the variant that returned it, so be very careful not to call this method on temporary
        var objects that are the return-value of a function, and which may go out of scope before
        you use the MemoryBlock!
    */
    MemoryBlock* getBinaryData() const noexcept;

    ReferenceCountedObject* getObject() const noexcept;
    DynamicObject* getDynamicObject() const noexcept;

    //==============================================================================
    bool isVoid() const noexcept;
    bool isInt() const noexcept;
    bool isInt64() const noexcept;
    bool isBool() const noexcept;
    bool isDouble() const noexcept;
    bool isString() const noexcept;
    bool isObject() const noexcept;
    bool isArray() const noexcept;
    bool isBinaryData() const noexcept;
    bool isMethod() const noexcept;

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

    //==============================================================================
    /** If the var is an array, this returns the number of elements.
        If the var isn't actually an array, this will return 0.
    */
    int size() const;

    /** If the var is an array, this can be used to return one of its elements.
        To call this method, you must make sure that the var is actually an array, and
        that the index is a valid number. If these conditions aren't met, behaviour is
        undefined.
        For more control over the array's contents, you can call getArray() and manipulate
        it directly as an Array\<var\>.
    */
    const var& operator[] (int arrayIndex) const;

    /** If the var is an array, this can be used to return one of its elements.
        To call this method, you must make sure that the var is actually an array, and
        that the index is a valid number. If these conditions aren't met, behaviour is
        undefined.
        For more control over the array's contents, you can call getArray() and manipulate
        it directly as an Array\<var\>.
    */
    var& operator[] (int arrayIndex);

    /** Appends an element to the var, converting it to an array if it isn't already one.
        If the var isn't an array, it will be converted to one, and if its value was non-void,
        this value will be kept as the first element of the new array. The parameter value
        will then be appended to it.
        For more control over the array's contents, you can call getArray() and manipulate
        it directly as an Array\<var\>.
    */
    void append (const var& valueToAppend);

    /** Inserts an element to the var, converting it to an array if it isn't already one.
        If the var isn't an array, it will be converted to one, and if its value was non-void,
        this value will be kept as the first element of the new array. The parameter value
        will then be inserted into it.
        For more control over the array's contents, you can call getArray() and manipulate
        it directly as an Array\<var\>.
    */
    void insert (int index, const var& value);

    /** If the var is an array, this removes one of its elements.
        If the index is out-of-range or the var isn't an array, nothing will be done.
        For more control over the array's contents, you can call getArray() and manipulate
        it directly as an Array\<var\>.
    */
    void remove (int index);

    /** Treating the var as an array, this resizes it to contain the specified number of elements.
        If the var isn't an array, it will be converted to one, and if its value was non-void,
        this value will be kept as the first element of the new array before resizing.
        For more control over the array's contents, you can call getArray() and manipulate
        it directly as an Array\<var\>.
    */
    void resize (int numArrayElementsWanted);

    /** If the var is an array, this searches it for the first occurrence of the specified value,
        and returns its index.
        If the var isn't an array, or if the value isn't found, this returns -1.
    */
    int indexOf (const var& value) const;

    //==============================================================================
    /** If this variant is an object, this returns one of its properties. */
    var operator[] (const Identifier propertyName) const;
    /** If this variant is an object, this returns one of its properties. */
    var operator[] (const char* propertyName) const;
    /** If this variant is an object, this returns one of its properties, or a default
        fallback value if the property is not set. */
    var getProperty (const Identifier propertyName, const var& defaultReturnValue) const;

    /** If this variant is an object, this invokes one of its methods with no arguments. */
    var call (const Identifier method) const;
    /** If this variant is an object, this invokes one of its methods with one argument. */
    var call (const Identifier method, const var& arg1) const;
    /** If this variant is an object, this invokes one of its methods with 2 arguments. */
    var call (const Identifier method, const var& arg1, const var& arg2) const;
    /** If this variant is an object, this invokes one of its methods with 3 arguments. */
    var call (const Identifier method, const var& arg1, const var& arg2, const var& arg3);
    /** If this variant is an object, this invokes one of its methods with 4 arguments. */
    var call (const Identifier method, const var& arg1, const var& arg2, const var& arg3, const var& arg4) const;
    /** If this variant is an object, this invokes one of its methods with 5 arguments. */
    var call (const Identifier method, const var& arg1, const var& arg2, const var& arg3, const var& arg4, const var& arg5) const;
    /** If this variant is an object, this invokes one of its methods with a list of arguments. */
    var invoke (const Identifier method, const var* arguments, int numArguments) const;

    //==============================================================================
    /** Writes a binary representation of this value to a stream.
        The data can be read back later using readFromStream().
        @see JSON
    */
    void writeToStream (OutputStream& output) const;

    /** Reads back a stored binary representation of a value.
        The data in the stream must have been written using writeToStream(), or this
        will have unpredictable results.
        @see JSON
    */
    static var readFromStream (InputStream& input);

private:
    //==============================================================================
    class VariantType;         friend class VariantType;
    class VariantType_Void;    friend class VariantType_Void;
    class VariantType_Int;     friend class VariantType_Int;
    class VariantType_Int64;   friend class VariantType_Int64;
    class VariantType_Double;  friend class VariantType_Double;
    class VariantType_Bool;    friend class VariantType_Bool;
    class VariantType_String;  friend class VariantType_String;
    class VariantType_Object;  friend class VariantType_Object;
    class VariantType_Array;   friend class VariantType_Array;
    class VariantType_Binary;  friend class VariantType_Binary;
    class VariantType_Method;  friend class VariantType_Method;

    union ValueUnion
    {
        int intValue;
        int64 int64Value;
        bool boolValue;
        double doubleValue;
        char stringValue [sizeof (String)];
        ReferenceCountedObject* objectValue;
        Array<var>* arrayValue;
        MemoryBlock* binaryValue;
        MethodFunction methodValue;
    };

    const VariantType* type;
    ValueUnion value;

    Array<var>* convertToArray();
    friend class DynamicObject;
    var invokeMethod (DynamicObject*, const var*, int) const;
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
