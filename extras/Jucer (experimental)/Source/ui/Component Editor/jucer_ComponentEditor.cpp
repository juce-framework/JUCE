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
#include "../../model/Component/jucer_ComponentDocument.h"
#include "../jucer_JucerTreeViewBase.h"
#include "../Editor Base/jucer_EditorPanel.h"
#include "../Editor Base/jucer_EditorDragOperation.h"
#include "jucer_ComponentEditor.h"
#include "jucer_ComponentEditorCanvas.h"
#include "jucer_ComponentEditorTreeView.h"
#include "jucer_ComponentEditorCodeView.h"
#include "jucer_ComponentEditorToolbar.h"


//==============================================================================
class ComponentEditor::ClassInfoHolder  : public Component
{
public:
    ClassInfoHolder (ComponentEditor& editor_)
        : editor (editor_)
    {
        addAndMakeVisible (panel = new PropertyPanelWithTooltips());

        Array <PropertyComponent*> props;
        editor.getDocument().createClassProperties (props);
        panel->getPanel()->addSection ("Component Properties", props, true);
    }

    ~ClassInfoHolder()
    {
        deleteAllChildren();
    }

    void resized()
    {
        panel->setBounds (getLocalBounds());
    }

private:
    ComponentEditor& editor;
    PropertyPanelWithTooltips* panel;
};


//==============================================================================
class ComponentEditor::LayoutEditorHolder  : public EditorPanelBase
{
public:
    LayoutEditorHolder (ComponentEditor& editor_)
        : toolbarFactory (editor_),
          editor (editor_)
    {
    }

    ~LayoutEditorHolder()
    {
        shutdown();
    }

    void createCanvas()
    {
        initialise (new ComponentEditorCanvas (editor), toolbarFactory,
                    new ComponentEditorTreeView::Root (editor));
    }

    SelectedItemSet<String>& getSelection()
    {
        return editor.getSelection();
    }

    void getSelectedItemProperties (Array<PropertyComponent*>& newComps)
    {
        editor.getSelectedItemProperties (newComps);
    }

private:
    ComponentEditorToolbarFactory toolbarFactory;
    ComponentEditor& editor;
};

//==============================================================================
class ComponentEditor::BackgroundEditorHolder  : public Component
{
public:
    BackgroundEditorHolder (ComponentEditor& editor_)
        : editor (editor_)
    {
    }

    ~BackgroundEditorHolder()
    {
    }

private:
    ComponentEditor& editor;
};


//==============================================================================
ComponentEditor::ComponentEditor (OpenDocumentManager::Document* document_,
                                  Project* project_, ComponentDocument* componentDocument_)
    : DocumentEditorComponent (document_),
      project (project_),
      componentDocument (componentDocument_),
      classInfoHolder (0),
      layoutEditorHolder (0),
      backgroundEditorHolder (0),
      codeEditorHolder (0)
{
    setOpaque (true);

    if (componentDocument != 0)
    {
        classInfoHolder = new ClassInfoHolder (*this);
        layoutEditorHolder = new LayoutEditorHolder (*this);
        backgroundEditorHolder = new BackgroundEditorHolder (*this);
        codeEditorHolder = new CodeEditorHolder (*this);
        layoutEditorHolder->createCanvas();
    }

    addAndMakeVisible (tabs = new TabbedComponent (TabbedButtonBar::TabsAtRight));
    tabs->setTabBarDepth (22);
    tabs->setOutline (0);

    tabs->addTab ("Class Settings", Colour::greyLevel (0.88f), classInfoHolder, true);
    tabs->addTab ("Components", Colours::white, layoutEditorHolder, true);
    tabs->addTab ("Background", Colours::white, backgroundEditorHolder, true);
    tabs->addTab ("Source Code", Colours::white, codeEditorHolder, true);

    tabs->setCurrentTabIndex (1);
}

ComponentEditor::~ComponentEditor()
{
    deleteAllChildren();
}

void ComponentEditor::paint (Graphics& g)
{
    g.fillAll (Colours::white);
}

void ComponentEditor::resized()
{
    tabs->setBounds (getLocalBounds());
}

const StringArray ComponentEditor::getSelectedIds() const
{
    StringArray ids;
    const int num = selection.getNumSelected();
    for (int i = 0; i < num; ++i)
        ids.add (selection.getSelectedItem(i));

    return ids;
}

void ComponentEditor::getSelectedItemProperties (Array <PropertyComponent*>& props)
{
    getDocument().createItemProperties (props, getSelectedIds());
}

void ComponentEditor::deleteSelection()
{
    const StringArray ids (getSelectedIds());
    getSelection().deselectAll();

    getDocument().beginNewTransaction();

    for (int i = ids.size(); --i >= 0;)
    {
        const ValueTree comp (getDocument().getComponentWithID (ids[i]));

        if (comp.isValid())
            getDocument().removeComponent (comp);
    }

    getDocument().beginNewTransaction();
}

void ComponentEditor::deselectNonComponents()
{
    EditorCanvasBase::SelectedItems& sel = getSelection();

    for (int i = sel.getNumSelected(); --i >= 0;)
        if (! getDocument().getComponentWithID (sel.getSelectedItem (i)).isValid())
            sel.deselect (sel.getSelectedItem (i));
}

