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

#ifdef _WIN32
 #include <gl/gl.h>
 #include <gl/glu.h>
#elif defined (LINUX)
 #include <GL/gl.h>
 #include <GL/glut.h>
 #undef KeyPress
#else
 #include <GLUT/glut.h>
#endif

#ifndef GL_BGRA_EXT
 #define GL_BGRA_EXT 0x80e1
#endif

//==============================================================================
class DemoOpenGLCanvas  : public OpenGLComponent,
                          public Timer
{
    float rotation, delta;
    Image* image;

public:
    DemoOpenGLCanvas()
    {
        rotation = 0.0f;
        delta = 1.0f;

        Image* im = ImageFileFormat::loadFrom (BinaryData::juce_png, BinaryData::juce_pngSize);
        image = new Image (Image::RGB, 512, 512, true);
        Graphics g (*image);
        g.fillAll (Colours::white);
        g.drawImage (im, 0, 0, 512, 512, 0, 0, im->getWidth(), im->getHeight());
        delete im;

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
              << pixFormat->fullSceneAntiAliasingNumSamples;

            Logger::outputDebugString (formatDescription);
        }
    }

    ~DemoOpenGLCanvas()
    {
        delete image;
    }

    // when the component creates a new internal context, this is called, and
    // we'll use the opportunity to create the textures needed.
    void newOpenGLContextCreated()
    {
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

        int stride, pixStride;
        const void* pixels = image->lockPixelDataReadOnly (0, 0, image->getWidth(), image->getHeight(), stride, pixStride);

        glTexImage2D (GL_TEXTURE_2D, 0, 4, image->getWidth(), image->getHeight(),
                      0, GL_RGB,
                      GL_UNSIGNED_BYTE, pixels);
        image->releasePixelDataReadOnly (pixels);

        glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
        glHint (GL_POINT_SMOOTH_HINT, GL_NICEST);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void mouseDrag (const MouseEvent& e)
    {
        delta = e.getDistanceFromDragStartX() / 100.0f;
        repaint();
    }

    void renderOpenGL()
    {
        glClearColor (0.8f, 0.0f, 0.4f, 0.0f);
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho (0.0, getWidth(), 0.0, getHeight(), 0, 1);

        glColor4f (1.0f, 1.0f, 1.0f, fabsf (::sinf (rotation / 100.0f)));
        glBegin(GL_QUADS);
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
    }

    void timerCallback()
    {
        rotation += delta;

        repaint();
    }
};

//==============================================================================
class OpenGLDemo  : public Component
{
    //==============================================================================
    DemoOpenGLCanvas* canvas;

public:
    //==============================================================================
    OpenGLDemo()
    {
        setName (T("OpenGL"));

        canvas = new DemoOpenGLCanvas();
        addAndMakeVisible (canvas);
    }

    ~OpenGLDemo()
    {
        deleteAllChildren();
    }

    void resized()
    {
        canvas->setBounds (10, 10, getWidth() - 20, getHeight() - 50);
    }
};


//==============================================================================
Component* createOpenGLDemo()
{
    return new OpenGLDemo();
}

#endif
