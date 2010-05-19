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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_DrawableComposite.h"
#include "juce_DrawablePath.h"
#include "juce_DrawableImage.h"
#include "juce_DrawableText.h"
#include "../imaging/juce_Image.h"


//==============================================================================
DrawableComposite::DrawableComposite()
{
    controlPoints[1].setXY (1.0f, 0.0f);
    controlPoints[2].setXY (0.0f, 1.0f);
}

DrawableComposite::~DrawableComposite()
{
}

//==============================================================================
void DrawableComposite::insertDrawable (Drawable* drawable, const int index)
{
    if (drawable != 0)
    {
        jassert (! drawables.contains (drawable)); // trying to add a drawable that's already in here!
        drawables.insert (index, drawable);
    }
}

void DrawableComposite::insertDrawable (const Drawable& drawable, const int index)
{
    insertDrawable (drawable.createCopy(), index);
}

void DrawableComposite::removeDrawable (const int index, const bool deleteDrawable)
{
    drawables.remove (index, deleteDrawable);
}

void DrawableComposite::bringToFront (const int index)
{
    if (index >= 0 && index < drawables.size() - 1)
        drawables.move (index, -1);
}

void DrawableComposite::setTransform (const Point<float>& targetPositionForOrigin,
                                      const Point<float>& targetPositionForX1Y0,
                                      const Point<float>& targetPositionForX0Y1)
{
    controlPoints[0] = targetPositionForOrigin;
    controlPoints[1] = targetPositionForX1Y0;
    controlPoints[2] = targetPositionForX0Y1;
}

//==============================================================================
const AffineTransform DrawableComposite::getTransform() const
{
    return AffineTransform::fromTargetPoints (controlPoints[0].getX(), controlPoints[0].getY(),
                                              controlPoints[1].getX(), controlPoints[1].getY(),
                                              controlPoints[2].getX(), controlPoints[2].getY());
}

void DrawableComposite::render (const Drawable::RenderingContext& context) const
{
    if (drawables.size() > 0 && context.opacity > 0)
    {
        if (context.opacity >= 1.0f || drawables.size() == 1)
        {
            Drawable::RenderingContext contextCopy (context);
            contextCopy.transform = getTransform().followedBy (context.transform);

            for (int i = 0; i < drawables.size(); ++i)
                drawables.getUnchecked(i)->render (contextCopy);
        }
        else
        {
            // To correctly render a whole composite layer with an overall transparency,
            // we need to render everything opaquely into a temp buffer, then blend that
            // with the target opacity...
            const Rectangle<int> clipBounds (context.g.getClipBounds());
            Image tempImage (Image::ARGB, clipBounds.getWidth(), clipBounds.getHeight(), true);

            {
                Graphics tempG (tempImage);
                tempG.setOrigin (-clipBounds.getX(), -clipBounds.getY());
                Drawable::RenderingContext tempContext (tempG, context.transform, 1.0f);
                render (tempContext);
            }

            context.g.setOpacity (context.opacity);
            context.g.drawImageAt (&tempImage, clipBounds.getX(), clipBounds.getY());
        }
    }
}

const Rectangle<float> DrawableComposite::getUntransformedBounds() const
{
    Rectangle<float> bounds;

    for (int i = 0; i < drawables.size(); ++i)
        bounds = bounds.getUnion (drawables.getUnchecked(i)->getBounds());

    return bounds;
}

const Rectangle<float> DrawableComposite::getBounds() const
{
    return getUntransformedBounds().transformed (getTransform());
}

bool DrawableComposite::hitTest (float x, float y) const
{
    getTransform().inverted().transformPoint (x, y);

    for (int i = 0; i < drawables.size(); ++i)
        if (drawables.getUnchecked(i)->hitTest (x, y))
            return true;

    return false;
}

Drawable* DrawableComposite::createCopy() const
{
    DrawableComposite* const dc = new DrawableComposite();

    for (int i = 0; i < 3; ++i)
        dc->controlPoints[i] = controlPoints[i];

    for (int i = 0; i < drawables.size(); ++i)
        dc->drawables.add (drawables.getUnchecked(i)->createCopy());

    return dc;
}

