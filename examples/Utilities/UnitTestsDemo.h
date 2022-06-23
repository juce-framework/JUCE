/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2022 - Raw Material Software Limited

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

 name:             UnitTestsDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Performs unit tests.

 dependencies:     juce_analytics, juce_audio_basics, juce_audio_devices,
                   juce_audio_formats, juce_audio_processors, juce_audio_utils,
                   juce_core, juce_cryptography, juce_data_structures, juce_dsp,
                   juce_events, juce_graphics, juce_gui_basics, juce_gui_extra,
                   juce_opengl, juce_osc, juce_product_unlocking, juce_video
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1,JUCE_PLUGINHOST_VST3=1
 defines:          JUCE_UNIT_TESTS=1

 type:             Component
 mainClass:        UnitTestsDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
class UnitTestsDemo  : public Component
{
public:
    UnitTestsDemo()
    {
        setOpaque (true);

        addAndMakeVisible (startTestButton);
        startTestButton.onClick = [this] { start(); };

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

        setSize (500, 500);
    }

    ~UnitTestsDemo() override
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
        categoriesBox  .setBounds (topSlice.removeFromLeft (250));

        bounds.removeFromTop (5);
        testResultsBox.setBounds (bounds);
    }

    void start()
    {
        startTest (categoriesBox.getText());
    }

    void startTest (const String& category)
    {
        testResultsBox.clear();
        startTestButton.setEnabled (false);

        currentTestThread.reset (new TestRunnerThread (*this, category));
        currentTestThread->startThread();
    }

    void stopTest()
    {
        if (currentTestThread.get() != nullptr)
        {
            currentTestThread->stopThread (15000);
            currentTestThread.reset();
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
    //==============================================================================
    class TestRunnerThread  : public Thread,
                              private Timer
    {
    public:
        TestRunnerThread (UnitTestsDemo& utd, const String& ctg)
            : Thread ("Unit Tests"),
              owner (utd),
              category (ctg)
        {}

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
            WeakReference<UnitTestsDemo> safeOwner (&owner);

            MessageManager::callAsync ([=]
            {
                if (auto* o = safeOwner.get())
                    o->logMessage (message);
            });
        }

        void timerCallback() override
        {
            if (! isThreadRunning())
                owner.testFinished(); // inform the demo page when done, so it can delete this thread.
        }

    private:
        //==============================================================================
        // This subclass of UnitTestRunner is used to redirect the test output to our
        // TextBox, and to interrupt the running tests when our thread is asked to stop..
        class CustomTestRunner  : public UnitTestRunner
        {
        public:
            CustomTestRunner (TestRunnerThread& trt)  : owner (trt) {}

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

        UnitTestsDemo& owner;
        const String category;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TestRunnerThread)
    };
    std::unique_ptr<TestRunnerThread> currentTestThread;

    TextButton startTestButton { "Run Unit Tests..." };
    ComboBox categoriesBox;
    TextEditor testResultsBox;

    void lookAndFeelChanged() override
    {
        testResultsBox.applyFontToAllText (testResultsBox.getFont());
    }

    JUCE_DECLARE_WEAK_REFERENCEABLE (UnitTestsDemo)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UnitTestsDemo)
};
