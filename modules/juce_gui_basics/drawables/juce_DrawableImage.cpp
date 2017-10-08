/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

DrawableImage::DrawableImage()
    : opacity (1.0f),
      overlayColour (0x00000000)
{
    bounds.topRight = RelativePoint (Point<float> (1.0f, 0.0f));
    bounds.bottomLeft = RelativePoint (Point<float> (0.0f, 1.0f));
}

DrawableImage::DrawableImage (const DrawableImage& other)
    : Drawable (other),
      image (other.image),
      opacity (other.opacity),
      overlayColour (other.overlayColour),
      bounds (other.bounds)
{
    setBounds (other.getBounds());
}

DrawableImage::~DrawableImage()
{
}

//==============================================================================
void DrawableImage::setImage (const Image& imageToUse)
{
    image = imageToUse;
    setBounds (imageToUse.getBounds());

    bounds.topLeft = RelativePoint (Point<float> (0.0f, 0.0f));
    bounds.topRight = RelativePoint (Point<float> ((float) image.getWidth(), 0.0f));
    bounds.bottomLeft = RelativePoint (Point<float> (0.0f, (float) image.getHeight()));
    recalculateCoordinates (nullptr);

    repaint();
}

void DrawableImage::setOpacity (const float newOpacity)
{
    opacity = newOpacity;
}

void DrawableImage::setOverlayColour (Colour newOverlayColour)
{
    overlayColour = newOverlayColour;
}

void DrawableImage::setBoundingBox (const RelativeParallelogram& newBounds)
{
    if (bounds != newBounds)
    {
        bounds = newBounds;

        if (bounds.isDynamic())
        {
            Drawable::Positioner<DrawableImage>* const p = new Drawable::Positioner<DrawableImage> (*this);
            setPositioner (p);
            p->apply();
        }
        else
        {
            setPositioner (nullptr);
            recalculateCoordinates (nullptr);
        }
    }
}

//==============================================================================
bool DrawableImage::registerCoordinates (RelativeCoordinatePositionerBase& pos)
{
    bool ok = pos.addPoint (bounds.topLeft);
    ok = pos.addPoint (bounds.topRight) && ok;
    return pos.addPoint (bounds.bottomLeft) && ok;
}

void DrawableImage::recalculateCoordinates (Expression::Scope* scope)
{
    if (image.isValid())
    {
        Point<float> resolved[3];
        bounds.resolveThreePoints (resolved, scope);

        const Point<float> tr (resolved[0] + (resolved[1] - resolved[0]) / (float) image.getWidth());
        const Point<float> bl (resolved[0] + (resolved[2] - resolved[0]) / (float) image.getHeight());

        AffineTransform t (AffineTransform::fromTargetPoints (resolved[0].x, resolved[0].y,
                                                              tr.x, tr.y,
                                                              bl.x, bl.y));

        if (t.isSingularity())
            t = AffineTransform();

        setTransform (t);
    }
}

//==============================================================================
void DrawableImage::paint (Graphics& g)
{
    if (image.isValid())
    {
        if (opacity > 0.0f && ! overlayColour.isOpaque())
        {
            g.setOpacity (opacity);
            g.drawImageAt (image, 0, 0, false);
        }

        if (! overlayColour.isTransparent())
        {
            g.setColour (overlayColour.withMultipliedAlpha (opacity));
            g.drawImageAt (image, 0, 0, true);
        }
    }
}

Rectangle<float> DrawableImage::getDrawableBounds() const
{
    return image.getBounds().toFloat();
}

bool DrawableImage::hitTest (int x, int y)
{
    return Drawable::hitTest (x, y) && image.isValid() && image.getPixelAt (x, y).getAlpha() >= 127;
}

Drawable* DrawableImage::createCopy() const
{
    return new DrawableImage (*this);
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

var DrawableImage::ValueTreeWrapper::getImageIdentifier() const
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

Colour DrawableImage::ValueTreeWrapper::getOverlayColour() const
{
    return Colour::fromString (state [overlay].toString());
}

void DrawableImage::ValueTreeWrapper::setOverlayColour (Colour newColour, UndoManager* undoManager)
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

RelativeParallelogram DrawableImage::ValueTreeWrapper::getBoundingBox() const
{
    return RelativeParallelogram (state.getProperty (topLeft, "0, 0"),
                                  state.getProperty (topRight, "100, 0"),
                                  state.getProperty (bottomLeft, "0, 100"));
}

void DrawableImage::ValueTreeWrapper::setBoundingBox (const RelativeParallelogram& newBounds, UndoManager* undoManager)
{
    state.setProperty (topLeft, newBounds.topLeft.toString(), undoManager);
    state.setProperty (topRight, newBounds.topRight.toString(), undoManager);
    state.setProperty (bottomLeft, newBounds.bottomLeft.toString(), undoManager);
}


//==============================================================================
void DrawableImage::refreshFromValueTree (const ValueTree& tree, ComponentBuilder& builder)
{
    const ValueTreeWrapper controller (tree);
    setComponentID (controller.getID());

    const float newOpacity = controller.getOpacity();
    const Colour newOverlayColour (controller.getOverlayColour());

    Image newImage;
    const var imageIdentifier (controller.getImageIdentifier());


    jassert (builder.getImageProvider() != 0 || imageIdentifier.isVoid()); // if you're using images, you need to provide something that can load and save them!

    if (builder.getImageProvider() != nullptr)
        newImage = builder.getImageProvider()->getImageForIdentifier (imageIdentifier);

    const RelativeParallelogram newBounds (controller.getBoundingBox());

    if (bounds != newBounds || newOpacity != opacity
         || overlayColour != newOverlayColour || image != newImage)
    {
        repaint();
        opacity = newOpacity;
        overlayColour = newOverlayColour;

        if (image != newImage)
            setImage (newImage);

        setBoundingBox (newBounds);
    }
}

ValueTree DrawableImage::createValueTree (ComponentBuilder::ImageProvider* imageProvider) const
{
    ValueTree tree (valueTreeType);
    ValueTreeWrapper v (tree);

    v.setID (getComponentID());
    v.setOpacity (opacity, nullptr);
    v.setOverlayColour (overlayColour, nullptr);
    v.setBoundingBox (bounds, nullptr);

    if (image.isValid())
    {
        jassert (imageProvider != nullptr); // if you're using images, you need to provide something that can load and save them!

        if (imageProvider != nullptr)
            v.setImageIdentifier (imageProvider->getIdentifierForImage (image), nullptr);
    }

    return tree;
}

Path DrawableImage::getOutlineAsPath() const
{
    return {}; // not applicable for images
}

} // namespace juce
