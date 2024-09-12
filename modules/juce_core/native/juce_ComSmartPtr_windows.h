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

#if ! defined (_MSC_VER) && ! defined (__uuidof)
 #ifdef __uuidof
  #undef __uuidof
 #endif

 template <typename Type> struct UUIDGetter { static CLSID get() { jassertfalse; return {}; } };
 #define __uuidof(x)  UUIDGetter<x>::get()

 template <>
 struct UUIDGetter<::IUnknown>
 {
     static CLSID get()     { return { 0, 0, 0, { 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46 } }; }
 };

 #define JUCE_DECLARE_UUID_GETTER(name, uuid) \
    template <> struct UUIDGetter<name> { static CLSID get()  { return uuidFromString (uuid); } };

 #define JUCE_COMCLASS(name, guid) \
    struct name; \
    JUCE_DECLARE_UUID_GETTER (name, guid) \
    struct name

#else
 #define JUCE_DECLARE_UUID_GETTER(name, uuid)
 #define JUCE_COMCLASS(name, guid)       struct DECLSPEC_UUID (guid) name
#endif

#define JUCE_IUNKNOWNCLASS(name, guid)   JUCE_COMCLASS(name, guid) : public IUnknown
#define JUCE_COMRESULT                   HRESULT STDMETHODCALLTYPE
#define JUCE_COMCALL                     virtual HRESULT STDMETHODCALLTYPE

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")

inline GUID uuidFromString (const char* s) noexcept
{
    uint32 ints[4] = {};

    for (uint32 digitIndex = 0; digitIndex < 32;)
    {
        auto c = (uint32) *s++;
        uint32 digit;

        if (c >= '0' && c <= '9')       digit = c - '0';
        else if (c >= 'a' && c <= 'f')  digit = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F')  digit = c - 'A' + 10;
        else if (c == '-')              continue;
        else break;

        ints[digitIndex / 8] |= (digit << 4 * (7 - (digitIndex & 7)));
        ++digitIndex;
    }

    return { ints[0],
             (uint16) (ints[1] >> 16),
             (uint16) ints[1],
             { (uint8) (ints[2] >> 24), (uint8) (ints[2] >> 16), (uint8) (ints[2] >> 8), (uint8) ints[2],
               (uint8) (ints[3] >> 24), (uint8) (ints[3] >> 16), (uint8) (ints[3] >> 8), (uint8) ints[3] }};
}

//==============================================================================
/** A simple COM smart pointer.

    @tags{Core}
*/
template <class ComClass>
class ComSmartPtr
{
public:
    ComSmartPtr() noexcept = default;
    ComSmartPtr (std::nullptr_t) noexcept {}

    template <typename U>
    ComSmartPtr (const ComSmartPtr<U>& other) : ComSmartPtr (other, true) {}
    ComSmartPtr (const ComSmartPtr&    other) : ComSmartPtr (other, true) {}

    ~ComSmartPtr() noexcept { release(); }

    template <typename U>
    ComSmartPtr& operator= (const ComSmartPtr<U>& newP) { ComSmartPtr copy { newP }; std::swap (copy.p, p); return *this; }
    ComSmartPtr& operator= (const ComSmartPtr&    newP) { ComSmartPtr copy { newP }; std::swap (copy.p, p); return *this; }

    operator ComClass*() const noexcept     { return p; }
    ComClass& operator*() const noexcept    { return *p; }
    ComClass* operator->() const noexcept   { return p; }

    // Releases and nullifies this pointer and returns its address
    ComClass** resetAndGetPointerAddress()
    {
        release();
        return &p;
    }

    HRESULT CoCreateInstance (REFCLSID classUUID, DWORD dwClsContext = CLSCTX_INPROC_SERVER)
    {
        auto hr = ::CoCreateInstance (classUUID, nullptr, dwClsContext, __uuidof (ComClass), (void**) resetAndGetPointerAddress());
        jassert (hr != CO_E_NOTINITIALIZED); // You haven't called CoInitialize for the current thread!
        return hr;
    }

