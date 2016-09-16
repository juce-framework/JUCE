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

#include "../jucer_Headers.h"
#include "../Application/jucer_Application.h"
#include "../Wizards/jucer_NewFileWizard.h"
#include "jucer_JucerDocument.h"
#include "jucer_ObjectTypes.h"
#include "ui/jucer_JucerDocumentEditor.h"
#include "ui/jucer_TestComponent.h"
#include "jucer_UtilityFunctions.h"
#include "documents/jucer_ComponentDocument.h"
#include "documents/jucer_ButtonDocument.h"

const char* const defaultClassName = "NewComponent";
const char* const defaultParentClasses = "public Component";

//==============================================================================
JucerDocument::JucerDocument (SourceCodeDocument* c)
    : cpp (c),
      className (defaultClassName),
      parentClasses (defaultParentClasses),
      fixedSize (false),
      initialWidth (600),
      initialHeight (400),
      snapGridPixels (8),
      snapActive (true),
      snapShown (true),
      componentOverlayOpacity (0.33f)
{
    jassert (cpp != nullptr);
    resources.setDocument (this);

    ProjucerApplication::getCommandManager().commandStatusChanged();
    cpp->getCodeDocument().addListener (this);
    ProjucerApplication::getApp().openDocumentManager.addListener (this);
}

JucerDocument::~JucerDocument()
{
    ProjucerApplication::getApp().openDocumentManager.removeListener (this);
    cpp->getCodeDocument().removeListener (this);
    ProjucerApplication::getCommandManager().commandStatusChanged();
}

//==============================================================================
void JucerDocument::changed()
{
    sendChangeMessage();
    ProjucerApplication::getCommandManager().commandStatusChanged();
    startTimer (800);
}

struct UserDocChangeTimer  : public Timer
{
    UserDocChangeTimer (JucerDocument& d) : doc (d) {}
    void timerCallback() override       { doc.reloadFromDocument(); }

    JucerDocument& doc;
};

bool JucerDocument::documentAboutToClose (OpenDocumentManager::Document* doc)
{
    return doc != cpp;
}

void JucerDocument::userEditedCpp()
{
    if (userDocChangeTimer == nullptr)
        userDocChangeTimer = new UserDocChangeTimer (*this);

    userDocChangeTimer->startTimer (500);
}

void JucerDocument::beginTransaction()
{
    getUndoManager().beginNewTransaction();
}

void JucerDocument::beginTransaction (const String& name)
{
    getUndoManager().beginNewTransaction (name);
}

void JucerDocument::timerCallback()
{
    if (! Component::isMouseButtonDownAnywhere())
    {
        stopTimer();
        beginTransaction();

        flushChangesToDocuments (nullptr);
    }
}

void JucerDocument::codeDocumentTextInserted (const String&, int)   { userEditedCpp(); }
void JucerDocument::codeDocumentTextDeleted (int, int)              { userEditedCpp(); }

bool JucerDocument::perform (UndoableAction* const action, const String& actionName)
{
    return undoManager.perform (action, actionName);
}

void JucerDocument::refreshAllPropertyComps()
{
    if (ComponentLayout* l = getComponentLayout())
        l->getSelectedSet().changed();

    for (int i = getNumPaintRoutines(); --i >= 0;)
    {
        getPaintRoutine (i)->getSelectedElements().changed();
        getPaintRoutine (i)->getSelectedPoints().changed();
    }
}

//==============================================================================
void JucerDocument::setClassName (const String& newName)
{
    if (newName != className
         && CodeHelpers::makeValidIdentifier (newName, false, false, true).isNotEmpty())
    {
        className = CodeHelpers::makeValidIdentifier (newName, false, false, true);
        changed();
    }
}

void JucerDocument::setComponentName (const String& newName)
{
    if (newName != componentName)
    {
        componentName = newName;
        changed();
    }
}

