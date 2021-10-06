/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../Application/jucer_Headers.h"
#include "jucer_OpenDocumentManager.h"
#include "../CodeEditor/jucer_ItemPreviewComponent.h"
#include "../Application/jucer_Application.h"


//==============================================================================
class UnknownDocument  : public OpenDocumentManager::Document
{
public:
    UnknownDocument (Project* p, const File& f)
       : project (p), file (f)
    {
        reloadFromFile();
    }

    //==============================================================================
    struct Type  : public OpenDocumentManager::DocumentType
    {
        bool canOpenFile (const File&) override                     { return true; }
        Document* openFile (Project* p, const File& f) override     { return new UnknownDocument (p, f); }
    };

    //==============================================================================
    bool loadedOk() const override                           { return true; }
    bool isForFile (const File& f) const override            { return file == f; }
    bool isForNode (const ValueTree&) const override         { return false; }
    bool refersToProject (Project& p) const override         { return project == &p; }
    Project* getProject() const override                     { return project; }
    bool needsSaving() const override                        { return false; }
    bool saveSyncWithoutAsking() override                    { return true; }
    void saveAsync (std::function<void (bool)>) override     {}
    void saveAsAsync (std::function<void (bool)>) override   {}
    bool hasFileBeenModifiedExternally() override            { return fileModificationTime != file.getLastModificationTime(); }
    void reloadFromFile() override                           { fileModificationTime = file.getLastModificationTime(); }
    String getName() const override                          { return file.getFileName(); }
    File getFile() const override                            { return file; }
    std::unique_ptr<Component> createEditor() override       { return std::make_unique<ItemPreviewComponent> (file); }
    std::unique_ptr<Component> createViewer() override       { return createEditor(); }
    void fileHasBeenRenamed (const File& newFile) override   { file = newFile; }
    String getState() const override                         { return {}; }
    void restoreState (const String&) override               {}

    String getType() const override
    {
        if (file.getFileExtension().isNotEmpty())
            return file.getFileExtension() + " file";

        jassertfalse;
        return "Unknown";
    }

private:
    Project* const project;
    File file;
    Time fileModificationTime;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UnknownDocument)
};


//==============================================================================
OpenDocumentManager::DocumentType* createGUIDocumentType();

OpenDocumentManager::OpenDocumentManager()
{
    registerType (new UnknownDocument::Type());
    registerType (new SourceCodeDocument::Type());
    registerType (createGUIDocumentType());
}

OpenDocumentManager::~OpenDocumentManager()
{
}

void OpenDocumentManager::clear()
{
    documents.clear();
    types.clear();
}

//==============================================================================
void OpenDocumentManager::registerType (DocumentType* type, int index)
{
    types.insert (index, type);
}

//==============================================================================
void OpenDocumentManager::addListener (DocumentCloseListener* listener)
{
    listeners.addIfNotAlreadyThere (listener);
}

