/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../../Application/jucer_Headers.h"
#include "../../Settings/jucer_AppearanceSettings.h"
#include "../../Application/jucer_Application.h"
#include "jucer_JucerDocumentEditor.h"
#include "jucer_TestComponent.h"
#include "../jucer_ObjectTypes.h"
#include "jucer_ComponentLayoutPanel.h"
#include "jucer_PaintRoutinePanel.h"
#include "jucer_ResourceEditorPanel.h"
#include "../Properties/jucer_ComponentTextProperty.h"
#include "../Properties/jucer_ComponentChoiceProperty.h"
#include "../UI/jucer_JucerCommandIDs.h"

//==============================================================================
class ExtraMethodsList  : public PropertyComponent,
                          public ListBoxModel,
                          public ChangeListener
{
public:
    ExtraMethodsList (JucerDocument& doc)
        : PropertyComponent ("extra callbacks", 250),
          document (doc)
    {
        listBox.reset (new ListBox (String(), this));
        addAndMakeVisible (listBox.get());
        listBox->setRowHeight (22);

        document.addChangeListener (this);
    }

    ~ExtraMethodsList()
    {
        document.removeChangeListener (this);
    }

    int getNumRows() override
    {
        return methods.size();
    }

    void paintListBoxItem (int row, Graphics& g, int width, int height, bool rowIsSelected) override
    {
        if (row < 0 || row >= getNumRows())
            return;

        if (rowIsSelected)
        {
            g.fillAll (findColour (TextEditor::highlightColourId));
            g.setColour (findColour (defaultHighlightedTextColourId));
        }
        else
        {
            g.setColour (findColour (defaultTextColourId));
        }

        g.setFont (height * 0.6f);
        g.drawText (returnValues [row] + " " + baseClasses [row] + "::" + methods [row],
                    30, 0, width - 32, height,
                    Justification::centredLeft, true);

        getLookAndFeel().drawTickBox (g, *this, 6, 2, 18, 18, document.isOptionalMethodEnabled (methods [row]), true, false, false);
    }

    void listBoxItemClicked (int row, const MouseEvent& e) override
    {
        if (row < 0 || row >= getNumRows())
            return;

        if (e.x < 30)
            document.setOptionalMethodEnabled (methods [row],
                                               ! document.isOptionalMethodEnabled (methods [row]));
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::white);
    }

    void resized() override
    {
        listBox->setBounds (getLocalBounds());
    }

    void refresh() override
    {
        baseClasses.clear();
        returnValues.clear();
        methods.clear();
        initialContents.clear();

        document.getOptionalMethods (baseClasses, returnValues, methods, initialContents);

        listBox->updateContent();
        listBox->repaint();
    }

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        refresh();
    }

private:
    JucerDocument& document;
    std::unique_ptr<ListBox> listBox;

    StringArray baseClasses, returnValues, methods, initialContents;
};


//==============================================================================
class ClassPropertiesPanel  : public Component,
                              private ChangeListener
{
public:
    ClassPropertiesPanel (JucerDocument& doc)
        : document (doc)
    {
        addAndMakeVisible (panel1);
        addAndMakeVisible (panel2);

        Array <PropertyComponent*> props;
        props.add (new ComponentClassNameProperty (doc));
        props.add (new TemplateFileProperty (doc));
        props.add (new ComponentCompNameProperty (doc));
        props.add (new ComponentParentClassesProperty (doc));
        props.add (new ComponentConstructorParamsProperty (doc));
        props.add (new ComponentInitialisersProperty (doc));
        props.add (new ComponentInitialSizeProperty (doc, true));
        props.add (new ComponentInitialSizeProperty (doc, false));
        props.add (new FixedSizeProperty (doc));

        panel1.addSection ("General class settings", props);

        Array <PropertyComponent*> props2;
        props2.add (new ExtraMethodsList (doc));
        panel2.addSection ("Extra callback methods to generate", props2);

        doc.addExtraClassProperties (panel1);
        doc.addChangeListener (this);
    }

    ~ClassPropertiesPanel()
    {
        document.removeChangeListener (this);
    }

    void resized() override
    {
        int pw = jmin (getWidth() / 2 - 20, 350);
        panel1.setBounds (10, 6, pw, getHeight() - 12);
        panel2.setBounds (panel1.getRight() + 20, panel1.getY(), pw, panel1.getHeight());
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColour (secondaryBackgroundColourId));
    }

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        panel1.refreshAll();
        panel2.refreshAll();
    }

