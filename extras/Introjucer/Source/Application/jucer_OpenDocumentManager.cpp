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

#include "jucer_OpenDocumentManager.h"
#include "jucer_FilePreviewComponent.h"
#include "../Code Editor/jucer_SourceCodeEditor.h"

//==============================================================================
Component* SourceCodeDocument::createEditor()       { return SourceCodeEditor::createFor (this, codeDoc); }


//==============================================================================
class UnknownDocument  : public OpenDocumentManager::Document
{
public:
    UnknownDocument (Project* project_, const File& file_)
       : project (project_), file (file_)
    {
        reloadFromFile();
    }

    //==============================================================================
    struct Type  : public OpenDocumentManager::DocumentType
    {
        bool canOpenFile (const File&)                              { return true; }
        Document* openFile (Project* project, const File& file)     { return new UnknownDocument (project, file); }
    };

    //==============================================================================
    bool loadedOk() const                           { return true; }
    bool isForFile (const File& file_) const        { return file == file_; }
    bool isForNode (const ValueTree& node_) const   { return false; }
    bool refersToProject (Project& p) const         { return project == &p; }
    Project* getProject() const                     { return project; }
    bool needsSaving() const                        { return false; }
    bool save()                                     { return true; }
    bool hasFileBeenModifiedExternally()            { return fileModificationTime != file.getLastModificationTime(); }
    void reloadFromFile()                           { fileModificationTime = file.getLastModificationTime(); }
    String getName() const                          { return file.getFileName(); }
    File getFile() const                            { return file; }
    Component* createEditor()                       { return new ItemPreviewComponent (file); }
    Component* createViewer()                       { return createEditor(); }
    void fileHasBeenRenamed (const File& newFile)   { file = newFile; }

    String getType() const
    {
        if (file.getFileExtension().isNotEmpty())
            return file.getFileExtension() + " file";

        jassertfalse
        return "Unknown";
    }

private:
    Project* const project;
    File file;
    Time fileModificationTime;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UnknownDocument);
};


//==============================================================================
OpenDocumentManager::OpenDocumentManager()
{
    registerType (new UnknownDocument::Type());
    registerType (new SourceCodeDocument::Type());
}

OpenDocumentManager::~OpenDocumentManager()
{
    clearSingletonInstance();
}

juce_ImplementSingleton_SingleThreaded (OpenDocumentManager);

//==============================================================================
void OpenDocumentManager::registerType (DocumentType* type)
{
    types.add (type);
}

//==============================================================================
void OpenDocumentManager::addListener (DocumentCloseListener* listener)
{
    listeners.addIfNotAlreadyThere (listener);
}

void OpenDocumentManager::removeListener (DocumentCloseListener* listener)
{
    listeners.removeValue (listener);
}

//==============================================================================
bool OpenDocumentManager::canOpenFile (const File& file)
{
    for (int i = types.size(); --i >= 0;)
        if (types.getUnchecked(i)->canOpenFile (file))
            return true;

    return false;
}

OpenDocumentManager::Document* OpenDocumentManager::openFile (Project* project, const File& file)
{
    for (int i = documents.size(); --i >= 0;)
        if (documents.getUnchecked(i)->isForFile (file))
            return documents.getUnchecked(i);

    Document* d = nullptr;

    for (int i = types.size(); --i >= 0 && d == nullptr;)
    {
        if (types.getUnchecked(i)->canOpenFile (file))
        {
            d = types.getUnchecked(i)->openFile (project, file);
            jassert (d != nullptr);
        }
    }

    jassert (d != nullptr);  // should always at least have been picked up by UnknownDocument

    documents.add (d);
    commandManager->commandStatusChanged();
    return d;
}

int OpenDocumentManager::getNumOpenDocuments() const
{
    return documents.size();
}

OpenDocumentManager::Document* OpenDocumentManager::getOpenDocument (int index) const
{
    return documents.getUnchecked (index);
}

