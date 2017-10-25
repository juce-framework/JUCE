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

#include "../JuceDemoHeader.h"

#if JUCE_OPENGL

#include "WavefrontObjParser.h"


//==============================================================================
struct OpenGLDemoClasses
{
    /** Vertex data to be passed to the shaders.
        For the purposes of this demo, each vertex will have a 3D position, a colour and a
        2D texture co-ordinate. Of course you can ignore these or manipulate them in the
        shader programs but are some useful defaults to work from.
     */
    struct Vertex
    {
        float position[3];
        float normal[3];
        float colour[4];
        float texCoord[2];
    };

    //==============================================================================
    // This class just manages the attributes that the demo shaders use.
    struct Attributes
    {
        Attributes (OpenGLContext& openGLContext, OpenGLShaderProgram& shader)
        {
            position       = createAttribute (openGLContext, shader, "position");
            normal         = createAttribute (openGLContext, shader, "normal");
            sourceColour   = createAttribute (openGLContext, shader, "sourceColour");
            textureCoordIn = createAttribute (openGLContext, shader, "textureCoordIn");
        }

        void enable (OpenGLContext& openGLContext)
        {
            if (position != nullptr)
            {
                openGLContext.extensions.glVertexAttribPointer (position->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), 0);
                openGLContext.extensions.glEnableVertexAttribArray (position->attributeID);
            }

            if (normal != nullptr)
            {
                openGLContext.extensions.glVertexAttribPointer (normal->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 3));
                openGLContext.extensions.glEnableVertexAttribArray (normal->attributeID);
            }

