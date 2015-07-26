/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

UnitTest::UnitTest (const String& nm)
    : name (nm), runner (nullptr)
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

void UnitTest::performTest (UnitTestRunner* const newRunner)
{
    jassert (newRunner != nullptr);
    runner = newRunner;

    initialise();
    runTest();
    shutdown();
}

void UnitTest::logMessage (const String& message)
{
    // This method's only valid while the test is being run!
    jassert (runner != nullptr);

    runner->logMessage (message);
}

void UnitTest::beginTest (const String& testName)
{
    // This method's only valid while the test is being run!
    jassert (runner != nullptr);

    runner->beginNewTest (this, testName);
}

void UnitTest::expect (const bool result, const String& failureMessage)
{
    // This method's only valid while the test is being run!
    jassert (runner != nullptr);

    if (result)
        runner->addPass();
    else
        runner->addFail (failureMessage);
}

Random UnitTest::getRandom() const
{
    // This method's only valid while the test is being run!
    jassert (runner != nullptr);

    return runner->randomForTest;
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

void UnitTestRunner::runTests (const Array<UnitTest*>& tests, int64 randomSeed)
{
    results.clear();
    resultsUpdated();

    if (randomSeed == 0)
        randomSeed = Random().nextInt (0x7ffffff);

    randomForTest = Random (randomSeed);
    logMessage ("Random seed: 0x" + String::toHexString (randomSeed));

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

void UnitTestRunner::runAllTests (int64 randomSeed)
{
    runTests (UnitTest::getAllTests(), randomSeed);
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
