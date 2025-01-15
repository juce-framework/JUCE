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
    // This cast helps linters determine nullability
    return (NSString* _Nonnull) [NSString stringWithUTF8String: s.toUTF8()];
}

inline NSString* nsStringLiteral (const char* const s) noexcept
{
    jassert (s != nullptr);

    // This cast helps linters determine nullability
    return (NSString* _Nonnull) [NSString stringWithUTF8String: s];
}

inline NSString* nsEmptyString() noexcept
{
    // This cast helps linters determine nullability
    return (NSString* _Nonnull) [NSString string];
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
    auto array = [[NSMutableArray alloc] initWithCapacity: (NSUInteger) strings.size()];

    for (const auto& string: strings)
        [array addObject: juceStringToNS (string)];

    return [array autorelease];
}

inline NSData* varToJsonData (const var& varToParse)
{
    return [juceStringToNS (JSON::toString (varToParse)) dataUsingEncoding: NSUTF8StringEncoding];
}

inline var jsonDataToVar (NSData* jsonData)
{
    auto* jsonString = [[NSString alloc] initWithData: jsonData
                                             encoding: NSUTF8StringEncoding];

    jassert (jsonString != nullptr);
    return JSON::parse (nsStringToJuce ([jsonString autorelease]));
}

// If for any reason the given var cannot be converted into a valid dictionary
// an empty dictionary will be returned instead
inline NSDictionary* varToNSDictionary (const var& varToParse)
{
    NSError* error { nullptr };
    NSDictionary* dictionary = [NSJSONSerialization JSONObjectWithData: varToJsonData (varToParse)
                                                               options: NSJSONReadingMutableContainers
                                                                 error: &error];

    if (dictionary == nullptr || error != nullptr)
        return @{};

    return dictionary;
}

inline NSData* jsonObjectToData (const NSObject* jsonObject)
{
    NSError* error { nullptr };
    auto* jsonData = [NSJSONSerialization dataWithJSONObject: jsonObject
                                                     options: 0
                                                       error: &error];

    jassert (error == nullptr);
    jassert (jsonData != nullptr);

    return jsonData;
}

inline var nsDictionaryToVar (const NSDictionary* dictionary)
{
    return jsonDataToVar (jsonObjectToData (dictionary));
}

// NSRect is just another name for CGRect, but CGRect is available on iOS *and* macOS.
// Use makeCGRect below.
template <typename RectangleType>
CGRect makeNSRect (const RectangleType& r) noexcept = delete;

