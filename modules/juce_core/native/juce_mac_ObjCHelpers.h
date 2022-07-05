/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

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

#include "juce_mac_CFHelpers.h"

/* This file contains a few helper functions that are used internally but which
   need to be kept away from the public headers because they use obj-C symbols.
*/
namespace juce
{

//==============================================================================
inline Range<int> nsRangeToJuce (NSRange range)
{
    return { (int) range.location, (int) (range.location + range.length) };
}

inline NSRange juceRangeToNS (Range<int> range)
{
    return NSMakeRange ((NSUInteger) range.getStart(), (NSUInteger) range.getLength());
}

inline String nsStringToJuce (NSString* s)
{
    return CharPointer_UTF8 ([s UTF8String]);
}

inline NSString* juceStringToNS (const String& s)
{
    return [NSString stringWithUTF8String: s.toUTF8()];
}

inline NSString* nsStringLiteral (const char* const s) noexcept
{
    return [NSString stringWithUTF8String: s];
}

inline NSString* nsEmptyString() noexcept
{
    return [NSString string];
}

inline NSURL* createNSURLFromFile (const String& f)
{
    return [NSURL fileURLWithPath: juceStringToNS (f)];
}

inline NSURL* createNSURLFromFile (const File& f)
{
    return createNSURLFromFile (f.getFullPathName());
}

inline NSArray* createNSArrayFromStringArray (const StringArray& strings)
{
    auto array = [[NSMutableArray alloc] init];

    for (auto string: strings)
        [array addObject:juceStringToNS (string)];

    return [array autorelease];
}

inline NSArray* varArrayToNSArray (const var& varToParse);

inline NSDictionary* varObjectToNSDictionary (const var& varToParse)
{
    auto dictionary = [NSMutableDictionary dictionary];

    if (varToParse.isObject())
    {
        auto* dynamicObject = varToParse.getDynamicObject();

        auto& properties = dynamicObject->getProperties();

        for (int i = 0; i < properties.size(); ++i)
        {
            auto* keyString = juceStringToNS (properties.getName (i).toString());

            const var& valueVar = properties.getValueAt (i);

            if (valueVar.isObject())
            {
                auto* valueDictionary = varObjectToNSDictionary (valueVar);

                [dictionary setObject: valueDictionary forKey: keyString];
            }
            else if (valueVar.isArray())
            {
                auto* valueArray = varArrayToNSArray (valueVar);

                [dictionary setObject: valueArray forKey: keyString];
            }
            else
            {
                auto* valueString = juceStringToNS (valueVar.toString());

                [dictionary setObject: valueString forKey: keyString];
            }
        }
    }

    return dictionary;
}

inline NSArray* varArrayToNSArray (const var& varToParse)
{
    jassert (varToParse.isArray());

    if (! varToParse.isArray())
        return nil;

    const auto* varArray = varToParse.getArray();

    auto array = [NSMutableArray arrayWithCapacity: (NSUInteger) varArray->size()];

    for (const auto& aVar : *varArray)
    {
        if (aVar.isObject())
        {
            auto* valueDictionary = varObjectToNSDictionary (aVar);

            [array addObject: valueDictionary];
        }
        else if (aVar.isArray())
        {
            auto* valueArray = varArrayToNSArray (aVar);

            [array addObject: valueArray];
        }
        else
        {
            auto* valueString = juceStringToNS (aVar.toString());

            [array addObject: valueString];
        }
    }

    return array;
}

var nsObjectToVar (NSObject* array);

inline var nsDictionaryToVar (NSDictionary* dictionary)
{
    DynamicObject::Ptr dynamicObject (new DynamicObject());

    for (NSString* key in dictionary)
        dynamicObject->setProperty (nsStringToJuce (key), nsObjectToVar ([dictionary objectForKey: key]));

    return var (dynamicObject.get());
}

inline var nsArrayToVar (NSArray* array)
{
    Array<var> resultArray;

    for (id value in array)
        resultArray.add (nsObjectToVar (value));

    return var (resultArray);
}

inline var nsObjectToVar (NSObject* obj)
{
    if ([obj isKindOfClass: [NSString class]])          return nsStringToJuce ((NSString*) obj);
    else if ([obj isKindOfClass: [NSNumber class]])     return nsStringToJuce ([(NSNumber*) obj stringValue]);
    else if ([obj isKindOfClass: [NSDictionary class]]) return nsDictionaryToVar ((NSDictionary*) obj);
    else if ([obj isKindOfClass: [NSArray class]])      return nsArrayToVar ((NSArray*) obj);
    else
    {
        // Unsupported yet, add here!
        jassertfalse;
    }

    return {};
}

#if JUCE_MAC
template <typename RectangleType>
NSRect makeNSRect (const RectangleType& r) noexcept
{
    return NSMakeRect (static_cast<CGFloat> (r.getX()),
                       static_cast<CGFloat> (r.getY()),
                       static_cast<CGFloat> (r.getWidth()),
                       static_cast<CGFloat> (r.getHeight()));
}
#endif

#if JUCE_INTEL
 template <typename T>
 struct NeedsStret
 {
    #if JUCE_32BIT
     static constexpr auto value = sizeof (T) > 8;
    #else
     static constexpr auto value = sizeof (T) > 16;
    #endif
 };

