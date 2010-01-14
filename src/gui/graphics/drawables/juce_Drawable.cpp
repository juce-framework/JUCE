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

#include "juce_Drawable.h"
#include "juce_DrawableComposite.h"
#include "juce_DrawablePath.h"
#include "juce_DrawableImage.h"
#include "juce_DrawableText.h"
#include "../imaging/juce_ImageFileFormat.h"
#include "../../../text/juce_XmlDocument.h"
#include "../../../io/files/juce_FileInputStream.h"

//==============================================================================
Drawable::RenderingContext::RenderingContext (Graphics& g_,
                                              const AffineTransform& transform_,
                                              const float opacity_) throw()
    : g (g_),
      transform (transform_),
      opacity (opacity_)
{
}

//==============================================================================
Drawable::Drawable()
{
}

Drawable::~Drawable()
{
}

void Drawable::draw (Graphics& g, const float opacity,
                     const AffineTransform& transform) const
{
    render (RenderingContext (g, transform, opacity));
}

void Drawable::drawAt (Graphics& g, const float x, const float y, const float opacity) const
{
    draw (g, opacity, AffineTransform::translation (x, y));
}

void Drawable::drawWithin (Graphics& g,
                           const int destX,
                           const int destY,
                           const int destW,
                           const int destH,
                           const RectanglePlacement& placement,
                           const float opacity) const
{
    if (destW > 0 && destH > 0)
    {
        float x, y, w, h;
        getBounds (x, y, w, h);

        draw (g, opacity,
              placement.getTransformToFit (x, y, w, h,
                                           (float) destX, (float) destY,
                                           (float) destW, (float) destH));
    }
}

//==============================================================================
Drawable* Drawable::createFromImageData (const void* data, const size_t numBytes)
{
    Drawable* result = 0;

    Image* const image = ImageFileFormat::loadFrom (data, (int) numBytes);

    if (image != 0)
    {
        DrawableImage* const di = new DrawableImage();
        di->setImage (image, true);
        result = di;
    }
    else
    {
        const String asString (String::createStringFromData (data, (int) numBytes));

        XmlDocument doc (asString);
        ScopedPointer <XmlElement> outer (doc.getDocumentElement (true));

        if (outer != 0 && outer->hasTagName (T("svg")))
        {
            ScopedPointer <XmlElement> svg (doc.getDocumentElement());

            if (svg != 0)
                result = Drawable::createFromSVG (*svg);
        }
    }

    return result;
}

Drawable* Drawable::createFromImageDataStream (InputStream& dataSource)
{
    MemoryBlock mb;
    dataSource.readIntoMemoryBlock (mb);

    return createFromImageData (mb.getData(), mb.getSize());
}

Drawable* Drawable::createFromImageFile (const File& file)
{
    const ScopedPointer <FileInputStream> fin (file.createInputStream());

    return fin != 0 ? createFromImageDataStream (*fin) : 0;
}

//==============================================================================
Drawable* Drawable::createFromValueTree (const ValueTree& tree) throw()
{
    Drawable* d = DrawablePath::createFromValueTree (tree);

    if (d == 0)
    {
        d = DrawableComposite::createFromValueTree (tree);

        if (d == 0)
        {
            d = DrawableImage::createFromValueTree (tree);

            if (d == 0)
                d = DrawableText::createFromValueTree (tree);
        }
    }

    return d;
}


END_JUCE_NAMESPACE
