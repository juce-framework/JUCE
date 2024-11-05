//
//    ██████ ██   ██  ██████   ██████
//   ██      ██   ██ ██    ██ ██            ** Classy Header-Only Classes **
//   ██      ███████ ██    ██ ██
//   ██      ██   ██ ██    ██ ██           https://github.com/Tracktion/choc
//    ██████ ██   ██  ██████   ██████
//
//   CHOC is (C)2022 Tracktion Corporation, and is offered under the terms of the ISC license:
//
//   Permission to use, copy, modify, and/or distribute this software for any purpose with or
//   without fee is hereby granted, provided that the above copyright notice and this permission
//   notice appear in all copies. THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
//   WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
//   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
//   CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
//   WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
//   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef CHOC_VALUE_POOL_HEADER_INCLUDED
#define CHOC_VALUE_POOL_HEADER_INCLUDED

#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <memory>
#include <exception>
#include "../platform/choc_Assert.h"

namespace choc::value
{

class Value;
class ValueView;
class StringDictionary;
struct MemberNameAndType;
struct MemberNameAndValue;
struct ElementTypeAndOffset;

// This macro lets you override the primitive type that will be used for
// encoding bool elements in a choc::value::ValueView. This setting makes
// no difference to the way the serialisation is done, but affects the way
// elements are packed in memory in a live value object. By default this uses
// a uint32_t for bools, so that all data elements are 4-byte aligned, but you
// could change it to a uint8_t if you want to pack the data more tightly at
// the expense of aligned read/write access.
#ifdef CHOC_VALUE_BOOL_STORAGE_TYPE
 using BoolStorageType = CHOC_VALUE_BOOL_STORAGE_TYPE;
#else
 using BoolStorageType = uint32_t;
#endif

//==============================================================================
/// An exception object which is thrown by the Type, Value and ValueView classes when various
/// runtime checks fail.
/// @see Type, Value, ValueView
struct Error  : public std::exception
{
    Error (const char* desc) : description (desc) {}

    const char* what() const noexcept override    { return description; }
    const char* description;
};


/// Throws an error exception.
/// Note that the message string is taken as a raw pointer and not copied, so must be a string literal.
/// This is used by the Type, Value and ValueView classes.
/// @see Type, Value, ValueView
[[noreturn]] static void throwError (const char* errorMessage)     { throw Error (errorMessage); }

/// Throws an Error with the given message if the condition argument is false.
/// Note that the message string is taken as a raw pointer and not copied, so must be a string literal.
/// This is used by the Type, Value and ValueView classes.
static void check (bool condition, const char* errorMessage)       { if (! condition) throwError (errorMessage); }

/// Used by some deserialisation methods in Type, Value and StringDictionary
struct InputData
{
    const uint8_t* start;
    const uint8_t* end;
};

/// This helper class holds a chunk of data that is a serialised Value or ValueView, and has
/// a handy method to turn it back into a Value object.
struct SerialisedData
{
    std::vector<uint8_t> data;

    Value deserialise() const;
    InputData getInputData() const;
    void write (const void*, size_t);
};

/** A custom allocator class which can be used to replace the normal heap allocator
    for a Type object. This is mainly useful if you need to create and manipulate Type
    and Value objects on a realtime thread and need a fast pool allocator.
    If you pass a custom allocator to the Type class, you must make sure that its lifetime
    is greater than that of the Types that are created (both directly and possibly indirectly
    as nested sub-types).
*/
struct Allocator
{
    virtual ~Allocator() = default;
    virtual void* allocate (size_t size) = 0;
    virtual void* resizeIfPossible (void*, size_t requestedSize) = 0;
    virtual void free (void*) noexcept = 0;
};

//==============================================================================
/** */
template <size_t totalSize>
struct FixedPoolAllocator  : public Allocator
{
    FixedPoolAllocator() = default;
    ~FixedPoolAllocator() override = default;

    void reset() noexcept    { position = 0; }
    void* allocate (size_t size) override;
    void* resizeIfPossible (void* data, size_t requiredSize) override;
    void free (void*) noexcept override {}

private:
    size_t position = 0, lastAllocationPosition = 0;
    char pool[totalSize];
};

//==============================================================================
/** A type class that can represent primitives, vectors, strings, arrays and objects.

    A Type can represent:
      - A primitive int32 or int64
      - A primitive float or double
      - A primitive bool
      - A vector of primitives
      - A string
      - An array of other Values
      - An object, which has a class name and a set of named members, each holding another Value.

    The Type class attempts to be small and allocation-free for simple types like primitives, vectors and
    arrays of vectors, but will use heap storage when given something more complex to represent.

    A Type can also be serialised and deserialised to a packed format.

    @see Value, ValueView
*/
class Type  final
{
public:
    Type() = default;
    Type (Type&&);
    Type (const Type&);
    Type (Allocator*, const Type&);  ///< Constructs a copy of another type, using a custom allocator (which may be nullptr).
    Type& operator= (Type&&);
    Type& operator= (const Type&);
    ~Type() noexcept;

    bool isVoid() const noexcept        { return isType (MainType::void_); }
    bool isInt32() const noexcept       { return isType (MainType::int32); }
    bool isInt64() const noexcept       { return isType (MainType::int64); }
    bool isInt() const noexcept         { return isType (MainType::int32, MainType::int64); }
    bool isFloat32() const noexcept     { return isType (MainType::float32); }
    bool isFloat64() const noexcept     { return isType (MainType::float64); }
    bool isFloat() const noexcept       { return isType (MainType::float32, MainType::float64); }
    bool isBool() const noexcept        { return isType (MainType::boolean); }
    bool isPrimitive() const noexcept   { return isType (MainType::int32, MainType::int64, MainType::float32, MainType::float64, MainType::boolean); }
    bool isObject() const noexcept      { return isType (MainType::object); }
    bool isString() const noexcept      { return isType (MainType::string); }
    bool isVector() const noexcept      { return isType (MainType::vector); }
    bool isArray() const noexcept       { return isType (MainType::primitiveArray, MainType::complexArray); }
    bool isUniformArray() const;        ///< A uniform array is one where every element has the same type.
    bool isArrayOfVectors() const;
    bool isVectorSize1() const;

    /// Returns true if the type is the same as the provided template type (which must be a primitive)
    template <typename PrimitiveType> bool isPrimitiveType() const noexcept;

    /// Returns the number of elements in an array, vector or object. Throws an Error if the type is void.
    uint32_t getNumElements() const;

    /// If the type is an array or vector with a uniform element type, this returns it; if not, it throws an Error.
    Type getElementType() const;

    /// Returns the type of a given element in this type if it's an array. If the type isn't an array or the index is
    /// out of bounds, it will throw an Error.
    Type getArrayElementType (uint32_t index) const;

    /// For a vector or uniform array type, this allows the number of elements to be directly mutated.
    /// For any other type, this will throw an Error exception.
    void modifyNumElements (uint32_t newNumElements);

    /// Returns the name and type of one of the members if this type is an object; if not, or the index is out
    /// of range, then this will throw an Error exception.
    const MemberNameAndType& getObjectMember (uint32_t index) const;

    /// If this is an object, this returns the index of the member with a given name. If the name isn't found, it
    /// will return -1, and if the type isn't an object, it will throw an Error exception.
    int getObjectMemberIndex (std::string_view name) const;

    /// Returns the class-name of this type if it's an object, or throws an Error if it's not.
    std::string_view getObjectClassName() const;

    /// Returns true if this is an object with the given class-name.
    bool isObjectWithClassName (std::string_view name) const;

    bool operator== (const Type&) const;
    bool operator!= (const Type&) const;

    //==============================================================================
    static Type createVoid()            { return Type (MainType::void_); }
    static Type createInt32()           { return Type (MainType::int32); }
    static Type createInt64()           { return Type (MainType::int64); }
    static Type createFloat32()         { return Type (MainType::float32); }
    static Type createFloat64()         { return Type (MainType::float64); }
    static Type createBool()            { return Type (MainType::boolean); }
    static Type createString()          { return Type (MainType::string); }

    /// Creates a type based on the given template type.
    template <typename PrimitiveType>
    static Type createPrimitive();

    //==============================================================================
    /// Creates a vector type based on the given template type and size.
    template <typename PrimitiveType>
    static Type createVector (uint32_t numElements);

    static Type createVectorInt32   (uint32_t numElements)    { return Type (MainType::int32,   numElements); }
    static Type createVectorInt64   (uint32_t numElements)    { return Type (MainType::int64,   numElements); }
    static Type createVectorFloat32 (uint32_t numElements)    { return Type (MainType::float32, numElements); }
    static Type createVectorFloat64 (uint32_t numElements)    { return Type (MainType::float64, numElements); }
    static Type createVectorBool    (uint32_t numElements)    { return Type (MainType::boolean, numElements); }

    //==============================================================================
    /// Creates a type representing an empty array. Element types can be appended with addArrayElements().
    static Type createEmptyArray();

    /// Creates a type representing an array containing a set of elements of a fixed type.
    static Type createArray (Type elementType, uint32_t numElements);

    /// Creates a type representing an array of primitives based on the templated type.
    template <typename PrimitiveType>
    static Type createArray (uint32_t numArrayElements);

    /// Creates a type representing an array of vectors based on the templated type.
    template <typename PrimitiveType>
    static Type createArrayOfVectors (uint32_t numArrayElements, uint32_t numVectorElements);

    /// Appends a group of array elements with the given to this type's definition.
    /// This will throw an Error if this isn't possible for various reasons.
    void addArrayElements (Type elementType, uint32_t numElements);

    //==============================================================================
    /// Returns a type representing an empty object, with the given class name.
    static Type createObject (std::string_view className, Allocator* allocator = nullptr);

    /// Appends a member to an object type, with the given name and type. This will throw an Error if
    /// this isn't possible for some reason.
    void addObjectMember (std::string_view memberName, Type memberType);

    //==============================================================================
    /// Returns the size in bytes needed to store a value of this type.
    size_t getValueDataSize() const;

    /// Returns true if this type, or any of its sub-types are a string.
    bool usesStrings() const;

    /// Returns the type and packed-data position of one of this type's sub-elements.
    ElementTypeAndOffset getElementTypeAndOffset (uint32_t index) const;

    //==============================================================================
    /** Stores a representation of this type in a packed data format.
        It can later be reloaded with deserialise(). The OutputStream template can
        be any object which has a method write (const void*, size_t)

        The data format is simple:
        Primitives:  type (1 byte)
        Vectors:     type (1 byte), num elements (packed int), primitive type (1 byte)
        Array:       type (1 byte), num groups (packed int), [num repetitions (packed int), element type (type)]*
        Object:      type (1 byte), num members (packed int), name (null-term string), [member type (type), member name (null-term string)]*

        Packed ints are stored as a sequence of bytes in little-endian order, where each byte contains
        7 bits of data + the top bit is set if another byte follows it.

        @see deserialise
    */
    template <typename OutputStream>
    void serialise (OutputStream&) const;

    /// Recreates a type from a serialised version that was created by the serialise() method.
    /// Any errors while reading the data will cause an Error exception to be thrown.
    /// The InputData object will be left pointing to any remaining data after the type has been read.
    /// @see serialise
    static Type deserialise (InputData&, Allocator* allocator = nullptr);

    /// Returns a representation of this type in the form of a Value. @see fromValue
    Value toValue() const;

    /// Parses a Value which was created by toValue(), converting it back into a Type object.
    static Type fromValue (const ValueView&);

    /// Returns a human-interpretable description of this type, useful for debugging.
    std::string getDescription() const;

    /// Returns a compact string to uniquely describe this type's layout.
    /// The signature includes information about any sub-types, e.g object member types, array
    /// element types and array sizes. If includeNames is true, it also embeds the names of objects
    /// and members, but if false it will ingore names and represent the "duck" type.
    std::string getSignature (bool includeNames) const;

private:
    //==============================================================================
    enum class MainType  : uint8_t
    {
        void_           = 0,
        int32           = 0x00 + sizeof (int32_t),
        int64           = 0x00 + sizeof (int64_t),
        float32         = 0x10 + sizeof (float),
        float64         = 0x10 + sizeof (double),
        boolean         = 0x30 + sizeof (BoolStorageType),
        string          = 0x40 + sizeof (uint32_t),
        vector          = 0x50,
        primitiveArray  = 0x60,
        object          = 0x80, // these two must have the top bit set to make it quick
        complexArray    = 0x90  // to decide whether the content references a heap object
    };

    static constexpr uint32_t maxNumVectorElements = 256;
    static constexpr uint32_t maxNumArrayElements = 1024 * 1024;

    static constexpr uint32_t getPrimitiveSize (MainType t)   { return static_cast<uint32_t> (t) & 15; }

    friend class ValueView;
    friend class Value;
    struct SerialisationHelpers;
    struct ComplexArray;
    struct Object;
    template <typename ObjectType> struct AllocatedVector;

    struct Vector
    {
        MainType elementType;
        uint32_t numElements;

        size_t getElementSize() const;
        size_t getValueDataSize() const;
        ElementTypeAndOffset getElementInfo (uint32_t) const;
        ElementTypeAndOffset getElementRangeInfo (uint32_t start, uint32_t length) const;
        bool operator== (const Vector&) const;
    };

    struct PrimitiveArray
    {
        MainType elementType;
        uint32_t numElements, numVectorElements;

        Type getElementType() const;
        size_t getElementSize() const;
        size_t getValueDataSize() const;
        ElementTypeAndOffset getElementInfo (uint32_t) const;
        ElementTypeAndOffset getElementRangeInfo (uint32_t start, uint32_t length) const;
        bool operator== (const PrimitiveArray&) const;
    };

    union Content
    {
        Object* object;
        ComplexArray* complexArray;
        Vector vector;
        PrimitiveArray primitiveArray;
    };

    MainType mainType = MainType::void_;
    Content content = {};
    Allocator* allocator = nullptr;

    template <typename... Types> bool isType (Types... types) const noexcept   { return ((mainType == types) || ...); }
    template <typename Type> static constexpr MainType selectMainType();

    explicit Type (MainType);
    Type (MainType, Content, Allocator*);
    Type (MainType vectorElementType, uint32_t);
    void allocateCopy (const Type&);
    void deleteAllocatedObjects() noexcept;
    ElementTypeAndOffset getElementRangeInfo (uint32_t start, uint32_t length) const;
    template <typename Visitor> void visitStringHandles (size_t, const Visitor&) const;
    static Type createArray (Type elementType, uint32_t numElements, Allocator*);
};

//==============================================================================
/** This holds the type and location of a sub-element of a Type.
    @see Type::getElementTypeAndOffset()
*/
struct ElementTypeAndOffset
{
    Type elementType;
    size_t offset;   ///< The byte position within its parent value of the data representing this element
};

//==============================================================================
/** A simple dictionary base-class for mapping strings onto integer handles.
    This is needed by the Value and ValueView classes.
    @see Value, ValueView
*/
class StringDictionary
{
public:
    StringDictionary() = default;
    virtual ~StringDictionary() = default;

