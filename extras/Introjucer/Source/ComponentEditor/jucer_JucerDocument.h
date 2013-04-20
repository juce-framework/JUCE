/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#include "../Application/jucer_OpenDocumentManager.h"
#include "../Code Editor/jucer_SourceCodeEditor.h"
#include "components/jucer_ComponentTypeHandler.h"
#include "jucer_PaintRoutine.h"
#include "jucer_ComponentLayout.h"
#include "jucer_BinaryResources.h"

//==============================================================================
class JucerDocument  : public ChangeBroadcaster,
                       private Timer,
                       private CodeDocument::Listener
{
public:
    JucerDocument (SourceCodeDocument* cpp);
    ~JucerDocument();

    static bool isValidJucerCppFile (const File&);
    static XmlElement* pullMetaDataFromCppFile (const String& cpp);
    static JucerDocument* createForCppFile (Project* project, const File&);

    void changed();
    void beginTransaction();
    void beginTransaction (const String& name);

    virtual JucerDocument* createCopy() = 0;
    virtual String getTypeName() const = 0;

    SourceCodeDocument& getCppDocument() const      { return *cpp; }

    File getCppFile() const     { return cpp->getFile(); }
    File getHeaderFile() const  { return getCppFile().withFileExtension (".h"); }

    bool flushChangesToDocuments();
    bool reloadFromDocument();

    //==============================================================================
    UndoManager& getUndoManager() noexcept                                  { return undoManager; }

    bool perform (UndoableAction* const action, const String& actionName);

    void refreshAllPropertyComps();

    //==============================================================================
    const String& getClassName() const noexcept                             { return className; }
    void setClassName (const String& newName);

    const String& getComponentName() const noexcept                         { return componentName; }
    void setComponentName (const String& newName);

    String getParentClassString() const                                     { return parentClasses; }
    void setParentClasses (const String& classes);

    String getConstructorParams() const                                     { return constructorParams; }
    void setConstructorParams (const String& newParams);

    String getVariableInitialisers() const                                  { return variableInitialisers; }
    void setVariableInitialisers (const String& newInitlialisers);

    void setFixedSize (const bool isFixed);
    bool isFixedSize() const noexcept                                       { return fixedSize; }

    void setInitialSize (int w, int h);

    int getInitialWidth() const noexcept                                    { return initialWidth; }
    int getInitialHeight() const noexcept                                   { return initialHeight; }

    //==============================================================================
    virtual int getNumPaintRoutines() const = 0;
    virtual StringArray getPaintRoutineNames() const = 0;
    virtual PaintRoutine* getPaintRoutine (const int index) const = 0;
    virtual ComponentLayout* getComponentLayout() const = 0;
    virtual Component* createTestComponent (const bool alwaysFillBackground) = 0;
    virtual void addExtraClassProperties (PropertyPanel&);

    //==============================================================================
    virtual void getOptionalMethods (StringArray& baseClasses,
                                     StringArray& returnValues,
                                     StringArray& methods,
                                     StringArray& initialContents) const;

    void setOptionalMethodEnabled (const String& methodSignature, const bool enable);
    bool isOptionalMethodEnabled (const String& methodSignature) const noexcept;

    //==============================================================================
    BinaryResources& getResources() noexcept                                { return resources; }

    //==============================================================================
    void setSnappingGrid (const int numPixels, const bool active, const bool shown);

    int getSnappingGridSize() const noexcept                                { return snapGridPixels; }
    bool isSnapActive (const bool disableIfCtrlKeyDown) const noexcept;
    bool isSnapShown() const noexcept                                       { return snapShown; }

    int snapPosition (int pos) const noexcept;

    //==============================================================================
    void setComponentOverlayOpacity (const float alpha);
    float getComponentOverlayOpacity() const noexcept                       { return componentOverlayOpacity; }

    //==============================================================================
    static const char* const jucerCompXmlTag;

    bool findTemplateFiles (String& templateH, String& templateCpp) const;

    String getTemplateFile() const                                          { return templateFile; }
    void setTemplateFile (const String&);

protected:
    SourceCodeDocument* cpp;

    String className, componentName, templateFile;
    String parentClasses, constructorParams, variableInitialisers;

    bool fixedSize;
    int initialWidth, initialHeight;

    BinaryResources resources;

    virtual XmlElement* createXml() const;
    virtual bool loadFromXml (const XmlElement&);

    virtual void fillInGeneratedCode (GeneratedCode&) const;
    virtual void fillInPaintCode (GeneratedCode&) const;

    static void addMethod (const String& base, const String& returnVal,
                           const String& method, const String& initialContent,
                           StringArray& baseClasses, StringArray& returnValues,
                           StringArray& methods, StringArray& initialContents);

private:
    UndoManager undoManager;
    int snapGridPixels;
    bool snapActive, snapShown;
    float componentOverlayOpacity;
    StringArray activeExtraMethods;
    ScopedPointer<XmlElement> currentXML;
    ScopedPointer<Timer> userDocChangeTimer;

    void timerCallback();
    void codeDocumentTextInserted (const String& newText, int insertIndex);
    void codeDocumentTextDeleted (int startIndex, int endIndex);
    void userEditedCpp();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucerDocument);
};


#endif   // __JUCER_JUCERDOCUMENT_JUCEHEADER__
