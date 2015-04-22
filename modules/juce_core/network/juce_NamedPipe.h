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

#ifndef JUCE_NAMEDPIPE_H_INCLUDED
#define JUCE_NAMEDPIPE_H_INCLUDED


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


#endif   // JUCE_NAMEDPIPE_H_INCLUDED
