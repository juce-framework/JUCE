/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../Application/jucer_Headers.h"

#include "jucer_JucerDocument.h"
#include "Components/jucer_ComponentUndoableAction.h"
#include "Properties/jucer_JustificationProperty.h"
#include "Properties/jucer_FontPropertyComponent.h"
#include "Properties/jucer_ComponentBooleanProperty.h"
#include "Properties/jucer_ComponentChoiceProperty.h"
#include "Properties/jucer_ComponentTextProperty.h"
#include "Properties/jucer_ComponentColourProperty.h"
#include "Properties/jucer_FilePropertyComponent.h"
#include "PaintElements/jucer_ImageResourceProperty.h"

#include "jucer_ObjectTypes.h"

#include "PaintElements/jucer_PaintElementUndoableAction.h"
#include "PaintElements/jucer_PaintElementRectangle.h"
#include "PaintElements/jucer_PaintElementRoundedRectangle.h"
#include "PaintElements/jucer_PaintElementImage.h"
#include "PaintElements/jucer_PaintElementEllipse.h"
#include "PaintElements/jucer_PaintElementPath.h"
#include "PaintElements/jucer_PaintElementText.h"
#include "PaintElements/jucer_PaintElementGroup.h"

#include "Components/jucer_ButtonHandler.h"
#include "Components/jucer_TextButtonHandler.h"
#include "Components/jucer_ToggleButtonHandler.h"
#include "Components/jucer_SliderHandler.h"
#include "Components/jucer_LabelHandler.h"
#include "Components/jucer_TextEditorHandler.h"
#include "Components/jucer_ComboBoxHandler.h"
#include "Components/jucer_GroupComponentHandler.h"
#include "Components/jucer_JucerComponentHandler.h"
#include "Components/jucer_HyperlinkButtonHandler.h"
#include "Components/jucer_ViewportHandler.h"
#include "Components/jucer_TabbedComponentHandler.h"
#include "Components/jucer_TreeViewHandler.h"
#include "Components/jucer_GenericComponentHandler.h"
#include "Components/jucer_ImageButtonHandler.h"

namespace ObjectTypes
{

static const char* const elementNames[] =
{
    "Rectangle",
    "Rounded Rectangle",
    "Ellipse",
    "Path",
    "Image",
    "Text",
    nullptr
};

const char* const* const elementTypeNames = (const char* const*) elementNames;
const int numElementTypes = (sizeof (elementNames) / sizeof (elementNames[0])) - 1;

PaintElement* createNewElement (const int index, PaintRoutine* owner)
{
    switch (index)
    {
        case 0:   return new PaintElementRectangle (owner);
        case 1:   return new PaintElementRoundedRectangle (owner);
        case 2:   return new PaintElementEllipse (owner);
        case 3:   return new PaintElementPath (owner);
        case 4:   return new PaintElementImage (owner);
        case 5:   return new PaintElementText (owner);
        default:  jassertfalse; break;
    }

    return nullptr;
}

PaintElement* createNewImageElement (PaintRoutine* owner)
{
    return new PaintElementImage (owner);
}

PaintElement* createElementForXml (const XmlElement* const e, PaintRoutine* const owner)
{
    jassert (e != nullptr);

    std::unique_ptr<PaintElement> pe;

    if (e->hasTagName (PaintElementRectangle::getTagName()))                pe.reset (new PaintElementRectangle (owner));
    else if (e->hasTagName (PaintElementRoundedRectangle::getTagName()))    pe.reset (new PaintElementRoundedRectangle (owner));
    else if (e->hasTagName (PaintElementEllipse::getTagName()))             pe.reset (new PaintElementEllipse (owner));
    else if (e->hasTagName (PaintElementImage::getTagName()))               pe.reset (new PaintElementImage (owner));
    else if (e->hasTagName (PaintElementPath::getTagName()))                pe.reset (new PaintElementPath (owner));
    else if (e->hasTagName (PaintElementText::getTagName()))                pe.reset (new PaintElementText (owner));
    else if (e->hasTagName (PaintElementGroup::getTagName()))               pe.reset (new PaintElementGroup (owner));

    if (pe != nullptr && pe->loadFromXml (*e))
        return pe.release();

    jassertfalse;
    return nullptr;
}

//==============================================================================
static TextButtonHandler textButton;
static ToggleButtonHandler toggleButton;
static SliderHandler slider;
static LabelHandler label;
static TextEditorHandler textEditor;
static ComboBoxHandler comboBox;
static JucerComponentHandler jucerCompHandler;
static GroupComponentHandler group;
static HyperlinkButtonHandler hyperlink;
static ViewportHandler viewport;
static TabbedComponentHandler tabbedComp;
static TreeViewHandler treeview;
static GenericComponentHandler genericHandler;
static ImageButtonHandler imageButtonHandler;

static ComponentTypeHandler* const compTypes[] =
{
    &textButton,
    &toggleButton,
    &slider,
    &label,
    &textEditor,
    &comboBox,
    &group,
    &jucerCompHandler,
    &hyperlink,
    &viewport,
    &tabbedComp,
    &treeview,
    &genericHandler,
    &imageButtonHandler,
    nullptr
};

ComponentTypeHandler* const* const componentTypeHandlers = (ComponentTypeHandler* const*) compTypes;
const int numComponentTypes = numElementsInArray (compTypes) - 1;

}
