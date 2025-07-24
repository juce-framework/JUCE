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

#include <JuceHeader.h>

//==============================================================================
class ConsoleLogger final : public Logger
{
    void logMessage (const String& message) override
    {
        std::cout << message << std::endl;

       #if JUCE_WINDOWS
        Logger::outputDebugString (message);
       #endif
    }
};

//==============================================================================
class ConsoleUnitTestRunner final : public UnitTestRunner
{
    void logMessage (const String& message) override
    {
        Logger::writeToLog (message);
    }
};


//==============================================================================
int main (int argc, char **argv)
{
    constexpr auto helpOption = "--help|-h";
    constexpr auto listOption = "--list-categories|-l";
    constexpr auto categoryOption = "--category|-c";
    constexpr auto seedOption = "--seed|-s";
    constexpr auto nameOption = "--name|-n";

    ArgumentList args (argc, argv);

    if (args.containsOption (helpOption))
    {
        std::cout << argv[0]
                  << " [" << helpOption << "]"
                  << " [" << listOption << "]"
                  << " [" << categoryOption << "=category]"
                  << " [" << seedOption << "=seed]"
                  << " [" << nameOption << "=name]"
                  << std::endl;
        return 0;
    }

    if (args.containsOption (listOption))
    {
        for (auto& category : UnitTest::getAllCategories())
            std::cout << category << std::endl;

        return  0;
    }

    ConsoleLogger logger;
    Logger::setCurrentLogger (&logger);

    const ScopeGuard onExit { [&]
    {
        Logger::setCurrentLogger (nullptr);
        DeletedAtShutdown::deleteAll();
    }};

    ConsoleUnitTestRunner runner;

    const auto seed = std::invoke ([&]
    {
        if (args.containsOption (seedOption))
        {
            auto seedValueString = args.getValueForOption (seedOption);

            if (seedValueString.startsWith ("0x"))
                return seedValueString.getHexValue64();

            return seedValueString.getLargeIntValue();
        }

        return Random::getSystemRandom().nextInt64();
    });

    if (args.containsOption (categoryOption))
        runner.runTestsInCategory (args.getValueForOption (categoryOption), seed);
    else if (args.containsOption (nameOption))
        runner.runTestsWithName (args.getValueForOption (nameOption), seed);
    else
        runner.runAllTests (seed);

    std::vector<String> failures;

    for (int i = 0; i < runner.getNumResults(); ++i)
    {
        auto* result = runner.getResult (i);

        if (result->failures > 0)
        {
            const auto testName = result->unitTestName + " / " + result->subcategoryName;
            const auto testSummary = String (result->failures) + " test failure" + (result->failures > 1 ? "s" : "");
            const auto newLineAndTab = newLine + "\t";

            failures.push_back (testName + ": " + testSummary + newLineAndTab
                                + result->messages.joinIntoString (newLineAndTab));
        }
    }

    logger.writeToLog (newLine + String::repeatedString ("-", 65));

    if (! failures.empty())
    {
        logger.writeToLog ("Test failure summary:");

        for (const auto& failure : failures)
            logger.writeToLog (newLine + failure);

        return 1;
    }

    logger.writeToLog ("All tests completed successfully");
    return 0;
}
