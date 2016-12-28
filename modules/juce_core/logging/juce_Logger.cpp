/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/

Logger::Logger() {}

Logger::~Logger()
{
    // You're deleting this logger while it's still being used!
    // Always call Logger::setCurrentLogger (nullptr) before deleting the active logger.
    jassert (currentLogger != this);
}

Logger* Logger::currentLogger = nullptr;

void Logger::setCurrentLogger (Logger* const newLogger) noexcept    { currentLogger = newLogger; }
Logger* Logger::getCurrentLogger()  noexcept                        { return currentLogger; }

void Logger::writeToLog (const String& message)
{
    if (currentLogger != nullptr)
        currentLogger->logMessage (message);
    else
        outputDebugString (message);
}

#if JUCE_LOG_ASSERTIONS || JUCE_DEBUG
void JUCE_API JUCE_CALLTYPE logAssertion (const char* const filename, const int lineNum) noexcept
{
    String m ("JUCE Assertion failure in ");
    m << File::createFileWithoutCheckingPath (filename).getFileName() << ':' << lineNum;

   #if JUCE_LOG_ASSERTIONS
    Logger::writeToLog (m);
   #else
    DBG (m);
   #endif
}
#endif
