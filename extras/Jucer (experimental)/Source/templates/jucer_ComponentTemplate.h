/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Creation date:  %%creationTime%%

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Jucer version: 1.12

  ------------------------------------------------------------------------------

  The Jucer is part of the JUCE library - "Jules' Utility Class Extensions"
  Copyright 2004-6 by Raw Material Software ltd.

  ==============================================================================
*/

#ifndef %%headerGuard%%
#define %%headerGuard%%

//[Headers]     -- You can add your own extra header files here --
#include "juce.h"
//[/Headers]

%%includeFilesH%%

//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Jucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
%%classDeclaration%%
{
public:
    //==============================================================================
    %%className%% (%%constructorParams%%);
    ~%%className%%();

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    %%publicMemberDeclarations%%

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    %%privateMemberDeclarations%%

    //==============================================================================
    // (prevent copy constructor and operator= being generated..)
    %%className%% (const %%className%%&);
    const %%className%%& operator= (const %%className%%&);
};


#endif   // %%headerGuard%%
