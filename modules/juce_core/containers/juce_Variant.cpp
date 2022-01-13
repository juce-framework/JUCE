/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

enum VariantStreamMarkers
{
    varMarker_Int       = 1,
    varMarker_BoolTrue  = 2,
    varMarker_BoolFalse = 3,
    varMarker_Double    = 4,
    varMarker_String    = 5,
    varMarker_Int64     = 6,
    varMarker_Array     = 7,
    varMarker_Binary    = 8,
    varMarker_Undefined = 9
};

//==============================================================================
struct var::VariantType
{
    struct VoidTag      {};
    struct UndefinedTag {};
    struct IntTag       {};
    struct Int64Tag     {};
    struct DoubleTag    {};
    struct BoolTag      {};
    struct StringTag    {};
    struct ObjectTag    {};
    struct ArrayTag     {};
    struct BinaryTag    {};
    struct MethodTag    {};

    // members =====================================================================
    bool isVoid         = false;
    bool isUndefined    = false;
    bool isInt          = false;
    bool isInt64        = false;
    bool isBool         = false;
    bool isDouble       = false;
    bool isString       = false;
    bool isObject       = false;
    bool isArray        = false;
    bool isBinary       = false;
    bool isMethod       = false;
    bool isComparable   = false;

    int                     (*toInt)         (const ValueUnion&)                 = defaultToInt;
    int64                   (*toInt64)       (const ValueUnion&)                 = defaultToInt64;
    double                  (*toDouble)      (const ValueUnion&)                 = defaultToDouble;
    String                  (*toString)      (const ValueUnion&)                 = defaultToString;
    bool                    (*toBool)        (const ValueUnion&)                 = defaultToBool;
    ReferenceCountedObject* (*toObject)      (const ValueUnion&)                 = defaultToObject;
    Array<var>*             (*toArray)       (const ValueUnion&)                 = defaultToArray;
    MemoryBlock*            (*toBinary)      (const ValueUnion&)                 = defaultToBinary;
    var                     (*clone)         (const var&)                        = defaultClone;
    void                    (*cleanUp)       (ValueUnion&)                       = defaultCleanUp;
    void                    (*createCopy)    (ValueUnion&, const ValueUnion&)    = defaultCreateCopy;

    bool                    (*equals)        (const ValueUnion&, const ValueUnion&, const VariantType&) = nullptr;
    void                    (*writeToStream) (const ValueUnion&, OutputStream&) = nullptr;

    // defaults ====================================================================
    static int                     defaultToInt         (const ValueUnion&)                          { return 0; }
    static int64                   defaultToInt64       (const ValueUnion&)                          { return 0; }
    static double                  defaultToDouble      (const ValueUnion&)                          { return 0; }
    static String                  defaultToString      (const ValueUnion&)                          { return {}; }
    static bool                    defaultToBool        (const ValueUnion&)                          { return false; }
    static ReferenceCountedObject* defaultToObject      (const ValueUnion&)                          { return nullptr; }
    static Array<var>*             defaultToArray       (const ValueUnion&)                          { return nullptr; }
    static MemoryBlock*            defaultToBinary      (const ValueUnion&)                          { return nullptr; }
    static var                     defaultClone         (const var& other)                           { return other; }
    static void                    defaultCleanUp       (ValueUnion&)                                {}
    static void                    defaultCreateCopy    (ValueUnion& dest, const ValueUnion& source) { dest = source; }

    // void ========================================================================
    static bool voidEquals (const ValueUnion&, const ValueUnion&, const VariantType& otherType) noexcept
    {
        return otherType.isVoid || otherType.isUndefined;
    }

    static void voidWriteToStream (const ValueUnion&, OutputStream& output)
    {
        output.writeCompressedInt (0);
    }

    constexpr explicit VariantType (VoidTag) noexcept
        : isVoid            (true),
          isComparable      (true),
          equals            (voidEquals),
          writeToStream     (voidWriteToStream) {}

    // undefined ===================================================================
    static String undefinedToString (const ValueUnion&) { return "undefined"; }

    static bool undefinedEquals (const ValueUnion&, const ValueUnion&, const VariantType& otherType) noexcept
    {
        return otherType.isVoid || otherType.isUndefined;
    }

