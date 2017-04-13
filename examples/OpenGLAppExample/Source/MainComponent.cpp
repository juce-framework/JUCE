#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "Resources/WavefrontObjParser.h"

/*
 This demo shows how to display a 3-D object animation using OpenGLAppComponent class.
 It also shows how that we can still use regular core graphics 2-D objects on top
 of the OpenGL object and different means to colorize it.
 It is largely based on Jules' original OpenGlAppExample demo.
*/

//==============================================================================
/*
 OPENGL is a library of functions and objects which have direct control over
 the GPU (Graphics Processor Unit) of your PC, tablet, phone, etc.
 Directly controlling the GPU increases performance and allows to remove
 some process consuming tasks off of the main CPU while giving a great
 set of 3D-drawing and animation features on the computer screen.
 
 Terminology used in OpenGL and in particular for Juce's implementation
 of OpenGL support includes:
 - Vertex (plur. vertices):
 In order to draw and/or animate a 3-D object, points have to be defined in the
 3D world to define a "skeleton" representing the object to be drawn/animated.
 The skeleton is defined using polygons (generally triangles or quadrilaterals)
 whose angular points are called vertices. For example a cube defined using
 triangles can be drawn using 6 faces x 2 triangles x 3 vertices = 36 vertices
 (in fact some of the vertices are shared by different faces and in some instances
 can therefore be defined as one vertex).
 - Fragment:
 A fragment is a surface linking 2 or more vertices. It is the "skin" covering the
 "skeleton" representing a 3-D object.
 - Shaders:
 In addition to the OpenGL library, a specific language (GLSL) has been developped
 to write small routines which will be executed directly by the GPU.
 These routines are called "shaders". The 2 main shaders used by most GPU's
 are called Vertex Shader and Fragment Shader. The Vertex Shader defines the
 actions executed by the GPU to define vertex attributes (e.g. each vertex
 coordinates, color, etc.). 
 The Fragment Shader, which is executed after the Vertex Shader, defines the
 attributes of all the pixels other than those corresponding to the vertices.
 In other words, it will render (paint) the texture interpolated between vertices.
 Since the GPU essentially is a state machine (upon each new state
 a new frame is drawn on the screen), the Vertex Shader defines the attributes
 of each vertex to be drawn upon each state, and passes the information to the
 Fragment shader which may use it to render the texture upon each frame.
 Note that each shader includes not only instructions but also data that can
 be shared between the main CPU code and between the shaders themselves.
 So basically upon each state (frame) the CPU will send data (and eventually
 new instructions) to the GPU by using either OpenGL instructions or by sending
 new data to be used by the GPU shaders.
 IMPORTANT: Both instructions and data are defined in source form, and therefore
 need to be compiled and linked ,like any computer program, to be executed by
 the GPU. But the source code is compiled by the OpenGL library itself (by
 calling some specific methods) AT RUN TIME.
 
 A few words about matrices and vectors used in the 3-D world of computer graphics:
 The matrices used here (Matrix3D objects) actually are 4x4 matrices
 to allow for vector translation. This is used to take into account the
 perspective factor (scaling based on the distance from the camera).
 Effectively, every 3-D vector uses a 4th coordinate (making it a "homogeneous"
 vector) to be able to define the distance and therefore the perspective.
 It also makes sense since 4x4 matrices can only multiply 4x1 vectors.
 The 4th coordinate (called w) is usually set to 1 before any transformation
 is applied.
 
 More terms are defined in the code below as needed.

*/

// This will be used if the object color is set by the CPU code
// (textureType == COLOR_SET_BY_CPU). It's a greenish semi-transparent color.
// We use the same color for both the button and the 3-D object (teapot).
#define COLOR_BY_CPU Colour(0xC080FF80)

// This will be used if the object color is set by the GPU code (shader)
// (textureType == COLOR_SET_BY_GPU). It's a gold semi-transparent color.
// We use the same color for both the button and the 3-D object (teapot).
// As the 3D-object is painted by the GPU, which needs separate, color
// parameters, we need a list of define directives to address both CPU
// and GPU codes.
#define COLOR_BY_GPU_RED 0.95
#define COLOR_BY_GPU_GREEN 0.57
#define COLOR_BY_GPU_BLUE 0.03
#define COLOR_BY_GPU_ALPHA 0.7
// Define this color for the CPU code
#define COLOR_BY_GPU_USED_BY_CPU Colour((uint8)(COLOR_BY_GPU_RED*255),(uint8)(COLOR_BY_GPU_GREEN*255),(uint8)(COLOR_BY_GPU_BLUE*255),(float)COLOR_BY_GPU_ALPHA)
// Define this color for the GPU code (shader) string. See fragment shader below.
// (This is pretty ugly define code, but it works... If you have a better solution, please use it!)
#define STRINGER(x) #x
#define TO_STRING(x) STRINGER(x)
#define COLOR_BY_GPU_STRING TO_STRING(COLOR_BY_GPU_RED) "," TO_STRING(COLOR_BY_GPU_GREEN) "," TO_STRING(COLOR_BY_GPU_BLUE) "," TO_STRING(COLOR_BY_GPU_ALPHA)

