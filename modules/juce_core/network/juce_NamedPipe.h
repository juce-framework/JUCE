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

#ifndef __JUCE_NAMEDPIPE_JUCEHEADER__
#define __JUCE_NAMEDPIPE_JUCEHEADER__

#include "../threads/juce_ReadWriteLock.h"

//==============================================================================
/**
    A cross-process pipe that can have data written to and read from it.

    Two processes can use NamedPipe objects to exchange blocks of data.

    @see InterprocessConnection
*/
class JUCE_API  NamedPipe
{
public:
    //==============================================================================
    /** Creates a NamedPipe. */
    NamedPipe();

    /** Destructor. */
    ~NamedPipe();

    //==============================================================================
    /** Tries to open a pipe that already exists.
        Returns true if it succeeds.
    */
    bool openExisting (const String& pipeName);

    /** Tries to create a new pipe.
        Returns true if it succeeds.
    */
    bool createNewPipe (const String& pipeName);

    /** Closes the pipe, if it's open. */
    void close();

    /** True if the pipe is currently open. */
    bool isOpen() const;

    /** Returns the last name that was used to try to open this pipe. */
    String getName() const;

    //==============================================================================
    /** Reads data from the pipe.

        This will block until another thread has written enough data into the pipe to fill
        the number of bytes specified, or until another thread calls the cancelPendingReads()
        method.

        If the operation fails, it returns -1, otherwise, it will return the number of
        bytes read.

        If timeOutMilliseconds is less than zero, it will wait indefinitely, otherwise
        this is a maximum timeout for reading from the pipe.
    */
    int read (void* destBuffer, int maxBytesToRead, int timeOutMilliseconds);

    /** Writes some data to the pipe.
        @returns the number of bytes written, or -1 on failure.
    */
    int write (const void* sourceBuffer, int numBytesToWrite, int timeOutMilliseconds);

private:
    //==============================================================================
    JUCE_PUBLIC_IN_DLL_BUILD (class Pimpl)
    ScopedPointer<Pimpl> pimpl;
    String currentPipeName;
    ReadWriteLock lock;

    bool openInternal (const String& pipeName, const bool createPipe);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NamedPipe)
};


#endif   // __JUCE_NAMEDPIPE_JUCEHEADER__