    static void undefinedWriteToStream (const ValueUnion&, OutputStream& output)
    {
        output.writeCompressedInt (1);
        output.writeByte (varMarker_Undefined);
    }

    constexpr explicit VariantType (UndefinedTag) noexcept
        : isUndefined   (true),
          toString      (undefinedToString),
          equals        (undefinedEquals),
          writeToStream (undefinedWriteToStream) {}

    // int =========================================================================
    static int    intToInt    (const ValueUnion& data) noexcept   { return data.intValue; }
    static int64  intToInt64  (const ValueUnion& data) noexcept   { return (int64) data.intValue; }
    static double intToDouble (const ValueUnion& data) noexcept   { return (double) data.intValue; }
    static String intToString (const ValueUnion& data)            { return String (data.intValue); }
    static bool   intToBool   (const ValueUnion& data) noexcept   { return data.intValue != 0; }

    static bool intEquals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) noexcept
    {
        if (otherType.isDouble || otherType.isInt64 || otherType.isString)
            return otherType.equals (otherData, data, VariantType { IntTag{} });

        return otherType.toInt (otherData) == data.intValue;
    }

    static void intWriteToStream (const ValueUnion& data, OutputStream& output)
    {
        output.writeCompressedInt (5);
        output.writeByte (varMarker_Int);
        output.writeInt (data.intValue);
    }

    constexpr explicit VariantType (IntTag) noexcept
        : isInt         (true),
          isComparable  (true),
          toInt         (intToInt),
          toInt64       (intToInt64),
          toDouble      (intToDouble),
          toString      (intToString),
          toBool        (intToBool),
          equals        (intEquals),
          writeToStream (intWriteToStream) {}

    // int64 =======================================================================
    static int    int64ToInt    (const ValueUnion& data) noexcept   { return (int) data.int64Value; }
    static int64  int64ToInt64  (const ValueUnion& data) noexcept   { return data.int64Value; }
    static double int64ToDouble (const ValueUnion& data) noexcept   { return (double) data.int64Value; }
    static String int64ToString (const ValueUnion& data)            { return String (data.int64Value); }
    static bool   int64ToBool   (const ValueUnion& data) noexcept   { return data.int64Value != 0; }

    static bool int64Equals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) noexcept
    {
        if (otherType.isDouble || otherType.isString)
            return otherType.equals (otherData, data, VariantType { Int64Tag{} });

        return otherType.toInt64 (otherData) == data.int64Value;
    }

    static void int64WriteToStream (const ValueUnion& data, OutputStream& output)
    {
        output.writeCompressedInt (9);
        output.writeByte (varMarker_Int64);
        output.writeInt64 (data.int64Value);
    }

    constexpr explicit VariantType (Int64Tag) noexcept
        : isInt64       (true),
          isComparable  (true),
          toInt         (int64ToInt),
          toInt64       (int64ToInt64),
          toDouble      (int64ToDouble),
          toString      (int64ToString),
          toBool        (int64ToBool),
          equals        (int64Equals),
          writeToStream (int64WriteToStream) {}

    // double ======================================================================
    static int    doubleToInt    (const ValueUnion& data) noexcept   { return (int) data.doubleValue; }
    static int64  doubleToInt64  (const ValueUnion& data) noexcept   { return (int64) data.doubleValue; }
    static double doubleToDouble (const ValueUnion& data) noexcept   { return data.doubleValue; }
    static String doubleToString (const ValueUnion& data)            { return serialiseDouble (data.doubleValue); }
    static bool   doubleToBool   (const ValueUnion& data) noexcept   { return data.doubleValue != 0.0; }

    static bool doubleEquals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) noexcept
    {
        return std::abs (otherType.toDouble (otherData) - data.doubleValue) < std::numeric_limits<double>::epsilon();
    }

    static void doubleWriteToStream (const ValueUnion& data, OutputStream& output)
    {
        output.writeCompressedInt (9);
        output.writeByte (varMarker_Double);
        output.writeDouble (data.doubleValue);
    }

    constexpr explicit VariantType (DoubleTag) noexcept
        : isDouble      (true),
          isComparable  (true),
          toInt         (doubleToInt),
          toInt64       (doubleToInt64),
          toDouble      (doubleToDouble),
          toString      (doubleToString),
          toBool        (doubleToBool),
          equals        (doubleEquals),
          writeToStream (doubleWriteToStream) {}

    // bool ========================================================================
    static int    boolToInt    (const ValueUnion& data) noexcept   { return data.boolValue ? 1 : 0; }
    static int64  boolToInt64  (const ValueUnion& data) noexcept   { return data.boolValue ? 1 : 0; }
    static double boolToDouble (const ValueUnion& data) noexcept   { return data.boolValue ? 1.0 : 0.0; }
    static String boolToString (const ValueUnion& data)            { return String::charToString (data.boolValue ? (juce_wchar) '1' : (juce_wchar) '0'); }
    static bool   boolToBool   (const ValueUnion& data) noexcept   { return data.boolValue; }

    static bool boolEquals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) noexcept
    {
        return otherType.toBool (otherData) == data.boolValue;
    }

    static void boolWriteToStream (const ValueUnion& data, OutputStream& output)
    {
        output.writeCompressedInt (1);
        output.writeByte (data.boolValue ? (char) varMarker_BoolTrue : (char) varMarker_BoolFalse);
    }

    constexpr explicit VariantType (BoolTag) noexcept
        : isBool        (true),
          isComparable  (true),
          toInt         (boolToInt),
          toInt64       (boolToInt64),
          toDouble      (boolToDouble),
          toString      (boolToString),
          toBool        (boolToBool),
          equals        (boolEquals),
          writeToStream (boolWriteToStream) {}

    // string ======================================================================
    static const String* getString (const ValueUnion& data) noexcept   { return unalignedPointerCast<const String*> (data.stringValue); }
    static       String* getString (      ValueUnion& data) noexcept   { return unalignedPointerCast<String*> (data.stringValue); }

    static int    stringToInt    (const ValueUnion& data) noexcept   { return getString (data)->getIntValue(); }
    static int64  stringToInt64  (const ValueUnion& data) noexcept   { return getString (data)->getLargeIntValue(); }
    static double stringToDouble (const ValueUnion& data) noexcept   { return getString (data)->getDoubleValue(); }
    static String stringToString (const ValueUnion& data)            { return *getString (data); }
    static bool   stringToBool   (const ValueUnion& data) noexcept
    {
        return getString (data)->getIntValue() != 0
               || getString (data)->trim().equalsIgnoreCase ("true")
               || getString (data)->trim().equalsIgnoreCase ("yes");
    }

    static void stringCleanUp    (ValueUnion& data) noexcept                    { getString (data)-> ~String(); }
    static void stringCreateCopy (ValueUnion& dest, const ValueUnion& source)   { new (dest.stringValue) String (*getString (source)); }

    static bool stringEquals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) noexcept
    {
        return otherType.toString (otherData) == *getString (data);
    }

    static void stringWriteToStream (const ValueUnion& data, OutputStream& output)
    {
        auto* s = getString (data);
        const size_t len = s->getNumBytesAsUTF8() + 1;
        HeapBlock<char> temp (len);
        s->copyToUTF8 (temp, len);
        output.writeCompressedInt ((int) (len + 1));
        output.writeByte (varMarker_String);
        output.write (temp, len);
    }

    constexpr explicit VariantType (StringTag) noexcept
        : isString      (true),
          isComparable  (true),
          toInt         (stringToInt),
          toInt64       (stringToInt64),
          toDouble      (stringToDouble),
          toString      (stringToString),
          toBool        (stringToBool),
          cleanUp       (stringCleanUp),
          createCopy    (stringCreateCopy),
          equals        (stringEquals),
          writeToStream (stringWriteToStream) {}

    // object ======================================================================
    static String objectToString (const ValueUnion& data)
    {
        return "Object 0x" + String::toHexString ((int) (pointer_sized_int) data.objectValue);
    }

    static bool                    objectToBool   (const ValueUnion& data) noexcept   { return data.objectValue != nullptr; }
    static ReferenceCountedObject* objectToObject (const ValueUnion& data) noexcept   { return data.objectValue; }

    static var objectClone (const var& original)
    {
        if (auto* d = original.getDynamicObject())
            return d->clone().get();

        jassertfalse; // can only clone DynamicObjects!
        return {};
    }

    static void objectCleanUp (ValueUnion& data) noexcept   { if (data.objectValue != nullptr) data.objectValue->decReferenceCount(); }

    static void objectCreateCopy (ValueUnion& dest, const ValueUnion& source)
    {
        dest.objectValue = source.objectValue;
        if (dest.objectValue != nullptr)
            dest.objectValue->incReferenceCount();
    }

    static bool objectEquals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) noexcept
    {
        return otherType.toObject (otherData) == data.objectValue;
    }

    static void objectWriteToStream (const ValueUnion&, OutputStream& output)
    {
        jassertfalse; // Can't write an object to a stream!
        output.writeCompressedInt (0);
    }

    constexpr explicit VariantType (ObjectTag) noexcept
        : isObject      (true),
          toString      (objectToString),
          toBool        (objectToBool),
          toObject      (objectToObject),
          clone         (objectClone),
          cleanUp       (objectCleanUp),
          createCopy    (objectCreateCopy),
          equals        (objectEquals),
          writeToStream (objectWriteToStream) {}

    // array =======================================================================
    static String                  arrayToString (const ValueUnion&)            { return "[Array]"; }
    static ReferenceCountedObject* arrayToObject (const ValueUnion&) noexcept   { return nullptr; }

    static Array<var>* arrayToArray (const ValueUnion& data) noexcept
    {
        if (auto* a = dynamic_cast<RefCountedArray*> (data.objectValue))
            return &(a->array);

        return nullptr;
    }

    static bool arrayEquals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) noexcept
    {
        auto* thisArray = arrayToArray (data);
        auto* otherArray = otherType.toArray (otherData);
        return thisArray == otherArray || (thisArray != nullptr && otherArray != nullptr && *otherArray == *thisArray);
    }

    static var arrayClone (const var& original)
    {
        Array<var> arrayCopy;

        if (auto* array = arrayToArray (original.value))
        {
            arrayCopy.ensureStorageAllocated (array->size());

            for (auto& i : *array)
                arrayCopy.add (i.clone());
        }

        return var (arrayCopy);
    }

    static void arrayWriteToStream (const ValueUnion& data, OutputStream& output)
    {
        if (auto* array = arrayToArray (data))
        {
            MemoryOutputStream buffer (512);
            buffer.writeCompressedInt (array->size());

            for (auto& i : *array)
                i.writeToStream (buffer);

            output.writeCompressedInt (1 + (int) buffer.getDataSize());
            output.writeByte (varMarker_Array);
            output << buffer;
        }
    }

    struct RefCountedArray  : public ReferenceCountedObject
    {
        RefCountedArray (const Array<var>& a)  : array (a)  { incReferenceCount(); }
        RefCountedArray (Array<var>&& a)  : array (std::move (a)) { incReferenceCount(); }
        Array<var> array;
    };

    constexpr explicit VariantType (ArrayTag) noexcept
        : isObject      (true),
          isArray       (true),
          toString      (arrayToString),
          toBool        (objectToBool),
          toObject      (arrayToObject),
          toArray       (arrayToArray),
          clone         (arrayClone),
          cleanUp       (objectCleanUp),
          createCopy    (objectCreateCopy),
          equals        (arrayEquals),
          writeToStream (arrayWriteToStream) {}

    // binary ======================================================================
    static void binaryCleanUp    (ValueUnion& data) noexcept                    { delete data.binaryValue; }
    static void binaryCreateCopy (ValueUnion& dest, const ValueUnion& source)   { dest.binaryValue = new MemoryBlock (*source.binaryValue); }

    static String       binaryToString (const ValueUnion& data)            { return data.binaryValue->toBase64Encoding(); }
    static MemoryBlock* binaryToBinary (const ValueUnion& data) noexcept   { return data.binaryValue; }

    static bool binaryEquals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) noexcept
    {
        const MemoryBlock* const otherBlock = otherType.toBinary (otherData);
        return otherBlock != nullptr && *otherBlock == *data.binaryValue;
    }

    static void binaryWriteToStream (const ValueUnion& data, OutputStream& output)
    {
        output.writeCompressedInt (1 + (int) data.binaryValue->getSize());
        output.writeByte (varMarker_Binary);
        output << *data.binaryValue;
    }

    constexpr explicit VariantType (BinaryTag) noexcept
        : isBinary      (true),
          toString      (binaryToString),
          toBinary      (binaryToBinary),
          cleanUp       (binaryCleanUp),
          createCopy    (binaryCreateCopy),
          equals        (binaryEquals),
          writeToStream (binaryWriteToStream) {}

    // method ======================================================================
    static void methodCleanUp    (ValueUnion& data) noexcept                    { if (data.methodValue != nullptr ) delete data.methodValue; }
    static void methodCreateCopy (ValueUnion& dest, const ValueUnion& source)   { dest.methodValue = new NativeFunction (*source.methodValue); }

    static String methodToString (const ValueUnion&)                 { return "Method"; }
    static bool   methodToBool   (const ValueUnion& data) noexcept   { return data.methodValue != nullptr; }

    static bool methodEquals (const ValueUnion& data, const ValueUnion& otherData, const VariantType& otherType) noexcept
    {
        return otherType.isMethod && otherData.methodValue == data.methodValue;
    }

    static void methodWriteToStream (const ValueUnion&, OutputStream& output)
    {
        jassertfalse; // Can't write a method to a stream!
        output.writeCompressedInt (0);
    }

    constexpr explicit VariantType (MethodTag) noexcept
        : isMethod      (true),
          toString      (methodToString),
          toBool        (methodToBool),
          cleanUp       (methodCleanUp),
          createCopy    (methodCreateCopy),
          equals        (methodEquals),
          writeToStream (methodWriteToStream) {}
};

