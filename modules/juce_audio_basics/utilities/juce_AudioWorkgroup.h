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

//==============================================================================
/**
    Created by AudioWorkgroup to join the calling thread to a workgroup.
    To leave the workgroup again, destroy the WorkgroupToken.

    @see AudioWorkgroup

    @tags{Audio}
*/
class WorkgroupToken
{
public:
    /** @internal */
    class TokenProvider;

    /** @internal */
    using Erased = FixedSizeFunction<64, const TokenProvider*()>;

    /** @internal

        Creates a WorkgroupToken from a function returning a TokenProvider.
    */
    explicit WorkgroupToken (Erased e) : erased (std::move (e)) {}

    /** @internal

        Creates a disengaged WorkgroupToken, i.e. create a token without joining the thread to a
        workgroup.
    */
    WorkgroupToken()                                  = default;

    /** If the token joined the calling thread to a workgroup during construction, the destructor
        will cause the calling thread to leave that workgroup.
    */
    ~WorkgroupToken()                                 = default;

    /** @internal */
    WorkgroupToken (const WorkgroupToken&)            = delete;

    WorkgroupToken (WorkgroupToken&&) noexcept        = default;

    /** @internal */
    WorkgroupToken& operator= (const WorkgroupToken&) = delete;

    WorkgroupToken& operator= (WorkgroupToken&&)      = default;

    /** Returns true if and only if getTokenProvider() returns non-null. */
    explicit operator bool() const { return getTokenProvider() != nullptr; }

    /** The result of this function can be compared to nullptr to check whether the token
        successfully joined the calling thread to a workgroup.

        Used in the implementation to provide platform-specific information about this token.
    */
    [[nodiscard]] const TokenProvider* getTokenProvider() const { return erased != nullptr ? erased() : nullptr; }

    /** If this token was engaged by joining a workgroup, leaves that workgroup and disengages the token.

        After this call, getTokenProvider() will return nullptr.
    */
    void reset() { erased = nullptr; }

private:
    Erased erased;
};

//==============================================================================
/**
    A handle to an audio workgroup, which is a collection of realtime threads
    working together to produce audio by a common deadline.

    You can use this class to join a real-time worker thread to a workgroup.
    Rather than constructing instances of this class directly, you should use
    functions like AudioProcessor::audioWorkgroupContextChanged() and
    AudioIODevice::getWorkgroup() to fetch an engaged workgroup from the system.

    The class contains a single method, join(). Call this from your real-time
    thread to with register this workgroup.

    Here's an example of how you might use this class:

    @code
    Constructor()
    {
        startRealtimeThread (RealtimeThreadOptions{}.withApproximateAudioProcessingTime (samplesPerFrame, sampleRate));
        or
        startRealtimeThread (RealtimeThreadOptions{}.withProcessingTimeMs (10));
    }

    void Thread::run() override
    {
        WorkgroupToken token;

        getWorkgroup().join (token);

        while (wait (-1) && ! threadShouldExit())
        {
            // If the workgroup has changed, rejoin the workgroup with the same token.
            if (workgroupChanged())
                getWorkgroup().join (token);

            // Perform the work here
        }
    }

    void AudioProcessor::processBlock()
    {
        workerThread->notify();
    }
    @endcode

    @see Thread, AudioProcessor, WorkgroupToken

    @tags{Audio}
*/
class AudioWorkgroup
{
public:
    /** @internal */
    class WorkgroupProvider;

    /** @internal */
    using Erased = FixedSizeFunction<64, const WorkgroupProvider*()>;

    /** @internal

        Creates an AudioWorkgroup from a function returning a WorkgroupProvider.
    */
    explicit AudioWorkgroup (Erased e) : erased (std::move (e)) {}

    /** Move constructor. */
    AudioWorkgroup (AudioWorkgroup&&) = default;

    /** Move assignment operator. */
    AudioWorkgroup& operator= (AudioWorkgroup&&) = default;

    /** Copy constructor. */
    AudioWorkgroup (const AudioWorkgroup&);

    /** Copy assignment operator. */
    AudioWorkgroup& operator= (const AudioWorkgroup& other)
    {
        AudioWorkgroup { other }.swap (*this);
        return *this;
    }

    /** Constructs a disengaged handle that does not represent any workgroup. */
    AudioWorkgroup() = default;

    /**
        This method attempts to join the calling thread to this workgroup.

        If the join operation is successful, the token will be engaged, i.e. its
        getTokenProvider() function will return non-null.

        If the token is already engaged and represents a join to another workgroup,
        the thread will leave that workgroup before joining the workgroup represented by this
        object. If the 'token' is already engaged and is passed to the same workgroup, the method
        will not perform any action.

        It's important to note that the lifetime of the token should not exceed the lifetime
        of the associated thread and must be destroyed on the same thread.
    */
    void join (WorkgroupToken& token) const;

    /** Equality operator. */
    bool operator== (const AudioWorkgroup& other) const;

    /** Inequality operator. */
    bool operator!= (const AudioWorkgroup& other) const { return ! operator== (other); }

    /** Returns true if and only if this object represents a workgroup. */
    explicit operator bool() const;

    /** Disengages this instance so that it no longer represents a workgroup.

        After this call, operator bool() will return false.
    */
    void reset() { erased = nullptr; }

private:
    const WorkgroupProvider* getWorkgroupProvider() const { return erased != nullptr ? erased() : nullptr; }

    void swap (AudioWorkgroup& other) noexcept { std::swap (other.erased, erased); }

    Erased erased;
};

} // namespace juce
