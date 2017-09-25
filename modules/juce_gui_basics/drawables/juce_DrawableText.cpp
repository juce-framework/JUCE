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

DrawableText::DrawableText()
    : colour (Colours::black),
      justification (Justification::centredLeft)
{
    setBoundingBox (RelativeParallelogram (RelativePoint (0.0f, 0.0f),
                                           RelativePoint (50.0f, 0.0f),
                                           RelativePoint (0.0f, 20.0f)));
    setFont (Font (15.0f), true);
}

DrawableText::DrawableText (const DrawableText& other)
    : Drawable (other),
      bounds (other.bounds),
      fontHeight (other.fontHeight),
      fontHScale (other.fontHScale),
      font (other.font),
      text (other.text),
      colour (other.colour),
      justification (other.justification)
{
    refreshBounds();
}

DrawableText::~DrawableText()
{
}

//==============================================================================
void DrawableText::setText (const String& newText)
{
    if (text != newText)
    {
        text = newText;
        refreshBounds();
    }
}

void DrawableText::setColour (Colour newColour)
{
    if (colour != newColour)
    {
        colour = newColour;
        repaint();
    }
}

void DrawableText::setFont (const Font& newFont, bool applySizeAndScale)
{
    if (font != newFont)
    {
        font = newFont;

        if (applySizeAndScale)
        {
            fontHeight = font.getHeight();
            fontHScale = font.getHorizontalScale();
        }

        refreshBounds();
    }
}

void DrawableText::setJustification (Justification newJustification)
{
    justification = newJustification;
    repaint();
}

void DrawableText::setBoundingBox (const RelativeParallelogram& newBounds)
{
    if (bounds != newBounds)
    {
        bounds = newBounds;
        refreshBounds();
    }
}

void DrawableText::setFontHeight (const RelativeCoordinate& newHeight)
{
    if (fontHeight != newHeight)
    {
        fontHeight = newHeight;
        refreshBounds();
    }
}

void DrawableText::setFontHorizontalScale (const RelativeCoordinate& newScale)
{
    if (fontHScale != newScale)
    {
        fontHScale = newScale;
        refreshBounds();
    }
}

void DrawableText::refreshBounds()
{
    if (bounds.isDynamic() || fontHeight.isDynamic() || fontHScale.isDynamic())
    {
        Drawable::Positioner<DrawableText>* const p = new Drawable::Positioner<DrawableText> (*this);
        setPositioner (p);
        p->apply();
    }
    else
    {
        setPositioner (0);
        recalculateCoordinates (0);
    }
}

bool DrawableText::registerCoordinates (RelativeCoordinatePositionerBase& pos)
{
    bool ok = pos.addPoint (bounds.topLeft);
    ok = pos.addPoint (bounds.topRight) && ok;
    ok = pos.addPoint (bounds.bottomLeft) && ok;
    ok = pos.addCoordinate (fontHeight) && ok;
    return pos.addCoordinate (fontHScale) && ok;
}

void DrawableText::recalculateCoordinates (Expression::Scope* scope)
{
    bounds.resolveThreePoints (resolvedPoints, scope);

    const float w = Line<float> (resolvedPoints[0], resolvedPoints[1]).getLength();
    const float h = Line<float> (resolvedPoints[0], resolvedPoints[2]).getLength();

    const float height = jlimit (0.01f, jmax (0.01f, h), (float) fontHeight.resolve (scope));
    const float hscale = jlimit (0.01f, jmax (0.01f, w), (float) fontHScale.resolve (scope));

    scaledFont = font;
    scaledFont.setHeight (height);
    scaledFont.setHorizontalScale (hscale);

    setBoundsToEnclose (getDrawableBounds());
    repaint();
}

//==============================================================================
Rectangle<int> DrawableText::getTextArea (float w, float h) const
{
    return Rectangle<float> (w, h).getSmallestIntegerContainer();
}

AffineTransform DrawableText::getTextTransform (float w, float h) const
{
    return AffineTransform::fromTargetPoints (0, 0, resolvedPoints[0].x, resolvedPoints[0].y,
                                              w, 0, resolvedPoints[1].x, resolvedPoints[1].y,
                                              0, h, resolvedPoints[2].x, resolvedPoints[2].y);
}

void DrawableText::paint (Graphics& g)
{
    transformContextToCorrectOrigin (g);

    const float w = Line<float> (resolvedPoints[0], resolvedPoints[1]).getLength();
    const float h = Line<float> (resolvedPoints[0], resolvedPoints[2]).getLength();

    g.addTransform (getTextTransform (w, h));
    g.setFont (scaledFont);
    g.setColour (colour);

    g.drawFittedText (text, getTextArea (w, h), justification, 0x100000);
}

Rectangle<float> DrawableText::getDrawableBounds() const
{
    return RelativeParallelogram::getBoundingBox (resolvedPoints);
}

Drawable* DrawableText::createCopy() const
{
    return new DrawableText (*this);
}

//==============================================================================
const Identifier DrawableText::valueTreeType ("Text");

const Identifier DrawableText::ValueTreeWrapper::text ("text");
const Identifier DrawableText::ValueTreeWrapper::colour ("colour");
const Identifier DrawableText::ValueTreeWrapper::font ("font");
const Identifier DrawableText::ValueTreeWrapper::justification ("justification");
const Identifier DrawableText::ValueTreeWrapper::topLeft ("topLeft");
const Identifier DrawableText::ValueTreeWrapper::topRight ("topRight");
const Identifier DrawableText::ValueTreeWrapper::bottomLeft ("bottomLeft");
const Identifier DrawableText::ValueTreeWrapper::fontHeight ("fontHeight");
const Identifier DrawableText::ValueTreeWrapper::fontHScale ("fontHScale");

