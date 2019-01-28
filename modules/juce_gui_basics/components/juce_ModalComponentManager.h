/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    Manages the system's stack of modal components.

    Normally you'll just use the Component methods to invoke modal states in components,
    and won't have to deal with this class directly, but this is the singleton object that's
    used internally to manage the stack.

    @see Component::enterModalState, Component::exitModalState, Component::isCurrentlyModal,
         Component::getCurrentlyModalComponent, Component::isCurrentlyBlockedByAnotherModalComponent

    @tags{GUI}
*/
class JUCE_API  ModalComponentManager   : private AsyncUpdater,
                                          private DeletedAtShutdown
{
public:
    //==============================================================================
    /** Receives callbacks when a modal component is dismissed.

        You can register a callback using Component::enterModalState() or
        ModalComponentManager::attachCallback().

        For some quick ways of creating callback objects, see the ModalCallbackFunction class.
        @see ModalCallbackFunction
    */
    class JUCE_API  Callback
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
   #ifndef DOXYGEN
    JUCE_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL (ModalComponentManager)
   #endif

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
    bool isModal (const Component* component) const;

    /** Returns true if the specified component is currently the topmost modal component. */
    bool isFrontModalComponent (const Component* component) const;

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
    void bringModalComponentsToFront (bool topOneShouldGrabFocus = true);

    /** Calls exitModalState (0) on any components that are currently modal.
        @returns true if any components were modal; false if nothing needed cancelling
    */
    bool cancelAllModalComponents();

   #if JUCE_MODAL_LOOPS_PERMITTED
    /** Runs the event loop until the currently topmost modal component is dismissed, and
        returns the exit code for that component.
    */
    int runEventLoopForCurrentComponent();
   #endif

protected:
    /** Creates a ModalComponentManager.
        You shouldn't ever call the constructor - it's a singleton, so use ModalComponentManager::getInstance()
    */
    ModalComponentManager();

    /** Destructor. */
    ~ModalComponentManager() override;

    /** @internal */
    void handleAsyncUpdate() override;

private:
    //==============================================================================
    friend class Component;

    struct ModalItem;
    OwnedArray<ModalItem> stack;

    void startModal (Component*, bool autoDelete);
    void endModal (Component*, int returnValue);
    void endModal (Component*);

    JUCE_DECLARE_NON_COPYABLE (ModalComponentManager)
};

//==============================================================================
/**
    This class provides some handy utility methods for creating ModalComponentManager::Callback
    objects that will invoke a static function with some parameters when a modal component is dismissed.

    @tags{GUI}
*/
class JUCE_API ModalCallbackFunction
{
public:
    /** This is a utility function to create a ModalComponentManager::Callback that will
        call a lambda function.
        The lambda that you supply must take an integer parameter, which is the result code that
        was returned when the modal component was dismissed.

        @see ModalComponentManager::Callback
    */
    static ModalComponentManager::Callback* create (std::function<void(int)>);

    //==============================================================================
    /** This is a utility function to create a ModalComponentManager::Callback that will
        call a static function with a parameter.

        The function that you supply must take two parameters - the first being an int, which is
        the result code that was used when the modal component was dismissed, and the second
        can be a custom type. Note that this custom value will be copied and stored, so it must
        be a primitive type or a class that provides copy-by-value semantics.

        E.g. @code
        static void myCallbackFunction (int modalResult, double customValue)
        {
            if (modalResult == 1)
                doSomethingWith (customValue);
        }

        Component* someKindOfComp;
        ...
        someKindOfComp->enterModalState (ModalCallbackFunction::create (myCallbackFunction, 3.0));
        @endcode
        @see ModalComponentManager::Callback
    */
    template <typename ParamType>
    static ModalComponentManager::Callback* create (void (*functionToCall) (int, ParamType),
                                                    ParamType parameterValue)
    {
        return create ([=] (int r) { functionToCall (r, parameterValue); });
    }

    //==============================================================================
    /** This is a utility function to create a ModalComponentManager::Callback that will
        call a static function with two custom parameters.

        The function that you supply must take three parameters - the first being an int, which is
        the result code that was used when the modal component was dismissed, and the next two are
        your custom types. Note that these custom values will be copied and stored, so they must
        be primitive types or classes that provide copy-by-value semantics.

        E.g. @code
        static void myCallbackFunction (int modalResult, double customValue1, String customValue2)
        {
            if (modalResult == 1)
                doSomethingWith (customValue1, customValue2);
        }

        Component* someKindOfComp;
        ...
        someKindOfComp->enterModalState (ModalCallbackFunction::create (myCallbackFunction, 3.0, String ("xyz")));
        @endcode
        @see ModalComponentManager::Callback
    */
    template <typename ParamType1, typename ParamType2>
    static ModalComponentManager::Callback* withParam (void (*functionToCall) (int, ParamType1, ParamType2),
                                                       ParamType1 parameterValue1,
                                                       ParamType2 parameterValue2)
    {
        return create ([=] (int r) { functionToCall (r, parameterValue1, parameterValue2); });
    }

    //==============================================================================
    /** This is a utility function to create a ModalComponentManager::Callback that will
        call a static function with a component.

        The function that you supply must take two parameters - the first being an int, which is
        the result code that was used when the modal component was dismissed, and the second
        can be a Component class. The component will be stored as a WeakReference, so that if it gets
        deleted before this callback is invoked, the pointer that is passed to the function will be null.

        E.g. @code
        static void myCallbackFunction (int modalResult, Slider* mySlider)
        {
            if (modalResult == 1 && mySlider != nullptr) // (must check that mySlider isn't null in case it was deleted..)
                mySlider->setValue (0.0);
        }

        Component* someKindOfComp;
        Slider* mySlider;
        ...
        someKindOfComp->enterModalState (ModalCallbackFunction::forComponent (myCallbackFunction, mySlider));
        @endcode
        @see ModalComponentManager::Callback
    */
    template <class ComponentType>
    static ModalComponentManager::Callback* forComponent (void (*functionToCall) (int, ComponentType*),
                                                          ComponentType* component)
    {
        WeakReference<Component> comp (component);
        return create ([=] (int r) { functionToCall (r, static_cast<ComponentType*> (comp.get())); });
    }

    //==============================================================================
    /** Creates a ModalComponentManager::Callback that will call a static function with a component.

        The function that you supply must take three parameters - the first being an int, which is
        the result code that was used when the modal component was dismissed, the second being a Component
        class, and the third being a custom type (which must be a primitive type or have copy-by-value semantics).
        The component will be stored as a WeakReference, so that if it gets deleted before this callback is
        invoked, the pointer that is passed into the function will be null.

        E.g. @code
        static void myCallbackFunction (int modalResult, Slider* mySlider, String customParam)
        {
            if (modalResult == 1 && mySlider != nullptr) // (must check that mySlider isn't null in case it was deleted..)
                mySlider->setName (customParam);
        }

        Component* someKindOfComp;
        Slider* mySlider;
        ...
        someKindOfComp->enterModalState (ModalCallbackFunction::forComponent (myCallbackFunction, mySlider, String ("hello")));
        @endcode
        @see ModalComponentManager::Callback
    */
    template <class ComponentType, typename ParamType>
    static ModalComponentManager::Callback* forComponent (void (*functionToCall) (int, ComponentType*, ParamType),
                                                          ComponentType* component,
                                                          ParamType param)
    {
        WeakReference<Component> comp (component);
        return create ([=] (int r) { functionToCall (r, static_cast<ComponentType*> (comp.get()), param); });
    }

private:
    ModalCallbackFunction() = delete;
    ~ModalCallbackFunction() = delete;
};

} // namespace juce