struct var::Instance
{
    static constexpr VariantType attributesVoid           { VariantType::VoidTag{} };
    static constexpr VariantType attributesUndefined      { VariantType::UndefinedTag{} };
    static constexpr VariantType attributesInt            { VariantType::IntTag{} };
    static constexpr VariantType attributesInt64          { VariantType::Int64Tag{} };
    static constexpr VariantType attributesBool           { VariantType::BoolTag{} };
    static constexpr VariantType attributesDouble         { VariantType::DoubleTag{} };
    static constexpr VariantType attributesMethod         { VariantType::MethodTag{} };
    static constexpr VariantType attributesArray          { VariantType::ArrayTag{} };
    static constexpr VariantType attributesString         { VariantType::StringTag{} };
    static constexpr VariantType attributesBinary         { VariantType::BinaryTag{} };
    static constexpr VariantType attributesObject         { VariantType::ObjectTag{} };
};

constexpr var::VariantType var::Instance::attributesVoid;
constexpr var::VariantType var::Instance::attributesUndefined;
constexpr var::VariantType var::Instance::attributesInt;
constexpr var::VariantType var::Instance::attributesInt64;
constexpr var::VariantType var::Instance::attributesBool;
constexpr var::VariantType var::Instance::attributesDouble;
constexpr var::VariantType var::Instance::attributesMethod;
constexpr var::VariantType var::Instance::attributesArray;
constexpr var::VariantType var::Instance::attributesString;
constexpr var::VariantType var::Instance::attributesBinary;
constexpr var::VariantType var::Instance::attributesObject;