private:
    JucerDocument& document;
    PropertyPanel panel1, panel2;

    //==============================================================================
    class ComponentClassNameProperty    : public ComponentTextProperty <Component>
    {
    public:
        ComponentClassNameProperty (JucerDocument& doc)
            : ComponentTextProperty<Component> ("Class name", 128, false, nullptr, doc)
        {}

        void setText (const String& newText) override    { document.setClassName (newText); }
        String getText() const override                  { return document.getClassName(); }
    };

    //==============================================================================
    class ComponentCompNameProperty    : public ComponentTextProperty <Component>
    {
    public:
        ComponentCompNameProperty (JucerDocument& doc)
            : ComponentTextProperty<Component> ("Component name", 200, false, nullptr, doc)
        {}

        void setText (const String& newText) override    { document.setComponentName (newText); }
        String getText() const override                  { return document.getComponentName(); }
    };

    //==============================================================================
    class ComponentParentClassesProperty    : public ComponentTextProperty <Component>
    {
    public:
        ComponentParentClassesProperty (JucerDocument& doc)
            : ComponentTextProperty<Component> ("Parent classes", 512, false, nullptr, doc)
        {}

        void setText (const String& newText) override    { document.setParentClasses (newText); }
        String getText() const override                  { return document.getParentClassString(); }
    };

    //==============================================================================
    class ComponentConstructorParamsProperty    : public ComponentTextProperty <Component>
    {
    public:
        ComponentConstructorParamsProperty (JucerDocument& doc)
            : ComponentTextProperty<Component> ("Constructor params", 2048, false, nullptr, doc)
        {}

        void setText (const String& newText) override    { document.setConstructorParams (newText); }
        String getText() const override                  { return document.getConstructorParams(); }
    };

    //==============================================================================
    class ComponentInitialisersProperty   : public ComponentTextProperty <Component>
    {
    public:
        ComponentInitialisersProperty (JucerDocument& doc)
            : ComponentTextProperty <Component> ("Member initialisers", 16384, true, nullptr, doc)
        {
            preferredHeight = 24 * 3;
        }

        void setText (const String& newText) override    { document.setVariableInitialisers (newText); }
        String getText() const override                  { return document.getVariableInitialisers(); }
    };


    //==============================================================================
    class ComponentInitialSizeProperty    : public ComponentTextProperty <Component>
    {
    public:
        ComponentInitialSizeProperty (JucerDocument& doc, const bool isWidth_)
            : ComponentTextProperty<Component> (isWidth_ ? "Initial width"
                                                         : "Initial height",
                                     10, false, nullptr, doc),
              isWidth (isWidth_)
        {}

        void setText (const String& newText) override
        {
            if (isWidth)
                document.setInitialSize  (newText.getIntValue(), document.getInitialHeight());
            else
                document.setInitialSize  (document.getInitialWidth(), newText.getIntValue());
        }

        String getText() const override
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
        FixedSizeProperty (JucerDocument& doc)
            : ComponentChoiceProperty<Component> ("Fixed size", nullptr, doc)
        {
            choices.add ("Resize component to fit workspace");
            choices.add ("Keep component size fixed");
        }

        void setIndex (int newIndex)        { document.setFixedSize (newIndex != 0); }
        int getIndex() const                { return document.isFixedSize() ? 1 : 0; }
    };

    //==============================================================================
    class TemplateFileProperty    : public ComponentTextProperty <Component>
    {
    public:
        TemplateFileProperty (JucerDocument& doc)
            : ComponentTextProperty<Component> ("Template file", 2048, false, nullptr, doc)
        {}

        void setText (const String& newText) override    { document.setTemplateFile (newText); }
        String getText() const override                  { return document.getTemplateFile(); }
    };
};

static const Colour tabColour (Colour (0xff888888));

static SourceCodeEditor* createCodeEditor (const File& file, SourceCodeDocument& sourceCodeDoc)
{
    return new SourceCodeEditor (&sourceCodeDoc,
                                 new CppCodeEditorComponent (file, sourceCodeDoc.getCodeDocument()));
}

//==============================================================================
JucerDocumentEditor::JucerDocumentEditor (JucerDocument* const doc)
    : document (doc),
      tabbedComponent (doc)
{
    setOpaque (true);

    if (document != nullptr)
    {
        setSize (document->getInitialWidth(),
                 document->getInitialHeight());

        addAndMakeVisible (tabbedComponent);
        tabbedComponent.setOutline (0);

        tabbedComponent.addTab ("Class", tabColour, new ClassPropertiesPanel (*document), true);

        if (document->getComponentLayout() != nullptr)
            tabbedComponent.addTab ("Subcomponents", tabColour,
                                    compLayoutPanel = new ComponentLayoutPanel (*document, *document->getComponentLayout()), true);

        tabbedComponent.addTab ("Resources", tabColour, new ResourceEditorPanel (*document), true);

        tabbedComponent.addTab ("Code", tabColour, createCodeEditor (document->getCppFile(),
                                                                     document->getCppDocument()), true);

        updateTabs();
        restoreLastSelectedTab();

        document->addChangeListener (this);

        resized();
        refreshPropertiesPanel();

        changeListenerCallback (nullptr);

        if (auto* project = document->getCppDocument().getProject())
        {
            if (project->shouldSendGUIBuilderAnalyticsEvent())
                Analytics::getInstance()->logEvent ("GUI Builder", {}, ProjucerAnalyticsEvent::projectEvent);
        }
    }
}

JucerDocumentEditor::~JucerDocumentEditor()
{
    saveLastSelectedTab();
    tabbedComponent.clearTabs();
}

void JucerDocumentEditor::refreshPropertiesPanel() const
{
    for (int i = tabbedComponent.getNumTabs(); --i >= 0;)
    {
        if (ComponentLayoutPanel* layoutPanel = dynamic_cast<ComponentLayoutPanel*> (tabbedComponent.getTabContentComponent (i)))
        {
            if (layoutPanel->isVisible())
                layoutPanel->updatePropertiesList();
        }
        else
        {
            if (PaintRoutinePanel* pr = dynamic_cast<PaintRoutinePanel*> (tabbedComponent.getTabContentComponent (i)))
                if (pr->isVisible())
                    pr->updatePropertiesList();
        }
    }
}

void JucerDocumentEditor::updateTabs()
{
    const StringArray paintRoutineNames (document->getPaintRoutineNames());

    for (int i = tabbedComponent.getNumTabs(); --i >= 0;)
    {
        if (dynamic_cast<PaintRoutinePanel*> (tabbedComponent.getTabContentComponent (i)) != nullptr
             && ! paintRoutineNames.contains (tabbedComponent.getTabNames() [i]))
        {
            tabbedComponent.removeTab (i);
        }
    }

    for (int i = 0; i < document->getNumPaintRoutines(); ++i)
    {
        if (! tabbedComponent.getTabNames().contains (paintRoutineNames [i]))
        {
            int index, numPaintRoutinesSeen = 0;
            for (index = 1; index < tabbedComponent.getNumTabs(); ++index)
            {
                if (dynamic_cast<PaintRoutinePanel*> (tabbedComponent.getTabContentComponent (index)) != nullptr)
                {
                    if (++numPaintRoutinesSeen == i)
                    {
                        ++index;
                        break;
                    }
                }
            }

            if (numPaintRoutinesSeen == 0)
                index = document->getComponentLayout() != nullptr ? 2 : 1;

            tabbedComponent.addTab (paintRoutineNames[i], tabColour,
                                    new PaintRoutinePanel (*document,
                                                           *document->getPaintRoutine (i),
                                                           this), true, index);
        }
    }
}

