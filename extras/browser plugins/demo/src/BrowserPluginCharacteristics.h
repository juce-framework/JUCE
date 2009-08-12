/*
  ==============================================================================

  This file contains values that describe your plugin's behaviour.

  ==============================================================================
*/


//==============================================================================
#define JuceBrowserPlugin_Company           "Raw Material Software Ltd"
#define JuceBrowserPlugin_Name              "Juce Plugin Demo!"
#define JuceBrowserPlugin_Desc              "Juce Browser Plugin Demo!"

//==============================================================================
/** This should be the same version number, in different forms..
*/
#define JuceBrowserPlugin_Version           "0.1"
#define JuceBrowserPlugin_WinVersion        0, 0, 1, 0

//==============================================================================
/** This is the mime-type of the plugin.

    In your HTML, this is the 'type' parameter of the embed tag, e.g.

    <embed id="plugin" type="application/npjucedemo-plugin" width=90% height=500>

    These two macros must be the same string, but the "raw" one shouldn't have quotes around it.
*/
#define JuceBrowserPlugin_MimeType_Raw      application/npjucedemo-plugin
#define JuceBrowserPlugin_MimeType          "application/npjucedemo-plugin"

//==============================================================================
/** Because plugins are associated with a file-type, this is the suffix of the file type the plugin
    can open. If you don't need to use it, just use a made-up name here.
*/
#define JuceBrowserPlugin_FileSuffix        ".jucedemo"

/** If you're building an activeX version, you'll need to create a unique GUID for
    your plugin. Use a tool like uuidgen.exe to create this.
*/
#define JuceBrowserPlugin_ActiveXCLSID      "F683B990-3ADF-11DE-BDFE-F9CB55D89593"
