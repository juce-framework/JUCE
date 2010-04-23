/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#ifndef __JUCE_INCLUDEBROWSERPLUGININFO_JUCEHEADER__
#define __JUCE_INCLUDEBROWSERPLUGININFO_JUCEHEADER__

#include "BrowserPluginCharacteristics.h"


//==============================================================================
// The following macros just cause a compile error if you've forgotten to
// define all your plugin settings properly.

#if ! (defined (JuceBrowserPlugin_Name) \
        && defined (JuceBrowserPlugin_Desc) \
        && defined (JuceBrowserPlugin_Company) \
        && defined (JuceBrowserPlugin_MimeType) \
        && defined (JuceBrowserPlugin_MimeType_Raw) \
        && defined (JuceBrowserPlugin_FileSuffix) \
        && defined (JuceBrowserPlugin_Version) \
        && defined (JuceBrowserPlugin_WinVersion))
 #error "You haven't defined all the necessary JuceBrowserPlugin_xx values in your BrowserPluginCharacteristics.h file!"
#endif

#endif   // __JUCE_INCLUDEBROWSERPLUGININFO_JUCEHEADER__