//==============================================================================
void JucerDocumentEditor::paint (Graphics& g)
{
    g.fillAll (findColour (backgroundColourId));
}

void JucerDocumentEditor::resized()
{
    tabbedComponent.setBounds (getLocalBounds().withTrimmedLeft (12));
}

void JucerDocumentEditor::changeListenerCallback (ChangeBroadcaster*)
{
    setName (document->getClassName());
    updateTabs();
}

//==============================================================================
ApplicationCommandTarget* JucerDocumentEditor::getNextCommandTarget()
{
    return findFirstTargetParentComponent();
}

ComponentLayout* JucerDocumentEditor::getCurrentLayout() const
{
    if (ComponentLayoutPanel* panel = dynamic_cast<ComponentLayoutPanel*> (tabbedComponent.getCurrentContentComponent()))
        return &(panel->layout);

    return nullptr;
}

PaintRoutine* JucerDocumentEditor::getCurrentPaintRoutine() const
{
    if (PaintRoutinePanel* panel = dynamic_cast<PaintRoutinePanel*> (tabbedComponent.getCurrentContentComponent()))
        return &(panel->getPaintRoutine());

    return nullptr;
}

void JucerDocumentEditor::showLayout()
{
    if (getCurrentLayout() == nullptr)
    {
        for (int i = 0; i < tabbedComponent.getNumTabs(); ++i)
        {
            if (dynamic_cast<ComponentLayoutPanel*> (tabbedComponent.getTabContentComponent (i)) != nullptr)
            {
                tabbedComponent.setCurrentTabIndex (i);
                break;
            }
        }
    }
}

void JucerDocumentEditor::showGraphics (PaintRoutine* routine)
{
    if (getCurrentPaintRoutine() != routine || routine == nullptr)
    {
        for (int i = 0; i < tabbedComponent.getNumTabs(); ++i)
        {
            if (auto pr = dynamic_cast<PaintRoutinePanel*> (tabbedComponent.getTabContentComponent (i)))
            {
                if (routine == &(pr->getPaintRoutine()) || routine == nullptr)
                {
                    tabbedComponent.setCurrentTabIndex (i);
                    break;
                }
            }
        }
    }
}

//==============================================================================
void JucerDocumentEditor::setViewportToLastPos (Viewport* vp, EditingPanelBase& editor)
{
    vp->setViewPosition (lastViewportX, lastViewportY);
    editor.setZoom (currentZoomLevel);
}

void JucerDocumentEditor::storeLastViewportPos (Viewport* vp, EditingPanelBase& editor)
{
    lastViewportX = vp->getViewPositionX();
    lastViewportY = vp->getViewPositionY();

    currentZoomLevel = editor.getZoom();
}

void JucerDocumentEditor::setZoom (double scale)
{
    scale = jlimit (1.0 / 4.0, 32.0, scale);

    if (EditingPanelBase* panel = dynamic_cast<EditingPanelBase*> (tabbedComponent.getCurrentContentComponent()))
        panel->setZoom (scale);
}

double JucerDocumentEditor::getZoom() const
{
    if (EditingPanelBase* panel = dynamic_cast<EditingPanelBase*> (tabbedComponent.getCurrentContentComponent()))
        return panel->getZoom();

    return 1.0;
}

static double snapToIntegerZoom (double zoom)
{
    if (zoom >= 1.0)
        return (double) (int) (zoom + 0.5);

    return 1.0 / (int) (1.0 / zoom + 0.5);
}

void JucerDocumentEditor::addElement (const int index)
{
    if (PaintRoutinePanel* const panel = dynamic_cast<PaintRoutinePanel*> (tabbedComponent.getCurrentContentComponent()))
    {
        PaintRoutine* const currentPaintRoutine = & (panel->getPaintRoutine());
        const Rectangle<int> area (panel->getComponentArea());

        document->beginTransaction();

        PaintElement* e = ObjectTypes::createNewElement (index, currentPaintRoutine);

        e->setInitialBounds (area.getWidth(), area.getHeight());

        e = currentPaintRoutine->addNewElement (e, -1, true);

        if (e != nullptr)
        {
            const int randomness = jmin (80, area.getWidth() / 2, area.getHeight() / 2);
            int x = area.getX() + area.getWidth() / 2 + Random::getSystemRandom().nextInt (randomness) - randomness / 2;
            int y = area.getY() + area.getHeight() / 2 + Random::getSystemRandom().nextInt (randomness) - randomness / 2;
            x = document->snapPosition (x);
            y = document->snapPosition (y);

            panel->xyToTargetXY (x, y);

            Rectangle<int> r (e->getCurrentBounds (area));
            r.setPosition (x, y);
            e->setCurrentBounds (r, area, true);

            currentPaintRoutine->getSelectedElements().selectOnly (e);
        }

        document->beginTransaction();
    }
}

