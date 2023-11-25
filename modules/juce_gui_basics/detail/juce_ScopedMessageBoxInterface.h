/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce::detail
{

/*
    Instances of this type can show and dismiss a message box.

    This is an interface rather than a concrete type so that platforms can pick an implementation at
    runtime if necessary.
*/
struct ScopedMessageBoxInterface
{
    virtual ~ScopedMessageBoxInterface() = default;

    /*  Shows the message box.

        When the message box exits normally, it should send the result to the passed-in function.
        The passed-in function is safe to call from any thread at any time.
    */
    virtual void runAsync (std::function<void (int)>) = 0;

    /*  Shows the message box and blocks. */
    virtual int runSync() = 0;

    /*  Forcefully closes the message box.

        This will be called when the message box handle has fallen out of scope.
        If the message box has already been closed by the user, this shouldn't do anything.
    */
    virtual void close() = 0;

    /*  Implemented differently for each platform. */
    static std::unique_ptr<ScopedMessageBoxInterface> create (const MessageBoxOptions& options);
};

} // namespace juce::detail
