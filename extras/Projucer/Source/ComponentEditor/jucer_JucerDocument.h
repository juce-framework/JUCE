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

#include "../CodeEditor/jucer_OpenDocumentManager.h"
#include "../CodeEditor/jucer_SourceCodeEditor.h"
#include "Components/jucer_ComponentTypeHandler.h"
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
    ~JucerDocument() override;

    static bool isValidJucerCppFile (const File&);
    static std::unique_ptr<XmlElement> pullMetaDataFromCppFile (const String& cpp);
    static JucerDocument* createForCppFile (Project*, const File&);

    void changed();
    void beginTransaction();
    void beginTransaction (const String& name);

    virtual JucerDocument* createCopy() = 0;
    virtual String getTypeName() const = 0;

    SourceCodeDocument& getCppDocument() const      { return *cpp; }

    File getCppFile() const     { return cpp->getFile(); }
    File getHeaderFile() const  { return getCppFile().withFileExtension (".h"); }

    bool flushChangesToDocuments (Project*, bool);
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

    static bool shouldUseTransMacro() noexcept                              { return true; }

    //==============================================================================
    void refreshCustomCodeFromDocument();

protected:
    SourceCodeDocument* cpp;

    String className, componentName, templateFile;
    String parentClasses, constructorParams, variableInitialisers;

    bool fixedSize = false;
    int initialWidth = 600, initialHeight = 400;

    BinaryResources resources;

    virtual std::unique_ptr<XmlElement> createXml() const;
    virtual bool loadFromXml (const XmlElement&);

    virtual void fillInGeneratedCode (GeneratedCode&) const;
    virtual void fillInPaintCode (GeneratedCode&) const;

    virtual void applyCustomPaintSnippets (StringArray&) {}

    static void addMethod (const String& base, const String& returnVal,
                           const String& method, const String& initialContent,
                           StringArray& baseClasses, StringArray& returnValues,
                           StringArray& methods, StringArray& initialContents);

private:
    UndoManager undoManager;
    int snapGridPixels = 8;
    bool snapActive = true, snapShown = true;
    float componentOverlayOpacity = 0.33f;
    StringArray activeExtraMethods;
    std::unique_ptr<XmlElement> currentXML;
    std::unique_ptr<Timer> userDocChangeTimer;

    void timerCallback() override;
    void codeDocumentTextInserted (const String& newText, int insertIndex) override;
    void codeDocumentTextDeleted (int startIndex, int endIndex) override;
    void userEditedCpp();
    void extractCustomPaintSnippetsFromCppFile (const String& cpp);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucerDocument)
};