void JucerDocumentEditor::addComponent (const int index)
{
    showLayout();

    if (ComponentLayoutPanel* const panel = dynamic_cast<ComponentLayoutPanel*> (tabbedComponent.getCurrentContentComponent()))
    {
        const Rectangle<int> area (panel->getComponentArea());

        document->beginTransaction ("Add new " + ObjectTypes::componentTypeHandlers [index]->getTypeName());

        const int randomness = jmin (80, area.getWidth() / 2, area.getHeight() / 2);
        int x = area.getWidth() / 2 + Random::getSystemRandom().nextInt (randomness) - randomness / 2;
        int y = area.getHeight() / 2 + Random::getSystemRandom().nextInt (randomness) - randomness / 2;
        x = document->snapPosition (x);
        y = document->snapPosition (y);

        panel->xyToTargetXY (x, y);

        if (Component* newOne = panel->layout.addNewComponent (ObjectTypes::componentTypeHandlers [index], x, y))
            panel->layout.getSelectedSet().selectOnly (newOne);

        document->beginTransaction();
    }
}
//==============================================================================
void JucerDocumentEditor::saveLastSelectedTab() const
{
    if (document != nullptr)
    {
        if (auto* project = document->getCppDocument().getProject())
        {
            auto& projectProps = project->getStoredProperties();

            std::unique_ptr<XmlElement> root (projectProps.getXmlValue ("GUIComponentsLastTab"));

            if (root == nullptr)
                root.reset (new XmlElement ("FILES"));

            auto fileName = document->getCppFile().getFileName();

            auto* child = root->getChildByName (fileName);

            if (child == nullptr)
                child = root->createNewChildElement (fileName);

            child->setAttribute ("tab", tabbedComponent.getCurrentTabIndex());

            projectProps.setValue ("GUIComponentsLastTab", root.get());
        }
    }
}

void JucerDocumentEditor::restoreLastSelectedTab()
{
    if (document != nullptr)
    {
        if (auto* project = document->getCppDocument().getProject())
        {
            std::unique_ptr<XmlElement> root (project->getStoredProperties().getXmlValue ("GUIComponentsLastTab"));

            if (root != nullptr)
            {
                auto* child = root->getChildByName (document->getCppFile().getFileName());

                if (child != nullptr)
                    tabbedComponent.setCurrentTabIndex (child->getIntAttribute ("tab"));
            }
        }
    }
}

//==============================================================================
bool JucerDocumentEditor::isSomethingSelected() const
{
    if (auto* layout = getCurrentLayout())
        return layout->getSelectedSet().getNumSelected() > 0;

    if (auto* routine = getCurrentPaintRoutine())
        return routine->getSelectedElements().getNumSelected() > 0;

    return false;
}

bool JucerDocumentEditor::areMultipleThingsSelected() const
{
    if (auto* layout = getCurrentLayout())
        return layout->getSelectedSet().getNumSelected() > 1;

    if (auto* routine = getCurrentPaintRoutine())
        return routine->getSelectedElements().getNumSelected() > 1;

    return false;
}

//==============================================================================
void JucerDocumentEditor::getAllCommands (Array <CommandID>& commands)
{
    const CommandID ids[] =
    {
        JucerCommandIDs::test,
        JucerCommandIDs::toFront,
        JucerCommandIDs::toBack,
        JucerCommandIDs::group,
        JucerCommandIDs::ungroup,
        JucerCommandIDs::bringBackLostItems,
        JucerCommandIDs::enableSnapToGrid,
        JucerCommandIDs::showGrid,
        JucerCommandIDs::editCompLayout,
        JucerCommandIDs::editCompGraphics,
        JucerCommandIDs::zoomIn,
        JucerCommandIDs::zoomOut,
        JucerCommandIDs::zoomNormal,
        JucerCommandIDs::spaceBarDrag,
        JucerCommandIDs::compOverlay0,
        JucerCommandIDs::compOverlay33,
        JucerCommandIDs::compOverlay66,
        JucerCommandIDs::compOverlay100,
        JucerCommandIDs::alignTop,
        JucerCommandIDs::alignRight,
        JucerCommandIDs::alignBottom,
        JucerCommandIDs::alignLeft,
        StandardApplicationCommandIDs::undo,
        StandardApplicationCommandIDs::redo,
        StandardApplicationCommandIDs::cut,
        StandardApplicationCommandIDs::copy,
        StandardApplicationCommandIDs::paste,
        StandardApplicationCommandIDs::del,
        StandardApplicationCommandIDs::selectAll,
        StandardApplicationCommandIDs::deselectAll
    };

    commands.addArray (ids, numElementsInArray (ids));

    for (int i = 0; i < ObjectTypes::numComponentTypes; ++i)
        commands.add (JucerCommandIDs::newComponentBase + i);

    for (int i = 0; i < ObjectTypes::numElementTypes; ++i)
        commands.add (JucerCommandIDs::newElementBase + i);
}

