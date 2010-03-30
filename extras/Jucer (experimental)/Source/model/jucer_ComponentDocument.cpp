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

#include "jucer_ComponentDocument.h"


//==============================================================================
static const char* const componentDocumentTag = "COMPONENT";


//==============================================================================
ComponentDocument::ComponentDocument (Project* project_, const File& cppFile_)
   : project (project_), cppFile (cppFile_), root (componentDocumentTag)
{
}

ComponentDocument::~ComponentDocument()
{
}

bool ComponentDocument::isComponentFile (const File& file)
{
    if (! file.hasFileExtension (".cpp"))
        return false;

    ScopedPointer <InputStream> in (file.createInputStream());

    if (in == 0)
        return false;

    const int amountToRead = 1024;
    HeapBlock <char> initialData;
    initialData.calloc (amountToRead + 4);
    in->read (initialData, amountToRead);

    return String::createStringFromData (initialData, amountToRead)
                  .contains ("JUCER_" "COMPONENT"); // written like this to avoid thinking this file is a component!
}

bool ComponentDocument::save()
{
    return false;
}
