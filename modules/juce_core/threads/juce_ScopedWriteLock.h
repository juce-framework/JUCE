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

#ifndef JUCE_SCOPEDWRITELOCK_H_INCLUDED
#define JUCE_SCOPEDWRITELOCK_H_INCLUDED


//==============================================================================
/**
    Automatically locks and unlocks a ReadWriteLock object.

    Use one of these as a local variable to control access to a ReadWriteLock.

    e.g. @code

    ReadWriteLock myLock;

    for (;;)
    {
        const ScopedWriteLock myScopedLock (myLock);
        // myLock is now locked

        ...do some stuff...

        // myLock gets unlocked here.
    }
    @endcode

    @see ReadWriteLock, ScopedReadLock
*/
class JUCE_API  ScopedWriteLock
{
public:
    //==============================================================================
    /** Creates a ScopedWriteLock.

        As soon as it is created, this will call ReadWriteLock::enterWrite(), and
        when the ScopedWriteLock object is deleted, the ReadWriteLock will
        be unlocked.

        Make sure this object is created and deleted by the same thread,
        otherwise there are no guarantees what will happen! Best just to use it
        as a local stack object, rather than creating one with the new() operator.
    */
    inline explicit ScopedWriteLock (const ReadWriteLock& lock) noexcept : lock_ (lock) { lock.enterWrite(); }

    /** Destructor.

        The ReadWriteLock's exitWrite() method will be called when the destructor is called.

        Make sure this object is created and deleted by the same thread,
        otherwise there are no guarantees what will happen!
    */
    inline ~ScopedWriteLock() noexcept                                   { lock_.exitWrite(); }


private:
    //==============================================================================
    const ReadWriteLock& lock_;

    JUCE_DECLARE_NON_COPYABLE (ScopedWriteLock)
};


#endif   // JUCE_SCOPEDWRITELOCK_H_INCLUDED
