/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#include "../jucedemo_headers.h"

#if JUCE_OPENGL

//==============================================================================
class DemoOpenGLCanvas  : public Component,
                          public OpenGLRenderer,
                          public Timer
{
public:
    DemoOpenGLCanvas()
        : rotation (0.0f),
          textScrollPos (200)
    {
        infoLabel.setText ("These sliders demonstrate how components and 2D graphics can be rendered "
                           "using OpenGL by using the OpenGLContext class.", dontSendNotification);
        infoLabel.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (&infoLabel);
        infoLabel.setBounds ("parent.width * 0.05, bottom - 150, parent.width * 0.4, parent.height - 60");

        speedSlider.setRange (-10.0, 10.0, 0.1);
        speedSlider.setPopupMenuEnabled (true);
        speedSlider.setValue (Random::getSystemRandom().nextDouble() * 3.0, dontSendNotification);
        speedSlider.setSliderStyle (Slider::LinearHorizontal);
        speedSlider.setTextBoxStyle (Slider::TextBoxLeft, false, 80, 20);
        addAndMakeVisible (&speedSlider);
        speedSlider.setBounds ("parent.width * 0.05, parent.height - 65, parent.width * 0.6, top + 24");

        sizeSlider.setRange (0.2, 2.0, 0.01);
        sizeSlider.setPopupMenuEnabled (true);
        sizeSlider.setValue (Random::getSystemRandom().nextDouble() + 0.5, dontSendNotification);
        sizeSlider.setSliderStyle (Slider::LinearHorizontal);
        sizeSlider.setTextBoxStyle (Slider::TextBoxLeft, false, 80, 20);
        addAndMakeVisible (&sizeSlider);
        sizeSlider.setBounds ("parent.width * 0.05, parent.height - 35, parent.width * 0.6, top + 24");

        openGLContext.setRenderer (this);
        openGLContext.setComponentPaintingEnabled (true);
        openGLContext.setContinuousRepainting (true);
        openGLContext.attachTo (*this);

        startTimer (1000 / 30);
    }

    ~DemoOpenGLCanvas()
    {
        openGLContext.detach();
    }

    // when the component creates a new internal context, this is called, and
    // we'll use the opportunity to create some images to use as textures.
    void newOpenGLContextCreated()
    {
        logoImage = createLogoImage();
        dynamicTextureImage = Image (Image::ARGB, 128, 128, true, OpenGLImageType());
    }

    void openGLContextClosing()
    {
        // We have to make sure we release any openGL images before the
        // GL context gets closed..
        logoImage = Image::null;
        dynamicTextureImage = Image::null;
    }

    void mouseDown (const MouseEvent& e)
    {
        draggableOrientation.mouseDown (e.getPosition());
    }

    void mouseDrag (const MouseEvent& e)
    {
        draggableOrientation.mouseDrag (e.getPosition());
        openGLContext.triggerRepaint();
    }

    void resized()
    {
        draggableOrientation.setViewport (getLocalBounds());
    }

    void paint (Graphics&) {}

    void renderOpenGL()
    {
        OpenGLHelpers::clear (Colours::darkgrey.withAlpha (1.0f));

        updateTextureImage();  // this will update our dynamically-changing texture image.

        const float scale = (float) openGLContext.getRenderingScale();
        drawBackground2DStuff (scale); // draws some 2D content to demonstrate the OpenGLGraphicsContext class

        // Having used the juce 2D renderer, it will have messed-up a whole load of GL state, so
        // we'll put back any important settings before doing our normal GL 3D drawing..
        glEnable (GL_DEPTH_TEST);
        glDepthFunc (GL_LESS);
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable (GL_TEXTURE_2D);

       #if JUCE_USE_OPENGL_FIXED_FUNCTION
        OpenGLHelpers::prepareFor2D (roundToInt (scale * getWidth()),
                                     roundToInt (scale * getHeight()));
        OpenGLHelpers::setPerspective (45.0, getWidth() / (double) getHeight(), 0.1, 100.0);

        glTranslatef (0.0f, 0.0f, -5.0f);
        draggableOrientation.applyToOpenGLMatrix();

        // logoImage and dynamicTextureImage are actually OpenGL images, so we can use this utility function to
        // extract the frame buffer which is their backing store, and use it directly.
        OpenGLFrameBuffer* tex1 = OpenGLImageType::getFrameBufferFrom (logoImage);
        OpenGLFrameBuffer* tex2 = OpenGLImageType::getFrameBufferFrom (dynamicTextureImage);

        if (tex1 != nullptr && tex2 != nullptr)
        {
            // This draws the sides of our spinning cube.
            // I've used some of the juce helper functions, but you can also just use normal GL calls here too.
            tex1->draw3D (-1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f, Colours::white);
            tex1->draw3D (-1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, Colours::white);
            tex1->draw3D (-1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, Colours::white);
            tex2->draw3D (-1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f, Colours::white);
            tex2->draw3D ( 1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f, Colours::white);
            tex2->draw3D (-1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, Colours::white);
        }
       #endif
    }

    void updateTextureImage()
    {
        // This image is a special framebuffer-backed image, so when we draw to it, the context
        // will render directly into its framebuffer

        if (dynamicTextureImage.isValid())
        {
            dynamicTextureImage.clear (dynamicTextureImage.getBounds(),
                                       Colours::red.withRotatedHue (fabsf (::sinf (rotation / 300.0f))).withAlpha (0.7f));

            Graphics g (dynamicTextureImage);

            g.setFont (dynamicTextureImage.getHeight() / 3.0f);
            g.setColour (Colours::black);
            drawScrollingMessage (g, dynamicTextureImage.getHeight() / 2);
        }
    }

    void drawBackground2DStuff (float scale)
    {
        // Create an OpenGLGraphicsContext that will draw into this GL window..
        ScopedPointer<LowLevelGraphicsContext> glRenderer (createOpenGLGraphicsContext (openGLContext,
                                                                                        roundToInt (scale * getWidth()),
                                                                                        roundToInt (scale * getHeight())));

        if (glRenderer != nullptr)
        {
            Graphics g (*glRenderer);
            g.addTransform (AffineTransform::scale (scale));

            // This stuff just creates a spinning star shape and fills it..
            Path p;
            p.addStar (Point<float> (getWidth() * 0.7f, getHeight() * 0.4f), 7,
                       getHeight() * 0.4f * (float) sizeSlider.getValue(),
                       getHeight() * 0.4f,
                       rotation / 50.0f);

            g.setGradientFill (ColourGradient (Colours::green.withRotatedHue (fabsf (::sinf (rotation / 300.0f))),
                                               0, 0,
                                               Colours::green.withRotatedHue (fabsf (::cosf (rotation / -431.0f))),
                                               0, (float) getHeight(), false));
            g.fillPath (p);
        }
    }

    void timerCallback()
    {
        rotation += (float) speedSlider.getValue();
        textScrollPos += 1.4f;
    }

private:
    OpenGLContext openGLContext;
    Image logoImage, dynamicTextureImage;
    float rotation, textScrollPos;
    Draggable3DOrientation draggableOrientation;

    Slider speedSlider, sizeSlider;
    Label infoLabel;

    // Functions to create a couple of images to use as textures..
    static Image createLogoImage()
    {
        Image image (Image::ARGB, 256, 256, true, OpenGLImageType());

        if (image.isValid())
        {
            Graphics g (image);

            g.fillAll (Colours::lightgrey.withAlpha (0.8f));
            g.drawImageWithin (ImageFileFormat::loadFrom (BinaryData::juce_png, BinaryData::juce_pngSize),
                               0, 0, image.getWidth(), image.getHeight(), RectanglePlacement::stretchToFit);

            drawRandomStars (g, image.getWidth(), image.getHeight());
        }

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

    void drawScrollingMessage (Graphics& g, int y) const
    {
        g.drawSingleLineText ("The background, foreground and texture are all being drawn using the OpenGLGraphicsContext class, which "
                              "lets you use a standard JUCE 2D graphics context to render directly onto an OpenGL window or framebuffer...  ",
                              (int) -std::fmod (textScrollPos, 2500.0f), y);
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLDemo)
};

//==============================================================================
Component* createOpenGLDemo()
{
    return new OpenGLDemo();
}

#endif
