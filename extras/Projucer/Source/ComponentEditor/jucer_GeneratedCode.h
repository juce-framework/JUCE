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

#pragma once

#include "../Project/jucer_Project.h"
class JucerDocument;

//==============================================================================
/**
    A class for collecting the various snippets of c++ that will be assembled into
    the final cpp and header files.
*/
class GeneratedCode
{
public:
    GeneratedCode (const JucerDocument*);
    ~GeneratedCode();

    //==============================================================================
    void applyToCode (String& code,
                      const File& targetFile,
                      const String& oldFileWithUserData,
                      Project* project) const;

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
    Array<File> includeFilesH, includeFilesCPP;
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

    OwnedArray<CallbackMethod> callbacks;

    String& getCallbackCode (const String& requiredParentClass,
                             const String& returnType,
                             const String& prototype,
                             const bool hasPrePostUserSections);

    void removeCallback (const String& returnType, const String& prototype);

    void addImageResourceLoader (const String& imageMemberName, const String& resourceName);

    String getCallbackDeclarations() const;
    String getCallbackDefinitions() const;
    StringArray getExtraParentClasses() const;

    bool shouldUseTransMacro() const noexcept;

private:
    String getClassDeclaration() const;
    String getInitialiserList() const;
    int suffix;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GeneratedCode)
};
