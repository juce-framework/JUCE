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
#include "juce_DrawablePath.h"
#include "juce_DrawableImage.h"
#include "juce_DrawableText.h"
#include "../imaging/juce_Image.h"


//==============================================================================
DrawableComposite::DrawableComposite()
{
}

DrawableComposite::~DrawableComposite()
{
}

//==============================================================================
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

void DrawableComposite::removeDrawable (const int index, const bool deleteDrawable)
{
    drawables.remove (index, deleteDrawable);
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

//==============================================================================
void DrawableComposite::render (const Drawable::RenderingContext& context) const
{
    if (drawables.size() > 0 && context.opacity > 0)
    {
        if (context.opacity >= 1.0f || drawables.size() == 1)
        {
            Drawable::RenderingContext contextCopy (context);

            for (int i = 0; i < drawables.size(); ++i)
            {
                const AffineTransform* const t = transforms.getUnchecked(i);
                contextCopy.transform = (t == 0) ? context.transform
                                                 : t->followedBy (context.transform);

                drawables.getUnchecked(i)->render (contextCopy);
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
                render (tempContext);
            }

            context.g.setOpacity (context.opacity);
            context.g.drawImageAt (&tempImage, clipBounds.getX(), clipBounds.getY());
        }
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

//==============================================================================
ValueTree DrawableComposite::createValueTree() const throw()
{
    ValueTree v (T("Group"));

    if (getName().isNotEmpty())
        v.setProperty ("id", getName(), 0);

    for (int i = 0; i < drawables.size(); ++i)
    {
        Drawable* const d = drawables.getUnchecked(i);

        ValueTree child (d->createValueTree());

        AffineTransform* transform = transforms.getUnchecked(i);

        if (transform != 0)
        {
            String t;
            t <<  transform->mat00 << " " << transform->mat01 << " " << transform->mat02 << " "
              << transform->mat10 << " " << transform->mat11 << " " << transform->mat12;

            child.setProperty ("transform", t, 0);
        }

        v.addChild (child, -1, 0);
    }

    return v;
}

DrawableComposite* DrawableComposite::createFromValueTree (const ValueTree& tree) throw()
{
    if (! tree.hasType ("Group"))
        return 0;

    DrawableComposite* dc = new DrawableComposite();
    dc->setName (tree ["id"]);

    for (int i = 0; i < tree.getNumChildren(); ++i)
    {
        ValueTree childTree (tree.getChild (i));
        Drawable* d = Drawable::createFromValueTree (childTree);

        if (d != 0)
        {
            AffineTransform transform;
            const String transformAtt (childTree ["transform"].toString());

            if (transformAtt.isNotEmpty())
            {
                StringArray tokens;
                tokens.addTokens (transformAtt.trim(), false);
                tokens.removeEmptyStrings (true);

                if (tokens.size() == 6)
                {
                    float f[6];
                    for (int j = 0; j < 6; ++j)
                        f[j] = (float) tokens[j].getDoubleValue();

                    transform = AffineTransform (f[0], f[1], f[2], f[3], f[4], f[5]);
                }
            }

            dc->insertDrawable (d, transform);
        }
    }

    return dc;
}


END_JUCE_NAMESPACE
