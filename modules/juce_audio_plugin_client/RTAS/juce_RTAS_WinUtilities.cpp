/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include <juce_core/system/juce_TargetPlatform.h>
#include "../utility/juce_CheckSettingMacros.h"

#if JucePlugin_Build_RTAS

// (these functions are in their own file because of problems including windows.h
// at the same time as the Digi headers)

#define _DO_NOT_DECLARE_INTERLOCKED_INTRINSICS_IN_MEMORY // (workaround for a VC build problem)

#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#undef STRICT
#define STRICT
#include <intrin.h>
#include <windows.h>

#pragma pack (push, 8)
#include "../utility/juce_IncludeModuleHeaders.h"
#pragma pack (pop)

//==============================================================================
void JUCE_CALLTYPE attachSubWindow (void* hostWindow,
                                    int& titleW, int& titleH,
                                    Component* comp)
{
    using namespace juce;

    RECT clientRect;
    GetClientRect ((HWND) hostWindow, &clientRect);

    titleW = clientRect.right - clientRect.left;
    titleH = jmax (0, (int) (clientRect.bottom - clientRect.top) - comp->getHeight());
    comp->setTopLeftPosition (0, titleH);

    comp->addToDesktop (0);

    HWND plugWnd = (HWND) comp->getWindowHandle();
    SetParent (plugWnd, (HWND) hostWindow);

    DWORD val = GetWindowLong (plugWnd, GWL_STYLE);
    val = (val & ~WS_POPUP) | WS_CHILD;
    SetWindowLong (plugWnd, GWL_STYLE, val);

    val = GetWindowLong ((HWND) hostWindow, GWL_STYLE);
    SetWindowLong ((HWND) hostWindow, GWL_STYLE, val | WS_CLIPCHILDREN);
}

void JUCE_CALLTYPE resizeHostWindow (void* hostWindow,
                                     int& titleW, int& titleH,
                                     Component* comp)
{
    using namespace juce;

    RECT clientRect, windowRect;
    GetClientRect ((HWND) hostWindow, &clientRect);
    GetWindowRect ((HWND) hostWindow, &windowRect);
    const int borderW = (windowRect.right - windowRect.left) - (clientRect.right - clientRect.left);
    const int borderH = (windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top);

    SetWindowPos ((HWND) hostWindow, 0, 0, 0,
                  borderW + jmax (titleW, comp->getWidth()),
                  borderH + comp->getHeight() + titleH,
                  SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER);
}

extern "C" BOOL WINAPI DllMainRTAS (HINSTANCE, DWORD, LPVOID);

extern "C" BOOL WINAPI DllMain (HINSTANCE instance, DWORD reason, LPVOID reserved)
{
    if (reason == DLL_PROCESS_ATTACH)
        juce::Process::setCurrentModuleInstanceHandle (instance);

    if (GetModuleHandleA ("DAE.DLL") != 0)
        return DllMainRTAS (instance, reason, reserved);

    juce::ignoreUnused (reserved);
    return TRUE;
}

#if ! JucePlugin_EditorRequiresKeyboardFocus

namespace
{
    HWND findMDIParentOf (HWND w)
    {
        const int frameThickness = GetSystemMetrics (SM_CYFIXEDFRAME);

        while (w != 0)
        {
            HWND parent = GetParent (w);

            if (parent == 0)
                break;

            TCHAR windowType [32] = { 0 };
            GetClassName (parent, windowType, 31);

            if (juce::String (windowType).equalsIgnoreCase ("MDIClient"))
            {
                w = parent;
                break;
            }

            RECT windowPos, parentPos;
            GetWindowRect (w, &windowPos);
            GetWindowRect (parent, &parentPos);

            int dw = (parentPos.right - parentPos.left) - (windowPos.right - windowPos.left);
            int dh = (parentPos.bottom - parentPos.top) - (windowPos.bottom - windowPos.top);

            if (dw > 100 || dh > 100)
                break;

            w = parent;

            if (dw == 2 * frameThickness)
                break;
        }

        return w;
    }
}

void JUCE_CALLTYPE passFocusToHostWindow (void* hostWindow)
{
    SetFocus (findMDIParentOf ((HWND) hostWindow));
}

#endif

#endif
