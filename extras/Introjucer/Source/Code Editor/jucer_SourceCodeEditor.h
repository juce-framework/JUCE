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

#ifndef __JUCER_SOURCECODEEDITOR_JUCEHEADER__
#define __JUCER_SOURCECODEEDITOR_JUCEHEADER__

#include "../Project/jucer_Project.h"
#include "../Application/jucer_DocumentEditorComponent.h"


//==============================================================================
class SourceCodeDocument  : public OpenDocumentManager::Document
{
public:
    SourceCodeDocument (Project*, const File&);

    bool loadedOk() const                           { return true; }
    bool isForFile (const File& file) const         { return getFile() == file; }
    bool isForNode (const ValueTree& node) const    { return false; }
    bool refersToProject (Project& p) const         { return project == &p; }
    Project* getProject() const                     { return project; }
    String getName() const                          { return getFile().getFileName(); }
    String getType() const                          { return getFile().getFileExtension() + " file"; }
    File getFile() const                            { return modDetector.getFile(); }
    bool needsSaving() const                        { return codeDoc != nullptr && codeDoc->hasChangedSinceSavePoint(); }
    bool hasFileBeenModifiedExternally()            { return modDetector.hasBeenModified(); }
    void fileHasBeenRenamed (const File& newFile)   { modDetector.fileHasBeenRenamed (newFile); }
    String getState() const                         { return lastState != nullptr ? lastState->toString() : String::empty; }
    void restoreState (const String& state)         { lastState = new CodeEditorComponent::State (state); }

    void reloadFromFile();
    bool save();

    Component* createEditor();
    Component* createViewer()       { return createEditor(); }

    void updateLastState (CodeEditorComponent& editor);
    void applyLastState (CodeEditorComponent& editor) const;

    CodeDocument& getCodeDocument();

    //==============================================================================
    struct Type  : public OpenDocumentManager::DocumentType
    {
        bool canOpenFile (const File& file)                     { return file.hasFileExtension ("cpp;h;hpp;mm;m;c;cc;cxx;txt;inc;tcc;xml;plist;rtf;html;htm;php;py;rb;cs"); }
        Document* openFile (Project* project, const File& file) { return new SourceCodeDocument (project, file); }
    };

protected:
    FileModificationDetector modDetector;
    ScopedPointer<CodeDocument> codeDoc;
    Project* project;

    ScopedPointer<CodeEditorComponent::State> lastState;

    void reloadInternal();
};

//==============================================================================
class SourceCodeEditor  : public DocumentEditorComponent,
                          private ValueTree::Listener
{
public:
    SourceCodeEditor (OpenDocumentManager::Document* document);
    ~SourceCodeEditor();

    void createEditor (CodeDocument& codeDocument);
    void setEditor (CodeEditorComponent*);

    void highlightLine (int lineNum, int characterIndex);

    ScopedPointer<CodeEditorComponent> editor;

private:
    void resized();

    void valueTreePropertyChanged (ValueTree&, const Identifier&);
    void valueTreeChildAdded (ValueTree&, ValueTree&);
    void valueTreeChildRemoved (ValueTree&, ValueTree&);
    void valueTreeChildOrderChanged (ValueTree&);
    void valueTreeParentChanged (ValueTree&);
    void valueTreeRedirected (ValueTree&);

    void updateColourScheme();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SourceCodeEditor);
};


//==============================================================================
class CppCodeEditorComponent  : public CodeEditorComponent
{
public:
    CppCodeEditorComponent (CodeDocument& codeDocument);

    void handleReturnKey();
    void insertTextAtCaret (const String& newText);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CppCodeEditorComponent);
};


#endif   // __JUCER_SOURCECODEEDITOR_JUCEHEADER__
