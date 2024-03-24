/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#include "../../Application/jucer_Headers.h"
#include "../../Application/jucer_Application.h"
#include "../UI/jucer_JucerCommandIDs.h"
#include "jucer_PaintRoutineEditor.h"
#include "../jucer_ObjectTypes.h"
#include "jucer_JucerDocumentEditor.h"

//==============================================================================
PaintRoutineEditor::PaintRoutineEditor (PaintRoutine& pr, JucerDocument& doc,
                                        JucerDocumentEditor* docHolder)
    : graphics (pr),
      document (doc),
      documentHolder (docHolder),
      componentOverlay (nullptr),
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
}

void PaintRoutineEditor::removeAllElementComps()
{
    for (int i = getNumChildComponents(); --i >= 0;)
        if (PaintElement* const e = dynamic_cast<PaintElement*> (getChildComponent (i)))
            removeChildComponent (e);
}

Rectangle<int> PaintRoutineEditor::getComponentArea() const
{
    if (document.isFixedSize())
        return Rectangle<int> ((getWidth() - document.getInitialWidth()) / 2,
                               (getHeight() - document.getInitialHeight()) / 2,
                               document.getInitialWidth(),
                               document.getInitialHeight());

    return getLocalBounds().reduced (4);
}

//==============================================================================
void PaintRoutineEditor::paint (Graphics& g)
{
    const Rectangle<int> clip (getComponentArea());

    g.reduceClipRegion (clip);
    g.setOrigin (clip.getPosition());

    graphics.fillWithBackground (g, true);
    grid.draw (g, &graphics);
}

void PaintRoutineEditor::paintOverChildren (Graphics& g)
{
    if (componentOverlay.isNull() && document.getComponentOverlayOpacity() > 0.0f)
        updateComponentOverlay();

    if (componentOverlay.isValid())
    {
        const Rectangle<int> clip (getComponentArea());
        g.drawImageAt (componentOverlay, clip.getX(), clip.getY());
    }
}

void PaintRoutineEditor::resized()
{
    if (getWidth() > 0 && getHeight() > 0)
    {
        componentOverlay = Image();
        refreshAllElements();
    }
}

void PaintRoutineEditor::updateChildBounds()
{
    const Rectangle<int> clip (getComponentArea());

    for (int i = 0; i < getNumChildComponents(); ++i)
        if (PaintElement* const e = dynamic_cast<PaintElement*> (getChildComponent (i)))
            e->updateBounds (clip);
}

void PaintRoutineEditor::updateComponentOverlay()
{
    if (componentOverlay.isValid())
        repaint();

    componentOverlay = Image();
    componentOverlayOpacity = document.getComponentOverlayOpacity();

    if (componentOverlayOpacity > 0.0f)
    {
        if (documentHolder != nullptr)
            componentOverlay = documentHolder->createComponentLayerSnapshot();

        if (componentOverlay.isValid())
        {
            componentOverlay.multiplyAllAlphas (componentOverlayOpacity);
            repaint();
        }
    }
}

void PaintRoutineEditor::visibilityChanged()
{
    document.beginTransaction();

    if (isVisible())
    {
        refreshAllElements();
        document.addChangeListener (this);
    }
    else
    {
        document.removeChangeListener (this);
        componentOverlay = Image();
    }
}

void PaintRoutineEditor::refreshAllElements()
{
    for (int i = getNumChildComponents(); --i >= 0;)
        if (auto* e = dynamic_cast<PaintElement*> (getChildComponent (i)))
            if (! graphics.containsElement (e))
                removeChildComponent (e);

    Component* last = nullptr;

    for (int i = graphics.getNumElements(); --i >= 0;)
    {
        auto* e = graphics.getElement (i);

        addAndMakeVisible (e);

        if (last != nullptr)
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
        repaint();
    }

    if (! approximatelyEqual (componentOverlayOpacity, document.getComponentOverlayOpacity()))
    {
        componentOverlay = Image();
        componentOverlayOpacity = document.getComponentOverlayOpacity();
        repaint();
    }
}

void PaintRoutineEditor::changeListenerCallback (ChangeBroadcaster*)
{
    refreshAllElements();
}

void PaintRoutineEditor::mouseDown (const MouseEvent& e)
{
    if (e.mods.isPopupMenu())
    {
        ApplicationCommandManager* commandManager = &ProjucerApplication::getCommandManager();

        PopupMenu m;

        m.addCommandItem (commandManager, JucerCommandIDs::editCompLayout);
        m.addCommandItem (commandManager, JucerCommandIDs::editCompGraphics);
        m.addSeparator();

        for (int i = 0; i < ObjectTypes::numElementTypes; ++i)
            m.addCommandItem (commandManager, JucerCommandIDs::newElementBase + i);

        m.showMenuAsync (PopupMenu::Options());
    }
    else
    {
        addChildComponent (lassoComp);
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

    if (! (e.mouseWasDraggedSinceMouseDown() || e.mods.isAnyModifierKeyDown()))
    {
        graphics.getSelectedElements().deselectAll();
        graphics.getSelectedPoints().deselectAll();
    }
}

void PaintRoutineEditor::findLassoItemsInArea (Array <PaintElement*>& results, const Rectangle<int>& lasso)
{
    for (int i = 0; i < getNumChildComponents(); ++i)
        if (PaintElement* const e = dynamic_cast<PaintElement*> (getChildComponent (i)))
            if (e->getBounds().expanded (-e->borderThickness).intersects (lasso))
                results.add (e);
}

SelectedItemSet <PaintElement*>& PaintRoutineEditor::getLassoSelection()
{
    return graphics.getSelectedElements();
}

bool PaintRoutineEditor::isInterestedInFileDrag (const StringArray& files)
{
    return File::createFileWithoutCheckingPath (files[0])
             .hasFileExtension ("jpg;jpeg;png;gif;svg");
}

void PaintRoutineEditor::filesDropped (const StringArray& filenames, int x, int y)
{
    const File f (filenames [0]);

    if (f.existsAsFile())
    {
        std::unique_ptr<Drawable> d (Drawable::createFromImageFile (f));

        if (d != nullptr)
        {
            d.reset();

            document.beginTransaction();

            graphics.dropImageAt (f,
                                  jlimit (10, getWidth() - 10, x),
                                  jlimit (10, getHeight() - 10, y));

            document.beginTransaction();
        }
    }
}
