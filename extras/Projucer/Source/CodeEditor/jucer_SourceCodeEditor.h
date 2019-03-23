/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "jucer_DocumentEditorComponent.h"

//==============================================================================
class SourceCodeDocument  : public OpenDocumentManager::Document
{
public:
    SourceCodeDocument (Project*, const File&);

    bool loadedOk() const override                           { return true; }
    bool isForFile (const File& file) const override         { return getFile() == file; }
    bool isForNode (const ValueTree&) const override         { return false; }
    bool refersToProject (Project& p) const override         { return project == &p; }
    Project* getProject() const override                     { return project; }
    String getName() const override                          { return getFile().getFileName(); }
    String getType() const override                          { return getFile().getFileExtension() + " file"; }
    File getFile() const override                            { return modDetector.getFile(); }
    bool needsSaving() const override                        { return codeDoc != nullptr && codeDoc->hasChangedSinceSavePoint(); }
    bool hasFileBeenModifiedExternally() override            { return modDetector.hasBeenModified(); }
    void fileHasBeenRenamed (const File& newFile) override   { modDetector.fileHasBeenRenamed (newFile); }
    String getState() const override                         { return lastState != nullptr ? lastState->toString() : String(); }
    void restoreState (const String& state) override         { lastState.reset (new CodeEditorComponent::State (state)); }

    File getCounterpartFile() const override
    {
        auto file = getFile();

        if (file.hasFileExtension (sourceFileExtensions))
        {
            static const char* extensions[] = { "h", "hpp", "hxx", "hh", nullptr };
            return findCounterpart (file, extensions);
        }

        if (file.hasFileExtension (headerFileExtensions))
        {
            static const char* extensions[] = { "cpp", "mm", "cc", "cxx", "c", "m", nullptr };
            return findCounterpart (file, extensions);
        }

        return {};
    }

    static File findCounterpart (const File& file, const char** extensions)
    {
        while (*extensions != nullptr)
        {
            auto f = file.withFileExtension (*extensions++);

            if (f.existsAsFile())
                return f;
        }

        return {};
    }

    void reloadFromFile() override;
    bool save() override;
    bool saveAs() override;

    Component* createEditor() override;
    Component* createViewer() override       { return createEditor(); }

    void updateLastState (CodeEditorComponent&);
    void applyLastState (CodeEditorComponent&) const;

    CodeDocument& getCodeDocument();

    //==============================================================================
    struct Type  : public OpenDocumentManager::DocumentType
    {
        bool canOpenFile (const File& file) override
        {
            if (file.hasFileExtension (sourceOrHeaderFileExtensions)
                 || file.hasFileExtension ("txt;inc;tcc;xml;plist;rtf;html;htm;php;py;rb;cs"))
                return true;

            MemoryBlock mb;
            if (file.loadFileAsData (mb)
                 && seemsToBeText (static_cast<const char*> (mb.getData()), (int) mb.getSize())
                 && ! file.hasFileExtension ("svg"))
                return true;

            return false;
        }

        static bool seemsToBeText (const char* const chars, const int num) noexcept
        {
            for (int i = 0; i < num; ++i)
            {
                const char c = chars[i];
                if ((c < 32 && c != '\t' && c != '\r' && c != '\n') || chars[i] > 126)
                    return false;
            }

            return true;
        }

        Document* openFile (Project* p, const File& file) override   { return new SourceCodeDocument (p, file); }
    };

protected:
    FileModificationDetector modDetector;
    std::unique_ptr<CodeDocument> codeDoc;
    Project* project;

    std::unique_ptr<CodeEditorComponent::State> lastState;

    void reloadInternal();
};

class GenericCodeEditorComponent;

//==============================================================================
class SourceCodeEditor  : public DocumentEditorComponent,
                          private ValueTree::Listener,
                          private CodeDocument::Listener
{
public:
    SourceCodeEditor (OpenDocumentManager::Document*, CodeDocument&);
    SourceCodeEditor (OpenDocumentManager::Document*, GenericCodeEditorComponent*);
    ~SourceCodeEditor() override;

    void scrollToKeepRangeOnScreen (Range<int> range);
    void highlight (Range<int> range, bool cursorAtStart);

    std::unique_ptr<GenericCodeEditorComponent> editor;

private:
    void resized() override;
    void lookAndFeelChanged() override;

    void valueTreePropertyChanged (ValueTree&, const Identifier&) override;
    void valueTreeChildAdded (ValueTree&, ValueTree&) override;
    void valueTreeChildRemoved (ValueTree&, ValueTree&, int) override;
    void valueTreeChildOrderChanged (ValueTree&, int, int) override;
    void valueTreeParentChanged (ValueTree&) override;
    void valueTreeRedirected (ValueTree&) override;

    void codeDocumentTextInserted (const String&, int) override;
    void codeDocumentTextDeleted (int, int) override;

    void setEditor (GenericCodeEditorComponent*);
    void updateColourScheme();
    void checkSaveState();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SourceCodeEditor)
};


//==============================================================================
class GenericCodeEditorComponent  : public CodeEditorComponent
{
public:
    GenericCodeEditorComponent (const File&, CodeDocument&, CodeTokeniser*);
    ~GenericCodeEditorComponent() override;

    void addPopupMenuItems (PopupMenu&, const MouseEvent*) override;
    void performPopupMenuAction (int menuItemID) override;

    void getAllCommands (Array<CommandID>&) override;
    void getCommandInfo (CommandID, ApplicationCommandInfo&) override;
    bool perform (const InvocationInfo&) override;

    void showFindPanel();
    void hideFindPanel();
    void findSelection();
    void findNext (bool forwards, bool skipCurrentSelection);
    void handleEscapeKey() override;
    void editorViewportPositionChanged() override;

    void resized() override;

    static String getSearchString()                 { return getAppSettings().getGlobalProperties().getValue ("searchString"); }
    static void setSearchString (const String& s)   { getAppSettings().getGlobalProperties().setValue ("searchString", s); }
    static bool isCaseSensitiveSearch()             { return getAppSettings().getGlobalProperties().getBoolValue ("searchCaseSensitive"); }
    static void setCaseSensitiveSearch (bool b)     { getAppSettings().getGlobalProperties().setValue ("searchCaseSensitive", b); }

    struct Listener
    {
        virtual ~Listener() {}
        virtual void codeEditorViewportMoved (CodeEditorComponent&) = 0;
    };

    void addListener (Listener* listener);
    void removeListener (Listener* listener);

private:
    File file;
    class FindPanel;
    std::unique_ptr<FindPanel> findPanel;
    ListenerList<Listener> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GenericCodeEditorComponent)
};

//==============================================================================
class CppCodeEditorComponent  : public GenericCodeEditorComponent
{
public:
    CppCodeEditorComponent (const File&, CodeDocument&);
    ~CppCodeEditorComponent() override;

    void addPopupMenuItems (PopupMenu&, const MouseEvent*) override;
    void performPopupMenuAction (int menuItemID) override;

    void handleReturnKey() override;
    void insertTextAtCaret (const String& newText) override;

private:
    void insertComponentClass();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CppCodeEditorComponent)
};
