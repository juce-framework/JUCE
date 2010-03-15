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

#ifndef __JUCER_OPENDOCUMENTMANAGER_JUCEHEADER__
#define __JUCER_OPENDOCUMENTMANAGER_JUCEHEADER__

#include "../model/jucer_Project.h"
#include "../model/jucer_DrawableDocument.h"
class DocumentEditorComponent;


//==============================================================================
/**
*/
class OpenDocumentManager
{
public:
    //==============================================================================
    OpenDocumentManager();
    ~OpenDocumentManager();

    juce_DeclareSingleton_SingleThreaded_Minimal (OpenDocumentManager);

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
        virtual const String getName() const = 0;
        virtual const String getType() const = 0;
        virtual bool needsSaving() const = 0;
        virtual bool save() = 0;
        virtual bool hasFileBeenModifiedExternally() = 0;
        virtual void reloadFromFile() = 0;
        virtual Component* createEditor() = 0;
    };

    Document* getDocumentForFile (Project* project, const File& file);
    bool canOpenFile (const File& file);

    //==============================================================================
    int getNumOpenDocuments() const;
    Document* getOpenDocument (int index) const;
    void moveDocumentToTopOfStack (Document* doc);

    bool closeDocument (int index, bool saveIfNeeded);
    bool closeDocument (Document* document, bool saveIfNeeded);
    bool closeAllDocumentsUsingProject (Project& project, bool saveIfNeeded);
    void closeFile (const File& f, bool saveIfNeeded);
    bool anyFilesNeedSaving() const;
    bool saveAll();
    FileBasedDocument::SaveResult saveIfNeededAndUserAgrees (Document* doc);
    void reloadModifiedFiles();

    //==============================================================================
    void registerEditor (DocumentEditorComponent* editor);
    void deregisterEditor (DocumentEditorComponent* editor);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    OwnedArray <Document> documents;
    SortedSet <DocumentEditorComponent*> editors;
};


#endif   // __JUCER_OPENDOCUMENTMANAGER_JUCEHEADER__
