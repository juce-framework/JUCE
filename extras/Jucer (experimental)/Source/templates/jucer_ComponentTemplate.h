/*
  ==============================================================================

  This is an automatically generated file created by the Jucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created for JUCE version: %%juceVersion%%

  ------------------------------------------------------------------------------

  JUCE and the Jucer are copyright 2004-10 by Raw Material Software ltd.

  ==============================================================================
*/

#ifndef %%headerGuard%%
#define %%headerGuard%%

//[Headers]     -- You can add your own extra header files here --
%%defaultJuceInclude%%
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
    %%className%%& operator= (const %%className%%&);
};


#endif   // %%headerGuard%%
