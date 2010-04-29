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
#include "../jucer_JucerTreeViewBase.h"


//==============================================================================
namespace ComponentTree
{
    //==============================================================================
    class Base  : public JucerTreeViewBase,
                  public ValueTree::Listener,
                  public ChangeListener
    {
    public:
        Base (ComponentEditor& editor_)
            : editor (editor_)
        {
            editor.getCanvas()->getSelection().addChangeListener (this);
        }

        ~Base()
        {
            editor.getCanvas()->getSelection().removeChangeListener (this);
        }

        //==============================================================================
        void valueTreePropertyChanged (ValueTree& tree, const var::identifier& property)  {}
        void valueTreeParentChanged (ValueTree& tree)       {}
        void valueTreeChildrenChanged (ValueTree& tree)     {}

        const String getUniqueName() const
        {
            jassert (getItemId().isNotEmpty());
            return getItemId();
        }

        //==============================================================================
        void itemOpennessChanged (bool isNowOpen)
        {
            if (isNowOpen)
                refreshSubItems();
        }

        virtual void refreshSubItems() = 0;
        virtual const String getItemId() const = 0;

        void setName (const String& newName)            {}

        void itemClicked (const MouseEvent& e)          {}
        void itemDoubleClicked (const MouseEvent& e)    {}

        void itemSelectionChanged (bool isNowSelected)
        {
            if (isNowSelected)
                editor.getCanvas()->getSelection().addToSelection (getItemId());
            else
                editor.getCanvas()->getSelection().deselect (getItemId());
        }

        void changeListenerCallback (void*)             { updateSelectionState(); }

        void updateSelectionState()
        {
            setSelected (editor.getCanvas()->getSelection().isSelected (getItemId()), false);
        }

        bool isMissing()                            { return false; }
        const String getTooltip()                   { return String::empty; }

    protected:
        //==============================================================================
        ComponentEditor& editor;
    };


    //==============================================================================
    class ComponentItem  : public Base
    {
    public:
        ComponentItem (ComponentEditor& editor_, const ValueTree& componentState_)
            : Base (editor_), componentState (componentState_)
        {
            componentState.addListener (this);
            updateSelectionState();
        }

        ~ComponentItem()
        {
            componentState.removeListener (this);
        }

        //==============================================================================
        const String getItemId() const              { return componentState [ComponentDocument::idProperty]; }

        bool mightContainSubItems()                 { return false; }
        void refreshSubItems()                      {}

        const String getDisplayName() const         { return getRenamingName(); }
        const String getRenamingName() const        { return componentState [ComponentDocument::memberNameProperty]; }

        Image* getIcon() const                      { return LookAndFeel::getDefaultLookAndFeel().getDefaultDocumentFileImage(); }

        const String getDragSourceDescription()     { return componentItemDragType; }

        void valueTreePropertyChanged (ValueTree& tree, const var::identifier& property)
        {
            if (property == ComponentDocument::memberNameProperty)
                repaintItem();
        }

    private:
        //==============================================================================
        ValueTree componentState;
    };

    //==============================================================================
    class ComponentList  : public Base
    {
    public:
        ComponentList (ComponentEditor& editor_)
            : Base (editor_), componentTree (editor.getDocument().getComponentGroup())
        {
            componentTree.addListener (this);
        }

        ~ComponentList()
        {
            componentTree.removeListener (this);
        }

        //==============================================================================
        const String getItemId() const              { return "components"; }
        bool mightContainSubItems()                 { return true; }

        void valueTreeChildrenChanged (ValueTree& tree)
        {
            if (tree == componentTree)
                refreshSubItems();
        }

        void refreshSubItems()
        {
            ScopedPointer <XmlElement> openness (getOpennessState());
            clearSubItems();

            ComponentDocument& doc = editor.getDocument();

            const int num = doc.getNumComponents();
            for (int i = 0; i < num; ++i)
                addSubItem (new ComponentItem (editor, doc.getComponent (i)));

            if (openness != 0)
                restoreOpennessState (*openness);
        }

        const String getDisplayName() const         { return getRenamingName(); }
        const String getRenamingName() const        { return "Components"; }
        Image* getIcon() const                      { return LookAndFeel::getDefaultLookAndFeel().getDefaultFolderImage(); }
        const String getDragSourceDescription()     { return String::empty; }

    private:
        ValueTree componentTree;
    };


    //==============================================================================
    class MarkerItem  : public Base
    {
    public:
        MarkerItem (ComponentEditor& editor_, const ValueTree& markerState_)
            : Base (editor_), markerState (markerState_)
        {
            markerState.addListener (this);
            updateSelectionState();
        }

        ~MarkerItem()
        {
            markerState.removeListener (this);
        }

        //==============================================================================
        const String getItemId() const              { return markerState [ComponentDocument::idProperty]; }

        bool mightContainSubItems()                 { return false; }
        void refreshSubItems()                      {}

        const String getDisplayName() const         { return getRenamingName(); }
        const String getRenamingName() const        { return markerState [ComponentDocument::markerNameProperty]; }

