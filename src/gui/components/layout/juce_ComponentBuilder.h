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

#ifndef __JUCE_COMPONENTBUILDER_JUCEHEADER__
#define __JUCE_COMPONENTBUILDER_JUCEHEADER__

#include "../../../containers/juce_ValueTree.h"
#include "../juce_Component.h"


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
*/
class JUCE_API  ComponentBuilder  : public ValueTree::Listener
{
public:
    /**
    */
    explicit ComponentBuilder (const ValueTree& state);

    /** Destructor. */
    ~ComponentBuilder();

    //==============================================================================
    /**
    */
    ValueTree& getState() throw()               { return state; }

    /**
    */
    const ValueTree& getState() const throw()   { return state; }

    /**
    */
    Component* getComponent();

    /**
    */
    Component* getAndReleaseComponent();

    //==============================================================================
    /**
    */
    class JUCE_API  TypeHandler
    {
    public:
        /**
        */
        explicit TypeHandler (const Identifier& valueTreeType);

        /** Destructor. */
        virtual ~TypeHandler();

        /**
        */
        const Identifier& getType() const throw()           { return valueTreeType; }

        /**
        */
        virtual Component* addNewComponentFromState (const ValueTree& state, Component* parent) = 0;

        /**
        */
        virtual void updateComponentFromState (Component* component, const ValueTree& state) = 0;

        /**
        */
        ComponentBuilder* getBuilder() const throw();

    private:
        friend class ComponentBuilder;
        ComponentBuilder* builder;
        const Identifier valueTreeType;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TypeHandler);
    };

    /**
    */
    void registerTypeHandler (TypeHandler* type);

    /**
    */
    TypeHandler* getHandlerForState (const ValueTree& state) const;

    /**
    */
    int getNumHandlers() const throw();

    /**
    */
    TypeHandler* getHandler (int index) const throw();

    //=============================================================================
    /** This class is used when loading Drawables that contain images, and retrieves
        the image for a stored identifier.
        @see Drawable::createFromValueTree
    */
    class JUCE_API  ImageProvider
    {
    public:
        ImageProvider() {}
        virtual ~ImageProvider() {}

        /** Retrieves the image associated with this identifier, which could be any
            kind of string, number, filename, etc.

            The image that is returned will be owned by the caller, but it may come
            from the ImageCache.
        */
        virtual const Image getImageForIdentifier (const var& imageIdentifier) = 0;

        /** Returns an identifier to be used to refer to a given image.
            This is used when converting a drawable into a ValueTree, so if you're
            only loading drawables, you can just return a var::null here.
        */
        virtual const var getIdentifierForImage (const Image& image) = 0;
    };

    /** */
    void setImageProvider (ImageProvider* newImageProvider) throw();

    /** */
    ImageProvider* getImageProvider() const throw();

    //=============================================================================
    /** @internal */
    void valueTreePropertyChanged (ValueTree& treeWhosePropertyHasChanged, const Identifier& property);
    /** @internal */
    void valueTreeChildrenChanged (ValueTree& treeWhoseChildHasChanged);
    /** @internal */
    void valueTreeParentChanged (ValueTree& treeWhoseParentHasChanged);

    /**
    */
    void updateChildComponents (Component& parent, const ValueTree& children);

    /**
    */
    static const Identifier idProperty;

private:
    //=============================================================================
    ValueTree state;
    OwnedArray <TypeHandler> types;
    ScopedPointer<Component> component;
    ImageProvider* imageProvider;

    void updateComponent (const ValueTree& state);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentBuilder);
};


#endif   // __JUCE_COMPONENTBUILDER_JUCEHEADER__
