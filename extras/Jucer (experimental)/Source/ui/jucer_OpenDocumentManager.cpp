/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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
#include "Code Editor/jucer_SourceCodeEditor.h"
#include "Drawable Editor/jucer_DrawableEditor.h"
#include "Project Editor/jucer_ItemPreviewComponent.h"
#include "Component Editor/jucer_ComponentEditor.h"


//==============================================================================
class SourceCodeDocument  : public OpenDocumentManager::Document
{
public:
    SourceCodeDocument (const File& file_)
        : modDetector (file_)
    {
        codeDoc = new CodeDocument();
        reloadFromFile();
    }

    ~SourceCodeDocument()
    {
    }

    bool loadedOk() const                               { return true; }
    bool isForFile (const File& file) const             { return modDetector.getFile() == file; }
    bool isForNode (const ValueTree& node) const        { return false; }
    bool refersToProject (Project& project) const       { return false; }
    const String getName() const                        { return modDetector.getFile().getFileName(); }
    const String getType() const                        { return modDetector.getFile().getFileExtension() + " file"; }
    bool needsSaving() const                            { return codeDoc != 0 && codeDoc->hasChangedSinceSavePoint(); }
    bool hasFileBeenModifiedExternally()                { return modDetector.hasBeenModified(); }
    void fileHasBeenRenamed (const File& newFile)       { modDetector.fileHasBeenRenamed (newFile); }

    void reloadFromFile()
    {
        modDetector.updateHash();

        ScopedPointer <InputStream> in (modDetector.getFile().createInputStream());

        if (in != 0)
            codeDoc->loadFromStream (*in);
    }

    bool save()
    {
        TemporaryFile temp (modDetector.getFile());
        ScopedPointer <FileOutputStream> out (temp.getFile().createOutputStream());

        if (out == 0 || ! codeDoc->writeToStream (*out))
            return false;

        out = 0;
        if (! temp.overwriteTargetFileWithTemporary())
            return false;

        modDetector.updateHash();
        return true;
    }

    Component* createEditor()
    {
        CodeTokeniser* tokeniser = 0;

        if (SourceCodeEditor::isCppFile (modDetector.getFile()))
            tokeniser = &cppTokeniser;

        return new SourceCodeEditor (this, *codeDoc, tokeniser);
    }

private:
    FileModificationDetector modDetector;
    ScopedPointer <CodeDocument> codeDoc;
    CPlusPlusCodeTokeniser cppTokeniser;
};

//==============================================================================
class ComponentDocumentType  : public OpenDocumentManager::Document
{
public:
    ComponentDocumentType (Project* project_, const File& file_)
        : project (project_),
          modDetector (file_)
    {
        reloadFromFile();
    }

    ~ComponentDocumentType()
    {
        componentDoc = 0;
    }

    static bool isComponentFile (const File& file)  { return ComponentDocument::isComponentFile (file); }

    bool loadedOk() const                           { return componentDoc != 0; }
    bool isForFile (const File& file) const         { return modDetector.getFile() == file; }
    bool isForNode (const ValueTree& node) const    { return false; }
    bool refersToProject (Project& p) const         { return project == &p; }
    const String getType() const                    { return "Jucer Component"; }
    const String getName() const                    { return modDetector.getFile().getFileName(); }
    bool needsSaving() const                        { return componentDoc != 0 && componentDoc->hasChangedSinceLastSave(); }
    bool hasFileBeenModifiedExternally()            { return modDetector.hasBeenModified(); }

    void fileHasBeenRenamed (const File& newFile)
    {
        if (componentDoc != 0)
            componentDoc->cppFileHasMoved (newFile);

        modDetector.fileHasBeenRenamed (newFile);
    }

    void reloadFromFile()
    {
        modDetector.updateHash();

        if (componentDoc == 0)
            componentDoc = new ComponentDocument (project, modDetector.getFile());

        if (! componentDoc->reload())
            componentDoc = 0;
    }

    bool save()
    {
        if (componentDoc->save())
        {
            modDetector.updateHash();
            return true;
        }

        return false;
    }

    Component* createEditor()
    {
        if (componentDoc == 0)
        {
            jassertfalse;
            return 0;
        }

        return new ComponentEditor (this, project, componentDoc);
    }

private:
    Project* project;
    FileModificationDetector modDetector;
    ScopedPointer <ComponentDocument> componentDoc;
};

//==============================================================================
class DrawableDocumentType  : public OpenDocumentManager::Document
{
public:
    DrawableDocumentType (Project* project_, const File& file_)
        : project (project_),
          modDetector (file_)
    {
        reloadFromFile();
    }

