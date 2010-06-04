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
#include "../Editor Base/jucer_EditorPanel.h"
#include "../Editor Base/jucer_EditorCanvas.h"
#include "../Editor Base/jucer_EditorDragOperation.h"
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
                    new DrawableTreeViewItem (editor, editor.getDocument().getRootDrawableNode().getState()));
    }

    SelectedItemSet<String>& getSelection()
    {
        return editor.getSelection();
    }

    void getSelectedItemProperties (Array<PropertyComponent*>& props)
    {
        editor.getDocument().createItemProperties (props, editor.getSelectedIds());
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

//==============================================================================
const StringArray DrawableEditor::getSelectedIds() const
{
    StringArray ids;
    const int num = selection.getNumSelected();
    for (int i = 0; i < num; ++i)
        ids.add (selection.getSelectedItem(i));

    return ids;
}

void DrawableEditor::deleteSelection()
{
    getUndoManager()->beginNewTransaction();
    DrawableComposite::ValueTreeWrapper root (getDocument().getRootDrawableNode());

    const StringArray ids (getSelectedIds());
    for (int i = ids.size(); --i >= 0;)
    {
        const ValueTree v (root.getDrawableWithId (ids[i], false));
        root.removeDrawable (v, getUndoManager());
    }

    getUndoManager()->beginNewTransaction();
}

void DrawableEditor::selectionToFront()
{
    getUndoManager()->beginNewTransaction();

    DrawableComposite::ValueTreeWrapper root (getDocument().getRootDrawableNode());
    int index = 0;

    for (int i = root.getNumDrawables(); --i >= 0;)
    {
        const Drawable::ValueTreeWrapperBase d (root.getDrawableState (index));

        if (getSelection().isSelected (d.getID()))
            root.moveDrawableOrder (index, -1, getUndoManager());
        else
            ++index;
    }

    getUndoManager()->beginNewTransaction();
}

void DrawableEditor::selectionToBack()
{
    getUndoManager()->beginNewTransaction();

    DrawableComposite::ValueTreeWrapper root (getDocument().getRootDrawableNode());
    int index = root.getNumDrawables() - 1;

    for (int i = root.getNumDrawables(); --i >= 0;)
    {
        const Drawable::ValueTreeWrapperBase d (root.getDrawableState (index));

        if (getSelection().isSelected (d.getID()))
            root.moveDrawableOrder (index, 0, getUndoManager());
        else
            --index;
    }

    getUndoManager()->beginNewTransaction();
}

void DrawableEditor::showNewShapeMenu (Component* componentToAttachTo)
{
    PopupMenu m;
    getDocument().addNewItemMenuItems (m);
    const int r = m.showAt (componentToAttachTo);

    ValueTree newItem (getDocument().performNewItemMenuItem (r));

    if (newItem.isValid())
        getSelection().selectOnly (Drawable::ValueTreeWrapperBase (newItem).getID());
}

//==============================================================================
void DrawableEditor::getAllCommands (Array <CommandID>& commands)
{
    DocumentEditorComponent::getAllCommands (commands);

    const CommandID ids[] = { CommandIDs::undo,
                              CommandIDs::redo,
                              CommandIDs::toFront,
                              CommandIDs::toBack,
                              CommandIDs::showOrHideProperties,
                              CommandIDs::showOrHideTree,
                              CommandIDs::showOrHideMarkers,
                              CommandIDs::toggleSnapping,
                              StandardApplicationCommandIDs::del };

    commands.addArray (ids, numElementsInArray (ids));
}

void DrawableEditor::getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
{
    result.setActive (document != 0);

    switch (commandID)
    {
    case CommandIDs::undo:
        result.setInfo ("Undo", "Undoes the last change", CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress ('z', ModifierKeys::commandModifier, 0));
        break;

    case CommandIDs::redo:
        result.setInfo ("Redo", "Redoes the last change", CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress ('z', ModifierKeys::shiftModifier | ModifierKeys::commandModifier, 0));
        result.defaultKeypresses.add (KeyPress ('y', ModifierKeys::commandModifier, 0));
        break;

    case CommandIDs::toFront:
        result.setInfo ("Bring to Front", "Brings the selected items to the front", CommandCategories::editing, 0);
        break;

    case CommandIDs::toBack:
        result.setInfo ("Send to Back", "Moves the selected items to the back", CommandCategories::editing, 0);
        break;

    case CommandIDs::showOrHideProperties:
        result.setInfo ("Show/Hide Tree", "Shows or hides the component tree view", CommandCategories::editing, 0);
        result.setTicked (panel != 0 && panel->arePropertiesVisible());
        break;

    case CommandIDs::showOrHideTree:
        result.setInfo ("Show/Hide Properties", "Shows or hides the component properties panel", CommandCategories::editing, 0);
        result.setTicked (panel != 0 && panel->isTreeVisible());
        break;

    case CommandIDs::showOrHideMarkers:
        result.setInfo ("Show/Hide Markers", "Shows or hides the markers", CommandCategories::editing, 0);
        result.setTicked (panel != 0 && panel->areMarkersVisible());
        break;

    case CommandIDs::toggleSnapping:
        result.setInfo ("Toggle snapping", "Turns object snapping on or off", CommandCategories::editing, 0);
        result.setTicked (panel != 0 && panel->isSnappingEnabled());
        break;

    case StandardApplicationCommandIDs::del:
        result.setInfo ("Delete", String::empty, CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress (KeyPress::deleteKey, 0, 0));
        result.defaultKeypresses.add (KeyPress (KeyPress::backspaceKey, 0, 0));
        break;

    default:
        DocumentEditorComponent::getCommandInfo (commandID, result);
        break;
    }
}

bool DrawableEditor::perform (const InvocationInfo& info)
{
    switch (info.commandID)
    {
    case CommandIDs::undo:
        getUndoManager()->beginNewTransaction();
        getUndoManager()->undo();
        return true;

    case CommandIDs::redo:
        getUndoManager()->beginNewTransaction();
        getUndoManager()->redo();
        return true;

    case CommandIDs::toFront:
        selectionToFront();
        return true;

    case CommandIDs::toBack:
        selectionToBack();
        return true;

    case CommandIDs::showOrHideProperties:
        panel->showOrHideProperties();
        return true;

    case CommandIDs::showOrHideTree:
        panel->showOrHideTree();
        return true;

    case CommandIDs::showOrHideMarkers:
        panel->showOrHideMarkers();
        return true;

    case CommandIDs::toggleSnapping:
        panel->toggleSnapping();
        return true;

    case StandardApplicationCommandIDs::del:
        deleteSelection();
        return true;

    default:
        break;
    }

    return DocumentEditorComponent::perform (info);
}
