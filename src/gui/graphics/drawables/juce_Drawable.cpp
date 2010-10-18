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
const Identifier Drawable::ValueTreeWrapperBase::type ("type");
const Identifier Drawable::ValueTreeWrapperBase::gradientPoint1 ("point1");
const Identifier Drawable::ValueTreeWrapperBase::gradientPoint2 ("point2");
const Identifier Drawable::ValueTreeWrapperBase::gradientPoint3 ("point3");
const Identifier Drawable::ValueTreeWrapperBase::colour ("colour");
const Identifier Drawable::ValueTreeWrapperBase::radial ("radial");
const Identifier Drawable::ValueTreeWrapperBase::colours ("colours");
const Identifier Drawable::ValueTreeWrapperBase::imageId ("imageId");
const Identifier Drawable::ValueTreeWrapperBase::imageOpacity ("imageOpacity");

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

const FillType Drawable::ValueTreeWrapperBase::readFillType (const ValueTree& v, RelativePoint* const gp1, RelativePoint* const gp2, RelativePoint* const gp3,
                                                             Expression::EvaluationContext* const nameFinder, ImageProvider* imageProvider)
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
        RelativePoint p1 (v [gradientPoint1]), p2 (v [gradientPoint2]), p3 (v [gradientPoint3]);

        ColourGradient g;

        if (gp1 != 0)  *gp1 = p1;
        if (gp2 != 0)  *gp2 = p2;
        if (gp3 != 0)  *gp3 = p3;

        g.point1 = p1.resolve (nameFinder);
        g.point2 = p2.resolve (nameFinder);
        g.isRadial = v[radial];

        StringArray colourSteps;
        colourSteps.addTokens (v[colours].toString(), false);

        for (int i = 0; i < colourSteps.size() / 2; ++i)
            g.addColour (colourSteps[i * 2].getDoubleValue(),
                         Colour ((uint32)  colourSteps[i * 2 + 1].getHexValue32()));

        FillType fillType (g);

        if (g.isRadial)
        {
            const Point<float> point3 (p3.resolve (nameFinder));
            const Point<float> point3Source (g.point1.getX() + g.point2.getY() - g.point1.getY(),
                                             g.point1.getY() + g.point1.getX() - g.point2.getX());

            fillType.transform = AffineTransform::fromTargetPoints (g.point1.getX(), g.point1.getY(), g.point1.getX(), g.point1.getY(),
                                                                    g.point2.getX(), g.point2.getY(), g.point2.getX(), g.point2.getY(),
                                                                    point3Source.getX(), point3Source.getY(), point3.getX(), point3.getY());
        }

        return fillType;
    }
    else if (newType == "image")
    {
        Image im;
        if (imageProvider != 0)
            im = imageProvider->getImageForIdentifier (v[imageId]);

        FillType f (im, AffineTransform::identity);
        f.setOpacity ((float) v.getProperty (imageOpacity, 1.0f));
        return f;
    }

    jassertfalse;
    return FillType();
}

static const Point<float> calcThirdGradientPoint (const FillType& fillType)
{
    const ColourGradient& g = *fillType.gradient;
    const Point<float> point3Source (g.point1.getX() + g.point2.getY() - g.point1.getY(),
                                     g.point1.getY() + g.point1.getX() - g.point2.getX());

    return point3Source.transformedBy (fillType.transform);
}

void Drawable::ValueTreeWrapperBase::writeFillType (ValueTree& v, const FillType& fillType,
                                                    const RelativePoint* const gp1, const RelativePoint* const gp2, const RelativePoint* gp3,
                                                    ImageProvider* imageProvider, UndoManager* const undoManager)
{
    if (fillType.isColour())
    {
        v.setProperty (type, "solid", undoManager);
        v.setProperty (colour, String::toHexString ((int) fillType.colour.getARGB()), undoManager);
    }
    else if (fillType.isGradient())
    {
        v.setProperty (type, "gradient", undoManager);
        v.setProperty (gradientPoint1, gp1 != 0 ? gp1->toString() : fillType.gradient->point1.toString(), undoManager);
        v.setProperty (gradientPoint2, gp2 != 0 ? gp2->toString() : fillType.gradient->point2.toString(), undoManager);
        v.setProperty (gradientPoint3, gp3 != 0 ? gp3->toString() : calcThirdGradientPoint (fillType).toString(), undoManager);

        v.setProperty (radial, fillType.gradient->isRadial, undoManager);

        String s;
        for (int i = 0; i < fillType.gradient->getNumColours(); ++i)
            s << ' ' << fillType.gradient->getColourPosition (i)
              << ' ' << String::toHexString ((int) fillType.gradient->getColour(i).getARGB());

        v.setProperty (colours, s.trimStart(), undoManager);
    }
    else if (fillType.isTiledImage())
    {
        v.setProperty (type, "image", undoManager);

        if (imageProvider != 0)
            v.setProperty (imageId, imageProvider->getIdentifierForImage (fillType.image), undoManager);

        if (fillType.getOpacity() < 1.0f)
            v.setProperty (imageOpacity, fillType.getOpacity(), undoManager);
        else
            v.removeProperty (imageOpacity, undoManager);
    }
    else
    {
        jassertfalse;
    }
}


END_JUCE_NAMESPACE