void JucerDocumentEditor::getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result)
{
    ComponentLayout* const currentLayout = getCurrentLayout();
    PaintRoutine* const currentPaintRoutine = getCurrentPaintRoutine();

    const int cmd = ModifierKeys::commandModifier;
    const int shift = ModifierKeys::shiftModifier;

    if (commandID >= JucerCommandIDs::newComponentBase
         && commandID < JucerCommandIDs::newComponentBase + ObjectTypes::numComponentTypes)
    {
        const int index = commandID - JucerCommandIDs::newComponentBase;

        result.setInfo ("New " + ObjectTypes::componentTypeHandlers [index]->getTypeName(),
                        "Creates a new " + ObjectTypes::componentTypeHandlers [index]->getTypeName(),
                        CommandCategories::editing, 0);
        return;
    }

    if (commandID >= JucerCommandIDs::newElementBase
         && commandID < JucerCommandIDs::newElementBase + ObjectTypes::numElementTypes)
    {
        const int index = commandID - JucerCommandIDs::newElementBase;

        result.setInfo (String ("New ") + ObjectTypes::elementTypeNames [index],
                        String ("Adds a new ") + ObjectTypes::elementTypeNames [index],
                        CommandCategories::editing, 0);

        result.setActive (currentPaintRoutine != nullptr);
        return;
    }

    switch (commandID)
    {
    case JucerCommandIDs::toFront:
        result.setInfo (TRANS("Bring to front"), TRANS("Brings the currently selected component to the front."), CommandCategories::editing, 0);
        result.setActive (isSomethingSelected());
        result.defaultKeypresses.add (KeyPress ('f', cmd, 0));
        break;

    case JucerCommandIDs::toBack:
        result.setInfo (TRANS("Send to back"), TRANS("Sends the currently selected component to the back."), CommandCategories::editing, 0);
        result.setActive (isSomethingSelected());
        result.defaultKeypresses.add (KeyPress ('b', cmd, 0));
        break;

    case JucerCommandIDs::group:
        result.setInfo (TRANS("Group selected items"), TRANS("Turns the currently selected elements into a single group object."), CommandCategories::editing, 0);
        result.setActive (currentPaintRoutine != nullptr && currentPaintRoutine->getSelectedElements().getNumSelected() > 1);
        result.defaultKeypresses.add (KeyPress ('k', cmd, 0));
        break;

    case JucerCommandIDs::ungroup:
        result.setInfo (TRANS("Ungroup selected items"), TRANS("Turns the currently selected elements into a single group object."), CommandCategories::editing, 0);
        result.setActive (currentPaintRoutine != nullptr
                           && currentPaintRoutine->getSelectedElements().getNumSelected() == 1
                           && currentPaintRoutine->getSelectedElements().getSelectedItem (0)->getTypeName() == "Group");
        result.defaultKeypresses.add (KeyPress ('k', cmd | shift, 0));
        break;

    case JucerCommandIDs::test:
        result.setInfo (TRANS("Test component..."), TRANS("Runs the current component interactively."), CommandCategories::view, 0);
        result.defaultKeypresses.add (KeyPress ('t', cmd, 0));
        break;

    case JucerCommandIDs::enableSnapToGrid:
        result.setInfo (TRANS("Enable snap-to-grid"), TRANS("Toggles whether components' positions are aligned to a grid."), CommandCategories::view, 0);
        result.setTicked (document != nullptr && document->isSnapActive (false));
        result.defaultKeypresses.add (KeyPress ('g', cmd, 0));
        break;

    case JucerCommandIDs::showGrid:
        result.setInfo (TRANS("Show snap-to-grid"), TRANS("Toggles whether the snapping grid is displayed on-screen."), CommandCategories::view, 0);
        result.setTicked (document != nullptr && document->isSnapShown());
        result.defaultKeypresses.add (KeyPress ('g', cmd | shift, 0));
        break;

    case JucerCommandIDs::editCompLayout:
        result.setInfo (TRANS("Edit sub-component layout"), TRANS("Switches to the sub-component editor view."), CommandCategories::view, 0);
        result.setTicked (currentLayout != nullptr);
        result.defaultKeypresses.add (KeyPress ('n', cmd, 0));
        break;

    case JucerCommandIDs::editCompGraphics:
        result.setInfo (TRANS("Edit background graphics"), TRANS("Switches to the background graphics editor view."), CommandCategories::view, 0);
        result.setTicked (currentPaintRoutine != nullptr);
        result.defaultKeypresses.add (KeyPress ('m', cmd, 0));
        break;

    case JucerCommandIDs::bringBackLostItems:
        result.setInfo (TRANS("Retrieve offscreen items"), TRANS("Moves any items that are lost beyond the edges of the screen back to the centre."), CommandCategories::editing, 0);
        result.setActive (currentPaintRoutine != nullptr || currentLayout != nullptr);
        result.defaultKeypresses.add (KeyPress ('m', cmd, 0));
        break;

    case JucerCommandIDs::zoomIn:
        result.setInfo (TRANS("Zoom in"), TRANS("Zooms in on the current component."), CommandCategories::editing, 0);
        result.setActive (currentPaintRoutine != nullptr || currentLayout != nullptr);
        result.defaultKeypresses.add (KeyPress (']', cmd, 0));
        break;

    case JucerCommandIDs::zoomOut:
        result.setInfo (TRANS("Zoom out"), TRANS("Zooms out on the current component."), CommandCategories::editing, 0);
        result.setActive (currentPaintRoutine != nullptr || currentLayout != nullptr);
        result.defaultKeypresses.add (KeyPress ('[', cmd, 0));
        break;

    case JucerCommandIDs::zoomNormal:
        result.setInfo (TRANS("Zoom to 100%"), TRANS("Restores the zoom level to normal."), CommandCategories::editing, 0);
        result.setActive (currentPaintRoutine != nullptr || currentLayout != nullptr);
        result.defaultKeypresses.add (KeyPress ('1', cmd, 0));
        break;

    case JucerCommandIDs::spaceBarDrag:
        result.setInfo (TRANS("Scroll while dragging mouse"), TRANS("When held down, this key lets you scroll around by dragging with the mouse."),
                        CommandCategories::view, ApplicationCommandInfo::wantsKeyUpDownCallbacks);
        result.setActive (currentPaintRoutine != nullptr || currentLayout != nullptr);
        result.defaultKeypresses.add (KeyPress (KeyPress::spaceKey, 0, 0));
        break;

    case JucerCommandIDs::compOverlay0:
    case JucerCommandIDs::compOverlay33:
    case JucerCommandIDs::compOverlay66:
    case JucerCommandIDs::compOverlay100:
        {
            int amount = 0, num = 0;

            if (commandID == JucerCommandIDs::compOverlay33)
            {
                amount = 33;
                num = 1;
            }
            else if (commandID == JucerCommandIDs::compOverlay66)
            {
                amount = 66;
                num = 2;
            }
            else if (commandID == JucerCommandIDs::compOverlay100)
            {
                amount = 100;
                num = 3;
            }

            result.defaultKeypresses.add (KeyPress ('2' + num, cmd, 0));

            int currentAmount = 0;
            if (document != nullptr && document->getComponentOverlayOpacity() > 0.9f)
                currentAmount = 100;
            else if (document != nullptr && document->getComponentOverlayOpacity() > 0.6f)
                currentAmount = 66;
            else if (document != nullptr && document->getComponentOverlayOpacity() > 0.3f)
                currentAmount = 33;

            result.setInfo (commandID == JucerCommandIDs::compOverlay0
                                ? TRANS("No component overlay")
                                : TRANS("Overlay with opacity of 123%").replace ("123", String (amount)),
                            TRANS("Changes the opacity of the components that are shown over the top of the graphics editor."),
                            CommandCategories::view, 0);
            result.setActive (currentPaintRoutine != nullptr && document->getComponentLayout() != nullptr);
            result.setTicked (amount == currentAmount);
        }
        break;

    case JucerCommandIDs::alignTop:
            result.setInfo (TRANS ("Align top"),
                            TRANS ("Aligns the top edges of all selected components to the first component that was selected."),
                            CommandCategories::editing, 0);
            result.setActive (areMultipleThingsSelected());
        break;

    case JucerCommandIDs::alignRight:
            result.setInfo (TRANS ("Align right"),
                            TRANS ("Aligns the right edges of all selected components to the first component that was selected."),
                            CommandCategories::editing, 0);
            result.setActive (areMultipleThingsSelected());
        break;

    case JucerCommandIDs::alignBottom:
            result.setInfo (TRANS ("Align bottom"),
                            TRANS ("Aligns the bottom edges of all selected components to the first component that was selected."),
                            CommandCategories::editing, 0);
            result.setActive (areMultipleThingsSelected());
        break;

    case JucerCommandIDs::alignLeft:
            result.setInfo (TRANS ("Align left"),
                            TRANS ("Aligns the left edges of all selected components to the first component that was selected."),
                            CommandCategories::editing, 0);
            result.setActive (areMultipleThingsSelected());
        break;

    case StandardApplicationCommandIDs::undo:
        result.setInfo (TRANS ("Undo"), TRANS ("Undo"), "Editing", 0);
        result.setActive (document != nullptr && document->getUndoManager().canUndo());
        result.defaultKeypresses.add (KeyPress ('z', cmd, 0));
        break;

    case StandardApplicationCommandIDs::redo:
        result.setInfo (TRANS ("Redo"), TRANS ("Redo"), "Editing", 0);
        result.setActive (document != nullptr && document->getUndoManager().canRedo());
        result.defaultKeypresses.add (KeyPress ('z', cmd | shift, 0));
        break;

    case StandardApplicationCommandIDs::cut:
        result.setInfo (TRANS ("Cut"), String(), "Editing", 0);
        result.setActive (isSomethingSelected());
        result.defaultKeypresses.add (KeyPress ('x', cmd, 0));
        break;

    case StandardApplicationCommandIDs::copy:
        result.setInfo (TRANS ("Copy"), String(), "Editing", 0);
        result.setActive (isSomethingSelected());
        result.defaultKeypresses.add (KeyPress ('c', cmd, 0));
        break;

    case StandardApplicationCommandIDs::paste:
        {
            result.setInfo (TRANS ("Paste"), String(), "Editing", 0);
            result.defaultKeypresses.add (KeyPress ('v', cmd, 0));

            bool canPaste = false;

            if (auto doc = parseXML (SystemClipboard::getTextFromClipboard()))
            {
                if (doc->hasTagName (ComponentLayout::clipboardXmlTag))
                    canPaste = (currentLayout != nullptr);
                else if (doc->hasTagName (PaintRoutine::clipboardXmlTag))
                    canPaste = (currentPaintRoutine != nullptr);
            }

            result.setActive (canPaste);
        }

        break;

    case StandardApplicationCommandIDs::del:
        result.setInfo (TRANS ("Delete"), String(), "Editing", 0);
        result.setActive (isSomethingSelected());
        break;

    case StandardApplicationCommandIDs::selectAll:
        result.setInfo (TRANS ("Select All"), String(), "Editing", 0);
        result.setActive (currentPaintRoutine != nullptr || currentLayout != nullptr);
        result.defaultKeypresses.add (KeyPress ('a', cmd, 0));
        break;

    case StandardApplicationCommandIDs::deselectAll:
        result.setInfo (TRANS ("Deselect All"), String(), "Editing", 0);
        result.setActive (currentPaintRoutine != nullptr || currentLayout != nullptr);
        result.defaultKeypresses.add (KeyPress ('d', cmd, 0));
        break;

    default:
        break;
    }
}

