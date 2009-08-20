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

#ifndef __JUCE_WIN32_DYNAMICLIBRARYLOADER_JUCEHEADER__
#define __JUCE_WIN32_DYNAMICLIBRARYLOADER_JUCEHEADER__

#ifndef DOXYGEN

//==============================================================================
// use with DynamicLibraryLoader to simplify importing functions
//
// functionName: function to import
// localFunctionName: name you want to use to actually call it (must be different)
// returnType: the return type
// object: the DynamicLibraryLoader to use
// params: list of params (bracketed)
//
#define DynamicLibraryImport(functionName, localFunctionName, returnType, object, params) \
    typedef returnType (WINAPI *type##localFunctionName) params; \
    type##localFunctionName localFunctionName  \
        = (type##localFunctionName)object.findProcAddress (#functionName);


//==============================================================================
// loads and unloads a DLL automatically
class JUCE_API  DynamicLibraryLoader
{
public:
    DynamicLibraryLoader (const String& name);
    ~DynamicLibraryLoader();

    void* findProcAddress (const String& functionName);

private:
    void* libHandle;
};


#endif
#endif   // __JUCE_WIN32_DYNAMICLIBRARYLOADER_JUCEHEADER__
