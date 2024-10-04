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

namespace juce::detail
{

class ConcreteScopedContentSharerImpl : public ScopedMessageBoxImpl,
                                        public AsyncUpdater
{
public:
    static ScopedMessageBox show (std::unique_ptr<ScopedContentSharerInterface>&& native,
                                  ContentSharer::Callback callback)
    {
        return ScopedMessageBox (runAsync (std::move (native), std::move (callback)));
    }

    ~ConcreteScopedContentSharerImpl() override
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
    static std::shared_ptr<ConcreteScopedContentSharerImpl> runAsync (std::unique_ptr<ScopedContentSharerInterface>&& p,
                                                                      ContentSharer::Callback&& c)
    {
        std::shared_ptr<ConcreteScopedContentSharerImpl> result (new ConcreteScopedContentSharerImpl (std::move (p), std::move (c)));
        result->self = result;
        result->triggerAsyncUpdate();
        return result;
    }

    ConcreteScopedContentSharerImpl (std::unique_ptr<ScopedContentSharerInterface>&& p,
                                     ContentSharer::Callback&& c)
        : callback (std::move (c)), nativeImplementation (std::move (p)) {}

    void handleAsyncUpdate() override
    {
        nativeImplementation->runAsync ([weakRecipient = std::weak_ptr<ConcreteScopedContentSharerImpl> (self)] (bool result, const String& error)
                                        {
                                            const auto notifyRecipient = [result, error, weakRecipient]
                                            {
                                                if (const auto locked = weakRecipient.lock())
                                                {
                                                    NullCheckedInvocation::invoke (locked->callback, result, error);
                                                    locked->self.reset();
                                                }
                                            };

                                            if (MessageManager::getInstance()->isThisTheMessageThread())
                                                notifyRecipient();
                                            else
                                                MessageManager::callAsync (notifyRecipient);
                                        });
    }

    ContentSharer::Callback callback;
    std::unique_ptr<ScopedContentSharerInterface> nativeImplementation;

    /*  The 'old' native message box API doesn't have a concept of content sharer owners.
        Instead, content sharers have to clean up after themselves, once they're done displaying.
        To allow this mode of usage, the implementation keeps an owning reference to itself,
        which is cleared once the content sharer is closed or asked to quit. To display a content
        sharer box without a scoped lifetime, just create a Pimpl instance without using
        the ScopedContentSharer wrapper, and the Pimpl will destroy itself after it is dismissed.
    */
    std::shared_ptr<ConcreteScopedContentSharerImpl> self;
};

} // namespace juce::detail