//==============================================================================
var::var() noexcept : type (&Instance::attributesVoid) {}
var::var (const VariantType& t) noexcept  : type (&t) {}
var::~var() noexcept  { type->cleanUp (value); }

//==============================================================================
var::var (const var& valueToCopy)  : type (valueToCopy.type)
{
    type->createCopy (value, valueToCopy.value);
}

var::var (const int v) noexcept       : type (&Instance::attributesInt)    { value.intValue = v; }
var::var (const int64 v) noexcept     : type (&Instance::attributesInt64)  { value.int64Value = v; }
var::var (const bool v) noexcept      : type (&Instance::attributesBool)   { value.boolValue = v; }
var::var (const double v) noexcept    : type (&Instance::attributesDouble) { value.doubleValue = v; }
var::var (NativeFunction m) noexcept  : type (&Instance::attributesMethod) { value.methodValue = new NativeFunction (m); }
var::var (const Array<var>& v)        : type (&Instance::attributesArray)  { value.objectValue = new VariantType::RefCountedArray (v); }
var::var (const String& v)            : type (&Instance::attributesString) { new (value.stringValue) String (v); }
var::var (const char* const v)        : type (&Instance::attributesString) { new (value.stringValue) String (v); }
var::var (const wchar_t* const v)     : type (&Instance::attributesString) { new (value.stringValue) String (v); }
var::var (const void* v, size_t sz)   : type (&Instance::attributesBinary) { value.binaryValue = new MemoryBlock (v, sz); }
var::var (const MemoryBlock& v)       : type (&Instance::attributesBinary) { value.binaryValue = new MemoryBlock (v); }

