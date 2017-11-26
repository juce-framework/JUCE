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
    bool save() override                                     { return true; }
    bool saveAs() override                                   { return false; }
    bool hasFileBeenModifiedExternally() override            { return fileModificationTime != file.getLastModificationTime(); }
    void reloadFromFile() override                           { fileModificationTime = file.getLastModificationTime(); }
    String getName() const override                          { return file.getFileName(); }
    File getFile() const override                            { return file; }
    Component* createEditor() override                       { return new ItemPreviewComponent (file); }
    Component* createViewer() override                       { return createEditor(); }
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

FileBasedDocument::SaveResult OpenDocumentManager::saveIfNeededAndUserAgrees (OpenDocumentManager::Document* doc)
{
    if (! doc->needsSaving())
        return FileBasedDocument::savedOk;

    const int r = AlertWindow::showYesNoCancelBox (AlertWindow::QuestionIcon,
                                                   TRANS("Closing document..."),
                                                   TRANS("Do you want to save the changes to \"")
                                                       + doc->getName() + "\"?",
                                                   TRANS("Save"),
                                                   TRANS("Discard changes"),
                                                   TRANS("Cancel"));

    if (r == 1)  // save changes
        return doc->save() ? FileBasedDocument::savedOk
                           : FileBasedDocument::failedToWriteToFile;

    if (r == 2)  // discard changes
        return FileBasedDocument::savedOk;

    return FileBasedDocument::userCancelledSave;
}


bool OpenDocumentManager::closeDocument (int index, bool saveIfNeeded)
{
    if (Document* doc = documents [index])
    {
        if (saveIfNeeded)
            if (saveIfNeededAndUserAgrees (doc) != FileBasedDocument::savedOk)
                return false;

        bool canClose = true;

        for (int i = listeners.size(); --i >= 0;)
            if (DocumentCloseListener* l = listeners[i])
                if (! l->documentAboutToClose (doc))
                    canClose = false;

        if (! canClose)
            return false;

        documents.remove (index);
        ProjucerApplication::getCommandManager().commandStatusChanged();
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
        if (Document* d = documents[i])
            if (d->isForFile (f))
                closeDocument (i, saveIfNeeded);
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
        if (Document* d = documents[i])
            if (d->refersToProject (project))
                if (! closeDocument (i, saveIfNeeded))
                    return false;

    return true;
}

bool OpenDocumentManager::anyFilesNeedSaving() const
{
    for (int i = documents.size(); --i >= 0;)
        if (documents.getUnchecked (i)->needsSaving())
            return true;

    return false;
}

bool OpenDocumentManager::saveAll()
{
    for (int i = documents.size(); --i >= 0;)
    {
        if (! documents.getUnchecked (i)->save())
            return false;

        ProjucerApplication::getCommandManager().commandStatusChanged();
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

        forEachXmlChildElementWithTagName (*xml, e, "DOC")
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

XmlElement* RecentDocumentList::createXML() const
{
    XmlElement* xml = new XmlElement ("RECENT_DOCUMENTS");

    saveDocList (previousDocs, *xml->createNewChildElement ("PREVIOUS"));
    saveDocList (nextDocs,     *xml->createNewChildElement ("NEXT"));

    return xml;
}