    struct Handle
    {
        uint32_t handle = 0;

        bool operator== (Handle h) const        { return handle == h.handle; }
        bool operator!= (Handle h) const        { return handle != h.handle; }
        bool operator<  (Handle h) const        { return handle <  h.handle; }
    };

    /// Finds or creates a handle for a string.
    virtual Handle getHandleForString (std::string_view stringToAdd) = 0;

    /// Fetches the string for a given handle. If the handle isn't found,
    /// the implementation may throw an error.
    virtual std::string_view getStringForHandle (Handle handle) const = 0;
};


//==============================================================================
/** A simple implementation of StringDictionary.
    This should have good performance for typical-sized dictionaries.
    Adding new strings will require O(log n) time where n = dictionary size, but
    retrieving the string for a handle is fast with O(1).
*/
struct SimpleStringDictionary  : public StringDictionary
{
    SimpleStringDictionary() = default;
    SimpleStringDictionary (const SimpleStringDictionary& other) : strings (other.strings), stringMap (other.stringMap) {}
    SimpleStringDictionary (SimpleStringDictionary&& other)      : strings (std::move (other.strings)), stringMap (std::move (other.stringMap)) {}
    SimpleStringDictionary& operator= (const SimpleStringDictionary& other) { strings = other.strings; stringMap = other.stringMap; return *this; }
    SimpleStringDictionary& operator= (SimpleStringDictionary&& other)      { strings = std::move (other.strings); stringMap = std::move (other.stringMap); return *this; }

    Handle getHandleForString (std::string_view) override;
    std::string_view getStringForHandle (Handle handle) const override;

    bool empty() const { return strings.empty(); }
    void clear();

    size_t getRawDataSize() const   { return strings.size(); }
    const char* getRawData() const  { return strings.data(); }

    void setRawData (const void*, size_t);

private:
    std::pair<std::vector<uint32_t>::const_iterator, bool> findGreaterThanOrEqual (std::string_view) const;

    /// The strings are stored in a single chunk, which can be saved and
    /// reloaded if necessary. The stringMap is a sorted vector of handles
    /// supporting fast lookup of strings in the map
    std::vector<char> strings;
    std::vector<uint32_t> stringMap;
};

//==============================================================================
/**
    Represents a view onto an object which can represent various types of primitive,
    array and object types.

    The ValueView and Value classes differ in that ValueView does not own the data that it
    points to, but Value does. A ValueView should be used as a temporary wrapper around some
    data whose lifetime can be trusted to outlive the ValueView object. As a rule-of-thumb, you
    should treat Value and Valueview in the same way as std::string and std::string_view, so
    a ValueView makes a great type for a function parameter, but probably shouldn't be used
    as a function return type unless you really know what you're doing.

    The purpose of these classes is to allow manipulation of complex, dynamically-typed objects
    where the data holding a value is stored in a contiguous, packed, well-specified data
    format, so that it can be manipulated directly as raw memory when necessary. The ValueView
    is a lightweight wrapper around a type and a pointer to the raw data containing a value of that
    type. The Value class provides the same interface, but also owns the storage needed, and can
    return a ValueView of itself.

    @see Type, Value, choc::json::toString()
*/
class ValueView  final
{
public:
    ValueView();                                             ///< Creates an empty value with a type of 'void'.
    ValueView (Type&&, void* data, StringDictionary*);       ///< Creates a value using the given type and raw block of data.
    ValueView (const Type&, void* data, StringDictionary*);  ///< Creates a value using the given type and raw block of data.

    ValueView (const ValueView&) = default;
    ValueView& operator= (const ValueView&) = default;
    ValueView& operator= (ValueView&&) = default;

    //==============================================================================
    const Type& getType() const                 { return type; }
    Type& getMutableType()                      { return type; }

    bool isVoid() const noexcept                { return type.isVoid(); }
    bool isInt32() const noexcept               { return type.isInt32(); }
    bool isInt64() const noexcept               { return type.isInt64(); }
    bool isInt() const noexcept                 { return type.isInt(); }
    bool isFloat32() const noexcept             { return type.isFloat32(); }
    bool isFloat64() const noexcept             { return type.isFloat64(); }
    bool isFloat() const noexcept               { return type.isFloat(); }
    bool isBool() const noexcept                { return type.isBool(); }
    bool isPrimitive() const noexcept           { return type.isPrimitive(); }
    bool isObject() const noexcept              { return type.isObject(); }
    bool isString() const noexcept              { return type.isString(); }
    bool isVector() const noexcept              { return type.isVector(); }
    bool isArray() const noexcept               { return type.isArray(); }

    //==============================================================================
    int32_t                   getInt32() const;          ///< Retrieves the value if this is an int32, otherwise throws an Error exception.
    int64_t                   getInt64() const;          ///< Retrieves the value if this is an int64, otherwise throws an Error exception.
    float                     getFloat32() const;        ///< Retrieves the value if this is a float, otherwise throws an Error exception.
    double                    getFloat64() const;        ///< Retrieves the value if this is a double, otherwise throws an Error exception.
    bool                      getBool() const;           ///< Retrieves the value if this is a bool, otherwise throws an Error exception.
    std::string_view          getString() const;         ///< Retrieves the value if this is a string, otherwise throws an Error exception.
    StringDictionary::Handle  getStringHandle() const;   ///< Retrieves the value if this is a string handle, otherwise throws an Error exception.

    explicit operator int32_t() const            { return getInt32(); }      ///< If the object is not an int32, this will throw an Error.
    explicit operator int64_t() const            { return getInt64(); }      ///< If the object is not an int64, this will throw an Error.
    explicit operator float() const              { return getFloat32(); }    ///< If the object is not a float, this will throw an Error.
    explicit operator double() const             { return getFloat64(); }    ///< If the object is not a double, this will throw an Error.
    explicit operator bool() const               { return getBool(); }       ///< If the object is not a bool, this will throw an Error.
    explicit operator std::string_view() const   { return getString(); }     ///< If the object is not a string, this will throw an Error.

    /// Attempts to cast this value to the given primitive target type. If the type is void or something that
    /// can't be cast, it will throw an exception. This will do some minor casting, such as ints to doubles,
    /// but won't attempt do any kind of string to number conversions.
    template <typename TargetType> TargetType get() const;

    /// Attempts to get this value as the given target type, but if this isn't possible,
    /// returns the default value provided instead of throwing an Error.
    template <typename TargetType> TargetType getWithDefault (TargetType defaultValue) const;

    /// A handy way to convert this value as a string where possible, or to return an empty
    /// string (without throwing any errors) if not possible. The function is basically the
    /// same as calling getWithDefault<std::string> ({})
    std::string toString() const;

    /// Attempts to write a new value to the memory pointed to by this view, as long as the type
    /// provided exactly matches the value's type.
    template <typename PrimitiveType> void set (PrimitiveType newValue);

    /// Resets this value to a 'zero' state. Note that for arrays, this won't change the size
    /// of the array, it just sets all the existing elements to zero.
    void setToZero();

    //==============================================================================
    /// If this object is a vector, array or object, this returns the number of items it contains; otherwise
    /// it will throw an Error exception.
    uint32_t size() const;

    /// If this object is an array or vector, and the index is valid, this returns one of its elements.
    /// Throws an error exception if the object is not a vector or the index is out of range.
    ValueView operator[] (int index) const;

    /// If this object is an array or vector, and the index is valid, this returns one of its elements.
    /// Throws an error exception if the object is not a vector or the index is out of range.
    ValueView operator[] (uint32_t index) const;

    /// If this object is an array or vector, and the index and length do not exceed its bounds, this
    /// will return a view onto a range of its elements.
    /// Throws an error exception if the object is not a vector or the range is invalid.
    ValueView getElementRange (uint32_t startIndex, uint32_t length) const;

    //==============================================================================
    struct Iterator;
    struct EndIterator {};

    /// Iterating a Value is only valid for an array, vector or object.
    Iterator begin() const;
    EndIterator end() const     { return {}; }

    //==============================================================================
    /// Returns the class name of this object.
    /// This will throw an error if the value is not an object.
    std::string_view getObjectClassName() const;

    /// Returns true if this is an object with the given class-name.
    bool isObjectWithClassName (std::string_view name) const;

    /// Returns the name and value of a member by index.
    /// This will throw an error if the value is not an object of if the index is out of range. (Use
    /// size() to find out how many members there are). To get a named value from an object, you can
    /// use operator[].
    /// @see size
    MemberNameAndValue getObjectMemberAt (uint32_t index) const;

    /// Returns the value of a named member, or a void value if no such member exists.
    /// This will throw an error if the value is not an object.
    ValueView operator[] (std::string_view name) const;

    /// Returns the value of a named member, or a void value if no such member exists.
    /// This will throw an error if the value is not an object.
    ValueView operator[] (const char* name) const;

    /// Returns true if this is an object and contains the given member name.
    bool hasObjectMember (std::string_view name) const;

    /// Calls a functor on each member in an object.
    /// The functor must take two parameters of type (string_view name, const ValueView& value).
    template <typename Visitor>
    void visitObjectMembers (Visitor&&) const;

    //==============================================================================
    /// Performs a comparison between two values, where only a bit-for-bit match is
    /// considered to be true.
    bool operator== (const ValueView&) const;
    /// Performs a comparison between two values, where only a bit-for-bit match is
    /// considered to be true.
    bool operator!= (const ValueView&) const;

    //==============================================================================
    /// Gets a pointer to the string dictionary that the view is using, or nullptr
    /// if it doesn't have one.
    StringDictionary* getDictionary() const     { return stringDictionary; }

    /// Allows you to change the string dictionary which this view is using.
    /// Changing the dictionary will visit all the strings inside the object,
    /// remapping old handles into new ones from the new dictionary.
    void setDictionary (StringDictionary* newDictionary);

    /// Gets a pointer to the memory that this view is using for its content.
    void* getRawData()                          { return data; }
    /// Gets a pointer to the memory that this view is using for its content.
    const void* getRawData() const              { return data; }
    /// Allows you to directly modify the internal pointer to the data that this view is
    /// using. Obviously this should only be used if you really know what you're doing!
    void setRawData (void* newAddress)          { data = static_cast<uint8_t*> (newAddress); }

    //==============================================================================
    /// Stores a complete representation of this value and its type in a packed data format.
    /// It can later be reloaded with Value::deserialise() or ValueView::deserialise().
    /// The OutputStream object can be any class which has a method write (const void*, size_t).
    /// The data format is:
    /// - The serialised Type data, as written by Type::serialise()
    /// - The block of value data, which is a copy of getRawData(), the size being Type::getValueDataSize()
    /// - If any strings are in the dictionary, this is followed by a packed int for the total size of
    ///   the remaining string block, then a sequence of null-terminated strings. String handles are
    ///   encoded as a byte offset into this table, where the first character of the first string = 1.
    /// @see Value::deserialise, ValueView::deserialise
    template <typename OutputStream>
    void serialise (OutputStream&) const;

    /// Returns an object containing a serialised representation of this value. This is a helper
    /// function to make it easier to call serialise() without needing to use your own output
    /// stream class.
    SerialisedData serialise() const;

    /// Recreates a temporary ValueView from serialised data that was created by the
    /// ValueView::serialise() method.
    /// If a ValueView is successfully deserialised from the data, the handler functor will be
    /// called with this (temporary!) ValueView as its argument.
    /// Any errors while reading the data will cause an Error exception to be thrown.
    /// The InputData object will be left pointing to any remaining data after the value has been read.
    /// @see Value::serialise
    template <typename Handler>
    static void deserialise (InputData&, Handler&& handleResult,
                             Allocator* allocator = nullptr);

private:
    //==============================================================================
    friend class Value;

    Type type;
    uint8_t* data = nullptr;
    StringDictionary* stringDictionary = nullptr;

    ValueView (StringDictionary&);
    template <typename TargetType> TargetType readContentAs() const;
    template <typename TargetType> TargetType castToType (TargetType*) const;
    template <typename PrimitiveType> void setUnchecked (PrimitiveType);
    void updateStringHandles (StringDictionary&, StringDictionary&);

    ValueView operator[] (const void*) const = delete;
    ValueView operator[] (bool) const = delete;
};


//==============================================================================
/** Represents the name and type of a member in an object.
    @see Type
*/
struct MemberNameAndType
{
    std::string_view name;
    Type type;
};

/** Represents the name and value of a member in an object.
    @see Value, ValueView
*/
struct MemberNameAndValue
{
    const char* name;
    ValueView value;
};


//==============================================================================
/**
    Stores a value of any type that the Type class can represent.

    A Value class can be treated as a by-value class, and manages all the storage needed to
    represent a ValueView object.

    The ValueView and Value classes differ in that ValueView does not own the data that it
    points to, but Value does. A ValueView should be used as a temporary wrapper around some
    data whose lifetime can be trusted to outlive the ValueView object.

    The purpose of these classes is to allow manipulation of complex, dynamically-typed objects
    where the data holding a value is stored in a contiguous, packed, well-specified data
    format, so that it can be manipulated directly as raw memory when necessary. The ValueView
    is a lightweight wrapper around a type and a pointer to the raw data containing a value of that
    type. The Value class provides the same interface, but also owns the storage needed, and can
    return a ValueView of itself.

    The Value class is versatile enough, and close enough to JSON's architecture that it can be
    parsed and printed as JSON (though storing a Value as JSON will be a slightly lossy operation
    as JSON has fewer types).

    @see ValueView, Type, choc::json::parse(), choc::json::toString()
*/
class Value   final
{
public:
    /// Creates an empty value with a type of 'void'.
    Value();

    Value (Value&&);
    Value (const Value&);
    Value& operator= (Value&&);
    Value& operator= (const Value&);

    /// Creates a zero-initialised value with the given type.
    explicit Value (const Type&);

    /// Creates a zero-initialised value with the given type.
    explicit Value (Type&&);

    /// Creates a deep-copy of the given ValueView.
    explicit Value (const ValueView&);

    /// Creates a deep-copy of the given ValueView.
    explicit Value (ValueView&&);

    /// Creates a deep-copy of the given ValueView.
    Value& operator= (const ValueView&);

