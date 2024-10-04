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

struct ModalComponentManager::ModalItem final : public ComponentMovementWatcher
{
    ModalItem (Component* comp, bool shouldAutoDelete)
        : ComponentMovementWatcher (comp),
          component (comp), autoDelete (shouldAutoDelete)
    {
        jassert (comp != nullptr);
    }

    ~ModalItem() override
    {
        if (autoDelete)
            std::unique_ptr<Component> componentDeleter (component);
    }

    void componentMovedOrResized (bool, bool) override {}

    using ComponentMovementWatcher::componentMovedOrResized;

    void componentPeerChanged() override
    {
        componentVisibilityChanged();
    }

    void componentVisibilityChanged() override
    {
        if (! component->isShowing())
            cancel();
    }

    using ComponentMovementWatcher::componentVisibilityChanged;

    void componentBeingDeleted (Component& comp) override
    {
        ComponentMovementWatcher::componentBeingDeleted (comp);

        if (component == &comp || comp.isParentOf (component))
        {
            autoDelete = false;
            cancel();
        }
    }

    void cancel()
    {
        if (isActive)
        {
            isActive = false;

            if (auto* mcm = ModalComponentManager::getInstanceWithoutCreating())
                mcm->triggerAsyncUpdate();
        }
    }

    Component* component;
    OwnedArray<Callback> callbacks;
    int returnValue = 0;
    bool isActive = true, autoDelete;

    JUCE_DECLARE_NON_COPYABLE (ModalItem)
};

//==============================================================================
ModalComponentManager::ModalComponentManager()
{
}

ModalComponentManager::~ModalComponentManager()
{
    stack.clear();
    clearSingletonInstance();
}

JUCE_IMPLEMENT_SINGLETON (ModalComponentManager)


//==============================================================================
void ModalComponentManager::startModal (Key, Component* component, bool autoDelete)
{
    if (component != nullptr)
    {
        stack.add (new ModalItem (component, autoDelete));
        detail::ComponentHelpers::ModalComponentManagerChangeNotifier::getInstance().modalComponentManagerChanged();
    }
}

void ModalComponentManager::attachCallback (Component* component, Callback* callback)
{
    if (callback != nullptr)
    {
        std::unique_ptr<Callback> callbackDeleter (callback);

        for (int i = stack.size(); --i >= 0;)
        {
            auto* item = stack.getUnchecked (i);

            if (item->component == component)
            {
                item->callbacks.add (callback);
                callbackDeleter.release();
                break;
            }
        }
    }
}

void ModalComponentManager::endModal (Key, Component* component, int returnValue)
{
    for (int i = stack.size(); --i >= 0;)
    {
        auto* item = stack.getUnchecked (i);

        if (item->component == component)
        {
            item->returnValue = returnValue;
            item->cancel();
        }
    }
}

int ModalComponentManager::getNumModalComponents() const
{
    int n = 0;

    for (auto* item : stack)
        if (item->isActive)
            ++n;

    return n;
}

Component* ModalComponentManager::getModalComponent (int index) const
{
    int n = 0;

    for (int i = stack.size(); --i >= 0;)
    {
        auto* item = stack.getUnchecked (i);

        if (item->isActive)
            if (n++ == index)
                return item->component;
    }

    return nullptr;
}

bool ModalComponentManager::isModal (const Component* comp) const
{
    for (auto* item : stack)
        if (item->isActive && item->component == comp)
            return true;

    return false;
}

bool ModalComponentManager::isFrontModalComponent (const Component* comp) const
{
    return comp == getModalComponent (0);
}

void ModalComponentManager::handleAsyncUpdate()
{
    for (int i = stack.size(); --i >= 0;)
    {
        auto* item = stack.getUnchecked (i);

        if (! item->isActive)
        {
            std::unique_ptr<ModalItem> deleter (stack.removeAndReturn (i));
            Component::SafePointer<Component> compToDelete (item->autoDelete ? item->component : nullptr);

            for (int j = item->callbacks.size(); --j >= 0;)
                item->callbacks.getUnchecked (j)->modalStateFinished (item->returnValue);

            compToDelete.deleteAndZero();

            detail::ComponentHelpers::ModalComponentManagerChangeNotifier::getInstance().modalComponentManagerChanged();
        }
    }
}

void ModalComponentManager::bringModalComponentsToFront (bool topOneShouldGrabFocus)
{
    ComponentPeer* lastOne = nullptr;

    for (int i = 0; i < getNumModalComponents(); ++i)
    {
        auto* c = getModalComponent (i);

        if (c == nullptr)
            break;

        if (auto* peer = c->getPeer())
        {
            if (peer != lastOne)
            {
                if (lastOne == nullptr)
                {
                    peer->toFront (topOneShouldGrabFocus);

                    if (topOneShouldGrabFocus)
                        peer->grabFocus();
                }
                else
                {
                    peer->toBehind (lastOne);
                }

                lastOne = peer;
            }
        }
    }
}

bool ModalComponentManager::cancelAllModalComponents()
{
    auto numModal = getNumModalComponents();

    for (int i = numModal; --i >= 0;)
        if (auto* c = getModalComponent (i))
            c->exitModalState (0);

    return numModal > 0;
}

//==============================================================================
#if JUCE_MODAL_LOOPS_PERMITTED
int ModalComponentManager::runEventLoopForCurrentComponent()
{
    // This can only be run from the message thread!
    JUCE_ASSERT_MESSAGE_THREAD

    int returnValue = 0;

    if (auto* currentlyModal = getModalComponent (0))
    {
        detail::FocusRestorer focusRestorer;
        bool finished = false;

        attachCallback (currentlyModal, ModalCallbackFunction::create ([&] (int r) { returnValue = r; finished = true; }));

        JUCE_TRY
        {
            while (! finished)
            {
                if  (! MessageManager::getInstance()->runDispatchLoopUntil (20))
                    break;
            }
        }
        JUCE_CATCH_EXCEPTION
    }

    return returnValue;
}
#endif

} // namespace juce
