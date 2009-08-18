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
#include "jucer_ComponentLayoutEditor.h"
#include "../model/jucer_ObjectTypes.h"
#include "../model/components/jucer_JucerComponentHandler.h"

//==============================================================================
class SubComponentHolderComp    : public Component
{
public:
    SubComponentHolderComp (JucerDocument& document_,
                            SnapGridPainter& grid_)
       : document (document_),
         grid (grid_),
         dontFillBackground (false)
    {
        setInterceptsMouseClicks (false, false);
        setWantsKeyboardFocus (false);
        setFocusContainer (true);
    }

    ~SubComponentHolderComp()
    {
    }

    void paint (Graphics& g)
    {
        if (! dontFillBackground)
        {
            PaintRoutine* const background = document.getPaintRoutine (0);

            if (background != 0)
            {
                background->fillWithBackground (g, false);
                background->drawElements (g, Rectangle (0, 0, getWidth(), getHeight()));
            }

            grid.draw (g, background);
        }
    }

    void resized()
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
                    ComponentTypeHandler* const type = ComponentTypeHandler::getHandlerFor (*comp);

                    if (type != 0)
                    {
                        const Rectangle newBounds (type->getComponentPosition (comp)
                                                    .getRectangle (Rectangle (0, 0, getWidth(), getHeight()),
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

    void moved()
    {
        ((ComponentLayoutEditor*) getParentComponent())->updateOverlayPositions();
    }

    JucerDocument& document;
    SnapGridPainter& grid;
    bool dontFillBackground;
};

//==============================================================================
ComponentLayoutEditor::ComponentLayoutEditor (JucerDocument& document_, ComponentLayout& layout_)
    : document (document_),
      layout (layout_),
      firstResize (true)
{
    setWantsKeyboardFocus (true);

    addAndMakeVisible (subCompHolder = new SubComponentHolderComp (document, grid));

    refreshAllComponents();

    setSize (document.getInitialWidth(), document.getInitialHeight());
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
    document.getUndoManager().beginNewTransaction();

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

void ComponentLayoutEditor::changeListenerCallback (void*)
{
    refreshAllComponents();
}

void ComponentLayoutEditor::paint (Graphics& g)
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

const Rectangle ComponentLayoutEditor::getComponentArea() const
{
    if (document.isFixedSize())
    {
        return Rectangle ((getWidth() - document.getInitialWidth()) / 2,
                          (getHeight() - document.getInitialHeight()) / 2,
                          document.getInitialWidth(),
                          document.getInitialHeight());
    }
    else
    {
        return Rectangle (editorEdgeGap, editorEdgeGap,
                          getWidth() - editorEdgeGap * 2,
                          getHeight() - editorEdgeGap * 2);
    }
}

Image* ComponentLayoutEditor::createComponentLayerSnapshot() const
{
    ((SubComponentHolderComp*) subCompHolder)->dontFillBackground = true;
    Image* const im = subCompHolder->createComponentSnapshot (Rectangle (0, 0, subCompHolder->getWidth(), subCompHolder->getHeight()));
    ((SubComponentHolderComp*) subCompHolder)->dontFillBackground = false;

    return im;
}

void ComponentLayoutEditor::updateOverlayPositions()
{
    for (int i = getNumChildComponents(); --i >= 0;)
    {
        ComponentOverlayComponent* const overlay = dynamic_cast <ComponentOverlayComponent*> (getChildComponent (i));

        if (overlay != 0)
            overlay->updateBoundsToMatchTarget();
    }
}

void ComponentLayoutEditor::refreshAllComponents()
{
    int i;
    for (i = getNumChildComponents(); --i >= 0;)
    {
        ComponentOverlayComponent* const overlay = dynamic_cast <ComponentOverlayComponent*> (getChildComponent (i));

        if (overlay != 0 && ! layout.containsComponent (overlay->target))
            delete overlay;
    }

    for (i = subCompHolder->getNumChildComponents(); --i >= 0;)
    {
        Component* const comp = subCompHolder->getChildComponent (i);

        if (! layout.containsComponent (comp))
            subCompHolder->removeChildComponent (comp);
    }

    Component* lastComp = 0;
    Component* lastOverlay = 0;

    for (i = layout.getNumComponents(); --i >= 0;)
    {
        Component* const c = layout.getComponent (i);
        ComponentOverlayComponent* overlay = getOverlayCompFor (c);

        bool isNewOverlay = false;

        if (overlay == 0)
        {
            ComponentTypeHandler* const handler = ComponentTypeHandler::getHandlerFor (*c);
            jassert (handler != 0);

            overlay = handler->createOverlayComponent (c, layout);

            addAndMakeVisible (overlay);
            isNewOverlay = true;
        }

        if (lastOverlay != 0)
            overlay->toBehind (lastOverlay);
        else
            overlay->toFront (false);

        lastOverlay = overlay;

        subCompHolder->addAndMakeVisible (c);

        if (lastComp != 0)
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
        PopupMenu m;

        m.addCommandItem (commandManager, CommandIDs::editCompLayout);
        m.addCommandItem (commandManager, CommandIDs::editCompGraphics);
        m.addSeparator();

        for (int i = 0; i < ObjectTypes::numComponentTypes; ++i)
            m.addCommandItem (commandManager, CommandIDs::newComponentBase + i);

        m.show();
    }
    else
    {
        addChildComponent (&lassoComp);
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

    if (e.mouseWasClicked() && ! e.mods.isAnyModifierKeyDown())
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
    return f.hasFileExtension (T(".cpp"));
}

void ComponentLayoutEditor::filesDropped (const StringArray& filenames, int x, int y)
{
    const File f (filenames [0]);

    if (f.hasFileExtension (T(".cpp")))
    {
        JucerDocument* doc = ObjectTypes::loadDocumentFromFile (f, false);

        if (doc != 0)
        {
            delete doc;

            JucerComponentHandler jucerDocHandler;
            layout.getDocument()->getUndoManager().beginNewTransaction();

            TestComponent* newOne
                = dynamic_cast <TestComponent*> (layout.addNewComponent (&jucerDocHandler,
                                                                         x - subCompHolder->getX(),
                                                                         y - subCompHolder->getY()));

            if (newOne != 0)
            {
                JucerComponentHandler::setJucerComponentFile (*layout.getDocument(), newOne, f.getRelativePathFrom (document.getFile().getParentDirectory()));

                layout.getSelectedSet().selectOnly (newOne);
            }

            layout.getDocument()->getUndoManager().beginNewTransaction();
        }
    }
}

ComponentOverlayComponent* ComponentLayoutEditor::getOverlayCompFor (Component* compToFind) const
{
    for (int i = getNumChildComponents(); --i >= 0;)
    {
        ComponentOverlayComponent* const overlay = dynamic_cast <ComponentOverlayComponent*> (getChildComponent (i));

        if (overlay != 0 && overlay->target == compToFind)
            return overlay;
    }

    return 0;
}

void ComponentLayoutEditor::findLassoItemsInArea (Array <Component*>& results,
                                                  int x, int y, int w, int h)
{
    const Rectangle lasso (x - subCompHolder->getX(), y - subCompHolder->getY(), w, h);

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