    template <class OtherComClass>
    HRESULT QueryInterface (REFCLSID classUUID, ComSmartPtr<OtherComClass>& destObject) const
    {
        if (p == nullptr)
            return E_POINTER;

        return p->QueryInterface (classUUID, (void**) destObject.resetAndGetPointerAddress());
    }

    template <class OtherComClass>
    HRESULT QueryInterface (ComSmartPtr<OtherComClass>& destObject) const
    {
        return this->QueryInterface (__uuidof (OtherComClass), destObject);
    }

    template <class OtherComClass>
    ComSmartPtr<OtherComClass> getInterface() const
    {
        ComSmartPtr<OtherComClass> destObject;

        if (const auto hr = QueryInterface (destObject); FAILED (hr))
            return {};

        return destObject;
    }

    /** Increments refcount. */
    static auto addOwner (ComClass* t)
    {
        return ComSmartPtr (t, true);
    }

    /** Does not initially increment refcount; assumes t has a positive refcount. */
    static auto becomeOwner (ComClass* t)
    {
        return ComSmartPtr (t, false);
    }

private:
    template <typename U>
    friend class ComSmartPtr;

    ComSmartPtr (ComClass* object, bool autoAddRef) noexcept
        : p (object)
    {
        if (p != nullptr && autoAddRef)
            p->AddRef();
    }

    void release()
    {
        if (auto* q = std::exchange (p, nullptr))
            q->Release();
    }

    ComClass** operator&() noexcept; // private to avoid it being used accidentally

    ComClass* p = nullptr;
};

/** Increments refcount. */
template <class ObjectType>
auto addComSmartPtrOwner (ObjectType* t)
{
    return ComSmartPtr<ObjectType>::addOwner (t);
}

/** Does not initially increment refcount; assumes t has a positive refcount. */
template <class ObjectType>
auto becomeComSmartPtrOwner (ObjectType* t)
{
    return ComSmartPtr<ObjectType>::becomeOwner (t);
}

//==============================================================================
template <class First, class... ComClasses>
class ComBaseClassHelperBase   : public First, public ComClasses...
{
public:
    ComBaseClassHelperBase() = default;
    virtual ~ComBaseClassHelperBase() = default;

    ULONG STDMETHODCALLTYPE AddRef()    override { return ++refCount; }
    ULONG STDMETHODCALLTYPE Release()   override { auto r = --refCount; if (r == 0) delete this; return r; }

protected:
    ULONG refCount = 1;

    JUCE_COMRESULT QueryInterface (REFIID refId, void** result) override
    {
        if (refId == __uuidof (IUnknown))
            return castToType<First> (result);

        *result = nullptr;
        return E_NOINTERFACE;
    }

    template <class Type>
    JUCE_COMRESULT castToType (void** result)
    {
        this->AddRef();
        *result = dynamic_cast<Type*> (this);

        return S_OK;
    }
};

/** Handy base class for writing COM objects, providing ref-counting and a basic QueryInterface method.

    @tags{Core}
*/
template <class... ComClasses>
class ComBaseClassHelper   : public ComBaseClassHelperBase<ComClasses...>
{
public:
    ComBaseClassHelper() = default;

    JUCE_COMRESULT QueryInterface (REFIID refId, void** result) override
    {
        const std::tuple<IID, void*> bases[]
        {
            std::make_tuple (__uuidof (ComClasses),
                             static_cast<void*> (static_cast<ComClasses*> (this)))...
        };

        for (const auto& base : bases)
        {
            if (refId == std::get<0> (base))
            {
                this->AddRef();
                *result = std::get<1> (base);
                return S_OK;
            }
        }

        return ComBaseClassHelperBase<ComClasses...>::QueryInterface (refId, result);
    }
};

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

} // namespace juce
