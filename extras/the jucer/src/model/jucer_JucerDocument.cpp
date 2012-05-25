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

#include "../jucer_Headers.h"
#include "jucer_JucerDocument.h"
#include "jucer_ObjectTypes.h"
#include "../ui/jucer_TestComponent.h"

const char* const defaultClassName = "NewJucerComponent";
const char* const defaultParentClasses = "public Component";

static const int timerInterval = 150;

//==============================================================================
JucerDocument::JucerDocument()
    : FileBasedDocument (".cpp", "*.cpp",
                         "Open a Jucer C++ file...",
                         "Save as a Jucer C++ file..."),
      className (defaultClassName),
      parentClasses (defaultParentClasses),
      fixedSize (false),
      initialWidth (600),
      initialHeight (400),
      snapGridPixels (8),
      snapActive (true),
      snapShown (true),
      lastFocusedComp (0),
      lastClickCounter (0),
      componentOverlayOpacity (0.33f)
{
    resources.setDocument (this);

    startTimer (timerInterval);
    commandManager->commandStatusChanged();
}

JucerDocument::~JucerDocument()
{
    commandManager->commandStatusChanged();
}

//==============================================================================
void JucerDocument::changed()
{
    FileBasedDocument::changed();
    commandManager->commandStatusChanged();
}

void JucerDocument::timerCallback()
{
    if ((lastFocusedComp != Component::getCurrentlyFocusedComponent()
          || lastClickCounter != Desktop::getMouseButtonClickCounter())
        && ! Component::isMouseButtonDownAnywhere())
    {
        lastFocusedComp = Component::getCurrentlyFocusedComponent();
        lastClickCounter = Desktop::getMouseButtonClickCounter();

        getUndoManager().beginNewTransaction();
    }
}

bool JucerDocument::perform (UndoableAction* const action, const String& actionName)
{
    startTimer (timerInterval);
    return undoManager.perform (action, actionName);
}

void JucerDocument::refreshAllPropertyComps()
{
    if (getComponentLayout() != 0)
        getComponentLayout()->getSelectedSet().changed();

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
         && makeValidCppIdentifier (newName, false, false, true).isNotEmpty())
    {
        className = makeValidCppIdentifier (newName, false, false, true);
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

const String JucerDocument::getParentClassString() const
{
    return parentClasses;
}

void JucerDocument::setParentClasses (const String& classes)
{
    if (classes != parentClasses)
    {
        StringArray parentClassLines;
        parentClassLines.addTokens (classes, ",", String::empty);
        parentClassLines.trim();
        parentClassLines.removeEmptyStrings();
        parentClassLines.removeDuplicates (false);

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
                    type = s = String::empty;
            }

            s = type + makeValidCppIdentifier (s.trim(), false, false, true);

            parentClassLines.set (i, s);
        }

        parentClasses = parentClassLines.joinIntoString (", ");
        changed();
    }
}

const String JucerDocument::getConstructorParams() const
{
    return constructorParams;
}

void JucerDocument::setConstructorParams (const String& newParams)
{
    if (constructorParams != newParams)
    {
        constructorParams = newParams;
        changed();
    }
}