    explicit Value (int32_t);
    explicit Value (int64_t);
    explicit Value (float);
    explicit Value (double);
    explicit Value (bool);
    explicit Value (std::string_view);
    explicit Value (const char*);

    //==============================================================================
    /// Appends an element to this object, if it's an array. If not, then this will throw an Error exception.
    template <typename ElementType>
    void addArrayElement (ElementType);

    /// Appends one or more members to an object, with the given names and values.
    /// The value can be a supported primitive type, a string, or a Value or ValueView.
    /// The function can take any number of name/value pairs.
    /// This will throw an Error if this isn't possible for some reason (e.g. if the value isn't an object)
    template <typename MemberType, typename... Others>
    void addMember (std::string_view name, MemberType value, Others&&...);

    /// Adds or changes an object member to a new value.
    template <typename MemberType>
    void setMember (std::string_view name, MemberType newValue);

    //==============================================================================
    bool isVoid() const                         { return value.isVoid(); }
    bool isInt32() const                        { return value.isInt32(); }
    bool isInt64() const                        { return value.isInt64(); }
    bool isInt() const                          { return value.isInt(); }
    bool isFloat32() const                      { return value.isFloat32(); }
    bool isFloat64() const                      { return value.isFloat64(); }
    bool isFloat() const                        { return value.isFloat(); }
    bool isBool() const                         { return value.isBool(); }
    bool isPrimitive() const                    { return value.isPrimitive(); }
    bool isObject() const                       { return value.isObject(); }
    bool isString() const                       { return value.isString(); }
    bool isVector() const                       { return value.isVector(); }
    bool isArray() const                        { return value.isArray(); }

    //==============================================================================
    int32_t                   getInt32() const          { return value.getInt32(); }        ///< Retrieves the value if this is an int32, otherwise throws an Error exception.
    int64_t                   getInt64() const          { return value.getInt64(); }        ///< Retrieves the value if this is an int64, otherwise throws an Error exception.
    float                     getFloat32() const        { return value.getFloat32(); }      ///< Retrieves the value if this is a float, otherwise throws an Error exception.
    double                    getFloat64() const        { return value.getFloat64(); }      ///< Retrieves the value if this is a double, otherwise throws an Error exception.
    bool                      getBool() const           { return value.getBool(); }         ///< Retrieves the value if this is a bool, otherwise throws an Error exception.
    std::string_view          getString() const         { return value.getString(); }       ///< Retrieves the value if this is a string, otherwise throws an Error exception.
    StringDictionary::Handle  getStringHandle() const   { return value.getStringHandle(); } ///< Retrieves the value if this is a string handle, otherwise throws an Error exception.

    explicit operator int32_t() const           { return value.getInt32(); }      ///< If the object is not an int32, this will throw an Error.
    explicit operator int64_t() const           { return value.getInt64(); }      ///< If the object is not an int64, this will throw an Error.
    explicit operator float() const             { return value.getFloat32(); }    ///< If the object is not a float, this will throw an Error.
    explicit operator double() const            { return value.getFloat64(); }    ///< If the object is not a double, this will throw an Error.
    explicit operator bool() const              { return value.getBool(); }       ///< If the object is not a bool, this will throw an Error.
    explicit operator std::string_view() const  { return value.getString(); }     ///< If the object is not a string, this will throw an Error.

    /// Attempts to cast this value to the given primitive target type. If the type is void or something that
    /// can't be cast, it will throw an exception. This will do some minor casting, such as ints to doubles,
    /// but won't attempt do any kind of string to number conversions.
    template <typename TargetType> TargetType get() const;

    /// Attempts to get this value as the given target type, but if this isn't possible,
    /// returns the default value provided instead of throwing an Error.
    template <typename TargetType> TargetType getWithDefault (TargetType defaultValue) const;

    /// A handy way to convert this value as a string where possible, or to return an empty
    /// string (without throwing any errors) if not possible. The function is basically the
    /// same as calling getWithDefault<std::string> ({})
    std::string toString() const;

    /// If this object is a vector, array or object, this returns the number of items it contains; otherwise
    /// it will throw an Error exception.
    uint32_t size() const                                               { return value.size(); }

    /// If this object is an array or vector, and the index is valid, this returns one of its elements.
    /// Note that this returns a view of the parent Value, which will become invalid as soon as any
    /// change is made to the parent Value.
    /// Throws an error exception if the object is not a vector or the index is out of range.
    ValueView operator[] (int index) const                              { return value[index]; }

    /// If this object is an array or vector, and the index is valid, this returns one of its elements.
    /// Note that this returns a view of the parent Value, which will become invalid as soon as any
    /// change is made to the parent Value.
    /// Throws an error exception if the object is not a vector or the index is out of range.
    ValueView operator[] (uint32_t index) const                         { return value[index]; }

    /// If this object is an array or vector, and the index and length do not exceed its bounds, this
    /// will return a view onto a range of its elements.
    /// Throws an error exception if the object is not a vector or the range is invalid.
    ValueView getElementRange (uint32_t startIndex, uint32_t length) const      { return value.getElementRange (startIndex, length); }

    //==============================================================================
    /// Performs a comparison between two values, where only a bit-for-bit match is
    /// considered to be true.
    bool operator== (const ValueView& other) const                      { return value == other; }

    /// Performs a comparison between two values, where only a bit-for-bit match is
    /// considered to be true.
    bool operator!= (const ValueView& other) const                      { return value != other; }

    //==============================================================================
    /// Iterating a Value is only valid for an array, vector or object.
    ValueView::Iterator begin() const;
    ValueView::EndIterator end() const;

    //==============================================================================
    /// Returns the class name of this object.
    /// This will throw an error if the value is not an object.
    std::string_view getObjectClassName() const                         { return value.getObjectClassName(); }

    /// Returns true if this is an object with the given class-name.
    bool isObjectWithClassName (std::string_view name) const            { return value.isObjectWithClassName (name); }

    /// Returns the name and value of a member by index.
    /// This will throw an error if the value is not an object of if the index is out of range. (Use
    /// size() to find out how many members there are). To get a named value from an object, you can
    /// use operator[].
    /// @see size
    MemberNameAndValue getObjectMemberAt (uint32_t index) const         { return value.getObjectMemberAt (index); }

    /// Returns the value of a named member, or a void value if no such member exists.
    /// Note that this returns a view of the parent Value, which will become invalid as soon as any
    /// change is made to the parent Value.
    /// This will throw an error if the value is not an object.
    ValueView operator[] (std::string_view name) const                  { return value[name]; }

    /// Returns the value of a named member, or a void value if no such member exists.
    /// Note that this returns a view of the parent Value, which will become invalid as soon as any
    /// change is made to the parent Value.
    /// This will throw an error if the value is not an object.
    ValueView operator[] (const char* name) const                       { return value[name]; }

    /// Returns true if this is an object and contains the given member name.
    bool hasObjectMember (std::string_view name) const                  { return value.hasObjectMember (name); }

    /// Returns a ValueView of this Value. The ValueView will become invalid as soon as any change is made to this Value.
    operator const ValueView&() const                                   { return value; }

    /// Returns a ValueView of this Value. The ValueView will become invalid as soon as any change is made to this Value.
    const ValueView& getView() const                                    { return value; }

    /// Returns a mutable reference to the ValueView held inside this Value. This is only for use if you know what you're doing.
    ValueView& getViewReference()                                       { return value; }

    /// Returns the type of this value.
    const Type& getType() const                                         { return value.getType(); }

    /// Returns a pointer to the raw data that stores this value.
    const void* getRawData() const                                      { return packedData.data(); }
    /// Returns a pointer to the raw data that stores this value.
    void* getRawData()                                                  { return packedData.data(); }
    /// Returns the size of the raw data that stores this value.
    size_t getRawDataSize() const                                       { return packedData.size(); }

    /// Gets a pointer to the string dictionary that the view is using, or nullptr
    /// if it doesn't have one.
    StringDictionary* getDictionary() const                             { return value.getDictionary(); }

    /// Stores a complete representation of this value and its type in a packed data format.
    /// It can later be reloaded with Value::deserialise() or ValueView::deserialise().
    /// The OutputStream object can be any class which has a method write (const void*, size_t).
    /// The data format is:
    /// - The serialised Type data, as written by Type::serialise()
    /// - The block of value data, which is a copy of getRawData(), the size being Type::getValueDataSize()
    /// - If any strings are in the dictionary, this is followed by a packed int for the total size of
    ///   the remaining string block, then a sequence of null-terminated strings. String handles are
    ///   encoded as a byte offset into this table, where the first character of the first string = 1.
    /// @see Value::deserialise, ValueView::deserialise
    template <typename OutputStream>
    void serialise (OutputStream&) const;

    /// Returns an object containing a serialised representation of this value. This is a helper
    /// function to make it easier to call serialise() without needing to use your own output
    /// stream class.
    SerialisedData serialise() const;

    /// Recreates a Value from serialised data that was created by the Value::serialise() method.
    /// Any errors while reading the data will cause an Error exception to be thrown.
    /// The InputData object will be left pointing to any remaining data after the value has been read.
    /// @see Value::serialise
    static Value deserialise (InputData&);

    /// @internal
    Value (Type&&, const void*, size_t);
    /// @internal
    Value (Type&&, const void*, size_t, StringDictionary*);
    /// @internal
    Value (const Type&, const void*, size_t, StringDictionary*);

private:
    //==============================================================================
    Value (const void*) = delete;
    void appendData (const void*, size_t);
    void appendValue (const ValueView&);
    void appendMember (std::string_view, Type&&, const void*, size_t);
    void changeMember (uint32_t, const Type&, void*, StringDictionary*);

    std::vector<uint8_t> packedData;
    SimpleStringDictionary dictionary;
    ValueView value;
};

//==============================================================================
static Value createInt32   (int32_t);
static Value createInt64   (int64_t);
static Value createFloat32 (float);
static Value createFloat64 (double);
static Value createBool    (bool);

static Value createPrimitive (int32_t);
static Value createPrimitive (int64_t);
static Value createPrimitive (float);
static Value createPrimitive (double);
static Value createPrimitive (bool);

static Value createString (std::string_view);

/// Allocates a vector, populating it from an array of primitive values.
template <typename ElementType>
static Value createVector (const ElementType* sourceElements, uint32_t numElements);

/// Allocates a vector, populating it using a functor to return the initial primitive values.
/// The functor must be a class or lambda which takes a uint32_t index parameter and returns
/// the primitive value for that index. The type of the returned primitive is used as the
/// vector's element type.
template <typename GetElementValue>
static Value createVector (uint32_t numElements, const GetElementValue& getValueForIndex);

/// Creates an empty array (to which elements can then be appended with addArrayElement)
static Value createEmptyArray();

/// Allocates an array, populating it using a functor to return the initial values.
/// The functor must be a class or lambda which takes a uint32_t index parameter and returns
/// either Value objects or primitive types to store at that index.
template <typename GetElementValue>
static Value createArray (uint32_t numElements, const GetElementValue& getValueForIndex);

/// Allocates an array which is a packed array of vector primitives, populating it using a
/// functor to return the initial values.
/// The functor must be a class or lambda which takes two uint32_t index parameters (the outer
/// and inner indices for the required element) and returns a primitive type to store at that
/// location.
template <typename GetElementValue>
static Value createArray (uint32_t numArrayElements, uint32_t numVectorElements, const GetElementValue& getValueAt);

/// Creates an array from an iterable container such as a std::vector. The container
/// must contain either Values, or primitive elements which can be turned into Values.
template <typename ContainerType>
static Value createArray (const ContainerType&);

/// Allocates a copy of a packed array of vector primitives.
template <typename ElementType>
static Value create2DArray (const ElementType* sourceElements, uint32_t numArrayElements, uint32_t numVectorElements);

/// Creates a view directly onto a packed array of primitives.
/// The ValueView that is returned will not take a copy of the data, so its lifetime must be managed by the caller.
template <typename ElementType>
static ValueView createArrayView (ElementType* targetData, uint32_t numElements);

/// Creates a view directly onto a packed array of vector primitives.
/// The ValueView that is returned will not take a copy of the data, so its lifetime must be managed by the caller.
template <typename ElementType>
static ValueView create2DArrayView (ElementType* targetData, uint32_t numArrayElements, uint32_t numVectorElements);


/// Returns a Value which is a new empty object.
static Value createObject (std::string_view className);

/// Returns a Value which is a new object, with some member values set.
template <typename... Members>
static Value createObject (std::string_view className, Members&&... members);


//==============================================================================
//        _        _           _  _
//     __| |  ___ | |_   __ _ (_)| | ___
//    / _` | / _ \| __| / _` || || |/ __|
//   | (_| ||  __/| |_ | (_| || || |\__ \ _  _  _
//    \__,_| \___| \__| \__,_||_||_||___/(_)(_)(_)
//
//   Code beyond this point is implementation detail...
//
//==============================================================================

namespace
{
    template <typename Type1> static constexpr bool matchesType()                                       { return false; }
    template <typename Type1, typename Type2, typename... Type3> static constexpr bool matchesType()    { return std::is_same<const Type1, const Type2>::value || matchesType<Type1, Type3...>(); }
    template <typename Type> static constexpr bool isPrimitiveType()    { return matchesType<Type, int32_t, int64_t, float, double, bool, StringDictionary::Handle>(); }
    template <typename Type> static constexpr bool isStringType()       { return matchesType<Type, std::string, std::string&, std::string_view, const char*>(); }
    template <typename Type> static constexpr bool isValueType()        { return matchesType<Type, Value, ValueView>(); }
    template <typename Type> static constexpr size_t getTypeSize()      { return std::is_same<const Type, const bool>::value ? sizeof (BoolStorageType) : sizeof (Type); }

    template <typename TargetType> TargetType readUnaligned (const void* src)
    {
        if constexpr (std::is_same<const TargetType, const bool>::value)
        {
            BoolStorageType b;
            std::memcpy (std::addressof (b), src, sizeof (b));
            return b != 0;
        }
        else
        {
            TargetType v;
            std::memcpy (std::addressof (v), src, sizeof (v));
            return v;
        }
    }

    template <typename TargetType> void writeUnaligned (void* dest, TargetType src)
    {
        if constexpr (std::is_same<const TargetType, const bool>::value)
        {
            BoolStorageType b = src ? 1 : 0;
            std::memcpy (dest, std::addressof (b), sizeof (b));
        }
        else
        {
            std::memcpy (dest, std::addressof (src), sizeof (TargetType));
        }
    }

