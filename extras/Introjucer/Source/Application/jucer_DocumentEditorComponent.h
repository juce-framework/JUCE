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

#ifndef JUCER_DOCUMENTEDITORCOMPONENT_H_INCLUDED
#define JUCER_DOCUMENTEDITORCOMPONENT_H_INCLUDED

#include "jucer_OpenDocumentManager.h"


//==============================================================================
/**
*/
class DocumentEditorComponent  : public Component,
                                 public OpenDocumentManager::DocumentCloseListener
{
public:
    //==============================================================================
    DocumentEditorComponent (OpenDocumentManager::Document* document);
    ~DocumentEditorComponent();

    OpenDocumentManager::Document* getDocument() const              { return document; }

    bool documentAboutToClose (OpenDocumentManager::Document*) override;

protected:
    OpenDocumentManager::Document* document;

    void setEditedState (bool hasBeenEdited);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DocumentEditorComponent)
};


#endif   // JUCER_DOCUMENTEDITORCOMPONENT_H_INCLUDED
