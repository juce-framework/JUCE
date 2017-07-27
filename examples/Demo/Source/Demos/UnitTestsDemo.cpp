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


struct UnitTestClasses
{
    class UnitTestsDemo;
    class TestRunnerThread;

    //==============================================================================
    // This subclass of UnitTestRunner is used to redirect the test output to our
    // TextBox, and to interrupt the running tests when our thread is asked to stop..
    class CustomTestRunner  : public UnitTestRunner
    {
    public:
        CustomTestRunner (TestRunnerThread& trt)  : owner (trt)
        {
        }

        void logMessage (const String& message) override
        {
            owner.logMessage (message);
        }

        bool shouldAbortTests() override
        {
            return owner.threadShouldExit();
        }

    private:
        TestRunnerThread& owner;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomTestRunner)
    };

    //==============================================================================
    class TestRunnerThread  : public Thread,
                              private Timer
    {
    public:
        TestRunnerThread (UnitTestsDemo& utd, const String& ctg)
            : Thread ("Unit Tests"),
              owner (utd),
              category (ctg)
        {
        }

        void run() override
        {
            CustomTestRunner runner (*this);

            if (category == "All Tests")
                runner.runAllTests();
            else
                runner.runTestsInCategory (category);

            startTimer (50); // when finished, start the timer which will
                             // wait for the thread to end, then tell our component.
        }

        void logMessage (const String& message)
        {
            MessageManagerLock mm (this);

            if (mm.lockWasGained()) // this lock may fail if this thread has been told to stop
                owner.logMessage (message);
        }

        void timerCallback() override
        {
            if (! isThreadRunning())
                owner.testFinished(); // inform the demo page when done, so it can delete this thread.
        }

    private:
        UnitTestsDemo& owner;
        const String category;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TestRunnerThread)
    };


    //==============================================================================
    class UnitTestsDemo  : public Component,
                           public Button::Listener
    {
    public:
        UnitTestsDemo()
            : startTestButton ("Run Unit Tests...")
        {
            setOpaque (true);

            addAndMakeVisible (startTestButton);
            startTestButton.addListener (this);

            addAndMakeVisible (testResultsBox);
            testResultsBox.setMultiLine (true);
            testResultsBox.setFont (Font (Font::getDefaultMonospacedFontName(), 12.0f, Font::plain));

            addAndMakeVisible (categoriesBox);
            categoriesBox.addItem ("All Tests", 1);

            auto categories = UnitTest::getAllCategories();
            categories.sort (true);

            categoriesBox.addItemList (categories, 2);
            categoriesBox.setSelectedId (1);

            logMessage ("This panel runs the built-in JUCE unit-tests from the selected category.\n");
            logMessage ("To add your own unit-tests, see the JUCE_UNIT_TESTS macro.");
        }

        ~UnitTestsDemo()
        {
            stopTest();
        }

        //==============================================================================
        void paint (Graphics& g) override
        {
            g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground,
                                               Colours::grey));
        }

        void resized() override
        {
            auto bounds = getLocalBounds().reduced (6);

            auto topSlice = bounds.removeFromTop (25);
            startTestButton.setBounds (topSlice.removeFromLeft (200));
            topSlice.removeFromLeft (10);
            categoriesBox.setBounds (topSlice.removeFromLeft (250));

            bounds.removeFromTop (5);
            testResultsBox.setBounds (bounds);
        }

        void buttonClicked (Button* buttonThatWasClicked) override
        {
            if (buttonThatWasClicked == &startTestButton)
                startTest (categoriesBox.getText());
        }

        void startTest (const String& category)
        {
            testResultsBox.clear();
            startTestButton.setEnabled (false);

            currentTestThread = new TestRunnerThread (*this, category);
            currentTestThread->startThread();
        }

        void stopTest()
        {
            if (currentTestThread != nullptr)
            {
                currentTestThread->stopThread (15000);
                currentTestThread = nullptr;
            }
        }

        void logMessage (const String& message)
        {
            testResultsBox.moveCaretToEnd();
            testResultsBox.insertTextAtCaret (message + newLine);
            testResultsBox.moveCaretToEnd();
        }

        void testFinished()
        {
            stopTest();
            startTestButton.setEnabled (true);
            logMessage (newLine + "*** Tests finished ***");
        }

    private:
        ScopedPointer<TestRunnerThread> currentTestThread;

        TextButton startTestButton;
        ComboBox categoriesBox;
        TextEditor testResultsBox;

        void lookAndFeelChanged() override
        {
            testResultsBox.applyFontToAllText (testResultsBox.getFont());
        }

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UnitTestsDemo)
    };
};

// This static object will register this demo type in a global list of demos..
static JuceDemoType<UnitTestClasses::UnitTestsDemo> demo ("40 Unit Tests");
