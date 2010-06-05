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

#include "juce_DrawableImage.h"
#include "juce_DrawableComposite.h"
#include "../imaging/juce_ImageCache.h"


//==============================================================================
DrawableImage::DrawableImage()
    : image (0),
      opacity (1.0f),
      overlayColour (0x00000000)
{
    controlPoints[1] = RelativePoint (Point<float> (1.0f, 0.0f));
    controlPoints[2] = RelativePoint (Point<float> (0.0f, 1.0f));
}

DrawableImage::DrawableImage (const DrawableImage& other)
    : image (other.image),
      opacity (other.opacity),
      overlayColour (other.overlayColour)
{
    for (int i = 0; i < numElementsInArray (controlPoints); ++i)
        controlPoints[i] = other.controlPoints[i];
}

DrawableImage::~DrawableImage()
{
}

//==============================================================================
void DrawableImage::setImage (const Image& imageToUse)
{
    image = imageToUse;

    if (image.isValid())
    {
        controlPoints[0] = RelativePoint (Point<float> (0.0f, 0.0f));
        controlPoints[1] = RelativePoint (Point<float> ((float) image.getWidth(), 0.0f));
        controlPoints[2] = RelativePoint (Point<float> (0.0f, (float) image.getHeight()));
    }
}

void DrawableImage::setOpacity (const float newOpacity)
{
    opacity = newOpacity;
}

void DrawableImage::setOverlayColour (const Colour& newOverlayColour)
{
    overlayColour = newOverlayColour;
}

void DrawableImage::setTransform (const RelativePoint& imageTopLeftPosition,
                                  const RelativePoint& imageTopRightPosition,
                                  const RelativePoint& imageBottomLeftPosition)
{
    controlPoints[0] = imageTopLeftPosition;
    controlPoints[1] = imageTopRightPosition;
    controlPoints[2] = imageBottomLeftPosition;
}

//==============================================================================
const AffineTransform DrawableImage::calculateTransform() const
{
    if (image.isNull())
        return AffineTransform::identity;

    Point<float> resolved[3];
    for (int i = 0; i < 3; ++i)
        resolved[i] = controlPoints[i].resolve (parent);

    const Point<float> tr (resolved[0] + (resolved[1] - resolved[0]) / (float) image.getWidth());
    const Point<float> bl (resolved[0] + (resolved[2] - resolved[0]) / (float) image.getHeight());

    return AffineTransform::fromTargetPoints (resolved[0].getX(), resolved[0].getY(),
                                              tr.getX(), tr.getY(),
                                              bl.getX(), bl.getY());
}

void DrawableImage::render (const Drawable::RenderingContext& context) const
{
    if (image.isValid())
    {
        const AffineTransform t (calculateTransform().followedBy (context.transform));

        if (opacity > 0.0f && ! overlayColour.isOpaque())
        {
            context.g.setOpacity (context.opacity * opacity);
            context.g.drawImageTransformed (image, image.getBounds(), t, false);
        }

        if (! overlayColour.isTransparent())
        {
            context.g.setColour (overlayColour.withMultipliedAlpha (context.opacity));
            context.g.drawImageTransformed (image, image.getBounds(), t, true);
        }
    }
}

const Rectangle<float> DrawableImage::getBounds() const
{
    if (image.isNull())
        return Rectangle<float>();

    Point<float> corners[4];
    for (int i = 0; i < 3; ++i)
        corners[i] = controlPoints[i].resolve (parent);

    corners[3] = corners[1] + (corners[2] - corners[0]);
    return Rectangle<float>::findAreaContainingPoints (corners, 4);
}

bool DrawableImage::hitTest (float x, float y) const
{
    if (image.isNull())
        return false;

    calculateTransform().inverted().transformPoint (x, y);

    const int ix = roundToInt (x);
    const int iy = roundToInt (y);

    return ix >= 0
            && iy >= 0
            && ix < image.getWidth()
            && iy < image.getHeight()
            && image.getPixelAt (ix, iy).getAlpha() >= 127;
}

Drawable* DrawableImage::createCopy() const
{
    return new DrawableImage (*this);
}

void DrawableImage::invalidatePoints()
{
}

//==============================================================================
const Identifier DrawableImage::valueTreeType ("Image");

const Identifier DrawableImage::ValueTreeWrapper::opacity ("opacity");
const Identifier DrawableImage::ValueTreeWrapper::overlay ("overlay");
const Identifier DrawableImage::ValueTreeWrapper::image ("image");
const Identifier DrawableImage::ValueTreeWrapper::topLeft ("topLeft");
const Identifier DrawableImage::ValueTreeWrapper::topRight ("topRight");
const Identifier DrawableImage::ValueTreeWrapper::bottomLeft ("bottomLeft");

//==============================================================================
DrawableImage::ValueTreeWrapper::ValueTreeWrapper (const ValueTree& state_)
    : ValueTreeWrapperBase (state_)
{
    jassert (state.hasType (valueTreeType));
}

const var DrawableImage::ValueTreeWrapper::getImageIdentifier() const
{
    return state [image];
}

Value DrawableImage::ValueTreeWrapper::getImageIdentifierValue (UndoManager* undoManager)
{
    return state.getPropertyAsValue (image, undoManager);
}

