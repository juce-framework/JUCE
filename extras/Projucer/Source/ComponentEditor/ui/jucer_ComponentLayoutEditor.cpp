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

#include "../../jucer_Headers.h"
#include "../../Application/jucer_Application.h"
#include "jucer_ComponentLayoutEditor.h"
#include "../ui/jucer_JucerCommandIDs.h"
#include "../jucer_ObjectTypes.h"
#include "../components/jucer_JucerComponentHandler.h"

//==============================================================================
class SubComponentHolderComp    : public Component
{
public:
    SubComponentHolderComp (JucerDocument& doc,
                            SnapGridPainter& g)
       : document (doc), grid (g),
         dontFillBackground (false)
    {
        setInterceptsMouseClicks (false, false);
        setWantsKeyboardFocus (false);
        setFocusContainer (true);
    }

    void paint (Graphics& g) override
    {
        if (! dontFillBackground)
        {
            if (PaintRoutine* const background = document.getPaintRoutine (0))
            {
                background->fillWithBackground (g, false);
                background->drawElements (g, getLocalBounds());

                grid.draw (g, background);
            }
            else
            {
                grid.draw (g, nullptr);
            }
        }
    }

    void resized() override
    {
        if (! getBounds().isEmpty())
        {
            int numTimesToTry = 10;

            while (--numTimesToTry >= 0)
            {
                bool anyCompsMoved = false;

                for (int i = 0; i < getNumChildComponents(); ++i)
                {
                    Component* comp = getChildComponent (i);

                    if (ComponentTypeHandler* const type = ComponentTypeHandler::getHandlerFor (*comp))
                    {
                        const Rectangle<int> newBounds (type->getComponentPosition (comp)
                                                          .getRectangle (getLocalBounds(),
                                                                         document.getComponentLayout()));

                        anyCompsMoved = anyCompsMoved || (comp->getBounds() != newBounds);
                        comp->setBounds (newBounds);
                    }
                }

                // repeat this loop until they've all stopped shuffling (might require a few
                // loops for all the relative positioned comps to settle down)
                if (! anyCompsMoved)
                    break;
            }
        }
    }

    void moved() override
    {
        ((ComponentLayoutEditor*) getParentComponent())->updateOverlayPositions();
    }

    JucerDocument& document;
    SnapGridPainter& grid;

    bool dontFillBackground;
};

//==============================================================================
ComponentLayoutEditor::ComponentLayoutEditor (JucerDocument& doc, ComponentLayout& cl)
    : document (doc), layout (cl), firstResize (true)
{
    setWantsKeyboardFocus (true);

    addAndMakeVisible (subCompHolder = new SubComponentHolderComp (document, grid));

    refreshAllComponents();

    setSize (document.getInitialWidth(),
             document.getInitialHeight());
}

ComponentLayoutEditor::~ComponentLayoutEditor()
{
    document.removeChangeListener (this);

    removeChildComponent (&lassoComp);
    deleteAllChildren();
}

//==============================================================================
void ComponentLayoutEditor::visibilityChanged()
{
    document.beginTransaction();

    if (isVisible())
    {
        refreshAllComponents();
        document.addChangeListener (this);
    }
    else
    {
        document.removeChangeListener (this);
    }
}

void ComponentLayoutEditor::changeListenerCallback (ChangeBroadcaster*)
{
    refreshAllComponents();
}

void ComponentLayoutEditor::paint (Graphics&)
{
}

void ComponentLayoutEditor::resized()
{
    if (firstResize && getWidth() > 0 && getHeight() > 0)
    {
        firstResize = false;
        refreshAllComponents();
    }

    subCompHolder->setBounds (getComponentArea());
    updateOverlayPositions();
}

Rectangle<int> ComponentLayoutEditor::getComponentArea() const
{
    const int editorEdgeGap = 4;

    if (document.isFixedSize())
        return Rectangle<int> ((getWidth() - document.getInitialWidth()) / 2,
                               (getHeight() - document.getInitialHeight()) / 2,
                               document.getInitialWidth(),
                               document.getInitialHeight());

    return Rectangle<int> (editorEdgeGap, editorEdgeGap,
                           getWidth() - editorEdgeGap * 2,
                           getHeight() - editorEdgeGap * 2);
}

