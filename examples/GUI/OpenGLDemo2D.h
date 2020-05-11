/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2020 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             OpenGLDemo2D
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Simple 2D OpenGL application.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra, juce_opengl
 exporters:        xcode_mac, vs2019, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        OpenGLDemo2D

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
class OpenGLDemo2D  : public Component,
                      private CodeDocument::Listener,
                      private Timer
{
public:
    OpenGLDemo2D()
    {
        setOpaque (true);

        if (auto* peer = getPeer())
            peer->setCurrentRenderingEngine (0);

        openGLContext.attachTo (*getTopLevelComponent());

        addAndMakeVisible (statusLabel);
        statusLabel.setJustificationType (Justification::topLeft);
        statusLabel.setFont (Font (14.0f));

        auto presets = getPresets();

        for (int i = 0; i < presets.size(); ++i)
            presetBox.addItem (presets[i].name, i + 1);

        addAndMakeVisible (presetLabel);
        presetLabel.attachToComponent (&presetBox, true);

        addAndMakeVisible (presetBox);
        presetBox.onChange = [this] { selectPreset (presetBox.getSelectedItemIndex()); };

        fragmentEditorComp.setOpaque (false);
        fragmentDocument.addListener (this);
        addAndMakeVisible (fragmentEditorComp);

        presetBox.setSelectedItemIndex (0);

        setSize (500, 500);
    }

    ~OpenGLDemo2D() override
    {
        openGLContext.detach();
        shader.reset();
    }

    void paint (Graphics& g) override
    {
        g.fillCheckerBoard (getLocalBounds().toFloat(), 48.0f, 48.0f, Colours::lightgrey, Colours::white);

        if (shader.get() == nullptr || shader->getFragmentShaderCode() != fragmentCode)
        {
            shader.reset();

            if (fragmentCode.isNotEmpty())
            {
                shader.reset (new OpenGLGraphicsContextCustomShader (fragmentCode));

                auto result = shader->checkCompilation (g.getInternalContext());

                if (result.failed())
                {
                    statusLabel.setText (result.getErrorMessage(), dontSendNotification);
                    shader.reset();
                }
            }
        }

        if (shader.get() != nullptr)
        {
            statusLabel.setText ({}, dontSendNotification);

            shader->fillRect (g.getInternalContext(), getLocalBounds());
        }
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced (4);

        statusLabel.setBounds (area.removeFromTop (75));

        area.removeFromTop (area.getHeight() / 2);

        auto presets = area.removeFromTop (25);
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

    std::unique_ptr<OpenGLGraphicsContextCustomShader> shader;

    Label statusLabel, presetLabel  { {}, "Shader Preset:" };
    ComboBox presetBox;
    CodeDocument fragmentDocument;
    CodeEditorComponent fragmentEditorComp  { fragmentDocument, nullptr };
    String fragmentCode;

private:
    OpenGLContext openGLContext;

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

    struct ShaderPreset
    {
        const char* name;
        const char* fragmentShader;
    };

    static Array<ShaderPreset> getPresets()
    {
        #define SHADER_2DDEMO_HEADER \
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

                SHADER_2DDEMO_HEADER
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

                SHADER_2DDEMO_HEADER
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

                SHADER_2DDEMO_HEADER
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

                SHADER_2DDEMO_HEADER
                "void main()\n"
                "{\n"
                "    gl_FragColor = vec4 (1.0, 0.6, 0.1, pixelAlpha);\n"
                "}\n"
            }
        };

        return Array<ShaderPreset> (presets, numElementsInArray (presets));
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLDemo2D)
};
