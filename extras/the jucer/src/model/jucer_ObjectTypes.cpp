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

#include "../jucer_Headers.h"

#include "jucer_ObjectTypes.h"

#include "paintelements/jucer_PaintElementUndoableAction.h"
#include "paintelements/jucer_PaintElementRectangle.h"
#include "paintelements/jucer_PaintElementRoundedRectangle.h"
#include "paintelements/jucer_PaintElementImage.h"
#include "paintelements/jucer_PaintElementEllipse.h"
#include "paintelements/jucer_PaintElementPath.h"
#include "paintelements/jucer_PaintElementText.h"
#include "paintelements/jucer_PaintElementGroup.h"

#include "documents/jucer_ComponentDocument.h"
#include "documents/jucer_ButtonDocument.h"

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

//==============================================================================
static const char* const documentNames[] =
{
    "Component",
    "Button",
    0
};

const char** const documentTypeNames = (const char**) documentNames;
const int numDocumentTypes = numElementsInArray (documentNames) - 1;

JucerDocument* createNewDocument (const int index)
{
    jassert (index >= 0 && index < numDocumentTypes);
    JucerDocument* newDoc = 0;

    switch (index)
    {
    case 0:
        newDoc = new ComponentDocument();
        break;

    case 1:
        newDoc = new ButtonDocument();
        break;

    default:
        break;
    }

    jassert (newDoc != 0 && newDoc->getTypeName() == documentTypeNames[index]);
    return newDoc;
}

JucerDocument* loadDocumentFromFile (const File& f, const bool showErrorMessage)
{
    File file (f);

    if (file == File::nonexistent && showErrorMessage)
    {
        FileChooser fc ("Open a Jucer C++ file...",
                        StoredSettings::getInstance()->recentFiles.getFile (0),
                        "*.cpp");

        if (! fc.browseForFileToOpen())
            return 0;

        file = fc.getResult();
    }

    ScopedPointer<XmlElement> xml (JucerDocument::pullMetaDataFromCppFile (file.loadFileAsString()));

    if (xml == nullptr || ! xml->hasTagName (JucerDocument::jucerCompXmlTag))
    {
        if (file != File::nonexistent && showErrorMessage)
            AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                         TRANS("Failed to open file..."),
                                         TRANS("This wasn't a valid Jucer .cpp file..."));

        return 0;
    }

    const String docType (xml->getStringAttribute ("documentType"));

    // (reverse order so ComponentDocument is default last-case)
    for (int i = numDocumentTypes; --i >= 0;)
    {
        if (docType.equalsIgnoreCase (documentTypeNames[i]) || i == 0)
        {
            ScopedPointer<JucerDocument> doc (createNewDocument (i));

            if (doc->loadFrom (file, showErrorMessage))
                return doc.release();

            break;
        }
    }

    return 0;
}

//==============================================================================
static const char* const elementNames[] =
{
    "Rectangle",
    "Rounded Rectangle",
    "Ellipse",
    "Path",
    "Image",
    "Text",
    0
};

const char** const elementTypeNames = (const char**) elementNames;
const int numElementTypes = (sizeof (elementNames) / sizeof (elementNames[0])) - 1;

PaintElement* createNewElement (const int index, PaintRoutine* owner)
{
    switch (index)
    {
    case 0:
        return new PaintElementRectangle (owner);

    case 1:
        return new PaintElementRoundedRectangle (owner);

    case 2:
        return new PaintElementEllipse (owner);

    case 3:
        return new PaintElementPath (owner);

    case 4:
        return new PaintElementImage (owner);

    case 5:
        return new PaintElementText (owner);

    default:
        jassertfalse
        break;
    }

    return 0;
}

PaintElement* createNewImageElement (PaintRoutine* owner)
{
    return new PaintElementImage (owner);
}

PaintElement* createElementForXml (const XmlElement* const e, PaintRoutine* const owner)
{
    jassert (e != 0);

    ScopedPointer<PaintElement> pe;

    if (e->hasTagName (PaintElementRectangle::getTagName()))
    {
        pe = new PaintElementRectangle (owner);
    }
    else if (e->hasTagName (PaintElementRoundedRectangle::getTagName()))
    {
        pe = new PaintElementRoundedRectangle (owner);
    }
    else if (e->hasTagName (PaintElementEllipse::getTagName()))
    {
        pe = new PaintElementEllipse (owner);
    }
    else if (e->hasTagName (PaintElementImage::getTagName()))
    {
        pe = new PaintElementImage (owner);
    }
    else if (e->hasTagName (PaintElementPath::getTagName()))
    {
        pe = new PaintElementPath (owner);
    }
    else if (e->hasTagName (PaintElementText::getTagName()))
    {
        pe = new PaintElementText (owner);
    }
    else if (e->hasTagName (PaintElementGroup::getTagName()))
    {
        pe = new PaintElementGroup (owner);
    }

    if (pe != nullptr && pe->loadFromXml (*e))
        return pe.release();

    jassertfalse
    return 0;
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
    0
};

ComponentTypeHandler** const componentTypeHandlers = (ComponentTypeHandler**) compTypes;
const int numComponentTypes = numElementsInArray (compTypes) - 1;

}
