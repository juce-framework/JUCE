/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "jucer_DocumentEditorComponent.h"

//==============================================================================
class SourceCodeDocument : public OpenDocumentManager::Document
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
    bool saveSyncWithoutAsking() override;
    void saveAsync (std::function<void (bool)>) override;
    void saveAsAsync (std::function<void (bool)>) override;

    std::unique_ptr<Component> createEditor() override;
    std::unique_ptr<Component> createViewer() override  { return createEditor(); }

    void updateLastState (CodeEditorComponent&);
    void applyLastState (CodeEditorComponent&) const;

    CodeDocument& getCodeDocument();

    //==============================================================================
    struct Type final : public OpenDocumentManager::DocumentType
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

private:
    std::unique_ptr<FileChooser> chooser;
};

class GenericCodeEditorComponent;

//==============================================================================
class SourceCodeEditor final : public DocumentEditorComponent,
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
class GenericCodeEditorComponent : public CodeEditorComponent
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
class CppCodeEditorComponent final : public GenericCodeEditorComponent
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

    std::unique_ptr<AlertWindow> asyncAlertWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CppCodeEditorComponent)
};
