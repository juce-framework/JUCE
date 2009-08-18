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

#if _MSC_VER

// (these functions are in their own file because of problems including windows.h
// at the same time as the Digi headers)

#include <windows.h>

#ifdef _MSC_VER
  #pragma pack (push, 8)
#endif

#include "../juce_PluginHeaders.h"

#ifdef _MSC_VER
  #pragma pack (pop)
#endif

#if JucePlugin_Build_RTAS

//==============================================================================
void JUCE_CALLTYPE attachSubWindow (void* hostWindow,
                                    int& titleW, int& titleH,
                                    Component* comp)
{
    RECT clientRect;
    GetClientRect ((HWND) hostWindow, &clientRect);

    titleW = clientRect.right - clientRect.left;
    titleH = jmax (0, (clientRect.bottom - clientRect.top) - comp->getHeight());
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

#if ! JucePlugin_EditorRequiresKeyboardFocus

static HWND findMDIParentOf (HWND w)
{
    const int frameThickness = GetSystemMetrics (SM_CYFIXEDFRAME);

    while (w != 0)
    {
        HWND parent = GetParent (w);

        if (parent == 0)
            break;

        TCHAR windowType [32];
        zeromem (windowType, sizeof (windowType));
        GetClassName (parent, windowType, 31);

        if (String (windowType).equalsIgnoreCase (T("MDIClient")))
        {
            w = parent;
            break;
        }

        RECT windowPos;
        GetWindowRect (w, &windowPos);

        RECT parentPos;
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

void JUCE_CALLTYPE passFocusToHostWindow (void* hostWindow)
{
    SetFocus (findMDIParentOf ((HWND) hostWindow));
}

#endif
#endif
#endif
