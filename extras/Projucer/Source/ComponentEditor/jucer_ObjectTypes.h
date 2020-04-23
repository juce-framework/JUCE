/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "jucer_JucerDocument.h"
#include "PaintElements/jucer_PaintElement.h"

//==============================================================================
namespace ObjectTypes
{
    //==============================================================================
    // Component types
    extern ComponentTypeHandler* const* const componentTypeHandlers;
    extern const int numComponentTypes;

    //==============================================================================
    // Element types
    extern const char* const* const elementTypeNames;
    extern const int numElementTypes;

    PaintElement* createNewElement (const int index, PaintRoutine* owner);
    PaintElement* createNewImageElement (PaintRoutine* owner);
    PaintElement* createElementForXml (const XmlElement* const e, PaintRoutine* const owner);
}