var::var (const StringArray& v)       : type (&Instance::attributesArray)
{
    Array<var> strings;
    strings.ensureStorageAllocated (v.size());

    for (auto& i : v)
        strings.add (var (i));

    value.objectValue = new VariantType::RefCountedArray (strings);
}

var::var (ReferenceCountedObject* const object)  : type (&Instance::attributesObject)
{
    value.objectValue = object;

    if (object != nullptr)
        object->incReferenceCount();
}

var var::undefined() noexcept           { return var (Instance::attributesUndefined); }

//==============================================================================
bool var::isVoid() const noexcept       { return type->isVoid; }
bool var::isUndefined() const noexcept  { return type->isUndefined; }
bool var::isInt() const noexcept        { return type->isInt; }
bool var::isInt64() const noexcept      { return type->isInt64; }
bool var::isBool() const noexcept       { return type->isBool; }
bool var::isDouble() const noexcept     { return type->isDouble; }
bool var::isString() const noexcept     { return type->isString; }
bool var::isObject() const noexcept     { return type->isObject; }
bool var::isArray() const noexcept      { return type->isArray; }
bool var::isBinaryData() const noexcept { return type->isBinary; }
bool var::isMethod() const noexcept     { return type->isMethod; }

var::operator int() const noexcept                      { return type->toInt (value); }
var::operator int64() const noexcept                    { return type->toInt64 (value); }
var::operator bool() const noexcept                     { return type->toBool (value); }
var::operator float() const noexcept                    { return (float) type->toDouble (value); }
var::operator double() const noexcept                   { return type->toDouble (value); }
String var::toString() const                            { return type->toString (value); }
var::operator String() const                            { return type->toString (value); }
ReferenceCountedObject* var::getObject() const noexcept { return type->toObject (value); }
Array<var>* var::getArray() const noexcept              { return type->toArray (value); }
MemoryBlock* var::getBinaryData() const noexcept        { return type->toBinary (value); }
DynamicObject* var::getDynamicObject() const noexcept   { return dynamic_cast<DynamicObject*> (getObject()); }