void JucerDocument::setParentClasses (const String& classes)
{
    if (classes != parentClasses)
    {
        StringArray parentClassLines (getCleanedStringArray (StringArray::fromTokens (classes, ",", StringRef())));

        for (int i = parentClassLines.size(); --i >= 0;)
        {
            String s (parentClassLines[i]);
            String type;

            if (s.startsWith ("public ")
                || s.startsWith ("protected ")
                || s.startsWith ("private "))
            {
                type = s.upToFirstOccurrenceOf (" ", true, false);
                s = s.fromFirstOccurrenceOf (" ", false, false);

                if (s.trim().isEmpty())
                    type = s = String();
            }

            s = type + CodeHelpers::makeValidIdentifier (s.trim(), false, false, true);

            parentClassLines.set (i, s);
        }

        parentClasses = parentClassLines.joinIntoString (", ");
        changed();
    }
}

void JucerDocument::setConstructorParams (const String& newParams)
{
    if (constructorParams != newParams)
    {
        constructorParams = newParams;
        changed();
    }
}

void JucerDocument::setVariableInitialisers (const String& newInitlialisers)
{
    if (variableInitialisers != newInitlialisers)
    {
        variableInitialisers = newInitlialisers;
        changed();
    }
}

void JucerDocument::setFixedSize (const bool isFixed)
{
    if (fixedSize != isFixed)
    {
        fixedSize = isFixed;
        changed();
    }
}

void JucerDocument::setInitialSize (int w, int h)
{
    w = jmax (1, w);
    h = jmax (1, h);

    if (initialWidth != w || initialHeight != h)
    {
        initialWidth = w;
        initialHeight = h;
        changed();
    }
}

//==============================================================================
bool JucerDocument::isSnapActive (const bool disableIfCtrlKeyDown) const noexcept
{
    return snapActive != (disableIfCtrlKeyDown && ModifierKeys::getCurrentModifiers().isCtrlDown());
}

int JucerDocument::snapPosition (int pos) const noexcept
{
    if (isSnapActive (true))
    {
        jassert (snapGridPixels > 0);
        pos = ((pos + snapGridPixels * 1024 + snapGridPixels / 2) / snapGridPixels - 1024) * snapGridPixels;
    }

    return pos;
}

void JucerDocument::setSnappingGrid (const int numPixels, const bool active, const bool shown)
{
    if (numPixels != snapGridPixels
         || active != snapActive
         || shown != snapShown)
    {
        snapGridPixels = numPixels;
        snapActive = active;
        snapShown = shown;
        changed();
    }
}

void JucerDocument::setComponentOverlayOpacity (const float alpha)
{
    if (alpha != componentOverlayOpacity)
    {
        componentOverlayOpacity = alpha;
        changed();
    }
}

//==============================================================================
void JucerDocument::addMethod (const String& base, const String& returnVal, const String& method, const String& initialContent,
                               StringArray& baseClasses, StringArray& returnValues, StringArray& methods, StringArray& initialContents)
{
    baseClasses.add (base);
    returnValues.add (returnVal);
    methods.add (method);
    initialContents.add (initialContent);
}

