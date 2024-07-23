/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

Box2DRenderer::Box2DRenderer() noexcept   : graphics (nullptr)
{
    SetFlags (e_shapeBit);
}

void Box2DRenderer::render (Graphics& g, b2World& world,
                            float left, float top, float right, float bottom,
                            const Rectangle<float>& target)
{
    graphics = &g;

    g.addTransform (AffineTransform::fromTargetPoints (left,  top,    target.getX(),     target.getY(),
                                                       right, top,    target.getRight(), target.getY(),
                                                       left,  bottom, target.getX(),     target.getBottom()));

    world.SetDebugDraw (this);
    world.DrawDebugData();
}

Colour Box2DRenderer::getColour (const b2Color& c) const
{
    return Colour::fromFloatRGBA (c.r, c.g, c.b, 1.0f);
}

float Box2DRenderer::getLineThickness() const
{
    return 0.1f;
}

static void createPath (Path& p, const b2Vec2* vertices, int32 vertexCount)
{
    p.startNewSubPath (vertices[0].x, vertices[0].y);

    for (int i = 1; i < vertexCount; ++i)
        p.lineTo (vertices[i].x, vertices[i].y);

    p.closeSubPath();
}

void Box2DRenderer::DrawPolygon (const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
    graphics->setColour (getColour (color));

    Path p;
    createPath (p, vertices, vertexCount);
    graphics->strokePath (p, PathStrokeType (getLineThickness()));
}

void Box2DRenderer::DrawSolidPolygon (const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
    graphics->setColour (getColour (color));

    Path p;
    createPath (p, vertices, vertexCount);
    graphics->fillPath (p);
}

void Box2DRenderer::DrawCircle (const b2Vec2& center, float32 radius, const b2Color& color)
{
    graphics->setColour (getColour (color));
    graphics->drawEllipse (center.x - radius, center.y - radius,
                           radius * 2.0f, radius * 2.0f,
                           getLineThickness());
}

void Box2DRenderer::DrawSolidCircle (const b2Vec2& center, float32 radius, const b2Vec2& /*axis*/, const b2Color& colour)
{
    graphics->setColour (getColour (colour));
    graphics->fillEllipse (center.x - radius, center.y - radius,
                           radius * 2.0f, radius * 2.0f);
}

void Box2DRenderer::DrawSegment (const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
    graphics->setColour (getColour (color));
    graphics->drawLine (p1.x, p1.y, p2.x, p2.y, getLineThickness());
}

void Box2DRenderer::DrawTransform (const b2Transform&)
{
}

} // namespace juce