// This will be used if a 2-D texture file is used to colorize the object
// (textureType == TEXTURE)
// The texture file has been included in the source using Juce's handy BinaryData class which
// automatically creates source code to represent memory blocks filled with binary data
// (here, a jpg file) by simply declaring them in the IntroJucer project
// (and storing the source file in the resource folder, for instance).
#define TEXTURE_DATA BinaryData::rainbow_gradient_vertical_jpg,BinaryData::rainbow_gradient_vertical_jpgSize


//==================================================================================
/**
 This is a handy class which allows to completely customize a button, for instance
 using a graphics file as background, and text on it. Note that this is part of
 JUCE core graphics framework, NOT OpenGL...
*/
class ImageTextButton: public Button
{
public:
    ImageTextButton () : Button("ITBUTTON"), buttonFont(Font(14.0f)), fontColor(Colours::black), buttonLabel(""), width(0), height(0), xPos(0), yPos(0)
    {
    }
    ImageTextButton (String title, int x, int y, int w, int h, Image& image) : Button("ITBUTTON"), buttonFont(Font(14.0f)), fontColor(Colours::black), buttonLabel(title), backgroundImage(image), width(w), height(h), xPos(x), yPos(y)
    {
    }
    //==============================================================================
    // This is automatically called when the button needs be repainted.
    void paintButton(Graphics &g, bool isMouseOver, bool isButtonDown) override
    {
        // A path defines a series of graphics elements to draw something.
        // Here, its shape (a rounded rectangle) will be used as a mask
        // to get round angles around the image used as background.
        Path path;
        path.addRoundedRectangle(1.0f, 1.0f, (float)width-2.0f, (float)height-2.0f, (float)3.0f);
        
        if (buttonLabel == "") return;  // Don't draw anything if no label
        
        if (isMouseOver) {
            g.setOpacity(0.8f);
            if (isButtonDown) {
                g.setOpacity(0.5f);
            }
        } else {
            g.setOpacity(1.0f);
        }
        // If called with no specific dimensions, it will use the graphics image dimensions
        if ((height == 0) || (width == 0)) {
            g.drawImageAt (backgroundImage,0, 0);
            width = backgroundImage.getWidth();
            height = backgroundImage.getHeight();
        } else {
            // Otherwise it will use the given dimensions with rounded angles
            g.reduceClipRegion(path);
            g.drawImage(backgroundImage, 0, 0, width, height, 0, 0, backgroundImage.getWidth(), backgroundImage.getHeight());
            g.setColour(Colour(0x80FFFFFF));
            g.drawRect(0, 0, width, height,2);
        }
        // Now that the button background has been drawn, add text on top of it
        // We write it twice with a shift to make it display nicely on any background
        g.setColour(fontColor);
        g.setFont(buttonFont);
        g.drawText(buttonLabel,0,0,width,height,Justification::horizontallyCentred);
        g.setColour(fontColor.contrasting());
        g.drawText(buttonLabel,1,1,width,height,Justification::horizontallyCentred);
        
    }
    // We define those as public to simplify things in this small project
    Font buttonFont;
    Colour fontColor;
    String buttonLabel;
    Image backgroundImage;
    int width,height,xPos,yPos;
};

//==================================================================================
/**
 Now entering OpenGL world !
 An OpenGLAppComponent is a Juce Component (ie a graphic entity) as well as an
 OpenGLRenderer (ie an object used in an OpenGL context as a callback) that
 will send instructions to the GPU to update the displayed information.
 Also a frameCounter has been added as a courtesy by Jules to determine how
 many frames have been used so far by the GPU to animate the graphics.
 Note that this main object also is a Button::Listener to receive callbacks
 when defined buttons are "pressed"
 */

class MainContentComponent   : public OpenGLAppComponent, public Button::Listener
{
public:
    //==============================================================================
    MainContentComponent()
    {
        setSize (800, 600);
    }

    ~MainContentComponent()
    {
        shutdownOpenGL();   // Release OpenGL resources
    }

    //==============================================================================
    // This method does NOT use the OpenGL engine but shows that we can draw regular
    // Juce Components on top of OpenGL-generated graphics
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
    
    //==============================================================================
    void resized() override
    {
        // Core (non-OpenGL) graphics:
        // This is called when the MainContentComponent is resized.
        // If you add any child components, this is where you should
        // update their positions.
        cpuColorButton.setBounds((800/2)-200, 20, 183, 32); // TextButton
        gpuColorButton.setBounds((800/2), 20, 183, 32);     // TextButton
        textureButton.setBounds(textureButton.xPos, textureButton.yPos, textureButton.width, textureButton.height); // ImageTextButton: use button-defined boundaries
    }
    
