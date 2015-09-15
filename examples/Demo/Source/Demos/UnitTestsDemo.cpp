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
        TestRunnerThread (UnitTestsDemo& utd)
            : Thread ("Unit Tests"),
              owner (utd)
        {
        }

        void run() override
        {
            CustomTestRunner runner (*this);
            runner.runAllTests();

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
            addAndMakeVisible (testResultsBox);
            testResultsBox.setMultiLine (true);
            testResultsBox.setFont (Font (Font::getDefaultMonospacedFontName(), 12.0f, Font::plain));

            startTestButton.addListener (this);

            logMessage ("This panel runs all the built-in JUCE unit-tests.\n");
            logMessage ("To add your own unit-tests, see the JUCE_UNIT_TESTS macro.");
        }

        ~UnitTestsDemo()
        {
            stopTest();
        }

        //==============================================================================
        void paint (Graphics& g) override
        {
            g.fillAll (Colours::grey);
        }

        void resized() override
        {
            Rectangle<int> r (getLocalBounds().reduced (6));

            startTestButton.setBounds (r.removeFromTop (25).removeFromLeft (200));
            testResultsBox.setBounds (r.withTrimmedTop (5));
        }

        void buttonClicked (Button* buttonThatWasClicked) override
        {
            if (buttonThatWasClicked == &startTestButton)
            {
                startTest();
            }
        }

        void startTest()
        {
            testResultsBox.clear();
            startTestButton.setEnabled (false);

            currentTestThread = new TestRunnerThread (*this);
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
        TextEditor testResultsBox;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UnitTestsDemo)
    };
};

// This static object will register this demo type in a global list of demos..
static JuceDemoType<UnitTestClasses::UnitTestsDemo> demo ("40 Unit Tests");
