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
#include "jucer_ComponentEditor.h"
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
class ComponentEditor::LayoutEditorHolder  : public Component
{
public:
    LayoutEditorHolder (ComponentEditor& editor_)
        : toolbarFactory (editor_), editor (editor_), infoPanel (0), tree (0)
    {
        addAndMakeVisible (toolbar = new Toolbar());
        toolbar->addDefaultItems (toolbarFactory);
        toolbar->setStyle (Toolbar::textOnly);

        addAndMakeVisible (viewport = new Viewport());

        addChildComponent (tree = new TreeView());
        tree->setRootItemVisible (true);
        tree->setMultiSelectEnabled (true);
        tree->setDefaultOpenness (true);
        tree->setColour (TreeView::backgroundColourId, Colours::white);
        tree->setIndentSize (15);
    }

    ~LayoutEditorHolder()
    {
        tree->deleteRootItem();
        deleteAndZero (infoPanel);
        deleteAllChildren();
    }

    void createCanvas()
    {
        viewport->setViewedComponent (new ComponentEditorCanvas (editor));
        addAndMakeVisible (infoPanel = new InfoPanel (editor));
        tree->setRootItem (new ComponentEditorTreeView::Root (editor));
        resized();
    }

    void resized()
    {
        const int toolbarHeight = 22;

        toolbar->setBounds (0, 0, getWidth(), toolbarHeight);

        int infoPanelWidth = 200;
        if (infoPanel != 0 && infoPanel->isVisible())
            infoPanel->setBounds (getWidth() - infoPanelWidth, toolbar->getBottom(), infoPanelWidth, getHeight() - toolbar->getBottom());
        else
            infoPanelWidth = 0;

        if (tree->isVisible())
        {
            tree->setBounds (0, toolbar->getBottom(), infoPanelWidth, getHeight() - toolbar->getBottom());
            viewport->setBounds (infoPanelWidth, toolbar->getBottom(), getWidth() - infoPanelWidth * 2, getHeight() - toolbar->getBottom());
        }
        else
        {
            viewport->setBounds (0, toolbar->getBottom(), getWidth() - infoPanelWidth, getHeight() - toolbar->getBottom());
        }
    }

    void showOrHideProperties()
    {
        infoPanel->setVisible (! infoPanel->isVisible());
        resized();
    }

    void showOrHideTree()
    {
        tree->setVisible (! tree->isVisible());
        resized();
    }

    Viewport* getViewport() const   { return viewport; }

private:
    //==============================================================================
    class InfoPanel  : public Component,
                       public ChangeListener
    {
    public:
        InfoPanel (ComponentEditor& editor_)
          : editor (editor_)
        {
            setOpaque (true);

            addAndMakeVisible (props = new PropertyPanel());

            editor.getCanvas()->getSelection().addChangeListener (this);
        }

        ~InfoPanel()
        {
            editor.getCanvas()->getSelection().removeChangeListener (this);

            props->clear();
            deleteAllChildren();
        }

        void changeListenerCallback (void*)
        {
            Array <PropertyComponent*> newComps;
            editor.getCanvas()->getSelectedItemProperties (newComps);

            props->clear();
            props->addProperties (newComps);
        }

        void paint (Graphics& g)
        {
            g.fillAll (Colour::greyLevel (0.92f));
        }

        void resized()
        {
            props->setSize (getWidth(), getHeight());
        }

    private:
        ComponentEditor& editor;
        PropertyPanel* props;
    };

    Toolbar* toolbar;
    ComponentEditorToolbarFactory toolbarFactory;
    ComponentEditor& editor;
    Viewport* viewport;
    InfoPanel* infoPanel;
    TreeView* tree;
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
ComponentEditor::ComponentEditor (OpenDocumentManager::Document* document,
                                  Project* project_, ComponentDocument* componentDocument_)
    : DocumentEditorComponent (document),
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

ComponentEditorCanvas* ComponentEditor::getCanvas() const
{
    return dynamic_cast <ComponentEditorCanvas*> (getViewport()->getViewedComponent());
}

Viewport* ComponentEditor::getViewport() const
{
    return layoutEditorHolder->getViewport();
}

//==============================================================================
class TestComponent     : public ComponentEditorCanvas::ComponentHolder
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
    }

    void resized()
    {
        document.getCanvasWidth() = getWidth();
        document.getCanvasHeight() = getHeight();

        ComponentEditorCanvas::ComponentHolder::resized();
        updateComponents (document, selected);
    }

private:
    ComponentDocument document;
    ComponentEditorCanvas::SelectedItems selected;
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
        break;

    case CommandIDs::showOrHideTree:
        result.setInfo ("Show/Hide Properties", "Shows or hides the component properties panel", CommandCategories::editing, 0);
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
        getCanvas()->selectionToFront();
        return true;

    case CommandIDs::toBack:
        getCanvas()->selectionToBack();
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

    case StandardApplicationCommandIDs::del:
        getCanvas()->deleteSelection();
        return true;

    default:
        break;
    }

    return DocumentEditorComponent::perform (info);
}
