/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../jucer_Headers.h"
#include "jucer_JucerDocumentHolder.h"
#include "jucer_TestComponent.h"
#include "jucer_MainWindow.h"
#include "../model/jucer_ObjectTypes.h"
#include "jucer_ComponentLayoutPanel.h"
#include "jucer_PaintRoutinePanel.h"
#include "jucer_ResourceEditorPanel.h"
#include "../properties/jucer_ComponentTextProperty.h"
#include "../properties/jucer_ComponentChoiceProperty.h"


//==============================================================================
class ExtraMethodsList  : public PropertyComponent,
                          public ListBoxModel,
                          public ChangeListener
{
public:
    ExtraMethodsList (JucerDocument& doc)
        : PropertyComponent (T("extra callbacks"), 250),
          document (doc)
    {
        addAndMakeVisible (listBox = new ListBox (String::empty, this));
        listBox->setRowHeight (22);

        document.addChangeListener (this);
    }

    ~ExtraMethodsList()
    {
        document.removeChangeListener (this);
        deleteAllChildren();
    }

    int getNumRows()
    {
        return methods.size();
    }

    void paintListBoxItem (int row, Graphics& g, int width, int height, bool rowIsSelected)
    {
        if (row < 0 || row >= getNumRows())
            return;

        if (rowIsSelected)
            g.fillAll (findColour (TextEditor::highlightColourId));

        g.setColour (Colours::black);
        g.setFont (height * 0.6f);
        g.drawText (returnValues [row] + T(" ") + baseClasses [row] + T("::") + methods [row],
                    30, 0, width - 32, height,
                    Justification::centredLeft, true);

        getLookAndFeel().drawTickBox (g, *this, 6, 2, 18, 18, document.isOptionalMethodEnabled (methods [row]), true, false, false);
    }

    void listBoxItemClicked (int row, const MouseEvent& e)
    {
        if (row < 0 || row >= getNumRows())
            return;

        if (e.x < 30)
            document.setOptionalMethodEnabled (methods [row],
                                               ! document.isOptionalMethodEnabled (methods [row]));
    }

    void paint (Graphics& g)
    {
        g.fillAll (Colours::white);
    }

    void resized()
    {
        listBox->setBounds (0, 0, getWidth(), getHeight());
    }

    void refresh()
    {
        baseClasses.clear();
        returnValues.clear();
        methods.clear();
        initialContents.clear();

        document.getOptionalMethods (baseClasses, returnValues, methods, initialContents);

        listBox->updateContent();
        listBox->repaint();
    }

    void changeListenerCallback (void*)
    {
        refresh();
    }

private:
    JucerDocument& document;
    ListBox* listBox;

    StringArray baseClasses;
    StringArray returnValues;
    StringArray methods;
    StringArray initialContents;
};


