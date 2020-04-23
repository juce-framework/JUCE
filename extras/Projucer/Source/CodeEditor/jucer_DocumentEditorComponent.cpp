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

#include "../Application/jucer_Headers.h"
#include "jucer_DocumentEditorComponent.h"
#include "../Application/jucer_Application.h"
#include "../Project/UI/jucer_ProjectContentComponent.h"

//==============================================================================
DocumentEditorComponent::DocumentEditorComponent (OpenDocumentManager::Document* doc)
    : document (doc)
{
    ProjucerApplication::getApp().openDocumentManager.addListener (this);
}

DocumentEditorComponent::~DocumentEditorComponent()
{
    ProjucerApplication::getApp().openDocumentManager.removeListener (this);
}

bool DocumentEditorComponent::documentAboutToClose (OpenDocumentManager::Document* closingDoc)
{
    if (document == closingDoc)
    {
        jassert (document != nullptr);

        if (auto* pcc = findParentComponentOfClass<ProjectContentComponent>())
            pcc->hideDocument (document);
    }

    return true;
}

void DocumentEditorComponent::setEditedState (bool hasBeenEdited)
{
    if (hasBeenEdited != lastEditedState)
    {
        if (auto* pcc = findParentComponentOfClass<ProjectContentComponent>())
            pcc->refreshProjectTreeFileStatuses();

        lastEditedState = hasBeenEdited;
    }
}