void DrawableImage::ValueTreeWrapper::setImageIdentifier (const var& newIdentifier, UndoManager* undoManager)
{
    state.setProperty (image, newIdentifier, undoManager);
}

float DrawableImage::ValueTreeWrapper::getOpacity() const
{
    return (float) state.getProperty (opacity, 1.0);
}

Value DrawableImage::ValueTreeWrapper::getOpacityValue (UndoManager* undoManager)
{
    if (! state.hasProperty (opacity))
        state.setProperty (opacity, 1.0, undoManager);

    return state.getPropertyAsValue (opacity, undoManager);
}

void DrawableImage::ValueTreeWrapper::setOpacity (float newOpacity, UndoManager* undoManager)
{
    state.setProperty (opacity, newOpacity, undoManager);
}

const Colour DrawableImage::ValueTreeWrapper::getOverlayColour() const
{
    return Colour (state [overlay].toString().getHexValue32());
}

void DrawableImage::ValueTreeWrapper::setOverlayColour (const Colour& newColour, UndoManager* undoManager)
{
    if (newColour.isTransparent())
        state.removeProperty (overlay, undoManager);
    else
        state.setProperty (overlay, String::toHexString ((int) newColour.getARGB()), undoManager);
}

Value DrawableImage::ValueTreeWrapper::getOverlayColourValue (UndoManager* undoManager)
{
    return state.getPropertyAsValue (overlay, undoManager);
}

const RelativePoint DrawableImage::ValueTreeWrapper::getTargetPositionForTopLeft() const
{
    const String pos (state [topLeft].toString());
    return pos.isNotEmpty() ? RelativePoint (pos) : RelativePoint();
}

void DrawableImage::ValueTreeWrapper::setTargetPositionForTopLeft (const RelativePoint& newPoint, UndoManager* undoManager)
{
    state.setProperty (topLeft, newPoint.toString(), undoManager);
}

const RelativePoint DrawableImage::ValueTreeWrapper::getTargetPositionForTopRight() const
{
    const String pos (state [topRight].toString());
    return pos.isNotEmpty() ? RelativePoint (pos) : RelativePoint (Point<float> (100.0f, 0.0f));
}

void DrawableImage::ValueTreeWrapper::setTargetPositionForTopRight (const RelativePoint& newPoint, UndoManager* undoManager)
{
    state.setProperty (topRight, newPoint.toString(), undoManager);
}

const RelativePoint DrawableImage::ValueTreeWrapper::getTargetPositionForBottomLeft() const
{
    const String pos (state [bottomLeft].toString());
    return pos.isNotEmpty() ? RelativePoint (pos) : RelativePoint (Point<float> (0.0f, 100.0f));
}

void DrawableImage::ValueTreeWrapper::setTargetPositionForBottomLeft (const RelativePoint& newPoint, UndoManager* undoManager)
{
    state.setProperty (bottomLeft, newPoint.toString(), undoManager);
}


//==============================================================================
const Rectangle<float> DrawableImage::refreshFromValueTree (const ValueTree& tree, ImageProvider* imageProvider)
{
    const ValueTreeWrapper controller (tree);
    setName (controller.getID());

    const float newOpacity = controller.getOpacity();
    const Colour newOverlayColour (controller.getOverlayColour());

    Image newImage;
    const var imageIdentifier (controller.getImageIdentifier());

    jassert (imageProvider != 0 || imageIdentifier.isVoid()); // if you're using images, you need to provide something that can load and save them!

    if (imageProvider != 0)
        newImage = imageProvider->getImageForIdentifier (imageIdentifier);

    RelativePoint newControlPoint[3] = { controller.getTargetPositionForTopLeft(),
                                         controller.getTargetPositionForTopRight(),
                                         controller.getTargetPositionForBottomLeft() };

    if (newOpacity != opacity || overlayColour != newOverlayColour || image != newImage
         || controlPoints[0] != newControlPoint[0]
         || controlPoints[1] != newControlPoint[1]
         || controlPoints[2] != newControlPoint[2])
    {
        const Rectangle<float> damage (getBounds());

        opacity = newOpacity;
        overlayColour = newOverlayColour;
        controlPoints[0] = newControlPoint[0];
        controlPoints[1] = newControlPoint[1];
        controlPoints[2] = newControlPoint[2];
        image = newImage;

        return damage.getUnion (getBounds());
    }

    return Rectangle<float>();
}

const ValueTree DrawableImage::createValueTree (ImageProvider* imageProvider) const
{
    ValueTree tree (valueTreeType);
    ValueTreeWrapper v (tree);

    v.setID (getName(), 0);
    v.setOpacity (opacity, 0);
    v.setOverlayColour (overlayColour, 0);
    v.setTargetPositionForTopLeft (controlPoints[0], 0);
    v.setTargetPositionForTopRight (controlPoints[1], 0);
    v.setTargetPositionForBottomLeft (controlPoints[2], 0);

    if (image.isValid())
    {
        jassert (imageProvider != 0); // if you're using images, you need to provide something that can load and save them!

        if (imageProvider != 0)
            v.setImageIdentifier (imageProvider->getIdentifierForImage (image), 0);
    }

    return tree;
}


END_JUCE_NAMESPACE
