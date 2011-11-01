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
class DemoOpenGLCanvas  : public OpenGLComponent,
                          public Timer
{
public:
    DemoOpenGLCanvas()
        : rotation (0.0f),
          delta (1.0f),
          textScrollPos (200)
    {
        startTimer (20);
    }

    // when the component creates a new internal context, this is called, and
    // we'll use the opportunity to create some images to use as textures.
    void newOpenGLContextCreated()
    {
        logoImage = createLogoImage();
        dynamicTextureImage = Image (Image::ARGB, 128, 128, true, OpenGLImageType());
    }

    void mouseDrag (const MouseEvent& e)
    {
        delta = e.getDistanceFromDragStartX() / 100.0f;
        repaint();
    }

    void renderOpenGL()
    {
        OpenGLHelpers::clear (Colours::darkgrey.withAlpha (1.0f));

        updateTextureImage();  // this will update our dynamically-changing texture image.

        drawBackground2DStuff(); // draws some 2D content to demonstrate the OpenGLRenderer class

        // Having used the juce 2D renderer, it will have messed-up a whole load of GL state, so
        // we'll put back any important settings before doing our normal GL 3D drawing..
        glEnable (GL_DEPTH_TEST);
        glDepthFunc (GL_LESS);
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable (GL_TEXTURE_2D);

        OpenGLHelpers::setPerspective (45.0, getWidth() / (double) getHeight(), 0.1, 100.0);

        glTranslatef (0.0f, 0.0f, -5.0f);
        glRotatef (rotation, 0.5f, 1.0f, 0.0f);

        // logoImage and dynamicTextureImage are actually OpenGL images, so we can use this utility function to
        // extract the frame buffer which is their backing store, and use it directly.
        OpenGLFrameBuffer* tex1 = OpenGLImageType::getFrameBufferFrom (logoImage);
        OpenGLFrameBuffer* tex2 = OpenGLImageType::getFrameBufferFrom (dynamicTextureImage);

        jassert (tex1 != nullptr && tex2 != nullptr); // (this would mean that our images weren't created correctly)

        // This draws the sides of our spinning cube.
        // I've used some of the juce helper functions, but you can also just use normal GL calls here too.
        tex1->draw3D (-1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f, Colours::white);
        tex1->draw3D (-1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, Colours::white);
        tex1->draw3D (-1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, Colours::white);
        tex2->draw3D (-1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f, Colours::white);
        tex2->draw3D ( 1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f, Colours::white);
        tex2->draw3D (-1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, Colours::white);

        drawForeground2DStuff(); // draws our scrolling text overlay
    }

    void updateTextureImage()
    {
        // This image is a special framebuffer-backed image, so when we draw to it, the context
        // will render directly into its framebuffer

        dynamicTextureImage.clear (dynamicTextureImage.getBounds(), Colours::red.withRotatedHue (fabsf (::sinf (rotation / 300.0f))).withAlpha (0.7f));

        Graphics g (dynamicTextureImage);

        g.setFont (dynamicTextureImage.getHeight() / 3.0f);
        g.setColour (Colours::black);
        drawScrollingMessage (g, dynamicTextureImage.getHeight() / 2);
    }

    void drawBackground2DStuff()
    {
        OpenGLRenderer glRenderer (*this); // Create an OpenGLRenderer that will draw into this GL window..
        Graphics g (&glRenderer);          // ..and then wrap it in a normal Graphics object so we can draw with it.

        // This stuff just creates a spinning star shape and fills it..
        Path p;
        const float scale = getHeight() * 0.4f;
        p.addStar (getLocalBounds().getCentre().toFloat(), 7,
                   scale + ::cosf (rotation * 0.0021f) * scale / 2,
                   scale + ::sinf (rotation * 0.001f) * scale / 2, rotation / 50.0f);

        g.setGradientFill (ColourGradient (Colours::green.withRotatedHue (fabsf (::sinf (rotation / 300.0f))),
                                           0, 0,
                                           Colours::green.withRotatedHue (fabsf (::cosf (rotation / -431.0f))),
                                           0, (float) getHeight(), false));
        g.fillPath (p);
    }

    void drawForeground2DStuff()
    {
        OpenGLRenderer glRenderer (*this); // Create an OpenGLRenderer that will draw into this GL window..
        Graphics g (&glRenderer);          // ..and then wrap it in a normal Graphics object so we can draw with it.

        // Then, just draw our scolling text like we would in any other component.
        g.setColour (Colours::blue.withAlpha (0.5f));
        g.setFont (30.0f, Font::bold);
        drawScrollingMessage (g, getHeight() / 2);
    }

    void drawScrollingMessage (Graphics& g, int y) const
    {
        g.drawSingleLineText ("The background, foreground and texture are all being drawn using the OpenGLRenderer class, which "
                              "lets you use a standard JUCE 2D graphics context to render directly onto an OpenGL window or framebuffer...  ",
                              (int) -std::fmod (textScrollPos, 2500.0f), y);
    }

    void timerCallback()
    {
        rotation += delta;
        textScrollPos += 1.4f;
        repaint();
    }

private:
    Image logoImage, dynamicTextureImage;
    float rotation, delta, textScrollPos;

    // Functions to create a couple of images to use as textures..
    static Image createLogoImage()
    {
        Image image (Image::ARGB, 256, 256, true, OpenGLImageType());

        Graphics g (image);

        g.fillAll (Colours::lightgrey.withAlpha (0.8f));
        g.drawImageWithin (ImageFileFormat::loadFrom (BinaryData::juce_png, BinaryData::juce_pngSize),
                           0, 0, image.getWidth(), image.getHeight(), RectanglePlacement::stretchToFit);

        drawRandomStars (g, image.getWidth(), image.getHeight());
        return image;
    }

    static void drawRandomStars (Graphics& g, int w, int h)
    {
        Random r;
        for (int i = 10; --i >= 0;)
        {
            Path pp;
            pp.addStar (Point<float> (r.nextFloat() * w, r.nextFloat() * h), r.nextInt (8) + 3, 10.0f, 20.0f, 0.0f);
            g.setColour (Colours::pink.withAlpha (0.4f));
            g.fillPath (pp);
        }
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