bool JucerDocumentEditor::perform (const InvocationInfo& info)
{
    ComponentLayout* const currentLayout = getCurrentLayout();
    PaintRoutine* const currentPaintRoutine = getCurrentPaintRoutine();

    document->beginTransaction();

    if (info.commandID >= JucerCommandIDs::newComponentBase
         && info.commandID < JucerCommandIDs::newComponentBase + ObjectTypes::numComponentTypes)
    {
        addComponent (info.commandID - JucerCommandIDs::newComponentBase);
        return true;
    }

    if (info.commandID >= JucerCommandIDs::newElementBase
         && info.commandID < JucerCommandIDs::newElementBase + ObjectTypes::numElementTypes)
    {
        addElement (info.commandID - JucerCommandIDs::newElementBase);
        return true;
    }

    switch (info.commandID)
    {
        case StandardApplicationCommandIDs::undo:
            document->getUndoManager().undo();
            document->dispatchPendingMessages();
            break;

        case StandardApplicationCommandIDs::redo:
            document->getUndoManager().redo();
            document->dispatchPendingMessages();
            break;

        case JucerCommandIDs::test:
            TestComponent::showInDialogBox (*document);
            break;

        case JucerCommandIDs::enableSnapToGrid:
            document->setSnappingGrid (document->getSnappingGridSize(),
                                       ! document->isSnapActive (false),
                                       document->isSnapShown());
            break;

        case JucerCommandIDs::showGrid:
            document->setSnappingGrid (document->getSnappingGridSize(),
                                       document->isSnapActive (false),
                                       ! document->isSnapShown());
            break;

        case JucerCommandIDs::editCompLayout:
            showLayout();
            break;

        case JucerCommandIDs::editCompGraphics:
            showGraphics (nullptr);
            break;

        case JucerCommandIDs::zoomIn:      setZoom (snapToIntegerZoom (getZoom() * 2.0)); break;
        case JucerCommandIDs::zoomOut:     setZoom (snapToIntegerZoom (getZoom() / 2.0)); break;
        case JucerCommandIDs::zoomNormal:  setZoom (1.0); break;

        case JucerCommandIDs::spaceBarDrag:
            if (EditingPanelBase* panel = dynamic_cast<EditingPanelBase*> (tabbedComponent.getCurrentContentComponent()))
                panel->dragKeyHeldDown (info.isKeyDown);

            break;

        case JucerCommandIDs::compOverlay0:
        case JucerCommandIDs::compOverlay33:
        case JucerCommandIDs::compOverlay66:
        case JucerCommandIDs::compOverlay100:
            {
                int amount = 0;

                if (info.commandID == JucerCommandIDs::compOverlay33)
                    amount = 33;
                else if (info.commandID == JucerCommandIDs::compOverlay66)
                    amount = 66;
                else if (info.commandID == JucerCommandIDs::compOverlay100)
                    amount = 100;

                document->setComponentOverlayOpacity (amount * 0.01f);
            }
            break;

        case JucerCommandIDs::bringBackLostItems:
            if (EditingPanelBase* panel = dynamic_cast<EditingPanelBase*> (tabbedComponent.getCurrentContentComponent()))
            {
                int w = panel->getComponentArea().getWidth();
                int h = panel->getComponentArea().getHeight();

                if (currentPaintRoutine != nullptr)
                    currentPaintRoutine->bringLostItemsBackOnScreen (panel->getComponentArea());
                else if (currentLayout != nullptr)
                    currentLayout->bringLostItemsBackOnScreen (w, h);
            }

            break;

        case JucerCommandIDs::toFront:
            if (currentLayout != nullptr)
                currentLayout->selectedToFront();
            else if (currentPaintRoutine != nullptr)
                currentPaintRoutine->selectedToFront();

            break;

        case JucerCommandIDs::toBack:
            if (currentLayout != nullptr)
                currentLayout->selectedToBack();
            else if (currentPaintRoutine != nullptr)
                currentPaintRoutine->selectedToBack();

            break;

        case JucerCommandIDs::group:
            if (currentPaintRoutine != nullptr)
                currentPaintRoutine->groupSelected();
            break;

        case JucerCommandIDs::ungroup:
            if (currentPaintRoutine != nullptr)
                currentPaintRoutine->ungroupSelected();
            break;

        case JucerCommandIDs::alignTop:
            if (currentLayout != nullptr)
                currentLayout->alignTop();
            else if (currentPaintRoutine != nullptr)
                currentPaintRoutine->alignTop();

            break;

        case JucerCommandIDs::alignRight:
            if (currentLayout != nullptr)
                currentLayout->alignRight();
            else if (currentPaintRoutine != nullptr)
                currentPaintRoutine->alignRight();

            break;

        case JucerCommandIDs::alignBottom:
            if (currentLayout != nullptr)
                currentLayout->alignBottom();
            else if (currentPaintRoutine != nullptr)
                currentPaintRoutine->alignBottom();

            break;

        case JucerCommandIDs::alignLeft:
            if (currentLayout != nullptr)
                currentLayout->alignLeft();
            else if (currentPaintRoutine != nullptr)
                currentPaintRoutine->alignLeft();

            break;

        case StandardApplicationCommandIDs::cut:
            if (currentLayout != nullptr)
            {
                currentLayout->copySelectedToClipboard();
                currentLayout->deleteSelected();
            }
            else if (currentPaintRoutine != nullptr)
            {
                currentPaintRoutine->copySelectedToClipboard();
                currentPaintRoutine->deleteSelected();
            }

            break;

        case StandardApplicationCommandIDs::copy:
            if (currentLayout != nullptr)
                currentLayout->copySelectedToClipboard();
            else if (currentPaintRoutine != nullptr)
                currentPaintRoutine->copySelectedToClipboard();

            break;

        case StandardApplicationCommandIDs::paste:
            {
                if (auto doc = parseXML (SystemClipboard::getTextFromClipboard()))
                {
                    if (doc->hasTagName (ComponentLayout::clipboardXmlTag))
                    {
                        if (currentLayout != nullptr)
                            currentLayout->paste();
                    }
                    else if (doc->hasTagName (PaintRoutine::clipboardXmlTag))
                    {
                        if (currentPaintRoutine != nullptr)
                            currentPaintRoutine->paste();
                    }
                }
            }
            break;

        case StandardApplicationCommandIDs::del:
            if (currentLayout != nullptr)
                currentLayout->deleteSelected();
            else if (currentPaintRoutine != nullptr)
                currentPaintRoutine->deleteSelected();
            break;

        case StandardApplicationCommandIDs::selectAll:
            if (currentLayout != nullptr)
                currentLayout->selectAll();
            else if (currentPaintRoutine != nullptr)
                currentPaintRoutine->selectAll();
            break;

        case StandardApplicationCommandIDs::deselectAll:
            if (currentLayout != nullptr)
            {
                currentLayout->getSelectedSet().deselectAll();
            }
            else if (currentPaintRoutine != nullptr)
            {
                currentPaintRoutine->getSelectedElements().deselectAll();
                currentPaintRoutine->getSelectedPoints().deselectAll();
            }

            break;

        default:
            return false;
    }

    document->beginTransaction();
    return true;
}