 template <>
 struct NeedsStret<void> { static constexpr auto value = false; };

 template <typename T, bool b = NeedsStret<T>::value>
 struct MetaSuperFn { static constexpr auto value = objc_msgSendSuper_stret; };

 template <typename T>
 struct MetaSuperFn<T, false> { static constexpr auto value = objc_msgSendSuper; };
#else
 template <typename>
 struct MetaSuperFn { static constexpr auto value = objc_msgSendSuper; };
#endif

template <typename SuperType, typename ReturnType, typename... Params>
inline ReturnType ObjCMsgSendSuper (id self, SEL sel, Params... params)
{
    using SuperFn = ReturnType (*) (struct objc_super*, SEL, Params...);
    const auto fn = reinterpret_cast<SuperFn> (MetaSuperFn<ReturnType>::value);

    objc_super s = { self, [SuperType class] };
    return fn (&s, sel, params...);
}

//==============================================================================
struct NSObjectDeleter
{
    void operator() (NSObject* object) const noexcept
    {
        if (object != nullptr)
            [object release];
    }
};

template <typename NSType>
using NSUniquePtr = std::unique_ptr<NSType, NSObjectDeleter>;

/*  This has very similar semantics to NSUniquePtr, with the main difference that it doesn't
    automatically add a pointer to the managed type. This makes it possible to declare
    scoped handles to id or block types.
*/
template <typename T>
class ObjCObjectHandle
{
public:
    ObjCObjectHandle() = default;

    // Note that this does *not* retain the argument.
    explicit ObjCObjectHandle (T ptr) : item (ptr) {}

    ~ObjCObjectHandle() noexcept { reset(); }

    ObjCObjectHandle (const ObjCObjectHandle& other)
        : item (other.item)
    {
        if (item != nullptr)
            [item retain];
    }

    ObjCObjectHandle& operator= (const ObjCObjectHandle& other)
    {
        auto copy = other;
        swap (copy);
        return *this;
    }

    ObjCObjectHandle (ObjCObjectHandle&& other) noexcept { swap (other); }

    ObjCObjectHandle& operator= (ObjCObjectHandle&& other) noexcept
    {
        reset();
        swap (other);
        return *this;
    }

    // Note that this does *not* retain the argument.
    void reset (T ptr) { *this = ObjCObjectHandle { ptr }; }

    T get() const { return item; }

    void reset()
    {
        if (item != nullptr)
            [item release];

        item = {};
    }

    bool operator== (const ObjCObjectHandle& other) const { return item == other.item; }
    bool operator!= (const ObjCObjectHandle& other) const { return ! (*this == other); }

    bool operator== (std::nullptr_t) const { return item == nullptr; }
    bool operator!= (std::nullptr_t) const { return ! (*this == nullptr); }

private:
    void swap (ObjCObjectHandle& other) noexcept { std::swap (other.item, item); }

    T item{};
};

//==============================================================================
namespace detail
{
    constexpr auto makeCompileTimeStr()
    {
        return std::array<char, 1> { { '\0' } };
    }

