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

#ifndef __JUCE_SYSTEMCLIPBOARD_JUCEHEADER__
#define __JUCE_SYSTEMCLIPBOARD_JUCEHEADER__


//==============================================================================
/**
    Handles reading/writing to the system's clipboard.
*/
class JUCE_API  SystemClipboard
{
public:
    /** Copies a string of text onto the clipboard */
    static void copyTextToClipboard (const String& text);

    /** Gets the current clipboard's contents.

        Obviously this might have come from another app, so could contain
        anything..
    */
    static String getTextFromClipboard();
};

#endif   // __JUCE_SYSTEMCLIPBOARD_JUCEHEADER__
