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

#ifndef __JUCE_WIN32_COMSMARTPTR_JUCEHEADER__
#define __JUCE_WIN32_COMSMARTPTR_JUCEHEADER__


//==============================================================================
/** A simple COM smart pointer.
*/
template <class ComClass>
class ComSmartPtr
{
public:
    ComSmartPtr() throw() : p (0)                               {}
    ComSmartPtr (ComClass* const p_) : p (p_)                   { if (p_ != 0) p_->AddRef(); }
    ComSmartPtr (const ComSmartPtr<ComClass>& p_) : p (p_.p)    { if (p  != 0) p ->AddRef(); }
    ~ComSmartPtr()                                              { release(); }

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
       #if ! JUCE_MINGW
        return ::CoCreateInstance (classUUID, 0, dwClsContext, __uuidof (ComClass), (void**) resetAndGetPointerAddress());
       #else
        jassertfalse; // need to find a mingw equivalent of __uuidof to make this possible
        return E_NOTIMPL;
       #endif
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
       #if ! JUCE_MINGW
        return this->QueryInterface (__uuidof (OtherComClass), destObject);
       #else
        jassertfalse; // need to find a mingw equivalent of __uuidof to make this possible
        return E_NOTIMPL;
       #endif
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
    ComBaseClassHelperBase()  : refCount (1) {}
    virtual ~ComBaseClassHelperBase() {}

    ULONG __stdcall AddRef()    { return ++refCount; }
    ULONG __stdcall Release()   { const ULONG r = --refCount; if (r == 0) delete this; return r; }

    void resetReferenceCount() noexcept     { refCount = 0; }

protected:
    ULONG refCount;
};

/** Handy base class for writing COM objects, providing ref-counting and a basic QueryInterface method.
*/
template <class ComClass>
class ComBaseClassHelper   : public ComBaseClassHelperBase <ComClass>
{
public:
    ComBaseClassHelper() {}
    ~ComBaseClassHelper() {}

    JUCE_COMRESULT QueryInterface (REFIID refId, void** result)
    {
       #if ! JUCE_MINGW
        if (refId == __uuidof (ComClass))   { AddRef(); *result = dynamic_cast <ComClass*> (this); return S_OK; }
       #else
        jassertfalse; // need to find a mingw equivalent of __uuidof to make this possible
       #endif

        if (refId == IID_IUnknown)          { AddRef(); *result = dynamic_cast <IUnknown*> (this); return S_OK; }

        *result = 0;
        return E_NOINTERFACE;
    }
};

#endif   // __JUCE_WIN32_COMSMARTPTR_JUCEHEADER__