const String JucerDocument::getVariableInitialisers() const
{
    return variableInitialisers;
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
bool JucerDocument::isSnapActive (const bool disableIfCtrlKeyDown) const throw()
{
    return snapActive != (disableIfCtrlKeyDown && ModifierKeys::getCurrentModifiers().isCtrlDown());
}

int JucerDocument::snapPosition (int pos) const throw()
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


//==============================================================================
void JucerDocument::setComponentOverlayOpacity (const float alpha)
{
    if (alpha != componentOverlayOpacity)
    {
        componentOverlayOpacity = alpha;
        changed();
    }
}

//==============================================================================
String JucerDocument::getDocumentTitle()
{
    return className;
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

void JucerDocument::setOptionalMethodEnabled (const String& methodSigniture, const bool enable)
{
    if (enable)
        activeExtraMethods.addIfNotAlreadyThere (methodSigniture);
    else
        activeExtraMethods.removeString (methodSigniture);

    changed();
}

bool JucerDocument::isOptionalMethodEnabled (const String& methodSigniture) const throw()
{
    return activeExtraMethods.contains (methodSigniture);
}


void JucerDocument::addExtraClassProperties (PropertyPanel* panel)
{
}

//==============================================================================
const char* const JucerDocument::jucerCompXmlTag = "JUCER_COMPONENT";

XmlElement* JucerDocument::createXml() const
{
    XmlElement* doc = new XmlElement (jucerCompXmlTag);

    doc->setAttribute ("documentType", getTypeName());
    doc->setAttribute ("className", className);
    doc->setAttribute ("componentName", componentName);
    doc->setAttribute ("parentClasses", parentClasses);
    doc->setAttribute ("constructorParams", constructorParams);
    doc->setAttribute ("variableInitialisers", variableInitialisers);
    doc->setAttribute ("snapPixels", snapGridPixels);
    doc->setAttribute ("snapActive", snapActive);
    doc->setAttribute ("snapShown", snapShown);
    doc->setAttribute ("overlayOpacity", (double) componentOverlayOpacity);
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
         && getTypeName().equalsIgnoreCase (xml.getStringAttribute ("documentType", ObjectTypes::documentTypeNames[0])))
    {
        className = xml.getStringAttribute ("className", defaultClassName);
        componentName = xml.getStringAttribute ("componentName", String::empty);
        parentClasses = xml.getStringAttribute ("parentClasses", defaultParentClasses);
        constructorParams = xml.getStringAttribute ("constructorParams", String::empty);
        variableInitialisers = xml.getStringAttribute ("variableInitialisers", String::empty);

        fixedSize = xml.getBoolAttribute ("fixedSize", false);
        initialWidth = xml.getIntAttribute ("initialWidth", 300);
        initialHeight = xml.getIntAttribute ("initialHeight", 200);

        snapGridPixels = xml.getIntAttribute ("snapPixels", snapGridPixels);
        snapActive = xml.getBoolAttribute ("snapActive", snapActive);
        snapShown = xml.getBoolAttribute ("snapShown", snapShown);

        componentOverlayOpacity = (float) xml.getDoubleAttribute ("overlayOpacity", 0.0);

        activeExtraMethods.clear();

        XmlElement* const methods = xml.getChildByName ("METHODS");

        if (methods != 0)
        {
            forEachXmlChildElementWithTagName (*methods, e, "METHOD")
            {
                activeExtraMethods.addIfNotAlreadyThere (e->getStringAttribute ("name"));
            }
        }

        activeExtraMethods.trim();
        activeExtraMethods.removeEmptyStrings();

        changed();
        getUndoManager().clearUndoHistory();
        return true;
    }

    return false;
}

//==============================================================================
Result JucerDocument::loadDocument (const File& file)
{
    const File cppFile (file.withFileExtension (".cpp"));

    const String cppFileString (cppFile.loadFileAsString());

    resources.loadFromCpp (file, cppFileString);

    ScopedPointer<XmlElement> xml (pullMetaDataFromCppFile (cppFileString));

    if (xml != nullptr)
    {
        if (loadFromXml (*xml))
            return Result::ok();

        return Result::fail (TRANS("Couldn't parse the XML section of this file correctly"));
    }

    return Result::fail (TRANS("Not a valid Jucer cpp file"));
}

Result JucerDocument::saveDocument (const File& file)
{
    const File cppFile (file.withFileExtension (".cpp"));
    const File hFile (file.withFileExtension (".h"));

    String templateH, templateCpp;

    if (! findTemplateFiles (templateH, templateCpp))
        return Result::fail (TRANS("Couldn't find the required Jucer template files...\n\nMake sure the template files directory is set up correctly in the preferences box."));

    const bool ok = writeCodeFiles (hFile, cppFile, templateH, templateCpp);
    TestComponent::reloadAll();

    if (ok)
        return Result::ok();
    else
        return Result::fail (TRANS("Couldn't write to the file."));
}

//==============================================================================
File JucerDocument::getLastDocumentOpened()
{
    return StoredSettings::getInstance()->recentFiles.getFile (0);
}

void JucerDocument::setLastDocumentOpened (const File& file)
{
    StoredSettings::getInstance()->recentFiles.addFile (file);
}

//==============================================================================
XmlElement* JucerDocument::pullMetaDataFromCppFile (const String& cpp)
{
    StringArray lines;
    lines.addLines (cpp);

    int startLine = indexOfLineStartingWith (lines, "BEGIN_JUCER_METADATA", 0);

    if (startLine > 0)
    {
        int endLine = indexOfLineStartingWith (lines, "END_JUCER_METADATA", 0);

        if (endLine > startLine)
        {
            String xmlText;

            for (int i = startLine + 1; i < endLine; ++i)
                xmlText << lines[i] << "\n";

            XmlDocument doc (xmlText);
            return doc.getDocumentElement();
        }
    }

    return 0;
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
        code.parentClassInitialiser = "Component (" + quotedString (code.componentName) + ")";

    // call these now, just to make sure they're the first two methods in the list.
    code.getCallbackCode (String::empty, "void", "paint (Graphics& g)", false)
        << "//[UserPrePaint] Add your own custom painting code here..\n//[/UserPrePaint]\n\n";

    code.getCallbackCode (String::empty, "void", "resized()", false);

    if (getComponentLayout() != 0)
        getComponentLayout()->fillInGeneratedCode (code);

    fillInPaintCode (code);

    ScopedPointer<XmlElement> e (createXml());
    jassert (e != 0);
    code.jucerMetadata = e->createDocument (String::empty, false, false);

    resources.fillInGeneratedCode (code);

    code.constructorCode
        << "\n//[UserPreSize]\n//[/UserPreSize]\n";

    if (initialWidth > 0 || initialHeight > 0)
        code.constructorCode
            << "\nsetSize (" << initialWidth << ", " << initialHeight << ");\n";

    code.getCallbackCode (String::empty, "void", "paint (Graphics& g)", false)
        << "//[UserPaint] Add your own custom painting code here..\n//[/UserPaint]";

    code.getCallbackCode (String::empty, "void", "resized()", false)
        << "//[UserResized] Add your own custom resize handling here..\n//[/UserResized]";

    // add optional methods
    StringArray baseClasses, returnValues, methods, initialContents;
    getOptionalMethods (baseClasses, returnValues, methods, initialContents);

    for (int i = 0; i < methods.size(); ++i)
    {
        if (isOptionalMethodEnabled (methods[i]))
        {
            String& s = code.getCallbackCode (baseClasses[i], returnValues[i], methods[i], false);

            if (! s.contains ("//["))
            {
                String userCommentTag ("UserCode_");
                userCommentTag += methods[i].upToFirstOccurrenceOf ("(", false, false).trim();

                s << "\n//["
                  << userCommentTag
                  << "] -- Add your code here...\n"
                  << initialContents[i];

                if (initialContents[i].isNotEmpty() && ! initialContents[i].endsWithChar ('\n'))
                    s << '\n';

                s << "//[/"
                  << userCommentTag
                  << "]\n";
            }
        }
    }
}

void JucerDocument::fillInPaintCode (GeneratedCode& code) const
{
    for (int i = 0; i < getNumPaintRoutines(); ++i)
    {
        getPaintRoutine (i)
            ->fillInGeneratedCode (code, code.getCallbackCode (String::empty, "void", "paint (Graphics& g)", false));
    }
}

//==============================================================================
bool JucerDocument::findTemplateFiles (String& templateH, String& templateCpp) const
{
    const File templateDir (StoredSettings::getInstance()->getTemplatesDir());

    const File hTemplate   (templateDir.getChildFile ("jucer_ComponentTemplate.h"));
    const File cppTemplate (templateDir.getChildFile ("jucer_ComponentTemplate.cpp"));

    if (! (cppTemplate.existsAsFile() && hTemplate.existsAsFile()))
        return false;

    templateH = hTemplate.loadFileAsString();
    templateCpp = cppTemplate.loadFileAsString();

    const String jucerVersionString ("Jucer version: " + String (JUCER_MAJOR_VERSION)
                                        + "." + String (JUCER_MINOR_VERSION));

    // This checks the template files to see if they're the ones that shipped with this
    // version of the jucer. If it fails, you're probably using the wrong ones.
    // If you're using customised template files, just add the appropriate version line to
    // their headers to avoid this warning.
    jassert (templateH.containsIgnoreCase (jucerVersionString));
    jassert (templateCpp.containsIgnoreCase (jucerVersionString));

    return true;
}

void JucerDocument::getPreviewFiles (String& h, String& cpp)
{
    if (! findTemplateFiles (h, cpp))
    {
        h = cpp = TRANS("Couldn't find the required Jucer template files...\n\nMake sure the template files directory is set up correctly in the preferences box.");
    }
    else
    {
        GeneratedCode generated (this);
        fillInGeneratedCode (generated);
        generated.includeFilesCPP.insert (0, getFile().withFileExtension ("h").getFileName());

        generated.applyToCode (h, getClassName(), true);
        generated.applyToCode (cpp, getClassName(), true);
    }
}

static const String fixNewLines (const String& s)
{
    StringArray lines;
    lines.addLines (s);

    for (int i = 0; i < lines.size(); ++i)
        lines.set (i, lines[i].trimEnd());

    while (lines.size() > 0 && lines [lines.size() - 1].trim().isEmpty())
        lines.remove (lines.size() - 1);

    return lines.joinIntoString ("\r\n") + "\r\n";
}

bool JucerDocument::writeCodeFiles (const File& headerFile,
                                    const File& cppFile,
                                    String h,
                                    String cpp) const
{
    GeneratedCode generated (this);
    fillInGeneratedCode (generated);

    generated.includeFilesCPP.insert (0, headerFile.getFileName());

    String existingHeader (h), existingCpp (cpp);

    if (headerFile.existsAsFile())
        existingHeader = headerFile.loadFileAsString();

    if (cppFile.existsAsFile())
        existingCpp = cppFile.loadFileAsString();

    generated.applyToCode (h, headerFile.getFileNameWithoutExtension(), false, existingHeader);
    generated.applyToCode (cpp, headerFile.getFileNameWithoutExtension(), false, existingCpp);

    h = fixNewLines (h);
    cpp = fixNewLines (cpp);

    return headerFile.replaceWithText (h, false, false)
             && cppFile.replaceWithText (cpp, false, false);
}
