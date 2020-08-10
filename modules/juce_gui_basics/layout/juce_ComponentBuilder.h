/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
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

//==============================================================================
/**
    Loads and maintains a tree of Components from a ValueTree that represents them.

    To allow the state of a tree of components to be saved as a ValueTree and re-loaded,
    this class lets you register a set of type-handlers for the different components that
    are involved, and then uses these types to re-create a set of components from its
    stored state.

    Essentially, to use this, you need to create a ComponentBuilder with your ValueTree,
    then use registerTypeHandler() to give it a set of type handlers that can cope with
    all the items in your tree. Then you can call getComponent() to build the component.
    Once you've got the component you can either take it and delete the ComponentBuilder
    object, or if you keep the ComponentBuilder around, it'll monitor any changes in the
    ValueTree and automatically update the component to reflect these changes.

    @tags{GUI}
*/
class JUCE_API  ComponentBuilder  : private ValueTree::Listener
{
public:
    /** Creates a ComponentBuilder that will use the given state.
        Once you've created your builder, you should use registerTypeHandler() to register some
        type handlers for it, and then you can call createComponent() or getManagedComponent()
        to get the actual component.
    */
    explicit ComponentBuilder (const ValueTree& state);

    /** Creates a builder that doesn't have a state object. */
    ComponentBuilder();

    /** Destructor. */
    ~ComponentBuilder() override;

    /** This is the ValueTree data object that the builder is working with. */
    ValueTree state;

    //==============================================================================
    /** Returns the builder's component (creating it if necessary).

        The first time that this method is called, the builder will attempt to create a component
        from the ValueTree, so you must have registered some suitable type handlers before calling
        this. If there's a problem and the component can't be created, this method returns nullptr.

        The component that is returned is owned by this ComponentBuilder, so you can put it inside
        your own parent components, but don't delete it! The ComponentBuilder will delete it automatically
        when the builder is destroyed. If you want to get a component that you can delete yourself,
        call createComponent() instead.

        The ComponentBuilder will update this component if any changes are made to the ValueTree, so if
        there's a chance that the tree might change, be careful not to keep any pointers to sub-components,
        as they may be changed or removed.
    */
    Component* getManagedComponent();

    /** Creates and returns a new instance of the component that the ValueTree represents.
        The caller is responsible for using and deleting the object that is returned. Unlike
        getManagedComponent(), the component that is returned will not be updated by the builder.
    */
    Component* createComponent();

    //==============================================================================
    /**
        The class is a base class for objects that manage the loading of a type of component
        from a ValueTree.

        To store and re-load a tree of components as a ValueTree, each component type must have
        a TypeHandler to represent it.

        @see ComponentBuilder::registerTypeHandler(), Drawable::registerDrawableTypeHandlers()
    */
    class JUCE_API  TypeHandler
    {
    public:
        //==============================================================================
        /** Creates a TypeHandler.
            The valueTreeType must be the type name of the ValueTrees that this handler can parse.
        */
        explicit TypeHandler (const Identifier& valueTreeType);

        /** Destructor. */
        virtual ~TypeHandler();

        /** Returns the type of the ValueTrees that this handler can parse. */
        const Identifier type;

        /** Returns the builder that this type is registered with. */
        ComponentBuilder* getBuilder() const noexcept;

        //==============================================================================
        /** This method must create a new component from the given state, add it to the specified
            parent component (which may be null), and return it.

            The ValueTree will have been pre-checked to make sure that its type matches the type
            that this handler supports.

            There's no need to set the new Component's ID to match that of the state - the builder
            will take care of that itself.
        */
        virtual Component* addNewComponentFromState (const ValueTree& state, Component* parent) = 0;

        /** This method must update an existing component from a new ValueTree state.

            A component that has been created with addNewComponentFromState() may need to be updated
            if the ValueTree changes, so this method is used to do that. Your implementation must do
            whatever's necessary to update the component from the new state provided.

            The ValueTree will have been pre-checked to make sure that its type matches the type
            that this handler supports, and the component will have been created by this type's
            addNewComponentFromState() method.
        */
        virtual void updateComponentFromState (Component* component, const ValueTree& state) = 0;

    private:
        //==============================================================================
        friend class ComponentBuilder;
        ComponentBuilder* builder;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TypeHandler)
    };

    //==============================================================================
    /** Adds a type handler that the builder can use when trying to load components.
        @see Drawable::registerDrawableTypeHandlers()
    */
    void registerTypeHandler (TypeHandler* type);

    /** Tries to find a registered type handler that can load a component from the given ValueTree. */
    TypeHandler* getHandlerForState (const ValueTree& state) const;

    /** Returns the number of registered type handlers.
        @see getHandler, registerTypeHandler
    */
    int getNumHandlers() const noexcept;

    /** Returns one of the registered type handlers.
        @see getNumHandlers, registerTypeHandler
    */
    TypeHandler* getHandler (int index) const noexcept;

    /** Registers handlers for various standard juce components. */
    void registerStandardComponentTypes();

    //==============================================================================
    /** This class is used when references to images need to be stored in ValueTrees.

        An instance of an ImageProvider provides a mechanism for converting an Image to/from
        a reference, which may be a file, URL, ID string, or whatever system is appropriate in
        your app.

        When you're loading components from a ValueTree that may need a way of loading images, you
        should call ComponentBuilder::setImageProvider() to supply a suitable provider before
        trying to load the component.

        @see ComponentBuilder::setImageProvider()
    */
    class JUCE_API  ImageProvider
    {
    public:
        ImageProvider() = default;
        virtual ~ImageProvider() = default;

        /** Retrieves the image associated with this identifier, which could be any
            kind of string, number, filename, etc.

            The image that is returned will be owned by the caller, but it may come
            from the ImageCache.
        */
        virtual Image getImageForIdentifier (const var& imageIdentifier) = 0;

        /** Returns an identifier to be used to refer to a given image.
            This is used when a reference to an image is stored in a ValueTree.
        */
        virtual var getIdentifierForImage (const Image& image) = 0;
    };

    //==============================================================================
    /** Gives the builder an ImageProvider object that the type handlers can use when
        loading images from stored references.

        The object that is passed in is not owned by the builder, so the caller must delete
        it when it is no longer needed, but not while the builder may still be using it. To
        clear the image provider, just call setImageProvider (nullptr).
    */
    void setImageProvider (ImageProvider* newImageProvider) noexcept;

    /** Returns the current image provider that this builder is using, or nullptr if none has been set. */
    ImageProvider* getImageProvider() const noexcept;

    //==============================================================================
    /** Updates the children of a parent component by updating them from the children of
        a given ValueTree.
    */
    void updateChildComponents (Component& parent, const ValueTree& children);

    /** An identifier for the property of the ValueTrees that is used to store a unique ID
        for that component.
    */
    static const Identifier idProperty;

private:
    //==============================================================================
    OwnedArray<TypeHandler> types;
    std::unique_ptr<Component> component;
    ImageProvider* imageProvider;
   #if JUCE_DEBUG
    WeakReference<Component> componentRef;
   #endif

    void valueTreePropertyChanged (ValueTree&, const Identifier&) override;
    void valueTreeChildAdded (ValueTree&, ValueTree&) override;
    void valueTreeChildRemoved (ValueTree&, ValueTree&, int) override;
    void valueTreeChildOrderChanged (ValueTree&, int, int) override;
    void valueTreeParentChanged (ValueTree&) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentBuilder)
};

} // namespace juce
