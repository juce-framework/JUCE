/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifdef _MSC_VER
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

#if ! JUCE_MINGW
 #include <crtdbg.h>
 #include <comutil.h>
#endif

#if JUCE_OPENGL
 #include <gl/gl.h>
#endif

#undef PACKED

//==============================================================================
#if JUCE_ASIO
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

    2) Rebuild the whole of JUCE, setting the global definition JUCE_ASIO (you
       can un-comment the "#define JUCE_ASIO" line in juce_Config.h
       if you prefer). Make sure that your header search path will find the
       iasiodrv.h file that comes with the SDK. (Only about 2-3 of the SDK header
       files are actually needed - so to simplify things, you could just copy
       these into your JUCE directory).

    If you're compiling and you get an error here because you don't have the
    ASIO SDK installed, you can disable ASIO support by commenting-out the
    "#define JUCE_ASIO" line in juce_Config.h, and rebuild your Juce library.
 */
 #include "iasiodrv.h"
#endif

//==============================================================================
#if JUCE_USE_CDBURNER

 /* You'll need the Platform SDK for these headers - if you don't have it and don't
    need to use CD-burning, then you might just want to disable the JUCE_USE_CDBURNER
    flag in juce_Config.h to avoid these includes.
 */
 #include <imapi.h>
 #include <imapierror.h>
#endif

//==============================================================================
#if JUCE_USE_CAMERA

 /*  If you're using the camera classes, you'll need access to a few DirectShow headers.

     Both of these files are provided in the normal Windows SDK, but some Microsoft plonker
     didn't realise that qedit.h doesn't actually compile without the rest of the DirectShow SDK!
     Microsoft's suggested fix for this is to hack their qedit.h file! See:
     http://social.msdn.microsoft.com/Forums/en-US/windowssdk/thread/ed097d2c-3d68-4f48-8448-277eaaf68252
     .. which is a pathetic bodge, but a lot less hassle than installing the full DShow SDK.
 */
 #include <dshow.h>
 #include <qedit.h>
#endif

//==============================================================================
#if JUCE_WASAPI
 #include <MMReg.h>
 #include <mmdeviceapi.h>
 #include <Audioclient.h>
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
#ifdef _MSC_VER
 #pragma warning (pop)
#endif

//==============================================================================
/** A simple COM smart pointer.
    Avoids having to include ATL just to get one of these.
*/
template <class T>
class ComSmartPtr
{
public:
    ComSmartPtr() throw() : p (0)                       {}
    ComSmartPtr (T* const p_) : p (p_)                  { if (p_ != 0) p_->AddRef(); }
    ComSmartPtr (const ComSmartPtr<T>& p_) : p (p_.p)   { if (p != 0) p->AddRef(); }
    ~ComSmartPtr()                                      { if (p != 0) p->Release(); }

    operator T*() const throw()     { return p; }
    T& operator*() const throw()    { return *p; }
    T** operator&() throw()         { return &p; }
    T* operator->() const throw()   { return p; }

    T* operator= (T* const newP)
    {
        if (newP != 0)
            newP->AddRef();

        if (p != 0)
            p->Release();

        p = newP;
        return newP;
    }

    T* operator= (const ComSmartPtr<T>& newP)  { return operator= (newP.p); }

    HRESULT CoCreateInstance (REFCLSID rclsid, DWORD dwClsContext)
    {
#ifndef __MINGW32__
        operator= (0);
        return ::CoCreateInstance (rclsid, 0, dwClsContext, __uuidof(T), (void**) &p);
#else
        return S_FALSE;
#endif
    }

    T* p;
};


#endif   // __JUCE_WIN32_NATIVEINCLUDES_JUCEHEADER__
