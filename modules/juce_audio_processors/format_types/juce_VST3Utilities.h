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

#pragma once

/** @cond */
namespace juce
{

JUCE_BEGIN_NO_SANITIZE ("vptr")

//==============================================================================
#define JUCE_DECLARE_VST3_COM_REF_METHODS \
    Steinberg::uint32 PLUGIN_API addRef() override   { return (Steinberg::uint32) ++refCount; } \
    Steinberg::uint32 PLUGIN_API release() override  { const int r = --refCount; if (r == 0) delete this; return (Steinberg::uint32) r; }

#define JUCE_DECLARE_VST3_COM_QUERY_METHODS \
    Steinberg::tresult PLUGIN_API queryInterface (const Steinberg::TUID, void** obj) override \
    { \
        jassertfalse; \
        *obj = nullptr; \
        return Steinberg::kNotImplemented; \
    }

inline bool doUIDsMatch (const Steinberg::TUID a, const Steinberg::TUID b) noexcept
{
    return std::memcmp (a, b, sizeof (Steinberg::TUID)) == 0;
}

/*  Holds a tresult and a pointer to an object.

    Useful for holding intermediate results of calls to queryInterface.
*/
class QueryInterfaceResult
{
public:
    QueryInterfaceResult() = default;

    QueryInterfaceResult (Steinberg::tresult resultIn, void* ptrIn)
        : result (resultIn), ptr (ptrIn) {}

    bool isOk() const noexcept   { return result == Steinberg::kResultOk; }

    Steinberg::tresult extract (void** obj) const
    {
        *obj = result == Steinberg::kResultOk ? ptr : nullptr;
        return result;
    }

private:
    Steinberg::tresult result = Steinberg::kResultFalse;
    void* ptr = nullptr;
};

/*  Holds a tresult and a pointer to an object.

    Calling InterfaceResultWithDeferredAddRef::extract() will also call addRef
    on the pointed-to object. It is expected that users will use
    InterfaceResultWithDeferredAddRef to hold intermediate results of a queryInterface
    call. When a suitable interface is found, the function can be exited with
    `return suitableInterface.extract (obj)`, which will set the obj pointer,
    add a reference to the interface, and return the appropriate result code.
*/
class InterfaceResultWithDeferredAddRef
{
public:
    InterfaceResultWithDeferredAddRef() = default;

    template <typename Ptr>
    InterfaceResultWithDeferredAddRef (Steinberg::tresult resultIn, Ptr* ptrIn)
        : result (resultIn, ptrIn),
          addRefFn (doAddRef<Ptr>) {}

    bool isOk() const noexcept   { return result.isOk(); }

    Steinberg::tresult extract (void** obj) const
    {
        const auto toReturn = result.extract (obj);

        if (result.isOk() && *obj != nullptr && addRefFn != nullptr)
            addRefFn (*obj);

        return toReturn;
    }

private:
    template <typename Ptr>
    static void doAddRef (void* obj)   { static_cast<Ptr*> (obj)->addRef(); }

    QueryInterfaceResult result;
    void (*addRefFn) (void*) = nullptr;
};

template <typename ClassType>                                   struct UniqueBase {};
template <typename CommonClassType, typename SourceClassType>   struct SharedBase {};

template <typename ToTest, typename CommonClassType, typename SourceClassType>
InterfaceResultWithDeferredAddRef testFor (ToTest& toTest,
                                           const Steinberg::TUID targetIID,
                                           SharedBase<CommonClassType, SourceClassType>)
{
    if (! doUIDsMatch (targetIID, CommonClassType::iid))
        return {};

    return { Steinberg::kResultOk, static_cast<CommonClassType*> (static_cast<SourceClassType*> (std::addressof (toTest))) };
}

template <typename ToTest, typename ClassType>
InterfaceResultWithDeferredAddRef testFor (ToTest& toTest,
                                           const Steinberg::TUID targetIID,
                                           UniqueBase<ClassType>)
{
    return testFor (toTest, targetIID, SharedBase<ClassType, ClassType>{});
}

template <typename ToTest>
InterfaceResultWithDeferredAddRef testForMultiple (ToTest&, const Steinberg::TUID) { return {}; }

template <typename ToTest, typename Head, typename... Tail>
InterfaceResultWithDeferredAddRef testForMultiple (ToTest& toTest, const Steinberg::TUID targetIID, Head head, Tail... tail)
{
    const auto result = testFor (toTest, targetIID, head);

    if (result.isOk())
        return result;

    return testForMultiple (toTest, targetIID, tail...);
}

//==============================================================================
// We have to trust that Steinberg won't double-delete
// NOLINTBEGIN(clang-analyzer-cplusplus.NewDelete)
template <class ObjectType>
class VSTComSmartPtr
{
public:
    VSTComSmartPtr() = default;
    VSTComSmartPtr (const VSTComSmartPtr& other) noexcept : source (other.source)            { if (source != nullptr) source->addRef(); }
    ~VSTComSmartPtr()                                                                        { if (source != nullptr) source->release(); }

    explicit operator bool() const noexcept           { return operator!= (nullptr); }
    ObjectType* get() const noexcept                  { return source; }
    ObjectType& operator*() const noexcept            { return *source; }
    ObjectType* operator->() const noexcept           { return source; }

    VSTComSmartPtr& operator= (const VSTComSmartPtr& other)
    {
        auto p = other;
        std::swap (p.source, source);
        return *this;
    }

    VSTComSmartPtr& operator= (std::nullptr_t)
    {
        return operator= (VSTComSmartPtr{});
    }

    bool operator== (std::nullptr_t) const noexcept { return source == nullptr; }
    bool operator!= (std::nullptr_t) const noexcept { return source != nullptr; }

    bool loadFrom (Steinberg::FUnknown* o)
    {
        *this = nullptr;
        return o != nullptr && o->queryInterface (ObjectType::iid, (void**) &source) == Steinberg::kResultOk;
    }

    bool loadFrom (Steinberg::IPluginFactory* factory, const Steinberg::TUID& uuid)
    {
        jassert (factory != nullptr);
        *this = nullptr;
        return factory->createInstance (uuid, ObjectType::iid, (void**) &source) == Steinberg::kResultOk;
    }

    /** Increments refcount. */
    static auto addOwner (ObjectType* t)
    {
        return VSTComSmartPtr (t, true);
    }

    /** Does not initially increment refcount; assumes t has a positive refcount. */
    static auto becomeOwner (ObjectType* t)
    {
        return VSTComSmartPtr (t, false);
    }

private:
    explicit VSTComSmartPtr (ObjectType* object, bool autoAddRef) noexcept  : source (object)  { if (source != nullptr && autoAddRef) source->addRef(); }
    ObjectType* source = nullptr;
};

/** Increments refcount. */
template <class ObjectType>
auto addVSTComSmartPtrOwner (ObjectType* t)
{
    return VSTComSmartPtr<ObjectType>::addOwner (t);
}

/** Does not initially increment refcount; assumes t has a positive refcount. */
template <class ObjectType>
auto becomeVSTComSmartPtrOwner (ObjectType* t)
{
    return VSTComSmartPtr<ObjectType>::becomeOwner (t);
}

// NOLINTEND(clang-analyzer-cplusplus.NewDelete)

JUCE_END_NO_SANITIZE

} // namespace juce
/** @endcond */
