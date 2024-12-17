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
    ArgumentList args (argc, argv);

    if (args.containsOption ("--help|-h"))
    {
        std::cout << argv[0] << " [--help|-h] [--list-categories] [--category=category] [--seed=seed]" << std::endl;
        return 0;
    }

    if (args.containsOption ("--list-categories"))
    {
        for (auto& category : UnitTest::getAllCategories())
            std::cout << category << std::endl;

        return  0;
    }

    ConsoleLogger logger;
    Logger::setCurrentLogger (&logger);

    ConsoleUnitTestRunner runner;

    auto seed = [&args]
    {
        if (args.containsOption ("--seed"))
        {
            auto seedValueString = args.getValueForOption ("--seed");

            if (seedValueString.startsWith ("0x"))
                return seedValueString.getHexValue64();

            return seedValueString.getLargeIntValue();
        }

        return Random::getSystemRandom().nextInt64();
    }();

    if (args.containsOption ("--category"))
        runner.runTestsInCategory (args.getValueForOption ("--category"), seed);
    else
        runner.runAllTests (seed);

    std::vector<String> failures;

    for (int i = 0; i < runner.getNumResults(); ++i)
    {
        auto* result = runner.getResult (i);

        if (result->failures > 0)
            failures.push_back (result->unitTestName + " / " + result->subcategoryName + ": " + String (result->failures) + " test failure" + (result->failures > 1 ? "s" : ""));
    }

    if (! failures.empty())
    {
        logger.writeToLog (newLine + "Test failure summary:" + newLine);

        for (const auto& failure : failures)
            logger.writeToLog (failure);

        Logger::setCurrentLogger (nullptr);
        return 1;
    }

    logger.writeToLog (newLine + "All tests completed successfully");
    Logger::setCurrentLogger (nullptr);

    DeletedAtShutdown::deleteAll();

    return 0;
}