    //==============================================================================
    // Called once when the OpenGL context is created
    void initialise() override
    {
        // In this example the shader source codes are downloaded from a memory
        // structure, but they could be downloaded from a text file or any other
        // string structure
        createShaders();
        // In one case (colorizer == TEXTURE), we will use a 2-D texture (graphic)
        // file or memory block to render (paint) the main object:
        // Get memory data block created by IntroJucer in BinaryDaya.cpp file
        Image textureImage = ImageCache::getFromMemory (TEXTURE_DATA);
        // Image must have height and width equal to a power of 2 pixels to be more efficient
        // when used with older GPU architectures
        if (! (isPowerOfTwo (textureImage.getWidth()) && isPowerOfTwo (textureImage.getHeight())))
            textureImage = textureImage.rescaled (jmin (1024, nextPowerOfTwo (textureImage.getWidth())),
                                   jmin (1024, nextPowerOfTwo (textureImage.getHeight())));
        // Use that image as a 2-D texture for the object that will be painted
        texture.loadImage(textureImage);
        
        // Now add the non-OpenGL object (option buttons)
        // To draw non-OpenGL objects in this OpenGL-specific thread,
        // we need to use a locked access to the MessageManager thread
        const MessageManagerLock lock (Thread::getCurrentThread());
        if (lock.lockWasGained()) {
            
            cpuColorButton.setColour(TextButton::buttonColourId, COLOR_BY_CPU);
            cpuColorButton.addListener(this);   // To call buttonClicked when pressed
            addAndMakeVisible(cpuColorButton);  // Draw this as part of the main object
            
            gpuColorButton.setColour(TextButton::buttonColourId,COLOR_BY_GPU_USED_BY_CPU);
            gpuColorButton.addListener(this);   // To call buttonClicked when pressed
            addAndMakeVisible(gpuColorButton);  // Draw this as part of the main object
            
            textureButton.addListener(this);   // To call buttonClicked when pressed
            addAndMakeVisible(textureButton);  // Draw this as part of the main object
        }
    }
    //==============================================================================
    // This will be called every time a button (attached to a listener) is "pressed"
    void buttonClicked(Button* clickedButton) override
    {
        // The buttons are used to select the colorizer state, which is used by the
        // CPU and the GPU to determine the 3D-object's color.
        if (clickedButton == &textureButton) {
            colorizer = TEXTURE;
        } else if (clickedButton == &cpuColorButton) {
            colorizer = COLOR_SET_BY_CPU;
        } else if (clickedButton == &gpuColorButton) {
            colorizer = COLOR_SET_BY_GPU;
        }
    }

    //==============================================================================
    void shutdown() override
    {
        usedShaderProgram = nullptr;
        shape = nullptr;
        attributes = nullptr;
        uniforms = nullptr;
        texture.release();
    }

    //==============================================================================
    // This matrix will be constant throughout the demo as it will be used by the GPU
    // to modify the coordinates of each vertex based on the "frustum" defining the
    // view from the virtual camera that "sees" the 3-D view. The frustum is just
    // a truncated horizontal pyramid whose base represents the far side of the view
    // (background) and the top is the near side of the view. Only the objects inside
    // this truncated pyramid will be displayed by the GPU.
    // (Use https://en.wikipedia.org/wiki/Viewing_frustum for a graphic explanation).
    // The multiplication by this matrix will be the final step used by the GPU
    // (in the Vertex shader) to create the object positions in the final 3-D view.
    
    Matrix3D<float> getProjectionMatrix() const
    {
        float w = 1.0f / (0.5f + 0.1f);
        float h = w * getLocalBounds().toFloat().getAspectRatio (false);
        // All objects coordinates in the pre-final view will be modified by multiplying
        // their position vectors by this matrix (see getViewMatrix() below).
        // The first 4 parameters define the view size, the 5th one is a factor that
        // defines from how far the near side will be seen.
        // The 6th one is a factor that defines from how far the far side will be seen.
        // Obiviously this 6th param should be greater than the 5th one.
        // All the values used in this method are somewhat empirical and depend on
        // what you want to show and how close (big) the displayed objects will be.
        return Matrix3D<float>::fromFrustum (-w, w, -h, h, 4.0f, 100.0f);
    }

    //==============================================================================
    // This creates a rotation matrix that can be used to rotate an object.
    // A 3-dimension vector is given as an argmument to define the rotations around
    // each axis (x,y,z), using the well-known Euler formulas.
    // As always in our graphics world, a 4th dimension is added to get a homogeneous
    // matrix.
    