bool JucerDocumentEditor::keyPressed (const KeyPress& key)
{
    if (key.isKeyCode (KeyPress::deleteKey) || key.isKeyCode (KeyPress::backspaceKey))
    {
        ProjucerApplication::getCommandManager().invokeDirectly (StandardApplicationCommandIDs::del, true);
        return true;
    }

    return false;
}

JucerDocumentEditor* JucerDocumentEditor::getActiveDocumentHolder()
{
    ApplicationCommandInfo info (0);
    return dynamic_cast<JucerDocumentEditor*> (ProjucerApplication::getCommandManager()
                                                  .getTargetForCommand (JucerCommandIDs::editCompLayout, info));
}

Image JucerDocumentEditor::createComponentLayerSnapshot() const
{
    if (compLayoutPanel != nullptr)
        return compLayoutPanel->createComponentSnapshot();

    return {};
}

const int gridSnapMenuItemBase = 0x8723620;
const int snapSizes[] = { 2, 3, 4, 5, 6, 8, 10, 12, 16, 20, 24, 32 };

void createGUIEditorMenu (PopupMenu&);
void createGUIEditorMenu (PopupMenu& menu)
{
    auto* commandManager = &ProjucerApplication::getCommandManager();

    menu.addCommandItem (commandManager, JucerCommandIDs::editCompLayout);
    menu.addCommandItem (commandManager, JucerCommandIDs::editCompGraphics);
    menu.addSeparator();

    PopupMenu newComps;
    for (int i = 0; i < ObjectTypes::numComponentTypes; ++i)
        newComps.addCommandItem (commandManager, JucerCommandIDs::newComponentBase + i);

    menu.addSubMenu ("Add new component", newComps);

    PopupMenu newElements;
    for (int i = 0; i < ObjectTypes::numElementTypes; ++i)
        newElements.addCommandItem (commandManager, JucerCommandIDs::newElementBase + i);

    menu.addSubMenu ("Add new graphic element", newElements);

    menu.addSeparator();
    menu.addCommandItem (commandManager, StandardApplicationCommandIDs::cut);
    menu.addCommandItem (commandManager, StandardApplicationCommandIDs::copy);
    menu.addCommandItem (commandManager, StandardApplicationCommandIDs::paste);
    menu.addCommandItem (commandManager, StandardApplicationCommandIDs::del);
    menu.addCommandItem (commandManager, StandardApplicationCommandIDs::selectAll);
    menu.addCommandItem (commandManager, StandardApplicationCommandIDs::deselectAll);
    menu.addSeparator();
    menu.addCommandItem (commandManager, JucerCommandIDs::toFront);
    menu.addCommandItem (commandManager, JucerCommandIDs::toBack);
    menu.addSeparator();
    menu.addCommandItem (commandManager, JucerCommandIDs::group);
    menu.addCommandItem (commandManager, JucerCommandIDs::ungroup);
    menu.addSeparator();
    menu.addCommandItem (commandManager, JucerCommandIDs::bringBackLostItems);

    menu.addSeparator();
    menu.addCommandItem (commandManager, JucerCommandIDs::showGrid);
    menu.addCommandItem (commandManager, JucerCommandIDs::enableSnapToGrid);

    auto* holder = JucerDocumentEditor::getActiveDocumentHolder();

    {
        auto currentSnapSize = holder != nullptr ? holder->getDocument()->getSnappingGridSize() : -1;
        PopupMenu m;

        for (int i = 0; i < numElementsInArray (snapSizes); ++i)
            m.addItem (gridSnapMenuItemBase + i, String (snapSizes[i]) + " pixels",
                       true, snapSizes[i] == currentSnapSize);

        menu.addSubMenu ("Grid size", m, currentSnapSize >= 0);
    }

    menu.addSeparator();
    menu.addCommandItem (commandManager, JucerCommandIDs::zoomIn);
    menu.addCommandItem (commandManager, JucerCommandIDs::zoomOut);
    menu.addCommandItem (commandManager, JucerCommandIDs::zoomNormal);

    menu.addSeparator();
    menu.addCommandItem (commandManager, JucerCommandIDs::test);

    menu.addSeparator();

    {
        PopupMenu overlays;
        overlays.addCommandItem (commandManager, JucerCommandIDs::compOverlay0);
        overlays.addCommandItem (commandManager, JucerCommandIDs::compOverlay33);
        overlays.addCommandItem (commandManager, JucerCommandIDs::compOverlay66);
        overlays.addCommandItem (commandManager, JucerCommandIDs::compOverlay100);

        menu.addSubMenu ("Component Overlay", overlays, holder != nullptr);
    }
}

void handleGUIEditorMenuCommand (int);
void handleGUIEditorMenuCommand (int menuItemID)
{
    if (auto* ed = JucerDocumentEditor::getActiveDocumentHolder())
    {
        int gridIndex = menuItemID - gridSnapMenuItemBase;

        if (isPositiveAndBelow (gridIndex, numElementsInArray (snapSizes)))
        {
            auto& doc = *ed->getDocument();

            doc.setSnappingGrid (snapSizes [gridIndex],
                                 doc.isSnapActive (false),
                                 doc.isSnapShown());
        }
    }
}

void registerGUIEditorCommands();
void registerGUIEditorCommands()
{
    JucerDocumentEditor dh (nullptr);
    ProjucerApplication::getCommandManager().registerAllCommandsForTarget (&dh);
}
