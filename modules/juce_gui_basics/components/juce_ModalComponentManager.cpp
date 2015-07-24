/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

class ModalComponentManager::ModalItem  : public ComponentMovementWatcher
{
public:
    ModalItem (Component* const comp, const bool autoDelete_)
        : ComponentMovementWatcher (comp),
          component (comp), returnValue (0),
          isActive (true), autoDelete (autoDelete_)
    {
        jassert (comp != nullptr);
    }

    void componentMovedOrResized (bool, bool) override {}

    void componentPeerChanged() override
    {
        if (! component->isShowing())
            cancel();
    }

    void componentVisibilityChanged() override
    {
        if (! component->isShowing())
            cancel();
    }

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

            if (ModalComponentManager* mcm = ModalComponentManager::getInstanceWithoutCreating())
                mcm->triggerAsyncUpdate();
        }
    }

    Component* component;
    OwnedArray<Callback> callbacks;
    int returnValue;
    bool isActive, autoDelete;

private:
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

juce_ImplementSingleton_SingleThreaded (ModalComponentManager)


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
        ScopedPointer<Callback> callbackDeleter (callback);

        for (int i = stack.size(); --i >= 0;)
        {
            ModalItem* const item = stack.getUnchecked(i);

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
        ModalItem* const item = stack.getUnchecked(i);

        if (item->component == component)
            item->cancel();
    }
}

void ModalComponentManager::endModal (Component* component, int returnValue)
{
    for (int i = stack.size(); --i >= 0;)
    {
        ModalItem* const item = stack.getUnchecked(i);

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
    for (int i = 0; i < stack.size(); ++i)
        if (stack.getUnchecked(i)->isActive)
            ++n;

    return n;
}

Component* ModalComponentManager::getModalComponent (const int index) const
{
    int n = 0;
    for (int i = stack.size(); --i >= 0;)
    {
        const ModalItem* const item = stack.getUnchecked(i);
        if (item->isActive)
            if (n++ == index)
                return item->component;
    }

    return nullptr;
}

bool ModalComponentManager::isModal (Component* const comp) const
{
    for (int i = stack.size(); --i >= 0;)
    {
        const ModalItem* const item = stack.getUnchecked(i);
        if (item->isActive && item->component == comp)
            return true;
    }

    return false;
}

bool ModalComponentManager::isFrontModalComponent (Component* const comp) const
{
    return comp == getModalComponent (0);
}

void ModalComponentManager::handleAsyncUpdate()
{
    for (int i = stack.size(); --i >= 0;)
    {
        const ModalItem* const item = stack.getUnchecked(i);

        if (! item->isActive)
        {
            ScopedPointer<ModalItem> deleter (stack.removeAndReturn (i));
            Component::SafePointer<Component> compToDelete (item->autoDelete ? item->component : nullptr);

            for (int j = item->callbacks.size(); --j >= 0;)
                item->callbacks.getUnchecked(j)->modalStateFinished (item->returnValue);

            compToDelete.deleteAndZero();
        }
    }
}

void ModalComponentManager::bringModalComponentsToFront (bool topOneShouldGrabFocus)
{
    ComponentPeer* lastOne = nullptr;

    for (int i = 0; i < getNumModalComponents(); ++i)
    {
        Component* const c = getModalComponent (i);

        if (c == nullptr)
            break;

        ComponentPeer* peer = c->getPeer();

        if (peer != nullptr && peer != lastOne)
        {
            if (lastOne == nullptr)
            {
                peer->toFront (topOneShouldGrabFocus);

                if (topOneShouldGrabFocus)
                    peer->grabFocus();
            }
            else
                peer->toBehind (lastOne);

            lastOne = peer;
        }
    }
}

bool ModalComponentManager::cancelAllModalComponents()
{
    const int numModal = getNumModalComponents();

    for (int i = numModal; --i >= 0;)
        if (Component* const c = getModalComponent(i))
            c->exitModalState (0);

    return numModal > 0;
}

#if JUCE_MODAL_LOOPS_PERMITTED
class ModalComponentManager::ReturnValueRetriever     : public ModalComponentManager::Callback
{
public:
    ReturnValueRetriever (int& v, bool& done) : value (v), finished (done) {}

    void modalStateFinished (int returnValue)
    {
        finished = true;
        value = returnValue;
    }

private:
    int& value;
    bool& finished;

    JUCE_DECLARE_NON_COPYABLE (ReturnValueRetriever)
};

int ModalComponentManager::runEventLoopForCurrentComponent()
{
    // This can only be run from the message thread!
    jassert (MessageManager::getInstance()->isThisTheMessageThread());

    int returnValue = 0;

    if (Component* currentlyModal = getModalComponent (0))
    {
        FocusRestorer focusRestorer;

        bool finished = false;
        attachCallback (currentlyModal, new ReturnValueRetriever (returnValue, finished));

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
