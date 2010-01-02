/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_Logger.h"


//==============================================================================
Logger::Logger()
{
}

Logger::~Logger()
{
}

//==============================================================================
static Logger* currentLogger = 0;

void Logger::setCurrentLogger (Logger* const newLogger,
                               const bool deleteOldLogger)
{
    Logger* const oldLogger = currentLogger;
    currentLogger = newLogger;

    if (deleteOldLogger)
        delete oldLogger;
}

void Logger::writeToLog (const String& message)
{
    if (currentLogger != 0)
        currentLogger->logMessage (message);
    else
        outputDebugString (message);
}

#if JUCE_LOG_ASSERTIONS
void JUCE_API juce_LogAssertion (const char* filename, const int lineNum) throw()
{
    String m ("JUCE Assertion failure in ");
    m << filename << ", line " << lineNum;

    Logger::writeToLog (m);
}
#endif

END_JUCE_NAMESPACE