    template <typename A, size_t... As, typename B, size_t... Bs>
    constexpr auto joinCompileTimeStrImpl (A&& a, std::index_sequence<As...>,
                                           B&& b, std::index_sequence<Bs...>)
    {
        return std::array<char, sizeof... (As) + sizeof... (Bs) + 1> { { a[As]..., b[Bs]..., '\0' } };
    }

    template <size_t A, size_t B>
    constexpr auto joinCompileTimeStr (const char (&a)[A], std::array<char, B> b)
    {
        return joinCompileTimeStrImpl (a, std::make_index_sequence<A - 1>(),
                                       b, std::make_index_sequence<B - 1>());
    }

    template <size_t A, typename... Others>
    constexpr auto makeCompileTimeStr (const char (&v)[A], Others&&... others)
    {
        return joinCompileTimeStr (v, makeCompileTimeStr (others...));
    }

    template <typename Functor, typename Return, typename... Args>
    static constexpr auto toFnPtr (Functor functor, Return (Functor::*) (Args...) const)
    {
        return static_cast<Return (*) (Args...)> (functor);
    }

    template <typename Functor>
    static constexpr auto toFnPtr (Functor functor) { return toFnPtr (functor, &Functor::operator()); }
} // namespace detail

//==============================================================================
template <typename Type>
inline Type getIvar (id self, const char* name)
{
    void* v = nullptr;
    object_getInstanceVariable (self, name, &v);
    return static_cast<Type> (v);
}

template <typename SuperclassType>
struct ObjCClass
{
    ObjCClass (const char* nameRoot)
        : cls (objc_allocateClassPair ([SuperclassType class], getRandomisedName (nameRoot).toUTF8(), 0))
    {
    }

    ~ObjCClass()
    {
        auto kvoSubclassName = String ("NSKVONotifying_") + class_getName (cls);

        if (objc_getClass (kvoSubclassName.toUTF8()) == nullptr)
            objc_disposeClassPair (cls);
    }

    void registerClass()
    {
        objc_registerClassPair (cls);
    }

    SuperclassType* createInstance() const
    {
        return class_createInstance (cls, 0);
    }

    template <typename Type>
    void addIvar (const char* name)
    {
        BOOL b = class_addIvar (cls, name, sizeof (Type), (uint8_t) rint (log2 (sizeof (Type))), @encode (Type));
        jassert (b); ignoreUnused (b);
    }

    template <typename Fn>
    void addMethod (SEL selector, Fn callbackFn) { addMethod (selector, detail::toFnPtr (callbackFn)); }

    template <typename Result, typename... Args>
    void addMethod (SEL selector, Result (*callbackFn) (id, SEL, Args...))
    {
        const auto s = detail::makeCompileTimeStr (@encode (Result), @encode (id), @encode (SEL), @encode (Args)...);
        const auto b = class_addMethod (cls, selector, (IMP) callbackFn, s.data());
        jassertquiet (b);
    }

    void addProtocol (Protocol* protocol)
    {
        BOOL b = class_addProtocol (cls, protocol);
        jassert (b); ignoreUnused (b);
    }

    template <typename ReturnType, typename... Params>
    static ReturnType sendSuperclassMessage (id self, SEL sel, Params... params)
    {
        return ObjCMsgSendSuper<SuperclassType, ReturnType, Params...> (self, sel, params...);
    }

    Class cls;

private:
    static String getRandomisedName (const char* root)
    {
        return root + String::toHexString (juce::Random::getSystemRandom().nextInt64());
    }

    JUCE_DECLARE_NON_COPYABLE (ObjCClass)
};

//==============================================================================
#ifndef DOXYGEN
template <class JuceClass>
struct ObjCLifetimeManagedClass : public ObjCClass<NSObject>
{
    ObjCLifetimeManagedClass()
        : ObjCClass<NSObject> ("ObjCLifetimeManagedClass_")
    {
        addIvar<JuceClass*> ("cppObject");

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
        addMethod (@selector (initWithJuceObject:), initWithJuceObject);
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        addMethod (@selector (dealloc),             dealloc);

        registerClass();
    }

