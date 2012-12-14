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
    /** Creates a FileInputSource for a file.
        If the useFileTimeInHashGeneration parameter is true, then this object's
        hashCode() method will incorporate the file time into its hash code; if
        false, only the file name will be used for the hash.
    */
    FileInputSource (const File& file, bool useFileTimeInHashGeneration = false);

    /** Destructor. */
    ~FileInputSource();

    InputStream* createInputStream();
    InputStream* createInputStreamFor (const String& relatedItemPath);
    int64 hashCode() const;

private:
    //==============================================================================
    const File file;
    bool useFileTimeInHashGeneration;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileInputSource)
};


#endif   // __JUCE_FILEINPUTSOURCE_JUCEHEADER__
