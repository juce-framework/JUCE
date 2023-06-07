/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

class FileDescriptor
{
public:
    explicit FileDescriptor (int fileDescriptorId)
        : id { fileDescriptorId } {}

    ~FileDescriptor()
    {
        if (isValid())
            close (id);
    }

    bool isValid() const { return id >= 0; }
    int get() const { return id; }

    template <typename DataType>
    DataType readData() const
    {
        if (! isValid())
            return {};

        alignas (DataType) std::array<std::byte, sizeof (DataType)> buffer;
        size_t numberOfBytesRead { 0 };

        while (numberOfBytesRead < buffer.size())
        {
            const auto result = read (get(), buffer.data() + numberOfBytesRead, buffer.size() - numberOfBytesRead);

            if (result < 0)
                return {};

            numberOfBytesRead += (size_t) result;
        }

        return readUnaligned<DataType> (buffer.data());
    }

    template <typename DataType>
    void writeData (const DataType& value) const
    {
        if (! isValid())
            return;

        alignas (DataType) std::array<std::byte, sizeof (DataType)> buffer;
        writeUnaligned (buffer.data(), value);
        size_t numberOfBytesWritten { 0 };

        while (numberOfBytesWritten < buffer.size())
        {
            const auto result = write (get(), buffer.data() + numberOfBytesWritten, buffer.size() - numberOfBytesWritten);

            if (result < 0)
                return;

            numberOfBytesWritten += (size_t) result;
        }
    }

private:
    int id{};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FileDescriptor)
    JUCE_DECLARE_NON_MOVEABLE (FileDescriptor)
};

class EventFd
{
public:
    EventFd() = default;
    void signal() const { fd.writeData ((uint64_t) 1); }

    int get() const { return fd.get(); }
    bool isValid() const { return fd.isValid(); }

private:
    FileDescriptor fd { eventfd (0, EFD_CLOEXEC) };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EventFd)
    JUCE_DECLARE_NON_MOVEABLE (EventFd)
};

class TimerFd
{
public:
    TimerFd() = default;

    bool setIntervalMs (int ms) const
    {
        if (! fd.isValid())
            return false;

        jassert (ms >= 0);

        const auto seconds = ms / 1'000;
        const auto nanoseconds = (ms % 1'000) * 1'000'000;

        const itimerspec spec
        {
            { seconds, nanoseconds },
            { seconds, nanoseconds }
        };

        return timerfd_settime (fd.get(), 0, &spec, nullptr) == 0;
    }

    int getIntervalMs() const
    {
        if (! fd.isValid())
            return 0;

        itimerspec result{};

        if (timerfd_gettime (fd.get(), &result) != 0)
            return 0;

        return static_cast<int> (result.it_interval.tv_sec * 1'000 + result.it_interval.tv_nsec / 1'000'000);
    }

    int getAndClearNumberOfExpirations() const
    {
        return (int) fd.readData<uint64_t>();
    }

    int get() const { return fd.get(); }
    bool isValid() const { return fd.isValid(); }

private:
    FileDescriptor fd { timerfd_create (CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK) };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TimerFd)
    JUCE_DECLARE_NON_MOVEABLE (TimerFd)
};

} // namespace juce
