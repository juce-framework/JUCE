/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include <JuceHeader.h>

//==============================================================================
class ConsoleLogger : public Logger
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
class ConsoleUnitTestRunner : public UnitTestRunner
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
        std::cout << argv[0] << " [--help|-h] [--list-categories] [--category category] [--seed seed]" << std::endl;
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

    Logger::setCurrentLogger (nullptr);

    for (int i = 0; i < runner.getNumResults(); ++i)
        if (runner.getResult(i)->failures > 0)
            return 1;

    return 0;
}