//==============================================================================
void var::swapWith (var& other) noexcept
{
    std::swap (type, other.type);
    std::swap (value, other.value);
}

var& var::operator= (const var& v)               { type->cleanUp (value); type = v.type; type->createCopy (value, v.value); return *this; }
var& var::operator= (const int v)                { type->cleanUp (value); type = &Instance::attributesInt; value.intValue = v; return *this; }
var& var::operator= (const int64 v)              { type->cleanUp (value); type = &Instance::attributesInt64; value.int64Value = v; return *this; }
var& var::operator= (const bool v)               { type->cleanUp (value); type = &Instance::attributesBool; value.boolValue = v; return *this; }
var& var::operator= (const double v)             { type->cleanUp (value); type = &Instance::attributesDouble; value.doubleValue = v; return *this; }
var& var::operator= (const char* const v)        { type->cleanUp (value); type = &Instance::attributesString; new (value.stringValue) String (v); return *this; }
var& var::operator= (const wchar_t* const v)     { type->cleanUp (value); type = &Instance::attributesString; new (value.stringValue) String (v); return *this; }
var& var::operator= (const String& v)            { type->cleanUp (value); type = &Instance::attributesString; new (value.stringValue) String (v); return *this; }
var& var::operator= (const MemoryBlock& v)       { type->cleanUp (value); type = &Instance::attributesBinary; value.binaryValue = new MemoryBlock (v); return *this; }
var& var::operator= (const Array<var>& v)        { var v2 (v); swapWith (v2); return *this; }
var& var::operator= (ReferenceCountedObject* v)  { var v2 (v); swapWith (v2); return *this; }
var& var::operator= (NativeFunction v)           { var v2 (v); swapWith (v2); return *this; }

