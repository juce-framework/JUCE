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

#ifndef __JUCE_FILEINPUTSOURCE_JUCEHEADER__
#define __JUCE_FILEINPUTSOURCE_JUCEHEADER__

#include "juce_InputSource.h"
#include "../files/juce_File.h"


//==============================================================================
/**
    A type of InputSource that represents a normal file.

    @see InputSource
*/
class JUCE_API  FileInputSource     : public InputSource
{
public:
    //==============================================================================
    FileInputSource (const File& file);
    ~FileInputSource();

    InputStream* createInputStream();
    InputStream* createInputStreamFor (const String& relatedItemPath);
    int64 hashCode() const;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    const File file;

    FileInputSource (const FileInputSource&);
    const FileInputSource& operator= (const FileInputSource&);
};


#endif   // __JUCE_FILEINPUTSOURCE_JUCEHEADER__
