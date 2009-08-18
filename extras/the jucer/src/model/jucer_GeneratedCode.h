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

#ifndef __JUCER_GENERATEDCODE_JUCEHEADER__
#define __JUCER_GENERATEDCODE_JUCEHEADER__

class JucerDocument;


//==============================================================================
/**
    A class for collecting the various snippets of c++ that will be assembled into
    the final cpp and header files.
*/
class GeneratedCode
{
public:
    //==============================================================================
    GeneratedCode (const JucerDocument* const document);
    ~GeneratedCode();

    //==============================================================================
    void applyToCode (String& code,
                      const String& fileNameRoot,
                      const bool isForPreview,
                      const String& oldFileWithUserData = String::empty) const;

    int getUniqueSuffix();

    //==============================================================================
    const JucerDocument* const document;

    String className;
    String componentName;
    String parentClassInitialiser;  // optional parent class initialiser to go before the items in the initialisers list
    StringArray initialisers; // (a list of the member variables that need initalising after the constructor declaration)
    String parentClasses;
    String constructorParams;
    String privateMemberDeclarations;
    String publicMemberDeclarations;
    StringArray includeFilesH, includeFilesCPP;
    String constructorCode;
    String destructorCode;
    String staticMemberDefinitions;
    String jucerMetadata;

    struct CallbackMethod
    {
        String requiredParentClass;
        String returnType;
        String prototype;
        String content;
        bool hasPrePostUserSections;
    };

    OwnedArray <CallbackMethod> callbacks;

    String& getCallbackCode (const String& requiredParentClass,
                             const String& returnType,
                             const String& prototype,
                             const bool hasPrePostUserSections);

    void removeCallback (const String& returnType, const String& prototype);

    void addImageResourceLoader (const String& imageMemberName, const String& resourceName);

    const String getCallbackDeclarations() const;
    const String getCallbackDefinitions() const;
    const StringArray getExtraParentClasses() const;

private:
    const String getClassDeclaration() const;
    const String getInitialiserList() const;
    int suffix;

    GeneratedCode (const GeneratedCode&);
    const GeneratedCode& operator= (const GeneratedCode&);
};


#endif   // __JUCER_GENERATEDCODE_JUCEHEADER__
