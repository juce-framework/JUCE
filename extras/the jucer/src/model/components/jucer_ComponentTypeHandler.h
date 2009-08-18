/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCER_COMPONENTTYPEHANDLER_JUCEHEADER__
#define __JUCER_COMPONENTTYPEHANDLER_JUCEHEADER__

class ComponentOverlayComponent;
class ComponentLayout;
#include "../jucer_GeneratedCode.h"


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

    virtual ~ComponentTypeHandler();

    //==============================================================================
    virtual bool canHandle (Component& component) const;

    static ComponentTypeHandler* getHandlerFor (Component& component);

    //==============================================================================
    virtual const String getXmlTagName() const throw()              { return className.toUpperCase(); }

    static ComponentTypeHandler* getHandlerForXmlTag (const String& tagName);

    virtual XmlElement* createXmlFor (Component* component, const ComponentLayout* layout);
    virtual bool restoreFromXml (const XmlElement& xml, Component* component, const ComponentLayout* layout);

    virtual void getEditableProperties (Component* component,
                                        JucerDocument& document,
                                        Array <PropertyComponent*>& properties);

    virtual void addPropertiesToPropertyPanel (Component* component,
                                               JucerDocument& document,
                                               PropertyPanel& panel);


    void registerEditableColour (int colourId,
                                 const String& colourIdCode,
                                 const String& colourName,
                                 const String& xmlTagName);

    #define registerColour(colourId, colourName, xmlTagName)   \
        registerEditableColour (colourId, #colourId, T(colourName), T(xmlTagName))

    void addColourProperties (Component* component,
                              JucerDocument& document,
                              Array <PropertyComponent*>& properties);

    const String getColourIntialisationCode (Component* component,
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

    virtual void fillInMemberVariableDeclarations (GeneratedCode& code, Component* component, const String& memberVariableName);
    virtual void fillInResizeCode (GeneratedCode& code, Component* component, const String& memberVariableName);
    virtual void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName);
    virtual const String getCreationParameters (Component* component);
    virtual void fillInDeletionCode (GeneratedCode& code, Component* component, const String& memberVariableName);

    //==============================================================================
    const String& getTypeName() const throw()                       { return typeName; }
    virtual const String getClassName (Component*) const            { return className; }

    int getDefaultWidth() const throw()                             { return defaultWidth; }
    int getDefaultHeight() const throw()                            { return defaultHeight; }

    static int64 getComponentId (Component* comp);
    static void setComponentId (Component* comp, const int64 newID);

    static const RelativePositionedRectangle getComponentPosition (Component* comp);
    static void setComponentPosition (Component* comp,
                                      const RelativePositionedRectangle& newPos,
                                      const ComponentLayout* layout);

    static JucerDocument* findParentDocument (Component* component);

    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    //==============================================================================
    const String typeName, className, virtualClass, componentClassRawName;
    int defaultWidth, defaultHeight;

    struct ComponentColourInfo
    {
        int colourId;
        String colourIdCode, colourName, xmlTagName;
    };

    OwnedArray <ComponentColourInfo> colours;

private:
    ComponentTypeHandler (const ComponentTypeHandler&);
    const ComponentTypeHandler& operator= (const ComponentTypeHandler&);
};


#endif   // __JUCER_COMPONENTTYPEHANDLER_JUCEHEADER__
