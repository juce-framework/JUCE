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

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_DrawableComposite.h"


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

void DrawableComposite::draw (Graphics& g, const AffineTransform& transform) const
{
    for (int i = 0; i < drawables.size(); ++i)
    {
        const AffineTransform* const t = transforms.getUnchecked(i);

        drawables.getUnchecked(i)->draw (g, t == 0 ? transform
                                                   : t->followedBy (transform));
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