var::var (var&& other) noexcept
    : type (other.type),
      value (other.value)
{
    other.type = &Instance::attributesVoid;
}

var& var::operator= (var&& other) noexcept
{
    swapWith (other);
    return *this;
}

var::var (String&& v)  : type (&Instance::attributesString)
{
    new (value.stringValue) String (std::move (v));
}

var::var (MemoryBlock&& v)  : type (&Instance::attributesBinary)
{
    value.binaryValue = new MemoryBlock (std::move (v));
}

var::var (Array<var>&& v)  : type (&Instance::attributesArray)
{
    value.objectValue = new VariantType::RefCountedArray (std::move (v));
}

var& var::operator= (String&& v)
{
    type->cleanUp (value);
    type = &Instance::attributesString;
    new (value.stringValue) String (std::move (v));
    return *this;
}

//==============================================================================
bool var::equals (const var& other) const noexcept
{
    return type->equals (value, other.value, *other.type);
}

bool var::equalsWithSameType (const var& other) const noexcept
{
    return hasSameTypeAs (other) && equals (other);
}

bool var::hasSameTypeAs (const var& other) const noexcept
{
    return type == other.type;
}

bool canCompare (const var& v1, const var& v2)
{
    return v1.type->isComparable && v2.type->isComparable;
}

static int compare (const var& v1, const var& v2)
{
    if (v1.isString() && v2.isString())
        return v1.toString().compare (v2.toString());

    auto diff = static_cast<double> (v1) - static_cast<double> (v2);
    return diff == 0 ? 0 : (diff < 0 ? -1 : 1);
}

bool operator== (const var& v1, const var& v2)     { return v1.equals (v2); }
bool operator!= (const var& v1, const var& v2)     { return ! v1.equals (v2); }
bool operator<  (const var& v1, const var& v2)     { return canCompare (v1, v2) && compare (v1, v2) <  0; }
bool operator>  (const var& v1, const var& v2)     { return canCompare (v1, v2) && compare (v1, v2) >  0; }
bool operator<= (const var& v1, const var& v2)     { return canCompare (v1, v2) && compare (v1, v2) <= 0; }
bool operator>= (const var& v1, const var& v2)     { return canCompare (v1, v2) && compare (v1, v2) >= 0; }

bool operator== (const var& v1, const String& v2)  { return v1.toString() == v2; }
bool operator!= (const var& v1, const String& v2)  { return v1.toString() != v2; }
bool operator== (const var& v1, const char* v2)    { return v1.toString() == v2; }
bool operator!= (const var& v1, const char* v2)    { return v1.toString() != v2; }

//==============================================================================
var var::clone() const noexcept
{
    return type->clone (*this);
}

//==============================================================================
const var& var::operator[] (const Identifier& propertyName) const
{
    if (auto* o = getDynamicObject())
        return o->getProperty (propertyName);

    return getNullVarRef();
}

const var& var::operator[] (const char* const propertyName) const
{
    return operator[] (Identifier (propertyName));
}

var var::getProperty (const Identifier& propertyName, const var& defaultReturnValue) const
{
    if (auto* o = getDynamicObject())
        return o->getProperties().getWithDefault (propertyName, defaultReturnValue);

    return defaultReturnValue;
}

bool var::hasProperty (const Identifier& propertyName) const noexcept
{
    if (auto* o = getDynamicObject())
        return o->hasProperty (propertyName);

    return false;
}

var::NativeFunction var::getNativeFunction() const
{
    return isMethod() && (value.methodValue != nullptr) ? *value.methodValue : nullptr;
}

var var::invoke (const Identifier& method, const var* arguments, int numArguments) const
{
    if (auto* o = getDynamicObject())
        return o->invokeMethod (method, var::NativeFunctionArgs (*this, arguments, numArguments));

    return {};
}

