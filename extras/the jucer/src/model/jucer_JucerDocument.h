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

#ifndef __JUCER_JUCERDOCUMENT_JUCEHEADER__
#define __JUCER_JUCERDOCUMENT_JUCEHEADER__

#include "components/jucer_ComponentTypeHandler.h"
#include "jucer_PaintRoutine.h"
#include "jucer_ComponentLayout.h"
#include "jucer_BinaryResources.h"


//==============================================================================
/**
    This is the main document base class, which loads, saves and manages the state of
    an entire jucer component.

    It contains a ComponentLayout object to represent its sub-components, and
    one or more PaintRoutine objects to represent sets of drawing operations it
    might want to do.

    It also holds a BinaryResources object to manage its resources.
*/
class JucerDocument  : public FileBasedDocument,
                       private Timer
{
public:
    //==============================================================================
    JucerDocument();
    ~JucerDocument();

    void changed();

    virtual JucerDocument* createCopy() = 0;
    virtual const String getTypeName() const = 0;

    //==============================================================================
    UndoManager& getUndoManager() throw()                                   { return undoManager; }

    bool perform (UndoableAction* const action, const String& actionName);

    void refreshAllPropertyComps();

    //==============================================================================
    const String& getClassName() const throw()                              { return className; }
    void setClassName (const String& newName);

    const String& getComponentName() const throw()                          { return componentName; }
    void setComponentName (const String& newName);

    const String getParentClassString() const;
    void setParentClasses (const String& classes);

    const String getConstructorParams() const;
    void setConstructorParams (const String& newParams);

    const String getVariableInitialisers() const;
    void setVariableInitialisers (const String& newInitlialisers);

    void setFixedSize (const bool isFixed);
    bool isFixedSize() const throw()                                        { return fixedSize; }

    void setInitialSize (int w, int h);

    int getInitialWidth() const throw()                                     { return initialWidth; }
    int getInitialHeight() const throw()                                    { return initialHeight; }

    //==============================================================================
    virtual int getNumPaintRoutines() const = 0;
    virtual const StringArray getPaintRoutineNames() const = 0;
    virtual PaintRoutine* getPaintRoutine (const int index) const = 0;

    virtual ComponentLayout* getComponentLayout() const = 0;

    virtual Component* createTestComponent (const bool alwaysFillBackground) = 0;

    virtual void addExtraClassProperties (PropertyPanel* panel);

    //==============================================================================
    virtual void getOptionalMethods (StringArray& baseClasses,
                                     StringArray& returnValues,
                                     StringArray& methods,
                                     StringArray& initialContents) const;

    void setOptionalMethodEnabled (const String& methodSigniture, const bool enable);
    bool isOptionalMethodEnabled (const String& methodSigniture) const throw();

    //==============================================================================
    BinaryResources& getResources() throw()                                 { return resources; }

    //==============================================================================
    void setSnappingGrid (const int numPixels, const bool active, const bool shown);

    int getSnappingGridSize() const throw()                                 { return snapGridPixels; }
    bool isSnapActive (const bool disableIfCtrlKeyDown) const throw();
    bool isSnapShown() const throw()                                        { return snapShown; }

    int snapPosition (int pos) const throw();

    //==============================================================================
    void setComponentOverlayOpacity (const float alpha);
    float getComponentOverlayOpacity() const throw()                  { return componentOverlayOpacity; }

    //==============================================================================
    static const tchar* const jucerCompXmlTag;

    /** Creates the document's metadata xml section.

        This doesn't include resources, which are done separately.
    */
    virtual XmlElement* createXml() const;

    /** Restores the sub-components and graphics from xml metadata.

        This doesn't include resources, which aren't altered.
    */
    virtual bool loadFromXml (const XmlElement& xml);

    static XmlElement* pullMetaDataFromCppFile (const String& cpp);

    //==============================================================================
    //  Code generation

    /** Fills in a GeneratedCode structure with this component's state. */
    virtual void fillInGeneratedCode (GeneratedCode& code) const;

    virtual void fillInPaintCode (GeneratedCode& code) const;

    /** Tries to track down and load the header and cpp templates.
    */
    bool findTemplateFiles (String& templateH, String& templateCpp) const;

    /** Generates and returns the header and cpp file contents for this component. */
    void getPreviewFiles (String& header, String& cpp);

    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    const String getDocumentTitle();
    const String loadDocument (const File& file);
    const String saveDocument (const File& file);
    const File getLastDocumentOpened();
    void setLastDocumentOpened (const File& file);


protected:
    //==============================================================================
    String className;
    String componentName;
    String parentClasses, constructorParams, variableInitialisers;
    bool fixedSize;
    int initialWidth, initialHeight;

    BinaryResources resources;

    static void addMethod (const String& base, const String& returnVal, const String& method, const String& initialContent,
                           StringArray& baseClasses, StringArray& returnValues, StringArray& methods, StringArray& initialContents);

private:
    UndoManager undoManager;
    int snapGridPixels;
    bool snapActive, snapShown;
    Component* lastFocusedComp;
    int lastClickCounter;
    float componentOverlayOpacity;

    StringArray activeExtraMethods;

    void timerCallback();

    //==============================================================================
    const String getMetadata() const;

    bool writeCodeFiles (const File& headerFile,
                         const File& cppFile,
                         String headerTemplate,
                         String cppTemplate) const;
};


#endif   // __JUCER_JUCERDOCUMENT_JUCEHEADER__
