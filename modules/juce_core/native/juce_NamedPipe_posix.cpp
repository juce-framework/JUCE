/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#if ! JUCE_WASM

class NamedPipe::Pimpl
{
public:
    Pimpl (const String& pipePath, bool createPipe)
       : pipeInName  (pipePath + "_in"),
         pipeOutName (pipePath + "_out"),
         createdPipe (createPipe)
    {
        signal (SIGPIPE, signalHandler);
        juce_siginterrupt (SIGPIPE, 1);
    }

    ~Pimpl()
    {
        pipeIn .close();
        pipeOut.close();

        if (createdPipe)
        {
            if (createdFifoIn)  unlink (pipeInName.toUTF8());
            if (createdFifoOut) unlink (pipeOutName.toUTF8());
        }
    }

    bool connect (int timeOutMilliseconds)
    {
        return openPipe (true, getTimeoutEnd (timeOutMilliseconds)) != invalidPipe;
    }

    int read (char* destBuffer, int maxBytesToRead, int timeOutMilliseconds)
    {
        auto timeoutEnd = getTimeoutEnd (timeOutMilliseconds);
        int bytesRead = 0;

        while (bytesRead < maxBytesToRead)
        {
            const auto pipe = pipeIn.get();

            auto bytesThisTime = maxBytesToRead - bytesRead;
            auto numRead = (int) ::read (pipe, destBuffer, (size_t) bytesThisTime);

            if (numRead <= 0)
            {
                const auto error = errno;

                if (! (error == EWOULDBLOCK || error == EAGAIN) || stopReadOperation.load() || hasExpired (timeoutEnd))
                    return -1;

                const int maxWaitingTime = 30;
                waitForInput (pipe, timeoutEnd == 0 ? maxWaitingTime
                                                    : jmin (maxWaitingTime,
                                                            (int) (timeoutEnd - Time::getMillisecondCounter())));
                continue;
            }

            bytesRead += numRead;
            destBuffer += numRead;
        }

        return bytesRead;
    }

    int write (const char* sourceBuffer, int numBytesToWrite, int timeOutMilliseconds)
    {
        auto timeoutEnd = getTimeoutEnd (timeOutMilliseconds);

        const auto pipe = openPipe (false, timeoutEnd);

        if (pipe == invalidPipe)
            return -1;

        int bytesWritten = 0;

        while (bytesWritten < numBytesToWrite && ! hasExpired (timeoutEnd))
        {
            auto bytesThisTime = numBytesToWrite - bytesWritten;
            auto numWritten = (int) ::write (pipe, sourceBuffer, (size_t) bytesThisTime);

            if (numWritten < 0)
            {
                const auto error = errno;
                const int maxWaitingTime = 30;

                if (error == EWOULDBLOCK || error == EAGAIN)
                    waitToWrite (pipe, timeoutEnd == 0 ? maxWaitingTime
                                                       : jmin (maxWaitingTime,
                                                               (int) (timeoutEnd - Time::getMillisecondCounter())));
                else
                    return -1;

                numWritten = 0;
            }

            bytesWritten += numWritten;
            sourceBuffer += numWritten;
        }

        return bytesWritten;
    }

    static bool createFifo (const String& name, bool mustNotExist)
    {
        return mkfifo (name.toUTF8(), 0666) == 0 || ((! mustNotExist) && errno == EEXIST);
    }

    bool createFifos (bool mustNotExist)
    {
        createdFifoIn  = createFifo (pipeInName, mustNotExist);
        createdFifoOut = createFifo (pipeOutName, mustNotExist);

        return createdFifoIn && createdFifoOut;
    }

    static constexpr auto invalidPipe = -1;

    class PipeDescriptor
    {
    public:
        template <typename Fn>
        int get (Fn&& fn)
        {
            {
                const ScopedReadLock l (mutex);

                if (descriptor != invalidPipe)
                    return descriptor;
            }

            const ScopedWriteLock l (mutex);
            return descriptor = fn();
        }

        void close()
        {
            {
                const ScopedReadLock l (mutex);

                if (descriptor == invalidPipe)
                    return;
            }

            const ScopedWriteLock l (mutex);
            ::close (descriptor);
            descriptor = invalidPipe;
        }