Image ComponentLayoutEditor::createComponentLayerSnapshot() const
{
    ((SubComponentHolderComp*) subCompHolder)->dontFillBackground = true;
    Image im = subCompHolder->createComponentSnapshot (Rectangle<int> (0, 0, subCompHolder->getWidth(), subCompHolder->getHeight()));
    ((SubComponentHolderComp*) subCompHolder)->dontFillBackground = false;

    return im;
}

void ComponentLayoutEditor::updateOverlayPositions()
{
    for (int i = getNumChildComponents(); --i >= 0;)
        if (ComponentOverlayComponent* const overlay = dynamic_cast<ComponentOverlayComponent*> (getChildComponent (i)))
            overlay->updateBoundsToMatchTarget();
}

void ComponentLayoutEditor::refreshAllComponents()
{
    for (int i = getNumChildComponents(); --i >= 0;)
    {
        ScopedPointer<ComponentOverlayComponent> overlay (dynamic_cast<ComponentOverlayComponent*> (getChildComponent (i)));

        if (overlay != nullptr && layout.containsComponent (overlay->target))
            overlay.release();
    }

    for (int i = subCompHolder->getNumChildComponents(); --i >= 0;)
    {
        Component* const comp = subCompHolder->getChildComponent (i);

        if (! layout.containsComponent (comp))
            subCompHolder->removeChildComponent (comp);
    }

    Component* lastComp = nullptr;
    Component* lastOverlay = nullptr;

    for (int i = layout.getNumComponents(); --i >= 0;)
    {
        Component* const c = layout.getComponent (i);
        jassert (c != nullptr);

        ComponentOverlayComponent* overlay = getOverlayCompFor (c);

        bool isNewOverlay = false;

        if (overlay == 0)
        {
            ComponentTypeHandler* const handler = ComponentTypeHandler::getHandlerFor (*c);
            jassert (handler != nullptr);

            overlay = handler->createOverlayComponent (c, layout);

            addAndMakeVisible (overlay);
            isNewOverlay = true;
        }

        if (lastOverlay != nullptr)
            overlay->toBehind (lastOverlay);
        else
            overlay->toFront (false);

        lastOverlay = overlay;

        subCompHolder->addAndMakeVisible (c);

        if (lastComp != nullptr)
            c->toBehind (lastComp);
        else
            c->toFront (false);

        lastComp = c;

        c->setWantsKeyboardFocus (false);
        c->setFocusContainer (true);

        if (isNewOverlay)
            overlay->updateBoundsToMatchTarget();
    }

    if (grid.updateFromDesign (document))
        subCompHolder->repaint();

    subCompHolder->setBounds (getComponentArea());
    subCompHolder->resized();
}

void ComponentLayoutEditor::mouseDown (const MouseEvent& e)
{
    if (e.mods.isPopupMenu())
    {
        ApplicationCommandManager* commandManager = &ProjucerApplication::getCommandManager();

        PopupMenu m;

        m.addCommandItem (commandManager, JucerCommandIDs::editCompLayout);
        m.addCommandItem (commandManager, JucerCommandIDs::editCompGraphics);
        m.addSeparator();

        for (int i = 0; i < ObjectTypes::numComponentTypes; ++i)
            m.addCommandItem (commandManager, JucerCommandIDs::newComponentBase + i);

        m.show();
    }
    else
    {
        addChildComponent (lassoComp);
        lassoComp.beginLasso (e, this);
    }
}

void ComponentLayoutEditor::mouseDrag (const MouseEvent& e)
{
    lassoComp.toFront (false);
    lassoComp.dragLasso (e);
}

void ComponentLayoutEditor::mouseUp (const MouseEvent& e)
{
    lassoComp.endLasso();
    removeChildComponent (&lassoComp);

    if (! (e.mouseWasDraggedSinceMouseDown() || e.mods.isAnyModifierKeyDown()))
        layout.getSelectedSet().deselectAll();
}