var var::call (const Identifier& method) const
{
    return invoke (method, nullptr, 0);
}

var var::call (const Identifier& method, const var& arg1) const
{
    return invoke (method, &arg1, 1);
}

var var::call (const Identifier& method, const var& arg1, const var& arg2) const
{
    var args[] = { arg1, arg2 };
    return invoke (method, args, 2);
}

var var::call (const Identifier& method, const var& arg1, const var& arg2, const var& arg3)
{
    var args[] = { arg1, arg2, arg3 };
    return invoke (method, args, 3);
}

var var::call (const Identifier& method, const var& arg1, const var& arg2, const var& arg3, const var& arg4) const
{
    var args[] = { arg1, arg2, arg3, arg4 };
    return invoke (method, args, 4);
}

var var::call (const Identifier& method, const var& arg1, const var& arg2, const var& arg3, const var& arg4, const var& arg5) const
{
    var args[] = { arg1, arg2, arg3, arg4, arg5 };
    return invoke (method, args, 5);
}

//==============================================================================
int var::size() const
{
    if (auto array = getArray())
        return array->size();

    return 0;
}

const var& var::operator[] (int arrayIndex) const
{
    auto array = getArray();

    // When using this method, the var must actually be an array, and the index
    // must be in-range!
    jassert (array != nullptr && isPositiveAndBelow (arrayIndex, array->size()));

    return array->getReference (arrayIndex);
}

var& var::operator[] (int arrayIndex)
{
    auto array = getArray();

    // When using this method, the var must actually be an array, and the index
    // must be in-range!
    jassert (array != nullptr && isPositiveAndBelow (arrayIndex, array->size()));

    return array->getReference (arrayIndex);
}

Array<var>* var::convertToArray()
{
    if (auto array = getArray())
        return array;

    Array<var> tempVar;

    if (! isVoid())
        tempVar.add (*this);

    *this = tempVar;
    return getArray();
}

void var::append (const var& n)
{
    convertToArray()->add (n);
}

void var::remove (const int index)
{
    if (auto array = getArray())
        array->remove (index);
}

void var::insert (const int index, const var& n)
{
    convertToArray()->insert (index, n);
}

void var::resize (const int numArrayElementsWanted)
{
    convertToArray()->resize (numArrayElementsWanted);
}

int var::indexOf (const var& n) const
{
    if (auto array = getArray())
        return array->indexOf (n);

    return -1;
}

//==============================================================================
void var::writeToStream (OutputStream& output) const
{
    type->writeToStream (value, output);
}

var var::readFromStream (InputStream& input)
{
    const int numBytes = input.readCompressedInt();

    if (numBytes > 0)
    {
        switch (input.readByte())
        {
            case varMarker_Int:         return var (input.readInt());
            case varMarker_Int64:       return var (input.readInt64());
            case varMarker_BoolTrue:    return var (true);
            case varMarker_BoolFalse:   return var (false);
            case varMarker_Double:      return var (input.readDouble());

            case varMarker_String:
            {
                MemoryOutputStream mo;
                mo.writeFromInputStream (input, numBytes - 1);
                return var (mo.toUTF8());
            }

            case varMarker_Binary:
            {
                MemoryBlock mb ((size_t) numBytes - 1);

                if (numBytes > 1)
                {
                    const int numRead = input.read (mb.getData(), numBytes - 1);
                    mb.setSize ((size_t) numRead);
                }

                return var (mb);
            }

            case varMarker_Array:
            {
                var v;
                auto* destArray = v.convertToArray();

                for (int i = input.readCompressedInt(); --i >= 0;)
                    destArray->add (readFromStream (input));

                return v;
            }

            default:
                input.skipNextBytes (numBytes - 1); break;
        }
    }

    return {};
}

var::NativeFunctionArgs::NativeFunctionArgs (const var& t, const var* args, int numArgs) noexcept
    : thisObject (t), arguments (args), numArguments (numArgs)
{
}

//==============================================================================
#if JUCE_ALLOW_STATIC_NULL_VARIABLES

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4996)

const var var::null;

JUCE_END_IGNORE_WARNINGS_GCC_LIKE
JUCE_END_IGNORE_WARNINGS_MSVC

#endif

} // namespace juce
