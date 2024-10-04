/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

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
