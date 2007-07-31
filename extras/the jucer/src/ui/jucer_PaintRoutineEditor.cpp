/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../jucer_Headers.h"
#include "jucer_PaintRoutineEditor.h"
#include "../model/jucer_ObjectTypes.h"
#include "jucer_JucerDocumentHolder.h"


//==============================================================================
PaintRoutineEditor::PaintRoutineEditor (PaintRoutine& graphics_,
                                        JucerDocument& document_,
                                        JucerDocumentHolder* const docHolder)
    : graphics (graphics_),
      document (document_),
      documentHolder (docHolder),
      componentOverlay (0),
      componentOverlayOpacity (0.0f)
{
    refreshAllElements();

    setSize (document.getInitialWidth(),
             document.getInitialHeight());
}

PaintRoutineEditor::~PaintRoutineEditor()
{
    document.removeChangeListener (this);

    removeAllElementComps();

    removeChildComponent (&lassoComp);
    deleteAllChildren();

    delete componentOverlay;
}

void PaintRoutineEditor::removeAllElementComps()
{
    for (int i = getNumChildComponents(); --i >= 0;)
    {
        PaintElement* const e = dynamic_cast <PaintElement*> (getChildComponent (i));

        if (e != 0)
            removeChildComponent (e);
    }
}

const Rectangle PaintRoutineEditor::getComponentArea() const
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

//==============================================================================
void PaintRoutineEditor::paint (Graphics& g)
{
    const Rectangle clip (getComponentArea());

    g.setOrigin (clip.getX(), clip.getY());
    g.reduceClipRegion (0, 0, clip.getWidth(), clip.getHeight());

    graphics.fillWithBackground (g, true);
    grid.draw (g, &graphics);
}

void PaintRoutineEditor::paintOverChildren (Graphics& g)
{
    if (componentOverlay == 0 && document.getComponentOverlayOpacity() > 0.0f)
        updateComponentOverlay();

    if (componentOverlay != 0)
    {
        const Rectangle clip (getComponentArea());
        g.drawImageAt (componentOverlay, clip.getX(), clip.getY());
    }
}

void PaintRoutineEditor::resized()
{
    if (getWidth() > 0 && getHeight() > 0)
    {
        deleteAndZero (componentOverlay);
        refreshAllElements();
    }
}

void PaintRoutineEditor::updateChildBounds()
{
    const Rectangle clip (getComponentArea());

    for (int i = 0; i < getNumChildComponents(); ++i)
    {
        PaintElement* const e = dynamic_cast <PaintElement*> (getChildComponent (i));

        if (e != 0)
            e->updateBounds (clip);
    }
}

void PaintRoutineEditor::updateComponentOverlay()
{
    if (componentOverlay != 0)
        repaint();

    deleteAndZero (componentOverlay);

    componentOverlayOpacity = document.getComponentOverlayOpacity();

    if (componentOverlayOpacity > 0.0f)
    {
        if (documentHolder != 0)
            componentOverlay = documentHolder->createComponentLayerSnapshot();

        if (componentOverlay != 0)
        {
            componentOverlay->multiplyAllAlphas (componentOverlayOpacity);
            repaint();
        }
    }
}

void PaintRoutineEditor::visibilityChanged()
{
    document.getUndoManager().beginNewTransaction();

    if (isVisible())
    {
        refreshAllElements();
        document.addChangeListener (this);
    }
    else
    {
        document.removeChangeListener (this);
        deleteAndZero (componentOverlay);
    }
}

void PaintRoutineEditor::refreshAllElements()
{
    int i;
    for (i = getNumChildComponents(); --i >= 0;)
    {
        PaintElement* const e = dynamic_cast <PaintElement*> (getChildComponent (i));

        if (e != 0 && ! graphics.containsElement (e))
            removeChildComponent (e);
    }

    Component* last = 0;

    for (i = graphics.getNumElements(); --i >= 0;)
    {
        PaintElement* const e = graphics.getElement (i);

        addAndMakeVisible (e);

        if (last != 0)
            e->toBehind (last);
        else
            e->toFront (false);

        last = e;
    }

    updateChildBounds();

    if (grid.updateFromDesign (document))
        repaint();

    if (currentBackgroundColour != graphics.getBackgroundColour())
    {
        currentBackgroundColour = graphics.getBackgroundColour();
        grid.updateColour();
        repaint();
    }

    if (componentOverlayOpacity != document.getComponentOverlayOpacity())
    {
        deleteAndZero (componentOverlay);
        componentOverlayOpacity = document.getComponentOverlayOpacity();
        repaint();
    }
}

void PaintRoutineEditor::changeListenerCallback (void* source)
{
    refreshAllElements();
}

void PaintRoutineEditor::mouseDown (const MouseEvent& e)
{
    if (e.mods.isPopupMenu())
    {
        PopupMenu m;

        m.addCommandItem (commandManager, CommandIDs::editCompLayout);
        m.addCommandItem (commandManager, CommandIDs::editCompGraphics);
        m.addSeparator();

        for (int i = 0; i < ObjectTypes::numElementTypes; ++i)
            m.addCommandItem (commandManager, CommandIDs::newElementBase + i);

        m.show();
    }
    else
    {
        addChildComponent (&lassoComp);
        lassoComp.beginLasso (e, this);
    }
}

void PaintRoutineEditor::mouseDrag (const MouseEvent& e)
{
    lassoComp.toFront (false);
    lassoComp.dragLasso (e);
}

void PaintRoutineEditor::mouseUp (const MouseEvent& e)
{
    lassoComp.endLasso();

    if (e.mouseWasClicked() && ! e.mods.isAnyModifierKeyDown())
    {
        graphics.getSelectedElements().deselectAll();
        graphics.getSelectedPoints().deselectAll();
    }
}

void PaintRoutineEditor::findLassoItemsInArea (Array <PaintElement*>& results,
                                               int x, int y, int w, int h)
{
    const Rectangle lasso (x, y, w, h);

    for (int i = 0; i < getNumChildComponents(); ++i)
    {
        PaintElement* const e = dynamic_cast <PaintElement*> (getChildComponent (i));

        if (e != 0 && e->getBounds().expanded (-e->borderThickness, -e->borderThickness)
                                    .intersects (lasso))
            results.add (e);
    }
}

SelectedItemSet <PaintElement*>& PaintRoutineEditor::getLassoSelection()
{
    return graphics.getSelectedElements();
}

bool PaintRoutineEditor::filesDropped (const StringArray& filenames, int x, int y)
{
    File f (filenames [0]);

    if (f.existsAsFile())
    {
        Drawable* d = Drawable::createFromImageFile (f);

        if (d != 0)
        {
            delete d;

            document.getUndoManager().beginNewTransaction();

            graphics.dropImageAt (f,
                                  jlimit (10, getWidth() - 10, x),
                                  jlimit (10, getHeight() - 10, y));

            document.getUndoManager().beginNewTransaction();

            return true;
        }
    }

    return false;
}
