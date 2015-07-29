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

#ifndef JUCE_MODALCOMPONENTMANAGER_H_INCLUDED
#define JUCE_MODALCOMPONENTMANAGER_H_INCLUDED


//==============================================================================
/**
    Manages the system's stack of modal components.

    Normally you'll just use the Component methods to invoke modal states in components,
    and won't have to deal with this class directly, but this is the singleton object that's
    used internally to manage the stack.

    @see Component::enterModalState, Component::exitModalState, Component::isCurrentlyModal,
         Component::getCurrentlyModalComponent, Component::isCurrentlyBlockedByAnotherModalComponent
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
    juce_DeclareSingleton_SingleThreaded_Minimal (ModalComponentManager)

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
    ~ModalComponentManager();

    /** @internal */
    void handleAsyncUpdate() override;

private:
    //==============================================================================
    class ModalItem;
    class ReturnValueRetriever;

    friend class Component;
    friend struct ContainerDeletePolicy<ModalItem>;
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
*/
class ModalCallbackFunction
{
public:
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
        return new FunctionCaller1 <ParamType> (functionToCall, parameterValue);
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
        return new FunctionCaller2 <ParamType1, ParamType2> (functionToCall, parameterValue1, parameterValue2);
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
        return new ComponentCaller1 <ComponentType> (functionToCall, component);
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
        return new ComponentCaller2 <ComponentType, ParamType> (functionToCall, component, param);
    }

private:
    //==============================================================================
    template <typename ParamType>
    class FunctionCaller1  : public ModalComponentManager::Callback
    {
    public:
        typedef void (*FunctionType) (int, ParamType);

        FunctionCaller1 (FunctionType& f, ParamType& p1)
            : function (f), param (p1) {}

        void modalStateFinished (int returnValue)  { function (returnValue, param); }

    private:
        const FunctionType function;
        ParamType param;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FunctionCaller1)
    };

    template <typename ParamType1, typename ParamType2>
    class FunctionCaller2  : public ModalComponentManager::Callback
    {
    public:
        typedef void (*FunctionType) (int, ParamType1, ParamType2);

        FunctionCaller2 (FunctionType& f, ParamType1& p1, ParamType2& p2)
            : function (f), param1 (p1), param2 (p2) {}

        void modalStateFinished (int returnValue)   { function (returnValue, param1, param2); }

    private:
        const FunctionType function;
        ParamType1 param1;
        ParamType2 param2;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FunctionCaller2)
    };

    template <typename ComponentType>
    class ComponentCaller1  : public ModalComponentManager::Callback
    {
    public:
        typedef void (*FunctionType) (int, ComponentType*);

        ComponentCaller1 (FunctionType& f, ComponentType* c)
            : function (f), comp (c) {}

        void modalStateFinished (int returnValue)
        {
            function (returnValue, static_cast <ComponentType*> (comp.get()));
        }

    private:
        const FunctionType function;
        WeakReference<Component> comp;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentCaller1)
    };

    template <typename ComponentType, typename ParamType1>
    class ComponentCaller2  : public ModalComponentManager::Callback
    {
    public:
        typedef void (*FunctionType) (int, ComponentType*, ParamType1);

        ComponentCaller2 (FunctionType& f, ComponentType* c, ParamType1 p1)
            : function (f), comp (c), param1 (p1) {}

        void modalStateFinished (int returnValue)
        {
            function (returnValue, static_cast <ComponentType*> (comp.get()), param1);
        }

    private:
        const FunctionType function;
        WeakReference<Component> comp;
        ParamType1 param1;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentCaller2)
    };

    ModalCallbackFunction();
    ~ModalCallbackFunction();
    JUCE_DECLARE_NON_COPYABLE (ModalCallbackFunction)
};


#endif   // JUCE_MODALCOMPONENTMANAGER_H_INCLUDED