        Image* getIcon() const                      { return LookAndFeel::getDefaultLookAndFeel().getDefaultDocumentFileImage(); }

        const String getDragSourceDescription()     { return componentItemDragType; }

        void valueTreePropertyChanged (ValueTree& tree, const var::identifier& property)
        {
            if (property == ComponentDocument::markerNameProperty)
                repaintItem();
        }

    private:
        //==============================================================================
        ValueTree markerState;
    };

    //==============================================================================
    class MarkerList  : public Base
    {
    public:
        MarkerList (ComponentEditor& editor_, bool isX_)
            : Base (editor_), markerList (editor_.getDocument().getMarkerList (isX_).getGroup()), isX (isX_)
        {
            markerList.addListener (this);
        }

        ~MarkerList()
        {
            markerList.removeListener (this);
        }

        //==============================================================================
        const String getItemId() const              { return isX ? "markersX" : "markersY"; }
        bool mightContainSubItems()                 { return true; }

        void valueTreeChildrenChanged (ValueTree& tree)
        {
            refreshSubItems();
        }

        void refreshSubItems()
        {
            ScopedPointer <XmlElement> openness (getOpennessState());
            clearSubItems();

            ComponentDocument::MarkerList& markers = editor.getDocument().getMarkerList (isX);

            const int num = markers.size();
            for (int i = 0; i < num; ++i)
                addSubItem (new MarkerItem (editor, markers.getMarker (i)));

            if (openness != 0)
                restoreOpennessState (*openness);
        }

        const String getDisplayName() const         { return getRenamingName(); }
        const String getRenamingName() const        { return isX ? "Markers (X-axis)" : "Markers (Y-axis)"; }
        Image* getIcon() const                      { return LookAndFeel::getDefaultLookAndFeel().getDefaultFolderImage(); }
        const String getDragSourceDescription()     { return String::empty; }

    private:
        ValueTree markerList;
        bool isX;
    };

    //==============================================================================
    class Root  : public Base
    {
    public:
        Root (ComponentEditor& editor_)  : Base (editor_)  {}
        ~Root()    {}

        //==============================================================================
        const String getItemId() const              { return "root"; }
        bool mightContainSubItems()                 { return true; }

        void refreshSubItems()
        {
            ScopedPointer <XmlElement> openness (getOpennessState());
            clearSubItems();

            addSubItem (new ComponentList (editor));
            addSubItem (new MarkerList (editor, true));
            addSubItem (new MarkerList (editor, false));

            if (openness != 0)
                restoreOpennessState (*openness);
        }