    static inline void* allocateBytes (Allocator* a, size_t size)
    {
       #ifndef __clang_analyzer__ // this avoids some false positives in the Clang analyser
        if (a != nullptr)
            return a->allocate (size);

        return std::malloc (size);
       #endif
    }

    static inline void* resizeAllocationIfPossible (Allocator* a, void* data, size_t size)
    {
        if (a != nullptr)
            return a->resizeIfPossible (data, size);

        return std::realloc (data, size);
    }

    static inline void freeBytes (Allocator* a, void* data) noexcept
    {
        if (a != nullptr)
            return a->free (data);

        std::free (data);
    }

    template <typename ObjectType, typename... Args>
    ObjectType* allocateObject (Allocator* a, Args&&... args) { return new (allocateBytes (a, sizeof (ObjectType))) ObjectType (std::forward<Args> (args)...); }

    template <typename ObjectType>
    void freeObject (Allocator* a, ObjectType* t)  { if (t != nullptr) { static_cast<ObjectType*>(t)->~ObjectType(); freeBytes (a, t); } }

    static inline std::string_view allocateString (Allocator* a, std::string_view s)
    {
        if (auto size = s.length())
        {
            auto data = static_cast<char*> (allocateBytes (a, size + 1));
            std::memcpy (data, s.data(), size);
            data[size] = 0;
            return { data, size };
        }

        return {};
    }

    static inline void freeString (Allocator* a, std::string_view s) noexcept
    {
        freeBytes (a, const_cast<char*> (s.data()));
    }
}

//==============================================================================
template <size_t totalSize>
void* FixedPoolAllocator<totalSize>::allocate (size_t size)
{
    lastAllocationPosition = position;
    auto result = pool + position;
    auto newSize = position + ((size + 15u) & ~15u);

    if (newSize > sizeof (pool))
        throwError ("Out of local scratch space");

    position = newSize;
    return result;
}

template <size_t totalSize>
void* FixedPoolAllocator<totalSize>::resizeIfPossible (void* data, size_t requiredSize)
{
    if (pool + lastAllocationPosition != data)
        return {};

    position = lastAllocationPosition;
    return allocate (requiredSize);
}

//==============================================================================
// This as a minimal replacement for std::vector (necessary because of custom allocators)
template <typename ObjectType>
struct Type::AllocatedVector
{
    AllocatedVector (Allocator* a) : allocator (a) {}
    AllocatedVector (AllocatedVector&&) = delete;
    AllocatedVector (const AllocatedVector&) = delete;

    ~AllocatedVector() noexcept
    {
        for (decltype (size) i = 0; i < size; ++i)
            items[i].~ObjectType();

        freeBytes (allocator, items);
    }

    ObjectType* begin() const                   { return items; }
    ObjectType* end() const                     { return items + size; }
    bool empty() const                          { return size == 0; }
    ObjectType& front() const                   { return *items; }
    ObjectType& back() const                    { return items[size - 1]; }
    ObjectType& operator[] (uint32_t i) const   { return items[i]; }

    void push_back (ObjectType&& o)
    {
        reserve (size + 1);
        new (items + size) ObjectType (std::move (o));
        ++size;
    }

    bool operator== (const AllocatedVector& other) const
    {
        if (size != other.size)
            return false;

        for (decltype (size) i = 0; i < size; ++i)
            if (! (items[i] == other.items[i]))
                return false;

        return true;
    }

    void reserve (uint32_t needed)
    {
        if (capacity < needed)
        {
            needed = (needed + 7u) & ~7u;
            auto bytesNeeded = sizeof (ObjectType) * needed;

            if (auto reallocated = static_cast<ObjectType*> (resizeAllocationIfPossible (allocator, items, bytesNeeded)))
            {
                items = reallocated;
            }
            else
            {
                auto newItems = allocateBytes (allocator, bytesNeeded);

                if (size != 0)
                    std::memcpy (newItems, items, size * sizeof (ObjectType));

                freeBytes (allocator, items);
                items = static_cast<ObjectType*> (newItems);
            }

            capacity = needed;
        }
    }

    ObjectType* items = nullptr;
    uint32_t size = 0, capacity = 0;
    Allocator* const allocator;
};

inline size_t Type::Vector::getElementSize() const    { return getPrimitiveSize (elementType); }
inline size_t Type::Vector::getValueDataSize() const  { return getElementSize() * numElements; }

inline ElementTypeAndOffset Type::Vector::getElementInfo (uint32_t index) const
{
    check (index < numElements, "Index out of range");
    return { Type (elementType), getElementSize() * index };
}

inline ElementTypeAndOffset Type::Vector::getElementRangeInfo (uint32_t start, uint32_t length) const
{
    check (start < numElements && start + length <= numElements, "Illegal element range");
    return { Type (elementType, length), getElementSize() * start };
}

inline bool Type::Vector::operator== (const Vector& other) const  { return elementType == other.elementType && numElements == other.numElements; }

inline size_t Type::PrimitiveArray::getElementSize() const   { auto sz = getPrimitiveSize (elementType); if (numVectorElements != 0) sz *= numVectorElements; return sz; }
inline size_t Type::PrimitiveArray::getValueDataSize() const { return getElementSize() * numElements; }
inline Type Type::PrimitiveArray::getElementType() const     { return numVectorElements != 0 ? Type (elementType, numVectorElements) : Type (elementType); }

inline ElementTypeAndOffset Type::PrimitiveArray::getElementRangeInfo (uint32_t start, uint32_t length) const
{
    check (start < numElements && start + length <= numElements, "Illegal element range");

    Content c;
    c.primitiveArray = { elementType, length, numVectorElements };

    return { Type (MainType::primitiveArray, c, nullptr),
             start * getPrimitiveSize (elementType) * (numVectorElements != 0 ? numVectorElements : 1) };
}

inline ElementTypeAndOffset Type::PrimitiveArray::getElementInfo (uint32_t index) const
{
    check (index < numElements, "Index out of range");
    auto primitiveSize = getPrimitiveSize (elementType);

    if (numVectorElements != 0)
        return { Type (elementType, numVectorElements), primitiveSize * numVectorElements * index };

    return { Type (elementType), primitiveSize * index };
}

inline bool Type::PrimitiveArray::operator== (const PrimitiveArray& other) const
{
    return elementType == other.elementType && numElements == other.numElements && numVectorElements == other.numVectorElements;
}

struct Type::ComplexArray
{
    ComplexArray() = delete;
    ComplexArray (Allocator* a) : groups (a) {}
    ComplexArray (const ComplexArray&) = delete;

    ComplexArray (Allocator* a, const ComplexArray& other) : groups (a)
    {
        groups.reserve (other.groups.size);

        for (auto& g : other.groups)
            groups.push_back ({ a, g });
    }

    uint32_t size() const
    {
        uint32_t total = 0;

        for (auto& g : groups)
            total += g.repetitions;

        return total;
    }

    Type getElementType (uint32_t index) const
    {
        uint32_t count = 0;

        for (auto& g : groups)
        {
            count += g.repetitions;

            if (index < count)
                return g.elementType;
        }

        throwError ("Index out of range");
    }

    ElementTypeAndOffset getElementRangeInfo (Allocator* a, uint32_t start, uint32_t length) const
    {
        ElementTypeAndOffset info { Type (MainType::complexArray), 0 };
        info.elementType.content.complexArray = allocateObject<ComplexArray> (a, a);
        auto& destGroups = info.elementType.content.complexArray->groups;

        for (auto& g : groups)
        {
            auto groupLen = g.repetitions;

            if (start >= groupLen)
            {
                start -= groupLen;
                info.offset += g.repetitions * g.elementType.getValueDataSize();
                continue;
            }

            if (start > 0)
            {
                groupLen -= start;
                info.offset += start * g.elementType.getValueDataSize();
                start = 0;
            }

            if (length <= groupLen)
            {
                destGroups.push_back ({ length, Type (a, g.elementType) });
                return info;
            }

            destGroups.push_back ({ groupLen, Type (a, g.elementType) });
            length -= groupLen;
        }

        check (start == 0 && length == 0, "Illegal element range");
        return info;
    }

    size_t getValueDataSize() const
    {
        size_t total = 0;

        for (auto& g : groups)
            total += g.repetitions * g.elementType.getValueDataSize();

        return total;
    }

    bool usesStrings() const
    {
        for (auto& g : groups)
            if (g.elementType.usesStrings())
                return true;

        return false;
    }

    template <typename Visitor> void visitStringHandles (size_t offset, const Visitor& visitor) const
    {
        for (auto& g : groups)
        {
            auto elementSize = g.elementType.getValueDataSize();

            if (g.elementType.usesStrings())
            {
                for (uint32_t i = 0; i < g.repetitions; ++i)
                {
                    g.elementType.visitStringHandles (offset, visitor);
                    offset += elementSize;
                }
            }
            else
            {
                offset += elementSize * g.repetitions;
            }
        }
    }

    ElementTypeAndOffset getElementInfo (uint32_t index) const
    {
        size_t offset = 0;

        for (auto& g : groups)
        {
            auto elementSize = g.elementType.getValueDataSize();

            if (index < g.repetitions)
                return { g.elementType, offset + elementSize * index };

            index -= g.repetitions;
            offset += elementSize * g.repetitions;
        }

        throwError ("Index out of range");
    }

    void addElements (Type&& elementType, uint32_t numElementsToAdd)
    {
        if (! groups.empty() && groups.back().elementType == elementType)
            groups.back().repetitions += numElementsToAdd;
        else
            groups.push_back ({ numElementsToAdd, std::move (elementType) });
    }

    bool operator== (const ComplexArray& other) const   { return groups == other.groups; }
    bool isArrayOfVectors() const                       { return groups.size == 1 && groups.front().elementType.isVector(); }
    bool isUniform() const                              { return groups.empty() || groups.size == 1; }

    Type getUniformType() const
    {
        check (groups.size == 1, "This array does not contain a single element type");
        return groups.front().elementType;
    }

    struct RepeatedGroup
    {
        RepeatedGroup (const RepeatedGroup&) = delete;
        RepeatedGroup (RepeatedGroup&&) = default;
        RepeatedGroup (uint32_t reps, Type&& element) : repetitions (reps), elementType (std::move (element)) {}
        RepeatedGroup (Allocator* a, const RepeatedGroup& other) : repetitions (other.repetitions), elementType (a, other.elementType) {}

        uint32_t repetitions;
        Type elementType;

        bool operator== (const RepeatedGroup& other) const   { return repetitions == other.repetitions
                                                                   && elementType == other.elementType; }
    };

    AllocatedVector<RepeatedGroup> groups;
};

struct Type::Object
{
    Object() = delete;
    Object (const Object&) = delete;
    Object (Allocator* a, std::string_view name) : className (allocateString (a, name)), members (a) {}

    Object (Allocator* a, const Object& other) : className (allocateString (a, other.className)), members (a)
    {
        members.reserve (other.members.size);

        for (auto& m : other.members)
            members.push_back ({ allocateString (a, m.name), Type (a, m.type) });
    }

    ~Object() noexcept
    {
        freeString (members.allocator, className);

        for (auto& m : members)
            freeString (members.allocator, m.name);
    }

    std::string_view className;
    AllocatedVector<MemberNameAndType> members;

    size_t getValueDataSize() const
    {
        size_t total = 0;

        for (auto& m : members)
            total += m.type.getValueDataSize();

        return total;
    }

    bool usesStrings() const
    {
        for (auto& m : members)
            if (m.type.usesStrings())
                return true;

        return false;
    }

    template <typename Visitor> void visitStringHandles (size_t offset, const Visitor& visitor) const
    {
        for (uint32_t i = 0; i < members.size; ++i)
        {
            members[i].type.visitStringHandles (offset, visitor);
            offset += members[i].type.getValueDataSize();
        }
    }

    ElementTypeAndOffset getElementInfo (uint32_t index) const
    {
        size_t offset = 0;

        for (uint32_t i = 0; i < members.size; ++i)
        {
            if (i == index)
                return { members[i].type, offset };

            offset += members[i].type.getValueDataSize();
        }

        throwError ("Index out of range");
    }

