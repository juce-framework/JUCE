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

//[CppHeaders] You can add your own extra header files here...
//[/CppHeaders]

%%includeFilesCPP%%

//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

extern const unsigned char %%className%%_ComponentStateData[];

//==============================================================================
%%className%%::%%className%% (%%constructorParams%%)
%%initialisers%%{
    componentState = ValueTree::readFromData (%%className%%_ComponentStateData, %%statedatasize%%);
    jassert (componentState.isValid());
    const ValueTree componentStateList (componentState.getChildByName ("COMPONENTS"));

    %%constructor%%

    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

%%className%%::~%%className%%()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    %%destructor%%

    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
%%methodDefinitions%%

//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]

%%staticMemberDefinitions%%

//==============================================================================
/* This data contains the ValueTree that holds all the Jucer-generated settings for the components */
const unsigned char %%className%%_ComponentStateData[] = %%statedata%%;

//==============================================================================
//=======================  Jucer Information Section  ==========================
//==============================================================================
#if 0
/*  This section stores the Jucer's metadata - edit it at your own risk!

%%metadata%%
*/
#endif