            if (sourceColour != nullptr)
            {
                openGLContext.extensions.glVertexAttribPointer (sourceColour->attributeID, 4, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 6));
                openGLContext.extensions.glEnableVertexAttribArray (sourceColour->attributeID);
            }

            if (textureCoordIn != nullptr)
            {
                openGLContext.extensions.glVertexAttribPointer (textureCoordIn->attributeID, 2, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 10));
                openGLContext.extensions.glEnableVertexAttribArray (textureCoordIn->attributeID);
            }
        }

        void disable (OpenGLContext& openGLContext)
        {
            if (position != nullptr)       openGLContext.extensions.glDisableVertexAttribArray (position->attributeID);
            if (normal != nullptr)         openGLContext.extensions.glDisableVertexAttribArray (normal->attributeID);
            if (sourceColour != nullptr)   openGLContext.extensions.glDisableVertexAttribArray (sourceColour->attributeID);
            if (textureCoordIn != nullptr)  openGLContext.extensions.glDisableVertexAttribArray (textureCoordIn->attributeID);
        }

        ScopedPointer<OpenGLShaderProgram::Attribute> position, normal, sourceColour, textureCoordIn;

    private:
        static OpenGLShaderProgram::Attribute* createAttribute (OpenGLContext& openGLContext,
                                                                OpenGLShaderProgram& shader,
                                                                const char* attributeName)
        {
            if (openGLContext.extensions.glGetAttribLocation (shader.getProgramID(), attributeName) < 0)
                return nullptr;

            return new OpenGLShaderProgram::Attribute (shader, attributeName);
        }
    };

    //==============================================================================
    // This class just manages the uniform values that the demo shaders use.
    struct Uniforms
    {
        Uniforms (OpenGLContext& openGLContext, OpenGLShaderProgram& shader)
        {
            projectionMatrix = createUniform (openGLContext, shader, "projectionMatrix");
            viewMatrix       = createUniform (openGLContext, shader, "viewMatrix");
            texture          = createUniform (openGLContext, shader, "demoTexture");
            lightPosition    = createUniform (openGLContext, shader, "lightPosition");
            bouncingNumber   = createUniform (openGLContext, shader, "bouncingNumber");
        }

        ScopedPointer<OpenGLShaderProgram::Uniform> projectionMatrix, viewMatrix, texture, lightPosition, bouncingNumber;

    private:
        static OpenGLShaderProgram::Uniform* createUniform (OpenGLContext& openGLContext,
                                                            OpenGLShaderProgram& shader,
                                                            const char* uniformName)
        {
            if (openGLContext.extensions.glGetUniformLocation (shader.getProgramID(), uniformName) < 0)
                return nullptr;

            return new OpenGLShaderProgram::Uniform (shader, uniformName);
        }
    };

    //==============================================================================
    /** This loads a 3D model from an OBJ file and converts it into some vertex buffers
        that we can draw.
    */
    struct Shape
    {
        Shape (OpenGLContext& openGLContext)
        {
            if (shapeFile.load (BinaryData::teapot_obj).wasOk())
                for (int i = 0; i < shapeFile.shapes.size(); ++i)
                    vertexBuffers.add (new VertexBuffer (openGLContext, *shapeFile.shapes.getUnchecked(i)));

        }

        void draw (OpenGLContext& openGLContext, Attributes& attributes)
        {
            for (int i = 0; i < vertexBuffers.size(); ++i)
            {
                VertexBuffer& vertexBuffer = *vertexBuffers.getUnchecked (i);
                vertexBuffer.bind();

                attributes.enable (openGLContext);
                glDrawElements (GL_TRIANGLES, vertexBuffer.numIndices, GL_UNSIGNED_INT, 0);
                attributes.disable (openGLContext);
            }
        }

    private:
        struct VertexBuffer
        {
            VertexBuffer (OpenGLContext& context, WavefrontObjFile::Shape& shape) : openGLContext (context)
            {
                numIndices = shape.mesh.indices.size();

                openGLContext.extensions.glGenBuffers (1, &vertexBuffer);
                openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);

                Array<Vertex> vertices;
                createVertexListFromMesh (shape.mesh, vertices, Colours::green);

                openGLContext.extensions.glBufferData (GL_ARRAY_BUFFER, vertices.size() * (int) sizeof (Vertex),
                                                       vertices.getRawDataPointer(), GL_STATIC_DRAW);

                openGLContext.extensions.glGenBuffers (1, &indexBuffer);
                openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
                openGLContext.extensions.glBufferData (GL_ELEMENT_ARRAY_BUFFER, numIndices * (int) sizeof (juce::uint32),
                                                       shape.mesh.indices.getRawDataPointer(), GL_STATIC_DRAW);
            }

            ~VertexBuffer()
            {
                openGLContext.extensions.glDeleteBuffers (1, &vertexBuffer);
                openGLContext.extensions.glDeleteBuffers (1, &indexBuffer);
            }

            void bind()
            {
                openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
                openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
            }

            GLuint vertexBuffer, indexBuffer;
            int numIndices;
            OpenGLContext& openGLContext;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VertexBuffer)
        };

        WavefrontObjFile shapeFile;
        OwnedArray<VertexBuffer> vertexBuffers;

        static void createVertexListFromMesh (const WavefrontObjFile::Mesh& mesh, Array<Vertex>& list, Colour colour)
        {
            const float scale = 0.2f;
            WavefrontObjFile::TextureCoord defaultTexCoord = { 0.5f, 0.5f };
            WavefrontObjFile::Vertex defaultNormal = { 0.5f, 0.5f, 0.5f };

            for (int i = 0; i < mesh.vertices.size(); ++i)
            {
                const WavefrontObjFile::Vertex& v = mesh.vertices.getReference (i);

                const WavefrontObjFile::Vertex& n
                        = i < mesh.normals.size() ? mesh.normals.getReference (i) : defaultNormal;

                const WavefrontObjFile::TextureCoord& tc
                        = i < mesh.textureCoords.size() ? mesh.textureCoords.getReference (i) : defaultTexCoord;

                Vertex vert =
                {
                    { scale * v.x, scale * v.y, scale * v.z, },
                    { scale * n.x, scale * n.y, scale * n.z, },
                    { colour.getFloatRed(), colour.getFloatGreen(), colour.getFloatBlue(), colour.getFloatAlpha() },
                    { tc.x, tc.y }
                };

                list.add (vert);
            }
        }
    };

    //==============================================================================
    // These classes are used to load textures from the various sources that the demo uses..
    struct DemoTexture
    {
        virtual ~DemoTexture() {}
        virtual bool applyTo (OpenGLTexture&) = 0;

        String name;
    };

    struct DynamicTexture   : public DemoTexture
    {
        DynamicTexture() { name = "Dynamically-generated texture"; }

        Image image;
        BouncingNumber x, y;

        bool applyTo (OpenGLTexture& texture) override
        {
            const int size = 128;

            if (! image.isValid())
                image = Image (Image::ARGB, size, size, true);

            {
                Graphics g (image);
                g.fillAll (Colours::lightcyan);

                g.setColour (Colours::darkred);
                g.drawRect (0, 0, size, size, 2);

                g.setColour (Colours::green);
                g.fillEllipse (x.getValue() * size * 0.9f, y.getValue() * size * 0.9f, size * 0.1f, size * 0.1f);

                g.setColour (Colours::black);
                g.setFont (40);
                g.drawFittedText (String (Time::getCurrentTime().getMilliseconds()), image.getBounds(), Justification::centred, 1);
            }

            texture.loadImage (image);
            return true;
        }
    };

    struct BuiltInTexture   : public DemoTexture
    {
        BuiltInTexture (const char* nm, const void* imageData, size_t imageSize)
            : image (resizeImageToPowerOfTwo (ImageFileFormat::loadFrom (imageData, imageSize)))
        {
            name = nm;
        }

        Image image;

        bool applyTo (OpenGLTexture& texture) override
        {
            texture.loadImage (image);
            return false;
        }
    };

    struct TextureFromFile   : public DemoTexture
    {
        TextureFromFile (const File& file)
        {
            name = file.getFileName();
            image = resizeImageToPowerOfTwo (ImageFileFormat::loadFrom (file));
        }

        Image image;

        bool applyTo (OpenGLTexture& texture) override
        {
            texture.loadImage (image);
            return false;
        }
    };

    static Image resizeImageToPowerOfTwo (Image image)
    {
        if (! (isPowerOfTwo (image.getWidth()) && isPowerOfTwo (image.getHeight())))
            return image.rescaled (jmin (1024, nextPowerOfTwo (image.getWidth())),
                                   jmin (1024, nextPowerOfTwo (image.getHeight())));

        return image;
    }

    class OpenGLDemo;

    //==============================================================================
    /**
        This component sits on top of the main GL demo, and contains all the sliders
        and widgets that control things.
    */
    class DemoControlsOverlay  : public Component,
                                 private CodeDocument::Listener,
                                 private ComboBox::Listener,
                                 private Slider::Listener,
                                 private Button::Listener,
                                 private Timer
    {
    public:
        DemoControlsOverlay (OpenGLDemo& d)
            : demo (d),
              vertexEditorComp (vertexDocument, nullptr),
              fragmentEditorComp (fragmentDocument, nullptr),
              tabbedComp (TabbedButtonBar::TabsAtLeft),
              showBackgroundToggle ("Draw 2D graphics in background")
        {
            addAndMakeVisible (statusLabel);
            statusLabel.setJustificationType (Justification::topLeft);
            statusLabel.setFont (Font (14.0f));

            addAndMakeVisible (sizeSlider);
            sizeSlider.setRange (0.0, 1.0, 0.001);
            sizeSlider.addListener (this);

            addAndMakeVisible (zoomLabel);
            zoomLabel.setText ("Zoom:", dontSendNotification);
            zoomLabel.attachToComponent (&sizeSlider, true);

            addAndMakeVisible (speedSlider);
            speedSlider.setRange (0.0, 0.5, 0.001);
            speedSlider.addListener (this);
            speedSlider.setSkewFactor (0.5f);

            addAndMakeVisible (speedLabel);
            speedLabel.setText ("Speed:", dontSendNotification);
            speedLabel.attachToComponent (&speedSlider, true);

            addAndMakeVisible (showBackgroundToggle);
            showBackgroundToggle.addListener (this);

            addAndMakeVisible (tabbedComp);
            tabbedComp.setTabBarDepth (25);
            tabbedComp.setColour (TabbedButtonBar::tabTextColourId, Colours::grey);
            tabbedComp.addTab ("Vertex", Colours::transparentBlack, &vertexEditorComp, false);
            tabbedComp.addTab ("Fragment", Colours::transparentBlack, &fragmentEditorComp, false);

            vertexDocument.addListener (this);
            fragmentDocument.addListener (this);

            textures.add (new BuiltInTexture ("Portmeirion", BinaryData::portmeirion_jpg, BinaryData::portmeirion_jpgSize));
            textures.add (new BuiltInTexture ("Tiled Background", BinaryData::tile_background_png, BinaryData::tile_background_pngSize));
            textures.add (new BuiltInTexture ("JUCE logo", BinaryData::juce_icon_png, BinaryData::juce_icon_pngSize));
            textures.add (new DynamicTexture());

            addAndMakeVisible (textureBox);
            textureBox.addListener (this);
            updateTexturesList();

            addAndMakeVisible (presetBox);
            presetBox.addListener (this);

            Array<ShaderPreset> presets (getPresets());
            StringArray presetNames;

            for (int i = 0; i < presets.size(); ++i)
                presetBox.addItem (presets[i].name, i + 1);

            addAndMakeVisible (presetLabel);
            presetLabel.setText ("Shader Preset:", dontSendNotification);
            presetLabel.attachToComponent (&presetBox, true);

            addAndMakeVisible (textureLabel);
            textureLabel.setText ("Texture:", dontSendNotification);
            textureLabel.attachToComponent (&textureBox, true);

            lookAndFeelChanged();
        }

        void initialise()
        {
            showBackgroundToggle.setToggleState (false, sendNotification);
            textureBox.setSelectedItemIndex (0);
            presetBox.setSelectedItemIndex (0);
            speedSlider.setValue (0.01);
            sizeSlider.setValue (0.5);
        }

        void resized() override
        {
            Rectangle<int> area (getLocalBounds().reduced (4));

            Rectangle<int> top (area.removeFromTop (75));

            Rectangle<int> sliders (top.removeFromRight (area.getWidth() / 2));
            showBackgroundToggle.setBounds (sliders.removeFromBottom (25));
            speedSlider.setBounds (sliders.removeFromBottom (25));
            sizeSlider.setBounds (sliders.removeFromBottom (25));

            top.removeFromRight (70);
            statusLabel.setBounds (top);

            Rectangle<int> shaderArea (area.removeFromBottom (area.getHeight() / 2));

            Rectangle<int> presets (shaderArea.removeFromTop (25));
            presets.removeFromLeft (100);
            presetBox.setBounds (presets.removeFromLeft (150));
            presets.removeFromLeft (100);
            textureBox.setBounds (presets);

            shaderArea.removeFromTop (4);
            tabbedComp.setBounds (shaderArea);
        }

        void mouseDown (const MouseEvent& e) override
        {
            demo.draggableOrientation.mouseDown (e.getPosition());
        }

        void mouseDrag (const MouseEvent& e) override
        {
            demo.draggableOrientation.mouseDrag (e.getPosition());
        }

        void mouseWheelMove (const MouseEvent&, const MouseWheelDetails& d) override
        {
            sizeSlider.setValue (sizeSlider.getValue() + d.deltaY);
        }

        void mouseMagnify (const MouseEvent&, float magnifyAmmount) override
        {
            sizeSlider.setValue (sizeSlider.getValue() + magnifyAmmount - 1.0f);
        }

        void selectPreset (int preset)
        {
            const ShaderPreset& p = getPresets()[preset];

            vertexDocument.replaceAllContent (p.vertexShader);
            fragmentDocument.replaceAllContent (p.fragmentShader);

            startTimer (1);
        }

        void selectTexture (int itemID)
        {
           #if JUCE_MODAL_LOOPS_PERMITTED
            if (itemID == 1000)
            {
                static File lastLocation = File::getSpecialLocation (File::userPicturesDirectory);

                FileChooser fc ("Choose an image to open...", lastLocation, "*.jpg;*.jpeg;*.png;*.gif");

                if (fc.browseForFileToOpen())
                {
                    lastLocation = fc.getResult();

                    textures.add (new TextureFromFile (fc.getResult()));
                    updateTexturesList();

                    textureBox.setSelectedId (textures.size());
                }
            }
            else
           #endif
            {
                if (DemoTexture* t = textures [itemID - 1])
                    demo.setTexture (t);
            }
        }

        void updateTexturesList()
        {
            textureBox.clear();

            for (int i = 0; i < textures.size(); ++i)
                textureBox.addItem (textures.getUnchecked(i)->name, i + 1);

           #if JUCE_MODAL_LOOPS_PERMITTED
            textureBox.addSeparator();
            textureBox.addItem ("Load from a file...", 1000);
           #endif
        }

        void updateShader()
        {
            startTimer (10);
        }

        Label statusLabel;

    private:
        void sliderValueChanged (Slider*) override
        {
            demo.scale = (float) sizeSlider.getValue();
            demo.rotationSpeed = (float) speedSlider.getValue();
        }

        void buttonClicked (Button*) override
        {
            demo.doBackgroundDrawing = showBackgroundToggle.getToggleState();
        }

        enum { shaderLinkDelay = 500 };

        void codeDocumentTextInserted (const String& /*newText*/, int /*insertIndex*/) override
        {
            startTimer (shaderLinkDelay);
        }

        void codeDocumentTextDeleted (int /*startIndex*/, int /*endIndex*/) override
        {
            startTimer (shaderLinkDelay);
        }

        void timerCallback() override
        {
            stopTimer();
            demo.setShaderProgram (vertexDocument.getAllContent(),
                                   fragmentDocument.getAllContent());
        }

        void comboBoxChanged (ComboBox* box) override
        {
            if (box == &presetBox)
                selectPreset (presetBox.getSelectedItemIndex());
            else if (box == &textureBox)
                selectTexture (textureBox.getSelectedId());
        }

        void lookAndFeelChanged() override
        {
            auto editorBackground = getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground,
                                                            Colours::white);

            for (int i = tabbedComp.getNumTabs(); i >= 0; --i)
                tabbedComp.setTabBackgroundColour (i, editorBackground);

            vertexEditorComp.setColour (CodeEditorComponent::backgroundColourId, editorBackground);
            fragmentEditorComp.setColour (CodeEditorComponent::backgroundColourId, editorBackground);
        }

        OpenGLDemo& demo;

        Label speedLabel, zoomLabel;

        CodeDocument vertexDocument, fragmentDocument;
        CodeEditorComponent vertexEditorComp, fragmentEditorComp;
        TabbedComponent tabbedComp;

        ComboBox presetBox, textureBox;
        Label presetLabel, textureLabel;

        Slider speedSlider, sizeSlider;
        ToggleButton showBackgroundToggle;

        OwnedArray<DemoTexture> textures;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DemoControlsOverlay)
    };

    //==============================================================================
    /** This is the main demo component - the GL context gets attached to it, and
        it implements the OpenGLRenderer callback so that it can do real GL work.
    */
    class OpenGLDemo  : public Component,
                        private OpenGLRenderer,
                        private AsyncUpdater
    {
    public:
        OpenGLDemo()
            : doBackgroundDrawing (false),
              scale (0.5f), rotationSpeed (0.0f), rotation (0.0f),
              textureToUse (nullptr), lastTexture (nullptr)
        {
            if (MainAppWindow* mw = MainAppWindow::getMainAppWindow())
                mw->setRenderingEngine (0);

            setOpaque (true);
            addAndMakeVisible (controlsOverlay = new DemoControlsOverlay (*this));

            openGLContext.setRenderer (this);
            openGLContext.attachTo (*this);
            openGLContext.setContinuousRepainting (true);

            controlsOverlay->initialise();
        }

        ~OpenGLDemo()
        {
            openGLContext.detach();
        }

        void newOpenGLContextCreated() override
        {
            // nothing to do in this case - we'll initialise our shaders + textures
            // on demand, during the render callback.
            freeAllContextObjects();

            if (controlsOverlay != nullptr)
                controlsOverlay->updateShader();
        }

        void openGLContextClosing() override
        {
            // When the context is about to close, you must use this callback to delete
            // any GPU resources while the context is still current.
            freeAllContextObjects();

            if (lastTexture != nullptr)
                setTexture (lastTexture);
        }

        void freeAllContextObjects()
        {
            shape = nullptr;
            shader = nullptr;
            attributes = nullptr;
            uniforms = nullptr;
            texture.release();
        }

        // This is a virtual method in OpenGLRenderer, and is called when it's time
        // to do your GL rendering.
        void renderOpenGL() override
        {
            jassert (OpenGLHelpers::isContextActive());

            const float desktopScale = (float) openGLContext.getRenderingScale();

            OpenGLHelpers::clear (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground,
                                                          Colours::lightblue));

            if (textureToUse != nullptr)
                if (! textureToUse->applyTo (texture))
                    textureToUse = nullptr;

            // First draw our background graphics to demonstrate the OpenGLGraphicsContext class
            if (doBackgroundDrawing)
                drawBackground2DStuff (desktopScale);

            updateShader();   // Check whether we need to compile a new shader

            if (shader == nullptr)
                return;

            // Having used the juce 2D renderer, it will have messed-up a whole load of GL state, so
            // we need to initialise some important settings before doing our normal GL 3D drawing..
            glEnable (GL_DEPTH_TEST);
            glDepthFunc (GL_LESS);
            glEnable (GL_BLEND);
            glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            openGLContext.extensions.glActiveTexture (GL_TEXTURE0);
            glEnable (GL_TEXTURE_2D);

            glViewport (0, 0, roundToInt (desktopScale * getWidth()), roundToInt (desktopScale * getHeight()));

            texture.bind();

            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            shader->use();

            if (uniforms->projectionMatrix != nullptr)
                uniforms->projectionMatrix->setMatrix4 (getProjectionMatrix().mat, 1, false);

            if (uniforms->viewMatrix != nullptr)
                uniforms->viewMatrix->setMatrix4 (getViewMatrix().mat, 1, false);

            if (uniforms->texture != nullptr)
                uniforms->texture->set ((GLint) 0);

            if (uniforms->lightPosition != nullptr)
                uniforms->lightPosition->set (-15.0f, 10.0f, 15.0f, 0.0f);

            if (uniforms->bouncingNumber != nullptr)
                uniforms->bouncingNumber->set (bouncingNumber.getValue());

            shape->draw (openGLContext, *attributes);

            // Reset the element buffers so child Components draw correctly
            openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
            openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);

            if (! controlsOverlay->isMouseButtonDown())
                rotation += (float) rotationSpeed;
        }

        Matrix3D<float> getProjectionMatrix() const
        {
            float w = 1.0f / (scale + 0.1f);
            float h = w * getLocalBounds().toFloat().getAspectRatio (false);
            return Matrix3D<float>::fromFrustum (-w, w, -h, h, 4.0f, 30.0f);
        }

        Matrix3D<float> getViewMatrix() const
        {
            Matrix3D<float> viewMatrix = draggableOrientation.getRotationMatrix()
                                            * Vector3D<float> (0.0f, 1.0f, -10.0f);

            Matrix3D<float> rotationMatrix = viewMatrix.rotated (Vector3D<float> (rotation, rotation, -0.3f));

            return rotationMatrix * viewMatrix;
        }

        void setTexture (DemoTexture* t)
        {
            lastTexture = textureToUse = t;
        }

        void setShaderProgram (const String& vertexShader, const String& fragmentShader)
        {
            newVertexShader = vertexShader;
            newFragmentShader = fragmentShader;
        }

        void paint (Graphics&) override {}

        void resized() override
        {
            controlsOverlay->setBounds (getLocalBounds());
            draggableOrientation.setViewport (getLocalBounds());
        }

        Draggable3DOrientation draggableOrientation;
        bool doBackgroundDrawing;
        float scale, rotationSpeed;
        BouncingNumber bouncingNumber;

    private:
        void handleAsyncUpdate() override
        {
            controlsOverlay->statusLabel.setText (statusText, dontSendNotification);
        }

        void drawBackground2DStuff (float desktopScale)
        {
            // Create an OpenGLGraphicsContext that will draw into this GL window..
            ScopedPointer<LowLevelGraphicsContext> glRenderer (createOpenGLGraphicsContext (openGLContext,
                                                                                            roundToInt (desktopScale * getWidth()),
                                                                                            roundToInt (desktopScale * getHeight())));

            if (glRenderer != nullptr)
            {
                Graphics g (*glRenderer);
                g.addTransform (AffineTransform::scale (desktopScale));

                for (int i = 0; i < numElementsInArray (stars); ++i)
                {
                    float size = 0.25f;

                    // This stuff just creates a spinning star shape and fills it..
                    Path p;
                    p.addStar (Point<float> (getWidth() * stars[i].x.getValue(),
                                             getHeight() * stars[i].y.getValue()), 7,
                               getHeight() * size * 0.5f,
                               getHeight() * size,
                               stars[i].angle.getValue());

                    float hue = stars[i].hue.getValue();

                    g.setGradientFill (ColourGradient (Colours::green.withRotatedHue (hue).withAlpha (0.8f),
                                                       0, 0,
                                                       Colours::red.withRotatedHue (hue).withAlpha (0.5f),
                                                       0, (float) getHeight(), false));
                    g.fillPath (p);
                }
            }
        }

        OpenGLContext openGLContext;

        ScopedPointer<DemoControlsOverlay> controlsOverlay;

        float rotation;

        ScopedPointer<OpenGLShaderProgram> shader;
        ScopedPointer<Shape> shape;
        ScopedPointer<Attributes> attributes;
        ScopedPointer<Uniforms> uniforms;

        OpenGLTexture texture;
        DemoTexture* textureToUse, *lastTexture;

        String newVertexShader, newFragmentShader, statusText;

        struct BackgroundStar
        {
            SlowerBouncingNumber x, y, hue, angle;
        };

        BackgroundStar stars[3];

        //==============================================================================
        void updateShader()
        {
            if (newVertexShader.isNotEmpty() || newFragmentShader.isNotEmpty())
            {
                ScopedPointer<OpenGLShaderProgram> newShader (new OpenGLShaderProgram (openGLContext));

                if (newShader->addVertexShader (OpenGLHelpers::translateVertexShaderToV3 (newVertexShader))
                      && newShader->addFragmentShader (OpenGLHelpers::translateFragmentShaderToV3 (newFragmentShader))
                      && newShader->link())
                {
                    shape = nullptr;
                    attributes = nullptr;
                    uniforms = nullptr;

                    shader = newShader;
                    shader->use();

                    shape      = new Shape (openGLContext);
                    attributes = new Attributes (openGLContext, *shader);
                    uniforms   = new Uniforms (openGLContext, *shader);

                    statusText = "GLSL: v" + String (OpenGLShaderProgram::getLanguageVersion(), 2);
                }
                else
                {
                    statusText = newShader->getLastError();
                }

                triggerAsyncUpdate();

                newVertexShader = String();
                newFragmentShader = String();
            }
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLDemo)
    };

    //==============================================================================
    struct ShaderPreset
    {
        const char* name;
        const char* vertexShader;
        const char* fragmentShader;
    };

    static Array<ShaderPreset> getPresets()
    {
        #define SHADER_DEMO_HEADER \
            "/*  This is a live OpenGL Shader demo.\n" \
            "    Edit the shader program below and it will be \n" \
            "    compiled and applied to the model above!\n" \
            "*/\n\n"

        ShaderPreset presets[] =
        {
            {
                "Texture + Lighting",

                SHADER_DEMO_HEADER
                "attribute vec4 position;\n"
                "attribute vec4 normal;\n"
                "attribute vec4 sourceColour;\n"
                "attribute vec2 textureCoordIn;\n"
                "\n"
                "uniform mat4 projectionMatrix;\n"
                "uniform mat4 viewMatrix;\n"
                "uniform vec4 lightPosition;\n"
                "\n"
                "varying vec4 destinationColour;\n"
                "varying vec2 textureCoordOut;\n"
                "varying float lightIntensity;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    destinationColour = sourceColour;\n"
                "    textureCoordOut = textureCoordIn;\n"
                "\n"
                "    vec4 light = viewMatrix * lightPosition;\n"
                "    lightIntensity = dot (light, normal);\n"
                "\n"
                "    gl_Position = projectionMatrix * viewMatrix * position;\n"
                "}\n",

                SHADER_DEMO_HEADER
               #if JUCE_OPENGL_ES
                "varying lowp vec4 destinationColour;\n"
                "varying lowp vec2 textureCoordOut;\n"
                "varying highp float lightIntensity;\n"
               #else
                "varying vec4 destinationColour;\n"
                "varying vec2 textureCoordOut;\n"
                "varying float lightIntensity;\n"
               #endif
                "\n"
                "uniform sampler2D demoTexture;\n"
                "\n"
                "void main()\n"
                "{\n"
               #if JUCE_OPENGL_ES
                "   highp float l = max (0.3, lightIntensity * 0.3);\n"
                "   highp vec4 colour = vec4 (l, l, l, 1.0);\n"
               #else
                "   float l = max (0.3, lightIntensity * 0.3);\n"
                "   vec4 colour = vec4 (l, l, l, 1.0);\n"
               #endif
                "    gl_FragColor = colour * texture2D (demoTexture, textureCoordOut);\n"
                "}\n"
            },

            {
                "Textured",

                SHADER_DEMO_HEADER
                "attribute vec4 position;\n"
                "attribute vec4 sourceColour;\n"
                "attribute vec2 textureCoordIn;\n"
                "\n"
                "uniform mat4 projectionMatrix;\n"
                "uniform mat4 viewMatrix;\n"
                "\n"
                "varying vec4 destinationColour;\n"
                "varying vec2 textureCoordOut;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    destinationColour = sourceColour;\n"
                "    textureCoordOut = textureCoordIn;\n"
                "    gl_Position = projectionMatrix * viewMatrix * position;\n"
                "}\n",

                SHADER_DEMO_HEADER
               #if JUCE_OPENGL_ES
                "varying lowp vec4 destinationColour;\n"
                "varying lowp vec2 textureCoordOut;\n"
               #else
                "varying vec4 destinationColour;\n"
                "varying vec2 textureCoordOut;\n"
               #endif
                "\n"
                "uniform sampler2D demoTexture;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    gl_FragColor = texture2D (demoTexture, textureCoordOut);\n"
                "}\n"
            },

            {
                "Flat Colour",

                SHADER_DEMO_HEADER
                "attribute vec4 position;\n"
                "attribute vec4 sourceColour;\n"
                "attribute vec2 textureCoordIn;\n"
                "\n"
                "uniform mat4 projectionMatrix;\n"
                "uniform mat4 viewMatrix;\n"
                "\n"
                "varying vec4 destinationColour;\n"
                "varying vec2 textureCoordOut;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    destinationColour = sourceColour;\n"
                "    textureCoordOut = textureCoordIn;\n"
                "    gl_Position = projectionMatrix * viewMatrix * position;\n"
                "}\n",

                SHADER_DEMO_HEADER
               #if JUCE_OPENGL_ES
                "varying lowp vec4 destinationColour;\n"
                "varying lowp vec2 textureCoordOut;\n"
               #else
                "varying vec4 destinationColour;\n"
                "varying vec2 textureCoordOut;\n"
               #endif
                "\n"
                "void main()\n"
                "{\n"
                "    gl_FragColor = destinationColour;\n"
                "}\n"
            },

            {
                "Rainbow",

                SHADER_DEMO_HEADER
                "attribute vec4 position;\n"
                "attribute vec4 sourceColour;\n"
                "attribute vec2 textureCoordIn;\n"
                "\n"
                "uniform mat4 projectionMatrix;\n"
                "uniform mat4 viewMatrix;\n"
                "\n"
                "varying vec4 destinationColour;\n"
                "varying vec2 textureCoordOut;\n"
                "\n"
                "varying float xPos;\n"
                "varying float yPos;\n"
                "varying float zPos;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    vec4 v = vec4 (position);\n"
                "    xPos = clamp (v.x, 0.0, 1.0);\n"
                "    yPos = clamp (v.y, 0.0, 1.0);\n"
                "    zPos = clamp (v.z, 0.0, 1.0);\n"
                "    gl_Position = projectionMatrix * viewMatrix * position;\n"
                "}",

                SHADER_DEMO_HEADER
               #if JUCE_OPENGL_ES
                "varying lowp vec4 destinationColour;\n"
                "varying lowp vec2 textureCoordOut;\n"
                "varying lowp float xPos;\n"
                "varying lowp float yPos;\n"
                "varying lowp float zPos;\n"
               #else
                "varying vec4 destinationColour;\n"
                "varying vec2 textureCoordOut;\n"
                "varying float xPos;\n"
                "varying float yPos;\n"
                "varying float zPos;\n"
               #endif
                "\n"
                "void main()\n"
                "{\n"
                "    gl_FragColor = vec4 (xPos, yPos, zPos, 1.0);\n"
                "}"
            },

            {
                "Changing Colour",

                SHADER_DEMO_HEADER
                "attribute vec4 position;\n"
                "attribute vec2 textureCoordIn;\n"
                "\n"
                "uniform mat4 projectionMatrix;\n"
                "uniform mat4 viewMatrix;\n"
                "\n"
                "varying vec2 textureCoordOut;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    textureCoordOut = textureCoordIn;\n"
                "    gl_Position = projectionMatrix * viewMatrix * position;\n"
                "}\n",

                SHADER_DEMO_HEADER
                "#define PI 3.1415926535897932384626433832795\n"
                "\n"
               #if JUCE_OPENGL_ES
                "precision mediump float;\n"
                "varying lowp vec2 textureCoordOut;\n"
               #else
                "varying vec2 textureCoordOut;\n"
               #endif
                "uniform float bouncingNumber;\n"
                "\n"
                "void main()\n"
                "{\n"
                "   float b = bouncingNumber;\n"
                "   float n = b * PI * 2.0;\n"
                "   float sn = (sin (n * textureCoordOut.x) * 0.5) + 0.5;\n"
                "   float cn = (sin (n * textureCoordOut.y) * 0.5) + 0.5;\n"
                "\n"
                "   vec4 col = vec4 (b, sn, cn, 1.0);\n"
                "   gl_FragColor = col;\n"
                "}\n"
            },

            {
                "Simple Light",

                SHADER_DEMO_HEADER
                "attribute vec4 position;\n"
                "attribute vec4 normal;\n"
                "\n"
                "uniform mat4 projectionMatrix;\n"
                "uniform mat4 viewMatrix;\n"
                "uniform vec4 lightPosition;\n"
                "\n"
                "varying float lightIntensity;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    vec4 light = viewMatrix * lightPosition;\n"
                "    lightIntensity = dot (light, normal);\n"
                "\n"
                "    gl_Position = projectionMatrix * viewMatrix * position;\n"
                "}\n",

                SHADER_DEMO_HEADER
               #if JUCE_OPENGL_ES
                "varying highp float lightIntensity;\n"
               #else
                "varying float lightIntensity;\n"
               #endif
                "\n"
                "void main()\n"
                "{\n"
               #if JUCE_OPENGL_ES
                "   highp float l = lightIntensity * 0.25;\n"
                "   highp vec4 colour = vec4 (l, l, l, 1.0);\n"
               #else
                "   float l = lightIntensity * 0.25;\n"
                "   vec4 colour = vec4 (l, l, l, 1.0);\n"
               #endif
                "\n"
                "    gl_FragColor = colour;\n"
                "}\n"
            },

            {
                "Flattened",

                SHADER_DEMO_HEADER
                "attribute vec4 position;\n"
                "attribute vec4 normal;\n"
                "\n"
                "uniform mat4 projectionMatrix;\n"
                "uniform mat4 viewMatrix;\n"
                "uniform vec4 lightPosition;\n"
                "\n"
                "varying float lightIntensity;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    vec4 light = viewMatrix * lightPosition;\n"
                "    lightIntensity = dot (light, normal);\n"
                "\n"
                "    vec4 v = vec4 (position);\n"
                "    v.z = v.z * 0.1;\n"
                "\n"
                "    gl_Position = projectionMatrix * viewMatrix * v;\n"
                "}\n",

                SHADER_DEMO_HEADER
               #if JUCE_OPENGL_ES
                "varying highp float lightIntensity;\n"
               #else
                "varying float lightIntensity;\n"
               #endif
                "\n"
                "void main()\n"
                "{\n"
               #if JUCE_OPENGL_ES
                "   highp float l = lightIntensity * 0.25;\n"
                "   highp vec4 colour = vec4 (l, l, l, 1.0);\n"
               #else
                "   float l = lightIntensity * 0.25;\n"
                "   vec4 colour = vec4 (l, l, l, 1.0);\n"
               #endif
                "\n"
                "    gl_FragColor = colour;\n"
                "}\n"
            },

            {
                "Toon Shader",

                SHADER_DEMO_HEADER
                "attribute vec4 position;\n"
                "attribute vec4 normal;\n"
                "\n"
                "uniform mat4 projectionMatrix;\n"
                "uniform mat4 viewMatrix;\n"
                "uniform vec4 lightPosition;\n"
                "\n"
                "varying float lightIntensity;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    vec4 light = viewMatrix * lightPosition;\n"
                "    lightIntensity = dot (light, normal);\n"
                "\n"
                "    gl_Position = projectionMatrix * viewMatrix * position;\n"
                "}\n",

                SHADER_DEMO_HEADER
               #if JUCE_OPENGL_ES
                "varying highp float lightIntensity;\n"
               #else
                "varying float lightIntensity;\n"
               #endif
                "\n"
                "void main()\n"
                "{\n"
               #if JUCE_OPENGL_ES
                "    highp float intensity = lightIntensity * 0.5;\n"
                "    highp vec4 colour;\n"
               #else
                "    float intensity = lightIntensity * 0.5;\n"
                "    vec4 colour;\n"
               #endif
                "\n"
                "    if (intensity > 0.95)\n"
                "        colour = vec4 (1.0, 0.5, 0.5, 1.0);\n"
                "    else if (intensity > 0.5)\n"
                "        colour  = vec4 (0.6, 0.3, 0.3, 1.0);\n"
                "    else if (intensity > 0.25)\n"
                "        colour  = vec4 (0.4, 0.2, 0.2, 1.0);\n"
                "    else\n"
                "        colour  = vec4 (0.2, 0.1, 0.1, 1.0);\n"
                "\n"
                "    gl_FragColor = colour;\n"
                "}\n"
            }
        };

        return Array<ShaderPreset> (presets, numElementsInArray (presets));
    }
};

// This static object will register this demo type in a global list of demos..
static JuceDemoType<OpenGLDemoClasses::OpenGLDemo> demo ("20 Graphics: OpenGL");

#endif
