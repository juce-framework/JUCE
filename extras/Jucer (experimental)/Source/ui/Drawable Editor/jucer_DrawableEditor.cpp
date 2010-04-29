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
#include "jucer_DrawableObjectComponent.h"
#include "jucer_DrawableEditor.h"
#include "../jucer_JucerTreeViewBase.h"
#include "jucer_DrawableTreeViewItem.h"


//==============================================================================
class RightHandPanel  : public Component
{
public:
    RightHandPanel (DrawableEditor& editor_)
      : editor (editor_)
    {
        setOpaque (true);

        addAndMakeVisible (tree = new TreeView());
        tree->setRootItemVisible (true);
        tree->setMultiSelectEnabled (true);
        tree->setDefaultOpenness (true);
        tree->setColour (TreeView::backgroundColourId, Colours::white);
        tree->setIndentSize (15);
        tree->setRootItem (DrawableTreeViewItem::createItemForNode (editor, editor.getDocument().getRootDrawableNode()));
    }

    ~RightHandPanel()
    {
        tree->deleteRootItem();
        deleteAllChildren();
    }

    void paint (Graphics& g)
    {
        g.fillAll (Colour::greyLevel (0.92f));
    }

    void resized()
    {
        tree->setSize (getWidth(), getHeight());
    }

private:
    DrawableEditor& editor;
    TreeView* tree;
};

//==============================================================================
DrawableEditor::DrawableEditor (OpenDocumentManager::Document* document,
                                Project* project_,
                                DrawableDocument* drawableDocument_)
    : DocumentEditorComponent (document),
      project (project_),
      drawableDocument (drawableDocument_)
{
    jassert (drawableDocument_ != 0);

    setOpaque (true);

    addAndMakeVisible (rightHandPanel = new RightHandPanel (*this));

    Canvas* canvas = new Canvas (*this);
    addAndMakeVisible (viewport = new Viewport());
    viewport->setViewedComponent (canvas);
    canvas->createRootObject();
}

DrawableEditor::~DrawableEditor()
{
    deleteAllChildren();
}

int64 DrawableEditor::getHashForNode (const ValueTree& node)
{
    return node ["id"].toString().hashCode64();
}

void DrawableEditor::paint (Graphics& g)
{
    g.fillAll (Colours::white);
}

void DrawableEditor::resized()
{
    rightHandPanel->setBounds (getWidth() - 200, 0, 200, getHeight());
    viewport->setBounds (0, 0, rightHandPanel->getX(), getHeight());
    getCanvas()->updateSize();
}

//==============================================================================
DrawableEditor::Canvas::Canvas (DrawableEditor& editor_)
   : editor (editor_), border (40)
{
    origin.setXY (50, 50);
}

DrawableEditor::Canvas::~Canvas()
{
    rootObject = 0;
}

void DrawableEditor::Canvas::createRootObject()
{
    addAndMakeVisible (rootObject = DrawableObjectComponent::create (editor.getDocument().getRootDrawableNode(),
                                                                     editor, 0));
    rootObject->drawableOriginRelativeToParentTopLeft = origin;
    rootObject->reloadFromValueTree();
}

void DrawableEditor::Canvas::paint (Graphics& g)
{
/*    g.setColour (Colours::lightgrey);

    g.fillRect (0, border.getTop() - 1, getWidth(), 1);
    g.fillRect (0, getHeight() - border.getBottom(), getWidth(), 1);
    g.fillRect (border.getLeft() - 1, 0, 1, getHeight());
    g.fillRect (getWidth() - border.getRight(), 0, 1, getHeight());*/

    g.setColour (Colours::grey);
    g.fillRect (0, origin.getY(), getWidth(), 1);
    g.fillRect (origin.getX(), 0, 1, getHeight());
}

void DrawableEditor::Canvas::mouseDown (const MouseEvent& e)
{
    lasso = 0;

    if (e.mods.isPopupMenu())
    {
        PopupMenu m;
        m.addItem (1, "New Rectangle");
        m.addItem (2, "New Circle");

        switch (m.show())
        {
        case 1:
            editor.getDocument().addRectangle();
            break;

        case 2:
            editor.getDocument().addCircle();
            break;

        default:
            break;
        }
    }
    else
    {
        addAndMakeVisible (lasso = new LassoComponent <int64>());
        lasso->beginLasso (e, this);
    }
}

void DrawableEditor::Canvas::mouseDrag (const MouseEvent& e)
{
    if (lasso != 0)
        lasso->dragLasso (e);
}

void DrawableEditor::Canvas::mouseUp (const MouseEvent& e)
{
    if (lasso != 0)
    {
        lasso->endLasso();
        lasso = 0;
    }
}

void DrawableEditor::Canvas::findLassoItemsInArea (Array <int64>& itemsFound, const Rectangle<int>& area)
{
    for (int i = getNumChildComponents(); --i >= 0;)
    {
        DrawableObjectComponent* d = dynamic_cast <DrawableObjectComponent*> (getChildComponent(i));

        if (d != 0)
            d->findLassoItemsInArea (itemsFound, area);
    }
}

SelectedItemSet <int64>& DrawableEditor::Canvas::getLassoSelection()
{
    return editor.selectedItems;
}

void DrawableEditor::Canvas::updateSize()
{
    Rectangle<int> r (rootObject->getBounds());

    setSize (jmax (editor.viewport->getMaximumVisibleWidth(), r.getRight()),
             jmax (editor.viewport->getMaximumVisibleHeight(), r.getBottom()));
}

void DrawableEditor::Canvas::childBoundsChanged (Component* child)
{
    if (child == rootObject)
        updateSize();
}
