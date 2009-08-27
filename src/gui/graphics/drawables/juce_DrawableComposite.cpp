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

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_DrawableComposite.h"
#include "../imaging/juce_Image.h"


//==============================================================================
DrawableComposite::DrawableComposite()
{
}

DrawableComposite::~DrawableComposite()
{
}

void DrawableComposite::insertDrawable (Drawable* drawable,
                                        const AffineTransform& transform,
                                        const int index)
{
    if (drawable != 0)
    {
        if (! drawables.contains (drawable))
        {
            drawables.insert (index, drawable);

            if (transform.isIdentity())
                transforms.insert (index, 0);
            else
                transforms.insert (index, new AffineTransform (transform));
        }
        else
        {
            jassertfalse // trying to add a drawable that's already in here!
        }
    }
}

void DrawableComposite::insertDrawable (const Drawable& drawable,
                                        const AffineTransform& transform,
                                        const int index)
{
    insertDrawable (drawable.createCopy(), transform, index);
}

void DrawableComposite::removeDrawable (const int index)
{
    drawables.remove (index);
    transforms.remove (index);
}

void DrawableComposite::bringToFront (const int index)
{
    if (index >= 0 && index < drawables.size() - 1)
    {
        drawables.move (index, -1);
        transforms.move (index, -1);
    }
}

void DrawableComposite::draw (const Drawable::RenderingContext& context) const
{
    if (drawables.size() > 1)
    {
        Drawable::RenderingContext contextCopy (context);

        if (context.opacity >= 1.0f)
        {
            for (int i = 0; i < drawables.size(); ++i)
            {
                const AffineTransform* const t = transforms.getUnchecked(i);
                contextCopy.transform = (t == 0) ? context.transform
                                                 : t->followedBy (context.transform);

                drawables.getUnchecked(i)->draw (context);
            }
        }
        else
        {
            // To correctly render a whole composite layer with an overall transparency,
            // we need to render everything opaquely into a temp buffer, then blend that
            // with the target opacity...
            const Rectangle clipBounds (context.g.getClipBounds());
            Image tempImage (Image::ARGB, clipBounds.getWidth(), clipBounds.getHeight(), true);

            {
                Graphics tempG (tempImage);
                tempG.setOrigin (-clipBounds.getX(), -clipBounds.getY());
                Drawable::RenderingContext tempContext (tempG, context.transform, 1.0f);
                draw (tempContext);
            }

            context.g.setOpacity (context.opacity);
            context.g.drawImageAt (&tempImage, clipBounds.getX(), clipBounds.getY());
        }
    }
    else if (drawables.size() > 0)
    {
        drawables.getUnchecked(0)->draw (context);
    }
}

void DrawableComposite::getBounds (float& x, float& y, float& width, float& height) const
{
    Path totalPath;

    for (int i = 0; i < drawables.size(); ++i)
    {
        drawables.getUnchecked(i)->getBounds (x, y, width, height);

        if (width > 0.0f && height > 0.0f)
        {
            Path outline;
            outline.addRectangle (x, y, width, height);

            const AffineTransform* const t = transforms.getUnchecked(i);

            if (t == 0)
                totalPath.addPath (outline);
            else
                totalPath.addPath (outline, *t);
        }
    }

    totalPath.getBounds (x, y, width, height);
}

bool DrawableComposite::hitTest (float x, float y) const
{
    for (int i = 0; i < drawables.size(); ++i)
    {
        float tx = x;
        float ty = y;

        const AffineTransform* const t = transforms.getUnchecked(i);

        if (t != 0)
            t->inverted().transformPoint (tx, ty);

        if (drawables.getUnchecked(i)->hitTest (tx, ty))
            return true;
    }

    return false;
}

Drawable* DrawableComposite::createCopy() const
{
    DrawableComposite* const dc = new DrawableComposite();

    for (int i = 0; i < drawables.size(); ++i)
    {
        dc->drawables.add (drawables.getUnchecked(i)->createCopy());

        const AffineTransform* const t = transforms.getUnchecked(i);
        dc->transforms.add (t != 0 ? new AffineTransform (*t) : 0);
    }

    return dc;
}

END_JUCE_NAMESPACE
