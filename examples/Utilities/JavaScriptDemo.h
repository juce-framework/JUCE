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

 name:             JavaScriptDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Showcases JavaScript features.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        JavaScriptDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

static constexpr const char javascriptSource[] = R"x(/*
    Javascript! In this simple demo, the native
    code provides an object called 'Demo' which
    has a method 'print' that writes to the
    console below...
*/

Demo.print ("Hello World in JUCE + Javascript!");
Demo.print ("");

function factorial (n)
{
    var total = 1;
    while (n > 0)
        total = total * n--;
    return total;
}

for (var i = 1; i < 10; ++i)
    Demo.print ("Factorial of " + i + " = " + factorial (i));
)x";

//==============================================================================
class JavaScriptDemo final : public Component,
                             private CodeDocument::Listener,
                             private Timer
{
public:
    JavaScriptDemo()
    {
        setOpaque (true);

        editor.reset (new CodeEditorComponent (codeDocument, nullptr));
        addAndMakeVisible (editor.get());
        editor->setFont (FontOptions { Font::getDefaultMonospacedFontName(), 14.0f, Font::plain });
        editor->setTabSize (4, true);

        outputDisplay.setMultiLine (true);
        outputDisplay.setReadOnly (true);
        outputDisplay.setCaretVisible (false);
        outputDisplay.setFont (FontOptions { Font::getDefaultMonospacedFontName(), 14.0f, Font::plain });
        addAndMakeVisible (outputDisplay);

        codeDocument.addListener (this);

        editor->loadContent (javascriptSource);

        setSize (600, 750);
    }

    void runScript()
    {
        outputDisplay.clear();

        JavascriptEngine engine;
        engine.maximumExecutionTime = RelativeTime::seconds (5);
        engine.registerNativeObject ("Demo", new DemoClass (*this));

        auto startTime = Time::getMillisecondCounterHiRes();

        auto result = engine.execute (codeDocument.getAllContent());

        auto elapsedMs = Time::getMillisecondCounterHiRes() - startTime;

        if (result.failed())
            outputDisplay.setText (result.getErrorMessage());
        else
            outputDisplay.insertTextAtCaret ("\n(Execution time: " + String (elapsedMs, 2) + " milliseconds)");
    }

    void consoleOutput (const String& message)
    {
        outputDisplay.moveCaretToEnd();
        outputDisplay.insertTextAtCaret (message + newLine);
    }

    //==============================================================================
    // This class is used by the script, and provides methods that the JS can call.
    struct DemoClass final : public DynamicObject
    {
        DemoClass (JavaScriptDemo& demo) : owner (demo)
        {
            setMethod ("print", print);
        }

        static Identifier getClassName()    { return "Demo"; }

        static var print (const var::NativeFunctionArgs& args)
        {
            if (args.numArguments > 0)
                if (auto* thisObject = dynamic_cast<DemoClass*> (args.thisObject.getObject()))
                    thisObject->owner.consoleOutput (args.arguments[0].toString());

            return var::undefined();
        }

        JavaScriptDemo& owner;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DemoClass)
    };


    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

private:
    CodeDocument codeDocument;
    std::unique_ptr<CodeEditorComponent> editor;
    TextEditor outputDisplay;

    void codeDocumentTextInserted (const String&, int) override    { startTimer (300); }
    void codeDocumentTextDeleted (int, int) override               { startTimer (300); }

    void timerCallback() override
    {
        stopTimer();
        runScript();
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (8);

        editor->setBounds       (r.removeFromTop (proportionOfHeight (0.6f)));
        outputDisplay.setBounds (r.withTrimmedTop (8));
    }

    void lookAndFeelChanged() override
    {
        outputDisplay.applyFontToAllText (outputDisplay.getFont());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JavaScriptDemo)
};
