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

#include "jucer_OpenDocumentManager.h"

//==============================================================================
class DocumentEditorComponent  : public Component,
                                 public OpenDocumentManager::DocumentCloseListener
{
public:
    //==============================================================================
    DocumentEditorComponent (OpenDocumentManager::Document* document);
    ~DocumentEditorComponent() override;

    OpenDocumentManager::Document* getDocument() const              { return document; }

    bool documentAboutToClose (OpenDocumentManager::Document*) override;

protected:
    OpenDocumentManager::Document* document;
    bool lastEditedState = false;

    void setEditedState (bool hasBeenEdited);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DocumentEditorComponent)
};
