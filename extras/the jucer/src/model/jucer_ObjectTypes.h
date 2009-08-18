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

#ifndef __JUCER_OBJECTTYPES_JUCEHEADER__
#define __JUCER_OBJECTTYPES_JUCEHEADER__

#include "jucer_JucerDocument.h"
#include "paintelements/jucer_PaintElement.h"


//==============================================================================
/**
    A list of the various types of document, component and elements available,
    and functions to create them.

*/
namespace ObjectTypes
{
    //==============================================================================
    /**
        Documents.
    */
    extern const tchar** const documentTypeNames;
    extern const int numDocumentTypes;

    JucerDocument* createNewDocument (const int index);
    JucerDocument* loadDocumentFromFile (const File& file, const bool showErrorMessage);

    //==============================================================================
    /**
        Component types.
    */
    extern ComponentTypeHandler** const componentTypeHandlers;
    extern const int numComponentTypes;

    //==============================================================================
    /**
        Elements.
    */
    extern const tchar** const elementTypeNames;
    extern const int numElementTypes;

    PaintElement* createNewElement (const int index, PaintRoutine* owner);
    PaintElement* createNewImageElement (PaintRoutine* owner);
    PaintElement* createElementForXml (const XmlElement* const e, PaintRoutine* const owner);
}

#endif   // __JUCER_OBJECTTYPES_JUCEHEADER__