void ComponentEditor::selectionToFront()
{
    getDocument().beginNewTransaction();

    int index = 0;
    for (int i = getDocument().getNumComponents(); --i >= 0;)
    {
        const ValueTree comp (getDocument().getComponent (index));

        if (comp.isValid() && getSelection().isSelected (comp [ComponentDocument::idProperty]))
        {
            ValueTree parent (comp.getParent());
            parent.moveChild (parent.indexOf (comp), -1, getDocument().getUndoManager());
        }
        else
        {
            ++index;
        }
    }

    getDocument().beginNewTransaction();
}

void ComponentEditor::selectionToBack()
{
    getDocument().beginNewTransaction();

    int index = getDocument().getNumComponents() - 1;
    for (int i = getDocument().getNumComponents(); --i >= 0;)
    {
        const ValueTree comp (getDocument().getComponent (index));

        if (comp.isValid() && getSelection().isSelected (comp [ComponentDocument::idProperty]))
        {
            ValueTree parent (comp.getParent());
            parent.moveChild (parent.indexOf (comp), 0, getDocument().getUndoManager());
        }
        else
        {
            --index;
        }
    }

    getDocument().beginNewTransaction();
}

//==============================================================================
void ComponentEditor::showNewComponentMenu (Component* componentToAttachTo)
{
    PopupMenu m;
    getDocument().addNewComponentMenuItems (m);

    const int r = m.showAt (componentToAttachTo);

    const ValueTree newComp (getDocument().performNewComponentMenuItem (r));

    if (newComp.isValid())
        getSelection().selectOnly (newComp [ComponentDocument::idProperty]);
}

//==============================================================================
class TestComponent     : public Component
{
public:
    TestComponent (ComponentDocument& document_)
        : document (document_)
    {
        setSize (document.getCanvasWidth().getValue(),
                 document.getCanvasHeight().getValue());
    }

    ~TestComponent()
    {
        deleteAllChildren();
    }

    void resized()
    {
        document.getCanvasWidth() = getWidth();
        document.getCanvasHeight() = getHeight();

        ComponentEditorCanvas::updateComponentsIn (this, document, selected);
    }

private:
    ComponentDocument document;
    ComponentEditorCanvas::SelectedItems selected;
    TooltipWindow tooltipWindow;
};

void ComponentEditor::test()
{
    TestComponent testComp (getDocument());

    DialogWindow::showModalDialog ("Testing: " + getDocument().getClassName().toString(),
                                   &testComp, this, Colours::lightgrey, true, true);
}

//==============================================================================
void ComponentEditor::getAllCommands (Array <CommandID>& commands)
{
    DocumentEditorComponent::getAllCommands (commands);

    const CommandID ids[] = { CommandIDs::undo,
                              CommandIDs::redo,
                              CommandIDs::toFront,
                              CommandIDs::toBack,
                              CommandIDs::test,
                              CommandIDs::showOrHideProperties,
                              CommandIDs::showOrHideTree,
                              CommandIDs::showOrHideMarkers,
                              CommandIDs::toggleSnapping,
                              StandardApplicationCommandIDs::del };

    commands.addArray (ids, numElementsInArray (ids));
}

void ComponentEditor::getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
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

    case CommandIDs::test:
        result.setInfo ("Test", "Test the current component", CommandCategories::editing, 0);
        result.defaultKeypresses.add (KeyPress ('t', ModifierKeys::commandModifier, 0));
        break;

    case CommandIDs::showOrHideProperties:
        result.setInfo ("Show/Hide Tree", "Shows or hides the component tree view", CommandCategories::editing, 0);
        result.setTicked (layoutEditorHolder != 0 && layoutEditorHolder->arePropertiesVisible());
        break;

    case CommandIDs::showOrHideTree:
        result.setInfo ("Show/Hide Properties", "Shows or hides the component properties panel", CommandCategories::editing, 0);
        result.setTicked (layoutEditorHolder != 0 && layoutEditorHolder->isTreeVisible());
        break;

    case CommandIDs::showOrHideMarkers:
        result.setInfo ("Show/Hide Markers", "Shows or hides the markers", CommandCategories::editing, 0);
        result.setTicked (layoutEditorHolder != 0 && layoutEditorHolder->areMarkersVisible());
        break;

    case CommandIDs::toggleSnapping:
        result.setInfo ("Toggle snapping", "Turns object snapping on or off", CommandCategories::editing, 0);
        result.setTicked (layoutEditorHolder != 0 && layoutEditorHolder->isSnappingEnabled());
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

bool ComponentEditor::perform (const InvocationInfo& info)
{
    switch (info.commandID)
    {
    case CommandIDs::undo:
        getDocument().getUndoManager()->undo();
        return true;

    case CommandIDs::redo:
        getDocument().getUndoManager()->redo();
        return true;

    case CommandIDs::toFront:
        selectionToFront();
        return true;

    case CommandIDs::toBack:
        selectionToBack();
        return true;

    case CommandIDs::test:
        test();
        return true;

    case CommandIDs::showOrHideProperties:
        layoutEditorHolder->showOrHideProperties();
        return true;

    case CommandIDs::showOrHideTree:
        layoutEditorHolder->showOrHideTree();
        return true;

    case CommandIDs::showOrHideMarkers:
        layoutEditorHolder->showOrHideMarkers();
        return true;

    case CommandIDs::toggleSnapping:
        layoutEditorHolder->toggleSnapping();
        return true;

    case StandardApplicationCommandIDs::del:
        deleteSelection();
        return true;

    default:
        break;
    }

    return DocumentEditorComponent::perform (info);
}
