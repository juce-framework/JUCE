/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             HelloWorldDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Simple HelloWorld application.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics
 exporters:        xcode_mac, vs2022, linux_make, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        HelloWorldDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once


//==============================================================================
class HelloWorldDemo final : public Component
{
public:
    //==============================================================================
    HelloWorldDemo()
    {
        addAndMakeVisible (helloWorldLabel);

        helloWorldLabel.setFont (FontOptions (40.00f, Font::bold));
        helloWorldLabel.setJustificationType (Justification::centred);
        helloWorldLabel.setEditable (false, false, false);
        helloWorldLabel.setColour (Label::textColourId, Colours::black);
        helloWorldLabel.setColour (TextEditor::textColourId, Colours::black);
        helloWorldLabel.setColour (TextEditor::backgroundColourId, Colour (0x00000000));

        addAndMakeVisible (quitButton);
        quitButton.onClick = [] { JUCEApplication::quit(); };

        setSize (600, 300);
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (Colour (0xffc1d0ff));

        g.setColour (Colours::white);
        g.fillPath (internalPath);

        g.setColour (Colour (0xff6f6f6f));
        g.strokePath (internalPath, PathStrokeType (5.200f));
    }

    void resized() override
    {
        helloWorldLabel.setBounds (152, 80, 296, 48);
        quitButton.setBounds (getWidth() - 176, getHeight() - 60, 120, 32);

        internalPath.clear();
        internalPath.startNewSubPath (136.0f, 80.0f);
        internalPath.quadraticTo (176.0f, 24.0f, 328.0f, 32.0f);
        internalPath.quadraticTo (472.0f, 40.0f, 472.0f, 104.0f);
        internalPath.quadraticTo (472.0f, 192.0f, 232.0f, 176.0f);
        internalPath.lineTo (184.0f, 216.0f);
        internalPath.lineTo (200.0f, 168.0f);
        internalPath.quadraticTo (96.0f, 136.0f, 136.0f, 80.0f);
        internalPath.closeSubPath();
    }

private:
    //==============================================================================
    Label helloWorldLabel { {}, TRANS ("Hello World!") };
    TextButton quitButton { TRANS ("Quit") };
    Path internalPath;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HelloWorldDemo)
};