//==============================================================================
class ClassPropertiesPanel  : public Component,
                              private ChangeListener
{
public:
    ClassPropertiesPanel (JucerDocument& document_)
        : document (document_)
    {
        addAndMakeVisible (panel1 = new PropertyPanel());
        addAndMakeVisible (panel2 = new PropertyPanel());

        Array <PropertyComponent*> props;
        props.add (new ComponentClassNameProperty (document_));
        props.add (new ComponentCompNameProperty (document_));
        props.add (new ComponentParentClassesProperty (document_));
        props.add (new ComponentConstructorParamsProperty (document_));
        props.add (new ComponentInitialisersProperty (document_));
        props.add (new ComponentInitialSizeProperty (document_, true));
        props.add (new ComponentInitialSizeProperty (document_, false));
        props.add (new FixedSizeProperty (document_));

        panel1->addSection (T("General class settings"), props);

        Array <PropertyComponent*> props2;
        props2.add (new ExtraMethodsList (document_));
        panel2->addSection (T("Extra callback methods to generate"), props2);

        document_.addExtraClassProperties (panel1);

        document_.addChangeListener (this);
    }

    ~ClassPropertiesPanel()
    {
        document.removeChangeListener (this);
        deleteAllChildren();
    }

    void resized()
    {
        int pw = jmin (getWidth() / 2 - 20, 350);
        panel1->setBounds (10, 6, pw, getHeight() - 12);
        panel2->setBounds (panel1->getRight() + 20, panel1->getY(), pw, panel1->getHeight());
    }

    void changeListenerCallback (void*)
    {
        panel1->refreshAll();
        panel2->refreshAll();
    }

private:
    JucerDocument& document;

    PropertyPanel* panel1;
    PropertyPanel* panel2;

    //==============================================================================
    class ComponentClassNameProperty    : public ComponentTextProperty <Component>
    {
    public:
        ComponentClassNameProperty (JucerDocument& document_)
            : ComponentTextProperty <Component> (T("class name"), 128, false, 0, document_)
        {}

        void setText (const String& newText)    { document.setClassName (newText); }
        const String getText() const            { return document.getClassName(); }
    };

    //==============================================================================
    class ComponentCompNameProperty    : public ComponentTextProperty <Component>
    {
    public:
        ComponentCompNameProperty (JucerDocument& document_)
            : ComponentTextProperty <Component> (T("component name"), 200, false, 0, document_)
        {}

        void setText (const String& newText)    { document.setComponentName (newText); }
        const String getText() const            { return document.getComponentName(); }
    };

    //==============================================================================
    class ComponentParentClassesProperty    : public ComponentTextProperty <Component>
    {
    public:
        ComponentParentClassesProperty (JucerDocument& document_)
            : ComponentTextProperty <Component> (T("parent classes"), 512, false, 0, document_)
        {}

        void setText (const String& newText)    { document.setParentClasses (newText); }
        const String getText() const            { return document.getParentClassString(); }
    };

    //==============================================================================
    class ComponentConstructorParamsProperty    : public ComponentTextProperty <Component>
    {
    public:
        ComponentConstructorParamsProperty (JucerDocument& document_)
            : ComponentTextProperty <Component> (T("constructor params"), 2048, false, 0, document_)
        {}

        void setText (const String& newText)    { document.setConstructorParams (newText); }
        const String getText() const            { return document.getConstructorParams(); }
    };

    //==============================================================================
    class ComponentInitialisersProperty   : public ComponentTextProperty <Component>
    {
    public:
        ComponentInitialisersProperty (JucerDocument& document_)
            : ComponentTextProperty <Component> (T("member intialisers"), 2048, true, 0, document_)
        {
            preferredHeight = 24 * 3;
        }

        void setText (const String& newText)    { document.setVariableInitialisers (newText); }
        const String getText() const            { return document.getVariableInitialisers(); }
    };


    //==============================================================================
    class ComponentInitialSizeProperty    : public ComponentTextProperty <Component>
    {
    public:
        ComponentInitialSizeProperty (JucerDocument& document_, const bool isWidth_)
            : ComponentTextProperty <Component> (isWidth_ ? T("initial width") : T("initial height"),
                                     10, false, 0, document_),
              isWidth (isWidth_)
        {}

        void setText (const String& newText)
        {
            if (isWidth)
                document.setInitialSize  (newText.getIntValue(), document.getInitialHeight());
            else
                document.setInitialSize  (document.getInitialWidth(), newText.getIntValue());
        }

        const String getText() const
        {
            return String (isWidth ? document.getInitialWidth()
                                   : document.getInitialHeight());
        }

    private:
        const bool isWidth;
    };

    //==============================================================================
    class FixedSizeProperty    : public ComponentChoiceProperty <Component>
    {
    public:
        FixedSizeProperty (JucerDocument& document_)
            : ComponentChoiceProperty <Component> (T("fixed size"), 0, document_)
        {
            choices.add (T("Resize component to fit workspace"));
            choices.add (T("Keep component size fixed"));
        }

        void setIndex (const int newIndex)  { document.setFixedSize (newIndex != 0); }
        int getIndex() const                { return document.isFixedSize() ? 1 : 0; }
    };
};

//==============================================================================
class CodeViewerComp    : public Component,
                          private ButtonListener
{
public:
    //==============================================================================
    CodeViewerComp (JucerDocument& document_)
        : document (document_),
          isHeader (showHeaderFileFirst)
    {
        addAndMakeVisible (editor = new CodeEditorComponent (codeDocument, &tokeniser));

        addAndMakeVisible (switchButton = new TextButton (String::empty));
        switchButton->addButtonListener (this);

        setWantsKeyboardFocus (true);
    }

    ~CodeViewerComp()
    {
        showHeaderFileFirst = isHeader;
        deleteAllChildren();
    }

    void showFile (const bool isHeader_)
    {
        isHeader = isHeader_;
        editor->loadContent (isHeader ? h : cpp);
        switchButton->setButtonText (isHeader ? T("Show .cpp") : T("Show .h"));
    }

    void resized()
    {
        editor->setBounds (4, 4, getWidth() - 8, getHeight() - 8);
        switchButton->setBounds (getWidth() - 130, 10, 90, 22);
    }

    void buttonClicked (Button*)
    {
        showFile (! isHeader);
    }

    void visibilityChanged()
    {
        if (isVisible())
        {
            document.getPreviewFiles (h, cpp);
            showFile (isHeader);
        }
    }

    //==============================================================================
private:
    JucerDocument& document;
    String h, cpp;
    bool isHeader;
    CodeDocument codeDocument;
    CPlusPlusCodeTokeniser tokeniser;
    CodeEditorComponent* editor;
    TextButton* switchButton;

    static bool showHeaderFileFirst;
};

bool CodeViewerComp::showHeaderFileFirst = false;

static const Colour tabColour (Colour (0xffc4cdcd));