    Matrix3D<float> createRotationMatrix (Vector3D<float> eulerAngleRadians) const noexcept
    {
        const float cx = std::cos (eulerAngleRadians.x),  sx = std::sin (eulerAngleRadians.x),
        cy = std::cos (eulerAngleRadians.y),  sy = std::sin (eulerAngleRadians.y),
        cz = std::cos (eulerAngleRadians.z),  sz = std::sin (eulerAngleRadians.z);
        
        return Matrix3D<float> ((cy * cz) + (sx * sy * sz), cx * sz, (cy * sx * sz) - (cz * sy), 0.0f,
                         (cz * sx * sy) - (cy * sz), cx * cz, (cy * cz * sx) + (sy * sz), 0.0f,
                         cx * sy, -sx, cx * cy, 0.0f,
                         0.0f, 0.0f, 0.0f, 1.0f);
    }
    //==============================================================================
    // This matrix will be modified upon each frame in the demo as it will be used
    // to modify the coordinates of each pixel based on the animation. It actually is
    // a combination (product) of 2 matrices that will determine the "pre-final" view
    // before the Projection matrix is applied (see getProjectMatrix() above).
   
    Matrix3D<float> getViewMatrix() const
    {
        // The viewMatrix will be used to modify the vertex coordinates in order
        // to transform object coordinates as viewed-by-camera (or eye) coordinates.
        // Standard x,y,z values are used. Obviously the z value used here will have
        // to be in the range near side < z < far side as defined by the frustum used
        // in the projection matrix (see getProjectionMatrix() above).
        Matrix3D<float> viewMatrix (Vector3D<float> (0.0f, 0.0f, -50.0f));
        
        // The rotation matrix will be applied on each frame.
        // The vector passed here contains the Euler angle values for each axis
        // The empiric values used as params will create a slight but constant tilting
        // on the x-axis, a periodic rotation on the y-axis and no rotation on the
        // z-axis.
        Matrix3D<float> rotationMatrix
            = createRotationMatrix (Vector3D<float> (-0.3f, 5.0f * std::sin (getFrameCounter() * 0.01f), 0.0f));

        return rotationMatrix * viewMatrix;
    }

    //==============================================================================
    // By default, the OpenGLAppComponent will call this method every time the GPU
    // is ready to paint (render) another frame. If you need a timely, precise frame
    // rate you may use OpenGLContext::setContinuousRepainting(false) and then
    // OpenGLContext::triggerRepaint() based on timer callbacks.
    // See OpenGLRenderer class definition for more details.
    