void OpenDocumentManager::removeListener (DocumentCloseListener* listener)
{
    listeners.removeFirstMatchingValue (listener);
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
    ProjucerApplication::getCommandManager().commandStatusChanged();
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

void OpenDocumentManager::saveIfNeededAndUserAgrees (OpenDocumentManager::Document* doc,
                                                    std::function<void (FileBasedDocument::SaveResult)> callback)
{
    if (! doc->needsSaving())
    {
        if (callback != nullptr)
            callback (FileBasedDocument::savedOk);

        return;
    }

    AlertWindow::showYesNoCancelBox (MessageBoxIconType::QuestionIcon,
                                     TRANS("Closing document..."),
                                     TRANS("Do you want to save the changes to \"")
                                         + doc->getName() + "\"?",
                                     TRANS("Save"),
                                     TRANS("Discard changes"),
                                     TRANS("Cancel"),
                                     nullptr,
                                     ModalCallbackFunction::create ([parent = WeakReference<OpenDocumentManager> { this }, doc, callback] (int r)
    {
        if (parent == nullptr)
            return;

        if (r == 1)
        {
            doc->saveAsync ([parent, callback] (bool hasSaved)
            {
                if (parent == nullptr)
                    return;

                if (callback != nullptr)
                    callback (hasSaved ? FileBasedDocument::savedOk : FileBasedDocument::failedToWriteToFile);
            });
            return;
        }

        if (callback != nullptr)
            callback (r == 2 ? FileBasedDocument::savedOk : FileBasedDocument::userCancelledSave);
    }));
}

bool OpenDocumentManager::closeDocumentWithoutSaving (Document* doc)
{
    if (documents.contains (doc))
    {
        bool canClose = true;

        for (int i = listeners.size(); --i >= 0;)
            if (auto* l = listeners[i])
                if (! l->documentAboutToClose (doc))
                    canClose = false;

        if (! canClose)
            return false;

        documents.removeObject (doc);
        ProjucerApplication::getCommandManager().commandStatusChanged();
    }

    return true;
}

void OpenDocumentManager::closeDocumentAsync (Document* doc, SaveIfNeeded saveIfNeeded, std::function<void (bool)> callback)
{
    if (! documents.contains (doc))
    {
        if (callback != nullptr)
            callback (true);

        return;
    }

    if (saveIfNeeded == SaveIfNeeded::yes)
    {
        saveIfNeededAndUserAgrees (doc,
                                   [parent = WeakReference<OpenDocumentManager> { this }, doc, callback] (FileBasedDocument::SaveResult result)
        {
            if (parent == nullptr)
                return;

            if (result != FileBasedDocument::savedOk)
            {
                if (callback != nullptr)
                    callback (false);

                return;
            }

            auto closed = parent->closeDocumentWithoutSaving (doc);

            if (callback != nullptr)
                callback (closed);
        });

        return;
    }

    auto closed = closeDocumentWithoutSaving (doc);

    if (callback != nullptr)
        callback (closed);
}

void OpenDocumentManager::closeFileWithoutSaving (const File& f)
{
    for (int i = documents.size(); --i >= 0;)
        if (auto* d = documents[i])
            if (d->isForFile (f))
                closeDocumentWithoutSaving (d);
}

static void closeLastAsyncRecusrsive (WeakReference<OpenDocumentManager> parent,
                                      OpenDocumentManager::SaveIfNeeded askUserToSave,
                                      std::function<void (bool)> callback)
{
    auto lastIndex = parent->getNumOpenDocuments() - 1;

    if (lastIndex < 0)
    {
        if (callback != nullptr)
            callback (true);

        return;
    }

    parent->closeDocumentAsync (parent->getOpenDocument (lastIndex),
                                askUserToSave,
                                [parent, askUserToSave, callback] (bool closedSuccessfully)
    {
        if (parent == nullptr)
            return;

        if (! closedSuccessfully)
        {
            if (callback != nullptr)
                callback (false);

            return;
        }

        closeLastAsyncRecusrsive (parent, askUserToSave, std::move (callback));
    });
}

void OpenDocumentManager::closeAllAsync (SaveIfNeeded askUserToSave, std::function<void (bool)> callback)
{
    closeLastAsyncRecusrsive (this, askUserToSave, std::move (callback));
}

void OpenDocumentManager::closeLastDocumentUsingProjectRecursive (WeakReference<OpenDocumentManager> parent,
                                                                  Project* project,
                                                                  SaveIfNeeded askUserToSave,
                                                                  std::function<void (bool)> callback)
{
    for (int i = documents.size(); --i >= 0;)
    {
        if (auto* d = documents[i])
        {
            if (d->getProject() == project)
            {
                closeDocumentAsync (d, askUserToSave, [parent, project, askUserToSave, callback] (bool closedSuccessfully)
                {
                    if (parent == nullptr)
                        return;

                    if (! closedSuccessfully)
                    {
                        if (callback != nullptr)
                            callback (false);

                        return;
                    }

                    parent->closeLastDocumentUsingProjectRecursive (parent, project, askUserToSave, std::move (callback));
                });

                return;
            }
        }
    }

    if (callback != nullptr)
        callback (true);
}

void OpenDocumentManager::closeAllDocumentsUsingProjectAsync (Project& project, SaveIfNeeded askUserToSave, std::function<void (bool)> callback)
{
    WeakReference<OpenDocumentManager> parent { this };
    closeLastDocumentUsingProjectRecursive (parent, &project, askUserToSave, std::move (callback));
}

void OpenDocumentManager::closeAllDocumentsUsingProjectWithoutSaving (Project& project)
{
    for (int i = documents.size(); --i >= 0;)
        if (Document* d = documents[i])
            if (d->refersToProject (project))
                closeDocumentWithoutSaving (d);
}

bool OpenDocumentManager::anyFilesNeedSaving() const
{
    for (int i = documents.size(); --i >= 0;)
        if (documents.getUnchecked (i)->needsSaving())
            return true;

    return false;
}

void OpenDocumentManager::saveAllSyncWithoutAsking()
{
    for (int i = documents.size(); --i >= 0;)
    {
        if (documents.getUnchecked (i)->saveSyncWithoutAsking())
            ProjucerApplication::getCommandManager().commandStatusChanged();
    }
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


//==============================================================================
RecentDocumentList::RecentDocumentList()
{
    ProjucerApplication::getApp().openDocumentManager.addListener (this);
}

RecentDocumentList::~RecentDocumentList()
{
    ProjucerApplication::getApp().openDocumentManager.removeListener (this);
}

void RecentDocumentList::clear()
{
    previousDocs.clear();
    nextDocs.clear();
}

void RecentDocumentList::newDocumentOpened (OpenDocumentManager::Document* document)
{
    if (document != nullptr && document != getCurrentDocument())
    {
        nextDocs.clear();
        previousDocs.add (document);
    }
}

bool RecentDocumentList::canGoToPrevious() const
{
    return previousDocs.size() > 1;
}

bool RecentDocumentList::canGoToNext() const
{
    return nextDocs.size() > 0;
}

OpenDocumentManager::Document* RecentDocumentList::getPrevious()
{
    if (! canGoToPrevious())
        return nullptr;

    nextDocs.insert (0, previousDocs.removeAndReturn (previousDocs.size() - 1));
    return previousDocs.getLast();
}

OpenDocumentManager::Document* RecentDocumentList::getNext()
{
    if (! canGoToNext())
        return nullptr;

    OpenDocumentManager::Document* d = nextDocs.removeAndReturn (0);
    previousDocs.add (d);
    return d;
}

bool RecentDocumentList::contains (const File& f) const
{
    for (int i = previousDocs.size(); --i >= 0;)
        if (previousDocs.getUnchecked(i)->getFile() == f)
            return true;

    return false;
}

OpenDocumentManager::Document* RecentDocumentList::getClosestPreviousDocOtherThan (OpenDocumentManager::Document* oneToAvoid) const
{
    for (int i = previousDocs.size(); --i >= 0;)
        if (previousDocs.getUnchecked(i) != oneToAvoid)
            return previousDocs.getUnchecked(i);

    return nullptr;
}

bool RecentDocumentList::documentAboutToClose (OpenDocumentManager::Document* document)
{
    previousDocs.removeAllInstancesOf (document);
    nextDocs.removeAllInstancesOf (document);

    jassert (! previousDocs.contains (document));
    jassert (! nextDocs.contains (document));

    return true;
}

static void restoreDocList (Project& project, Array <OpenDocumentManager::Document*>& list, const XmlElement* xml)
{
    if (xml != nullptr)
    {
        OpenDocumentManager& odm = ProjucerApplication::getApp().openDocumentManager;

        for (auto* e : xml->getChildWithTagNameIterator ("DOC"))
        {
            const File file (e->getStringAttribute ("file"));

            if (file.exists())
            {
                if (OpenDocumentManager::Document* doc = odm.openFile (&project, file))
                {
                    doc->restoreState (e->getStringAttribute ("state"));

                    list.add (doc);
                }
            }
        }
    }
}

void RecentDocumentList::restoreFromXML (Project& project, const XmlElement& xml)
{
    clear();

    if (xml.hasTagName ("RECENT_DOCUMENTS"))
    {
        restoreDocList (project, previousDocs, xml.getChildByName ("PREVIOUS"));
        restoreDocList (project, nextDocs,     xml.getChildByName ("NEXT"));
    }
}

static void saveDocList (const Array <OpenDocumentManager::Document*>& list, XmlElement& xml)
{
    for (int i = 0; i < list.size(); ++i)
    {
        const OpenDocumentManager::Document& doc = *list.getUnchecked(i);

        XmlElement* e = xml.createNewChildElement ("DOC");

        e->setAttribute ("file", doc.getFile().getFullPathName());
        e->setAttribute ("state", doc.getState());
    }
}

std::unique_ptr<XmlElement> RecentDocumentList::createXML() const
{
    auto xml = std::make_unique<XmlElement> ("RECENT_DOCUMENTS");

    saveDocList (previousDocs, *xml->createNewChildElement ("PREVIOUS"));
    saveDocList (nextDocs,     *xml->createNewChildElement ("NEXT"));

    return xml;
}
