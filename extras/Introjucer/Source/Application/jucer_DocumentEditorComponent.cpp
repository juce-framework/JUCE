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

#include "jucer_DocumentEditorComponent.h"
#include "../Project/jucer_ProjectContentComponent.h"


//==============================================================================
DocumentEditorComponent::DocumentEditorComponent (OpenDocumentManager::Document* document_)
    : document (document_)
{
    OpenDocumentManager::getInstance()->addListener (this);
}

DocumentEditorComponent::~DocumentEditorComponent()
{
    OpenDocumentManager::getInstance()->removeListener (this);
}

void DocumentEditorComponent::documentAboutToClose (OpenDocumentManager::Document* closingDoc)
{
    if (document == closingDoc)
    {
        jassert (document != nullptr);
        ProjectContentComponent* pcc = findParentComponentOfClass ((ProjectContentComponent*) 0);

        if (pcc != nullptr)
        {
            pcc->hideDocument (document);
            return;
        }

        jassertfalse
    }
}

ApplicationCommandTarget* DocumentEditorComponent::getNextCommandTarget()
{
    return findFirstTargetParentComponent();
}

void DocumentEditorComponent::getAllCommands (Array <CommandID>& commands)
{
    const CommandID ids[] = { CommandIDs::saveDocument,
                              CommandIDs::saveDocumentAs,
                              CommandIDs::closeDocument };

    commands.addArray (ids, numElementsInArray (ids));
}

void DocumentEditorComponent::getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result)
{
    result.setActive (document != nullptr);
    String name;

    if (document != nullptr)
        name = " '" + document->getName().substring (0, 32) + "'";

    switch (commandID)
    {
    case CommandIDs::saveDocument:
        result.setInfo ("Save" + name,
                        "Saves the current document",
                        CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress ('s', ModifierKeys::commandModifier, 0));
        break;

    case CommandIDs::saveDocumentAs:
        result.setInfo ("Save" + name + " As...",
                        "Saves the current document to a different filename",
                        CommandCategories::general, 0);
        result.setActive (document != nullptr && document->canSaveAs());
        result.defaultKeypresses.add (KeyPress ('s', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0));
        break;

    case CommandIDs::closeDocument:
        result.setInfo ("Close" + name,
                        "Closes the current document",
                        CommandCategories::general, 0);
       #if JUCE_MAC
        result.defaultKeypresses.add (KeyPress ('w', ModifierKeys::commandModifier | ModifierKeys::ctrlModifier, 0));
       #else
        result.defaultKeypresses.add (KeyPress ('w', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0));
       #endif
        break;

    default:
        break;
    }
}

bool DocumentEditorComponent::perform (const InvocationInfo& info)
{
    switch (info.commandID)
    {
    case CommandIDs::saveDocument:
        document->save();
        return true;

    case CommandIDs::saveDocumentAs:
        document->saveAs();
        return true;

    case CommandIDs::closeDocument:
        OpenDocumentManager::getInstance()->closeDocument (document, true);
        return true;

    default:
        break;
    }

    return false;
}
