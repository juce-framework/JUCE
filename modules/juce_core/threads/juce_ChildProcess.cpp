/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

ChildProcess::ChildProcess() {}
ChildProcess::~ChildProcess() {}

bool ChildProcess::isRunning() const
{
    return activeProcess != nullptr && activeProcess->isRunning();
}

int ChildProcess::readProcessOutput (void* dest, int numBytes)
{
    return activeProcess != nullptr ? activeProcess->read (dest, numBytes) : 0;
}

bool ChildProcess::kill()
{
    return activeProcess == nullptr || activeProcess->killProcess();
}

uint32 ChildProcess::getExitCode() const
{
    return activeProcess != nullptr ? activeProcess->getExitCode() : 0;
}

bool ChildProcess::waitForProcessToFinish (const int timeoutMs) const
{
    const uint32 timeoutTime = Time::getMillisecondCounter() + (uint32) timeoutMs;

    do
    {
        if (! isRunning())
            return true;
    }
    while (timeoutMs < 0 || Time::getMillisecondCounter() < timeoutTime);

    return false;
}

String ChildProcess::readAllProcessOutput()
{
    MemoryOutputStream result;

    for (;;)
    {
        char buffer [512];
        const int num = readProcessOutput (buffer, sizeof (buffer));

        if (num <= 0)
            break;

        result.write (buffer, (size_t) num);
    }

    return result.toString();
}

//==============================================================================
#if JUCE_UNIT_TESTS

class ChildProcessTests  : public UnitTest
{
public:
    ChildProcessTests() : UnitTest ("ChildProcess") {}

    void runTest()
    {
        beginTest ("Child Processes");

      #if JUCE_WINDOWS || JUCE_MAC || JUCE_LINUX
        ChildProcess p;

       #if JUCE_WINDOWS
        expect (p.start ("tasklist"));
       #else
        expect (p.start ("ls /"));
       #endif

        //String output (p.readAllProcessOutput());
        //expect (output.isNotEmpty());
      #endif
    }
};

static ChildProcessTests childProcessUnitTests;

#endif
