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