//==============================================================================
JucerDocumentHolder::JucerDocumentHolder (JucerDocument* const document_)
    : document (document_),
      tabbedComponent (0),
      compLayoutPanel (0),
      lastViewportX (0),
      lastViewportY (0),
      currentZoomLevel (1.0)
{
    jassert (document != 0);

    setOpaque (true);

    setSize (document->getInitialWidth(),
             document->getInitialHeight());

    addAndMakeVisible (tabbedComponent = new TabbedComponent (TabbedButtonBar::TabsAtRight));
    tabbedComponent->setOutline (0);

    tabbedComponent->addTab (T("Class"), tabColour, new ClassPropertiesPanel (*document), true);

    if (document->getComponentLayout() != 0)
    {
        tabbedComponent->addTab (T("Subcomponents"), tabColour,
                                 compLayoutPanel = new ComponentLayoutPanel (*document, *document->getComponentLayout()), true);
    }

    tabbedComponent->addTab (T("Resources"), tabColour, new ResourceEditorPanel (*document), true);
    tabbedComponent->addTab (T("Code Preview"), tabColour, new CodeViewerComp (*document), true);

    updateTabs();

    tabbedComponent->setCurrentTabIndex (0);

    document->addChangeListener (this);

    resized();
    refreshPropertiesPanel();

    changeListenerCallback (0);
}

JucerDocumentHolder::~JucerDocumentHolder()
{
    tabbedComponent->clearTabs();
    deleteAndZero (tabbedComponent);
    delete document;
}

bool JucerDocumentHolder::close()
{
    MainWindow* const mw = findParentComponentOfClass ((MainWindow*) 0);

    if (mw != 0)
        return mw->closeDocument (this);

    jassertfalse
    return false;
}

void JucerDocumentHolder::refreshPropertiesPanel() const
{
    if (tabbedComponent != 0)
    {
        for (int i = tabbedComponent->getNumTabs(); --i >= 0;)
        {
            ComponentLayoutPanel* layoutPanel = dynamic_cast <ComponentLayoutPanel*> (tabbedComponent->getTabContentComponent (i));

            if (layoutPanel != 0)
            {
                if (layoutPanel->isVisible())
                    layoutPanel->updatePropertiesList();
            }
            else
            {
                PaintRoutinePanel* pr = dynamic_cast <PaintRoutinePanel*> (tabbedComponent->getTabContentComponent (i));

                if (pr != 0 && pr->isVisible())
                    pr->updatePropertiesList();
            }
        }
    }
}

void JucerDocumentHolder::updateTabs()
{
    const StringArray paintRoutineNames (document->getPaintRoutineNames());
    int i;

    for (i = tabbedComponent->getNumTabs(); --i >= 0;)
    {
        if (dynamic_cast <PaintRoutinePanel*> (tabbedComponent->getTabContentComponent (i)) != 0
             && ! paintRoutineNames.contains (tabbedComponent->getTabNames() [i]))
        {
            tabbedComponent->removeTab (i);
        }
    }

    for (i = 0; i < document->getNumPaintRoutines(); ++i)
    {
        if (! tabbedComponent->getTabNames().contains (paintRoutineNames [i]))
        {
            int index, numPaintRoutinesSeen = 0;
            for (index = 1; index < tabbedComponent->getNumTabs(); ++index)
            {
                if (dynamic_cast <PaintRoutinePanel*> (tabbedComponent->getTabContentComponent (index)) != 0)
                {
                    if (++numPaintRoutinesSeen == i)
                    {
                        ++index;
                        break;
                    }
                }
            }

            if (numPaintRoutinesSeen == 0)
                index = document->getComponentLayout() != 0 ? 2 : 1;

            tabbedComponent->addTab (paintRoutineNames[i], tabColour,
                                     new PaintRoutinePanel (*document,
                                                            *document->getPaintRoutine (i),
                                                            this), true, index);
        }
    }
}

//==============================================================================
void JucerDocumentHolder::paint (Graphics& g)
{
    g.fillAll (Colours::lightgrey);

    if (tabbedComponent == 0)
    {
        g.setColour (Colours::black);
        g.drawText (T("no component currently open"), 0, 0, getWidth(), getHeight(), Justification::centred, false);
    }
}

void JucerDocumentHolder::resized()
{
    if (tabbedComponent != 0)
        tabbedComponent->setBounds (0, 0, getWidth(), getHeight());
}

void JucerDocumentHolder::changeListenerCallback (void*)
{
    setName (document->getClassName());
    updateTabs();
}

//==============================================================================
ApplicationCommandTarget* JucerDocumentHolder::getNextCommandTarget()
{
    return findFirstTargetParentComponent();
}

ComponentLayout* JucerDocumentHolder::getCurrentLayout() const
{
    if (tabbedComponent != 0)
    {
        ComponentLayoutPanel* panel = dynamic_cast <ComponentLayoutPanel*> (tabbedComponent->getCurrentContentComponent());

        if (panel != 0)
            return &(panel->getLayout());
    }

    return 0;
}

PaintRoutine* JucerDocumentHolder::getCurrentPaintRoutine() const
{
    if (tabbedComponent != 0)
    {
        PaintRoutinePanel* panel = dynamic_cast <PaintRoutinePanel*> (tabbedComponent->getCurrentContentComponent());

        if (panel != 0)
            return &(panel->getPaintRoutine());
    }

    return 0;
}

void JucerDocumentHolder::showLayout()
{
    if (getCurrentLayout() == 0 && tabbedComponent != 0)
    {
        for (int i = 0; i < tabbedComponent->getNumTabs(); ++i)
        {
            if (dynamic_cast <ComponentLayoutPanel*> (tabbedComponent->getTabContentComponent (i)) != 0)
            {
                tabbedComponent->setCurrentTabIndex (i);
                break;
            }
        }
    }
}

