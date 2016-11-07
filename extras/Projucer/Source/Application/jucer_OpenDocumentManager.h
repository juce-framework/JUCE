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

#ifndef JUCER_OPENDOCUMENTMANAGER_H_INCLUDED
#define JUCER_OPENDOCUMENTMANAGER_H_INCLUDED

#include "../Project/jucer_Project.h"

//==============================================================================
/**
*/
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
        virtual File getCounterpartFile() const   { return File(); }
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


#endif   // JUCER_OPENDOCUMENTMANAGER_H_INCLUDED
