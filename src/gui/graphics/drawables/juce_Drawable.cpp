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
                           const int destX,
                           const int destY,
                           const int destW,
                           const int destH,
                           const RectanglePlacement& placement,
                           const float opacity) const
{
    if (destW > 0 && destH > 0)
    {
        Rectangle<float> bounds (getBounds());

        draw (g, opacity,
              placement.getTransformToFit (bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
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
Drawable* Drawable::createFromValueTree (const ValueTree& tree, ImageProvider* imageProvider)
{
    const Identifier type (tree.getType());

    Drawable* d = 0;
    if (type == DrawablePath::valueTreeType)
        d = new DrawablePath();
    else if (type == DrawableComposite::valueTreeType)
        d = new DrawableComposite();
    else if (type == DrawableImage::valueTreeType)
        d = new DrawableImage();
    else if (type == DrawableText::valueTreeType)
        d = new DrawableText();

    if (d != 0)
        d->refreshFromValueTree (tree, imageProvider);

    return d;
}


//==============================================================================
const Identifier Drawable::ValueTreeWrapperBase::idProperty ("id");
const Identifier Drawable::ValueTreeWrapperBase::type ("type");
const Identifier Drawable::ValueTreeWrapperBase::x1 ("x1");
const Identifier Drawable::ValueTreeWrapperBase::x2 ("x2");
const Identifier Drawable::ValueTreeWrapperBase::y1 ("y1");
const Identifier Drawable::ValueTreeWrapperBase::y2 ("y2");
const Identifier Drawable::ValueTreeWrapperBase::colour ("colour");
const Identifier Drawable::ValueTreeWrapperBase::radial ("radial");
const Identifier Drawable::ValueTreeWrapperBase::colours ("colours");

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

void Drawable::ValueTreeWrapperBase::setID (const String& newID, UndoManager* undoManager)
{
    if (newID.isEmpty())
        state.removeProperty (idProperty, undoManager);
    else
        state.setProperty (idProperty, newID, undoManager);
}

const FillType Drawable::ValueTreeWrapperBase::readFillType (const ValueTree& v)
{
    const String newType (v[type].toString());

    if (newType == "solid")
    {
        const String colourString (v [colour].toString());
        return FillType (Colour (colourString.isEmpty() ? (uint32) 0xff000000
                                                        : (uint32) colourString.getHexValue32()));
    }
    else if (newType == "gradient")
    {
        ColourGradient g;
        g.point1.setXY (v[x1], v[y1]);
        g.point2.setXY (v[x2], v[y2]);
        g.isRadial = v[radial];

        StringArray colourSteps;
        colourSteps.addTokens (v[colours].toString(), false);

        for (int i = 0; i < colourSteps.size() / 2; ++i)
            g.addColour (colourSteps[i * 2].getDoubleValue(),
                         Colour ((uint32)  colourSteps[i * 2 + 1].getHexValue32()));

        return FillType (g);
    }
    else if (newType == "image")
    {
        jassertfalse; //xxx todo
    }

    jassertfalse;
    return FillType();
}

void Drawable::ValueTreeWrapperBase::replaceFillType (const Identifier& tag, const FillType& fillType, UndoManager* undoManager)
{
    ValueTree v (state.getChildWithName (tag));

    if (! v.isValid())
    {
        state.addChild (ValueTree (tag), -1, undoManager);
        v = state.getChildWithName (tag);
    }

    if (fillType.isColour())
    {
        v.setProperty (type, "solid", undoManager);
        v.setProperty (colour, String::toHexString ((int) fillType.colour.getARGB()), undoManager);
        v.removeProperty (x1, undoManager);
        v.removeProperty (x2, undoManager);
        v.removeProperty (y1, undoManager);
        v.removeProperty (y2, undoManager);
        v.removeProperty (radial, undoManager);
        v.removeProperty (colours, undoManager);
    }
    else if (fillType.isGradient())
    {
        v.setProperty (type, "gradient", undoManager);
        v.setProperty (x1, fillType.gradient->point1.getX(), undoManager);
        v.setProperty (y1, fillType.gradient->point1.getY(), undoManager);
        v.setProperty (x2, fillType.gradient->point2.getX(), undoManager);
        v.setProperty (y2, fillType.gradient->point2.getY(), undoManager);
        v.setProperty (radial, fillType.gradient->isRadial, undoManager);

        String s;
        for (int i = 0; i < fillType.gradient->getNumColours(); ++i)
            s << " " << fillType.gradient->getColourPosition (i)
              << " " << String::toHexString ((int) fillType.gradient->getColour(i).getARGB());

        v.setProperty (colours, s.trimStart(), undoManager);
        v.removeProperty (colour, undoManager);
    }
    else if (fillType.isTiledImage())
    {
        v.setProperty (type, "image", undoManager);

        jassertfalse; //xxx todo

        v.removeProperty (x1, undoManager);
        v.removeProperty (x2, undoManager);
        v.removeProperty (y1, undoManager);
        v.removeProperty (y2, undoManager);
        v.removeProperty (radial, undoManager);
        v.removeProperty (colours, undoManager);
        v.removeProperty (colour, undoManager);
    }
    else
    {
        jassertfalse;
    }
}


END_JUCE_NAMESPACE
