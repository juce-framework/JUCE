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

#include "../JuceLibraryCode/JuceHeader.h"

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
    ConsoleLogger logger;
    Logger::setCurrentLogger (&logger);

    ConsoleUnitTestRunner runner;

    ArgumentList args (argc, argv);

    if (args.size() == 0)
    {
        runner.runAllTests();
    }
    else
    {
        if (args.containsOption ("--help|-h"))
        {
            std::cout << argv[0] << " [--help|-h] [--category category] [--list-categories]" << std::endl;
            return 0;
        }

        if (args.containsOption ("--list-categories"))
        {
            for (auto& category : UnitTest::getAllCategories())
                std::cout << category << std::endl;

            return  0;
        }

        if (args.containsOption ("--category"))
            runner.runTestsInCategory (args.getValueForOption ("--category"));
    }

    Logger::setCurrentLogger (nullptr);

    for (int i = 0; i < runner.getNumResults(); ++i)
        if (runner.getResult(i)->failures > 0)
            return 1;

    return 0;
}