//==============================================================================
const Identifier DrawableComposite::valueTreeType ("Group");

namespace DrawableCompositeHelpers
{
    static const Identifier topLeft ("topLeft");
    static const Identifier topRight ("topRight");
    static const Identifier bottomLeft ("bottomLeft");

    static void stringToPoint (const String& coords, Point<float>& point)
    {
        if (coords.isNotEmpty())
        {
            const int comma = coords.indexOfChar (',');
            point.setXY (coords.substring (0, comma).getFloatValue(),
                         coords.substring (comma + 1).getFloatValue());
        }
    }

    static const var pointToString (const Point<float>& point)
    {
        return String (point.getX()) + ", " + String (point.getY());
    }
}

const Rectangle<float> DrawableComposite::refreshFromValueTree (const ValueTree& tree, ImageProvider* imageProvider)
{
    jassert (tree.hasType (valueTreeType));

    Rectangle<float> damageRect;

    setName (tree [idProperty]);

    Point<float> newControlPoint[3];
    DrawableCompositeHelpers::stringToPoint (tree [DrawableCompositeHelpers::topLeft].toString(), newControlPoint[0]);
    DrawableCompositeHelpers::stringToPoint (tree [DrawableCompositeHelpers::topRight].toString(), newControlPoint[1]);
    DrawableCompositeHelpers::stringToPoint (tree [DrawableCompositeHelpers::bottomLeft].toString(), newControlPoint[2]);
    bool controlPointsChanged = false;

    if (controlPoints[0] != newControlPoint[0]
         || controlPoints[1] != newControlPoint[1]
         || controlPoints[2] != newControlPoint[2])
    {
        controlPointsChanged = true;
        damageRect = getUntransformedBounds();
        controlPoints[0] = newControlPoint[0];
        controlPoints[1] = newControlPoint[1];
        controlPoints[2] = newControlPoint[2];
    }

    int i;
    for (i = drawables.size(); --i >= tree.getNumChildren();)
    {
        damageRect = damageRect.getUnion (drawables.getUnchecked(i)->getBounds());
        drawables.remove (i);
    }

    for (int i = 0; i < tree.getNumChildren(); ++i)
    {
        const ValueTree childTree (tree.getChild (i));
        Drawable* d = drawables[i];

        if (d != 0)
        {
            if (childTree.hasType (d->getValueTreeType()))
            {
                damageRect = damageRect.getUnion (d->refreshFromValueTree (childTree, imageProvider));
            }
            else
            {
                damageRect = damageRect.getUnion (d->getBounds());
                d = createFromValueTree (childTree, imageProvider);
                drawables.set (i, d);
                damageRect = damageRect.getUnion (d->getBounds());
            }
        }
        else
        {
            d = createFromValueTree (childTree, imageProvider);
            drawables.set (i, d);
            damageRect = damageRect.getUnion (d->getBounds());
        }
    }

    if (controlPointsChanged)
        damageRect = damageRect.getUnion (getUntransformedBounds());

    return damageRect.transformed (getTransform());
}

const ValueTree DrawableComposite::createValueTree (ImageProvider* imageProvider) const
{
    ValueTree v (valueTreeType);

    if (getName().isNotEmpty())
        v.setProperty (idProperty, getName(), 0);

    if (! getTransform().isIdentity())
    {
        v.setProperty (DrawableCompositeHelpers::topLeft, DrawableCompositeHelpers::pointToString (controlPoints[0]), 0);
        v.setProperty (DrawableCompositeHelpers::topRight, DrawableCompositeHelpers::pointToString (controlPoints[1]), 0);
        v.setProperty (DrawableCompositeHelpers::bottomLeft, DrawableCompositeHelpers::pointToString (controlPoints[2]), 0);
    }

    for (int i = 0; i < drawables.size(); ++i)
        v.addChild (drawables.getUnchecked(i)->createValueTree (imageProvider), -1, 0);

    return v;
}


END_JUCE_NAMESPACE
