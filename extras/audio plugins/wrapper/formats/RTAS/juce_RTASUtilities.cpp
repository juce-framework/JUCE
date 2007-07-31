/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#if _MSC_VER

// (these functions are in a separate file because of problems including windows.h
// at the same time as the Digi headers)

#include <windows.h>
#include "../../juce_AudioFilterBase.h"
#include "../../juce_AudioFilterEditor.h"


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