    static id initWithJuceObject (id _self, SEL, JuceClass* obj)
    {
        NSObject* self = sendSuperclassMessage<NSObject*> (_self, @selector (init));
        object_setInstanceVariable (self, "cppObject", obj);

        return self;
    }

    static void dealloc (id _self, SEL)
    {
        if (auto* obj = getIvar<JuceClass*> (_self, "cppObject"))
        {
            delete obj;
            object_setInstanceVariable (_self, "cppObject", nullptr);
        }

        sendSuperclassMessage<void> (_self, @selector (dealloc));
    }

    static ObjCLifetimeManagedClass objCLifetimeManagedClass;
};

template <typename Class>
ObjCLifetimeManagedClass<Class> ObjCLifetimeManagedClass<Class>::objCLifetimeManagedClass;
#endif

// this will return an NSObject which takes ownership of the JUCE instance passed-in
// This is useful to tie the life-time of a juce instance to the life-time of an NSObject
template <typename Class>
NSObject* createNSObjectFromJuceClass (Class* obj)
{
    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wobjc-method-access")
    return [ObjCLifetimeManagedClass<Class>::objCLifetimeManagedClass.createInstance() initWithJuceObject:obj];
    JUCE_END_IGNORE_WARNINGS_GCC_LIKE
}

// Get the JUCE class instance that was tied to the life-time of an NSObject with the
// function above
template <typename Class>
Class* getJuceClassFromNSObject (NSObject* obj)
{
    return obj != nullptr ? getIvar<Class*> (obj, "cppObject") : nullptr;
}

template <typename ReturnT, class Class, typename... Params>
ReturnT (^CreateObjCBlock(Class* object, ReturnT (Class::*fn)(Params...))) (Params...)
{
    __block Class* _this = object;
    __block ReturnT (Class::*_fn)(Params...) = fn;

    return [[^ReturnT (Params... params) { return (_this->*_fn) (params...); } copy] autorelease];
}

template <typename BlockType>
class ObjCBlock
{
public:
    ObjCBlock()  { block = nullptr; }
    template <typename R, class C, typename... P>
    ObjCBlock (C* _this, R (C::*fn)(P...))  : block (CreateObjCBlock (_this, fn)) {}
    ObjCBlock (BlockType b) : block ([b copy]) {}
    ObjCBlock& operator= (const BlockType& other) { if (block != nullptr) { [block release]; } block = [other copy]; return *this; }
    bool operator== (const void* ptr) const  { return ((const void*) block == ptr); }
    bool operator!= (const void* ptr) const  { return ((const void*) block != ptr); }
    ~ObjCBlock() { if (block != nullptr) [block release]; }

    operator BlockType() const { return block; }

private:
    BlockType block;
};

//==============================================================================
class ScopedNotificationCenterObserver
{
public:
    ScopedNotificationCenterObserver() = default;

    ScopedNotificationCenterObserver (id observerIn, SEL selector, NSNotificationName nameIn, id objectIn)
        : observer (observerIn), name (nameIn), object (objectIn)
    {
        [[NSNotificationCenter defaultCenter] addObserver: observer
                                                 selector: selector
                                                     name: name
                                                   object: object];
    }

    ~ScopedNotificationCenterObserver()
    {
        if (observer != nullptr && name != nullptr)
        {
            [[NSNotificationCenter defaultCenter] removeObserver: observer
                                                            name: name
                                                          object: object];
        }
    }

    ScopedNotificationCenterObserver (ScopedNotificationCenterObserver&& other) noexcept
    {
        swap (other);
    }

    ScopedNotificationCenterObserver& operator= (ScopedNotificationCenterObserver&& other) noexcept
    {
        auto moved = std::move (other);
        swap (moved);
        return *this;
    }

    ScopedNotificationCenterObserver (const ScopedNotificationCenterObserver&) = delete;
    ScopedNotificationCenterObserver& operator= (const ScopedNotificationCenterObserver&) = delete;

private:
    void swap (ScopedNotificationCenterObserver& other) noexcept
    {
        std::swap (other.observer, observer);
        std::swap (other.name, name);
        std::swap (other.object, object);
    }

    id observer = nullptr;
    NSNotificationName name = nullptr;
    id object = nullptr;
};

} // namespace juce
