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

#ifndef __JUCE_BROWSER_PLUGIN_JUCEHEADER__
#define __JUCE_BROWSER_PLUGIN_JUCEHEADER__

#include "../juce_gui_basics/juce_gui_basics.h"


//=============================================================================
/** Config: JUCE_BUILD_NPAPI
    Use this flag to enable or disable the building of an NPAPI verstion of your plugin.
*/
#ifndef JUCE_BUILD_NPAPI
 #define JUCE_BUILD_NPAPI 1
#endif

/** Config: JUCE_BUILD_ACTIVEX
    Use this flag to enable or disable the building of an ActiveX verstion of your plugin.
*/
#if JUCE_WINDOWS && ! defined (JUCE_BUILD_ACTIVEX)
 #define JUCE_BUILD_ACTIVEX 1
#endif

//=============================================================================
/** JuceBrowserPlugin_Company
    This should be a quoted string containing the name of your company.
*/
#ifndef JuceBrowserPlugin_Company
 #error "You must define the JuceBrowserPlugin_Company macro before including juce_browser_plugin.h!"
#endif

/** JuceBrowserPlugin_Name
    This should be a quoted string containing the name of your plugin.
*/
#ifndef JuceBrowserPlugin_Name
 #error "You must define the JuceBrowserPlugin_Name macro before including juce_browser_plugin.h!"
#endif

/** JuceBrowserPlugin_Desc
    This should be a quoted string containing a description of your plugin.
*/
#ifndef JuceBrowserPlugin_Desc
 #error "You must define the JuceBrowserPlugin_Desc macro before including juce_browser_plugin.h!"
#endif


//==============================================================================
/** JuceBrowserPlugin_Desc
    This should be a quoted string containing a version number for your plugin, e.g. "1.0.0"
    The same version number must also be provided in the JuceBrowserPlugin_WinVersion setting.
*/
#ifndef JuceBrowserPlugin_Version
 #error "You must define the JuceBrowserPlugin_Version macro before including juce_browser_plugin.h!"
#endif

/** JuceBrowserPlugin_Desc
    This should be a comma-separated windows resource version number for your plugin, e.g. 0, 1, 0, 0.
    It must be the same number as the JuceBrowserPlugin_Version setting.
*/
#ifndef JuceBrowserPlugin_WinVersion
 #error "You must define the JuceBrowserPlugin_WinVersion macro before including juce_browser_plugin.h!"
#endif

//==============================================================================
/** JuceBrowserPlugin_MimeType_Raw
    In your HTML, this is the 'type' parameter of the embed tag, e.g.
    <embed id="plugin" type="application/npjucedemo-plugin" width=90% height=500>

    Both the JuceBrowserPlugin_MimeType_Raw and JuceBrowserPlugin_MimeType must be the same string,
    but JuceBrowserPlugin_MimeType_Raw must have no quotes, whereas JuceBrowserPlugin_MimeType must be quoted.
*/
#ifndef JuceBrowserPlugin_MimeType_Raw
 #error "You must define the JuceBrowserPlugin_MimeType_Raw macro before including juce_browser_plugin.h!"
#endif

/** JuceBrowserPlugin_MimeType
    In your HTML, this is the 'type' parameter of the embed tag, e.g.
    <embed id="plugin" type="application/npjucedemo-plugin" width=90% height=500>

    Both the JuceBrowserPlugin_MimeType_Raw and JuceBrowserPlugin_MimeType must be the same string,
    but JuceBrowserPlugin_MimeType_Raw must have no quotes, whereas JuceBrowserPlugin_MimeType must be quoted.
*/
#ifndef JuceBrowserPlugin_MimeType
 #error "You must define the JuceBrowserPlugin_MimeType macro before including juce_browser_plugin.h!"
#endif

//==============================================================================
/** JuceBrowserPlugin_FileSuffix
    Because plugins are associated with a file-type, this is the suffix of the file type the plugin
    can open. If you don't need to use it, just use a made-up name here, e.g. ".jucedemo". The string
    must be quoted.
*/
#ifndef JuceBrowserPlugin_FileSuffix
 #error "You must define the JuceBrowserPlugin_FileSuffix macro before including juce_browser_plugin.h!"
#endif

/** JuceBrowserPlugin_ActiveXCLSID
    If you're building an ActiveX plugin, you'll need to create a unique GUID for
    your plugin. Use a tool like uuidgen.exe to create this. The guid must be defined here as a quoted
    string, e.g. "F683B990-3ADF-11DE-BDFE-F9CB55D89593".
*/
#if JUCE_BUILD_ACTIVEX && ! defined (JuceBrowserPlugin_ActiveXCLSID)
 #error "You must define the JuceBrowserPlugin_ActiveXCLSID macro before including juce_browser_plugin.h!"
#endif


//=============================================================================
namespace juce
{
    #include "wrapper/juce_BrowserPluginComponent.h"
}

//==============================================================================
/**
    This function must be implemented somewhere in your code to create the actual
    plugin object that you want to use.

    Obviously multiple instances may be used simultaneously, so be VERY cautious
    in your use of static variables!
*/
juce::BrowserPluginComponent* JUCE_CALLTYPE createBrowserPlugin();


#endif   // __JUCE_BROWSER_PLUGIN_JUCEHEADER__
