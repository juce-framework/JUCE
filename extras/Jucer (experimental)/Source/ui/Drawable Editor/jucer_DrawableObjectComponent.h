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

#ifndef __JUCER_DRAWABLEOBJECTCOMPONENT_JUCEHEADER__
#define __JUCER_DRAWABLEOBJECTCOMPONENT_JUCEHEADER__

#include "jucer_DrawableEditor.h"


//==============================================================================
class DrawableObjectComponent  : public Component,
                                 public ValueTree::Listener,
                                 public ChangeListener
{
public:
    DrawableObjectComponent (const ValueTree& drawableNode_,
                             DrawableEditor& editor_,
                             Drawable* drawable_)
        : drawable (drawable_),
          drawableNode (drawableNode_),
          editor (editor_)
    {
        setVisible (true);

        nodeHashCode = DrawableEditor::getHashForNode (drawableNode);

        drawableNode.addListener (this);
        editor.selectedItems.addChangeListener (this);
    }

    ~DrawableObjectComponent()
    {
        deleteAllChildren();
        editor.selectedItems.removeChangeListener (this);
        drawableNode.removeListener (this);
        drawable = 0;
    }

    static DrawableObjectComponent* create (const ValueTree& node, DrawableEditor& editor, Drawable* drawable_);

    //==============================================================================
    void paint (Graphics& g)
    {
        //g.setColour (Colours::pink.withAlpha (0.2f));
        //g.fillRect (0, 0, getWidth(), getHeight());

        if (isSelected())
        {
            g.setColour (Colours::red);
            g.drawRect (0, 0, getWidth(), getHeight(), 2);
        }

        const Point<int> offset (getDrawableOriginRelativeToTopLeft());
        g.setOrigin (offset.getX(), offset.getY());
        drawable->draw (g, 1.0f, transform);
    }

    const Point<int> getDrawableOriginRelativeToTopLeft() const
    {
        return drawableOriginRelativeToParentTopLeft - getPosition();
    }

    void findLassoItemsInArea (Array <int64>& itemsFound, Rectangle<int> r) const
    {
        if (getBounds().intersects (r))
            itemsFound.add (nodeHashCode);

        r.translate (-getX(), -getY());

        for (int i = getNumChildComponents(); --i >= 0;)
        {
            DrawableObjectComponent* d = dynamic_cast <DrawableObjectComponent*> (getChildComponent(i));

            if (d != 0)
                d->findLassoItemsInArea (itemsFound, r);
        }
    }

    bool isSelected() const
    {
        return editor.selectedItems.isSelected (nodeHashCode);
    }

    //==============================================================================
    void valueTreePropertyChanged (ValueTree& tree, const var::identifier& property)
    {
        if (tree == drawableNode)
        {
            editor.getDocument().changed();
            reloadFromValueTree();
            repaint();
        }
    }

    void valueTreeChildrenChanged (ValueTree& tree)
    {
        if (tree == drawableNode)
        {
            editor.getDocument().changed();
            reloadFromValueTree();
            repaint();
        }
    }

    void valueTreeParentChanged (ValueTree& tree)
    {
        editor.getDocument().changed();
        reloadFromValueTree();
        repaint();
    }

    void changeListenerCallback (void*)
    {
        repaint();
    }

    //==============================================================================
    virtual void reloadFromValueTree() = 0;

    void commitModifiedPath()
    {
        drawableNode = drawable->createValueTree();
    }

    //==============================================================================
    AffineTransform transform;
    ScopedPointer <Drawable> drawable;
    ValueTree drawableNode;
    int64 nodeHashCode;
    DrawableEditor& editor;
    Point<int> drawableOriginRelativeToParentTopLeft;
};

//==============================================================================
class PathDrawableComponent  : public DrawableObjectComponent
{
public:
    PathDrawableComponent (const ValueTree& drawableNode_,
                           DrawableEditor& editor_,
                           DrawablePath* drawable_)
        : DrawableObjectComponent (drawableNode_, editor_, drawable_)
    {
    }

    ~PathDrawableComponent()
    {
    }

    // Relative to the drawable's origin, not the parent component or any other comp.
    const Rectangle<int> getBoundsRectangle()
    {
        if (drawable == 0)
            reloadFromValueTree();

        return drawable->getBounds().getSmallestIntegerContainer().expanded (2, 2);
    }

