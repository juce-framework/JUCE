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

// (This file gets included by juce_win32_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE


//==============================================================================
void SystemClipboard::copyTextToClipboard (const String& text) throw()
{
    if (OpenClipboard (0) != 0)
    {
        if (EmptyClipboard() != 0)
        {
            const int len = text.length();

            if (len > 0)
            {
                HGLOBAL bufH = GlobalAlloc (GMEM_MOVEABLE | GMEM_DDESHARE,
                                            (len + 1) * sizeof (wchar_t));

                if (bufH != 0)
                {
                    wchar_t* const data = (wchar_t*) GlobalLock (bufH);
                    text.copyToBuffer (data, len);
                    GlobalUnlock (bufH);

                    SetClipboardData (CF_UNICODETEXT, bufH);
                }
            }
        }

        CloseClipboard();
    }
}

const String SystemClipboard::getTextFromClipboard() throw()
{
    String result;

    if (OpenClipboard (0) != 0)
    {
        HANDLE bufH = GetClipboardData (CF_UNICODETEXT);

        if (bufH != 0)
        {
            const wchar_t* const data = (const wchar_t*) GlobalLock (bufH);

            if (data != 0)
            {
                result = String (data, (int) (GlobalSize (bufH) / sizeof (tchar)));

                GlobalUnlock (bufH);
            }
        }

        CloseClipboard();
    }

    return result;
}


#endif
