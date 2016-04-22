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

#ifndef JUCER_JUCERDOCUMENT_H_INCLUDED
#define JUCER_JUCERDOCUMENT_H_INCLUDED

#include "../Application/jucer_OpenDocumentManager.h"
#include "../Code Editor/jucer_SourceCodeEditor.h"
#include "components/jucer_ComponentTypeHandler.h"
#include "jucer_PaintRoutine.h"
#include "jucer_ComponentLayout.h"
#include "jucer_BinaryResources.h"

//==============================================================================
class JucerDocument  : public ChangeBroadcaster,
                       private Timer,
                       private CodeDocument::Listener,
                       private OpenDocumentManager::DocumentCloseListener
{
public:
    JucerDocument (SourceCodeDocument* cpp);
    ~JucerDocument();

    static bool isValidJucerCppFile (const File&);
    static XmlElement* pullMetaDataFromCppFile (const String& cpp);
    static JucerDocument* createForCppFile (Project*, const File&);

    void changed();
    void beginTransaction();
    void beginTransaction (const String& name);

    virtual JucerDocument* createCopy() = 0;
    virtual String getTypeName() const = 0;

    SourceCodeDocument& getCppDocument() const      { return *cpp; }

    File getCppFile() const     { return cpp->getFile(); }
    File getHeaderFile() const  { return getCppFile().withFileExtension (".h"); }

    bool flushChangesToDocuments (Project*);
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

    void timerCallback() override;
    void codeDocumentTextInserted (const String& newText, int insertIndex) override;
    void codeDocumentTextDeleted (int startIndex, int endIndex) override;
    void userEditedCpp();
    bool documentAboutToClose (OpenDocumentManager::Document*) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucerDocument)
};


#endif   // JUCER_JUCERDOCUMENT_H_INCLUDED
