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
const char juce_drawableCompositeTransformFlag = 't';
const char juce_drawableCompositeBinaryType = 'C';
const char juce_drawablePathBinaryType = 'P';
const char juce_drawableImageBinaryType = 'I';
const char juce_drawableTextBinaryType = 'T';

bool DrawableComposite::readBinary (InputStream& input)
{
    AffineTransform transform;

    while (! input.isExhausted())
    {
        const int n = input.readByte();

        if (n == 0)
            break;

        if (n == juce_drawableCompositeTransformFlag)
        {
            float f[6];
            for (int i = 0; i < 6; ++i)
                f[i] = input.readFloat();

            transform = AffineTransform (f[0], f[1], f[2], f[3], f[4], f[5]);
        }
        else
        {
            Drawable* d = 0;

            switch (n)
            {
                case juce_drawableCompositeBinaryType:      d = new DrawableComposite(); break;
                case juce_drawablePathBinaryType:           d = new DrawablePath(); break;
                case juce_drawableImageBinaryType:          d = new DrawableImage(); break;
                case juce_drawableTextBinaryType:           d = new DrawableText(); break;
                default:                                    jassertfalse; return false;
            }

            d->setName (input.readString());

            if (! d->readBinary (input))
            {
                delete d;
                return false;
            }

            insertDrawable (d, transform);
            transform = AffineTransform::identity;
        }
    }

    return true;
}

bool DrawableComposite::writeBinary (OutputStream& output) const
{
    for (int i = 0; i < drawables.size(); ++i)
    {
        AffineTransform* transform = transforms.getUnchecked(i);

        if (transform != 0)
        {
            output.writeByte (juce_drawableCompositeTransformFlag);
            output.writeFloat (transform->mat00);
            output.writeFloat (transform->mat01);
            output.writeFloat (transform->mat02);
            output.writeFloat (transform->mat10);
            output.writeFloat (transform->mat11);
            output.writeFloat (transform->mat12);
        }

        Drawable* const d = drawables.getUnchecked(i);

        char typeFlag;

        if (dynamic_cast <DrawableComposite*> (d) != 0)
            typeFlag = juce_drawableCompositeBinaryType;
        else if (dynamic_cast <DrawablePath*> (d) != 0)
            typeFlag = juce_drawablePathBinaryType;
        else if (dynamic_cast <DrawableImage*> (d) != 0)
            typeFlag = juce_drawableImageBinaryType;
        else if (dynamic_cast <DrawableText*> (d) != 0)
            typeFlag = juce_drawableTextBinaryType;
        else
        {
            jassertfalse;
            continue;
        }

        output.writeByte (typeFlag);
        output.writeString (d->getName());
        d->writeBinary (output);
    }

    output.writeByte (0);
    return true;
}

//==============================================================================
const tchar* juce_drawableCompositeXmlTag = T("Group");
const tchar* juce_drawablePathXmlTag = T("Path");
const tchar* juce_drawableImageXmlTag = T("Image");
const tchar* juce_drawableTextXmlTag = T("Text");

bool DrawableComposite::readXml (const XmlElement& xml)
{
    forEachXmlChildElement (xml, e)
    {
        Drawable* d = 0;

        if (e->hasTagName (juce_drawableCompositeXmlTag))
            d = new DrawableComposite();
        else if (e->hasTagName (juce_drawablePathXmlTag))
            d = new DrawablePath();
        else if (e->hasTagName (juce_drawableImageXmlTag))
            d = new DrawableImage();
        else if (e->hasTagName (juce_drawableTextXmlTag))
            d = new DrawableText();
        else
        {
            jassertfalse;
            return false;
        }

        d->setName (e->getStringAttribute (T("id")));
        
        if (! d->readXml (*e))
        {
            jassertfalse;
            delete d;
            return false;
        }

        AffineTransform transform;
        const String transformAtt (e->getStringAttribute (T("transform")));
    
        if (transformAtt.isNotEmpty())
        {
            StringArray tokens;
            tokens.addTokens (transformAtt.trim(), false);
            tokens.removeEmptyStrings (true);
            
            if (tokens.size() == 6)
            {
                float f[6];
                for (int i = 0; i < 6; ++i)
                    f[i] = (float) tokens[i].getDoubleValue();

                transform = AffineTransform (f[0], f[1], f[2], f[3], f[4], f[5]);
            }
        }
        
        insertDrawable (d, transform);
    }

    return true;
}

void DrawableComposite::writeXml (XmlElement& xml) const
{
    for (int i = 0; i < drawables.size(); ++i)
    {
        XmlElement* e = 0;
        Drawable* const d = drawables.getUnchecked(i);

        if (dynamic_cast <DrawableComposite*> (d) != 0)
            e = new XmlElement (juce_drawableCompositeXmlTag);
        else if (dynamic_cast <DrawablePath*> (d) != 0)
            e = new XmlElement (juce_drawablePathXmlTag);
        else if (dynamic_cast <DrawableImage*> (d) != 0)
            e = new XmlElement (juce_drawableImageXmlTag);
        else if (dynamic_cast <DrawableText*> (d) != 0)
            e = new XmlElement (juce_drawableTextXmlTag);
        else
        {
            jassertfalse;
            continue;
        }

        AffineTransform* transform = transforms.getUnchecked(i);

        if (transform != 0)
        {
            String t;
            t <<  transform->mat00 << " " << transform->mat01 << " " << transform->mat02 << " " 
              << transform->mat10 << " " << transform->mat11 << " " << transform->mat12;

            e->setAttribute (T("transform"), t);
        }

        if (d->getName().isNotEmpty())
            e->setAttribute (T("id"), d->getName());

        d->writeXml (*e);

        xml.addChildElement (e);
    }
}

END_JUCE_NAMESPACE
