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

#ifndef __JUCE_CHILDPROCESS_JUCEHEADER__
#define __JUCE_CHILDPROCESS_JUCEHEADER__


//==============================================================================
/**
    Launches and monitors a child process.

    This class lets you launch an executable, and read its output. You can also
    use it to check whether the child process has finished.
*/
class JUCE_API  ChildProcess
{
public:
    //==============================================================================
    /** Creates a process object.
        To actually launch the process, use start().
    */
    ChildProcess();

    /** Destructor.
        Note that deleting this object won't terminate the child process.
    */
    ~ChildProcess();

    /** Attempts to launch a child process command.

        The command should be the name of the executable file, followed by any arguments
        that are required.
        If the process has already been launched, this will launch it again. If a problem
        occurs, the method will return false.
    */
    bool start (const String& command);

    /** Attempts to launch a child process command.

        The first argument should be the name of the executable file, followed by any other
        arguments that are needed.
        If the process has already been launched, this will launch it again. If a problem
        occurs, the method will return false.
    */
    bool start (const StringArray& arguments);

    /** Returns true if the child process is alive. */
    bool isRunning() const;

    /** Attempts to read some output from the child process.
        This will attempt to read up to the given number of bytes of data from the
        process. It returns the number of bytes that were actually read.
    */
    int readProcessOutput (void* destBuffer, int numBytesToRead);

    /** Blocks until the process has finished, and then returns its complete output
        as a string.
    */
    String readAllProcessOutput();

    /** Blocks until the process is no longer running. */
    bool waitForProcessToFinish (int timeoutMs) const;

    /** Attempts to kill the child process.
        Returns true if it succeeded. Trying to read from the process after calling this may
        result in undefined behaviour.
    */
    bool kill();

private:
    //==============================================================================
    class ActiveProcess;
    friend class ScopedPointer<ActiveProcess>;
    ScopedPointer<ActiveProcess> activeProcess;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChildProcess)
};


#endif   // __JUCE_CHILDPROCESS_JUCEHEADER__
