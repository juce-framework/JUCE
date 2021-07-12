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
#include "../Application/jucer_Application.h"
#include "../Utility/Helpers/jucer_NewFileWizard.h"
#include "jucer_JucerDocument.h"
#include "jucer_ObjectTypes.h"
#include "UI/jucer_JucerDocumentEditor.h"
#include "UI/jucer_TestComponent.h"
#include "jucer_UtilityFunctions.h"
#include "Documents/jucer_ComponentDocument.h"
#include "Documents/jucer_ButtonDocument.h"

const char* const defaultClassName = "NewComponent";
const char* const defaultParentClasses = "public juce::Component";

//==============================================================================
JucerDocument::JucerDocument (SourceCodeDocument* c)
    : cpp (c),
      className (defaultClassName),
      parentClasses (defaultParentClasses)
{
    jassert (cpp != nullptr);
    resources.setDocument (this);

    ProjucerApplication::getCommandManager().commandStatusChanged();
    cpp->getCodeDocument().addListener (this);
}

JucerDocument::~JucerDocument()
{
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
    explicit UserDocChangeTimer (JucerDocument& d) : doc (d) {}
    void timerCallback() override       { doc.reloadFromDocument(); }

    JucerDocument& doc;
};

void JucerDocument::userEditedCpp()
{
    if (userDocChangeTimer == nullptr)
        userDocChangeTimer.reset (new UserDocChangeTimer (*this));

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

        flushChangesToDocuments (nullptr, false);
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
        && build_tools::makeValidIdentifier (newName, false, false, true).isNotEmpty())
    {
        className = build_tools::makeValidIdentifier (newName, false, false, true);
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

            s = type + build_tools::makeValidIdentifier (s.trim(), false, false, true, true);

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
    return snapActive != (disableIfCtrlKeyDown && ModifierKeys::currentModifiers.isCtrlDown());
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
    addMethod ("juce::Component", "void", "visibilityChanged()", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("juce::Component", "void", "moved()", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("juce::Component", "void", "parentHierarchyChanged()", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("juce::Component", "void", "parentSizeChanged()", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("juce::Component", "void", "lookAndFeelChanged()", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("juce::Component", "bool", "hitTest (int x, int y)", "return true;", baseClasses, returnValues, methods, initialContents);
    addMethod ("juce::Component", "void", "broughtToFront()", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("juce::Component", "void", "filesDropped (const juce::StringArray& filenames, int mouseX, int mouseY)", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("juce::Component", "void", "handleCommandMessage (int commandId)", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("juce::Component", "void", "childrenChanged()", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("juce::Component", "void", "enablementChanged()", "", baseClasses, returnValues, methods, initialContents);

    addMethod ("juce::Component", "void", "mouseMove (const juce::MouseEvent& e)", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("juce::Component", "void", "mouseEnter (const juce::MouseEvent& e)", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("juce::Component", "void", "mouseExit (const juce::MouseEvent& e)", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("juce::Component", "void", "mouseDown (const juce::MouseEvent& e)", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("juce::Component", "void", "mouseDrag (const juce::MouseEvent& e)", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("juce::Component", "void", "mouseUp (const juce::MouseEvent& e)", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("juce::Component", "void", "mouseDoubleClick (const juce::MouseEvent& e)", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("juce::Component", "void", "mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)", "", baseClasses, returnValues, methods, initialContents);

    addMethod ("juce::Component", "bool", "keyPressed (const juce::KeyPress& key)", "return false;  // Return true if your handler uses this key event, or false to allow it to be passed-on.", baseClasses, returnValues, methods, initialContents);
    addMethod ("juce::Component", "bool", "keyStateChanged (bool isKeyDown)", "return false;  // Return true if your handler uses this key event, or false to allow it to be passed-on.", baseClasses, returnValues, methods, initialContents);
    addMethod ("juce::Component", "void", "modifierKeysChanged (const juce::ModifierKeys& modifiers)", "", baseClasses, returnValues, methods, initialContents);

    addMethod ("juce::Component", "void", "focusGained (FocusChangeType cause)", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("juce::Component", "void", "focusLost (FocusChangeType cause)", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("juce::Component", "void", "focusOfChildComponentChanged (FocusChangeType cause)", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("juce::Component", "void", "modifierKeysChanged (const juce::ModifierKeys& modifiers)", "", baseClasses, returnValues, methods, initialContents);
    addMethod ("juce::Component", "void", "inputAttemptWhenModal()", "", baseClasses, returnValues, methods, initialContents);
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
    return activeExtraMethods.contains (sig)
          || activeExtraMethods.contains (sig.replace ("juce::", {}));
}

void JucerDocument::addExtraClassProperties (PropertyPanel&)
{
}

//==============================================================================
const char* const JucerDocument::jucerCompXmlTag = "JUCER_COMPONENT";

std::unique_ptr<XmlElement> JucerDocument::createXml() const
{
    auto doc = std::make_unique<XmlElement> (jucerCompXmlTag);

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
            for (auto* e : methods->getChildWithTagNameIterator ("METHOD"))
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
    code.getCallbackCode (String(), "void", "paint (juce::Graphics& g)", false)
        << "//[UserPrePaint] Add your own custom painting code here..\n//[/UserPrePaint]\n\n";

    code.getCallbackCode (String(), "void", "resized()", false)
        << "//[UserPreResize] Add your own custom resize code here..\n//[/UserPreResize]\n\n";

    if (ComponentLayout* l = getComponentLayout())
        l->fillInGeneratedCode (code);

    fillInPaintCode (code);

    std::unique_ptr<XmlElement> e (createXml());
    jassert (e != nullptr);
    code.jucerMetadata = e->toString (XmlElement::TextFormat().withoutHeader());

    resources.fillInGeneratedCode (code);

    code.constructorCode
        << "\n//[UserPreSize]\n"
           "//[/UserPreSize]\n";

    if (initialWidth > 0 || initialHeight > 0)
        code.constructorCode << "\nsetSize (" << initialWidth << ", " << initialHeight << ");\n";

    code.getCallbackCode (String(), "void", "paint (juce::Graphics& g)", false)
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

            if (baseClassToAdd == "juce::Component" || baseClassToAdd == "juce::Button")
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
            ->fillInGeneratedCode (code, code.getCallbackCode (String(), "void", "paint (juce::Graphics& g)", false));
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

bool JucerDocument::flushChangesToDocuments (Project* project, bool isInitial)
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

        generated.applyToCode (headerTemplate, headerFile, existingHeader);
        generated.applyToCode (cppTemplate, headerFile.withFileExtension (".cpp"), existingCpp);

        if (isInitial)
        {
            jassert (project != nullptr);

            auto lineFeed = project->getProjectLineFeed();

            headerTemplate = replaceLineFeeds (headerTemplate, lineFeed);
            cppTemplate    = replaceLineFeeds (cppTemplate,    lineFeed);
        }
        else
        {
            headerTemplate = replaceLineFeeds (headerTemplate, getLineFeedForFile (existingHeader));
            cppTemplate    = replaceLineFeeds (cppTemplate,    getLineFeedForFile (existingCpp));
        }

        if (header->getCodeDocument().getAllContent() != headerTemplate)
            header->getCodeDocument().replaceAllContent (headerTemplate);

        if (cpp->getCodeDocument().getAllContent() != cppTemplate)
            cpp->getCodeDocument().replaceAllContent (cppTemplate);
    }

    userDocChangeTimer.reset();
    return true;
}

bool JucerDocument::reloadFromDocument()
{
    const String cppContent (cpp->getCodeDocument().getAllContent());

    std::unique_ptr<XmlElement> newXML (pullMetaDataFromCppFile (cppContent));

    if (newXML == nullptr || ! newXML->hasTagName (jucerCompXmlTag))
        return false;

    if (currentXML != nullptr && currentXML->isEquivalentTo (newXML.get(), true))
        return true;

    currentXML.reset (newXML.release());
    stopTimer();

    resources.loadFromCpp (getCppFile(), cppContent);

    bool result = loadFromXml (*currentXML);
    extractCustomPaintSnippetsFromCppFile (cppContent);
    return result;
}

void JucerDocument::refreshCustomCodeFromDocument()
{
    const String cppContent (cpp->getCodeDocument().getAllContent());
    extractCustomPaintSnippetsFromCppFile (cppContent);
}

void JucerDocument::extractCustomPaintSnippetsFromCppFile (const String& cppContent)
{
    StringArray customPaintSnippets;

    auto lines = StringArray::fromLines (cppContent);
    int last = 0;

    while (last >= 0)
    {
        const int start = indexOfLineStartingWith (lines, "//[UserPaintCustomArguments]", last);
        if (start < 0)
            break;

        const int end = indexOfLineStartingWith (lines, "//[/UserPaintCustomArguments]", start);
        if (end < 0)
            break;

        last = end + 1;
        String result;

        for (int i = start + 1; i < end; ++i)
            result << lines [i] << newLine;

        customPaintSnippets.add (CodeHelpers::unindent (result, 4));
    }

    applyCustomPaintSnippets (customPaintSnippets);
}

std::unique_ptr<XmlElement> JucerDocument::pullMetaDataFromCppFile (const String& cpp)
{
    auto lines = StringArray::fromLines (cpp);
    auto startLine = indexOfLineStartingWith (lines, "BEGIN_JUCER_METADATA", 0);

    if (startLine > 0)
    {
        auto endLine = indexOfLineStartingWith (lines, "END_JUCER_METADATA", startLine);

        if (endLine > startLine)
            return parseXML (lines.joinIntoString ("\n", startLine + 1, endLine - startLine - 1));
    }

    return nullptr;
}

bool JucerDocument::isValidJucerCppFile (const File& f)
{
    if (f.hasFileExtension (cppFileExtensions))
    {
        std::unique_ptr<XmlElement> xml (pullMetaDataFromCppFile (f.loadFileAsString()));

        if (xml != nullptr)
            return xml->hasTagName (jucerCompXmlTag);
    }

    return false;
}

static JucerDocument* createDocument (SourceCodeDocument* cpp)
{
    auto& codeDoc = cpp->getCodeDocument();

    std::unique_ptr<XmlElement> xml (JucerDocument::pullMetaDataFromCppFile (codeDoc.getAllContent()));

    if (xml == nullptr || ! xml->hasTagName (JucerDocument::jucerCompXmlTag))
        return nullptr;

    const String docType (xml->getStringAttribute ("documentType"));

    std::unique_ptr<JucerDocument> newDoc;

    if (docType.equalsIgnoreCase ("Button"))
        newDoc.reset (new ButtonDocument (cpp));

    if (docType.equalsIgnoreCase ("Component") || docType.isEmpty())
        newDoc.reset (new ComponentDocument (cpp));

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

    void saveAsync (std::function<void (bool)> callback) override
    {
        SourceCodeDocument::saveAsync ([parent = WeakReference<JucerComponentDocument> { this }, callback] (bool saveResult)
        {
            if (parent == nullptr)
                return;

            if (! saveResult)
            {
                callback (false);
                return;
            }

            parent->saveHeaderAsync ([parent, callback] (bool headerSaveResult)
            {
                if (parent != nullptr)
                    callback (headerSaveResult);
            });
        });
    }

    void saveHeaderAsync (std::function<void (bool)> callback)
    {
        auto& odm = ProjucerApplication::getApp().openDocumentManager;

        if (auto* header = odm.openFile (nullptr, getFile().withFileExtension (".h")))
        {
            header->saveAsync ([parent = WeakReference<JucerComponentDocument> { this }, callback] (bool saveResult)
            {
                if (parent == nullptr)
                    return;

                if (saveResult)
                    ProjucerApplication::getApp()
                        .openDocumentManager
                        .closeFileWithoutSaving (parent->getFile().withFileExtension (".h"));

                callback (saveResult);
            });

            return;
        }

        callback (false);
    }

    std::unique_ptr<Component> createEditor() override
    {
        if (ProjucerApplication::getApp().isGUIEditorEnabled())
        {
            std::unique_ptr<JucerDocument> jucerDoc (JucerDocument::createForCppFile (getProject(), getFile()));

            if (jucerDoc != nullptr)
                return std::make_unique<JucerDocumentEditor> (jucerDoc.release());
        }

        return SourceCodeDocument::createEditor();
    }

    struct Type  : public OpenDocumentManager::DocumentType
    {
        Type() {}

        bool canOpenFile (const File& f) override                { return JucerDocument::isValidJucerCppFile (f); }
        Document* openFile (Project* p, const File& f) override  { return new JucerComponentDocument (p, f); }
    };

    JUCE_DECLARE_WEAK_REFERENCEABLE (JucerComponentDocument)
};

OpenDocumentManager::DocumentType* createGUIDocumentType();
OpenDocumentManager::DocumentType* createGUIDocumentType()
{
    return new JucerComponentDocument::Type();
}

//==============================================================================
struct NewGUIComponentWizard  : public NewFileWizard::Type
{
    NewGUIComponentWizard (Project& proj)
        : project (proj)
    {}

    String getName() override  { return "GUI Component"; }

    void createNewFile (Project& p, Project::Item parent) override
    {
        jassert (&p == &project);

        askUserToChooseNewFile (String (defaultClassName) + ".h", "*.h;*.cpp", parent, [this, parent] (File newFile) mutable
        {
            if (newFile != File())
            {
                auto headerFile = newFile.withFileExtension (".h");
                auto cppFile = newFile.withFileExtension (".cpp");

                headerFile.replaceWithText (String());
                cppFile.replaceWithText (String());

                auto& odm = ProjucerApplication::getApp().openDocumentManager;

                if (auto* cpp = dynamic_cast<SourceCodeDocument*> (odm.openFile (&project, cppFile)))
                {
                    if (auto* header = dynamic_cast<SourceCodeDocument*> (odm.openFile (&project, headerFile)))
                    {
                        std::unique_ptr<JucerDocument> jucerDoc (new ComponentDocument (cpp));

                        if (jucerDoc != nullptr)
                        {
                            jucerDoc->setClassName (newFile.getFileNameWithoutExtension());

                            jucerDoc->flushChangesToDocuments (&project, true);
                            jucerDoc.reset();

                            for (auto* doc : { cpp, header })
                            {
                                doc->saveAsync ([doc] (bool)
                                {
                                    ProjucerApplication::getApp()
                                        .openDocumentManager
                                        .closeDocumentAsync (doc, OpenDocumentManager::SaveIfNeeded::yes, nullptr);
                                });
                            }

                            parent.addFileRetainingSortOrder (headerFile, true);
                            parent.addFileRetainingSortOrder (cppFile, true);
                        }
                    }
                }
            }
        });
    }

    Project& project;
};

NewFileWizard::Type* createGUIComponentWizard (Project&);
NewFileWizard::Type* createGUIComponentWizard (Project& p)
{
    return new NewGUIComponentWizard (p);
}
