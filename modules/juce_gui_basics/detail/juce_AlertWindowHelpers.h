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

struct AlertWindowHelpers
{
    AlertWindowHelpers() = delete;

    static std::unique_ptr<ScopedMessageBoxInterface> create (const MessageBoxOptions& opts)
    {
        class AlertWindowImpl : public detail::ScopedMessageBoxInterface
        {
        public:
            explicit AlertWindowImpl (const MessageBoxOptions& opts) : options (opts) {}

            void runAsync (std::function<void (int)> recipient) override
            {
                if (auto* comp = setUpAlert())
                    comp->enterModalState (true, ModalCallbackFunction::create (std::move (recipient)), true);
                else
                    NullCheckedInvocation::invoke (recipient, 0);
            }

            int runSync() override
            {
               #if JUCE_MODAL_LOOPS_PERMITTED
                if (auto comp = rawToUniquePtr (setUpAlert()))
                    return comp->runModalLoop();
               #endif

                jassertfalse;
                return 0;
            }

            void close() override
            {
                if (alert != nullptr)
                    if (alert->isCurrentlyModal())
                        alert->exitModalState();

                alert = nullptr;
            }

        private:
            Component* setUpAlert()
            {
                auto* component = options.getAssociatedComponent();

                auto& lf = component != nullptr ? component->getLookAndFeel()
                                                : LookAndFeel::getDefaultLookAndFeel();

                alert = lf.createAlertWindow (options.getTitle(),
                                              options.getMessage(),
                                              options.getButtonText (0),
                                              options.getButtonText (1),
                                              options.getButtonText (2),
                                              options.getIconType(),
                                              options.getNumButtons(),
                                              component);

                if (alert == nullptr)
                {
                    // You have to return an alert box!
                    jassertfalse;
                    return nullptr;
                }

                if (auto* parent = options.getParentComponent())
                {
                    parent->addAndMakeVisible (alert);

                    if (options.getAssociatedComponent() == nullptr)
                        alert->setCentrePosition (parent->getLocalBounds().getCentre());
                }

                alert->setAlwaysOnTop (WindowUtils::areThereAnyAlwaysOnTopWindows());

                return alert;
            }

            const MessageBoxOptions options;
            Component::SafePointer<AlertWindow> alert;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AlertWindowImpl)
        };

        return std::make_unique<AlertWindowImpl> (opts);
    }
};

} // namespace juce::detail
