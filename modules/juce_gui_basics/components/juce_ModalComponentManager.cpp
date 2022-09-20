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

struct ModalComponentManager::ModalItem  : public ComponentMovementWatcher
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
void ModalComponentManager::startModal (Component* component, bool autoDelete)
{
    if (component != nullptr)
        stack.add (new ModalItem (component, autoDelete));
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

void ModalComponentManager::endModal (Component* component)
{
    for (int i = stack.size(); --i >= 0;)
    {
        auto* item = stack.getUnchecked (i);

        if (item->component == component)
            item->cancel();
    }
}

void ModalComponentManager::endModal (Component* component, int returnValue)
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
        FocusRestorer focusRestorer;
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
