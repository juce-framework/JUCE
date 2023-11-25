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

//==============================================================================
class ScopedMessageBoxImpl
{
public:
    virtual ~ScopedMessageBoxImpl() = default;
    virtual void close() = 0;
};

//==============================================================================
class ConcreteScopedMessageBoxImpl : public ScopedMessageBoxImpl,
                                     private AsyncUpdater
{
public:
    static ScopedMessageBox show (std::unique_ptr<ScopedMessageBoxInterface>&& native,
                                  std::function<void (int)> callback)
    {
        return ScopedMessageBox (runAsync (std::move (native),
                                           rawToUniquePtr (ModalCallbackFunction::create (std::move (callback)))));
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

    ~ConcreteScopedMessageBoxImpl() override
    {
        cancelPendingUpdate();
    }

    void close() override
    {
        cancelPendingUpdate();
        nativeImplementation->close();
        self.reset();
    }

private:
    static std::shared_ptr<ConcreteScopedMessageBoxImpl> runAsync (std::unique_ptr<ScopedMessageBoxInterface>&& p,
                                                                   std::unique_ptr<ModalComponentManager::Callback>&& c)
    {
        std::shared_ptr<ConcreteScopedMessageBoxImpl> result (new ConcreteScopedMessageBoxImpl (std::move (p), std::move (c)));
        result->self = result;
        result->triggerAsyncUpdate();
        return result;
    }

    static int runSync (std::unique_ptr<ScopedMessageBoxInterface>&& p)
    {
        auto local = std::move (p);
        return local != nullptr ? local->runSync() : 0;
    }

    explicit ConcreteScopedMessageBoxImpl (std::unique_ptr<ScopedMessageBoxInterface>&& p)
        : ConcreteScopedMessageBoxImpl (std::move (p), nullptr) {}

    ConcreteScopedMessageBoxImpl (std::unique_ptr<ScopedMessageBoxInterface>&& p,
                                  std::unique_ptr<ModalComponentManager::Callback>&& c)
        : callback (std::move (c)), nativeImplementation (std::move (p)) {}

    void handleAsyncUpdate() override
    {
        nativeImplementation->runAsync ([weakRecipient = std::weak_ptr<ConcreteScopedMessageBoxImpl> (self)] (int result)
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
    std::shared_ptr<ConcreteScopedMessageBoxImpl> self;
};

} // namespace juce::detail
