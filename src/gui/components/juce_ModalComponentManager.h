/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_MODALCOMPONENTMANAGER_JUCEHEADER__
#define __JUCE_MODALCOMPONENTMANAGER_JUCEHEADER__

#include "../../core/juce_Singleton.h"
#include "../../events/juce_AsyncUpdater.h"
#include "../../utilities/juce_DeletedAtShutdown.h"


//==============================================================================
/**
    Manages the system's stack of modal components.

    Normally you'll just use the Component methods to invoke modal states in components,
    and won't have to deal with this class directly, but this is the singleton object that's
    used internally to manage the stack.

    @see Component::enterModalState, Component::exitModalState, Component::isCurrentlyModal,
         Component::getCurrentlyModalComponent, Component::isCurrentlyBlockedByAnotherModalComponent
*/
class JUCE_API  ModalComponentManager   : public AsyncUpdater,
                                          public DeletedAtShutdown
{
public:
    //==============================================================================
    /** Receives callbacks when a modal component is dismissed.

        You can register a callback using Component::enterModalState() or
        ModalComponentManager::attachCallback().
    */
    class Callback
    {
    public:
        /** */
        Callback() {}

        /** Destructor. */
        virtual ~Callback() {}

        /** Called to indicate that a modal component has been dismissed.

            You can register a callback using Component::enterModalState() or
            ModalComponentManager::attachCallback().

            The returnValue parameter is the value that was passed to Component::exitModalState()
            when the component was dismissed.

            The callback object will be deleted shortly after this method is called.
        */
        virtual void modalStateFinished (int returnValue) = 0;
    };

    //==============================================================================
    /** Returns the number of components currently being shown modally.
        @see getModalComponent
    */
    int getNumModalComponents() const;

    /** Returns one of the components being shown modally.
        An index of 0 is the most recently-shown, topmost component.
    */
    Component* getModalComponent (int index) const;

    /** Returns true if the specified component is in a modal state. */
    bool isModal (Component* component) const;

    /** Returns true if the specified component is currently the topmost modal component. */
    bool isFrontModalComponent (Component* component) const;

    /** Adds a new callback that will be called when the specified modal component is dismissed.

        If the component is modal, then when it is dismissed, either by being hidden, or by calling
        Component::exitModalState(), then the Callback::modalStateFinished() method will be
        called.

        Each component can have any number of callbacks associated with it, and this one is added
        to that list.

        The object that is passed in will be deleted by the manager when it's no longer needed. If
        the given component is not currently modal, the callback object is deleted immediately and
        no action is taken.
    */
    void attachCallback (Component* component, Callback* callback);

    /** Brings any modal components to the front. */
    void bringModalComponentsToFront();

    /** Runs the event loop until the currently topmost modal component is dismissed, and
        returns the exit code for that component.
    */
    int runEventLoopForCurrentComponent();

    //==============================================================================
    juce_DeclareSingleton_SingleThreaded_Minimal (ModalComponentManager);

protected:
    /** Creates a ModalComponentManager.
        You shouldn't ever call the constructor - it's a singleton, so use ModalComponentManager::getInstance()
    */
    ModalComponentManager();

    /** Destructor. */
    ~ModalComponentManager();

    /** @internal */
    void handleAsyncUpdate();

private:
    class ModalItem;
    class ReturnValueRetriever;

    friend class Component;
    friend class OwnedArray <ModalItem>;
    OwnedArray <ModalItem> stack;

    void startModal (Component* component, Callback* callback);
    void endModal (Component* component, int returnValue);
    void endModal (Component* component);

    JUCE_DECLARE_NON_COPYABLE (ModalComponentManager);
};


#endif   // __JUCE_MODALCOMPONENTMANAGER_JUCEHEADER__
