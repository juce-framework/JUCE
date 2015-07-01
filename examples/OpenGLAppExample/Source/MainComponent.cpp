/*
  ==============================================================================

    This file was auto-generated!

  ==============================================================================
*/

#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "Resources/WavefrontObjParser.h"



//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainContentComponent   : public OpenGLAppComponent
{
public:
    //==============================================================================
    MainContentComponent()
    {
        setSize (800, 600);
    }

    ~MainContentComponent()
    {
        shutdownOpenGL();
    }

    void initialise() override
    {
        createShaders();
    }

    void shutdown() override
    {
        shader = nullptr;
        shape = nullptr;
        attributes = nullptr;
        uniforms = nullptr;
    }

    Matrix3D<float> getProjectionMatrix() const
    {
        float w = 1.0f / (0.5f + 0.1f);
        float h = w * getLocalBounds().toFloat().getAspectRatio (false);
        return Matrix3D<float>::fromFrustum (-w, w, -h, h, 4.0f, 30.0f);
    }

    Matrix3D<float> getViewMatrix() const
    {
        Matrix3D<float> viewMatrix (Vector3D<float> (0.0f, 0.0f, -10.0f));
        Matrix3D<float> rotationMatrix
            = viewMatrix.rotated (Vector3D<float> (-0.3f, 5.0f * std::sin (getFrameCounter() * 0.01f), 0.0f));

        return rotationMatrix * viewMatrix;
    }

    void render() override
    {

        jassert (OpenGLHelpers::isContextActive());

        const float desktopScale = (float) openGLContext.getRenderingScale();
        OpenGLHelpers::clear (Colour::greyLevel (0.1f));

        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glViewport (0, 0, roundToInt (desktopScale * getWidth()), roundToInt (desktopScale * getHeight()));

        shader->use();

        if (uniforms->projectionMatrix != nullptr)
            uniforms->projectionMatrix->setMatrix4 (getProjectionMatrix().mat, 1, false);

        if (uniforms->viewMatrix != nullptr)
            uniforms->viewMatrix->setMatrix4 (getViewMatrix().mat, 1, false);

        shape->draw (openGLContext, *attributes);

        // Reset the element buffers so child Components draw correctly
        openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
        openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);

    }

    void paint (Graphics& g) override
    {
        // You can add your component specific drawing code here!
        // This will draw over the top of the openGL background.

        g.setColour(Colours::white);
        g.setFont (20);
        g.drawText ("OpenGL Example", 25, 20, 300, 30, Justification::left);
        g.drawLine (20, 20, 170, 20);
        g.drawLine (20, 50, 170, 50);
    }

    void resized() override
    {
        // This is called when the MainContentComponent is resized.
        // If you add any child components, this is where you should
        // update their positions.

    }

    void createShaders()
    {
        vertexShader =
            "attribute vec4 position;\n"
            "attribute vec4 sourceColour;\n"
            "attribute vec2 texureCoordIn;\n"
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
            "    textureCoordOut = texureCoordIn;\n"
            "    gl_Position = projectionMatrix * viewMatrix * position;\n"
            "}\n";

        fragmentShader =
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
            "    vec4 colour = vec4(0.95, 0.57, 0.03, 0.7);\n"
            "    gl_FragColor = colour;\n"
            "}\n";

        ScopedPointer<OpenGLShaderProgram> newShader (new OpenGLShaderProgram (openGLContext));
        String statusText;

        if (newShader->addVertexShader (OpenGLHelpers::translateVertexShaderToV3 (vertexShader))
              && newShader->addFragmentShader (OpenGLHelpers::translateFragmentShaderToV3 (fragmentShader))
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
    }


private:
    //==============================================================================
    struct Vertex
    {
        float position[3];
        float normal[3];
        float colour[4];
        float texCoord[2];
    };

    //==============================================================================
    // This class just manages the attributes that the shaders use.
    struct Attributes
    {
        Attributes (OpenGLContext& openGLContext, OpenGLShaderProgram& shaderProgram)
        {
            position      = createAttribute (openGLContext, shaderProgram, "position");
            normal        = createAttribute (openGLContext, shaderProgram, "normal");
            sourceColour  = createAttribute (openGLContext, shaderProgram, "sourceColour");
            texureCoordIn = createAttribute (openGLContext, shaderProgram, "texureCoordIn");
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

            if (texureCoordIn != nullptr)
            {
                openGLContext.extensions.glVertexAttribPointer (texureCoordIn->attributeID, 2, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 10));
                openGLContext.extensions.glEnableVertexAttribArray (texureCoordIn->attributeID);
            }
        }

        void disable (OpenGLContext& openGLContext)
        {
            if (position != nullptr)       openGLContext.extensions.glDisableVertexAttribArray (position->attributeID);
            if (normal != nullptr)         openGLContext.extensions.glDisableVertexAttribArray (normal->attributeID);
            if (sourceColour != nullptr)   openGLContext.extensions.glDisableVertexAttribArray (sourceColour->attributeID);
            if (texureCoordIn != nullptr)  openGLContext.extensions.glDisableVertexAttribArray (texureCoordIn->attributeID);
        }

        ScopedPointer<OpenGLShaderProgram::Attribute> position, normal, sourceColour, texureCoordIn;

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
        Uniforms (OpenGLContext& openGLContext, OpenGLShaderProgram& shaderProgram)
        {
            projectionMatrix = createUniform (openGLContext, shaderProgram, "projectionMatrix");
            viewMatrix       = createUniform (openGLContext, shaderProgram, "viewMatrix");
        }

        ScopedPointer<OpenGLShaderProgram::Uniform> projectionMatrix, viewMatrix;

    private:
        static OpenGLShaderProgram::Uniform* createUniform (OpenGLContext& openGLContext,
                                                            OpenGLShaderProgram& shaderProgram,
                                                            const char* uniformName)
        {
            if (openGLContext.extensions.glGetUniformLocation (shaderProgram.getProgramID(), uniformName) < 0)
                return nullptr;

            return new OpenGLShaderProgram::Uniform (shaderProgram, uniformName);
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

        void draw (OpenGLContext& openGLContext, Attributes& glAttributes)
        {
            for (int i = 0; i < vertexBuffers.size(); ++i)
            {
                VertexBuffer& vertexBuffer = *vertexBuffers.getUnchecked (i);
                vertexBuffer.bind();

                glAttributes.enable (openGLContext);
                glDrawElements (GL_TRIANGLES, vertexBuffer.numIndices, GL_UNSIGNED_INT, 0);
                glAttributes.disable (openGLContext);
            }
        }

    private:
        struct VertexBuffer
        {
            VertexBuffer (OpenGLContext& context, WavefrontObjFile::Shape& aShape) : openGLContext (context)
            {
                numIndices = aShape.mesh.indices.size();

                openGLContext.extensions.glGenBuffers (1, &vertexBuffer);
                openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);

                Array<Vertex> vertices;
                createVertexListFromMesh (aShape.mesh, vertices, Colours::green);

                openGLContext.extensions.glBufferData (GL_ARRAY_BUFFER,
                                                       static_cast<GLsizeiptr> (static_cast<size_t> (vertices.size()) * sizeof (Vertex)),
                                                       vertices.getRawDataPointer(), GL_STATIC_DRAW);

                openGLContext.extensions.glGenBuffers (1, &indexBuffer);
                openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
                openGLContext.extensions.glBufferData (GL_ELEMENT_ARRAY_BUFFER,
                                                       static_cast<GLsizeiptr> (static_cast<size_t> (numIndices) * sizeof (juce::uint32)),
                                                       aShape.mesh.indices.getRawDataPointer(), GL_STATIC_DRAW);
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

    const char* vertexShader;
    const char* fragmentShader;

    ScopedPointer<OpenGLShaderProgram> shader;
    ScopedPointer<Shape> shape;
    ScopedPointer<Attributes> attributes;
    ScopedPointer<Uniforms> uniforms;

    String newVertexShader, newFragmentShader;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()    { return new MainContentComponent(); }


#endif  // MAINCOMPONENT_H_INCLUDED
