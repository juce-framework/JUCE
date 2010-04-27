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

#ifndef __JUCER_CODEGENERATOR_H_F79AEF58__
#define __JUCER_CODEGENERATOR_H_F79AEF58__

#include "../jucer_Headers.h"
#include "jucer_Project.h"


//==============================================================================
class CodeGenerator
{
public:
    //==============================================================================
    CodeGenerator();
    ~CodeGenerator();

    //==============================================================================
    String className;
    String parentClassInitialiser;  // optional parent class initialiser to go before the items in the initialisers list
    StringArray memberInitialisers;
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
        String requiredParentClass, returnType, prototype, content;
        bool hasPrePostUserSections;
    };

    OwnedArray<CallbackMethod> callbacks;

    String& getCallbackCode (const String& requiredParentClass,
                             const String& returnType,
                             const String& prototype,
                             const bool hasPrePostUserSections);

    void removeCallback (const String& returnType, const String& prototype);

    const String getCallbackDeclarations() const;
    const String getCallbackDefinitions() const;
    const StringArray getExtraParentClasses() const;

    int getUniqueSuffix();

    //==============================================================================
    // An object to load and store all the user-defined bits of code as documents.
    class CustomisedCodeSnippets
    {
    public:
        CustomisedCodeSnippets();
        ~CustomisedCodeSnippets();

        void reloadFrom (const String& fileContent);
        void applyTo (String& fileContent) const;
        bool areAnySnippetsUnsaved() const;

        CodeDocument* getDocumentFor (const String& sectionName, bool createIfNotFound);
        const String getSectionContent (const String& sectionName) const;
        void removeSection (const String& sectionName);

    private:
        StringArray sectionNames;
        OwnedArray <CodeDocument> sectionContent;
    };

    //==============================================================================
    void applyToCode (String& codeTemplate, const File& targetFile,
                      bool isForPreview, Project* project) const;


private:
    //==============================================================================
    const String getClassDeclaration() const;
    const String getInitialiserList() const;
    int suffix;

    CodeGenerator (const CodeGenerator&);
    CodeGenerator& operator= (const CodeGenerator&);
};


#endif  // __JUCER_CODEGENERATOR_H_F79AEF58__