static void moveOrStretch (ComponentLayout& layout, int x, int y, bool snap, bool stretch)
{
    if (stretch)
        layout.stretchSelectedComps (x, y, snap);
    else
        layout.moveSelectedComps (x, y, snap);
}

bool ComponentLayoutEditor::keyPressed (const KeyPress& key)
{
    const bool snap = key.getModifiers().isAltDown();
    const bool stretch = key.getModifiers().isShiftDown();

    const int amount = snap ? document.getSnappingGridSize() + 1
                            : 1;

    if (key.isKeyCode (KeyPress::rightKey))
    {
        moveOrStretch (layout, amount, 0, snap, stretch);
    }
    else if (key.isKeyCode (KeyPress::downKey))
    {
        moveOrStretch (layout, 0,amount, snap, stretch);
    }
    else if (key.isKeyCode (KeyPress::leftKey))
    {
        moveOrStretch (layout, -amount, 0, snap, stretch);
    }
    else if (key.isKeyCode (KeyPress::upKey))
    {
        moveOrStretch (layout, 0, -amount, snap, stretch);
    }
    else
    {
        return false;
    }

    return true;
}

bool ComponentLayoutEditor::isInterestedInFileDrag (const StringArray& filenames)
{
    const File f (filenames [0]);
    return f.hasFileExtension (".cpp");
}

void ComponentLayoutEditor::filesDropped (const StringArray& filenames, int x, int y)
{
    const File f (filenames [0]);

    if (JucerDocument::isValidJucerCppFile (f))
    {
        JucerComponentHandler jucerDocHandler;
        layout.getDocument()->beginTransaction();

        if (TestComponent* newOne = dynamic_cast<TestComponent*> (layout.addNewComponent (&jucerDocHandler,
                                                                                          x - subCompHolder->getX(),
                                                                                          y - subCompHolder->getY())))
        {
            JucerComponentHandler::setJucerComponentFile (*layout.getDocument(), newOne,
                                                          f.getRelativePathFrom (document.getCppFile().getParentDirectory()));
            layout.getSelectedSet().selectOnly (newOne);
        }

        layout.getDocument()->beginTransaction();
    }
}

bool ComponentLayoutEditor::isInterestedInDragSource (const SourceDetails& dragSourceDetails)
{
    if (dragSourceDetails.description != projectItemDragType)
        return false;

    OwnedArray<Project::Item> selectedNodes;
    ProjectContentComponent::getSelectedProjectItemsBeingDragged (dragSourceDetails, selectedNodes);

    return selectedNodes.size() > 0;
}

void ComponentLayoutEditor::itemDropped (const SourceDetails& dragSourceDetails)
{
    OwnedArray <Project::Item> selectedNodes;
    ProjectContentComponent::getSelectedProjectItemsBeingDragged (dragSourceDetails, selectedNodes);

    StringArray filenames;

    for (int i = 0; i < selectedNodes.size(); ++i)
        if (selectedNodes.getUnchecked(i)->getFile().hasFileExtension (".cpp"))
            filenames.add (selectedNodes.getUnchecked(i)->getFile().getFullPathName());

    filesDropped (filenames, dragSourceDetails.localPosition.x, dragSourceDetails.localPosition.y);
}

ComponentOverlayComponent* ComponentLayoutEditor::getOverlayCompFor (Component* compToFind) const
{
    for (int i = getNumChildComponents(); --i >= 0;)
    {
        if (ComponentOverlayComponent* const overlay = dynamic_cast<ComponentOverlayComponent*> (getChildComponent (i)))
            if (overlay->target == compToFind)
                return overlay;
    }

    return nullptr;
}

void ComponentLayoutEditor::findLassoItemsInArea (Array <Component*>& results, const Rectangle<int>& area)
{
    const Rectangle<int> lasso (area - subCompHolder->getPosition());

    for (int i = 0; i < subCompHolder->getNumChildComponents(); ++i)
    {
        Component* c = subCompHolder->getChildComponent (i);

        if (c->getBounds().intersects (lasso))
            results.add (c);
    }
}

SelectedItemSet <Component*>& ComponentLayoutEditor::getLassoSelection()
{
    return layout.getSelectedSet();
}