void JucerDocumentHolder::showGraphics (PaintRoutine* routine)
{
    if ((getCurrentPaintRoutine() != routine || routine == 0) && tabbedComponent != 0)
    {
        for (int i = 0; i < tabbedComponent->getNumTabs(); ++i)
        {
            PaintRoutinePanel* pr = dynamic_cast <PaintRoutinePanel*> (tabbedComponent->getTabContentComponent (i));

            if (pr != 0 && (routine == &(pr->getPaintRoutine()) || routine == 0))
            {
                tabbedComponent->setCurrentTabIndex (i);
                break;
            }
        }
    }
}

//==============================================================================
void JucerDocumentHolder::setViewportToLastPos (Viewport* vp)
{
    vp->setViewPosition (lastViewportX, lastViewportY);

    MagnifierComponent* magnifier = dynamic_cast <MagnifierComponent*> (vp->getViewedComponent());
    magnifier->setScaleFactor (currentZoomLevel);
}

void JucerDocumentHolder::storeLastViewportPos (Viewport* vp)
{
    lastViewportX = vp->getViewPositionX();
    lastViewportY = vp->getViewPositionY();

    MagnifierComponent* magnifier = dynamic_cast <MagnifierComponent*> (vp->getViewedComponent());
    currentZoomLevel = magnifier->getScaleFactor();
}

void JucerDocumentHolder::setZoom (double scale)
{
    scale = jlimit (1.0 / 4.0, 32.0, scale);

    if (tabbedComponent != 0)
    {
        EditingPanelBase* panel = dynamic_cast <EditingPanelBase*> (tabbedComponent->getCurrentContentComponent());

        if (panel != 0)
            panel->setZoom (scale);
    }
}

double JucerDocumentHolder::getZoom() const
{
    if (tabbedComponent != 0)
    {
        EditingPanelBase* panel = dynamic_cast <EditingPanelBase*> (tabbedComponent->getCurrentContentComponent());

        if (panel != 0)
            return panel->getZoom();
    }

    return 1.0;
}

void JucerDocumentHolder::addElement (const int index)
{
    if (tabbedComponent != 0)
    {
        PaintRoutinePanel* const panel = dynamic_cast <PaintRoutinePanel*> (tabbedComponent->getCurrentContentComponent());

        if (panel != 0)
        {
            PaintRoutine* const currentPaintRoutine = & (panel->getPaintRoutine());
            const Rectangle area (panel->getComponentArea());

            document->getUndoManager().beginNewTransaction();

            PaintElement* e = ObjectTypes::createNewElement (index, currentPaintRoutine);

            e->setInitialBounds (area.getWidth(), area.getHeight());

            e = currentPaintRoutine->addNewElement (e, -1, true);

            if (e != 0)
            {
                const int randomness = jmin (80, area.getWidth() / 2, area.getHeight() / 2);
                int x = area.getX() + area.getWidth() / 2 + Random::getSystemRandom().nextInt (randomness) - randomness / 2;
                int y = area.getY() + area.getHeight() / 2 + Random::getSystemRandom().nextInt (randomness) - randomness / 2;
                x = document->snapPosition (x);
                y = document->snapPosition (y);

                panel->xyToTargetXY (x, y);

                Rectangle r (e->getCurrentBounds (area));
                r.setPosition (x, y);
                e->setCurrentBounds (r, area, true);

                currentPaintRoutine->getSelectedElements().selectOnly (e);
            }

            document->getUndoManager().beginNewTransaction();
        }
    }
}

void JucerDocumentHolder::addComponent (const int index)
{
    if (tabbedComponent != 0)
    {
        showLayout();
        ComponentLayoutPanel* const panel = dynamic_cast <ComponentLayoutPanel*> (tabbedComponent->getCurrentContentComponent());

        if (panel != 0)
        {
            const Rectangle area (panel->getComponentArea());

            document->getUndoManager().beginNewTransaction (T("Add new ") + ObjectTypes::componentTypeHandlers [index]->getTypeName());

            const int randomness = jmin (80, area.getWidth() / 2, area.getHeight() / 2);
            int x = area.getWidth() / 2 + Random::getSystemRandom().nextInt (randomness) - randomness / 2;
            int y = area.getHeight() / 2 + Random::getSystemRandom().nextInt (randomness) - randomness / 2;
            x = document->snapPosition (x);
            y = document->snapPosition (y);

            panel->xyToTargetXY (x, y);

            Component* newOne = panel->getLayout().addNewComponent (ObjectTypes::componentTypeHandlers [index], x, y);

            if (newOne != 0)
                panel->getLayout().getSelectedSet().selectOnly (newOne);

            document->getUndoManager().beginNewTransaction();
        }
    }
}