    bool operator== (const Object& other) const
    {
        if (className != other.className)
            return false;

        if (members.size != other.members.size)
            return false;

        for (uint32_t i = 0; i < members.size; ++i)
            if (members[i].name != other.members[i].name
                    || members[i].type != other.members[i].type)
                return false;

        return true;
    }
};

inline Type::Type (Type&& other) : mainType (other.mainType), content (other.content), allocator (other.allocator)
{
    other.mainType = MainType::void_;
}

inline void Type::allocateCopy (const Type& other)
{
    if (isType (MainType::complexArray))   content.complexArray = allocateObject<ComplexArray> (allocator, allocator, *other.content.complexArray);
    else if (isObject())                   content.object = allocateObject<Object> (allocator, allocator, *other.content.object);
    else                                   content = other.content;
}

inline Type::Type (const Type& other) : mainType (other.mainType)
{
    allocateCopy (other);
}

inline Type& Type::operator= (Type&& other)
{
    deleteAllocatedObjects();
    mainType = other.mainType;
    content = other.content;
    allocator = other.allocator;
    other.mainType = MainType::void_;
    return *this;
}

inline Type& Type::operator= (const Type& other)
{
    deleteAllocatedObjects();
    mainType = other.mainType;
    allocateCopy (other);
    return *this;
}

inline Type::Type (MainType t)  : mainType (t) {}
inline Type::Type (MainType t, Content c, Allocator* a)  : mainType (t), content (c), allocator (a) {}

inline Type::Type (MainType vectorElementType, uint32_t size)  : mainType (MainType::vector)
{
    check (size <= maxNumVectorElements, "Too many vector elements");
    content.vector = { vectorElementType, size };
}

inline Type::Type (Allocator* a, const Type& other)  : allocator (a)
{
    operator= (other);
}

inline Type::~Type() noexcept
{
    deleteAllocatedObjects();
}

inline void Type::deleteAllocatedObjects() noexcept
{
    if (static_cast<int8_t> (mainType) < 0)
    {
        if (isType (MainType::complexArray))   freeObject (allocator, content.complexArray);
        else if (isType (MainType::object))    freeObject (allocator, content.object);
    }
}

inline bool Type::isUniformArray() const     { return isType (MainType::primitiveArray) || (isType (MainType::complexArray) && content.complexArray->isUniform()); }
inline bool Type::isArrayOfVectors() const   { return isType (MainType::primitiveArray); }
inline bool Type::isVectorSize1() const      { return isVector() && content.vector.numElements == 1; }

inline uint32_t Type::getNumElements() const
{
    if (isVector())                         return content.vector.numElements;
    if (isType (MainType::primitiveArray))  return content.primitiveArray.numElements;
    if (isType (MainType::complexArray))    return content.complexArray->size();
    if (isObject())                         return static_cast<uint32_t> (content.object->members.size);
    if (isPrimitive() || isString())        return 1;

    throwError ("This type doesn't have sub-elements");
}

inline void Type::modifyNumElements (uint32_t newNumElements)
{
    if (isVector())
        content.vector.numElements = newNumElements;
    else if (isType (MainType::primitiveArray))
        content.primitiveArray.numElements = newNumElements;
    else if (isType (MainType::complexArray))
    {
        uint32_t previousElements = 0;

        for (auto& group : content.complexArray->groups)
        {
            if (previousElements + group.repetitions >= newNumElements)
            {
                group.repetitions = newNumElements - previousElements;
                break;
            }

            previousElements += group.repetitions;
        }
    }
    else
        throwError ("This type is not a uniform array or vector");
}

inline Type Type::getElementType() const
{
    if (isVector())                         return Type (content.vector.elementType);
    if (isType (MainType::primitiveArray))  return content.primitiveArray.getElementType();
    if (isType (MainType::complexArray))    return content.complexArray->getUniformType();

    throwError ("This type is not an array or vector");
}

inline Type Type::getArrayElementType (uint32_t index) const
{
    if (isType (MainType::primitiveArray))  return content.primitiveArray.getElementType();
    if (isType (MainType::complexArray))    return content.complexArray->getElementType (index);
    throwError ("This type is not an array");
}

inline const MemberNameAndType& Type::getObjectMember (uint32_t index) const
{
    check (isObject(), "This type is not an object");
    check (index < content.object->members.size, "Index out of range");
    return content.object->members[index];
}

inline int Type::getObjectMemberIndex (std::string_view name) const
{
    check (isObject(), "This type is not an object");
    int i = 0;

    for (auto& m : content.object->members)
    {
        if (m.name == name)
            return i;

        ++i;
    }

    return -1;
}

template <typename PrimitiveType>
inline constexpr Type::MainType Type::selectMainType()
{
    Type::MainType result = MainType::void_;

    if constexpr (std::is_same<const PrimitiveType, const int32_t>::value)          result = MainType::int32;
    else if constexpr (std::is_same<const PrimitiveType, const int64_t>::value)     result = MainType::int64;
    else if constexpr (std::is_same<const PrimitiveType, const float>::value)       result = MainType::float32;
    else if constexpr (std::is_same<const PrimitiveType, const double>::value)      result = MainType::float64;
    else if constexpr (std::is_same<const PrimitiveType, const bool>::value)        result = MainType::boolean;
    else if constexpr (std::is_same<const PrimitiveType, const char* const>::value) result = MainType::string;
    else if constexpr (std::is_same<const PrimitiveType, const std::string>::value) result = MainType::string;

    return result;
}

template <typename PrimitiveType>
bool Type::isPrimitiveType() const noexcept
{
    return (mainType == selectMainType<PrimitiveType>());
}

template <typename PrimitiveType>
Type Type::createPrimitive()
{
    constexpr auto type = selectMainType<PrimitiveType>();
    static_assert (type != MainType::void_, "The template type needs to be one of the supported primitive types");
    return Type (type);
}

template <typename PrimitiveType>
Type Type::createVector (uint32_t numElements)
{
    constexpr auto type = selectMainType<PrimitiveType>();
    static_assert (type != MainType::void_, "The template type needs to be one of the supported primitive types");
    return Type (type, numElements);
}

inline Type Type::createEmptyArray()
{
    Content c;
    c.primitiveArray = PrimitiveArray { MainType::void_, 0, 0 };
    return Type (MainType::primitiveArray, c, nullptr);
}

inline Type Type::createArray (Type elementType, uint32_t numElements)
{
    return createArray (std::move (elementType), numElements, nullptr);
}

inline Type Type::createArray (Type elementType, uint32_t numElements, Allocator* allocatorToUse)
{
    check (numElements < maxNumArrayElements, "Too many array elements");
    Content c;

    if (elementType.isPrimitive())
    {
        c.primitiveArray = { elementType.mainType, numElements, 0 };
        return Type (MainType::primitiveArray, c, allocatorToUse);
    }

    if (elementType.isVector())
    {
        c.primitiveArray = { elementType.content.vector.elementType, numElements, elementType.content.vector.numElements };
        return Type (MainType::primitiveArray, c, allocatorToUse);
    }

    c.complexArray = allocateObject<ComplexArray> (allocatorToUse, allocatorToUse);
    c.complexArray->groups.push_back ({ numElements, std::move (elementType) });
    return Type (MainType::complexArray, c, allocatorToUse);
}

template <typename PrimitiveType>
Type Type::createArray (uint32_t numArrayElements)
{
    return createArrayOfVectors<PrimitiveType> (numArrayElements, 0);
}

template <typename PrimitiveType>
Type Type::createArrayOfVectors (uint32_t numArrayElements, uint32_t numVectorElements)
{
    constexpr auto elementType = selectMainType<PrimitiveType>();
    static_assert (elementType != MainType::void_, "The element type needs to be one of the supported primitive types");

    Content c;
    c.primitiveArray = { elementType, numArrayElements, numVectorElements };
    return Type (MainType::primitiveArray, c, nullptr);
}

inline void Type::addArrayElements (Type elementType, uint32_t numElementsToAdd)
{
    if (isType (MainType::primitiveArray))
    {
        if (elementType == content.primitiveArray.getElementType())
        {
            content.primitiveArray.numElements += numElementsToAdd;
            return;
        }

        if (content.primitiveArray.numElements == 0)
        {
            *this = createArray (std::move (elementType), numElementsToAdd, allocator);
            return;
        }

        mainType = MainType::complexArray;
        auto newArray = allocateObject<ComplexArray> (allocator, allocator);
        newArray->groups.push_back ({ content.primitiveArray.numElements, content.primitiveArray.getElementType() });
        content.complexArray = newArray;
    }
    else
    {
        check (isType (MainType::complexArray), "Cannot add new elements to this type");
    }

    content.complexArray->addElements (std::move (elementType), numElementsToAdd);
}

inline Type Type::createObject (std::string_view className, Allocator* a)
{
    return Type (MainType::object, Content { allocateObject<Object> (a, a, className) }, a);
}

inline void Type::addObjectMember (std::string_view memberName, Type memberType)
{
    check (getObjectMemberIndex (memberName) < 0, "This object already contains a member with the given name");
    content.object->members.push_back ({ allocateString (allocator, memberName), std::move (memberType) });
}

inline std::string_view Type::getObjectClassName() const
{
    check (isObject(), "This type is not an object");
    return content.object->className;
}

inline bool Type::isObjectWithClassName (std::string_view name) const
{
    return isObject() && content.object->className == name;
}

inline bool Type::operator== (const Type& other) const
{
    if (mainType != other.mainType)
        return false;

    if (isVector())                         return content.vector == other.content.vector;
    if (isType (MainType::primitiveArray))  return content.primitiveArray == other.content.primitiveArray;
    if (isType (MainType::complexArray))    return *content.complexArray == *other.content.complexArray;
    if (isObject())                         return *content.object == *other.content.object;

    return true;
}

inline bool Type::operator!= (const Type& other) const  { return ! operator== (other); }

inline size_t Type::getValueDataSize() const
{
    switch (mainType)
    {
        case MainType::int32:
        case MainType::float32:         return 4;
        case MainType::int64:
        case MainType::float64:         return 8;
        case MainType::boolean:         return getTypeSize<bool>();
        case MainType::string:          return sizeof (StringDictionary::Handle::handle);
        case MainType::vector:          return content.vector.getValueDataSize();
        case MainType::primitiveArray:  return content.primitiveArray.getValueDataSize();
        case MainType::complexArray:    return content.complexArray->getValueDataSize();
        case MainType::object:          return content.object->getValueDataSize();
        case MainType::void_:           return 0;
        default:                        throwError ("Invalid type");
    }
}

inline bool Type::usesStrings() const
{
    return isString()
            || (isObject() && content.object->usesStrings())
            || (isType (MainType::complexArray) && content.complexArray->usesStrings());
}

template <typename Visitor> void Type::visitStringHandles (size_t offset, const Visitor& visitor) const
{
    if (isString())                         return visitor (offset);
    if (isObject())                         return content.object->visitStringHandles (offset, visitor);
    if (isType (MainType::complexArray))    return content.complexArray->visitStringHandles (offset, visitor);

    if (isType (MainType::primitiveArray) && content.primitiveArray.elementType == MainType::string)
    {
        for (uint32_t i = 0; i < content.primitiveArray.numElements; ++i)
        {
            visitor (offset);
            offset += sizeof (StringDictionary::Handle::handle);
        }
    }
}

inline ElementTypeAndOffset Type::getElementTypeAndOffset (uint32_t index) const
{
    if (isType (MainType::vector))          return content.vector.getElementInfo (index);
    if (isType (MainType::primitiveArray))  return content.primitiveArray.getElementInfo (index);
    if (isType (MainType::complexArray))    return content.complexArray->getElementInfo (index);
    if (isType (MainType::object))          return content.object->getElementInfo (index);

    throwError ("Invalid type");
}

inline ElementTypeAndOffset Type::getElementRangeInfo (uint32_t start, uint32_t length) const
{
    if (isType (MainType::vector))          return content.vector.getElementRangeInfo (start, length);
    if (isType (MainType::primitiveArray))  return content.primitiveArray.getElementRangeInfo (start, length);
    if (isType (MainType::complexArray))    return content.complexArray->getElementRangeInfo (allocator, start, length);

    throwError ("Invalid type");
}

//==============================================================================
struct Type::SerialisationHelpers
{
    enum class EncodedType  : uint8_t
    {
        void_       = 0,
        int32       = 1,
        int64       = 2,
        float32     = 3,
        float64     = 4,
        boolean     = 5,
        vector      = 6,
        array       = 7,
        object      = 8,
        string      = 9
    };

    [[noreturn]] static void throwDataError()      { throwError ("Malformed data"); }
    static void expect (bool condition)            { if (! condition) throwDataError(); }

    template <typename OutputStream>
    static void writeVariableLengthInt (OutputStream& out, uint32_t value)
    {
        uint8_t data[8];
        uint32_t index = 0;

        while (value > 127)
        {
            data[index++] = static_cast<uint8_t> ((value & 0x7fu) | 0x80u);
            value >>= 7;
        }

        data[index++] = static_cast<uint8_t> (value);
        out.write (data, index);
    }

    static uint32_t readVariableLengthInt (InputData& source)
    {
        uint32_t result = 0;

        for (int shift = 0;;)
        {
            expect (source.end > source.start);
            auto nextByte = *source.start++;

            if (shift == 28)
                expect (nextByte < 16);

            if (nextByte < 128)
                return result | (static_cast<uint32_t> (nextByte) << shift);

            result |= static_cast<uint32_t> (nextByte & 0x7fu) << shift;
            shift += 7;
        }
    }

    static std::string_view readNullTerminatedString (InputData& source)
    {
        auto start = source.start, end = source.end;

        for (auto p = start; p < end; ++p)
        {
            if (*p == 0)
            {
                source.start = p + 1;
                return { reinterpret_cast<const char*> (start), static_cast<size_t> (p - start) };
            }
        }

        throwDataError();
    }

    template <typename OutputStream>
    struct Writer
    {
        OutputStream& out;

        void writeType (const Type& t)
        {
            switch (t.mainType)
            {
                case MainType::int32:           writeType (EncodedType::int32); break;
                case MainType::int64:           writeType (EncodedType::int64); break;
                case MainType::float32:         writeType (EncodedType::float32); break;
                case MainType::float64:         writeType (EncodedType::float64); break;
                case MainType::boolean:         writeType (EncodedType::boolean); break;
                case MainType::string:          writeType (EncodedType::string); break;
                case MainType::void_:           writeType (EncodedType::void_); break;

                case MainType::vector:          return writeVector (t.content.vector);
                case MainType::primitiveArray:  return writeArray (t.content.primitiveArray);
                case MainType::complexArray:    return writeArray (*t.content.complexArray);
                case MainType::object:          return writeObject (*t.content.object);

                default:                        throwError ("Invalid type");
            }
        }

    private:
        void writeVector (const Vector& v)
        {
            writeType (EncodedType::vector);
            writeInt (v.numElements);
            writeType (Type (v.elementType));
        }

        void writeArray (const PrimitiveArray& a)
        {
            writeType (EncodedType::array);

            if (a.numElements == 0)
            {
                writeInt (0);
            }
            else
            {
                writeInt (1u);
                writeInt (a.numElements);
                writeType (a.getElementType());
            }
        }

        void writeArray (const ComplexArray& a)
        {
            writeType (EncodedType::array);
            writeInt (a.groups.size);

            for (auto& g : a.groups)
            {
                writeInt (g.repetitions);
                writeType (g.elementType);
            }
        }

        void writeObject (const Object& o)
        {
            writeType (EncodedType::object);
            writeInt (o.members.size);
            writeString (o.className);

            for (auto& m : o.members)
            {
                writeType (m.type);
                writeString (m.name);
            }
        }

        void writeType (EncodedType t)            { writeByte (static_cast<uint8_t> (t)); }
        void writeByte (uint8_t byte)             { out.write (&byte, 1); }
        void writeString (std::string_view s)     { out.write (s.data(), s.length()); writeByte (0); }
        void writeInt (uint32_t value)            { writeVariableLengthInt (out, value); }
    };

    struct Reader
    {
        InputData& source;
        Allocator* allocatorToUse;

        Type readType()
        {
            switch (static_cast<EncodedType> (readByte()))
            {
                case EncodedType::void_:     return {};
                case EncodedType::int32:     return createInt32();
                case EncodedType::int64:     return createInt64();
                case EncodedType::float32:   return createFloat32();
                case EncodedType::float64:   return createFloat64();
                case EncodedType::boolean:   return createBool();
                case EncodedType::string:    return createString();
                case EncodedType::vector:    return readVector();
                case EncodedType::array:     return readArray();
                case EncodedType::object:    return readObject();
                default:                     throwDataError();
            }
        }

