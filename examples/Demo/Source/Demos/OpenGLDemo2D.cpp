/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#include "../JuceDemoHeader.h"

#if JUCE_OPENGL

//==============================================================================
class OpenGL2DShaderDemo  : public Component,
                            private CodeDocument::Listener,
                            private ComboBox::Listener,
                            private Timer
{
public:
    OpenGL2DShaderDemo()
        : fragmentEditorComp (fragmentDocument, nullptr)
    {
        setOpaque (true);

        if (MainAppWindow* mw = MainAppWindow::getMainAppWindow())
            mw->setOpenGLRenderingEngine();

        addAndMakeVisible (statusLabel);
        statusLabel.setJustificationType (Justification::topLeft);
        statusLabel.setColour (Label::textColourId, Colours::black);
        statusLabel.setFont (Font (14.0f));

        Array<ShaderPreset> presets (getPresets());
        StringArray presetNames;

        for (int i = 0; i < presets.size(); ++i)
            presetBox.addItem (presets[i].name, i + 1);

        addAndMakeVisible (presetLabel);
        presetLabel.setText ("Shader Preset:", dontSendNotification);
        presetLabel.attachToComponent (&presetBox, true);

        addAndMakeVisible (presetBox);
        presetBox.addListener (this);

        Colour editorBackground (Colours::white.withAlpha (0.6f));
        fragmentEditorComp.setColour (CodeEditorComponent::backgroundColourId, editorBackground);
        fragmentEditorComp.setOpaque (false);
        fragmentDocument.addListener (this);
        addAndMakeVisible (fragmentEditorComp);

        presetBox.setSelectedItemIndex (0);
    }

    ~OpenGL2DShaderDemo()
    {
        shader = nullptr;
    }

    void paint (Graphics& g) override
    {
        g.fillCheckerBoard (getLocalBounds(), 48, 48, Colours::lightgrey, Colours::white);

        if (shader == nullptr || shader->getFragmentShaderCode() != fragmentCode)
        {
            shader = nullptr;

            if (fragmentCode.isNotEmpty())
            {
                shader = new OpenGLGraphicsContextCustomShader (fragmentCode);

                Result result (shader->checkCompilation (g.getInternalContext()));

                if (result.failed())
                {
                    statusLabel.setText (result.getErrorMessage(), dontSendNotification);
                    shader = nullptr;
                }
            }
        }

        if (shader != nullptr)
        {
            statusLabel.setText (String(), dontSendNotification);

            shader->fillRect (g.getInternalContext(), getLocalBounds());
        }
    }

    void resized() override
    {
        Rectangle<int> area (getLocalBounds().reduced (4));

        statusLabel.setBounds (area.removeFromTop (75));

        area.removeFromTop (area.getHeight() / 2);

        Rectangle<int> presets (area.removeFromTop (25));
        presets.removeFromLeft (100);
        presetBox.setBounds (presets.removeFromLeft (150));

        area.removeFromTop (4);
        fragmentEditorComp.setBounds (area);
    }

    void selectPreset (int preset)
    {
        fragmentDocument.replaceAllContent (getPresets()[preset].fragmentShader);
        startTimer (1);
    }

    ScopedPointer<OpenGLGraphicsContextCustomShader> shader;

    Label statusLabel, presetLabel;
    ComboBox presetBox;
    CodeDocument fragmentDocument;
    CodeEditorComponent fragmentEditorComp;
    String fragmentCode;

private:
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
        fragmentCode = fragmentDocument.getAllContent();
        repaint();
    }

    void comboBoxChanged (ComboBox*) override
    {
        selectPreset (presetBox.getSelectedItemIndex());
    }

    struct ShaderPreset
    {
        const char* name;
        const char* fragmentShader;
    };

    static Array<ShaderPreset> getPresets()
    {
        #define SHADER_DEMO_HEADER \
            "/*  This demo shows the use of the OpenGLGraphicsContextCustomShader,\n" \
            "    which allows a 2D area to be filled using a GL shader program.\n" \
            "\n" \
            "    Edit the shader program below and it will be \n" \
            "    recompiled in real-time!\n" \
            "*/\n\n"

        ShaderPreset presets[] =
        {
            {
                "Simple Gradient",

                SHADER_DEMO_HEADER
                "void main()\n"
                "{\n"
                "    " JUCE_MEDIUMP " vec4 colour1 = vec4 (1.0, 0.4, 0.6, 1.0);\n"
                "    " JUCE_MEDIUMP " vec4 colour2 = vec4 (0.0, 0.8, 0.6, 1.0);\n"
                "    " JUCE_MEDIUMP " float alpha = pixelPos.x / 1000.0;\n"
                "    gl_FragColor = pixelAlpha * mix (colour1, colour2, alpha);\n"
                "}\n"
            },

            {
                "Circular Gradient",

                SHADER_DEMO_HEADER
                "void main()\n"
                "{\n"
                "    " JUCE_MEDIUMP " vec4 colour1 = vec4 (1.0, 0.4, 0.6, 1.0);\n"
                "    " JUCE_MEDIUMP " vec4 colour2 = vec4 (0.3, 0.4, 0.4, 1.0);\n"
                "    " JUCE_MEDIUMP " float alpha = distance (pixelPos, vec2 (600.0, 500.0)) / 400.0;\n"
                "    gl_FragColor = pixelAlpha * mix (colour1, colour2, alpha);\n"
                "}\n"
            },

            {
                "Circle",

                SHADER_DEMO_HEADER
                "void main()\n"
                "{\n"
                "    " JUCE_MEDIUMP " vec4 colour1 = vec4 (0.1, 0.1, 0.9, 1.0);\n"
                "    " JUCE_MEDIUMP " vec4 colour2 = vec4 (0.0, 0.8, 0.6, 1.0);\n"
                "    " JUCE_MEDIUMP " float distance = distance (pixelPos, vec2 (600.0, 500.0));\n"
                "\n"
                "    " JUCE_MEDIUMP " float innerRadius = 200.0;\n"
                "    " JUCE_MEDIUMP " float outerRadius = 210.0;\n"
                "\n"
                "    if (distance < innerRadius)\n"
                "        gl_FragColor = colour1;\n"
                "    else if (distance > outerRadius)\n"
                "        gl_FragColor = colour2;\n"
                "    else\n"
                "        gl_FragColor = mix (colour1, colour2, (distance - innerRadius) / (outerRadius - innerRadius));\n"
                "\n"
                "    gl_FragColor *= pixelAlpha;\n"
                "}\n"
            },

            {
                "Solid Colour",

                SHADER_DEMO_HEADER
                "void main()\n"
                "{\n"
                "    gl_FragColor = vec4 (1.0, 0.6, 0.1, pixelAlpha);\n"
                "}\n"
            }
        };

        return Array<ShaderPreset> (presets, numElementsInArray (presets));
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGL2DShaderDemo)
};

//==============================================================================
// This static object will register this demo type in a global list of demos..
static JuceDemoType<OpenGL2DShaderDemo> demo ("20 Graphics: OpenGL 2D");

#endif