//==============================================================================
bool JucerDocumentHolder::isSomethingSelected() const
{
    ComponentLayout* layout = getCurrentLayout();

    if (layout != 0)
        return layout->getSelectedSet().getNumSelected() > 0;

    PaintRoutine* routine = getCurrentPaintRoutine();

    if (routine != 0)
        return routine->getSelectedElements().getNumSelected() > 0;

    return false;
}

//==============================================================================
void JucerDocumentHolder::getAllCommands (Array <CommandID>& commands)
{
    const CommandID ids[] =
    {
        CommandIDs::close,
        CommandIDs::save,
        CommandIDs::saveAs,
        CommandIDs::undo,
        CommandIDs::redo,
        CommandIDs::test,
        CommandIDs::toFront,
        CommandIDs::toBack,
        CommandIDs::group,
        CommandIDs::ungroup,
        CommandIDs::bringBackLostItems,
        CommandIDs::enableSnapToGrid,
        CommandIDs::showGrid,
        CommandIDs::editCompLayout,
        CommandIDs::editCompGraphics,
        CommandIDs::zoomIn,
        CommandIDs::zoomOut,
        CommandIDs::zoomNormal,
        CommandIDs::spaceBarDrag,
        CommandIDs::compOverlay0,
        CommandIDs::compOverlay33,
        CommandIDs::compOverlay66,
        CommandIDs::compOverlay100,
        StandardApplicationCommandIDs::cut,
        StandardApplicationCommandIDs::copy,
        StandardApplicationCommandIDs::paste,
        StandardApplicationCommandIDs::del,
        StandardApplicationCommandIDs::selectAll,
        StandardApplicationCommandIDs::deselectAll
    };

    commands.addArray (ids, numElementsInArray (ids));

    int i;
    for (i = 0; i < ObjectTypes::numComponentTypes; ++i)
        commands.add (CommandIDs::newComponentBase + i);

    for (i = 0; i < ObjectTypes::numElementTypes; ++i)
        commands.add (CommandIDs::newElementBase + i);
}

