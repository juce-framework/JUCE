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

#ifndef __JUCER_DRAWABLEEDITOR_JUCEHEADER__
#define __JUCER_DRAWABLEEDITOR_JUCEHEADER__

#include "../jucer_DocumentEditorComponent.h"
#include "../Editor Base/jucer_EditorCanvas.h"
class DrawableEditorCanvas;


//==============================================================================
/**
*/
class DrawableEditor  : public DocumentEditorComponent
{
public:
    //==============================================================================
    DrawableEditor (OpenDocumentManager::Document* document,
                    Project* project, DrawableDocument* drawableDocument);
    ~DrawableEditor();

    //==============================================================================
    void getAllCommands (Array <CommandID>& commands);
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result);
    bool perform (const InvocationInfo& info);

    void paint (Graphics& g);
    void resized();

    //==============================================================================
    const StringArray getSelectedIds() const;
    void deleteSelection();
    void selectionToFront();
    void selectionToBack();
    void showNewShapeMenu (Component* componentToAttachTo);

    //==============================================================================
    DrawableDocument& getDocument() const   { return *drawableDocument; }
    UndoManager* getUndoManager() const     { return getDocument().getUndoManager(); }

    EditorCanvasBase::SelectedItems& getSelection()         { return selection; }

    class Panel;

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    Project* project;
    DrawableDocument* drawableDocument;
    EditorCanvasBase::SelectedItems selection;

    Panel* panel;
};


#endif   // __JUCER_DRAWABLEEDITOR_JUCEHEADER__
