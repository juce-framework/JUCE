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

#include "../jucedemo_headers.h"

#if JUCE_OPENGL

//==============================================================================
// Simple wrapper for an openGL texture..
class OpenGLTexture
{
public:
    OpenGLTexture()
        : textureID (0), width (0), height (0)
    {
    }

    ~OpenGLTexture()
    {
        release();
    }

    void load (const Image& image)
    {
        release();

        width  = image.getWidth();
        height = image.getHeight();

        jassert (BitArray (width).countNumberOfSetBits() == 1); // these dimensions must be a power-of-two
        jassert (BitArray (height).countNumberOfSetBits() == 1);

        glGenTextures (1, &textureID);
        glBindTexture (GL_TEXTURE_2D, textureID);

        glTexEnvf (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

        glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
       #if ! JUCE_OPENGL_ES
        glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
       #endif

        glPixelStorei (GL_UNPACK_ALIGNMENT, 4);

        Image::BitmapData srcData (image, Image::BitmapData::readOnly);

        glTexImage2D (GL_TEXTURE_2D, 0, internalFormat,
                      width, height, 0,
                      image.getFormat() == Image::RGB ? GL_RGB : GL_BGRA_EXT,
                      GL_UNSIGNED_BYTE, srcData.data);
    }

    void release()
    {
        if (textureID != 0)
        {
            glDeleteTextures (1, &textureID);
            textureID = 0;
            width = 0;
            height = 0;
        }
    }

    void bind() const
    {
        glBindTexture (GL_TEXTURE_2D, textureID);
    }

    void draw2D (float x1, float y1,
                 float x2, float y2,
                 float x3, float y3,
                 float x4, float y4,
                 float alpha) const
    {
        bind();
        glColor4f (1.0f, 1.0f, 1.0f, alpha);

       #if JUCE_OPENGL_ES
        const GLfloat vertices[]      = { x1, y1, x2, y2, x4, y4, x3, y3 };
        const GLfloat textureCoords[] = { 0, 0, 1.0f, 0, 0, 1.0f, 1.0f, 1.0f };

        glEnableClientState (GL_VERTEX_ARRAY);
        glVertexPointer (2, GL_FLOAT, 0, vertices);

        glEnableClientState (GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer (2, GL_FLOAT, 0, textureCoords);

        glDisableClientState (GL_COLOR_ARRAY);
        glDisableClientState (GL_NORMAL_ARRAY);

        glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);

       #else
        glBegin (GL_QUADS);
        glTexCoord2i (0, 0); glVertex2f (x1, y1);
        glTexCoord2i (1, 0); glVertex2f (x2, y2);
        glTexCoord2i (1, 1); glVertex2f (x3, y3);
        glTexCoord2i (0, 1); glVertex2f (x4, y4);
        glEnd();
       #endif
    }

    void draw3D (float x1, float y1, float z1,
                 float x2, float y2, float z2,
                 float x3, float y3, float z3,
                 float x4, float y4, float z4,
                 float alpha) const
    {
        bind();
        glColor4f (1.0f, 1.0f, 1.0f, alpha);

       #if JUCE_OPENGL_ES
        const GLfloat vertices[]      = { x1, y1, z1, x2, y2, z2, x4, y4, z4, x3, y3, z3 };
        const GLfloat textureCoords[] = { 0, 0, 1.0f, 0, 0, 1.0f, 1.0f, 1.0f };

        glEnableClientState (GL_VERTEX_ARRAY);
        glVertexPointer (3, GL_FLOAT, 0, vertices);

        glEnableClientState (GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer (2, GL_FLOAT, 0, textureCoords);

        glDisableClientState (GL_COLOR_ARRAY);
        glDisableClientState (GL_NORMAL_ARRAY);

        glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);

       #else
        glBegin (GL_QUADS);
        glTexCoord2i (0, 0); glVertex3f (x1, y1, z1);
        glTexCoord2i (1, 0); glVertex3f (x2, y2, z2);
        glTexCoord2i (1, 1); glVertex3f (x3, y3, z3);
        glTexCoord2i (0, 1); glVertex3f (x4, y4, z4);
        glEnd();
       #endif
    }

private:
    GLuint textureID;
    int width, height;

   #if JUCE_OPENGL_ES
    enum { internalFormat = GL_RGBA };
   #else
    enum { internalFormat = 4 };
   #endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLTexture);
};

//==============================================================================
class DemoOpenGLCanvas  : public OpenGLComponent,
                          public Timer
{
public:
    DemoOpenGLCanvas()
        : rotation (0.0f),
          delta (1.0f)
    {
        startTimer (20);

        // Just for demo purposes, let's dump a list of all the available pixel formats..
        OwnedArray <OpenGLPixelFormat> availablePixelFormats;
        OpenGLPixelFormat::getAvailablePixelFormats (this, availablePixelFormats);

        for (int i = 0; i < availablePixelFormats.size(); ++i)
        {
            const OpenGLPixelFormat* const pixFormat = availablePixelFormats[i];

            DBG (i << ": RGBA=(" << pixFormat->redBits
                   << ", " << pixFormat->greenBits
                   << ", " << pixFormat->blueBits
                   << ", " << pixFormat->alphaBits
                   << "), depth=" << pixFormat->depthBufferBits
                   << ", stencil=" << pixFormat->stencilBufferBits
                   << ", accum RGBA=(" << pixFormat->accumulationBufferRedBits
                   << ", " << pixFormat->accumulationBufferGreenBits
                   << ", " << pixFormat->accumulationBufferBlueBits
                   << ", " << pixFormat->accumulationBufferAlphaBits
                   << "), full-scene AA="
                   << (int) pixFormat->fullSceneAntiAliasingNumSamples);
        }
    }

    // when the component creates a new internal context, this is called, and
    // we'll use the opportunity to create the textures needed.
    void newOpenGLContextCreated()
    {
        // (no need to call makeCurrentContextActive(), as that will have
        // been done for us before the method call).
        glClearColor (0.0f, 0.0f, 0.0f, 0.0f);

       #if ! JUCE_OPENGL_ES
        glClearDepth (1.0);
       #endif

        glDepthFunc (GL_LESS);
        glEnable (GL_DEPTH_TEST);
        glEnable (GL_TEXTURE_2D);
        glEnable (GL_BLEND);
        glShadeModel (GL_SMOOTH);

        glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
        glHint (GL_POINT_SMOOTH_HINT, GL_NICEST);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        texture1.load (createImage1());
        texture2.load (createImage2());
    }

    void mouseDrag (const MouseEvent& e)
    {
        delta = e.getDistanceFromDragStartX() / 100.0f;
        repaint();
    }

    void renderOpenGL()
    {
        glClearColor (0.25f, 0.25f, 0.25f, 0.0f);
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode (GL_PROJECTION);
        glLoadIdentity();

       #if JUCE_OPENGL_ES
        glOrthof (0.0f, (float) getWidth(), 0.0f, (float) getHeight(), 0.0f, 1.0f);
       #else
        glOrtho (0.0, getWidth(), 0.0, getHeight(), 0, 1);
       #endif

        texture1.draw2D (50.0f, getHeight() - 50.0f,
                         getWidth() - 50.0f, getHeight() - 50.0f,
                         getWidth() - 50.0f, 50.0f,
                         50.0f, 50.0f,
                         fabsf (::sinf (rotation / 100.0f)));

        glLoadIdentity();
        glClear (GL_DEPTH_BUFFER_BIT);

        setPerspective (45.0, getWidth() / (double) getHeight(), 0.1, 100.0);

        glMatrixMode (GL_MODELVIEW);

        glPushMatrix();
        glTranslatef (0.0f, 0.0f, -5.0f);
        glRotatef (rotation, 0.5f, 1.0f, 0.0f);

        // this draws the sides of our spinning cube..
        texture1.draw3D (-1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, 1.0f);
        texture1.draw3D (-1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f, 1.0f);
        texture1.draw3D (-1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, 1.0f);
        texture2.draw3D (-1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f, 1.0f);
        texture2.draw3D ( 1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f, 1.0f);
        texture2.draw3D (-1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, 1.0f);

        glPopMatrix();
    }

    void timerCallback()
    {
        rotation += delta;
        repaint();
    }

private:
    OpenGLTexture texture1, texture2;
    float rotation, delta;

    // Utility function to do the same job as gluPerspective()
    static void setPerspective (double fovy, double aspect, double zNear, double zFar)
    {
        const double ymax = zNear * tan (fovy * double_Pi / 360.0);
        const double ymin = -ymax;

       #if JUCE_OPENGL_ES
        glFrustumf (ymin * aspect, ymax * aspect, ymin, ymax, zNear, zFar);
       #else
        glFrustum  (ymin * aspect, ymax * aspect, ymin, ymax, zNear, zFar);
       #endif
    }

    // Functions to create a couple of images to use as textures..
    static Image createImage1()
    {
        Image image (Image::ARGB, 256, 256, true, Image::SoftwareImage);

        Graphics g (image);

        g.fillAll (Colours::white.withAlpha (0.7f));
        g.drawImageWithin (ImageFileFormat::loadFrom (BinaryData::juce_png, BinaryData::juce_pngSize),
                           0, 0, image.getWidth(), image.getHeight(), RectanglePlacement::stretchToFit);

        return image;
    }

    static Image createImage2()
    {
        Image image (Image::ARGB, 128, 128, true, Image::SoftwareImage);

        Graphics g (image);
        g.fillAll (Colours::darkred.withAlpha (0.7f));

        Path p;
        p.addStar (image.getBounds().getCentre().toFloat(), 11, image.getWidth() * 0.3f, image.getWidth() * 0.5f);

        g.setGradientFill (ColourGradient (Colours::blue,  image.getWidth() * 0.5f, image.getHeight() * 0.5f,
                                           Colours::green, image.getWidth() * 0.2f, image.getHeight() * 0.2f,
                                           true));
        g.fillPath (p);

        return image;
    }
};


//==============================================================================
class OpenGLDemo  : public Component
{
public:
    OpenGLDemo()
        : Component ("OpenGL")
    {
        addAndMakeVisible (&canvas);
    }

    void resized()
    {
        canvas.setBounds (10, 10, getWidth() - 20, getHeight() - 50);
    }

private:
    DemoOpenGLCanvas canvas;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLDemo);
};

//==============================================================================
Component* createOpenGLDemo()
{
    return new OpenGLDemo();
}

#endif