void JucerDocumentHolder::getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result)
{
    ComponentLayout* const currentLayout = getCurrentLayout();
    PaintRoutine* const currentPaintRoutine = getCurrentPaintRoutine();

    const int cmd = ModifierKeys::commandModifier;
    const int shift = ModifierKeys::shiftModifier;

    if (commandID >= CommandIDs::newComponentBase
         && commandID < CommandIDs::newComponentBase + ObjectTypes::numComponentTypes)
    {
        const int index = commandID - CommandIDs::newComponentBase;

        result.setInfo (T("New ") + ObjectTypes::componentTypeHandlers [index]->getTypeName(),
                        T("Creates a new ") + ObjectTypes::componentTypeHandlers [index]->getTypeName(),
                        CommandCategories::editing, 0);
        return;
    }

    if (commandID >= CommandIDs::newElementBase
         && commandID < CommandIDs::newElementBase + ObjectTypes::numElementTypes)
    {
        const int index = commandID - CommandIDs::newElementBase;

        result.setInfo (String (T("New ")) + ObjectTypes::elementTypeNames [index],
                        String (T("Adds a new ")) + ObjectTypes::elementTypeNames [index],
                        CommandCategories::editing, 0);

        result.setActive (currentPaintRoutine != 0);
        return;
    }

    switch (commandID)
    {
    case CommandIDs::close:
        result.setInfo (T("Close"),
                        T("Closes the component that's currently being edited."),
                        CommandCategories::general, 0);
        break;

    case CommandIDs::save:
        result.setInfo (T("Save"),
                        T("Saves the current component."),
                        CommandCategories::general, 0);

        result.defaultKeypresses.add (KeyPress (T('s'), cmd, 0));
        break;


    case CommandIDs::saveAs:
        result.setInfo (T("Save As..."),
                        T("Saves the current component to a specified file."),
                        CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress (T('s'), cmd | shift, 0));
        break;

    case CommandIDs::undo:
        result.setInfo (T("Undo"),
                        T("Undoes the last operation."),
                        CommandCategories::editing, 0);
        result.setActive (document->getUndoManager().canUndo());
        result.defaultKeypresses.add (KeyPress (T('z'), cmd, 0));
        break;

    case CommandIDs::redo:
        result.setInfo (T("Redo"),
                        T("Redoes the last operation."),
                        CommandCategories::editing, 0);
        result.setActive (document->getUndoManager().canRedo());
        result.defaultKeypresses.add (KeyPress (T('z'), cmd | shift, 0));
        result.defaultKeypresses.add (KeyPress (T('y'), cmd, 0));
        break;

    case CommandIDs::toFront:
        result.setInfo (T("Bring to front"),
                        T("Brings the currently selected component to the front."),
                        CommandCategories::editing, 0);
        result.setActive (isSomethingSelected());
        result.defaultKeypresses.add (KeyPress (T('f'), cmd, 0));
        break;

    case CommandIDs::toBack:
        result.setInfo (T("Send to back"),
                        T("Sends the currently selected component to the back."),
                        CommandCategories::editing, 0);
        result.setActive (isSomethingSelected());
        result.defaultKeypresses.add (KeyPress (T('b'), cmd, 0));
        break;

    case CommandIDs::group:
        result.setInfo (T("Group selected items"),
                        T("Turns the currently selected elements into a single group object."),
                        CommandCategories::editing, 0);
        result.setActive (currentPaintRoutine != 0
                           && currentPaintRoutine->getSelectedElements().getNumSelected() > 1);
        result.defaultKeypresses.add (KeyPress (T('k'), cmd, 0));
        break;

    case CommandIDs::ungroup:
        result.setInfo (T("Ungroup selected items"),
                        T("Turns the currently selected elements into a single group object."),
                        CommandCategories::editing, 0);
        result.setActive (currentPaintRoutine != 0
                           && currentPaintRoutine->getSelectedElements().getNumSelected() == 1
                           && currentPaintRoutine->getSelectedElements().getSelectedItem (0)->getTypeName() == T("Group"));
        result.defaultKeypresses.add (KeyPress (T('k'), cmd | shift, 0));
        break;

    case CommandIDs::test:
        result.setInfo (T("Test component..."),
                        T("Runs the current component interactively."),
                        CommandCategories::view, 0);
        result.defaultKeypresses.add (KeyPress (T('t'), cmd, 0));
        break;

    case CommandIDs::enableSnapToGrid:
        result.setInfo (T("Enable snap-to-grid"),
                        T("Toggles whether components' positions are aligned to a grid."),
                        CommandCategories::view, 0);
        result.setTicked (document->isSnapActive (false));
        result.defaultKeypresses.add (KeyPress (T('g'), cmd, 0));
        break;

    case CommandIDs::showGrid:
        result.setInfo (T("Show snap-to-grid"),
                        T("Toggles whether the snapping grid is displayed on-screen."),
                        CommandCategories::view, 0);
        result.setTicked (document->isSnapShown());
        result.defaultKeypresses.add (KeyPress (T('g'), cmd | shift, 0));
        break;

    case CommandIDs::editCompLayout:
        result.setInfo (T("Edit sub-component layout"),
                        T("Switches to the sub-component editor view."),
                        CommandCategories::view, 0);
        result.setActive (tabbedComponent != 0);
        result.setTicked (currentLayout != 0);
        result.defaultKeypresses.add (KeyPress (T('n'), cmd, 0));
        break;

    case CommandIDs::editCompGraphics:
        result.setInfo (T("Edit background graphics"),
                        T("Switches to the background graphics editor view."),
                        CommandCategories::view, 0);
        result.setActive (tabbedComponent != 0);
        result.setTicked (currentPaintRoutine != 0);
        result.defaultKeypresses.add (KeyPress (T('m'), cmd, 0));
        break;

    case CommandIDs::bringBackLostItems:
        result.setInfo (T("Retrieve offscreen items"),
                        T("Moves any items that are lost beyond the edges of the screen back to the centre."),
                        CommandCategories::editing, 0);
        result.setActive (currentPaintRoutine != 0 || currentLayout != 0);
        result.defaultKeypresses.add (KeyPress (T('m'), cmd, 0));
        break;

    case CommandIDs::zoomIn:
        result.setInfo (T("Zoom in"),
                        T("Zooms in on the current component."),
                        CommandCategories::editing, 0);
        result.setActive (currentPaintRoutine != 0 || currentLayout != 0);
        result.defaultKeypresses.add (KeyPress (T(']'), cmd, 0));
        break;

    case CommandIDs::zoomOut:
        result.setInfo (T("Zoom out"),
                        T("Zooms out on the current component."),
                        CommandCategories::editing, 0);
        result.setActive (currentPaintRoutine != 0 || currentLayout != 0);
        result.defaultKeypresses.add (KeyPress (T('['), cmd, 0));
        break;

    case CommandIDs::zoomNormal:
        result.setInfo (T("Zoom to 100%"),
                        T("Restores the zoom level to normal."),
                        CommandCategories::editing, 0);
        result.setActive (currentPaintRoutine != 0 || currentLayout != 0);
        result.defaultKeypresses.add (KeyPress (T('1'), cmd, 0));
        break;

    case CommandIDs::spaceBarDrag:
        result.setInfo (T("Scroll while dragging mouse"),
                        T("When held down, this key lets you scroll around by dragging with the mouse."),
                        CommandCategories::view, ApplicationCommandInfo::wantsKeyUpDownCallbacks);
        result.setActive (currentPaintRoutine != 0 || currentLayout != 0);
        result.defaultKeypresses.add (KeyPress (KeyPress::spaceKey, 0, 0));
        break;

    case CommandIDs::compOverlay0:
    case CommandIDs::compOverlay33:
    case CommandIDs::compOverlay66:
    case CommandIDs::compOverlay100:
        {
            int amount = 0, num = 0;

            if (commandID == CommandIDs::compOverlay33)
            {
                amount = 33;
                num = 1;
            }
            else if (commandID == CommandIDs::compOverlay66)
            {
                amount = 66;
                num = 2;
            }
            else if (commandID == CommandIDs::compOverlay100)
            {
                amount = 100;
                num = 3;
            }

            result.defaultKeypresses.add (KeyPress (T('2') + num, cmd, 0));

            int currentAmount = 0;
            if (document->getComponentOverlayOpacity() > 0.9f)
                currentAmount = 100;
            else if (document->getComponentOverlayOpacity() > 0.6f)
                currentAmount = 66;
            else if (document->getComponentOverlayOpacity() > 0.3f)
                currentAmount = 33;

            result.setInfo (commandID == CommandIDs::compOverlay0
                                ? T("No component overlay")
                                : T("Overlay with opacity of ") + String (amount) + "%",
                            T("Changes the opacity of the components that are shown over the top of the graphics editor."),
                            CommandCategories::view, 0);
            result.setActive (currentPaintRoutine != 0 && document->getComponentLayout() != 0);
            result.setTicked (amount == currentAmount);
        }
        break;

    case StandardApplicationCommandIDs::cut:
        result.setInfo (T("Cut"),
                        T("Copies the currently selected components to the clipboard and deletes them."),
                        CommandCategories::editing, 0);
        result.setActive (isSomethingSelected());
        result.defaultKeypresses.add (KeyPress (T('x'), cmd, 0));
        break;

    case StandardApplicationCommandIDs::copy:
        result.setInfo (T("Copy"),
                        T("Copies the currently selected components to the clipboard."),
                        CommandCategories::editing, 0);
        result.setActive (isSomethingSelected());
        result.defaultKeypresses.add (KeyPress (T('c'), cmd, 0));
        break;

    case StandardApplicationCommandIDs::paste:
        {
            result.setInfo (T("Paste"),
                            T("Pastes any components from the clipboard."),
                            CommandCategories::editing, 0);
            result.defaultKeypresses.add (KeyPress (T('v'), cmd, 0));

            bool canPaste = false;
            XmlDocument clip (SystemClipboard::getTextFromClipboard());
            XmlElement* const doc = clip.getDocumentElement (true);

            if (doc != 0)
            {
                if (doc->hasTagName (ComponentLayout::clipboardXmlTag))
                    canPaste = (currentLayout != 0);
                else if (doc->hasTagName (PaintRoutine::clipboardXmlTag))
                    canPaste = (currentPaintRoutine != 0);

                delete doc;
            }

            result.setActive (canPaste);
        }

        break;

    case StandardApplicationCommandIDs::del:
        result.setInfo (T("Delete"),
                        T("Deletes any selected components."),
                        CommandCategories::editing, 0);
        result.setActive (isSomethingSelected());
        result.defaultKeypresses.add (KeyPress (KeyPress::deleteKey, 0, 0));
        result.defaultKeypresses.add (KeyPress (KeyPress::backspaceKey, 0, 0));
        break;

    case StandardApplicationCommandIDs::selectAll:
        result.setInfo (T("Select All"),
                        T("Selects all of whatever item is currently selected."),
                        CommandCategories::editing, 0);
        result.setActive (currentPaintRoutine != 0 || currentLayout != 0);
        result.defaultKeypresses.add (KeyPress (T('a'), cmd, 0));
        break;

    case StandardApplicationCommandIDs::deselectAll:
        result.setInfo (T("Deselect All"),
                        T("Deselects whatever is currently selected."),
                        CommandCategories::editing, 0);
        result.setActive (currentPaintRoutine != 0 || currentLayout != 0);
        result.defaultKeypresses.add (KeyPress (T('d'), cmd, 0));
        break;

    default:
        break;
    }
}