template <typename RectangleType>
CGRect makeCGRect (const RectangleType& r) noexcept
{
    return CGRectMake (static_cast<CGFloat> (r.getX()),
                       static_cast<CGFloat> (r.getY()),
                       static_cast<CGFloat> (r.getWidth()),
                       static_cast<CGFloat> (r.getHeight()));
}

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
        // The class could not be created. Is the name already in use?
        jassert (cls != nil);
    }

    virtual ~ObjCClass()
    {
        auto kvoSubclassName = String ("NSKVONotifying_") + class_getName (cls);

        if (objc_getClass (kvoSubclassName.toUTF8()) == nullptr)
            objc_disposeClassPair (cls);
    }

    void registerClass()
    {
        if (cls != nil)
            objc_registerClassPair (cls);
    }

    SuperclassType* createInstance() const
    {
        return class_createInstance (cls, 0);
    }

    template <typename Type>
    void addIvar (const char* name)
    {
        [[maybe_unused]] BOOL b = class_addIvar (cls, name, sizeof (Type), (uint8_t) rint (log2 (sizeof (Type))), @encode (Type));
        jassert (b);
    }

    template <typename Fn>
    void addMethod (SEL selector, Fn callbackFn) { addMethod (selector, toFnPtr (callbackFn)); }

    template <typename Result, typename... Args>
    void addMethod (SEL selector, Result (*callbackFn) (id, SEL, Args...))
    {
        const auto s = detail::makeCompileTimeStr (@encode (Result), @encode (id), @encode (SEL), @encode (Args)...);
        [[maybe_unused]] const auto b = class_addMethod (cls, selector, (IMP) callbackFn, s.data());
        jassert (b);
    }

    void addProtocol (Protocol* protocol)
    {
        [[maybe_unused]] BOOL b = class_addProtocol (cls, protocol);
        jassert (b);
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

namespace detail
{
template <typename> struct Signature;
template <typename R, typename... A> struct Signature<R (A...)> {};

template <typename Class, typename Result, typename... Args>
constexpr auto getSignature (Result (Class::*) (Args...))       { return Signature<Result (Args...)>{}; }

template <typename Class, typename Result, typename... Args>
constexpr auto getSignature (Result (Class::*) (Args...) const) { return Signature<Result (Args...)>{}; }

template <typename Class, typename Fn, typename Result, typename... Params>
auto createObjCBlockImpl (Class* object, Fn func, Signature<Result (Params...)>)
{
    return [[^Result (Params... params) { return (object->*func) (params...); } copy] autorelease];
}
} // namespace detail

/*  Creates an Obj-C block automatically from a member function. */
template <typename Class, typename MemberFunc>
auto CreateObjCBlock (Class* object, MemberFunc fn)
{
    return detail::createObjCBlockImpl (object, fn, detail::getSignature (fn));
}

/*  Automatically copies and releases a block, a bit like a smart pointer for an Obj-C block.

    This is helpful to automatically manage the lifetime of blocks, e.g. if you need to keep a block
    around to be used later. This is the case in the AudioUnit API, where the host may provide a
    musicalContextBlock that can be called by the plugin during rendering. Copying blocks isn't
    realtime-safe, so the plugin must cache the block before rendering.

    If you're just creating blocks to pass them directly to an Obj-C API, you probably won't need to
    use this type.
*/
template <typename BlockType>
class ObjCBlock
{
public:
    ObjCBlock() = default;

    ObjCBlock (BlockType b)
        : block ([b copy]) {}

    ObjCBlock (const ObjCBlock& other)
        : block (other.block != nullptr ? [other.block copy] : nullptr) {}

    ObjCBlock& operator= (const BlockType& other)
    {
        ObjCBlock { other }.swap (*this);
        return *this;
    }

    ~ObjCBlock() noexcept
    {
        if (block != nullptr)
            [block release];
    }

    bool operator== (BlockType ptr) const  { return block == ptr; }
    bool operator!= (BlockType ptr) const  { return block != ptr; }

    operator BlockType() const { return block; }

    void swap (ObjCBlock& other) noexcept
    {
        std::swap (other.block, block);
    }

private:
    BlockType block = nullptr;
};

//==============================================================================
class ScopedNotificationCenterObserver
{
public:
    ScopedNotificationCenterObserver() = default;

    ScopedNotificationCenterObserver (id observerIn,
                                      SEL selector,
                                      NSNotificationName nameIn,
                                      id objectIn,
                                      Class klassIn = [NSNotificationCenter class])
        : observer (observerIn), name (nameIn), object (objectIn), klass (klassIn)
    {
        [[klass defaultCenter] addObserver: observer
                                  selector: selector
                                      name: name
                                    object: object];
    }

    ~ScopedNotificationCenterObserver()
    {
        if (observer != nullptr && name != nullptr)
        {
            [[klass defaultCenter] removeObserver: observer
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
        ScopedNotificationCenterObserver (std::move (other)).swap (*this);
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
        std::swap (other.klass, klass);
    }

    id observer = nullptr;
    NSNotificationName name = nullptr;
    id object = nullptr;
    Class klass = nullptr;
};

#if JUCE_IOS

// Defines a function that will check the requested version both at
// build time, and, if necessary, at runtime.
// The function's first template argument is a trait type containing
// two static member functions named newFn and oldFn.
// When the deployment target is at least equal to major.minor,
// newFn will be selected at compile time.
// When the build sdk does not support iOS SDK major.minor,
// oldFn will be selected at compile time.
// Otherwise, the OS version will be checked at runtime and newFn
// will be called if the OS version is at least equal to major.minor,
// otherwise oldFn will be called.
#define JUCE_DEFINE_IOS_VERSION_CHECKER_FOR_VERSION(major, minor)           \
    template <typename Trait, typename... Args>                             \
    auto ifelse_ ## major ## _ ## minor (Args&&... args)                    \
    {                                                                       \
        constexpr auto fullVersion = major * 10'000 + minor * 100;          \
        if constexpr (fullVersion <= __IPHONE_OS_VERSION_MIN_REQUIRED)      \
            return Trait::newFn (std::forward<Args> (args)...);             \
        else if constexpr (__IPHONE_OS_VERSION_MAX_ALLOWED < fullVersion)   \
            return Trait::oldFn (std::forward<Args> (args)...);             \
        else if (@available (ios major ## . ## minor, *))                   \
            return Trait::newFn (std::forward<Args> (args)...);             \
        else                                                                \
            return Trait::oldFn (std::forward<Args> (args)...);             \
    }

JUCE_DEFINE_IOS_VERSION_CHECKER_FOR_VERSION (14, 0)
JUCE_DEFINE_IOS_VERSION_CHECKER_FOR_VERSION (17, 0)

#endif

} // namespace juce
