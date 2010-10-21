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

#include "juce_Drawable.h"
#include "juce_DrawableComposite.h"
#include "juce_DrawablePath.h"
#include "juce_DrawableRectangle.h"
#include "juce_DrawableImage.h"
#include "juce_DrawableText.h"
#include "../imaging/juce_ImageFileFormat.h"
#include "../../../text/juce_XmlDocument.h"
#include "../../../io/files/juce_FileInputStream.h"
#include "../../../io/streams/juce_MemoryOutputStream.h"


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
    : parent (0)
{
}

Drawable::~Drawable()
{
}

void Drawable::draw (Graphics& g, const float opacity, const AffineTransform& transform) const
{
    render (RenderingContext (g, transform, opacity));
}

void Drawable::drawAt (Graphics& g, const float x, const float y, const float opacity) const
{
    draw (g, opacity, AffineTransform::translation (x, y));
}

void Drawable::drawWithin (Graphics& g,
                           const Rectangle<float>& destArea,
                           const RectanglePlacement& placement,
                           const float opacity) const
{
    if (! destArea.isEmpty())
        draw (g, opacity, placement.getTransformToFit (getBounds(), destArea));
}

//==============================================================================
Drawable* Drawable::createFromImageData (const void* data, const size_t numBytes)
{
    Drawable* result = 0;

    Image image (ImageFileFormat::loadFrom (data, (int) numBytes));

    if (image.isValid())
    {
        DrawableImage* const di = new DrawableImage();
        di->setImage (image);
        result = di;
    }
    else
    {
        const String asString (String::createStringFromData (data, (int) numBytes));

        XmlDocument doc (asString);
        ScopedPointer <XmlElement> outer (doc.getDocumentElement (true));

        if (outer != 0 && outer->hasTagName ("svg"))
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
    MemoryOutputStream mo;
    mo.writeFromInputStream (dataSource, -1);

    return createFromImageData (mo.getData(), mo.getDataSize());
}

Drawable* Drawable::createFromImageFile (const File& file)
{
    const ScopedPointer <FileInputStream> fin (file.createInputStream());

    return fin != 0 ? createFromImageDataStream (*fin) : 0;
}

//==============================================================================
Drawable* Drawable::createFromValueTree (const ValueTree& tree, ImageProvider* imageProvider)
{
    return createChildFromValueTree (0, tree, imageProvider);
}

Drawable* Drawable::createChildFromValueTree (DrawableComposite* parent, const ValueTree& tree, ImageProvider* imageProvider)
{
    const Identifier type (tree.getType());

    Drawable* d = 0;
    if (type == DrawablePath::valueTreeType)
        d = new DrawablePath();
    else if (type == DrawableComposite::valueTreeType)
        d = new DrawableComposite();
    else if (type == DrawableRectangle::valueTreeType)
        d = new DrawableRectangle();
    else if (type == DrawableImage::valueTreeType)
        d = new DrawableImage();
    else if (type == DrawableText::valueTreeType)
        d = new DrawableText();

    if (d != 0)
    {
        d->parent = parent;
        d->refreshFromValueTree (tree, imageProvider);
    }

    return d;
}


//==============================================================================
const Identifier Drawable::ValueTreeWrapperBase::idProperty ("id");

Drawable::ValueTreeWrapperBase::ValueTreeWrapperBase (const ValueTree& state_)
    : state (state_)
{
}

Drawable::ValueTreeWrapperBase::~ValueTreeWrapperBase()
{
}

const String Drawable::ValueTreeWrapperBase::getID() const
{
    return state [idProperty];
}

void Drawable::ValueTreeWrapperBase::setID (const String& newID, UndoManager* const undoManager)
{
    if (newID.isEmpty())
        state.removeProperty (idProperty, undoManager);
    else
        state.setProperty (idProperty, newID, undoManager);
}


END_JUCE_NAMESPACE
