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
        If mustNotExist is true then it will fail if a pipe is already
        open with the same name.
    */
    bool createNewPipe (const String& pipeName, bool mustNotExist = false);

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

    bool openInternal (const String& pipeName, bool createPipe, bool mustNotExist);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NamedPipe)
};


#endif   // JUCE_NAMEDPIPE_H_INCLUDED
