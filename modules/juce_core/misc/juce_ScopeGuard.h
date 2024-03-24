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

/** An easy way to ensure that a function is called at the end of the current
    scope.

    Usage:
    @code
    {
        if (flag == true)
            return;

        // While this code executes, flag is true e.g. to prevent reentrancy
        flag = true;
        // When we exit this scope, flag must be false
        const ScopeGuard scope { [&] { flag = false; } };

        if (checkInitialCondition())
            return; // Scope's lambda will fire here...

        if (checkCriticalCondition())
            throw std::runtime_error{}; // ...or here...

        doWorkHavingEstablishedPreconditions();
    } // ...or here!
    @endcode

    @tags{Core}
*/
template <typename Fn> struct ScopeGuard : Fn { ~ScopeGuard() { Fn::operator()(); } };
template <typename Fn> ScopeGuard (Fn) -> ScopeGuard<Fn>;

/**
    A ScopeGuard that uses a std::function internally to allow type erasure.
    This can be handy; it allows lots of ErasedScopeGuards, all with different
    callbacks, to be stored in a homogeneous container.

    An instance of this type will automatically call its callback when it is destroyed.

    ErasedScopeGuard has a few similarities with std::unique_ptr:
    - Calling reset() on a unique_ptr destroys the object if it hasn't been destroyed yet
      and puts the unique_ptr back into a default/null state; calling reset() on an
      ErasedScopeGuard calls the callback if it hasn't been called yet and puts the Guard
      back into a default/null state.
    - Calling release() on a unique_ptr returns the unique_ptr back to a default state
      without destroying the managed object; calling release() on an ErasedScopeGuard
      returns the Guard back to a default state without calling the callback.
    - Moving a unique_ptr transfers the responsibility of destroying the managed object
      to another unique_ptr instance; moving an ErasedScopeGuard transfers the
      responsibility of calling the callback to another Guard instance.

    @tags{Core}
*/
class [[nodiscard]] ErasedScopeGuard
{
public:
    /** Constructs an ErasedScopeGuard with no callback. */
    ErasedScopeGuard() = default;

    /** Constructs an ErasedScopeGuard that will call the provided callback
        when the Guard is destroyed.
    */
    explicit ErasedScopeGuard (std::function<void()> d);

    /** Constructs an instance that assumes responsibility for calling other's callback. */
    ErasedScopeGuard (ErasedScopeGuard&& other) noexcept;

    /** Calls the stored callback, if any, then assumes responsibility for calling
        other's callback. After this call, other will be reset to its default state.
    */
    ErasedScopeGuard& operator= (ErasedScopeGuard&& other) noexcept;

    /** Destructor, calls the callback assigned to this ScopeGuard.
    */
    ~ErasedScopeGuard() noexcept;

    /** Calls the stored callback, if any, then resets this instance to its
        default state.
    */
    void reset();

    /** Resets this instance to its default state without calling the stored
        callback.
    */
    void release();

    JUCE_DECLARE_NON_COPYABLE (ErasedScopeGuard)

private:
    std::function<void()> detach;
};

} // namespace juce