    void render() override
    {
        // Make sure an OpenGL graphics context has been defined
        jassert (OpenGLHelpers::isContextActive());

        // This allows to calculate correct pixel number by using
        // physical vs logical pixel number
        const float desktopScale = (float) openGLContext.getRenderingScale();
        
        // You need to clear the display upon every new frame unless you're dead sure
        // that nothing has changed, which you are not...
        OpenGLHelpers::clear (Colour::greyLevel (0.1f));

        // Having used the juce 2D renderer, it will have messed-up a whole load of GL state, so
        // we need to initialise some important settings before doing our normal GL 3D drawing..
        if (colorizer == TEXTURE) {
            // OpenGL methods to avoid displaying pixels behind front pixels
            glEnable (GL_DEPTH_TEST);   // Enable the test
            glDepthFunc (GL_LESS);      // Do not display back pixels
            // Using a texture to paint main OpenGL object (teapot)
            openGLContext.extensions.glActiveTexture (GL_TEXTURE0); // Using texture #0
            glEnable (GL_TEXTURE_2D);   // It's a 2-D image texture
            // Tell the GPU to use that texture
            texture.bind();
            // OpenGL method to specify how the image is horizontally tiled
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            // OpenGL method to specify how the image is vertically tiled
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }
        // OpenGL method to blend the computed fragment color values with the values in the color buffers
        glEnable (GL_BLEND);
        // OpenGL method to specify how the red, green, blue, and alpha blending factors are computed.
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        // OpenGL method to specify viewport's x, y, width and height
        glViewport (0, 0, roundToInt (desktopScale * getWidth()), roundToInt (desktopScale * getHeight()));
        // This will call an OpenGL method to tell the GPU to use this program
        usedShaderProgram->use();
        
        // Set the demoTexture uniform (global) variable in the shaders to 0
        // (actually a "Sampler2D" value equivalent to 0)
        if (uniforms->demoTexture != nullptr)
            uniforms->demoTexture->set ((GLint) 0);

        // Set the textureType uniform (global) variable in the shaders to a value
        // which reflects the type of coloring we want to use
        // Note that there is no int type for uniform variables so we use float.
        if (uniforms->textureType != nullptr) {
            switch (colorizer) {
                case COLOR_SET_BY_CPU:
                    uniforms->textureType->set ((GLfloat) 0.0);
                    break;
                case COLOR_SET_BY_GPU:
                    uniforms->textureType->set ((GLfloat) 1.0);
                    break;
                case TEXTURE:
                    uniforms->textureType->set ((GLfloat) 2.0);
                    break;
            }
        }

        // Modify the uniform (global) variable projectionMatrix that will be used by the GPU when executing the shaders
        if (uniforms->projectionMatrix != nullptr)
            // Update the projection matrix with the values given, 1 matrix, do not transpose
            uniforms->projectionMatrix->setMatrix4 (getProjectionMatrix().mat, 1, false);

        // Modify the uniform (global) variable viewMatrix that will be used by the GPU when executing the shaders
        if (uniforms->viewMatrix != nullptr)
            // Update the view matrix with the values given, 1 matrix, do not transpose
            uniforms->viewMatrix->setMatrix4 (getViewMatrix().mat, 1, false);

        // This will fill a vertex buffer that is sent to the GPU to be used along with the attributes
        // by the GPU in the shaders. See Shape struct below. This buffer is called a Vertex Buffer Object
        // (VBO) and allows to send a bunch of variables at once to the GPU for better efficiency
        shape->draw (openGLContext, *attributes);

        // Reset the element buffers so child Components draw correctly
        openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
        openGLContext.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    //==============================================================================
    // This routine will not only load the shaders, but also the
    // object data to be drawn (See Shape for more info). It will also call the
    // OpenGL routines to compile and link the shader codes.
    // This shader source codes could obviously alternatively be loaded from a
    // file, a memory block or a CodeDocument like in the Juce Demo

    // Specific comments for the shader codes:
    // - the attribute modifiers match the attribute definitions (see Attributes struct)
    // and are used to pass variables (scalars, vectors, matrices, etc) to the Vertex Shader;
    // - the uniform modifiers match the uniform definitions (see Uniforms struct below)
    // and are used to share variables (scalars, vectors, matrices, etc) between the CPU code,
    // the Vertex Shader and the Fragment Shader;
    // - the varying modifiers allow to pass variables from the Vertex Shader to the Fragment
    // Shader;
    // - glPosition is an implicitly defined variable which is calculated using the Vertex Shader
    // and is a vector containing the actual final position of the current vertex being drawn.
    // (Note that only one vertex at a time is being handled by the Vertex Shader code)
    // - glFragColor is an implicitly defined variable which is used in tge Fragment Shader to
    // define the pixel colors that will be interpolated between vertices if not explicitly defined
    // - textureType is used to define the type of coloring we use. Note that we use < comparisons
    // and not == as textureType is a float value (and it is always dangerous to compare floats due
    // to precision syndroms...)
    
    void createShaders()
    {
        vertexShader =
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
            "}\n";

        fragmentShader =
#if JUCE_OPENGL_ES
            "precision lowp float;\n"
            "varying lowp vec4 destinationColour;\n"
            "varying lowp vec2 textureCoordOut;\n"
#else
            "varying vec4 destinationColour;\n"
            "varying vec2 textureCoordOut;\n"
#endif
            "\n"
            "uniform sampler2D demoTexture;\n"
            "uniform float textureType;\n"
            "\n"
            "void main()\n"
            "{\n"
            "    if (textureType < 0.1)\n"
            "        gl_FragColor = destinationColour;\n"
            "    else if (textureType < 1.1)\n"
            "        gl_FragColor = vec4(" COLOR_BY_GPU_STRING ");\n"
            "    else if (textureType < 2.1)\n"
            "     gl_FragColor = texture2D (demoTexture, textureCoordOut);\n"
            "}\n";

        // An OpenGLShaderProgram is a combination of shaders which are compiled and linked together
        ScopedPointer<OpenGLShaderProgram> newShaderProgram (new OpenGLShaderProgram (openGLContext));

        String statusText;  // Can be used to give a success or failure message after GLSL compilation

        // Create the program that combines Vertex and Fragment shaders
        if (newShaderProgram->addVertexShader (OpenGLHelpers::translateVertexShaderToV3 (vertexShader))
              && newShaderProgram->addFragmentShader (OpenGLHelpers::translateFragmentShaderToV3 (fragmentShader))
              && newShaderProgram->link())
        {
            // Do the following to be in a known state in case problems occur
            shape = nullptr;
            attributes = nullptr;
            uniforms = nullptr;

            // Tell the GPU that this program is the one to be used for next frame
            usedShaderProgram = newShaderProgram;
// ** Not needed **: will be called when render callback is run            shader->use();

            // Load the object that will be used to defined vertices and fragments
            shape      = new Shape (openGLContext);                             // The object itself
            attributes = new Attributes (openGLContext, *usedShaderProgram);    // The object attributes
            uniforms   = new Uniforms (openGLContext, *usedShaderProgram);      // The global data to be shared between
                                                                                // the CPU code and the GPU shaders

            statusText = "GLSL: v" + String (OpenGLShaderProgram::getLanguageVersion(), 2); // Not used in this demo
        }
        else
        {
            statusText = newShaderProgram->getLastError();                 // Not used in this demo
        }
    }


private:
    //==============================================================================
    struct Vertex
    {
        float position[3];  // To define vertex x,y,z coordinates
        float normal[3];    // Orthogonal vector used to calculate light impact on the texture color
        float colour[4];    // Color used for the vertex. If no other color info is given for the fragment
                            // the pixel colors will be interpolated from the vertex colors
        float texCoord[2];  // A graphic image (file) can be used to define the texture of the drawn object.
                            // This 2-D vector gives the coordinates in the 2-D image file corresponding to
                            // the pixel color to be drawn
    };

    //==============================================================================
    // This class just manages the attributes that the shaders use.
    // "attribute" is a special variable type modifier in the shaders which allows to pass information
    // from the CPU code to the shaders. These attributes will be passed to the Vertex shader
    // to define the coordinates, normal vector, color and texture coordinate of each vertex.
    // Note that an attribute variable can be a scalar, a vector, a matrix, etc.
    struct Attributes
    {
        Attributes (OpenGLContext& openGLContext, OpenGLShaderProgram& shaderProgram)
        {
            // Call openGL functions to get the ID (a number specific to each object or variable)
            // corresponding to the attribute whose name is given as 3rd parameter.
            // This id will be used below to tell the GPU how to use them
            position      = createAttribute (openGLContext, shaderProgram, "position");
            normal        = createAttribute (openGLContext, shaderProgram, "normal");
            sourceColour  = createAttribute (openGLContext, shaderProgram, "sourceColour");
            textureCoordIn = createAttribute (openGLContext, shaderProgram, "textureCoordIn");
        }

        // This method calls openGL functions to tell the GPU that some attributes will be used
        // for each vertex (see comments below) and will be passed as an array of data
        void enable (OpenGLContext& openGLContext)
        {
            if (position != nullptr)
            {
                // Tell the GPU that the first attribute will be the position attribute
                // 2nd parameter gives the number of data (3 coordinates) for this attribute
                // 3rd parameter gives their type (floating-point)
                // 4th parameter indicates they will be left as is (not normalized)
                // 5th parameter indicates the size of the array defined for each stored element (vertex)
                // 6th parameter is the offset in that array for the given attribute in current element
                openGLContext.extensions.glVertexAttribPointer (position->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), 0);
                openGLContext.extensions.glEnableVertexAttribArray (position->attributeID);
            }

            if (normal != nullptr)
            {
                // Tell the GPU that the next attribute will be the normal attribute
                // 2nd parameter gives the number of data (3 coordinates) for this attribute
                // 3rd parameter gives their type (floating-point)
                // 4th parameter indicates they will be left as is (not normalized)
                // 5th parameter indicates the size of the array defined for each stored element (vertex)
                // 6th parameter is the byte offset in that array for the given attribute in current element (0+3 float)
                openGLContext.extensions.glVertexAttribPointer (normal->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 3));
                openGLContext.extensions.glEnableVertexAttribArray (normal->attributeID);
            }

            if (sourceColour != nullptr)
            {
                // Tell the GPU that the next attribute will be the color attribute
                // 2nd parameter gives the number of data (R+G+B+Alpha) for this attribute
                // 3rd parameter gives their type (floating-point)
                // 4th parameter indicates they will be left as is (not normalized)
                // 5th parameter indicates the size of the array defined for each stored element (vertex)
                // 6th parameter is the byte offset in that array for the given attribute in current element (0+3+3 float)
                openGLContext.extensions.glVertexAttribPointer (sourceColour->attributeID, 4, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 6));
                openGLContext.extensions.glEnableVertexAttribArray (sourceColour->attributeID);
            }

