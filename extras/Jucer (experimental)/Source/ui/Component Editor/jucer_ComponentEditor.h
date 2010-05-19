/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#ifndef __JUCE_COMPONENTEDITOR_H_6CAE6B7E__
#define __JUCE_COMPONENTEDITOR_H_6CAE6B7E__

#include "../jucer_DocumentEditorComponent.h"
#include "../../model/Component/jucer_ComponentDocument.h"
#include "../Editor Base/jucer_EditorCanvas.h"


//==============================================================================
/**
*/
class ComponentEditor   : public DocumentEditorComponent
{
public:
    //==============================================================================
    ComponentEditor (OpenDocumentManager::Document* document,
                     Project* project,
                     ComponentDocument* componentDocument);

    ~ComponentEditor();

    //==============================================================================
    void getAllCommands (Array <CommandID>& commands);
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result);
    bool perform (const InvocationInfo& info);

    void paint (Graphics& g);
    void resized();

    //==============================================================================
    ComponentDocument& getDocument() const      { return *componentDocument; }

    const StringArray getSelectedIds() const;
    void deleteSelection();
    void deselectNonComponents();
    void selectionToFront();
    void selectionToBack();
    void showNewComponentMenu (Component* componentToAttachTo);

    //==============================================================================
    void test();

    EditorCanvasBase::SelectedItems& getSelection()         { return selection; }

private:
    class ClassInfoHolder;
    class LayoutEditorHolder;
    class BackgroundEditorHolder;
    class CodeEditorHolder;

    Project* project;
    ComponentDocument* componentDocument;
    EditorCanvasBase::SelectedItems selection;

    TabbedComponent* tabs;
    ClassInfoHolder* classInfoHolder;
    LayoutEditorHolder* layoutEditorHolder;
    BackgroundEditorHolder* backgroundEditorHolder;
    CodeEditorHolder* codeEditorHolder;

    ComponentEditor (const ComponentEditor&);
    ComponentEditor& operator= (const ComponentEditor&);
};


#endif  // __JUCE_COMPONENTEDITOR_H_6CAE6B7E__