bool JucerDocumentHolder::perform (const InvocationInfo& info)
{
    ComponentLayout* const currentLayout = getCurrentLayout();
    PaintRoutine* const currentPaintRoutine = getCurrentPaintRoutine();

    document->getUndoManager().beginNewTransaction();

    if (info.commandID >= CommandIDs::newComponentBase
         && info.commandID < CommandIDs::newComponentBase + ObjectTypes::numComponentTypes)
    {
        addComponent (info.commandID - CommandIDs::newComponentBase);
        return true;
    }

    if (info.commandID >= CommandIDs::newElementBase
         && info.commandID < CommandIDs::newElementBase + ObjectTypes::numElementTypes)
    {
        addElement (info.commandID - CommandIDs::newElementBase);
        return true;
    }

    switch (info.commandID)
    {
    case CommandIDs::close:
        close();
        // (this comp will now be deleted)
        return true;

    case CommandIDs::save:
        document->save (true, true);
        break;

    case CommandIDs::saveAs:
        document->saveAsInteractive (true);
        break;

    case CommandIDs::undo:
        document->getUndoManager().undo();
        document->dispatchPendingMessages();
        break;

    case CommandIDs::redo:
        document->getUndoManager().redo();
        document->dispatchPendingMessages();
        break;

    case CommandIDs::test:
        TestComponent::showInDialogBox (*document);
        break;

    case CommandIDs::enableSnapToGrid:
        document->setSnappingGrid (document->getSnappingGridSize(),
                                   ! document->isSnapActive (false),
                                   document->isSnapShown());
        break;

    case CommandIDs::showGrid:
        document->setSnappingGrid (document->getSnappingGridSize(),
                                   document->isSnapActive (false),
                                   ! document->isSnapShown());
        break;

    case CommandIDs::editCompLayout:
        showLayout();
        break;

    case CommandIDs::editCompGraphics:
        showGraphics (0);
        break;

    case CommandIDs::zoomIn:
        setZoom (getZoom() * 2.0);
        break;

    case CommandIDs::zoomOut:
        setZoom (getZoom() / 2.0);
        break;

    case CommandIDs::zoomNormal:
        setZoom (1.0);
        break;

    case CommandIDs::spaceBarDrag:
        {
            EditingPanelBase* panel = dynamic_cast <EditingPanelBase*> (tabbedComponent->getCurrentContentComponent());

            if (panel != 0)
                panel->dragKeyHeldDown (info.isKeyDown);

            break;
        }

    case CommandIDs::compOverlay0:
    case CommandIDs::compOverlay33:
    case CommandIDs::compOverlay66:
    case CommandIDs::compOverlay100:
        {
            int amount = 0;

            if (info.commandID == CommandIDs::compOverlay33)
                amount = 33;
            else if (info.commandID == CommandIDs::compOverlay66)
                amount = 66;
            else if (info.commandID == CommandIDs::compOverlay100)
                amount = 100;

            document->setComponentOverlayOpacity (amount * 0.01f);
        }
        break;

    case CommandIDs::bringBackLostItems:
        {
            EditingPanelBase* panel = dynamic_cast <EditingPanelBase*> (tabbedComponent->getCurrentContentComponent());

            if (panel != 0)
            {
                int w = panel->getComponentArea().getWidth();
                int h = panel->getComponentArea().getHeight();

                if (currentPaintRoutine != 0)
                    currentPaintRoutine->bringLostItemsBackOnScreen (panel->getComponentArea());
                else if (currentLayout != 0)
                    currentLayout->bringLostItemsBackOnScreen (w, h);
            }

            break;
        }

    case CommandIDs::toFront:
        if (currentLayout != 0)
            currentLayout->selectedToFront();
        else if (currentPaintRoutine != 0)
            currentPaintRoutine->selectedToFront();

        break;

    case CommandIDs::toBack:
        if (currentLayout != 0)
            currentLayout->selectedToBack();
        else if (currentPaintRoutine != 0)
            currentPaintRoutine->selectedToBack();

        break;

    case CommandIDs::group:
        if (currentPaintRoutine != 0)
            currentPaintRoutine->groupSelected();
        break;

    case CommandIDs::ungroup:
        if (currentPaintRoutine != 0)
            currentPaintRoutine->ungroupSelected();
        break;

    case StandardApplicationCommandIDs::cut:
        if (currentLayout != 0)
        {
            currentLayout->copySelectedToClipboard();
            currentLayout->deleteSelected();
        }
        else if (currentPaintRoutine != 0)
        {
            currentPaintRoutine->copySelectedToClipboard();
            currentPaintRoutine->deleteSelected();
        }

        break;

    case StandardApplicationCommandIDs::copy:
        if (currentLayout != 0)
            currentLayout->copySelectedToClipboard();
        else if (currentPaintRoutine != 0)
            currentPaintRoutine->copySelectedToClipboard();

        break;

    case StandardApplicationCommandIDs::paste:
        {
            XmlDocument clip (SystemClipboard::getTextFromClipboard());
            XmlElement* const doc = clip.getDocumentElement (true);

            if (doc != 0)
            {
                if (doc->hasTagName (ComponentLayout::clipboardXmlTag))
                {
                    if (currentLayout != 0)
                        currentLayout->paste();
                }
                else if (doc->hasTagName (PaintRoutine::clipboardXmlTag))
                {
                    if (currentPaintRoutine != 0)
                        currentPaintRoutine->paste();
                }

                delete doc;
            }
        }
        break;

    case StandardApplicationCommandIDs::del:
        if (currentLayout != 0)
            currentLayout->deleteSelected();
        else if (currentPaintRoutine != 0)
            currentPaintRoutine->deleteSelected();

        break;

    case StandardApplicationCommandIDs::selectAll:
        if (currentLayout != 0)
            currentLayout->selectAll();
        else if (currentPaintRoutine != 0)
            currentPaintRoutine->selectAll();
        break;

    case StandardApplicationCommandIDs::deselectAll:
        if (currentLayout != 0)
        {
            currentLayout->getSelectedSet().deselectAll();
        }
        else if (currentPaintRoutine != 0)
        {
            currentPaintRoutine->getSelectedElements().deselectAll();
            currentPaintRoutine->getSelectedPoints().deselectAll();
        }

        break;

    default:
        return false;
    }

    document->getUndoManager().beginNewTransaction();
    return true;
}

JucerDocumentHolder* JucerDocumentHolder::getActiveDocumentHolder()
{
    ApplicationCommandInfo info (0);
    ApplicationCommandTarget* target = commandManager->getTargetForCommand (CommandIDs::close, info);

    return dynamic_cast <JucerDocumentHolder*> (target);
}

Image* JucerDocumentHolder::createComponentLayerSnapshot() const
{
    if (compLayoutPanel != 0)
        return compLayoutPanel->createComponentSnapshot();

    return 0;
}