void JucerDocument::getOptionalMethods (StringArray& baseClasses,
                                        StringArray& returnValues,
                                        StringArray& methods,
                                        StringArray& initialContents) const
{
    addMethod ("Component", "void", "visibilityChanged()", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("Component", "void", "moved()", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("Component", "void", "parentHierarchyChanged()", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("Component", "void", "parentSizeChanged()", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("Component", "void", "lookAndFeelChanged()", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("Component", "bool", "hitTest (int x, int y)", "return true;", baseClasses, returnValues, methods, initialContents);
    addMethod ("Component", "void", "broughtToFront()", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("Component", "void", "filesDropped (const StringArray& filenames, int mouseX, int mouseY)", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("Component", "void", "handleCommandMessage (int commandId)", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("Component", "void", "childrenChanged()", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("Component", "void", "enablementChanged()", "", baseClasses, returnValues, methods, initialContents);

    addMethod ("Component", "void", "mouseMove (const MouseEvent& e)", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("Component", "void", "mouseEnter (const MouseEvent& e)", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("Component", "void", "mouseExit (const MouseEvent& e)", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("Component", "void", "mouseDown (const MouseEvent& e)", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("Component", "void", "mouseDrag (const MouseEvent& e)", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("Component", "void", "mouseUp (const MouseEvent& e)", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("Component", "void", "mouseDoubleClick (const MouseEvent& e)", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("Component", "void", "mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel)", "", baseClasses, returnValues, methods, initialContents);

    addMethod ("Component", "bool", "keyPressed (const KeyPress& key)", "return false;  // Return true if your handler uses this key event, or false to allow it to be passed-on.", baseClasses, returnValues, methods, initialContents);
    addMethod ("Component", "bool", "keyStateChanged (const bool isKeyDown)", "return false;  // Return true if your handler uses this key event, or false to allow it to be passed-on.", baseClasses, returnValues, methods, initialContents);
    addMethod ("Component", "void", "modifierKeysChanged (const ModifierKeys& modifiers)", "", baseClasses, returnValues, methods, initialContents);

    addMethod ("Component", "void", "focusGained (FocusChangeType cause)", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("Component", "void", "focusLost (FocusChangeType cause)", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("Component", "void", "focusOfChildComponentChanged (FocusChangeType cause)", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("Component", "void", "modifierKeysChanged (const ModifierKeys& modifiers)", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("Component", "void", "inputAttemptWhenModal()", "", baseClasses, returnValues, methods, initialContents);
}

void JucerDocument::setOptionalMethodEnabled (const String& methodSignature, const bool enable)
{
    if (enable)
        activeExtraMethods.addIfNotAlreadyThere (methodSignature);
    else
        activeExtraMethods.removeString (methodSignature);

    changed();
}

bool JucerDocument::isOptionalMethodEnabled (const String& sig) const noexcept
{
    return activeExtraMethods.contains (sig);
}

void JucerDocument::addExtraClassProperties (PropertyPanel&)
{
}

//==============================================================================
const char* const JucerDocument::jucerCompXmlTag = "JUCER_COMPONENT";

XmlElement* JucerDocument::createXml() const
{
    XmlElement* doc = new XmlElement (jucerCompXmlTag);

    doc->setAttribute ("documentType", getTypeName());
    doc->setAttribute ("className", className);

    if (templateFile.trim().isNotEmpty())
        doc->setAttribute ("template", templateFile);

    doc->setAttribute ("componentName", componentName);
    doc->setAttribute ("parentClasses", parentClasses);
    doc->setAttribute ("constructorParams", constructorParams);
    doc->setAttribute ("variableInitialisers", variableInitialisers);
    doc->setAttribute ("snapPixels", snapGridPixels);
    doc->setAttribute ("snapActive", snapActive);
    doc->setAttribute ("snapShown", snapShown);
    doc->setAttribute ("overlayOpacity", String (componentOverlayOpacity, 3));
    doc->setAttribute ("fixedSize", fixedSize);
    doc->setAttribute ("initialWidth", initialWidth);
    doc->setAttribute ("initialHeight", initialHeight);

    if (activeExtraMethods.size() > 0)
    {
        XmlElement* extraMethods = new XmlElement ("METHODS");
        doc->addChildElement (extraMethods);

        for (int i = 0; i < activeExtraMethods.size(); ++i)
        {
            XmlElement* e = new XmlElement ("METHOD");
            extraMethods ->addChildElement (e);
            e->setAttribute ("name", activeExtraMethods[i]);
        }
    }

    return doc;
}

bool JucerDocument::loadFromXml (const XmlElement& xml)
{
    if (xml.hasTagName (jucerCompXmlTag)
         && getTypeName().equalsIgnoreCase (xml.getStringAttribute ("documentType")))
    {
        className = xml.getStringAttribute ("className", defaultClassName);
        templateFile = xml.getStringAttribute ("template", String());
        componentName = xml.getStringAttribute ("componentName", String());
        parentClasses = xml.getStringAttribute ("parentClasses", defaultParentClasses);
        constructorParams = xml.getStringAttribute ("constructorParams", String());
        variableInitialisers = xml.getStringAttribute ("variableInitialisers", String());

        fixedSize = xml.getBoolAttribute ("fixedSize", false);
        initialWidth = xml.getIntAttribute ("initialWidth", 300);
        initialHeight = xml.getIntAttribute ("initialHeight", 200);

        snapGridPixels = xml.getIntAttribute ("snapPixels", snapGridPixels);
        snapActive = xml.getBoolAttribute ("snapActive", snapActive);
        snapShown = xml.getBoolAttribute ("snapShown", snapShown);

        componentOverlayOpacity = (float) xml.getDoubleAttribute ("overlayOpacity", 0.0);

        activeExtraMethods.clear();

        if (XmlElement* const methods = xml.getChildByName ("METHODS"))
            forEachXmlChildElementWithTagName (*methods, e, "METHOD")
                activeExtraMethods.addIfNotAlreadyThere (e->getStringAttribute ("name"));

        activeExtraMethods.trim();
        activeExtraMethods.removeEmptyStrings();

        changed();
        getUndoManager().clearUndoHistory();
        return true;
    }

    return false;
}


//==============================================================================
void JucerDocument::fillInGeneratedCode (GeneratedCode& code) const
{
    code.className = className;
    code.componentName = componentName;
    code.parentClasses = parentClasses;
    code.constructorParams = constructorParams;
    code.initialisers.addLines (variableInitialisers);

    if (! componentName.isEmpty())
        code.constructorCode << "setName (" + quotedString (componentName, false) + ");\n";

    // call these now, just to make sure they're the first two methods in the list.
    code.getCallbackCode (String(), "void", "paint (Graphics& g)", false)
        << "//[UserPrePaint] Add your own custom painting code here..\n//[/UserPrePaint]\n\n";

    code.getCallbackCode (String(), "void", "resized()", false)
        << "//[UserPreResize] Add your own custom resize code here..\n//[/UserPreResize]\n\n";

    if (ComponentLayout* l = getComponentLayout())
        l->fillInGeneratedCode (code);

    fillInPaintCode (code);

    ScopedPointer<XmlElement> e (createXml());
    jassert (e != nullptr);
    code.jucerMetadata = e->createDocument ("", false, false);

    resources.fillInGeneratedCode (code);

    code.constructorCode
        << "\n//[UserPreSize]\n"
           "//[/UserPreSize]\n";

    if (initialWidth > 0 || initialHeight > 0)
        code.constructorCode << "\nsetSize (" << initialWidth << ", " << initialHeight << ");\n";

    code.getCallbackCode (String(), "void", "paint (Graphics& g)", false)
        << "//[UserPaint] Add your own custom painting code here..\n//[/UserPaint]";

    code.getCallbackCode (String(), "void", "resized()", false)
        << "//[UserResized] Add your own custom resize handling here..\n//[/UserResized]";

    // add optional methods
    StringArray baseClasses, returnValues, methods, initialContents;
    getOptionalMethods (baseClasses, returnValues, methods, initialContents);

    for (int i = 0; i < methods.size(); ++i)
    {
        if (isOptionalMethodEnabled (methods[i]))
        {
            String baseClassToAdd (baseClasses[i]);

            if (baseClassToAdd == "Component" || baseClassToAdd == "Button")
                baseClassToAdd.clear();

            String& s = code.getCallbackCode (baseClassToAdd, returnValues[i], methods[i], false);

            if (! s.contains ("//["))
            {
                String userCommentTag ("UserCode_");
                userCommentTag += methods[i].upToFirstOccurrenceOf ("(", false, false).trim();

                s << "\n//[" << userCommentTag << "] -- Add your code here...\n"
                  << initialContents[i];

                if (initialContents[i].isNotEmpty() && ! initialContents[i].endsWithChar ('\n'))
                    s << '\n';

                s << "//[/" << userCommentTag << "]\n";
            }
        }
    }
}

void JucerDocument::fillInPaintCode (GeneratedCode& code) const
{
    for (int i = 0; i < getNumPaintRoutines(); ++i)
        getPaintRoutine (i)
            ->fillInGeneratedCode (code, code.getCallbackCode (String(), "void", "paint (Graphics& g)", false));
}

void JucerDocument::setTemplateFile (const String& newFile)
{
    if (templateFile != newFile)
    {
        templateFile = newFile;
        changed();
    }
}

//==============================================================================
bool JucerDocument::findTemplateFiles (String& headerContent, String& cppContent) const
{
    if (templateFile.isNotEmpty())
    {
        const File f (getCppFile().getSiblingFile (templateFile));

        const File templateCpp (f.withFileExtension (".cpp"));
        const File templateH   (f.withFileExtension (".h"));

        headerContent = templateH.loadFileAsString();
        cppContent = templateCpp.loadFileAsString();

        if (headerContent.isNotEmpty() && cppContent.isNotEmpty())
            return true;
    }

    headerContent = BinaryData::jucer_ComponentTemplate_h;
    cppContent    = BinaryData::jucer_ComponentTemplate_cpp;
    return true;
}

static String fixLineEndings (const String& s)
{
    StringArray lines;
    lines.addLines (s);

    for (int i = 0; i < lines.size(); ++i)
        lines.set (i, lines[i].trimEnd());

    while (lines.size() > 0 && lines [lines.size() - 1].trim().isEmpty())
        lines.remove (lines.size() - 1);

    lines.add (String());

    return lines.joinIntoString ("\r\n");
}

bool JucerDocument::flushChangesToDocuments (Project* project)
{
    String headerTemplate, cppTemplate;
    if (! findTemplateFiles (headerTemplate, cppTemplate))
        return false;

    GeneratedCode generated (this);
    fillInGeneratedCode (generated);

    const File headerFile (getHeaderFile());
    generated.includeFilesCPP.insert (0, headerFile);

    OpenDocumentManager& odm = ProjucerApplication::getApp().openDocumentManager;

    if (SourceCodeDocument* header = dynamic_cast<SourceCodeDocument*> (odm.openFile (nullptr, headerFile)))
    {
        String existingHeader (header->getCodeDocument().getAllContent());
        String existingCpp (cpp->getCodeDocument().getAllContent());

        generated.applyToCode (headerTemplate, headerFile,
                               existingHeader, project);

        generated.applyToCode (cppTemplate, headerFile.withFileExtension (".cpp"),
                               existingCpp, project);

        headerTemplate = fixLineEndings (headerTemplate);
        cppTemplate    = fixLineEndings (cppTemplate);

        if (header->getCodeDocument().getAllContent() != headerTemplate)
            header->getCodeDocument().replaceAllContent (headerTemplate);

        if (cpp->getCodeDocument().getAllContent() != cppTemplate)
            cpp->getCodeDocument().replaceAllContent (cppTemplate);
    }

    userDocChangeTimer = nullptr;
    return true;
}

bool JucerDocument::reloadFromDocument()
{
    const String cppContent (cpp->getCodeDocument().getAllContent());

    ScopedPointer<XmlElement> newXML (pullMetaDataFromCppFile (cppContent));

    if (newXML == nullptr || ! newXML->hasTagName (jucerCompXmlTag))
        return false;

    if (currentXML != nullptr && currentXML->isEquivalentTo (newXML, true))
        return true;

    currentXML = newXML;
    stopTimer();

    resources.loadFromCpp (getCppFile(), cppContent);

    return loadFromXml (*currentXML);
}

XmlElement* JucerDocument::pullMetaDataFromCppFile (const String& cpp)
{
    StringArray lines;
    lines.addLines (cpp);

    const int startLine = indexOfLineStartingWith (lines, "BEGIN_JUCER_METADATA", 0);

    if (startLine > 0)
    {
        const int endLine = indexOfLineStartingWith (lines, "END_JUCER_METADATA", startLine);

        if (endLine > startLine)
            return XmlDocument::parse (lines.joinIntoString ("\n", startLine + 1,
                                                             endLine - startLine - 1));
    }

    return nullptr;
}

bool JucerDocument::isValidJucerCppFile (const File& f)
{
    if (f.hasFileExtension (".cpp"))
    {
        const ScopedPointer<XmlElement> xml (pullMetaDataFromCppFile (f.loadFileAsString()));
        return xml != nullptr && xml->hasTagName (jucerCompXmlTag);
    }

    return false;
}

static JucerDocument* createDocument (SourceCodeDocument* cpp)
{
    CodeDocument& codeDoc = cpp->getCodeDocument();

    ScopedPointer<XmlElement> xml (JucerDocument::pullMetaDataFromCppFile (codeDoc.getAllContent()));

    if (xml == nullptr || ! xml->hasTagName (JucerDocument::jucerCompXmlTag))
        return nullptr;

    const String docType (xml->getStringAttribute ("documentType"));

    ScopedPointer<JucerDocument> newDoc;

    if (docType.equalsIgnoreCase ("Button"))
        newDoc = new ButtonDocument (cpp);

    if (docType.equalsIgnoreCase ("Component") || docType.isEmpty())
        newDoc = new ComponentDocument (cpp);

    if (newDoc != nullptr && newDoc->reloadFromDocument())
        return newDoc.release();

    return nullptr;
}

JucerDocument* JucerDocument::createForCppFile (Project* p, const File& file)
{
    OpenDocumentManager& odm = ProjucerApplication::getApp().openDocumentManager;

    if (SourceCodeDocument* cpp = dynamic_cast<SourceCodeDocument*> (odm.openFile (p, file)))
        if (dynamic_cast<SourceCodeDocument*> (odm.openFile (p, file.withFileExtension (".h"))) != nullptr)
            return createDocument (cpp);

    return nullptr;
}

//==============================================================================
class JucerComponentDocument  : public SourceCodeDocument
{
public:
    JucerComponentDocument (Project* p, const File& f)
        : SourceCodeDocument (p, f)
    {
    }

    bool save() override
    {
        return SourceCodeDocument::save() && saveHeader();
    }

    bool saveHeader()
    {
        OpenDocumentManager& odm = ProjucerApplication::getApp().openDocumentManager;

        if (OpenDocumentManager::Document* header = odm.openFile (nullptr, getFile().withFileExtension (".h")))
            return header->save();

        return false;
    }

    Component* createEditor() override
    {
        ScopedPointer<JucerDocument> jucerDoc (JucerDocument::createForCppFile (getProject(), getFile()));

        if (jucerDoc != nullptr)
            return new JucerDocumentEditor (jucerDoc.release());

        return SourceCodeDocument::createEditor();
    }

    struct Type  : public OpenDocumentManager::DocumentType
    {
        Type() {}

        bool canOpenFile (const File& f) override                { return JucerDocument::isValidJucerCppFile (f); }
        Document* openFile (Project* p, const File& f) override  { return new JucerComponentDocument (p, f); }
    };
};

OpenDocumentManager::DocumentType* createGUIDocumentType()
{
    return new JucerComponentDocument::Type();
}

//==============================================================================
class NewGUIComponentWizard  : public NewFileWizard::Type
{
public:
    NewGUIComponentWizard() {}

    String getName() override  { return "GUI Component"; }

    void createNewFile (Project& project, Project::Item parent) override
    {
        const File newFile (askUserToChooseNewFile (String (defaultClassName) + ".h", "*.h;*.cpp", parent));

        if (newFile != File())
        {
            const File headerFile (newFile.withFileExtension (".h"));
            const File cppFile (newFile.withFileExtension (".cpp"));

            headerFile.replaceWithText (String());
            cppFile.replaceWithText (String());

            OpenDocumentManager& odm = ProjucerApplication::getApp().openDocumentManager;

            if (SourceCodeDocument* cpp = dynamic_cast<SourceCodeDocument*> (odm.openFile (nullptr, cppFile)))
            {
                if (SourceCodeDocument* header = dynamic_cast<SourceCodeDocument*> (odm.openFile (nullptr, headerFile)))
                {
                    ScopedPointer<JucerDocument> jucerDoc (new ComponentDocument (cpp));

                    if (jucerDoc != nullptr)
                    {
                        jucerDoc->setClassName (newFile.getFileNameWithoutExtension());

                        jucerDoc->flushChangesToDocuments (&project);
                        jucerDoc = nullptr;

                        cpp->save();
                        header->save();
                        odm.closeDocument (cpp, true);
                        odm.closeDocument (header, true);

                        parent.addFileRetainingSortOrder (headerFile, true);
                        parent.addFileRetainingSortOrder (cppFile, true);
                    }
                }
            }
        }
    }
};

NewFileWizard::Type* createGUIComponentWizard()
{
    return new NewGUIComponentWizard();
}
