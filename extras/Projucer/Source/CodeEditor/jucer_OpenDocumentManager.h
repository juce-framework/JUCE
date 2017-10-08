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

#include "../Project/jucer_Project.h"

//==============================================================================
class OpenDocumentManager
{
public:
    //==============================================================================
    OpenDocumentManager();
    ~OpenDocumentManager();

    //==============================================================================
    class Document
    {
    public:
        Document() {}
        virtual ~Document() {}

        virtual bool loadedOk() const = 0;
        virtual bool isForFile (const File& file) const = 0;
        virtual bool isForNode (const ValueTree& node) const = 0;
        virtual bool refersToProject (Project& project) const = 0;
        virtual Project* getProject() const = 0;
        virtual String getName() const = 0;
        virtual String getType() const = 0;
        virtual File getFile() const = 0;
        virtual bool needsSaving() const = 0;
        virtual bool save() = 0;
        virtual bool saveAs() = 0;
        virtual bool hasFileBeenModifiedExternally() = 0;
        virtual void reloadFromFile() = 0;
        virtual Component* createEditor() = 0;
        virtual Component* createViewer() = 0;
        virtual void fileHasBeenRenamed (const File& newFile) = 0;
        virtual String getState() const = 0;
        virtual void restoreState (const String& state) = 0;
        virtual File getCounterpartFile() const   { return {}; }
    };

    //==============================================================================
    int getNumOpenDocuments() const;
    Document* getOpenDocument (int index) const;
    void clear();

    bool canOpenFile (const File& file);
    Document* openFile (Project* project, const File& file);
    bool closeDocument (int index, bool saveIfNeeded);
    bool closeDocument (Document* document, bool saveIfNeeded);
    bool closeAll (bool askUserToSave);
    bool closeAllDocumentsUsingProject (Project& project, bool saveIfNeeded);
    void closeFile (const File& f, bool saveIfNeeded);
    bool anyFilesNeedSaving() const;
    bool saveAll();
    FileBasedDocument::SaveResult saveIfNeededAndUserAgrees (Document* doc);
    void reloadModifiedFiles();
    void fileHasBeenRenamed (const File& oldFile, const File& newFile);

    //==============================================================================
    class DocumentCloseListener
    {
    public:
        DocumentCloseListener() {}
        virtual ~DocumentCloseListener() {}

        // return false to force it to stop.
        virtual bool documentAboutToClose (Document* document) = 0;
    };

    void addListener (DocumentCloseListener*);
    void removeListener (DocumentCloseListener*);

    //==============================================================================
    class DocumentType
    {
    public:
        DocumentType() {}
        virtual ~DocumentType() {}

        virtual bool canOpenFile (const File& file) = 0;
        virtual Document* openFile (Project* project, const File& file) = 0;
    };

    void registerType (DocumentType* type, int index = -1);


private:
    OwnedArray<DocumentType> types;
    OwnedArray<Document> documents;
    Array<DocumentCloseListener*> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenDocumentManager)
};

//==============================================================================
class RecentDocumentList    : private OpenDocumentManager::DocumentCloseListener
{
public:
    RecentDocumentList();
    ~RecentDocumentList();

    void clear();

    void newDocumentOpened (OpenDocumentManager::Document* document);

    OpenDocumentManager::Document* getCurrentDocument() const       { return previousDocs.getLast(); }

    bool canGoToPrevious() const;
    bool canGoToNext() const;

    bool contains (const File&) const;

    OpenDocumentManager::Document* getPrevious();
    OpenDocumentManager::Document* getNext();

    OpenDocumentManager::Document* getClosestPreviousDocOtherThan (OpenDocumentManager::Document* oneToAvoid) const;

    void restoreFromXML (Project& project, const XmlElement& xml);
    XmlElement* createXML() const;

private:
    bool documentAboutToClose (OpenDocumentManager::Document*);

    Array<OpenDocumentManager::Document*> previousDocs, nextDocs;
};
