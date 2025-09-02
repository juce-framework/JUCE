/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

UnitTest::UnitTest (const String& nm, const String& ctg)
    : name (nm), category (ctg)
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

Array<UnitTest*> UnitTest::getTestsInCategory (const String& category)
{
    if (category.isEmpty())
        return getAllTests();

    Array<UnitTest*> unitTests;

    for (auto* test : getAllTests())
        if (test->getCategory() == category)
            unitTests.add (test);

    return unitTests;
}

Array<UnitTest*> UnitTest::getTestsWithName (const String& name)
{
    if (name.isEmpty())
        return getAllTests();

    Array<UnitTest*> unitTests;

    for (auto* test : getAllTests())
        if (test->getName() == name)
            unitTests.add (test);

    return unitTests;
}

StringArray UnitTest::getAllCategories()
{
    StringArray categories;

    for (auto* test : getAllTests())
        if (test->getCategory().isNotEmpty())
            categories.addIfNotAlreadyThere (test->getCategory());

    return categories;
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
UnitTestRunner::UnitTestRunner() {}
UnitTestRunner::~UnitTestRunner() {}

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

    for (auto* t : tests)
    {
        if (shouldAbortTests())
            break;

       #if JUCE_EXCEPTIONS_DISABLED
        t->performTest (this);
       #else
        try
        {
            t->performTest (this);
        }
        catch (...)
        {
            addFail ("An unhandled exception was thrown!");
        }
       #endif
    }

    endTest();
}

void UnitTestRunner::runAllTests (int64 randomSeed)
{
    runTests (UnitTest::getAllTests(), randomSeed);
}

void UnitTestRunner::runTestsInCategory (const String& category, int64 randomSeed)
{
    runTests (UnitTest::getTestsInCategory (category), randomSeed);
}

void UnitTestRunner::runTestsWithName (const String& name, int64 randomSeed)
{
    runTests (UnitTest::getTestsWithName (name), randomSeed);
}

void UnitTestRunner::logMessage (const String& message)
{
    Logger::writeToLog (message);
}

bool UnitTestRunner::shouldAbortTests()
{
    return false;
}

static String getTestNameString (const String& testName, const String& subCategory)
{
    return testName + " / " + subCategory;
}

void UnitTestRunner::beginNewTest (UnitTest* const test, const String& subCategory)
{
    endTest();
    currentTest = test;

    auto testName = test->getName();
    results.add (new TestResult (testName, subCategory));

    logMessage ("-----------------------------------------------------------------");
    logMessage ("Starting tests in: " + getTestNameString (testName, subCategory) + "...");

    resultsUpdated();
}

void UnitTestRunner::endTest()
{
    if (auto* r = results.getLast())
    {
        r->endTime = Time::getCurrentTime();

        if (r->failures > 0)
        {
            String m ("FAILED!!  ");
            m << r->failures << (r->failures == 1 ? " test" : " tests")
              << " failed, out of a total of " << (r->passes + r->failures);

            logMessage (String());
            logMessage (m);
            logMessage (String());
        }
        else
        {
            logMessage ("Completed tests in " + getTestNameString (r->unitTestName, r->subcategoryName));
        }
    }
}

void UnitTestRunner::addPass()
{
    {
        const ScopedLock sl (results.getLock());

        auto* r = results.getLast();
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

        auto* r = results.getLast();
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

} // namespace juce
