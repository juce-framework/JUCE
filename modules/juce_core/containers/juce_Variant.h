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

//==============================================================================
/**
    A variant class, that can be used to hold a range of primitive values.

    A var object can hold a range of simple primitive values, strings, or
    any kind of ReferenceCountedObject. The var class is intended to act like
    the kind of values used in dynamic scripting languages.

    You can save/load var objects either in a small, proprietary binary format
    using writeToStream()/readFromStream(), or as JSON by using the JSON class.

    @see JSON, DynamicObject

    @tags{Core}
*/
class JUCE_API  var
{
public:
    //==============================================================================
    /** This structure is passed to a NativeFunction callback, and contains invocation
        details about the function's arguments and context.
    */
    struct JUCE_API  NativeFunctionArgs
    {
        NativeFunctionArgs (const var& thisObject, const var* args, int numArgs) noexcept;

        const var& thisObject;
        const var* arguments;
        int numArguments;
    };

    using NativeFunction = std::function<var (const NativeFunctionArgs&)>;

    //==============================================================================
    /** Creates a void variant. */
    var() noexcept;

    /** Destructor. */
    ~var() noexcept;

    var (const var& valueToCopy);
    var (int value) noexcept;
    var (int64 value) noexcept;
    var (bool value) noexcept;
    var (double value) noexcept;
    var (const char* value);
    var (const wchar_t* value);
    var (const String& value);
    var (const Array<var>& value);
    var (const StringArray& value);
    var (ReferenceCountedObject* object);
    var (NativeFunction method) noexcept;
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
    var& operator= (const MemoryBlock& value);
    var& operator= (const Array<var>& value);
    var& operator= (ReferenceCountedObject* object);
    var& operator= (NativeFunction method);

    var (var&&) noexcept;
    var (String&&);
    var (MemoryBlock&&);
    var (Array<var>&&);
    var& operator= (var&&) noexcept;
    var& operator= (String&&);

    void swapWith (var& other) noexcept;

    /** Returns a var object that can be used where you need the javascript "undefined" value. */
    static var undefined() noexcept;

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
    bool isUndefined() const noexcept;
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

        Note that equality checking depends on the "wrapped" type of the object on which
        equals() is called. That means the following code will convert the right-hand-side
        argument to a string and compare the string values, because the object on the
        left-hand-side was initialised from a string:
        @code var ("123").equals (var (123)) @endcode
        However, the following code will convert the right-hand-side argument to a double
        and compare the values as doubles, because the object on the left-hand-side was
        initialised from a double:
        @code var (45.6).equals ("45.6000") @endcode

        @see equalsWithSameType
    */
    bool equals (const var& other) const noexcept;

    /** Returns true if this var has the same value and type as the one supplied.
        This differs from equals() because e.g. "123" and 123 will be considered different.
        @see equals
    */
    bool equalsWithSameType (const var& other) const noexcept;

    /** Returns true if this var has the same type as the one supplied. */
    bool hasSameTypeAs (const var& other) const noexcept;

    /** Returns a deep copy of this object.
        For simple types this just returns a copy, but if the object contains any arrays
        or DynamicObjects, they will be cloned (recursively).
    */
    var clone() const noexcept;

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
    const var& operator[] (const Identifier& propertyName) const;
    /** If this variant is an object, this returns one of its properties. */
    const var& operator[] (const char* propertyName) const;
    /** If this variant is an object, this returns one of its properties, or a default
        fallback value if the property is not set. */
    var getProperty (const Identifier& propertyName, const var& defaultReturnValue) const;
    /** Returns true if this variant is an object and if it has the given property. */
    bool hasProperty (const Identifier& propertyName) const noexcept;

    /** Invokes a named method call with no arguments. */
    var call (const Identifier& method) const;
    /** Invokes a named method call with one argument. */
    var call (const Identifier& method, const var& arg1) const;
    /** Invokes a named method call with 2 arguments. */
    var call (const Identifier& method, const var& arg1, const var& arg2) const;
    /** Invokes a named method call with 3 arguments. */
    var call (const Identifier& method, const var& arg1, const var& arg2, const var& arg3);
    /** Invokes a named method call with 4 arguments. */
    var call (const Identifier& method, const var& arg1, const var& arg2, const var& arg3, const var& arg4) const;
    /** Invokes a named method call with 5 arguments. */
    var call (const Identifier& method, const var& arg1, const var& arg2, const var& arg3, const var& arg4, const var& arg5) const;
    /** Invokes a named method call with a list of arguments. */
    var invoke (const Identifier& method, const var* arguments, int numArguments) const;
    /** If this object is a method, this returns the function pointer. */
    NativeFunction getNativeFunction() const;

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

    //==============================================================================
   #if JUCE_ALLOW_STATIC_NULL_VARIABLES
    /** @cond */
    [[deprecated ("This was a static empty var object, but is now deprecated as it's too easy to accidentally "
                 "use it indirectly during a static constructor leading to hard-to-find order-of-initialisation "
                 "problems. Use var() or {} instead. For returning an empty var from a function by reference, "
                 "use a function-local static var and return that.")]]
    static const var null;
    /** @endcond */
   #endif

private:
    //==============================================================================
    struct VariantType;
    struct Instance;

    union ValueUnion
    {
        int intValue;
        int64 int64Value;
        bool boolValue;
        double doubleValue;
        char stringValue[sizeof (String)];
        ReferenceCountedObject* objectValue;
        MemoryBlock* binaryValue;
        NativeFunction* methodValue;
    };

    friend bool canCompare (const var&, const var&);

    const VariantType* type;
    ValueUnion value;

    Array<var>* convertToArray();
    var (const VariantType&) noexcept;

    // This is needed to prevent the wrong constructor/operator being called
    var (const ReferenceCountedObject*) = delete;
    var& operator= (const ReferenceCountedObject*) = delete;
    var (const void*) = delete;
    var& operator= (const void*) = delete;
};

/** Compares the values of two var objects, using the var::equals() comparison. */
JUCE_API bool operator== (const var&, const var&);
/** Compares the values of two var objects, using the var::equals() comparison. */
JUCE_API bool operator!= (const var&, const var&);
/** Compares the values of two var objects, using the var::equals() comparison. */
JUCE_API bool operator<  (const var&, const var&);
/** Compares the values of two var objects, using the var::equals() comparison. */
JUCE_API bool operator<= (const var&, const var&);
/** Compares the values of two var objects, using the var::equals() comparison. */
JUCE_API bool operator>  (const var&, const var&);
/** Compares the values of two var objects, using the var::equals() comparison. */
JUCE_API bool operator>= (const var&, const var&);

JUCE_API bool operator== (const var&, const String&);
JUCE_API bool operator!= (const var&, const String&);
JUCE_API bool operator== (const var&, const char*);
JUCE_API bool operator!= (const var&, const char*);
} // namespace juce
