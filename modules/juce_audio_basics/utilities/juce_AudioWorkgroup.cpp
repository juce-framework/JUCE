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

#if JUCE_AUDIOWORKGROUP_TYPES_AVAILABLE

class WorkgroupToken::TokenProvider
{
public:
    explicit TokenProvider (os_workgroup_t wg)
        : workgroup (wg), attached (attach (wg, token)) {}

    ~TokenProvider()
    {
        if (attached)
            detach (workgroup, token);
    }

    TokenProvider (const TokenProvider&) = delete;
    TokenProvider (TokenProvider&& other) noexcept
        : workgroup (std::exchange (other.workgroup, os_workgroup_t{})),
          token (std::exchange (other.token, os_workgroup_join_token_s{})),
          attached (std::exchange (other.attached, false)) {}

    TokenProvider& operator= (const TokenProvider&) = delete;
    TokenProvider& operator= (TokenProvider&& other) noexcept
    {
        TokenProvider { std::move (other) }.swap (*this);
        return *this;
    }

    bool isAttached()             const { return attached; }
    os_workgroup_t getHandle()    const { return workgroup; }

private:
    static void detach (os_workgroup_t wg, os_workgroup_join_token_s token)
    {
        if (@available (macos 11.0, ios 14.0, *))
            os_workgroup_leave (wg, &token);
    }

    static bool attach (os_workgroup_t wg, os_workgroup_join_token_s& tokenOut)
    {
        if (@available (macos 11.0, ios 14.0, *))
        {
            if (wg != nullptr && os_workgroup_join (wg, &tokenOut) == 0)
                return true;
        }

        return false;
    }

    void swap (TokenProvider& other) noexcept
    {
        std::swap (other.workgroup, workgroup);
        std::swap (other.token, token);
        std::swap (other.attached, attached);
    }

    os_workgroup_t workgroup;
    os_workgroup_join_token_s token;
    bool attached;
};

class AudioWorkgroup::WorkgroupProvider
{
public:
    explicit WorkgroupProvider (os_workgroup_t ptr) : handle { ptr } {}

    void join (WorkgroupToken& token) const
    {
        if (const auto* tokenProvider = token.getTokenProvider())
            if (tokenProvider->isAttached() && tokenProvider->getHandle() == handle.get())
                return;

        // Explicit reset before constructing the new workgroup to ensure that the old workgroup
        // is left before the new one is joined.
        token.reset();

        if (handle.get() != nullptr)
            token = WorkgroupToken { [provider = WorkgroupToken::TokenProvider { handle.get() }] { return &provider; } };
    }

    static os_workgroup_t getWorkgroup (const AudioWorkgroup& wg)
    {
        if (auto* provider = wg.getWorkgroupProvider())
            return provider->handle.get();

        return nullptr;
    }

private:
    struct ScopedWorkgroupRetainer
    {
        ScopedWorkgroupRetainer (os_workgroup_t wg) : handle { wg }
        {
            if (handle != nullptr)
                os_retain (handle);
        }

        ~ScopedWorkgroupRetainer()
        {
            if (handle != nullptr)
                os_release (handle);
        }

        ScopedWorkgroupRetainer (const ScopedWorkgroupRetainer& other)
            : ScopedWorkgroupRetainer { other.handle } {}

        ScopedWorkgroupRetainer& operator= (const ScopedWorkgroupRetainer& other)
        {
            ScopedWorkgroupRetainer { other }.swap (*this);
            return *this;
        }

        ScopedWorkgroupRetainer (ScopedWorkgroupRetainer&& other) noexcept
        {
            swap (other);
        }

        ScopedWorkgroupRetainer& operator= (ScopedWorkgroupRetainer&& other) noexcept
        {
            swap (other);
            return *this;
        }

        void swap (ScopedWorkgroupRetainer& other) noexcept
        {
            std::swap (handle, other.handle);
        }

        os_workgroup_t get() const noexcept { return handle; }

    private:
        os_workgroup_t handle { nullptr };
    };

    ScopedWorkgroupRetainer handle;
};

#else

class WorkgroupToken::TokenProvider {};

class AudioWorkgroup::WorkgroupProvider
{
public:
    explicit WorkgroupProvider() = default;

    void join (WorkgroupToken& t) const { t.reset(); }

    static void* getWorkgroup (const AudioWorkgroup&) { return nullptr; }
};

#endif

AudioWorkgroup::AudioWorkgroup (const AudioWorkgroup& other)
    : erased ([&]() -> Erased
              {
                  if (auto* p = other.getWorkgroupProvider())
                      return [provider = *p] { return &provider; };

                  return nullptr;
              }()) {}

bool AudioWorkgroup::operator== (const AudioWorkgroup& other) const
{
    return WorkgroupProvider::getWorkgroup (*this) == WorkgroupProvider::getWorkgroup (other);
}

void AudioWorkgroup::join (WorkgroupToken& token) const
{
   #if JUCE_AUDIOWORKGROUP_TYPES_AVAILABLE

    if (const auto* p = getWorkgroupProvider())
    {
        p->join (token);
        return;
    }

   #endif

    token.reset();
}

size_t AudioWorkgroup::getMaxParallelThreadCount() const
{
   #if JUCE_AUDIOWORKGROUP_TYPES_AVAILABLE

    if (@available (macos 11.0, ios 14.0, *))
    {
        if (auto wg = WorkgroupProvider::getWorkgroup (*this))
            return (size_t) os_workgroup_max_parallel_threads (wg, nullptr);
    }

   #endif

   return 0;
}

AudioWorkgroup::operator bool() const { return WorkgroupProvider::getWorkgroup (*this) != nullptr; }

#if JUCE_AUDIOWORKGROUP_TYPES_AVAILABLE

AudioWorkgroup makeRealAudioWorkgroup (os_workgroup_t handle)
{
    if (handle == nullptr)
        return AudioWorkgroup{};

    return AudioWorkgroup { [provider = AudioWorkgroup::WorkgroupProvider { handle }] { return &provider; } };
}

#endif

} // namespace juce
