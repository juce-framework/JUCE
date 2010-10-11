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

#ifdef _WIN32
 #include <windows.h>
#endif

#include "../jucedemo_headers.h"

#if JUCE_OPENGL

#if JUCE_WINDOWS
 #include <gl/gl.h>
 #include <gl/glu.h>
#elif JUCE_LINUX
 #include <GL/gl.h>
 #include <GL/glut.h>
 #undef KeyPress
#elif JUCE_IPHONE
 #include <OpenGLES/ES1/gl.h>
 #include <OpenGLES/ES1/glext.h>
#elif JUCE_MAC
 #include <GLUT/glut.h>
#elif JUCE_IPHONE
 //#include <GL/glut.h>
#endif

#ifndef GL_BGRA_EXT
 #define GL_BGRA_EXT 0x80e1
#endif

//==============================================================================
class DemoOpenGLCanvas  : public OpenGLComponent,
                          public Timer
{
public:
    DemoOpenGLCanvas()
        : rotation (0.0f),
          delta (1.0f)
    {
#if JUCE_IPHONE
        // (On the iPhone, choose a format without a depth buffer)
        setPixelFormat (OpenGLPixelFormat (8, 8, 0, 0));
#endif

        image = Image (Image::RGB, 512, 512, true, Image::SoftwareImage);
        Graphics g (image);

        g.fillAll (Colours::white);
        g.drawImageWithin (ImageFileFormat::loadFrom (BinaryData::juce_png, BinaryData::juce_pngSize),
                           0, 0, 512, 512, RectanglePlacement::stretchToFit);

        startTimer (20);

        // Just for demo purposes, let's dump a list of all the available pixel formats..
        OwnedArray <OpenGLPixelFormat> availablePixelFormats;
        OpenGLPixelFormat::getAvailablePixelFormats (this, availablePixelFormats);

        for (int i = 0; i < availablePixelFormats.size(); ++i)
        {
            const OpenGLPixelFormat* const pixFormat = availablePixelFormats[i];

            String formatDescription;
            formatDescription
              << i << ": RGBA=(" << pixFormat->redBits
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
              << (int) pixFormat->fullSceneAntiAliasingNumSamples;

            Logger::outputDebugString (formatDescription);
        }
    }

    ~DemoOpenGLCanvas()
    {
    }

    // when the component creates a new internal context, this is called, and
    // we'll use the opportunity to create the textures needed.
    void newOpenGLContextCreated()
    {
#if ! JUCE_IPHONE
        // (no need to call makeCurrentContextActive(), as that will have
        // been done for us before the method call).
        glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
        glClearDepth (1.0);

        glDepthFunc (GL_LESS);
        glEnable (GL_DEPTH_TEST);
        glEnable (GL_TEXTURE_2D);
        glEnable (GL_BLEND);
        glShadeModel (GL_SMOOTH);

        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glPixelStorei (GL_UNPACK_ALIGNMENT, 4);

        Image::BitmapData srcData (image, false);

        glTexImage2D (GL_TEXTURE_2D, 0, 4, image.getWidth(), image.getHeight(),
                      0, GL_RGB,
                      GL_UNSIGNED_BYTE, srcData.data);

        glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
        glHint (GL_POINT_SMOOTH_HINT, GL_NICEST);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif
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

#if JUCE_IPHONE
        const GLfloat vertices[] = { -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f,  0.5f, 0.5f, 0.5f };
        const GLubyte colours[] = { 255, 255, 0, 255, 0, 255, 255, 255, 0, 0, 0, 0, 255, 0, 255, 255 };

        glOrthof (-1.0f, 1.0f, -1.5f, 1.5f, -1.0f, 1.0f);
        glMatrixMode (GL_MODELVIEW);
        glPushMatrix();
        glRotatef (rotation, 0.0f, 0.0f, 1.0f);

        glVertexPointer (2, GL_FLOAT, 0, vertices);
        glEnableClientState (GL_VERTEX_ARRAY);
        glColorPointer (4, GL_UNSIGNED_BYTE, 0, colours);
        glEnableClientState (GL_COLOR_ARRAY);

        glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
        glPopMatrix();
#else

        glOrtho (0.0, getWidth(), 0.0, getHeight(), 0, 1);

        glColor4f (1.0f, 1.0f, 1.0f, fabsf (::sinf (rotation / 100.0f)));
        glBegin (GL_QUADS);
            glTexCoord2i (0, 0); glVertex2f (50.0f, getHeight() - 50.0f);
            glTexCoord2i (1, 0); glVertex2f (getWidth() - 50.0f, getHeight() - 50.0f);
            glTexCoord2i (1, 1); glVertex2f (getWidth() - 50.0f, 50.0f);
            glTexCoord2i (0, 1); glVertex2f (50.0f, 50.0f);
        glEnd();

        glMatrixMode (GL_PROJECTION);
        glLoadIdentity();

        glClear (GL_DEPTH_BUFFER_BIT);
        gluPerspective (45.0f,
                        getWidth() / (GLfloat) getHeight(),
                        0.1f,
                        100.0f);

        glMatrixMode (GL_MODELVIEW);

        glLoadIdentity();
        glPushMatrix();

            glTranslatef (0.0f, 0.0f, -5.0f);
            glRotatef (rotation, 0.5f, 1.0f, 0.0f);

            glBegin (GL_QUADS);

                glColor3f (0.0f, 1.0f, 0.0f);

                glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
                glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
                glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
                glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);

                glColor3f (1.0f, 0.0f, 0.0f);

                glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
                glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
                glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
                glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);

                glColor3f (0.0f, 0.0f, 1.0f);

                glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
                glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
                glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
                glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f, -1.0f);

                glColor3f (1.0f, 1.0f, 0.0f);

                glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
                glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
                glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
                glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);

                glColor3f (0.0f, 1.0f, 1.0f);

                glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
                glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
                glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
                glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);

                glColor3f (1.0f, 0.0f, 1.0f);

                glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
                glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
                glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
                glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f,  1.0f, -1.0f);

            glEnd();

        glPopMatrix();
#endif
    }

    void timerCallback()
    {
        rotation += delta;

        repaint();
    }

private:
    float rotation, delta;
    Image image;
};


//==============================================================================
class OpenGLDemo  : public Component
{
public:
    //==============================================================================
    OpenGLDemo()
    {
        setName ("OpenGL");

        addAndMakeVisible (&canvas);
    }

    void resized()
    {
        canvas.setBounds (10, 10, getWidth() - 20, getHeight() - 50);
    }

private:
    DemoOpenGLCanvas canvas;

    OpenGLDemo (const OpenGLDemo&);
    OpenGLDemo& operator= (const OpenGLDemo&);
};


//==============================================================================
Component* createOpenGLDemo()
{
    return new OpenGLDemo();
}

#endif
