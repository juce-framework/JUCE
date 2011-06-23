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

//==============================================================================
#ifndef __JUCE_WIN32_NATIVEINCLUDES_JUCEHEADER__
#define __JUCE_WIN32_NATIVEINCLUDES_JUCEHEADER__


#include "../../core/juce_TargetPlatform.h"
#include "../../../juce_Config.h"

#ifndef STRICT
 #define STRICT 1
#endif

#undef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1

#if JUCE_MSVC
 #ifndef _CPPRTTI
  #error "You're compiling without RTTI enabled! This is needed for a lot of JUCE classes, please update your compiler settings!"
 #endif

 #ifndef _CPPUNWIND
  #error "You're compiling without exceptions enabled! This is needed for a lot of JUCE classes, please update your compiler settings!"
 #endif

 #pragma warning (push)
 #pragma warning (disable : 4100 4201 4514 4312 4995)
#endif

#define _WIN32_WINNT 0x0500
#define _UNICODE 1
#define UNICODE 1

#ifndef _WIN32_IE
 #define _WIN32_IE 0x0400
#endif

#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <shellapi.h>
#include <mmsystem.h>
#include <vfw.h>
#include <tchar.h>
#include <stddef.h>
#include <ctime>
#include <wininet.h>
#include <nb30.h>
#include <iphlpapi.h>
#include <mapi.h>
#include <float.h>
#include <process.h>
#include <Exdisp.h>
#include <exdispid.h>
#include <shlobj.h>
#include <shlwapi.h>

#if ! JUCE_MINGW
 #include <crtdbg.h>
 #include <comutil.h>
#endif

#if JUCE_OPENGL
 #include <gl/gl.h>
#endif

#undef PACKED

//==============================================================================
#if JUCE_ASIO && JUCE_BUILD_NATIVE
 /*
    This is very frustrating - we only need to use a handful of definitions from
    a couple of the header files in Steinberg's ASIO SDK, and it'd be easy to copy
    about 30 lines of code into this cpp file to create a fully stand-alone ASIO
    implementation...

    ..unfortunately that would break Steinberg's license agreement for use of
    their SDK, so I'm not allowed to do this.

    This means that anyone who wants to use JUCE's ASIO abilities will have to:

    1) Agree to Steinberg's licensing terms and download the ASIO SDK
        (see www.steinberg.net/Steinberg/Developers.asp).

    2) Enable this code with a global definition #define JUCE_ASIO 1.

    3) Make sure that your header search path contains the iasiodrv.h file that
       comes with the SDK. (Only about a handful of the SDK header files are actually
       needed - so to simplify things, you could just copy these into your JUCE directory).
 */
 #include <iasiodrv.h>
#endif

//==============================================================================
#if JUCE_USE_CDBURNER && JUCE_BUILD_NATIVE

 /* You'll need the Platform SDK for these headers - if you don't have it and don't
    need to use CD-burning, then you might just want to disable the JUCE_USE_CDBURNER
    flag in juce_Config.h to avoid these includes.
 */
 #include <imapi.h>
 #include <imapierror.h>
#endif

//==============================================================================
#if (JUCE_USE_CAMERA || JUCE_DIRECTSHOW) && JUCE_BUILD_NATIVE

 /*  If you're using the camera classes, you'll need access to a few DirectShow headers.

     These files are provided in the normal Windows SDK, but some Microsoft plonker
     didn't realise that qedit.h doesn't actually compile without the rest of the DirectShow SDK..
     Microsoft's suggested fix for this is to hack their qedit.h file! See:
     http://social.msdn.microsoft.com/Forums/en-US/windowssdk/thread/ed097d2c-3d68-4f48-8448-277eaaf68252
     .. which is a bit of a bodge, but a lot less hassle than installing the full DShow SDK.

     An alternative workaround is to create a dummy dxtrans.h file and put it in your include path.
     The dummy file just needs to contain the following content:
        #define __IDxtCompositor_INTERFACE_DEFINED__
        #define __IDxtAlphaSetter_INTERFACE_DEFINED__
        #define __IDxtJpeg_INTERFACE_DEFINED__
        #define __IDxtKey_INTERFACE_DEFINED__
    ..and that should be enough to convince qedit.h that you have the SDK!
 */
 #include <dshow.h>
 #include <qedit.h>
 #include <dshowasf.h>