        const String getDisplayName() const         { return getRenamingName(); }
        const String getRenamingName() const        { return editor.getDocument().getClassName().toString(); }
        Image* getIcon() const                      { return LookAndFeel::getDefaultLookAndFeel().getDefaultFolderImage(); }
        const String getDragSourceDescription()     { return String::empty; }
    };
}

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
class ComponentEditor::LayoutEditorHolder  : public Component,
                                             public ToolbarItemFactory
{
public:
    LayoutEditorHolder (ComponentEditor& editor_)
        : editor (editor_), infoPanel (0), tree (0)
    {
        addAndMakeVisible (toolbar = new Toolbar());
        toolbar->addDefaultItems (*this);
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
        tree->setRootItem (new ComponentTree::Root (editor));
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

    //==============================================================================
    enum DemoToolbarItemIds
    {
        createComponent     = 1,
        showInfo            = 2,
        showComponentTree   = 3,
    };

    void getAllToolbarItemIds (Array <int>& ids)
    {
        ids.add (createComponent);
        ids.add (showInfo);
        ids.add (showComponentTree);

        ids.add (separatorBarId);
        ids.add (spacerId);
        ids.add (flexibleSpacerId);
    }

    void getDefaultItemSet (Array <int>& ids)
    {
        ids.add (createComponent);
        ids.add (flexibleSpacerId);
        ids.add (showInfo);
        ids.add (showComponentTree);
    }

    ToolbarItemComponent* createItem (int itemId)
    {
        String name;
        int commandId = 0;

        switch (itemId)
        {
            case createComponent:   name = "new"; break;
            case showInfo:          name = "info"; commandId = CommandIDs::showOrHideProperties; break;
            case showComponentTree: name = "tree"; commandId = CommandIDs::showOrHideTree; break;
            default:                jassertfalse; return 0;
        }

        ToolbarButton* b = new ToolbarButton (itemId, name, new DrawablePath(), 0);
        b->setCommandToTrigger (commandManager, commandId, true);
        return b;
    }

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
class ComponentEditor::CodeEditorHolder  : public Component,
                                           public ButtonListener
{
public:
    CodeEditorHolder (ComponentEditor& editor_)
        : editor (editor_), switchFileButton (String::empty), showingHeader (true)
    {
        addAndMakeVisible (&viewport);
        viewport.setScrollBarsShown (true, false);

        addAndMakeVisible (&switchFileButton);

        buttonClicked (0);
        switchFileButton.addButtonListener (this);
    }

    ~CodeEditorHolder()
    {
    }

    void resized()
    {
        viewport.setBounds (getLocalBounds());

        int visWidth = viewport.getMaximumVisibleWidth();
        dynamic_cast <ContentHolder*> (viewport.getViewedComponent())->updateSize (visWidth);

        if (viewport.getMaximumVisibleWidth() != visWidth)
            dynamic_cast <ContentHolder*> (viewport.getViewedComponent())->updateSize (viewport.getMaximumVisibleWidth());

        switchFileButton.setBounds (getWidth() - 150, 4, 120, 20);
    }

    void buttonClicked (Button*)
    {
        showingHeader = ! showingHeader;
        viewport.setViewedComponent (new ContentHolder (editor.getDocument(), showingHeader));
        resized();
        switchFileButton.setButtonText (showingHeader ? "Show CPP file" : "Show header file");
    }

private:
    enum { updateCommandId = 0x23427fa1 };

    //==============================================================================
    class EditorHolder  : public Component,
                          public CodeDocument::Listener
    {
    public:
        EditorHolder (const CodeGenerator::CustomCodeList::CodeDocumentRef::Ptr doc,
                      const String& textBefore, const String& textAfter)
            : document (doc), cppTokeniser(), codeEditor (doc->getDocument(), &cppTokeniser)
        {
            linesBefore.addLines (textBefore);
            linesAfter.addLines (textAfter);

            addAndMakeVisible (&codeEditor);
            doc->getDocument().addListener (this);
        }

        ~EditorHolder()
        {
            document->getDocument().removeListener (this);
        }

        void paint (Graphics& g)
        {
            g.setFont (codeEditor.getFont());
            g.setColour (Colours::darkgrey);

            const int fontHeight = codeEditor.getLineHeight();
            const int fontAscent = (int) codeEditor.getFont().getAscent();
            const int textX = 5;

            int i;
            for (i = 0; i < linesBefore.size(); ++i)
                g.drawSingleLineText (linesBefore[i], textX, i * fontHeight + fontAscent);

            for (i = 0; i < linesAfter.size(); ++i)
                g.drawSingleLineText (linesAfter[i], textX, codeEditor.getBottom() + i * fontHeight + fontAscent);
        }

        void updateSize (int width)
        {
            const int fontHeight = codeEditor.getLineHeight();

            codeEditor.setBounds (0, fontHeight * linesBefore.size() + 1,
                                  width, 2 + codeEditor.getScrollbarThickness()
                                           + fontHeight * jlimit (1, 50, document->getDocument().getNumLines()));

            setSize (width, (linesBefore.size() + linesAfter.size()) * fontHeight + codeEditor.getHeight());
        }

        void codeDocumentChanged (const CodeDocument::Position&, const CodeDocument::Position&)
        {
            int oldHeight = getHeight();
            updateSize (getWidth());
            if (getHeight() != oldHeight)
                getParentComponent()->handleCommandMessage (updateCommandId);
        }

    private:
        const CodeGenerator::CustomCodeList::CodeDocumentRef::Ptr document;
        CPlusPlusCodeTokeniser cppTokeniser;
        CodeEditorComponent codeEditor;
        StringArray linesBefore, linesAfter;
    };

    //==============================================================================
    class ContentHolder  : public Component,
                           public ChangeListener
    {
    public:
        ContentHolder (ComponentDocument& document_, bool isHeader_)
            : document (document_), isHeader (isHeader_)
        {
            setOpaque (true);
            document.getCustomCodeList().addChangeListener (this);
            changeListenerCallback (0);
        }

        ~ContentHolder()
        {
            document.getCustomCodeList().removeChangeListener (this);
        }

        void paint (Graphics& g)
        {
            g.fillAll (Colours::lightgrey);
        }

        void updateSize (int width)
        {
            int y = 2;
            for (int i = 0; i < editors.size(); ++i)
            {
                EditorHolder* const ed = editors.getUnchecked(i);
                ed->updateSize (width - 8);
                ed->setTopLeftPosition (4, y + 1);
                y = ed->getBottom() + 1;
            }

            setSize (width, y + 2);
        }

        void changeListenerCallback (void*)
        {
            editors.clear();

            CodeGenerator::CustomCodeList::Iterator iter (isHeader ? document.getHeaderContent()
                                                                   : document.getCppContent(),
                                                          document.getCustomCodeList());

            while (iter.next())
            {
                EditorHolder* ed = new EditorHolder (iter.codeDocument, iter.textBefore, iter.textAfter);
                editors.add (ed);
                addAndMakeVisible (ed);
            }

            updateSize (getWidth());
        }

        void handleCommandMessage (int commandId)
        {
            if (commandId == updateCommandId)
                updateSize (getWidth());
            else
                Component::handleCommandMessage (commandId);
        }

    private:
        OwnedArray <EditorHolder> editors;
        ComponentDocument& document;
        bool isHeader;
    };

    ComponentEditor& editor;
    Viewport viewport;
    TextButton switchFileButton;
    bool showingHeader;
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
