/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

BEGIN_JUCE_NAMESPACE

void OpenGLHelpers::resetErrorState()
{
    while (glGetError() != GL_NO_ERROR) {}
}

void OpenGLHelpers::clear (const Colour& colour)
{
    glClearColor (colour.getFloatRed(), colour.getFloatGreen(),
                  colour.getFloatBlue(), colour.getFloatAlpha());

    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void OpenGLHelpers::setColour (const Colour& colour)
{
    glColor4f (colour.getFloatRed(), colour.getFloatGreen(),
               colour.getFloatBlue(), colour.getFloatAlpha());
}

void OpenGLHelpers::prepareFor2D (int width, int height)
{
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();

   #if JUCE_OPENGL_ES
    glOrthof (0.0f, (float) width, 0.0f, (float) height, 0.0f, 1.0f);
   #else
    glOrtho  (0.0, width, 0.0, height, 0, 1);
   #endif

    glViewport (0, 0, width, height);
}

void OpenGLHelpers::setPerspective (double fovy, double aspect, double zNear, double zFar)
{
    glLoadIdentity();

   #if JUCE_OPENGL_ES
    const float ymax = (float) (zNear * tan (fovy * double_Pi / 360.0));
    const float ymin = -ymax;

    glFrustumf (ymin * (float) aspect, ymax * (float) aspect, ymin, ymax, (float) zNear, (float) zFar);
   #else
    const double ymax = zNear * tan (fovy * double_Pi / 360.0);
    const double ymin = -ymax;

    glFrustum  (ymin * aspect, ymax * aspect, ymin, ymax, zNear, zFar);
   #endif
}

void OpenGLHelpers::drawQuad2D (float x1, float y1,
                                float x2, float y2,
                                float x3, float y3,
                                float x4, float y4,
                                const Colour& colour)
{
    const GLfloat vertices[]      = { x1, y1, x2, y2, x4, y4, x3, y3 };
    const GLfloat textureCoords[] = { 0, 0, 1.0f, 0, 0, 1.0f, 1.0f, 1.0f };

    setColour (colour);

    glEnableClientState (GL_VERTEX_ARRAY);
    glVertexPointer (2, GL_FLOAT, 0, vertices);

    glEnableClientState (GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer (2, GL_FLOAT, 0, textureCoords);

    glDisableClientState (GL_COLOR_ARRAY);
    glDisableClientState (GL_NORMAL_ARRAY);

    glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
}

void OpenGLHelpers::drawQuad3D (float x1, float y1, float z1,
                                float x2, float y2, float z2,
                                float x3, float y3, float z3,
                                float x4, float y4, float z4,
                                const Colour& colour)
{
    const GLfloat vertices[]      = { x1, y1, z1, x2, y2, z2, x4, y4, z4, x3, y3, z3 };
    const GLfloat textureCoords[] = { 0, 0, 1.0f, 0, 0, 1.0f, 1.0f, 1.0f };

    setColour (colour);

    glEnableClientState (GL_VERTEX_ARRAY);
    glVertexPointer (3, GL_FLOAT, 0, vertices);

    glEnableClientState (GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer (2, GL_FLOAT, 0, textureCoords);

    glDisableClientState (GL_COLOR_ARRAY);
    glDisableClientState (GL_NORMAL_ARRAY);

    glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
}

namespace OpenGLGradientHelpers
{
    void drawTriangles (GLenum mode, const GLfloat* vertices, const GLfloat* textureCoords, const int numElements)
    {
        glEnableClientState (GL_VERTEX_ARRAY);
        glEnableClientState (GL_TEXTURE_COORD_ARRAY);
        glDisableClientState (GL_COLOR_ARRAY);
        glDisableClientState (GL_NORMAL_ARRAY);

        glVertexPointer (2, GL_FLOAT, 0, vertices);
        glTexCoordPointer (2, GL_FLOAT, 0, textureCoords);

        glColor4f (1.0f, 1.0f, 1.0f, 1.0f);
        glDrawArrays (mode, 0, numElements);
    }

    void fillWithLinearGradient (const Rectangle<int>& rect,
                                 const ColourGradient& grad,
                                 const AffineTransform& transform,
                                 const int textureSize)
    {
        const Point<float> p1 (grad.point1.transformedBy (transform));
        const Point<float> p2 (grad.point2.transformedBy (transform));
        const Point<float> p3 (Point<float> (grad.point1.getX() - (grad.point2.getY() - grad.point1.getY()) / textureSize,
                                             grad.point1.getY() + (grad.point2.getX() - grad.point1.getX()) / textureSize).transformedBy (transform));

        const AffineTransform textureTransform (AffineTransform::fromTargetPoints (p1.getX(), p1.getY(),  0.0f, 0.0f,
                                                                                   p2.getX(), p2.getY(),  1.0f, 0.0f,
                                                                                   p3.getX(), p3.getY(),  0.0f, 1.0f));

        const float l = (float) rect.getX();
        const float r = (float) rect.getRight();
        const float t = (float) rect.getY();
        const float b = (float) rect.getBottom();

        const GLfloat vertices[] = { l, t, r, t, l, b, r, b };
        GLfloat textureCoords[]  = { l, t, r, t, l, b, r, b };

        textureTransform.transformPoints (textureCoords[0], textureCoords[1], textureCoords[2], textureCoords[3]);
        textureTransform.transformPoints (textureCoords[4], textureCoords[5], textureCoords[6], textureCoords[7]);

        drawTriangles (GL_TRIANGLE_STRIP, vertices, textureCoords, 4);
    }

    void fillWithRadialGradient (const Rectangle<int>& rect,
                                 const ColourGradient& grad,
                                 const AffineTransform& transform)
    {
        const Point<float> centre (grad.point1.transformedBy (transform));

        const float screenRadius = centre.getDistanceFrom (rect.getCentre().toFloat())
                                    + Point<int> (rect.getWidth() / 2,
                                                  rect.getHeight() / 2).getDistanceFromOrigin()
                                    + 8.0f;

        const AffineTransform inverse (transform.inverted());
        const float renderingRadius = jmax (Point<float> (screenRadius, 0.0f).transformedBy (inverse).getDistanceFromOrigin(),
                                            Point<float> (0.0f, screenRadius).transformedBy (inverse).getDistanceFromOrigin());

        const int numDivisions = 80;
        GLfloat vertices      [6 + numDivisions * 4];
        GLfloat textureCoords [6 + numDivisions * 4];

        {
            const float originalRadius = grad.point1.getDistanceFrom (grad.point2);
            const float texturePos = renderingRadius / originalRadius;

            GLfloat* t = textureCoords;
            *t++ = 0.0f;
            *t++ = 0.0f;

            for (int i = numDivisions + 1; --i >= 0;)
            {
                *t++ = texturePos;
                *t++ = 0.0f;
                *t++ = texturePos;
                *t++ = 1.0f;
            }

            jassert (t == textureCoords + numElementsInArray (vertices));
        }

        {
            GLfloat* v = vertices;

            *v++ = centre.getX();
            *v++ = centre.getY();

            const Point<float> first (grad.point1.translated (renderingRadius, -renderingRadius).transformedBy (transform));
            Point<float> last (first);

            for (int i = 0; i < numDivisions; ++i)
            {
                const float angle = (i + 1) * (float_Pi * 4.0f / numDivisions);
                const Point<float> next (grad.point1.translated (std::sin (angle) * renderingRadius,
                                                                 -std::cos (angle) * renderingRadius)
                                                    .transformedBy (transform));
                *v++ = last.getX();
                *v++ = last.getY();
                *v++ = next.getX();
                *v++ = next.getY();
                last = next;
            }

            *v++ = last.getX();
            *v++ = last.getY();
            *v++ = first.getX();
            *v++ = first.getY();

            jassert (v == vertices + numElementsInArray (vertices));
        }

        glEnable (GL_SCISSOR_TEST);
        glScissor (rect.getX(), rect.getY(), rect.getWidth(), rect.getHeight());
        drawTriangles (GL_TRIANGLE_FAN, vertices, textureCoords, numDivisions + 3);
        glDisable (GL_SCISSOR_TEST);
    }
}

void OpenGLHelpers::fillRectWithColourGradient (const Rectangle<int>& rect,
                                                const ColourGradient& gradient,
                                                const AffineTransform& transform)
{
    const int textureSize = 256;
    OpenGLTexture texture;

    HeapBlock<PixelARGB> lookup (textureSize);
    gradient.createLookupTable (lookup, textureSize);
    texture.load (lookup, textureSize, 1);
    texture.bind();

    if (gradient.isOpaque())
        glDisable (GL_BLEND);
    else
        glEnable (GL_BLEND);

    if (gradient.isRadial)
        OpenGLGradientHelpers::fillWithRadialGradient (rect, gradient, transform);
    else
        OpenGLGradientHelpers::fillWithLinearGradient (rect, gradient, transform, textureSize);
}

END_JUCE_NAMESPACE
