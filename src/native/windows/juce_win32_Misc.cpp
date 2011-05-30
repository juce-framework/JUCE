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

// (This file gets included by juce_win32_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE


//==============================================================================
void SystemClipboard::copyTextToClipboard (const String& text)
{
    if (OpenClipboard (0) != 0)
    {
        if (EmptyClipboard() != 0)
        {
            const int bytesNeeded = CharPointer_UTF16::getBytesRequiredFor (text.getCharPointer()) + 4;

            if (bytesNeeded > 0)
            {
                HGLOBAL bufH = GlobalAlloc (GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT, bytesNeeded + sizeof (WCHAR));

                if (bufH != 0)
                {
                    WCHAR* const data = static_cast <WCHAR*> (GlobalLock (bufH));
                    text.copyToUTF16 (data, bytesNeeded);
                    GlobalUnlock (bufH);

                    SetClipboardData (CF_UNICODETEXT, bufH);
                }
            }
        }

        CloseClipboard();
    }
}

String SystemClipboard::getTextFromClipboard()
{
    String result;

    if (OpenClipboard (0) != 0)
    {
        HANDLE bufH = GetClipboardData (CF_UNICODETEXT);

        if (bufH != 0)
        {
            const WCHAR* const data = (const WCHAR*) GlobalLock (bufH);

            if (data != nullptr)
            {
                result = String (data, (int) (GlobalSize (bufH) / sizeof (WCHAR)));

                GlobalUnlock (bufH);
            }
        }

        CloseClipboard();
    }

    return result;
}


#endif