            if (textureCoordIn != nullptr)
            {
                // Tell the GPU that the next attribute will be the texture coordinate attribute
                // 2nd parameter gives the number of data (x and y) for this attribute
                // 3rd parameter gives their type (floating-point)
                // 4th parameter indicates they will be left as is (not normalized)
                // 5th parameter indicates the size of the array defined for each stored element (vertex)
                // 6th parameter is the byte offset in that array for the given attribute in current element (0+3+3+4 float)
                openGLContext.extensions.glVertexAttribPointer (textureCoordIn->attributeID, 2, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 10));
                openGLContext.extensions.glEnableVertexAttribArray (textureCoordIn->attributeID);
            }
        }

        // This method calls openGL functions to tell the GPU to release the resources previously used to store attributes
        void disable (OpenGLContext& openGLContext)
        {
            if (position != nullptr)       openGLContext.extensions.glDisableVertexAttribArray (position->attributeID);
            if (normal != nullptr)         openGLContext.extensions.glDisableVertexAttribArray (normal->attributeID);
            if (sourceColour != nullptr)   openGLContext.extensions.glDisableVertexAttribArray (sourceColour->attributeID);
            if (textureCoordIn != nullptr)  openGLContext.extensions.glDisableVertexAttribArray (textureCoordIn->attributeID);
        }

        ScopedPointer<OpenGLShaderProgram::Attribute> position, normal, sourceColour, textureCoordIn;

    private:
        // This method calls openGL functions to get the ID (a number specific to each object or variable,
        // which is assigned by the GPU itself) corresponding to a certain attribute name, and create the
        // attribute for the OpenGL (CPU) world.
        // Basically this will allow to link a variable in the CPU code to one in the GPU (GLSL) shader.
        // Note that the variable can be a scalar, a vector, a matrix, etc.
        static OpenGLShaderProgram::Attribute* createAttribute (OpenGLContext& openGLContext,
                                                                OpenGLShaderProgram& shaderProgram,
                                                                const char* attributeName)
        {
            // Get the ID
            if (openGLContext.extensions.glGetAttribLocation (shaderProgram.getProgramID(), attributeName) < 0)
                return nullptr; // Return if error
            // Create the atttribute variable
            return new OpenGLShaderProgram::Attribute (shaderProgram, attributeName);
        }
    };

    //==============================================================================
    // This class just manages the uniform values that the demo shaders use.
    // "uniform" is a special variable type modifier in the shaders which allows to pass global
    // variables between the CPU code and the shaders.
    // These uniform global variables can be used by both the Vertex and the Fragment shaders.
    // Contrary to attributes, the uniform variables are not specific to vertices but are global.
    struct Uniforms
    {
        Uniforms (OpenGLContext& openGLContext, OpenGLShaderProgram& shaderProgram)
        {
            // Here we define 2 global variables (matrices)
            projectionMatrix = createUniform (openGLContext, shaderProgram, "projectionMatrix");
            viewMatrix       = createUniform (openGLContext, shaderProgram, "viewMatrix");
            demoTexture      = createUniform (openGLContext, shaderProgram, "demoTexture");
            textureType      = createUniform (openGLContext, shaderProgram, "textureType");
        }

        ScopedPointer<OpenGLShaderProgram::Uniform> projectionMatrix, viewMatrix, demoTexture, textureType;

    private:
        // This method calls openGL functions to get the ID (a number specific to each object or variable,
        // which is assigned by the GPU itself) corresponding to a certain uniform name, and create the
        // global variable for the OpenGL (CPU) world.
        // Basically this will allow to link a variable in the CPU code to one in the GPU (GLSL) shaders.
        // Note that the variable can be a scalar, a vector, a matrix, etc.
        static OpenGLShaderProgram::Uniform* createUniform (OpenGLContext& openGLContext,
                                                            OpenGLShaderProgram& shaderProgram,
                                                            const char* uniformName)
        {
            // Get the ID
            if (openGLContext.extensions.glGetUniformLocation (shaderProgram.getProgramID(), uniformName) < 0)
                return nullptr; // Return if error
            // Create the uniform variable
            return new OpenGLShaderProgram::Uniform (shaderProgram, uniformName);
        }
    };

    //==============================================================================
    /** This loads a 3D model from an OBJ file and converts it into some vertex buffers
        that we can draw.
        The OBJ format is a de facto standard that has been defined by a private company
        (Wavefront Technologies) to describe the contents of a vertex buffer.
        You can use software like Blender to create a 3D-object and export it as an OBJ file.
        Jules has done the routines to interpret this format and convert them to store
        the corresponding data into vertex buffers (see WaveFrontObjParser.h)
    */
    struct Shape
    {
        Shape (OpenGLContext& openGLContext)
        {
            // The obj file could contain multiple objects or groups but in this example
            // only one object has been used (shapeFile.shapes.size() = 1)
            if (shapeFile.load (BinaryData::teapot_obj).wasOk())
                for (int i = 0; i < shapeFile.shapes.size(); ++i)
                    // Store the ith object info into a vertex buffer which itself is part
                    // of a vertex buffer list. Again here inly one buffer will be used.
                    vertexBuffers.add (new VertexBuffer (openGLContext, *shapeFile.shapes.getUnchecked(i)));

        }

        // Send the buffer to the GPU to execute the vertex shader
        void draw (OpenGLContext& openGLContext, Attributes& glAttributes)
        {
            // Only one buffer used in this example (vertexBuffers.size() = 1)
            for (int i = 0; i < vertexBuffers.size(); ++i)
            {
                VertexBuffer& vertexBuffer = *vertexBuffers.getUnchecked (i);
                // Tell the GPU this is the buffer to be used
                vertexBuffer.bind();

                // Use the attributes that we have defined for each vertex
                glAttributes.enable (openGLContext);
                // Draw triangles (atomic graphic structure for OpenGL)
                // using the vertices stored in the vertexBuffer
                glDrawElements (GL_TRIANGLES, vertexBuffer.numIndices, GL_UNSIGNED_INT, 0);
                glAttributes.disable (openGLContext);   // Done
            }
        }

    private:
        struct VertexBuffer
        {
            VertexBuffer (OpenGLContext& context, WavefrontObjFile::Shape& aShape) : openGLContext (context)
            {
                // Indices are used to be more efficient. They allow for instance to define less vertices
                // when they are used more than once. For instance to draw a square using triangles (the
                // most basic shape used by GPUs) you need 2 triangles x 3 vertices, i.e. 6 vertices.
                // But in fact 2 of them are common to the 2 triangles. By indexing the vertices you can
                // significantly reduce the calls to the Vertex Shader for more complex structures.
                numIndices = aShape.mesh.indices.size();
                
                // These will call OpenGL routines to create a vertex buffer (VBO)
                openGLContext.extensions.glGenBuffers (1, &vertexBuffer);
                openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);

                Array<Vertex> vertices;
                
                // This will fill in the vertex buffer with the vertex info data that was stored in the OBJ file
                // Note that a default colour is defined here in case no info was given for the color.
                createVertexListFromMesh (aShape.mesh, vertices, COLOR_BY_CPU);
                openGLContext.extensions.glBufferData (GL_ARRAY_BUFFER,
                                                       static_cast<GLsizeiptr> (static_cast<size_t> (vertices.size()) * sizeof (Vertex)),
                                                       vertices.getRawDataPointer(), GL_STATIC_DRAW);

                // These will call OpenGL routines to create a buffer and store the indices in it
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
                // Tell the GPU to use the specified vertex and index buffers for next frame
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

        // This will fill in the vertex buffer with the vertex info data that was stored in the OBJ file.
        // A "mesh" is a set of vertices representing an object.
        // Note that a default colour (COLOR_BY_CPU) is passed here in case no info was given for the color.
        // That is the color that will be used if colorizer == COLOR_SET_BY_CPU
        static void createVertexListFromMesh (const WavefrontObjFile::Mesh& mesh, Array<Vertex>& list, Colour colour)
        {
            // Should be overwritten by the OBJ file data
            WavefrontObjFile::TextureCoord defaultTexCoord = { 0.5f, 0.5f };
            WavefrontObjFile::Vertex defaultNormal = { 0.5f, 0.5f, 0.5f };

            // Read each vertex data information and save it as a vertex in the vertex buffer
            for (int i = 0; i < mesh.vertices.size(); ++i)
            {
                const WavefrontObjFile::Vertex& v = mesh.vertices.getReference (i);

                const WavefrontObjFile::Vertex& n
                        = i < mesh.normals.size() ? mesh.normals.getReference (i) : defaultNormal;

                const WavefrontObjFile::TextureCoord& tc
                        = i < mesh.textureCoords.size() ? mesh.textureCoords.getReference (i) : defaultTexCoord;
                // Create the vertex attributes: position, normal vector, color, texture coordinate for that point
                Vertex vert =
                {
                    { v.x,  v.y, v.z },
                    {  n.x,  n.y, n.z },
                    { colour.getFloatRed(), colour.getFloatGreen(), colour.getFloatBlue(), colour.getFloatAlpha() },
                    { tc.x, tc.y }
                };
                // Add this vertex info to the vertex list
                list.add (vert);
            }
        }
    };

    const char* vertexShader;   // Source code for the vertx shader
    const char* fragmentShader; // Source code for the fragment shader
    OpenGLTexture texture;      // texture object filled with graphic file data that will be sent to the GPU

    ScopedPointer<OpenGLShaderProgram> usedShaderProgram;   // Set of vertex + fragment shaders
    ScopedPointer<Shape> shape;                     // Object providing the parameters of each vertex
    ScopedPointer<Attributes> attributes;           // Variables sent to the vertex shader
    ScopedPointer<Uniforms> uniforms;               // Variables sent to both the vertex and fragment shaders
    
    // Non-OpenGL stuff
    enum {COLOR_SET_BY_CPU,COLOR_SET_BY_GPU,TEXTURE} colorizer;
    TextButton cpuColorButton{"COLORED BY CPU"}, gpuColorButton{"COLORED BY GPU"};
    Image img = ImageCache::getFromMemory (BinaryData::rainbow_gradient_vertical_jpg, BinaryData::rainbow_gradient_vertical_jpgSize);
    ImageTextButton textureButton{"COLORED BY TEXTURE",(800/2)+200, 20, 183, 32,img};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};


// (This function is called by the app startup code to create our main component)
Component* createMainContentComponent()    { return new MainContentComponent(); }


#endif  // MAINCOMPONENT_H_INCLUDED
