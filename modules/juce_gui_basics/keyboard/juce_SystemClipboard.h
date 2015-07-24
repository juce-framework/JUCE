/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_SYSTEMCLIPBOARD_H_INCLUDED
#define JUCE_SYSTEMCLIPBOARD_H_INCLUDED


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

#endif   // JUCE_SYSTEMCLIPBOARD_H_INCLUDED