    private:
        Type readVector()
        {
            auto num = readInt();
            expect (num <= maxNumVectorElements);

            switch (static_cast<EncodedType> (readByte()))
            {
                case EncodedType::int32:      return Type (MainType::int32, num);
                case EncodedType::int64:      return Type (MainType::int64, num);
                case EncodedType::float32:    return Type (MainType::float32, num);
                case EncodedType::float64:    return Type (MainType::float64, num);
                case EncodedType::boolean:    return Type (MainType::boolean, num);
                case EncodedType::string:
                case EncodedType::vector:
                case EncodedType::array:
                case EncodedType::object:
                case EncodedType::void_:
                default:                      throwDataError();
            }
        }

        Type readArray()
        {
            auto t = createEmptyArray();
            t.allocator = allocatorToUse;
            auto numGroups = readInt();
            uint32_t elementCount = 0;

            for (uint32_t i = 0; i < numGroups; ++i)
            {
                auto numReps = readInt();
                expect (numReps <= maxNumArrayElements - elementCount);
                elementCount += numReps;
                t.addArrayElements (readType(), numReps);
            }

            return t;
        }

        Type readObject()
        {
            auto numMembers = readInt();
            auto t = createObject (readNullTerminatedString (source), allocatorToUse);

            for (uint32_t i = 0; i < numMembers; ++i)
            {
                auto memberType = readType();
                t.addObjectMember (readNullTerminatedString (source), std::move (memberType));
            }

            return t;
        }

        uint8_t readByte()
        {
            expect (source.end > source.start);
            return *source.start++;
        }

        uint32_t readInt()
        {
            return readVariableLengthInt (source);
        }
    };
};

template <typename OutputStream>
void Type::serialise (OutputStream& out) const
{
    SerialisationHelpers::Writer<OutputStream> w  { out };
    w.writeType (*this);
}

inline Type Type::deserialise (InputData& input, Allocator* a)
{
    SerialisationHelpers::Reader r { input, a };
    return r.readType();
}

//==============================================================================
inline ValueView::ValueView() = default;
inline ValueView::ValueView (StringDictionary& dic) : stringDictionary (std::addressof (dic)) {}
inline ValueView::ValueView (Type&& t, void* d, StringDictionary* dic) : type (std::move (t)), data (static_cast<uint8_t*> (d)), stringDictionary (dic) {}
inline ValueView::ValueView (const Type& t, void* d, StringDictionary* dic) : type (t), data (static_cast<uint8_t*> (d)), stringDictionary (dic) {}

template <typename ElementType>
ValueView createArrayView (ElementType* targetData, uint32_t numElements)
{
    return ValueView (Type::createArray<ElementType> (numElements), targetData, nullptr);
}

template <typename ElementType>
ValueView create2DArrayView (ElementType* sourceData, uint32_t numArrayElements, uint32_t numVectorElements)
{
    return ValueView (Type::createArrayOfVectors<ElementType> (numArrayElements, numVectorElements), sourceData, nullptr);
}

template <typename TargetType>
TargetType ValueView::readContentAs() const     { return readUnaligned<TargetType> (data); }

template <typename PrimitiveType> static PrimitiveType castString (std::string_view s, PrimitiveType* defaultValue)
{
    if (s.empty())
        return defaultValue != nullptr ? *defaultValue : PrimitiveType();

    if constexpr (matchesType<PrimitiveType, bool>())
        if (s == "true")
            return true;

    auto start = s.data();
    char* end;
    PrimitiveType result;

    if constexpr (matchesType<PrimitiveType, int32_t>())  result = static_cast<int32_t> (std::strtol  (start, std::addressof (end), 10));
    if constexpr (matchesType<PrimitiveType, int64_t>())  result = static_cast<int64_t> (std::strtoll (start, std::addressof (end), 10));
    if constexpr (matchesType<PrimitiveType, float>())    result = std::strtof (start, std::addressof (end));
    if constexpr (matchesType<PrimitiveType, double>())   result = std::strtod (start, std::addressof (end));
    if constexpr (matchesType<PrimitiveType, bool>())     result = std::strtol (start, std::addressof (end), 10) != 0;

    if (end != start)
        return result;

    if (defaultValue == nullptr)
        throwError ("Cannot convert this value to a numeric type");

    return *defaultValue;
}

template <typename TargetType> TargetType ValueView::castToType (TargetType* defaultValue) const
{
    (void) defaultValue;

    if constexpr (matchesType<TargetType, const char*>())
    {
        if (defaultValue == nullptr || isString())
        {
            auto s = getString();
            return s.empty() ? "" : s.data();
        }

        return *defaultValue;
    }
    else if constexpr (isStringType<TargetType>())
    {
        if (defaultValue == nullptr || isString())
            return TargetType (getString());

        return *defaultValue;
    }
    else if constexpr (matchesType<TargetType, uint32_t, uint64_t, size_t>())
    {
        if (defaultValue != nullptr)
        {
            using SignedType = typename std::make_signed<TargetType>::type;
            auto signedDefault = static_cast<SignedType> (*defaultValue);
            auto n = castToType<SignedType> (std::addressof (signedDefault));
            return n >= 0 ? static_cast<TargetType> (n) : *defaultValue;
        }

        auto n = castToType<typename std::make_signed<TargetType>::type> (nullptr);
        check (n >= 0, "Value out of range");
        return static_cast<TargetType> (n);
    }
    else
    {
        static_assert (isPrimitiveType<TargetType>(), "The TargetType template argument must be a valid primitive type");

        switch (type.isVectorSize1() ? type.content.vector.elementType
                                     : type.mainType)
        {
            case Type::MainType::int32:       return static_cast<TargetType> (readContentAs<int32_t>());
            case Type::MainType::int64:       return static_cast<TargetType> (readContentAs<int64_t>());
            case Type::MainType::float32:     return static_cast<TargetType> (readContentAs<float>());
            case Type::MainType::float64:     return static_cast<TargetType> (readContentAs<double>());
            case Type::MainType::boolean:     return static_cast<TargetType> (readContentAs<bool>());
            case Type::MainType::string:      return castString<TargetType> (getString(), defaultValue);

            case Type::MainType::vector:
            case Type::MainType::primitiveArray:
            case Type::MainType::complexArray:
            case Type::MainType::object:
            case Type::MainType::void_:
            default:
                if (defaultValue == nullptr)
                    throwError ("Cannot convert this value to a numeric type");

                return *defaultValue;
        }
    }
}

inline int32_t  ValueView::getInt32() const     { check (type.isInt32(),   "Value is not an int32");   return readContentAs<int32_t>(); }
inline int64_t  ValueView::getInt64() const     { check (type.isInt64(),   "Value is not an int64");   return readContentAs<int64_t>(); }
inline float    ValueView::getFloat32() const   { check (type.isFloat32(), "Value is not a float32");  return readContentAs<float>(); }
inline double   ValueView::getFloat64() const   { check (type.isFloat64(), "Value is not a float64");  return readContentAs<double>(); }
inline bool     ValueView::getBool() const      { check (type.isBool(),    "Value is not a bool");     return readContentAs<bool>(); }

template <typename TargetType> TargetType ValueView::get() const
{
    return castToType<TargetType> (nullptr);
}

template <typename TargetType> TargetType ValueView::getWithDefault (TargetType defaultValue) const
{
    return castToType<TargetType> (std::addressof (defaultValue));
}

inline std::string ValueView::toString() const  { return getWithDefault<std::string> ({}); }

template <typename PrimitiveType> void ValueView::setUnchecked (PrimitiveType v)
{
    static_assert (isPrimitiveType<PrimitiveType>() || isStringType<PrimitiveType>(),
                   "The template type needs to be one of the supported primitive types");

    if constexpr (matchesType<PrimitiveType, StringDictionary::Handle>())
    {
        setUnchecked (static_cast<int32_t> (v.handle));
    }
    else if constexpr (isStringType<PrimitiveType>())
    {
        check (stringDictionary != nullptr, "No string dictionary supplied");
        setUnchecked (stringDictionary->getHandleForString (v));
    }
    else
    {
        writeUnaligned (data, v);
    }
}

template <typename PrimitiveType> void ValueView::set (PrimitiveType v)
{
    static_assert (isPrimitiveType<PrimitiveType>() || isStringType<PrimitiveType>(),
                   "The template type needs to be one of the supported primitive types");

    if constexpr (matchesType<PrimitiveType, int32_t>())  check (type.isInt32(),   "Value is not an int32");
    if constexpr (matchesType<PrimitiveType, int64_t>())  check (type.isInt64(),   "Value is not an int64");
    if constexpr (matchesType<PrimitiveType, float>())    check (type.isFloat32(), "Value is not a float32");
    if constexpr (matchesType<PrimitiveType, double>())   check (type.isFloat64(), "Value is not a float64");
    if constexpr (matchesType<PrimitiveType, bool>())     check (type.isBool(),    "Value is not a bool");

    if constexpr (matchesType<PrimitiveType, StringDictionary::Handle>() || isStringType<PrimitiveType>())
        check (type.isString(), "Value is not a string");

    setUnchecked (v);
}

inline void ValueView::setToZero()
{
    if (data != nullptr)
        memset (data, 0, type.getValueDataSize());
}

inline StringDictionary::Handle ValueView::getStringHandle() const
{
    check (type.isString(), "Value is not a string");
    return StringDictionary::Handle { readContentAs<decltype (StringDictionary::Handle::handle)>() };
}

inline std::string_view ValueView::getString() const
{
    // To satisfy the MSVC code analyser this check needs to be handled directly
    // from this function
    if (stringDictionary == nullptr)
        throwError ("No string dictionary supplied");

    return stringDictionary->getStringForHandle (getStringHandle());
}

inline uint32_t ValueView::size() const             { return type.getNumElements(); }

inline ValueView ValueView::operator[] (uint32_t index) const
{
    auto info = type.getElementTypeAndOffset (index);
    return ValueView (std::move (info.elementType), data + info.offset, stringDictionary);
}

inline ValueView ValueView::getElementRange (uint32_t startIndex, uint32_t length) const
{
    auto info = type.getElementRangeInfo (startIndex, length);
    return ValueView (std::move (info.elementType), data + info.offset, stringDictionary);
}

inline ValueView ValueView::operator[] (int index) const          { return operator[] (static_cast<uint32_t> (index)); }
inline ValueView ValueView::operator[] (const char* name) const   { return operator[] (std::string_view (name)); }

inline ValueView ValueView::operator[] (std::string_view name) const
{
    auto index = type.getObjectMemberIndex (name);

    if (index < 0)
        return {};

    auto info = type.getElementTypeAndOffset (static_cast<uint32_t> (index));
    return ValueView (std::move (info.elementType), data + info.offset, stringDictionary);
}

inline std::string_view ValueView::getObjectClassName() const               { return type.getObjectClassName(); }
inline bool ValueView::isObjectWithClassName (std::string_view name) const  { return type.isObjectWithClassName (name); }

inline MemberNameAndValue ValueView::getObjectMemberAt (uint32_t index) const
{
    auto& member = type.getObjectMember (index);
    auto info = type.getElementTypeAndOffset (index);
    return { member.name.data(), ValueView (std::move (info.elementType), data + info.offset, stringDictionary) };
}

inline bool ValueView::hasObjectMember (std::string_view name) const
{
    return type.getObjectMemberIndex (name) >= 0;
}

template <typename Visitor>
void ValueView::visitObjectMembers (Visitor&& visit) const
{
    check (isObject(), "This value is not an object");
    auto numMembers = size();

    for (uint32_t i = 0; i < numMembers; ++i)
    {
        auto& member = type.getObjectMember (i);
        auto info = type.getElementTypeAndOffset (i);
        visit (member.name, ValueView (std::move (info.elementType), data + info.offset, stringDictionary));
    }
}

struct ValueView::Iterator
{
    Iterator (const ValueView& v) : value (v), numElements (v.size()) {}
    Iterator (const Iterator&) = default;
    Iterator& operator= (const Iterator&) = default;

    ValueView operator*() const             { return value[index]; }
    Iterator& operator++()                  { ++index; return *this; }
    Iterator operator++ (int)               { auto old = *this; ++*this; return old; }
    bool operator== (EndIterator) const     { return index == numElements; }
    bool operator!= (EndIterator) const     { return index != numElements; }

