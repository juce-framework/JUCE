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

#pragma once

class ComponentOverlayComponent;
class ComponentLayout;
#include "../jucer_GeneratedCode.h"
#include "../UI/jucer_RelativePositionedRectangle.h"

//==============================================================================
/**
    Base class for handlers that can understand the properties of all the component classes.
*/
class ComponentTypeHandler
{
public:
    //==============================================================================
    ComponentTypeHandler (const String& typeDescription_,
                          const String& className_,
                          const std::type_info& componentClass,
                          const int defaultWidth_,
                          const int defaultHeight_);

    virtual ~ComponentTypeHandler() {}

    //==============================================================================
    virtual bool canHandle (Component& component) const;

    static ComponentTypeHandler* getHandlerFor (Component& component);

    //==============================================================================
    virtual String getXmlTagName() const noexcept
    {
        if (className.startsWith ("juce::"))
            return className.substring (6).toUpperCase();

        return className.toUpperCase();
    }

    static ComponentTypeHandler* getHandlerForXmlTag (const String& tagName);

    virtual XmlElement* createXmlFor (Component* component, const ComponentLayout* layout);
    virtual bool restoreFromXml (const XmlElement& xml, Component* component, const ComponentLayout* layout);

    virtual void getEditableProperties (Component* component,
                                        JucerDocument& document,
                                        Array<PropertyComponent*>& props,
                                        bool multipleSelected);

    virtual void addPropertiesToPropertyPanel (Component* component,
                                               JucerDocument& document,
                                               PropertyPanel& panel,
                                               bool multipleSelected);


    void registerEditableColour (int colourId,
                                 const String& colourIdCode,
                                 const String& colourName,
                                 const String& xmlTagName);

    #define registerColour(colourId, colourName, xmlTagName)   \
        registerEditableColour (colourId, #colourId, colourName, xmlTagName)

    void addColourProperties (Component* component,
                              JucerDocument& document,
                              Array<PropertyComponent*>& props);

    String getColourIntialisationCode (Component* component,
                                       const String& objectName);

    //==============================================================================
    virtual Component* createNewComponent (JucerDocument*) = 0;

    virtual Component* createCopyOf (JucerDocument*, Component& existing);

    virtual ComponentOverlayComponent* createOverlayComponent (Component* child, ComponentLayout& layout);

    virtual void showPopupMenu (Component* component,
                                ComponentLayout& layout);

    //==============================================================================
    // Code-generation methods:

    virtual void fillInGeneratedCode (Component* component, GeneratedCode& code);

    virtual void fillInMemberVariableDeclarations (GeneratedCode&, Component*, const String& memberVariableName);
    virtual void fillInResizeCode (GeneratedCode&, Component*, const String& memberVariableName);
    virtual void fillInCreationCode (GeneratedCode&, Component*, const String& memberVariableName);
    virtual String getCreationParameters (GeneratedCode&, Component*);
    virtual void fillInDeletionCode (GeneratedCode&, Component*, const String& memberVariableName);

    //==============================================================================
    const String& getTypeName() const noexcept                { return typeName; }
    virtual String getClassName (Component*) const            { return className; }

    int getDefaultWidth() const noexcept                      { return defaultWidth; }
    int getDefaultHeight() const noexcept                     { return defaultHeight; }

    static int64 getComponentId (Component* comp);
    static void setComponentId (Component* comp, const int64 newID);

    static RelativePositionedRectangle getComponentPosition (Component* comp);
    static void setComponentPosition (Component* comp,
                                      const RelativePositionedRectangle& newPos,
                                      const ComponentLayout* layout);

    static JucerDocument* findParentDocument (Component* component);

protected:
    //==============================================================================
    const String typeName, className, virtualClass, componentClassRawName;
    int defaultWidth, defaultHeight;

    struct ComponentColourInfo
    {
        int colourId;
        String colourIdCode, colourName, xmlTagName;
    };

    OwnedArray<ComponentColourInfo> colours;

private:
    JUCE_DECLARE_NON_COPYABLE (ComponentTypeHandler)
};