//==============================================================================
DrawableText::ValueTreeWrapper::ValueTreeWrapper (const ValueTree& state_)
    : ValueTreeWrapperBase (state_)
{
    jassert (state.hasType (valueTreeType));
}

String DrawableText::ValueTreeWrapper::getText() const
{
    return state [text].toString();
}

void DrawableText::ValueTreeWrapper::setText (const String& newText, UndoManager* undoManager)
{
    state.setProperty (text, newText, undoManager);
}

Value DrawableText::ValueTreeWrapper::getTextValue (UndoManager* undoManager)
{
    return state.getPropertyAsValue (text, undoManager);
}

Colour DrawableText::ValueTreeWrapper::getColour() const
{
    return Colour::fromString (state [colour].toString());
}

void DrawableText::ValueTreeWrapper::setColour (Colour newColour, UndoManager* undoManager)
{
    state.setProperty (colour, newColour.toString(), undoManager);
}

Justification DrawableText::ValueTreeWrapper::getJustification() const
{
    return Justification ((int) state [justification]);
}

void DrawableText::ValueTreeWrapper::setJustification (Justification newJustification, UndoManager* undoManager)
{
    state.setProperty (justification, newJustification.getFlags(), undoManager);
}

Font DrawableText::ValueTreeWrapper::getFont() const
{
    return Font::fromString (state [font]);
}

void DrawableText::ValueTreeWrapper::setFont (const Font& newFont, UndoManager* undoManager)
{
    state.setProperty (font, newFont.toString(), undoManager);
}

Value DrawableText::ValueTreeWrapper::getFontValue (UndoManager* undoManager)
{
    return state.getPropertyAsValue (font, undoManager);
}

RelativeParallelogram DrawableText::ValueTreeWrapper::getBoundingBox() const
{
    return RelativeParallelogram (state [topLeft].toString(), state [topRight].toString(), state [bottomLeft].toString());
}

void DrawableText::ValueTreeWrapper::setBoundingBox (const RelativeParallelogram& newBounds, UndoManager* undoManager)
{
    state.setProperty (topLeft, newBounds.topLeft.toString(), undoManager);
    state.setProperty (topRight, newBounds.topRight.toString(), undoManager);
    state.setProperty (bottomLeft, newBounds.bottomLeft.toString(), undoManager);
}

RelativeCoordinate DrawableText::ValueTreeWrapper::getFontHeight() const
{
    return state [fontHeight].toString();
}

void DrawableText::ValueTreeWrapper::setFontHeight (const RelativeCoordinate& coord, UndoManager* undoManager)
{
    state.setProperty (fontHeight, coord.toString(), undoManager);
}

RelativeCoordinate DrawableText::ValueTreeWrapper::getFontHorizontalScale() const
{
    return state [fontHScale].toString();
}

void DrawableText::ValueTreeWrapper::setFontHorizontalScale (const RelativeCoordinate& coord, UndoManager* undoManager)
{
    state.setProperty (fontHScale, coord.toString(), undoManager);
}

//==============================================================================
void DrawableText::refreshFromValueTree (const ValueTree& tree, ComponentBuilder&)
{
    ValueTreeWrapper v (tree);
    setComponentID (v.getID());

    const RelativeParallelogram newBounds (v.getBoundingBox());
    const RelativeCoordinate newFontHeight (v.getFontHeight());
    const RelativeCoordinate newFontHScale (v.getFontHorizontalScale());
    const Colour newColour (v.getColour());
    const Justification newJustification (v.getJustification());
    const String newText (v.getText());
    const Font newFont (v.getFont());

    if (text != newText || font != newFont || justification != newJustification
         || colour != newColour || bounds != newBounds
         || newFontHeight != fontHeight || newFontHScale != fontHScale)
    {
        setBoundingBox (newBounds);
        setFontHeight (newFontHeight);
        setFontHorizontalScale (newFontHScale);
        setColour (newColour);
        setFont (newFont, false);
        setJustification (newJustification);
        setText (newText);
    }
}

ValueTree DrawableText::createValueTree (ComponentBuilder::ImageProvider*) const
{
    ValueTree tree (valueTreeType);
    ValueTreeWrapper v (tree);

    v.setID (getComponentID());
    v.setText (text, nullptr);
    v.setFont (font, nullptr);
    v.setJustification (justification, nullptr);
    v.setColour (colour, nullptr);
    v.setBoundingBox (bounds, nullptr);
    v.setFontHeight (fontHeight, nullptr);
    v.setFontHorizontalScale (fontHScale, nullptr);

    return tree;
}

Path DrawableText::getOutlineAsPath() const
{
    auto w = Line<float> (resolvedPoints[0], resolvedPoints[1]).getLength();
    auto h = Line<float> (resolvedPoints[0], resolvedPoints[2]).getLength();
    const auto area = getTextArea (w, h).toFloat();

    GlyphArrangement arr;
    arr.addFittedText (scaledFont, text,
                       area.getX(), area.getY(),
                       area.getWidth(), area.getHeight(),
                       justification,
                       0x100000);

    Path pathOfAllGlyphs;

    for (int i = 0; i < arr.getNumGlyphs(); ++i)
    {
        Path gylphPath;
        arr.getGlyph (i).createPath (gylphPath);
        pathOfAllGlyphs.addPath (gylphPath);
    }

    pathOfAllGlyphs.applyTransform (getTextTransform (w, h)
                                      .followedBy (getTransform()));

    return pathOfAllGlyphs;
}

} // namespace juce