#endif

#if JUCE_DIRECTSHOW && JUCE_MEDIAFOUNDATION && JUCE_BUILD_NATIVE
 #include <evr.h>
#endif

//==============================================================================
#if JUCE_WASAPI && JUCE_BUILD_NATIVE
 #include <MMReg.h>
 #include <mmdeviceapi.h>
 #include <Audioclient.h>
 #include <Audiopolicy.h>
 #include <Avrt.h>
 #include <functiondiscoverykeys.h>
#endif

//==============================================================================
#if JUCE_QUICKTIME

 /* If you've got an include error here, you probably need to install the QuickTime SDK and
    add its header directory to your include path.

    Alternatively, if you don't need any QuickTime services, just turn off the JUCE_QUICKTIME
    flag in juce_Config.h
 */
 #include <Movies.h>
 #include <QTML.h>
 #include <QuickTimeComponents.h>
 #include <MediaHandlers.h>
 #include <ImageCodec.h>

 /* If you've got QuickTime 7 installed, then these COM objects should be found in
    the "\Program Files\Quicktime" directory. You'll need to add this directory to
    your include search path to make these import statements work.
 */
 #import <QTOLibrary.dll>
 #import <QTOControl.dll>
#endif

//==============================================================================
#if JUCE_MSVC
 #pragma warning (pop)
#endif

#if JUCE_DIRECT2D && JUCE_BUILD_NATIVE
 #include <d2d1.h>
 #include <dwrite.h>
#endif

#ifndef WM_APPCOMMAND
 #define WM_APPCOMMAND 0x0319
#endif

//==============================================================================
/** A simple COM smart pointer.
    Avoids having to include ATL just to get one of these.
*/
template <class ComClass>
class ComSmartPtr
{
public:
    ComSmartPtr() throw() : p (0)                               {}
    ComSmartPtr (ComClass* const p_) : p (p_)                   { if (p_ != 0) p_->AddRef(); }
    ComSmartPtr (const ComSmartPtr<ComClass>& p_) : p (p_.p)    { if (p != 0) p->AddRef(); }
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
      #ifndef __MINGW32__
        return ::CoCreateInstance (classUUID, 0, dwClsContext, __uuidof (ComClass), (void**) resetAndGetPointerAddress());
      #else
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
        return this->QueryInterface (__uuidof (OtherComClass), destObject);
    }

private:
    ComClass* p;

    void release()  { if (p != 0) p->Release(); }

    ComClass** operator&() throw(); // private to avoid it being used accidentally
};

//==============================================================================
/** Handy base class for writing COM objects, providing ref-counting and a basic QueryInterface method.
*/
template <class ComClass>
class ComBaseClassHelper   : public ComClass
{
public:
    ComBaseClassHelper()  : refCount (1) {}
    virtual ~ComBaseClassHelper() {}

    HRESULT __stdcall QueryInterface (REFIID refId, void** result)
    {
      #ifndef __MINGW32__
        if (refId == __uuidof (ComClass))   { AddRef(); *result = dynamic_cast <ComClass*> (this); return S_OK; }
      #endif

        if (refId == IID_IUnknown)          { AddRef(); *result = dynamic_cast <IUnknown*> (this); return S_OK; }

        *result = 0;
        return E_NOINTERFACE;
    }

    ULONG __stdcall AddRef()    { return ++refCount; }
    ULONG __stdcall Release()   { const int r = --refCount; if (r == 0) delete this; return r; }

protected:
    int refCount;
};

#endif   // __JUCE_WIN32_NATIVEINCLUDES_JUCEHEADER__
