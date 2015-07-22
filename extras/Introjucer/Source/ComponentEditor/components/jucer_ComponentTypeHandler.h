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

#ifndef JUCER_COMPONENTTYPEHANDLER_H_INCLUDED
#define JUCER_COMPONENTTYPEHANDLER_H_INCLUDED

class ComponentOverlayComponent;
class ComponentLayout;
#include "../jucer_GeneratedCode.h"
#include "../ui/jucer_RelativePositionedRectangle.h"


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
    virtual String getXmlTagName() const noexcept             { return className.toUpperCase(); }

    static ComponentTypeHandler* getHandlerForXmlTag (const String& tagName);

    virtual XmlElement* createXmlFor (Component* component, const ComponentLayout* layout);
    virtual bool restoreFromXml (const XmlElement& xml, Component* component, const ComponentLayout* layout);

    virtual void getEditableProperties (Component* component,
                                        JucerDocument& document,
                                        Array<PropertyComponent*>& props);

    virtual void addPropertiesToPropertyPanel (Component* component,
                                               JucerDocument& document,
                                               PropertyPanel& panel);


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

    OwnedArray <ComponentColourInfo> colours;

private:
    JUCE_DECLARE_NON_COPYABLE (ComponentTypeHandler)
};


#endif   // JUCER_COMPONENTTYPEHANDLER_H_INCLUDED
