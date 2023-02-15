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

namespace juce
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

//==============================================================================
class ScopedMessageBox::Pimpl : private AsyncUpdater
{
public:
    static ScopedMessageBox show (std::unique_ptr<ScopedMessageBoxInterface>&& native, std::function<void (int)> callback)
    {
        return ScopedMessageBox (runAsync (std::move (native), rawToUniquePtr (ModalCallbackFunction::create (std::move (callback)))));
    }

    static int showUnmanaged (std::unique_ptr<ScopedMessageBoxInterface>&& native,
                              ModalComponentManager::Callback* cb)
    {
       #if JUCE_MODAL_LOOPS_PERMITTED
        if (cb == nullptr)
            return runSync (std::move (native));
       #endif

        runAsync (std::move (native), rawToUniquePtr (cb));

        return 0;
    }

    ~Pimpl() override
    {
        cancelPendingUpdate();
    }

    void close()
    {
        cancelPendingUpdate();
        nativeImplementation->close();
        self.reset();
    }

private:
    static std::shared_ptr<Pimpl> runAsync (std::unique_ptr<ScopedMessageBoxInterface>&& p,
                                            std::unique_ptr<ModalComponentManager::Callback>&& c)
    {
        std::shared_ptr<Pimpl> result (new Pimpl (std::move (p), std::move (c)));
        result->self = result;
        result->triggerAsyncUpdate();
        return result;
    }

    static int runSync (std::unique_ptr<ScopedMessageBoxInterface>&& p)
    {
        auto local = std::move (p);
        return local != nullptr ? local->runSync() : 0;
    }

    explicit Pimpl (std::unique_ptr<ScopedMessageBoxInterface>&& p)
        : Pimpl (std::move (p), nullptr) {}

    Pimpl (std::unique_ptr<ScopedMessageBoxInterface>&& p,
           std::unique_ptr<ModalComponentManager::Callback>&& c)
        : callback (std::move (c)), nativeImplementation (std::move (p)) {}

    void handleAsyncUpdate() override
    {
        nativeImplementation->runAsync ([weakRecipient = std::weak_ptr<Pimpl> (self)] (int result)
        {
            const auto notifyRecipient = [result, weakRecipient]
            {
                if (const auto locked = weakRecipient.lock())
                {
                    if (auto* cb = locked->callback.get())
                        cb->modalStateFinished (result);

                    locked->self.reset();
                }
            };

            if (MessageManager::getInstance()->isThisTheMessageThread())
                notifyRecipient();
            else
                MessageManager::callAsync (notifyRecipient);
        });
    }

    std::unique_ptr<ModalComponentManager::Callback> callback;
    std::unique_ptr<ScopedMessageBoxInterface> nativeImplementation;

    /*  The 'old' native message box API doesn't have a concept of message box owners.
        Instead, message boxes have to clean up after themselves, once they're done displaying.
        To allow this mode of usage, the implementation keeps an owning reference to itself,
        which is cleared once the message box is closed or asked to quit. To display a native
        message box without a scoped lifetime, just create a Pimpl instance without using
        the ScopedMessageBox wrapper, and the Pimpl will destroy itself after it is dismissed.
    */
    std::shared_ptr<Pimpl> self;
};

//==============================================================================
ScopedMessageBox::ScopedMessageBox() = default;

ScopedMessageBox::ScopedMessageBox (std::shared_ptr<Pimpl> p)
    : pimpl (std::move (p)) {}

ScopedMessageBox::~ScopedMessageBox() noexcept
{
    close();
}

ScopedMessageBox::ScopedMessageBox (ScopedMessageBox&& other) noexcept
    : pimpl (std::exchange (other.pimpl, nullptr)) {}

ScopedMessageBox& ScopedMessageBox::operator= (ScopedMessageBox&& other) noexcept
{
    ScopedMessageBox temp (std::move (other));
    std::swap (temp.pimpl, pimpl);
    return *this;
}

void ScopedMessageBox::close()
{
    if (pimpl != nullptr)
        pimpl->close();

    pimpl.reset();
}

} // namespace juce
