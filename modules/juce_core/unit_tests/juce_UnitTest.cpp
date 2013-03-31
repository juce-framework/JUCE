/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

UnitTest::UnitTest (const String& name_)
    : name (name_), runner (nullptr)
{
    getAllTests().add (this);
}

UnitTest::~UnitTest()
{
    getAllTests().removeFirstMatchingValue (this);
}

Array<UnitTest*>& UnitTest::getAllTests()
{
    static Array<UnitTest*> tests;
    return tests;
}

void UnitTest::initialise()  {}
void UnitTest::shutdown()   {}

void UnitTest::performTest (UnitTestRunner* const runner_)
{
    jassert (runner_ != nullptr);
    runner = runner_;

    initialise();
    runTest();
    shutdown();
}

void UnitTest::logMessage (const String& message)
{
    runner->logMessage (message);
}

void UnitTest::beginTest (const String& testName)
{
    runner->beginNewTest (this, testName);
}

void UnitTest::expect (const bool result, const String& failureMessage)
{
    if (result)
        runner->addPass();
    else
        runner->addFail (failureMessage);
}

//==============================================================================
UnitTestRunner::UnitTestRunner()
    : currentTest (nullptr),
      assertOnFailure (true),
      logPasses (false)
{
}

UnitTestRunner::~UnitTestRunner()
{
}

void UnitTestRunner::setAssertOnFailure (bool shouldAssert) noexcept
{
    assertOnFailure = shouldAssert;
}

void UnitTestRunner::setPassesAreLogged (bool shouldDisplayPasses) noexcept
{
    logPasses = shouldDisplayPasses;
}

int UnitTestRunner::getNumResults() const noexcept
{
    return results.size();
}

const UnitTestRunner::TestResult* UnitTestRunner::getResult (int index) const noexcept
{
    return results [index];
}

void UnitTestRunner::resultsUpdated()
{
}

void UnitTestRunner::runTests (const Array<UnitTest*>& tests)
{
    results.clear();
    resultsUpdated();

    for (int i = 0; i < tests.size(); ++i)
    {
        if (shouldAbortTests())
            break;

        try
        {
            tests.getUnchecked(i)->performTest (this);
        }
        catch (...)
        {
            addFail ("An unhandled exception was thrown!");
        }
    }

    endTest();
}

void UnitTestRunner::runAllTests()
{
    runTests (UnitTest::getAllTests());
}

void UnitTestRunner::logMessage (const String& message)
{
    Logger::writeToLog (message);
}

bool UnitTestRunner::shouldAbortTests()
{
    return false;
}

void UnitTestRunner::beginNewTest (UnitTest* const test, const String& subCategory)
{
    endTest();
    currentTest = test;

    TestResult* const r = new TestResult();
    results.add (r);
    r->unitTestName = test->getName();
    r->subcategoryName = subCategory;
    r->passes = 0;
    r->failures = 0;

    logMessage ("-----------------------------------------------------------------");
    logMessage ("Starting test: " + r->unitTestName + " / " + subCategory + "...");

    resultsUpdated();
}

void UnitTestRunner::endTest()
{
    if (results.size() > 0)
    {
        TestResult* const r = results.getLast();

        if (r->failures > 0)
        {
            String m ("FAILED!!  ");
            m << r->failures << (r->failures == 1 ? " test" : " tests")
              << " failed, out of a total of " << (r->passes + r->failures);

            logMessage (String::empty);
            logMessage (m);
            logMessage (String::empty);
        }
        else
        {
            logMessage ("All tests completed successfully");
        }
    }
}

void UnitTestRunner::addPass()
{
    {
        const ScopedLock sl (results.getLock());

        TestResult* const r = results.getLast();
        jassert (r != nullptr); // You need to call UnitTest::beginTest() before performing any tests!

        r->passes++;

        if (logPasses)
        {
            String message ("Test ");
            message << (r->failures + r->passes) << " passed";
            logMessage (message);
        }
    }

    resultsUpdated();
}

void UnitTestRunner::addFail (const String& failureMessage)
{
    {
        const ScopedLock sl (results.getLock());

        TestResult* const r = results.getLast();
        jassert (r != nullptr); // You need to call UnitTest::beginTest() before performing any tests!

        r->failures++;

        String message ("!!! Test ");
        message << (r->failures + r->passes) << " failed";

        if (failureMessage.isNotEmpty())
            message << ": " << failureMessage;

        r->messages.add (message);

        logMessage (message);
    }

    resultsUpdated();

    if (assertOnFailure) { jassertfalse; }
}
