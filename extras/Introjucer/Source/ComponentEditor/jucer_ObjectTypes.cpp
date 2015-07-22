/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#include "../jucer_Headers.h"

#include "jucer_JucerDocument.h"
#include "components/jucer_ComponentUndoableAction.h"
#include "properties/jucer_JustificationProperty.h"
#include "properties/jucer_FontPropertyComponent.h"
#include "properties/jucer_ComponentBooleanProperty.h"
#include "properties/jucer_ComponentChoiceProperty.h"
#include "properties/jucer_ComponentTextProperty.h"
#include "properties/jucer_ComponentColourProperty.h"
#include "properties/jucer_FilePropertyComponent.h"
#include "paintelements/jucer_ImageResourceProperty.h"

#include "jucer_ObjectTypes.h"

#include "paintelements/jucer_PaintElementUndoableAction.h"
#include "paintelements/jucer_PaintElementRectangle.h"
#include "paintelements/jucer_PaintElementRoundedRectangle.h"
#include "paintelements/jucer_PaintElementImage.h"
#include "paintelements/jucer_PaintElementEllipse.h"
#include "paintelements/jucer_PaintElementPath.h"
#include "paintelements/jucer_PaintElementText.h"
#include "paintelements/jucer_PaintElementGroup.h"

#include "components/jucer_ButtonHandler.h"
#include "components/jucer_TextButtonHandler.h"
#include "components/jucer_ToggleButtonHandler.h"
#include "components/jucer_SliderHandler.h"
#include "components/jucer_LabelHandler.h"
#include "components/jucer_TextEditorHandler.h"
#include "components/jucer_ComboBoxHandler.h"
#include "components/jucer_GroupComponentHandler.h"
#include "components/jucer_JucerComponentHandler.h"
#include "components/jucer_HyperlinkButtonHandler.h"
#include "components/jucer_ViewportHandler.h"
#include "components/jucer_TabbedComponentHandler.h"
#include "components/jucer_TreeViewHandler.h"
#include "components/jucer_GenericComponentHandler.h"
#include "components/jucer_ImageButtonHandler.h"

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

const char** const elementTypeNames = (const char**) elementNames;
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

    ScopedPointer<PaintElement> pe;

    if (e->hasTagName (PaintElementRectangle::getTagName()))                pe = new PaintElementRectangle (owner);
    else if (e->hasTagName (PaintElementRoundedRectangle::getTagName()))    pe = new PaintElementRoundedRectangle (owner);
    else if (e->hasTagName (PaintElementEllipse::getTagName()))             pe = new PaintElementEllipse (owner);
    else if (e->hasTagName (PaintElementImage::getTagName()))               pe = new PaintElementImage (owner);
    else if (e->hasTagName (PaintElementPath::getTagName()))                pe = new PaintElementPath (owner);
    else if (e->hasTagName (PaintElementText::getTagName()))                pe = new PaintElementText (owner);
    else if (e->hasTagName (PaintElementGroup::getTagName()))               pe = new PaintElementGroup (owner);

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

ComponentTypeHandler** const componentTypeHandlers = (ComponentTypeHandler**) compTypes;
const int numComponentTypes = numElementsInArray (compTypes) - 1;

}
