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

#include "../../jucer_Headers.h"
#include "../../model/Drawable/jucer_DrawableDocument.h"
#include "../jucer_JucerTreeViewBase.h"
#include "../Editor Base/jucer_EditorDragOperation.h"
#include "../Editor Base/jucer_EditorPanel.h"
#include "../Editor Base/jucer_EditorCanvas.h"
#include "jucer_DrawableEditor.h"
#include "jucer_DrawableEditorCanvas.h"
#include "jucer_DrawableEditorTreeView.h"
#include "jucer_DrawableEditorToolbar.h"


//==============================================================================
class DrawableEditor::Panel  : public EditorPanelBase
{
public:
    Panel (DrawableEditor& editor_)
        : toolbarFactory (editor_),
          editor (editor_)
    {
    }

    ~Panel()
    {
        shutdown();
    }

    void createCanvas()
    {
        initialise (new DrawableEditorCanvas (editor), toolbarFactory,
                    DrawableTreeViewItem::createItemForNode (editor, editor.getDocument().getRootDrawableNode()));
    }

    SelectedItemSet<String>& getSelection()
    {
        return editor.getSelection();
    }

    void getSelectedItemProperties (Array<PropertyComponent*>& newComps)
    {
        //editor.getSelectedItemProperties (newComps);
    }

private:
    DrawableEditorToolbarFactory toolbarFactory;
    DrawableEditor& editor;
};

//==============================================================================
DrawableEditor::DrawableEditor (OpenDocumentManager::Document* document_,
                                Project* project_,
                                DrawableDocument* drawableDocument_)
    : DocumentEditorComponent (document_),
      project (project_),
      drawableDocument (drawableDocument_)
{
    jassert (drawableDocument_ != 0);

    setOpaque (true);

    addAndMakeVisible (panel = new Panel (*this));
    panel->createCanvas();
}

DrawableEditor::~DrawableEditor()
{
    deleteAllChildren();
}

void DrawableEditor::paint (Graphics& g)
{
    g.fillAll (Colours::white);
}

void DrawableEditor::resized()
{
    panel->setBounds (getLocalBounds());
}