    ValueView value;
    uint32_t index = 0, numElements;
};

inline ValueView::Iterator ValueView::begin() const   { return ValueView::Iterator (*this); }

//==============================================================================
inline bool ValueView::operator== (const ValueView& other) const
{
    return type == other.type
             && (isVoid() || std::memcmp (getRawData(), other.getRawData(), type.getValueDataSize()) == 0);
}

inline bool ValueView::operator!= (const ValueView& other) const { return ! operator== (other); }

//==============================================================================
inline Value SerialisedData::deserialise() const        { auto i = getInputData(); return Value::deserialise (i); }
inline InputData SerialisedData::getInputData() const   { return { data.data(), data.data() + data.size() }; }

inline void SerialisedData::write (const void* d, size_t num)
{
    auto src = static_cast<const uint8_t*> (d);
    data.insert (data.end(), src, src + num);
}

//==============================================================================
template <typename OutputStream>
void ValueView::serialise (OutputStream& output) const
{
    type.serialise (output);

    if (type.isVoid())
        return;

    auto dataSize = type.getValueDataSize();
    check (dataSize > 0, "Invalid data size");

    if (stringDictionary == nullptr || ! type.usesStrings())
    {
        output.write (data, dataSize);
        return;
    }

   #if defined (_MSC_VER)
    #pragma warning (push)
    #pragma warning (disable: 6255)
    auto* localCopy = (uint8_t*) _alloca (dataSize);
    #pragma warning (pop)
   #elif defined (__MINGW32__)
    auto* localCopy = (uint8_t*) _alloca (dataSize);
   #else
    auto* localCopy = (uint8_t*) alloca (dataSize);
   #endif

    check (localCopy != nullptr, "Stack allocation failed");
    std::memcpy (localCopy, data, dataSize);

    static constexpr uint32_t maxStrings = 128;
    uint32_t numStrings = 0, stringDataSize = 0;
    uint32_t oldHandles[maxStrings], newHandles[maxStrings];

    type.visitStringHandles (0, [&] (size_t offset)
    {
        auto handleCopyAddress = localCopy + offset;
        auto oldHandle = readUnaligned<uint32_t> (handleCopyAddress);

        for (uint32_t i = 0; i < numStrings; ++i)
        {
            if (oldHandles[i] == oldHandle)
            {
                writeUnaligned<uint32_t> (handleCopyAddress, newHandles[i]);
                return;
            }
        }

        if (numStrings == maxStrings)
            throwError ("Out of local scratch space");

        oldHandles[numStrings] = oldHandle;
        auto newHandle = stringDataSize + 1u;
        writeUnaligned<uint32_t> (handleCopyAddress, newHandle);
        newHandles[numStrings++] = newHandle;
        stringDataSize += static_cast<uint32_t> (stringDictionary->getStringForHandle ({ oldHandle }).length() + 1u);
    });

    output.write (localCopy, dataSize);
    Type::SerialisationHelpers::writeVariableLengthInt (output, stringDataSize);

    for (uint32_t i = 0; i < numStrings; ++i)
    {
        auto text = stringDictionary->getStringForHandle ({ oldHandles[i] });
        output.write (text.data(), text.length());
        char nullTerm = 0;
        output.write (std::addressof (nullTerm), 1u);
    }
}

inline SerialisedData ValueView::serialise() const
{
    SerialisedData result;
    serialise (result);
    return result;
}

template <typename Handler>
void ValueView::deserialise (InputData& input, Handler&& handleResult, Allocator* allocator)
{
    ValueView result;
    result.type = Type::deserialise (input, allocator);
    auto valueDataSize = result.type.getValueDataSize();
    Type::SerialisationHelpers::expect (input.end >= input.start + valueDataSize);
    result.data = const_cast<uint8_t*> (input.start);
    input.start += valueDataSize;

    if (input.start >= input.end || ! result.type.usesStrings())
    {
        handleResult (result);
        return;
    }

    struct SerialisedStringDictionary  : public choc::value::StringDictionary
    {
        SerialisedStringDictionary (const void* d, size_t s) : start (static_cast<const char*> (d)), size (s) {}
        Handle getHandleForString (std::string_view) override     { CHOC_ASSERT (false); return {}; }

        std::string_view getStringForHandle (Handle handle) const override
        {
            handle.handle--;
            Type::SerialisationHelpers::expect (handle.handle < size);
            return std::string_view (start + handle.handle);
        }

        const char* const start;
        const size_t size;
    };

    auto stringDataSize = Type::SerialisationHelpers::readVariableLengthInt (input);
    Type::SerialisationHelpers::expect (input.start + stringDataSize <= input.end && input.start[stringDataSize - 1] == 0);
    SerialisedStringDictionary dictionary (input.start, stringDataSize);
    result.stringDictionary = std::addressof (dictionary);
    handleResult (result);
}

//==============================================================================
inline void ValueView::updateStringHandles (StringDictionary& oldDic, StringDictionary& newDic)
{
    if (type.isType (Type::MainType::string, Type::MainType::object, Type::MainType::primitiveArray, Type::MainType::complexArray))
    {
        type.visitStringHandles (0, [&oldDic, &newDic, d = this->data] (size_t offset)
        {
            auto oldHandle = StringDictionary::Handle { readUnaligned<decltype(StringDictionary::Handle::handle)> (d + offset) };
            writeUnaligned (d + offset, newDic.getHandleForString (oldDic.getStringForHandle (oldHandle)).handle);
        });
    }
}

inline void ValueView::setDictionary (StringDictionary* newDictionary)
{
    if (stringDictionary != newDictionary)
    {
        auto oldDictionary = stringDictionary;
        stringDictionary = newDictionary;

        if (oldDictionary != nullptr && newDictionary != nullptr)
            updateStringHandles (*oldDictionary, *newDictionary);
    }
}

//==============================================================================
inline Value::Value() : value (dictionary) {}

inline Value::Value (Value&& other)
   : packedData (std::move (other.packedData)), dictionary (std::move (other.dictionary)),
     value (std::move (other.value.type), packedData.data(), std::addressof (dictionary))
{
}

inline Value::Value (const Value& other)
   : packedData (other.packedData), dictionary (other.dictionary),
     value (other.value.type, packedData.data(), std::addressof (dictionary))
{
}

inline Value& Value::operator= (Value&& other)
{
    packedData = std::move (other.packedData);
    dictionary = std::move (other.dictionary);
    value.type = std::move (other.value.type);
    value.data = packedData.data();
    return *this;
}

inline Value& Value::operator= (const Value& other)
{
    packedData = other.packedData;
    dictionary = other.dictionary;
    value.type = other.value.type;
    value.data = packedData.data();
    return *this;
}

inline Value::Value (const Type& t)
   : packedData (static_cast<std::vector<char>::size_type> (t.getValueDataSize())),
     value (t, packedData.data(), std::addressof (dictionary))
{
}

inline Value::Value (Type&& t)
   : packedData (static_cast<std::vector<char>::size_type> (t.getValueDataSize())),
     value (std::move (t), packedData.data(), std::addressof (dictionary))
{
}

inline Value::Value (Type&& t, const void* source, size_t size)
   : packedData (static_cast<const uint8_t*> (source), static_cast<const uint8_t*> (source) + size),
     value (t, packedData.data(), std::addressof (dictionary))
{
}

inline Value::Value (const Type& t, const void* source, size_t size, StringDictionary* d)
   : packedData (static_cast<const uint8_t*> (source), static_cast<const uint8_t*> (source) + size),
     value (t, packedData.data(), d)
{
}

inline Value::Value (Type&& t, const void* source, size_t size, StringDictionary* d)
   : packedData (static_cast<const uint8_t*> (source), static_cast<const uint8_t*> (source) + size),
     value (std::move (t), packedData.data(), d)
{
}

inline Value::Value (const ValueView& source) : Value (source.type, source.getRawData(),
                                                       source.type.getValueDataSize(), source.getDictionary())
{
    // doing this as a separate step forces an import of string handles if needed
    value.setDictionary (std::addressof (dictionary));
}

inline Value::Value (ValueView&& source) : Value (std::move (source.type), source.getRawData(),
                                                  source.type.getValueDataSize(), source.getDictionary())
{
    // doing this as a separate step forces an import of string handles if needed
    value.setDictionary (std::addressof (dictionary));
}

inline Value::Value (int32_t n)           : Value (Type::createInt32(),   std::addressof (n), sizeof (n)) {}
inline Value::Value (int64_t n)           : Value (Type::createInt64(),   std::addressof (n), sizeof (n)) {}
inline Value::Value (float n)             : Value (Type::createFloat32(), std::addressof (n), sizeof (n)) {}
inline Value::Value (double n)            : Value (Type::createFloat64(), std::addressof (n), sizeof (n)) {}
inline Value::Value (bool n)              : Value (Type::createBool())     { writeUnaligned (value.data, n); }
inline Value::Value (std::string_view s)  : Value (Type::createString())   { writeUnaligned (value.data, dictionary.getHandleForString (s)); }
inline Value::Value (const char* s)       : Value (std::string_view (s)) {}

inline Value& Value::operator= (const ValueView& source)
{
    packedData.resize (source.getType().getValueDataSize());
    value.type = source.type;
    value.data = packedData.data();

    auto dataSize = getRawDataSize();
    if (dataSize > 0)
        std::memcpy (value.data, source.getRawData(), dataSize);

    dictionary.clear();

    if (auto sourceDictionary = source.getDictionary())
        value.updateStringHandles (*sourceDictionary, dictionary);

    return *this;
}

inline void Value::appendData (const void* source, size_t size)
{
    packedData.insert (packedData.end(), static_cast<const uint8_t*> (source), static_cast<const uint8_t*> (source) + size);
    value.data = packedData.data();
}

inline void Value::appendValue (const ValueView& newValue)
{
    auto oldSize = packedData.size();
    appendData (newValue.getRawData(), newValue.getType().getValueDataSize());

    if (newValue.stringDictionary != nullptr)
    {
        // this will force an update of any handles in the new data
        ValueView v (newValue);
        v.setRawData (packedData.data() + oldSize);
        v.setDictionary (std::addressof (dictionary));
    }
}

inline Value createPrimitive (int32_t n)           { return Value (n); }
inline Value createPrimitive (int64_t n)           { return Value (n); }
inline Value createPrimitive (float n)             { return Value (n); }
inline Value createPrimitive (double n)            { return Value (n); }
inline Value createPrimitive (bool n)              { return Value (n); }
inline Value createString    (std::string_view s)  { return Value (s); }
inline Value createInt32     (int32_t v)           { return Value (v); }
inline Value createInt64     (int64_t v)           { return Value (v); }
inline Value createFloat32   (float v)             { return Value (v); }
inline Value createFloat64   (double v)            { return Value (v); }
inline Value createBool      (bool v)              { return Value (v); }
inline Value createEmptyArray()                    { return Value (Type::createEmptyArray()); }

template <typename ElementType>
Value createVector (const ElementType* source, uint32_t numElements)
{
    return Value (Type::createVector<ElementType> (numElements), source, getTypeSize<ElementType>() * numElements);
}

template <typename GetElementValue>
Value createVector (uint32_t numElements, const GetElementValue& getValueForIndex)
{
    using ElementType = decltype (getValueForIndex (0));
    static_assert (isPrimitiveType<ElementType>(), "The template type needs to be one of the supported primitive types");
    Value v (Type::createVector<ElementType> (numElements));
    auto dest = static_cast<uint8_t*> (v.getRawData());

    for (uint32_t i = 0; i < numElements; ++i)
    {
        writeUnaligned (dest, getValueForIndex (i));
        dest += getTypeSize<ElementType>();
    }

    return v;
}

template <typename GetElementValue>
Value createArray (uint32_t numElements, const GetElementValue& getValueForIndex)
{
    using ElementType = decltype (getValueForIndex (0));
    static_assert (isPrimitiveType<ElementType>() || isValueType<ElementType>() || isStringType<ElementType>(),
                   "The functor needs to return either a supported primitive type, or a Value");

    if constexpr (isPrimitiveType<ElementType>())
    {
        Value v (Type::createArray (Type::createPrimitive<ElementType>(), numElements));
        auto dest = static_cast<uint8_t*> (v.getRawData());

        for (uint32_t i = 0; i < numElements; ++i)
        {
            writeUnaligned (dest, getValueForIndex (i));
            dest += getTypeSize<ElementType>();
        }

        return v;
    }
    else
    {
        Value v (Type::createEmptyArray());

        for (uint32_t i = 0; i < numElements; ++i)
            v.addArrayElement (getValueForIndex (i));

        return v;
    }
}

template <typename GetElementValue>
Value createArray (uint32_t numArrayElements, uint32_t numVectorElements, const GetElementValue& getValueAt)
{
    using ElementType = typename std::remove_const<typename std::remove_reference<decltype (getValueAt (0, 0))>::type>::type;
    static_assert (isPrimitiveType<ElementType>(), "The functor needs to return a supported primitive type");

    Value v (Type::createArray (Type::createVector<ElementType> (numVectorElements), numArrayElements));
    auto dest = static_cast<uint8_t*> (v.getRawData());

    for (uint32_t j = 0; j < numArrayElements; ++j)
    {
        for (uint32_t i = 0; i < numVectorElements; ++i)
        {
            writeUnaligned (dest, getValueAt (j, i));
            dest += getTypeSize<ElementType>();
        }
    }

    return v;
}

template <typename ContainerType>
Value createArray (const ContainerType& container)
{
    using ElementType = typename std::remove_const<typename std::remove_reference<decltype (container[0])>::type>::type;
    static_assert (isPrimitiveType<ElementType>() || isValueType<ElementType>() || isStringType<ElementType>(),
                   "The container provided must have elements which can be converted to a Value");

    return createArray (static_cast<uint32_t> (container.size()),
                        [&] (uint32_t i) { return container[i]; });
}

template <typename ElementType>
Value create2DArray (const ElementType* sourceData, uint32_t numArrayElements, uint32_t numVectorElements)
{
    static_assert (isPrimitiveType<ElementType>(), "The template type needs to be one of the supported primitive types");
    Value v (Type::createArrayOfVectors<ElementType> (numArrayElements, numVectorElements));
    std::memcpy (v.getRawData(), sourceData, numArrayElements * numVectorElements * getTypeSize<ElementType>());
    return v;
}

template <typename ElementType>
void Value::addArrayElement (ElementType v)
{
    static_assert (isPrimitiveType<ElementType>() || isValueType<ElementType>() || isStringType<ElementType>(),
                   "The template type needs to be one of the supported primitive types");

    if constexpr (matchesType<ElementType, int32_t>())   { value.type.addArrayElements (Type::createInt32(),   1); appendData (std::addressof (v), sizeof (v)); return; }
    if constexpr (matchesType<ElementType, int64_t>())   { value.type.addArrayElements (Type::createInt64(),   1); appendData (std::addressof (v), sizeof (v)); return; }
    if constexpr (matchesType<ElementType, float>())     { value.type.addArrayElements (Type::createFloat32(), 1); appendData (std::addressof (v), sizeof (v)); return; }
    if constexpr (matchesType<ElementType, double>())    { value.type.addArrayElements (Type::createFloat64(), 1); appendData (std::addressof (v), sizeof (v)); return; }
    if constexpr (matchesType<ElementType, bool>())      { value.type.addArrayElements (Type::createBool(),    1); BoolStorageType b = v ? 1 : 0; appendData (std::addressof (b), sizeof (b)); return; }

    if constexpr (isStringType<ElementType>())
    {
        value.type.addArrayElements (Type::createString(), 1);
        auto stringHandle = dictionary.getHandleForString (v);
        return appendData (std::addressof (stringHandle.handle), sizeof (stringHandle.handle));
    }

    if constexpr (isValueType<ElementType>())
    {
        value.type.addArrayElements (v.getType(), 1);
        return appendValue (v);
    }
}

inline Value createObject (std::string_view className)
{
    return Value (Type::createObject (className));
}

template <typename... Members>
inline Value createObject (std::string_view className, Members&&... members)
{
    static_assert ((sizeof...(members) & 1) == 0, "The member arguments must be a sequence of name, value pairs");

    auto v = createObject (className);
    v.addMember (std::forward<Members> (members)...);
    return v;
}

inline void Value::appendMember (std::string_view name, Type&& type, const void* data, size_t size)
{
    value.type.addObjectMember (name, std::move (type));
    appendData (data, size);
}

inline void Value::changeMember (uint32_t index, const Type& newType, void* newData, StringDictionary* newDictionary)
{
    auto info = value.type.getElementTypeAndOffset (index);

    if (info.elementType == newType)
    {
        auto elementAddress = value.data + info.offset;
        std::memcpy (elementAddress, newData, newType.getValueDataSize());

        if (newDictionary != nullptr)
        {
            // this will force an update of any handles in the newly-copied data
            ValueView v (newType, elementAddress, newDictionary);
            v.setDictionary (std::addressof (dictionary));
        }
    }
    else
    {
        // changing an existing member type involves re-packing the data..
        auto newCopy = createObject (getObjectClassName());
        auto numElements = value.type.getNumElements();

        for (uint32_t i = 0; i < numElements; ++i)
        {
            auto member = value.type.getObjectMember (i);
            newCopy.addMember (member.name, i == index ? ValueView (newType, newData, newDictionary) : value[i]);
        }

        *this = std::move (newCopy);
    }
}

template <typename MemberType, typename... Others>
void Value::addMember (std::string_view name, MemberType v, Others&&... others)
{
    static_assert ((sizeof...(others) & 1) == 0, "The arguments must be a sequence of name, value pairs");

    static_assert (isPrimitiveType<MemberType>() || isStringType<MemberType>() || isValueType<MemberType>(),
                   "The template type needs to be one of the supported primitive types");

    if constexpr (isValueType<MemberType>())
    {
        value.type.addObjectMember (name, v.getType());
        appendValue (v);
    }
    else if constexpr (isStringType<MemberType>())
    {
        auto stringHandle = dictionary.getHandleForString (v);
        appendMember (name, Type::createString(), std::addressof (stringHandle.handle), sizeof (stringHandle.handle));
    }
    else if constexpr (matchesType<MemberType, int32_t>())   { appendMember (name, Type::createInt32(),   std::addressof (v), sizeof (v)); }
    else if constexpr (matchesType<MemberType, int64_t>())   { appendMember (name, Type::createInt64(),   std::addressof (v), sizeof (v)); }
    else if constexpr (matchesType<MemberType, float>())     { appendMember (name, Type::createFloat32(), std::addressof (v), sizeof (v)); }
    else if constexpr (matchesType<MemberType, double>())    { appendMember (name, Type::createFloat64(), std::addressof (v), sizeof (v)); }
    else if constexpr (matchesType<MemberType, bool>())      { BoolStorageType b = v ? 1 : 0; appendMember (name, Type::createBool(), std::addressof (b), sizeof (b)); }

    if constexpr (sizeof...(others) != 0)
        addMember (std::forward<Others> (others)...);
}

template <typename MemberType>
void Value::setMember (std::string_view name, MemberType v)
{
    static_assert (isPrimitiveType<MemberType>() || isStringType<MemberType>() || isValueType<MemberType>(),
                   "The template type needs to be one of the supported primitive types");

    check (isObject(), "setMember() can only be called on an object");

    auto index = value.type.getObjectMemberIndex (name);

    if (index < 0)
        return addMember (name, v);

    if constexpr (isValueType<MemberType>())
    {
        changeMember (static_cast<uint32_t> (index), v.getType(), v.getRawData(), v.getDictionary());
    }
    else if constexpr (isStringType<MemberType>())
    {
        auto stringHandle = dictionary.getHandleForString (v);
        changeMember (static_cast<uint32_t> (index), Type::createString(), std::addressof (stringHandle.handle), std::addressof (dictionary));
    }
    else if constexpr (matchesType<MemberType, int32_t>())   { changeMember (static_cast<uint32_t> (index), Type::createInt32(),   std::addressof (v), nullptr); }
    else if constexpr (matchesType<MemberType, int64_t>())   { changeMember (static_cast<uint32_t> (index), Type::createInt64(),   std::addressof (v), nullptr); }
    else if constexpr (matchesType<MemberType, float>())     { changeMember (static_cast<uint32_t> (index), Type::createFloat32(), std::addressof (v), nullptr); }
    else if constexpr (matchesType<MemberType, double>())    { changeMember (static_cast<uint32_t> (index), Type::createFloat64(), std::addressof (v), nullptr); }
    else if constexpr (matchesType<MemberType, bool>())      { BoolStorageType b = v ? 1 : 0; changeMember (static_cast<uint32_t> (index), Type::createBool(), std::addressof (b), nullptr); }
}

template <typename TargetType> TargetType Value::get() const                           { return value.get<TargetType>(); }
template <typename TargetType> TargetType Value::getWithDefault (TargetType d) const   { return value.getWithDefault<TargetType> (std::forward<TargetType> (d)); }
inline std::string Value::toString() const                                             { return value.toString(); }

inline ValueView::Iterator Value::begin() const    { return value.begin(); }
inline ValueView::EndIterator Value::end() const   { return {}; }

template <typename OutputStream> void Value::serialise (OutputStream& o) const
{
    value.type.serialise (o);

    if (! value.type.isVoid())
    {
        o.write (getRawData(), value.type.getValueDataSize());

        if (auto stringDataSize = static_cast<uint32_t> (dictionary.getRawDataSize()))
        {
            Type::SerialisationHelpers::writeVariableLengthInt (o, stringDataSize);
            o.write (dictionary.getRawData(), stringDataSize);
        }
    }
}

inline SerialisedData Value::serialise() const
{
    SerialisedData result;
    serialise (result);
    return result;
}

inline Value Value::deserialise (InputData& input)
{
    auto type = Type::deserialise (input);
    auto valueDataSize = type.getValueDataSize();
    Type::SerialisationHelpers::expect (input.end >= input.start + valueDataSize);
    Value v (std::move (type));
    std::memcpy (v.getRawData(), input.start, valueDataSize);
    input.start += valueDataSize;

    if (input.end > input.start)
    {
        auto stringDataSize = Type::SerialisationHelpers::readVariableLengthInt (input);
        Type::SerialisationHelpers::expect (stringDataSize <= static_cast<uint32_t> (input.end - input.start));
        v.dictionary.setRawData (input.start, stringDataSize);
    }

    return v;
}

//==============================================================================
inline Value Type::toValue() const
{
    auto valueForArray = [] (const ComplexArray& a) -> Value
    {
        if (a.groups.empty())
            return value::createObject ({}, "type", "array");

        auto groupList = value::createEmptyArray();

        for (auto& g : a.groups)
            groupList.addArrayElement (value::createObject ({},
                                                            "type", g.elementType.toValue(),
                                                            "size", static_cast<int32_t> (g.repetitions)));

        return value::createObject ({},
                                    "type", "array",
                                    "types", groupList);
    };

    auto valueForObject = [] (const Object& o) -> Value
    {
        auto v = value::createObject ({},
                                      "type", "object");

        if (! o.className.empty())
            v.addMember ("class", o.className);

        if (! o.members.empty())
        {
            auto members = value::createObject ({});

            for (auto& m : o.members)
                members.addMember (m.name, m.type.toValue());

            v.addMember ("members", members);
        }

        return v;
    };

    switch (mainType)
    {
        case MainType::void_:           return value::createObject ({}, "type", "void");
        case MainType::int32:           return value::createObject ({}, "type", "int32");
        case MainType::int64:           return value::createObject ({}, "type", "int64");
        case MainType::float32:         return value::createObject ({}, "type", "float32");
        case MainType::float64:         return value::createObject ({}, "type", "float64");
        case MainType::boolean:         return value::createObject ({}, "type", "bool");
        case MainType::string:          return value::createObject ({}, "type", "string");
        case MainType::vector:          return value::createObject ({}, "type", "vector", "element", getElementType().toValue(), "size", static_cast<int32_t> (getNumElements()));
        case MainType::primitiveArray:  return value::createObject ({}, "type", "array",  "element", getElementType().toValue(), "size", static_cast<int32_t> (getNumElements()));
        case MainType::complexArray:    return valueForArray (*content.complexArray);
        case MainType::object:          return valueForObject (*content.object);
        default:                        throwError ("Invalid type");
    }
}

inline Type Type::fromValue (const ValueView& value)
{
    auto fromVector = [] (const ValueView& v) -> Type
    {
        auto elementType = fromValue (v["element"]);
        check (elementType.isPrimitive(), "Vectors can only contain primitive elements");
        return Type (elementType.mainType, v["size"].get<uint32_t>());
    };

    auto fromArray = [] (const ValueView& v) -> Type
    {
        if (v.hasObjectMember ("element"))
            return createArray (fromValue (v["element"]), v["size"].get<uint32_t>());

        if (v.hasObjectMember ("types"))
        {
            auto result = Type::createEmptyArray();

            for (auto group : v["types"])
                result.addArrayElements (fromValue (group["type"]), group["size"].get<uint32_t>());

            return result;
        }

       throwError ("This value doesn't match the format generated by Type::toValue()");
    };

    auto fromObject = [] (const ValueView& v) -> Type
    {
        auto classNameMember = v.getType().getObjectMemberIndex ("class");
        std::string_view className;

        if (classNameMember >= 0)
            className = v.getObjectMemberAt (static_cast<uint32_t> (classNameMember)).value.get<std::string_view>();

        auto o = createObject (className);

        if (v.hasObjectMember ("members"))
        {
            v["members"].visitObjectMembers ([&o] (std::string_view name, const ValueView& mv)
            {
                o.addObjectMember (name, fromValue (mv));
            });
        }

        return o;
    };

    if (value.isObject() && value.hasObjectMember ("type"))
    {
        auto name = value["type"].get<std::string_view>();

        if (name == "void")     return {};
        if (name == "int32")    return Type::createInt32();
        if (name == "int64")    return Type::createInt64();
        if (name == "float32")  return Type::createFloat32();
        if (name == "float64")  return Type::createFloat64();
        if (name == "bool")     return Type::createBool();
        if (name == "string")   return Type::createString();
        if (name == "vector")   return fromVector (value);
        if (name == "array")    return fromArray (value);
        if (name == "object")   return fromObject (value);
    }

    throwError ("This value doesn't match the format generated by Type::toValue()");
}

inline std::string Type::getDescription() const
{
    auto getComplexArrayDesc = [] (const ComplexArray& a)
    {
        std::string s = "array (";
        bool first = true;

        for (auto& g : a.groups)
        {
            if (first)
                first = false;
            else
                s += ", ";

            s += std::to_string (g.repetitions) + " x " + g.elementType.getDescription();
        }

        return s + ")";
    };

    auto getObjectDesc = [] (const Object& o)
    {
        std::string s = "object ";

        if (! o.className.empty())
        {
            s += '\"';
            s += o.className;
            s += "\" ";
        }

        s += "{ ";
        bool first = true;

        for (uint32_t i = 0; i < o.members.size; ++i)
        {
            if (first)
                first = false;
            else
                s += ", ";

            s += o.members[i].name;
            s += ": ";
            s += o.members[i].type.getDescription();
        }

        return s + " }";
    };

    switch (mainType)
    {
        case MainType::void_:           return "void";
        case MainType::int32:           return "int32";
        case MainType::int64:           return "int64";
        case MainType::float32:         return "float32";
        case MainType::float64:         return "float64";
        case MainType::boolean:         return "bool";
        case MainType::string:          return "string";
        case MainType::vector:          return "vector " + std::to_string (getNumElements()) + " x " + getElementType().getDescription();
        case MainType::primitiveArray:  return "array " + std::to_string (getNumElements()) + " x " + getElementType().getDescription();
        case MainType::complexArray:    return getComplexArrayDesc (*content.complexArray);
        case MainType::object:          return getObjectDesc (*content.object);
        default:                        throwError ("Invalid type");
    }
}

inline std::string Type::getSignature (bool includeNames) const
{
    auto getComplexArraySignature = [] (const ComplexArray& a, bool useNames)
    {
        auto numElements = a.size();
        auto s = 'A' + std::to_string (numElements);

        for (auto& g : a.groups)
            s += '_' + std::to_string (g.repetitions) + 'x' + g.elementType.getSignature (useNames);

        return s;
    };

    auto getObjectSignature = [] (const Object& o, bool useNames)
    {
        auto numElements = o.members.size;
        auto s = 'o' + std::to_string (numElements);

        if (useNames && ! o.className.empty())
        {
            s += '_';
            s += o.className;
        }

        for (uint32_t i = 0; i < numElements; ++i)
        {
            if (useNames)
            {
                s += '_';
                s += o.members[i].name;
            }

            s += '_' + o.members[i].type.getSignature (useNames);
        }

        return s;
    };

    switch (mainType)
    {
        case MainType::void_:           return "v";
        case MainType::int32:           return "i32";
        case MainType::int64:           return "i64";
        case MainType::float32:         return "f32";
        case MainType::float64:         return "f64";
        case MainType::boolean:         return "b";
        case MainType::string:          return "s";
        case MainType::vector:          return 'V' + std::to_string (getNumElements()) + '_' + getElementType().getSignature (includeNames);
        case MainType::primitiveArray:  return 'a' + std::to_string (getNumElements()) + '_' + getElementType().getSignature (includeNames);
        case MainType::complexArray:    return getComplexArraySignature (*content.complexArray, includeNames);
        case MainType::object:          return getObjectSignature (*content.object, includeNames);
        default:                        throwError ("Invalid type");
    }
}

//==============================================================================
inline SimpleStringDictionary::Handle SimpleStringDictionary::getHandleForString (std::string_view text)
{
    if (text.empty())
        return {};

    auto i = findGreaterThanOrEqual (text);

    if (i.second)
        return { *i.first };

    auto newHandle = static_cast<decltype(Handle::handle)> (strings.size() + 1);

    if (strings.size() > 100 && (strings.capacity() < (strings.size() + text.length() + 1)))
        strings.reserve (strings.size() + 1000);

    strings.insert (strings.end(), text.begin(), text.end());
    strings.push_back (0);

    stringMap.insert (i.first, newHandle);
    return { newHandle };
}

inline std::string_view SimpleStringDictionary::getStringForHandle (Handle handle) const
{
    if (handle == Handle())
        return {};

    if (handle.handle > strings.size())
        throwError ("Unknown string");

    return std::string_view (strings.data() + (handle.handle - 1));
}

inline void SimpleStringDictionary::clear()     { strings.clear(); stringMap.clear(); }

inline void SimpleStringDictionary::setRawData (const void* p, size_t n)
{
    strings.resize (n);
    std::memcpy (strings.data(), p, n);

    // Populate string map
    for (size_t i = 0; i < strings.size(); ++i)
    {
        std::string_view sv (strings.data() + i);
        auto v = findGreaterThanOrEqual (sv);
        stringMap.insert (v.first, static_cast<uint32_t> (i));
        i += sv.length();
    }
}

inline std::pair<std::vector<uint32_t>::const_iterator, bool> SimpleStringDictionary::findGreaterThanOrEqual (std::string_view v) const
{
    bool exactMatch = false;

    auto it = std::lower_bound (stringMap.begin(), stringMap.end(), v, [&] (uint32_t i, std::string_view sv) -> bool
    {
        auto c = sv.compare (getStringForHandle ( { i }));

        if (c == 0)
            exactMatch = true;

        return c > 0;
    });

    return std::pair (it, exactMatch);
}

} // namespace choc::value


#endif // CHOC_VALUE_POOL_HEADER_INCLUDED
