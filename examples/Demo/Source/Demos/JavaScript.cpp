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
        fillStandardDemoBackground (g);
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JavaScriptDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<JavaScriptDemo> demo ("40 JavaScript");