void OpenDocumentManager::moveDocumentToTopOfStack (Document* doc)
{
    for (int i = documents.size(); --i >= 0;)
    {
        if (doc == documents.getUnchecked(i))
        {
            documents.move (i, 0);
            commandManager->commandStatusChanged();
            break;
        }
    }
}

FileBasedDocument::SaveResult OpenDocumentManager::saveIfNeededAndUserAgrees (OpenDocumentManager::Document* doc)
{
    if (! doc->needsSaving())
        return FileBasedDocument::savedOk;

    const int r = AlertWindow::showYesNoCancelBox (AlertWindow::QuestionIcon,
                                                   TRANS("Closing document..."),
                                                   TRANS("Do you want to save the changes to \"")
                                                       + doc->getName() + "\"?",
                                                   TRANS("save"),
                                                   TRANS("discard changes"),
                                                   TRANS("cancel"));

    if (r == 1)
    {
        // save changes
        return doc->save() ? FileBasedDocument::savedOk
                           : FileBasedDocument::failedToWriteToFile;
    }
    else if (r == 2)
    {
        // discard changes
        return FileBasedDocument::savedOk;
    }

    return FileBasedDocument::userCancelledSave;
}


bool OpenDocumentManager::closeDocument (int index, bool saveIfNeeded)
{
    Document* doc = documents [index];

    if (doc != nullptr)
    {
        if (saveIfNeeded)
        {
            if (saveIfNeededAndUserAgrees (doc) != FileBasedDocument::savedOk)
                return false;
        }

        for (int i = listeners.size(); --i >= 0;)
            listeners.getUnchecked(i)->documentAboutToClose (doc);

        documents.remove (index);
        commandManager->commandStatusChanged();
    }

    return true;
}

bool OpenDocumentManager::closeDocument (Document* document, bool saveIfNeeded)
{
    return closeDocument (documents.indexOf (document), saveIfNeeded);
}

void OpenDocumentManager::closeFile (const File& f, bool saveIfNeeded)
{
    for (int i = documents.size(); --i >= 0;)
    {
        Document* d = documents.getUnchecked (i);

        if (d->isForFile (f))
            closeDocument (i, saveIfNeeded);
    }
}

bool OpenDocumentManager::closeAll (bool askUserToSave)
{
    for (int i = getNumOpenDocuments(); --i >= 0;)
        if (! closeDocument (i, askUserToSave))
            return false;

    return true;
}

bool OpenDocumentManager::closeAllDocumentsUsingProject (Project& project, bool saveIfNeeded)
{
    for (int i = documents.size(); --i >= 0;)
    {
        Document* d = documents.getUnchecked (i);

        if (d->refersToProject (project))
        {
            if (! closeDocument (i, saveIfNeeded))
                return false;
        }
    }

    return true;
}

bool OpenDocumentManager::anyFilesNeedSaving() const
{
    for (int i = documents.size(); --i >= 0;)
    {
        Document* d = documents.getUnchecked (i);

        if (d->needsSaving())
            return true;
    }

    return false;
}

bool OpenDocumentManager::saveAll()
{
    for (int i = documents.size(); --i >= 0;)
    {
        Document* d = documents.getUnchecked (i);

        if (! d->save())
            return false;
    }

    return true;
}

void OpenDocumentManager::reloadModifiedFiles()
{
    for (int i = documents.size(); --i >= 0;)
    {
        Document* d = documents.getUnchecked (i);

        if (d->hasFileBeenModifiedExternally())
            d->reloadFromFile();
    }
}

void OpenDocumentManager::fileHasBeenRenamed (const File& oldFile, const File& newFile)
{
    for (int i = documents.size(); --i >= 0;)
    {
        Document* d = documents.getUnchecked (i);

        if (d->isForFile (oldFile))
            d->fileHasBeenRenamed (newFile);
    }
}