    bool hitTest (int x, int y)
    {
        const Point<int> offset (getDrawableOriginRelativeToTopLeft());

        return drawable->hitTest ((float) x - offset.getX(),
                                  (float) y - offset.getY());
    }

    void mouseDown (const MouseEvent& e)
    {
        mouseDownSelectResult = editor.selectedItems.addToSelectionOnMouseDown (nodeHashCode, e.mods);
        pathBoundsOnMouseDown = getPath()->getPath().getBounds();
    }

    void mouseDrag (const MouseEvent& e)
    {
    }

    void mouseUp (const MouseEvent& e)
    {
        editor.selectedItems.addToSelectionOnMouseUp (nodeHashCode, e.mods,
                                                      ! e.mouseWasClicked(),
                                                      mouseDownSelectResult);
    }

    void reloadFromValueTree()
    {
        drawable = Drawable::createFromValueTree (drawableNode);
        jassert (dynamic_cast <DrawablePath*> ((Drawable*) drawable) != 0);

        setBounds (getBoundsRectangle().translated (drawableOriginRelativeToParentTopLeft.getX(),
                                                    drawableOriginRelativeToParentTopLeft.getY()));
    }

private:
    bool mouseDownSelectResult;
    Rectangle<float> pathBoundsOnMouseDown;

    DrawablePath* getPath() const
    {
        return dynamic_cast <DrawablePath*> ((Drawable*) drawable);
    }
};

//==============================================================================
class CompositeDrawableComponent  : public DrawableObjectComponent
{
public:
    CompositeDrawableComponent (const ValueTree& drawableNode_,
                                DrawableEditor& editor_,
                                DrawableComposite* drawable_)
        : DrawableObjectComponent (drawableNode_, editor_, drawable_)
    {
    }

    ~CompositeDrawableComponent()
    {
    }

    void reloadFromValueTree()
    {
        drawable = Drawable::createFromValueTree (drawableNode);

        DrawableComposite* dc = dynamic_cast <DrawableComposite*> ((Drawable*) drawable);
        jassert (dc != 0);

        deleteAllChildren();
        Rectangle<int> childBounds;

        int i;
        for (i = 0; i < dc->getNumDrawables(); ++i)
        {
            Drawable* d = dc->getDrawable (i);
            jassert (d != 0);

            DrawableObjectComponent* c = DrawableObjectComponent::create (drawableNode.getChild(i), editor, d);

            if (c != 0)
            {
                addChildComponent (c);

                const AffineTransform* t = dc->getDrawableTransform (i);
                if (t != 0)
                    c->transform = *t;

                c->drawableOriginRelativeToParentTopLeft = drawableOriginRelativeToParentTopLeft;
                c->reloadFromValueTree();

                if (childBounds.isEmpty())
                    childBounds = c->getBounds();
                else
                    childBounds = childBounds.getUnion (c->getBounds());
            }
        }

        for (i = dc->getNumDrawables(); --i >= 0;)
            dc->removeDrawable (i, false);

        setBounds (childBounds);

        for (i = getNumChildComponents(); --i >= 0;)
        {
            DrawableObjectComponent* dc = dynamic_cast <DrawableObjectComponent*> (getChildComponent (i));

            if (dc != 0)
            {
                dc->setTopLeftPosition (dc->getX() - getX(), dc->getY() - getY());
                dc->drawableOriginRelativeToParentTopLeft = drawableOriginRelativeToParentTopLeft - getPosition();
            }
        }
    }
};

//==============================================================================
DrawableObjectComponent* DrawableObjectComponent::create (const ValueTree& node, DrawableEditor& editor, Drawable* drawable_)
{
    ScopedPointer<Drawable> drawable (drawable_);

    if (drawable == 0)
        drawable = Drawable::createFromValueTree (node);

    DrawablePath* p = dynamic_cast <DrawablePath*> ((Drawable*) drawable);

    if (p != 0)
    {
        drawable.release();
        return new PathDrawableComponent (node, editor, p);
    }

    DrawableComposite* dc = dynamic_cast <DrawableComposite*> ((Drawable*) drawable);

    if (dc != 0)
    {
        drawable.release();
        return new CompositeDrawableComponent (node, editor, dc);
    }

    jassertfalse
    return 0;
}


#endif   // __JUCER_DRAWABLEOBJECTCOMPONENT_JUCEHEADER__