    ~DrawableDocumentType()
    {
        drawableDoc = 0;
    }

    static bool isDrawableFile (const File& file)   { return file.hasFileExtension (".drawable"); }

    bool loadedOk() const                           { return drawableDoc != 0; }
    bool isForFile (const File& file) const         { return modDetector.getFile() == file; }
    bool isForNode (const ValueTree& node) const    { return false; }
    bool refersToProject (Project& p) const         { return project == &p; }
    const String getType() const                    { return "Drawable"; }
    const String getName() const                    { return modDetector.getFile().getFileName(); }
    bool needsSaving() const                        { return drawableDoc != 0 && drawableDoc->hasChangedSinceLastSave(); }
    bool hasFileBeenModifiedExternally()            { return modDetector.hasBeenModified(); }
    void fileHasBeenRenamed (const File& newFile)   { modDetector.fileHasBeenRenamed (newFile); }

    void reloadFromFile()
    {
        modDetector.updateHash();

        if (drawableDoc == 0)
            drawableDoc = new DrawableDocument (project);

        if (! drawableDoc->reload (modDetector.getFile()))
            drawableDoc = 0;
    }

    bool save()
    {
        if (drawableDoc->save (modDetector.getFile()))
        {
            modDetector.updateHash();
            return true;
        }

        return false;
    }

    Component* createEditor()
    {
        jassert (drawableDoc != 0);

        if (drawableDoc == 0)
            return 0;

        return new DrawableEditor (this, project, drawableDoc);
    }

private:
    Project* project;
    FileModificationDetector modDetector;
    ScopedPointer <DrawableDocument> drawableDoc;
};

//==============================================================================
class UnknownDocument  : public OpenDocumentManager::Document
{
public:
    UnknownDocument (Project* project_, const File& file_)
       : project (project_), file (file_)
    {
        reloadFromFile();
    }

    ~UnknownDocument() {}

    bool loadedOk() const                           { return true; }
    bool isForFile (const File& file_) const        { return file == file_; }
    bool isForNode (const ValueTree& node_) const   { return false; }
    bool refersToProject (Project& p) const         { return project == &p; }
    bool needsSaving() const                        { return false; }
    bool save()                                     { return true; }
    bool hasFileBeenModifiedExternally()            { return fileModificationTime != file.getLastModificationTime(); }
    void reloadFromFile()                           { fileModificationTime = file.getLastModificationTime(); }
    const String getName() const                    { return file.getFileName(); }
    Component* createEditor()                       { return new ItemPreviewComponent (file); }
    void fileHasBeenRenamed (const File& newFile)   { file = newFile; }

    const String getType() const
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

    UnknownDocument (const UnknownDocument&);
    UnknownDocument& operator= (const UnknownDocument&);
};


//==============================================================================
OpenDocumentManager::OpenDocumentManager()
{
}

OpenDocumentManager::~OpenDocumentManager()
{
    clearSingletonInstance();
}

juce_ImplementSingleton_SingleThreaded (OpenDocumentManager);

//==============================================================================
void OpenDocumentManager::registerEditor (DocumentEditorComponent* editor)
{
    editors.add (editor);
}

void OpenDocumentManager::deregisterEditor (DocumentEditorComponent* editor)
{
    editors.removeValue (editor);
}

//==============================================================================
bool OpenDocumentManager::canOpenFile (const File& file)
{
    return DrawableDocumentType::isDrawableFile (file)
            || SourceCodeEditor::isTextFile (file);
}

OpenDocumentManager::Document* OpenDocumentManager::getDocumentForFile (Project* project, const File& file)
{
    for (int i = documents.size(); --i >= 0;)
        if (documents.getUnchecked(i)->isForFile (file))
            return documents.getUnchecked(i);

    Document* d = 0;

    if (ComponentDocumentType::isComponentFile (file))
        d = new ComponentDocumentType (project, file);
    else if (DrawableDocumentType::isDrawableFile (file))
        d = new DrawableDocumentType (project, file);
    else if (SourceCodeEditor::isTextFile (file))
        d = new SourceCodeDocument (file);
    else
        d = new UnknownDocument (project, file);

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

    if (doc != 0)
    {
        if (saveIfNeeded)
        {
            if (saveIfNeededAndUserAgrees (doc) != FileBasedDocument::savedOk)
                return false;
        }

        for (int i = editors.size(); --i >= 0;)
            if (editors.getUnchecked(i)->getDocument() == doc)
                editors.getUnchecked(i)->deleteSelf();

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
