/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

#ifndef JUCE_WIN32_COMSMARTPTR_H_INCLUDED
#define JUCE_WIN32_COMSMARTPTR_H_INCLUDED

#ifndef _MSC_VER
template<typename Type> struct UUIDGetter { static CLSID get() { jassertfalse; return CLSID(); } };
#define __uuidof(x)  UUIDGetter<x>::get()
#endif

inline GUID uuidFromString (const char* const s) noexcept
{
    unsigned long p0;
    unsigned int p1, p2, p3, p4, p5, p6, p7, p8, p9, p10;

  #ifndef _MSC_VER
    sscanf
  #else
    sscanf_s
  #endif
        (s, "%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
              &p0, &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9, &p10);

    GUID g = { p0, (uint16) p1, (uint16) p2, { (uint8) p3, (uint8) p4, (uint8) p5, (uint8) p6,
                                               (uint8) p7, (uint8) p8, (uint8) p9, (uint8) p10 }};
    return g;
}

//==============================================================================
/** A simple COM smart pointer.
*/
template <class ComClass>
class ComSmartPtr
{
public:
    ComSmartPtr() throw() : p (0)                                  {}
    ComSmartPtr (ComClass* const obj) : p (obj)                    { if (p) p->AddRef(); }
    ComSmartPtr (const ComSmartPtr<ComClass>& other) : p (other.p) { if (p) p->AddRef(); }
    ~ComSmartPtr()                                                 { release(); }

    operator ComClass*() const throw()     { return p; }
    ComClass& operator*() const throw()    { return *p; }
    ComClass* operator->() const throw()   { return p; }

    ComSmartPtr& operator= (ComClass* const newP)
    {
        if (newP != 0)  newP->AddRef();
        release();
        p = newP;
        return *this;
    }

    ComSmartPtr& operator= (const ComSmartPtr<ComClass>& newP)  { return operator= (newP.p); }

    // Releases and nullifies this pointer and returns its address
    ComClass** resetAndGetPointerAddress()
    {
        release();
        p = 0;
        return &p;
    }

    HRESULT CoCreateInstance (REFCLSID classUUID, DWORD dwClsContext = CLSCTX_INPROC_SERVER)
    {
        HRESULT hr = ::CoCreateInstance (classUUID, 0, dwClsContext, __uuidof (ComClass), (void**) resetAndGetPointerAddress());
        jassert (hr != CO_E_NOTINITIALIZED); // You haven't called CoInitialize for the current thread!
        return hr;
    }

    template <class OtherComClass>
    HRESULT QueryInterface (REFCLSID classUUID, ComSmartPtr<OtherComClass>& destObject) const
    {
        if (p == 0)
            return E_POINTER;

        return p->QueryInterface (classUUID, (void**) destObject.resetAndGetPointerAddress());
    }

    template <class OtherComClass>
    HRESULT QueryInterface (ComSmartPtr<OtherComClass>& destObject) const
    {
        return this->QueryInterface (__uuidof (OtherComClass), destObject);
    }

private:
    ComClass* p;

    void release()  { if (p != 0) p->Release(); }

    ComClass** operator&() throw(); // private to avoid it being used accidentally
};

//==============================================================================
#define JUCE_COMRESULT  HRESULT __stdcall

//==============================================================================
template <class ComClass>
class ComBaseClassHelperBase   : public ComClass
{
public:
    ComBaseClassHelperBase (unsigned int initialRefCount)  : refCount (initialRefCount) {}
    virtual ~ComBaseClassHelperBase() {}

    ULONG __stdcall AddRef()    { return ++refCount; }
    ULONG __stdcall Release()   { const ULONG r = --refCount; if (r == 0) delete this; return r; }

protected:
    ULONG refCount;

    JUCE_COMRESULT QueryInterface (REFIID refId, void** result)
    {
        if (refId == IID_IUnknown)
            return castToType <IUnknown> (result);

        *result = 0;
        return E_NOINTERFACE;
    }

    template <class Type>
    JUCE_COMRESULT castToType (void** result)
    {
        this->AddRef(); *result = dynamic_cast <Type*> (this); return S_OK;
    }
};

/** Handy base class for writing COM objects, providing ref-counting and a basic QueryInterface method.
*/
template <class ComClass>
class ComBaseClassHelper   : public ComBaseClassHelperBase <ComClass>
{
public:
    ComBaseClassHelper (unsigned int initialRefCount = 1) : ComBaseClassHelperBase <ComClass> (initialRefCount) {}
    ~ComBaseClassHelper() {}

    JUCE_COMRESULT QueryInterface (REFIID refId, void** result)
    {
        if (refId == __uuidof (ComClass))
            return this->template castToType <ComClass> (result);

        return ComBaseClassHelperBase <ComClass>::QueryInterface (refId, result);
    }
};

#endif   // JUCE_WIN32_COMSMARTPTR_H_INCLUDED