        int get()
        {
            const ScopedReadLock l (mutex);
            return descriptor;
        }

    private:
        ReadWriteLock mutex;
        int descriptor = invalidPipe;
    };

    const String pipeInName, pipeOutName;
    PipeDescriptor pipeIn, pipeOut;
    bool createdFifoIn = false, createdFifoOut = false;

    const bool createdPipe;
    std::atomic<bool> stopReadOperation { false };

private:
    static void signalHandler (int) {}

    static uint32 getTimeoutEnd (int timeOutMilliseconds)
    {
        return timeOutMilliseconds >= 0 ? Time::getMillisecondCounter() + (uint32) timeOutMilliseconds : 0;
    }

    static bool hasExpired (uint32 timeoutEnd)
    {
        return timeoutEnd != 0 && Time::getMillisecondCounter() >= timeoutEnd;
    }

    int openPipe (const String& name, int flags, uint32 timeoutEnd)
    {
        for (;;)
        {
            auto p = ::open (name.toUTF8(), flags);

            if (p != invalidPipe || hasExpired (timeoutEnd) || stopReadOperation.load())
                return p;

            Thread::sleep (2);
        }
    }

    int openPipe (bool isInput, uint32 timeoutEnd)
    {
        auto& pipe = isInput ? pipeIn : pipeOut;
        const auto flags = (isInput ? O_RDWR : O_WRONLY) | O_NONBLOCK;

        const String& pipeName = isInput ? (createdPipe ? pipeInName : pipeOutName)
                                         : (createdPipe ? pipeOutName : pipeInName);

        return pipe.get ([this, &pipeName, &flags, &timeoutEnd]
        {
            return openPipe (pipeName, flags, timeoutEnd);
        });
    }

    static void waitForInput (int handle, int timeoutMsecs) noexcept
    {
        pollfd pfd { handle, POLLIN, 0 };
        poll (&pfd, 1, timeoutMsecs);
    }

    static void waitToWrite (int handle, int timeoutMsecs) noexcept
    {
        pollfd pfd { handle, POLLOUT, 0 };
        poll (&pfd, 1, timeoutMsecs);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

void NamedPipe::close()
{
    {
        const ScopedReadLock sl (lock);

        if (pimpl != nullptr)
        {
            pimpl->stopReadOperation = true;

            const char buffer[] { 0 };
            [[maybe_unused]] const auto done = ::write (pimpl->pipeIn.get(), buffer, numElementsInArray (buffer));
        }
    }

    {
        const ScopedWriteLock sl (lock);
        pimpl.reset();
    }
}

bool NamedPipe::openInternal (const String& pipeName, bool createPipe, bool mustNotExist)
{
   #if JUCE_IOS
    pimpl.reset (new Pimpl (File::getSpecialLocation (File::tempDirectory)
                             .getChildFile (File::createLegalFileName (pipeName)).getFullPathName(), createPipe));
   #else
    auto file = pipeName;

    if (! File::isAbsolutePath (file))
        file = "/tmp/" + File::createLegalFileName (file);

    pimpl.reset (new Pimpl (file, createPipe));
   #endif

    if (createPipe && ! pimpl->createFifos (mustNotExist))
    {
        pimpl.reset();
        return false;
    }

    if (! pimpl->connect (200))
    {
        pimpl.reset();
        return false;
    }

    return true;
}

int NamedPipe::read (void* destBuffer, int maxBytesToRead, int timeOutMilliseconds)
{
    ScopedReadLock sl (lock);
    return pimpl != nullptr ? pimpl->read (static_cast<char*> (destBuffer), maxBytesToRead, timeOutMilliseconds) : -1;
}

int NamedPipe::write (const void* sourceBuffer, int numBytesToWrite, int timeOutMilliseconds)
{
    ScopedReadLock sl (lock);
    return pimpl != nullptr ? pimpl->write (static_cast<const char*> (sourceBuffer), numBytesToWrite, timeOutMilliseconds) : -1;
}

#endif

} // namespace juce
