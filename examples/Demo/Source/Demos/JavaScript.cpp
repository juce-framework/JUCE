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


//==============================================================================
class JavaScriptDemo    : public Component,
                          private CodeDocument::Listener,
                          private Timer
{
public:
    JavaScriptDemo()
    {
        setOpaque (true);

        addAndMakeVisible (editor = new CodeEditorComponent (codeDocument, nullptr));
        editor->setFont (Font (Font::getDefaultMonospacedFontName(), 14.0f, Font::plain));
        editor->setTabSize (4, true);

        outputDisplay.setMultiLine (true);
        outputDisplay.setReadOnly (true);
        outputDisplay.setCaretVisible (false);
        outputDisplay.setFont (Font (Font::getDefaultMonospacedFontName(), 14.0f, Font::plain));
        addAndMakeVisible (outputDisplay);

        codeDocument.addListener (this);

        editor->loadContent (
            "/*\n"
            "    Javascript! In this simple demo, the native\n"
            "    code provides an object called \'Demo\' which\n"
            "    has a method \'print\' that writes to the\n"
            "    console below...\n"
            "*/\n"
            "\n"
            "Demo.print (\"Hello World in JUCE + Javascript!\");\n"
            "Demo.print (\"\");\n"
            "\n"
            "function factorial (n)\n"
            "{\n"
            "    var total = 1;\n"
            "    while (n > 0)\n"
            "        total = total * n--;\n"
            "    return total;\n"
            "}\n"
            "\n"
            "for (var i = 1; i < 10; ++i)\n"
            "    Demo.print (\"Factorial of \" + i \n"
            "                   + \" = \" + factorial (i));\n");
    }

    void runScript()
    {
        outputDisplay.clear();

        JavascriptEngine engine;
        engine.maximumExecutionTime = RelativeTime::seconds (5);
        engine.registerNativeObject ("Demo", new DemoClass (*this));

        const double startTime = Time::getMillisecondCounterHiRes();

        Result result = engine.execute (codeDocument.getAllContent());

        const double elapsedMs = Time::getMillisecondCounterHiRes() - startTime;

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
    struct DemoClass  : public DynamicObject
    {
        DemoClass (JavaScriptDemo& demo) : owner (demo)
        {
            setMethod ("print", print);
        }

        static Identifier getClassName()   { return "Demo"; }

        static var print (const var::NativeFunctionArgs& args)
        {
            if (args.numArguments > 0)
                if (DemoClass* thisObject = dynamic_cast<DemoClass*> (args.thisObject.getObject()))
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
    ScopedPointer<CodeEditorComponent> editor;
    TextEditor outputDisplay;

    void codeDocumentTextInserted (const String&, int) override   { startTimer (300); }
    void codeDocumentTextDeleted (int, int) override              { startTimer (300); }

    void timerCallback() override
    {
        stopTimer();
        runScript();
    }

    void resized() override
    {
        Rectangle<int> r (getLocalBounds().reduced (8));

        editor->setBounds (r.removeFromTop (proportionOfHeight (0.6f)));
        outputDisplay.setBounds (r.withTrimmedTop (8));
    }

    void lookAndFeelChanged() override
    {
        outputDisplay.applyFontToAllText (outputDisplay.getFont());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JavaScriptDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<JavaScriptDemo> demo ("40 JavaScript");
