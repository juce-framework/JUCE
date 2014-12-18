/* ==================================== JUCER_BINARY_RESOURCE ====================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

namespace BinaryData
{

//================== jucer_AnimatedComponentTemplate.cpp ==================
static const unsigned char temp_binary_data_0[] =
"/*\r\n"
"  ==============================================================================\r\n"
"\r\n"
"    This file was auto-generated!\r\n"
"\r\n"
"  ==============================================================================\r\n"
"*/\r\n"
"\r\n"
"#ifndef MAINCOMPONENT_H_INCLUDED\r\n"
"#define MAINCOMPONENT_H_INCLUDED\r\n"
"\r\n"
"INCLUDE_JUCE\r\n"
"\r\n"
"//==============================================================================\r\n"
"/*\r\n"
"    This component lives inside our window, and this is where you should put all\r\n"
"    your controls and content.\r\n"
"*/\r\n"
"class MainContentComponent   : public AnimatedAppComponent\r\n"
"{\r\n"
"public:\r\n"
"    //==============================================================================\r\n"
"    MainContentComponent()\r\n"
"    {\r\n"
"        setSize (800, 600);\r\n"
"        setFramesPerSecond (60);\r\n"
"    }\r\n"
"\r\n"
"    ~MainContentComponent()\r\n"
"    {\r\n"
"    }\r\n"
"\r\n"
"    void update() override\r\n"
"    {\r\n"
"        // This function is called at the frequency specified by the setFramesPerSecond() call\r\n"
"        // in the constructor. You can use it to update counters, animate values, etc.\r\n"
"    }\r\n"
"\r\n"
"    void paint (Graphics& g) override\r\n"
"    {\r\n"
"        // (Our component is opaque, so we must completely fill the background with a solid colour)\r\n"
"        g.fillAll (Colours::black);\r\n"
"\r\n"
"\r\n"
"        // You can add your drawing code here!\r\n"
"    }\r\n"
"\r\n"
"    void resized() override\r\n"
"    {\r\n"
"        // This is called when the MainContentComponent is resized.\r\n"
"        // If you add any child components, this is where you should\r\n"
"        // update their positions.\r\n"
"    }\r\n"
"\r\n"
"\r\n"
"private:\r\n"
"    //==============================================================================\r\n"
"\r\n"
"    // Your private member variables go here...\r\n"
"\r\n"
"\r\n"
"\r\n"
"    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)\r\n"
"};\r\n"
"\r\n"
"\r\n"
"// (This function is called by the app startup code to create our main component)\r\n"
"Component* createMainContentComponent()    { return new MainContentComponent(); }\r\n"
"\r\n"
"\r\n"
"#endif  // MAINCOMPONENT_H_INCLUDED\r\n";

const char* jucer_AnimatedComponentTemplate_cpp = (const char*) temp_binary_data_0;

//================== jucer_AudioComponentTemplate.cpp ==================
static const unsigned char temp_binary_data_1[] =
"/*\r\n"
"  ==============================================================================\r\n"
"\r\n"
"    This file was auto-generated!\r\n"
"\r\n"
"  ==============================================================================\r\n"
"*/\r\n"
"\r\n"
"#ifndef MAINCOMPONENT_H_INCLUDED\r\n"
"#define MAINCOMPONENT_H_INCLUDED\r\n"
"\r\n"
"INCLUDE_JUCE\r\n"
"\r\n"
"//==============================================================================\r\n"
"/*\r\n"
"    This component lives inside our window, and this is where you should put all\r\n"
"    your controls and content.\r\n"
"*/\r\n"
"class MainContentComponent   : public AudioAppComponent\r\n"
"{\r\n"
"public:\r\n"
"    //==============================================================================\r\n"
"    MainContentComponent()\r\n"
"    {\r\n"
"        setSize (800, 600);\r\n"
"\r\n"
"        // specify the number of input and output channels that we want to open\r\n"
"        setAudioChannels (2, 2);\r\n"
"    }\r\n"
"\r\n"
"    ~MainContentComponent()\r\n"
"    {\r\n"
"        shutdownAudio();\r\n"
"    }\r\n"
"\r\n"
"    //=======================================================================\r\n"
"    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override\r\n"
"    {\r\n"
"        // This function will be called when the audio device is started, or when\r\n"
"        // its settings (i.e. sample rate, block size, etc) are changed.\r\n"
"\r\n"
"        // You can use this function to initialise any resources you might need,\r\n"
"        // but be careful - it will be called on the audio thread, not the GUI thread.\r\n"
"\r\n"
"        // For more details, see the help for AudioProcessor::prepareToPlay()\r\n"
"    }\r\n"
"\r\n"
"    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override\r\n"
"    {\r\n"
"        // Your audio-processing code goes here!\r\n"
"\r\n"
"        // For more details, see the help for AudioProcessor::getNextAudioBlock()\r\n"
"    }\r\n"
"\r\n"
"    void releaseResources() override\r\n"
"    {\r\n"
"        // This will be called when the audio device stops, or when it is being\r\n"
"        // restarted due to a setting change.\r\n"
"\r\n"
"        // For more details, see the help for AudioProcessor::releaseResources()\r\n"
"    }\r\n"
"\r\n"
"    //=======================================================================\r\n"
"    void paint (Graphics& g) override\r\n"
"    {\r\n"
"        // (Our component is opaque, so we must completely fill the background with a solid colour)\r\n"
"        g.fillAll (Colours::black);\r\n"
"\r\n"
"\r\n"
"        // You can add your drawing code here!\r\n"
"    }\r\n"
"\r\n"
"    void resized() override\r\n"
"    {\r\n"
"        // This is called when the MainContentComponent is resized.\r\n"
"        // If you add any child components, this is where you should\r\n"
"        // update their positions.\r\n"
"    }\r\n"
"\r\n"
"\r\n"
"private:\r\n"
"    //==============================================================================\r\n"
"\r\n"
"    // Your private member variables go here...\r\n"
"\r\n"
"\r\n"
"    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)\r\n"
"};\r\n"
"\r\n"
"\r\n"
"// (This function is called by the app startup code to create our main component)\r\n"
"Component* createMainContentComponent()     { return new MainContentComponent(); }\r\n"
"\r\n"
"\r\n"
"#endif  // MAINCOMPONENT_H_INCLUDED\r\n";

const char* jucer_AudioComponentTemplate_cpp = (const char*) temp_binary_data_1;

//================== jucer_AudioPluginEditorTemplate.cpp ==================
static const unsigned char temp_binary_data_2[] =
"/*\r\n"
"  ==============================================================================\r\n"
"\r\n"
"    This file was auto-generated by the Introjucer!\r\n"
"\r\n"
"    It contains the basic framework code for a JUCE plugin editor.\r\n"
"\r\n"
"  ==============================================================================\r\n"
"*/\r\n"
"\r\n"
"EDITORCPPHEADERS\r\n"
"\r\n"
"\r\n"
"//==============================================================================\r\n"
"EDITORCLASSNAME::EDITORCLASSNAME (FILTERCLASSNAME& p)\r\n"
"    : AudioProcessorEditor (&p), processor (p)\r\n"
"{\r\n"
"    // Make sure that before the constructor has finished, you've set the\r\n"
"    // editor's size to whatever you need it to be.\r\n"
"    setSize (400, 300);\r\n"
"}\r\n"
"\r\n"
"EDITORCLASSNAME::~EDITORCLASSNAME()\r\n"
"{\r\n"
"}\r\n"
"\r\n"
"//==============================================================================\r\n"
"void EDITORCLASSNAME::paint (Graphics& g)\r\n"
"{\r\n"
"    g.fillAll (Colours::white);\r\n"
"\r\n"
"    g.setColour (Colours::black);\r\n"
"    g.setFont (15.0f);\r\n"
"    g.drawFittedText (\"Hello World!\", getLocalBounds(), Justification::centred, 1);\r\n"
"}\r\n"
"\r\n"
"void EDITORCLASSNAME::resized()\r\n"
"{\r\n"
"    // This is generally where you'll want to lay out the positions of any\r\n"
"    // subcomponents in your editor..\r\n"
"}\r\n";

const char* jucer_AudioPluginEditorTemplate_cpp = (const char*) temp_binary_data_2;

//================== jucer_AudioPluginEditorTemplate.h ==================
static const unsigned char temp_binary_data_3[] =
"/*\r\n"
"  ==============================================================================\r\n"
"\r\n"
"    This file was auto-generated by the Introjucer!\r\n"
"\r\n"
"    It contains the basic framework code for a JUCE plugin editor.\r\n"
"\r\n"
"  ==============================================================================\r\n"
"*/\r\n"
"\r\n"
"#ifndef HEADERGUARD\r\n"
"#define HEADERGUARD\r\n"
"\r\n"
"EDITORHEADERS\r\n"
"\r\n"
"\r\n"
"//==============================================================================\r\n"
"/**\r\n"
"*/\r\n"
"class EDITORCLASSNAME  : public AudioProcessorEditor\r\n"
"{\r\n"
"public:\r\n"
"    EDITORCLASSNAME (FILTERCLASSNAME&);\r\n"
"    ~EDITORCLASSNAME();\r\n"
"\r\n"
"    //==============================================================================\r\n"
"    void paint (Graphics&) override;\r\n"
"    void resized() override;\r\n"
"\r\n"
"private:\r\n"
"    // This reference is provided as a quick way for your editor to\r\n"
"    // access the processor object that created it.\r\n"
"    FILTERCLASSNAME& processor;\r\n"
"\r\n"
"    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EDITORCLASSNAME)\r\n"
"};\r\n"
"\r\n"
"\r\n"
"#endif  // HEADERGUARD\r\n";

const char* jucer_AudioPluginEditorTemplate_h = (const char*) temp_binary_data_3;

//================== jucer_AudioPluginFilterTemplate.cpp ==================
static const unsigned char temp_binary_data_4[] =
"/*\r\n"
"  ==============================================================================\r\n"
"\r\n"
"    This file was auto-generated by the Introjucer!\r\n"
"\r\n"
"    It contains the basic framework code for a JUCE plugin processor.\r\n"
"\r\n"
"  ==============================================================================\r\n"
"*/\r\n"
"\r\n"
"FILTERHEADERS\r\n"
"\r\n"
"\r\n"
"//==============================================================================\r\n"
"FILTERCLASSNAME::FILTERCLASSNAME()\r\n"
"{\r\n"
"}\r\n"
"\r\n"
"FILTERCLASSNAME::~FILTERCLASSNAME()\r\n"
"{\r\n"
"}\r\n"
"\r\n"
"//==============================================================================\r\n"
"const String FILTERCLASSNAME::getName() const\r\n"
"{\r\n"
"    return JucePlugin_Name;\r\n"
"}\r\n"
"\r\n"
"int FILTERCLASSNAME::getNumParameters()\r\n"
"{\r\n"
"    return 0;\r\n"
"}\r\n"
"\r\n"
"float FILTERCLASSNAME::getParameter (int index)\r\n"
"{\r\n"
"    return 0.0f;\r\n"
"}\r\n"
"\r\n"
"void FILTERCLASSNAME::setParameter (int index, float newValue)\r\n"
"{\r\n"
"}\r\n"
"\r\n"
"const String FILTERCLASSNAME::getParameterName (int index)\r\n"
"{\r\n"
"    return String();\r\n"
"}\r\n"
"\r\n"
"const String FILTERCLASSNAME::getParameterText (int index)\r\n"
"{\r\n"
"    return String();\r\n"
"}\r\n"
"\r\n"
"const String FILTERCLASSNAME::getInputChannelName (int channelIndex) const\r\n"
"{\r\n"
"    return String (channelIndex + 1);\r\n"
"}\r\n"
"\r\n"
"const String FILTERCLASSNAME::getOutputChannelName (int channelIndex) const\r\n"
"{\r\n"
"    return String (channelIndex + 1);\r\n"
"}\r\n"
"\r\n"
"bool FILTERCLASSNAME::isInputChannelStereoPair (int index) const\r\n"
"{\r\n"
"    return true;\r\n"
"}\r\n"
"\r\n"
"bool FILTERCLASSNAME::isOutputChannelStereoPair (int index) const\r\n"
"{\r\n"
"    return true;\r\n"
"}\r\n"
"\r\n"
"bool FILTERCLASSNAME::acceptsMidi() const\r\n"
"{\r\n"
"   #if JucePlugin_WantsMidiInput\r\n"
"    return true;\r\n"
"   #else\r\n"
"    return false;\r\n"
"   #endif\r\n"
"}\r\n"
"\r\n"
"bool FILTERCLASSNAME::producesMidi() const\r\n"
"{\r\n"
"   #if JucePlugin_ProducesMidiOutput\r\n"
"    return true;\r\n"
"   #else\r\n"
"    return false;\r\n"
"   #endif\r\n"
"}\r\n"
"\r\n"
"bool FILTERCLASSNAME::silenceInProducesSilenceOut() const\r\n"
"{\r\n"
"    return false;\r\n"
"}\r\n"
"\r\n"
"double FILTERCLASSNAME::getTailLengthSeconds() const\r\n"
"{\r\n"
"    return 0.0;\r\n"
"}\r\n"
"\r\n"
"int FILTERCLASSNAME::getNumPrograms()\r\n"
"{\r\n"
"    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,\r\n"
"                // so this should be at least 1, even if you're not really implementing programs.\r\n"
"}\r\n"
"\r\n"
"int FILTERCLASSNAME::getCurrentProgram()\r\n"
"{\r\n"
"    return 0;\r\n"
"}\r\n"
"\r\n"
"void FILTERCLASSNAME::setCurrentProgram (int index)\r\n"
"{\r\n"
"}\r\n"
"\r\n"
"const String FILTERCLASSNAME::getProgramName (int index)\r\n"
"{\r\n"
"    return String();\r\n"
"}\r\n"
"\r\n"
"void FILTERCLASSNAME::changeProgramName (int index, const String& newName)\r\n"
"{\r\n"
"}\r\n"
"\r\n"
"//==============================================================================\r\n"
"void FILTERCLASSNAME::prepareToPlay (double sampleRate, int samplesPerBlock)\r\n"
"{\r\n"
"    // Use this method as the place to do any pre-playback\r\n"
"    // initialisation that you need..\r\n"
"}\r\n"
"\r\n"
"void FILTERCLASSNAME::releaseResources()\r\n"
"{\r\n"
"    // When playback stops, you can use this as an opportunity to free up any\r\n"
"    // spare memory, etc.\r\n"
"}\r\n"
"\r\n"
"void FILTERCLASSNAME::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)\r\n"
"{\r\n"
"    // In case we have more outputs than inputs, this code clears any output\r\n"
"    // channels that didn't contain input data, (because these aren't\r\n"
"    // guaranteed to be empty - they may contain garbage).\r\n"
"    // I've added this to avoid people getting screaming feedback\r\n"
"    // when they first compile the plugin, but obviously you don't need to\r\n"
"    // this code if your algorithm already fills all the output channels.\r\n"
"    for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)\r\n"
"        buffer.clear (i, 0, buffer.getNumSamples());\r\n"
"\r\n"
"    // This is the place where you'd normally do the guts of your plugin's\r\n"
"    // audio processing...\r\n"
"    for (int channel = 0; channel < getNumInputChannels(); ++channel)\r\n"
"    {\r\n"
"        float* channelData = buffer.getWritePointer (channel);\r\n"
"\r\n"
"        // ..do something to the data...\r\n"
"    }\r\n"
"}\r\n"
"\r\n"
"//==============================================================================\r\n"
"bool FILTERCLASSNAME::hasEditor() const\r\n"
"{\r\n"
"    return true; // (change this to false if you choose to not supply an editor)\r\n"
"}\r\n"
"\r\n"
"AudioProcessorEditor* FILTERCLASSNAME::createEditor()\r\n"
"{\r\n"
"    return new EDITORCLASSNAME (*this);\r\n"
"}\r\n"
"\r\n"
"//==============================================================================\r\n"
"void FILTERCLASSNAME::getStateInformation (MemoryBlock& destData)\r\n"
"{\r\n"
"    // You should use this method to store your parameters in the memory block.\r\n"
"    // You could do that either as raw data, or use the XML or ValueTree classes\r\n"
"    // as intermediaries to make it easy to save and load complex data.\r\n"
"}\r\n"
"\r\n"
"void FILTERCLASSNAME::setStateInformation (const void* data, int sizeInBytes)\r\n"
"{\r\n"
"    // You should use this method to restore your parameters from this memory block,\r\n"
"    // whose contents will have been created by the getStateInformation() call.\r\n"
"}\r\n"
"\r\n"
"//==============================================================================\r\n"
"// This creates new instances of the plugin..\r\n"
"AudioProcessor* JUCE_CALLTYPE createPluginFilter()\r\n"
"{\r\n"
"    return new FILTERCLASSNAME();\r\n"
"}\r\n";

const char* jucer_AudioPluginFilterTemplate_cpp = (const char*) temp_binary_data_4;

//================== jucer_AudioPluginFilterTemplate.h ==================
static const unsigned char temp_binary_data_5[] =
"/*\r\n"
"  ==============================================================================\r\n"
"\r\n"
"    This file was auto-generated by the Introjucer!\r\n"
"\r\n"
"    It contains the basic framework code for a JUCE plugin processor.\r\n"
"\r\n"
"  ==============================================================================\r\n"
"*/\r\n"
"\r\n"
"#ifndef HEADERGUARD\r\n"
"#define HEADERGUARD\r\n"
"\r\n"
"APPHEADERS\r\n"
"\r\n"
"\r\n"
"//==============================================================================\r\n"
"/**\r\n"
"*/\r\n"
"class FILTERCLASSNAME  : public AudioProcessor\r\n"
"{\r\n"
"public:\r\n"
"    //==============================================================================\r\n"
"    FILTERCLASSNAME();\r\n"
"    ~FILTERCLASSNAME();\r\n"
"\r\n"
"    //==============================================================================\r\n"
"    void prepareToPlay (double sampleRate, int samplesPerBlock) override;\r\n"
"    void releaseResources() override;\r\n"
"\r\n"
"    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;\r\n"
"\r\n"
"    //==============================================================================\r\n"
"    AudioProcessorEditor* createEditor() override;\r\n"
"    bool hasEditor() const override;\r\n"
"\r\n"
"    //==============================================================================\r\n"
"    const String getName() const override;\r\n"
"\r\n"
"    int getNumParameters() override;\r\n"
"    float getParameter (int index) override;\r\n"
"    void setParameter (int index, float newValue) override;\r\n"
"\r\n"
"    const String getParameterName (int index) override;\r\n"
"    const String getParameterText (int index) override;\r\n"
"\r\n"
"    const String getInputChannelName (int channelIndex) const override;\r\n"
"    const String getOutputChannelName (int channelIndex) const override;\r\n"
"    bool isInputChannelStereoPair (int index) const override;\r\n"
"    bool isOutputChannelStereoPair (int index) const override;\r\n"
"\r\n"
"    bool acceptsMidi() const override;\r\n"
"    bool producesMidi() const override;\r\n"
"    bool silenceInProducesSilenceOut() const override;\r\n"
"    double getTailLengthSeconds() const override;\r\n"
"\r\n"
"    //==============================================================================\r\n"
"    int getNumPrograms() override;\r\n"
"    int getCurrentProgram() override;\r\n"
"    void setCurrentProgram (int index) override;\r\n"
"    const String getProgramName (int index) override;\r\n"
"    void changeProgramName (int index, const String& newName) override;\r\n"
"\r\n"
"    //==============================================================================\r\n"
"    void getStateInformation (MemoryBlock& destData) override;\r\n"
"    void setStateInformation (const void* data, int sizeInBytes) override;\r\n"
"\r\n"
"private:\r\n"
"    //==============================================================================\r\n"
"    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FILTERCLASSNAME)\r\n"
"};\r\n"
"\r\n"
"\r\n"
"#endif  // HEADERGUARD\r\n";

const char* jucer_AudioPluginFilterTemplate_h = (const char*) temp_binary_data_5;

//================== jucer_ComponentTemplate.cpp ==================
static const unsigned char temp_binary_data_6[] =
"/*\r\n"
"  ==============================================================================\r\n"
"\r\n"
"  This is an automatically generated GUI class created by the Introjucer!\r\n"
"\r\n"
"  Be careful when adding custom code to these files, as only the code within\r\n"
"  the \"//[xyz]\" and \"//[/xyz]\" sections will be retained when the file is loaded\r\n"
"  and re-saved.\r\n"
"\r\n"
"  Created with Introjucer version: %%version%%\r\n"
"\r\n"
"  ------------------------------------------------------------------------------\r\n"
"\r\n"
"  The Introjucer is part of the JUCE library - \"Jules' Utility Class Extensions\"\r\n"
"  Copyright 2004-13 by Raw Material Software Ltd.\r\n"
"\r\n"
"  ==============================================================================\r\n"
"*/\r\n"
"\r\n"
"//[Headers] You can add your own extra header files here...\r\n"
"//[/Headers]\r\n"
"\r\n"
"%%includeFilesCPP%%\r\n"
"\r\n"
"//[MiscUserDefs] You can add your own user definitions and misc code here...\r\n"
"//[/MiscUserDefs]\r\n"
"\r\n"
"//==============================================================================\r\n"
"%%className%%::%%className%% (%%constructorParams%%)\r\n"
"%%initialisers%%{\r\n"
"    %%constructor%%\r\n"
"\r\n"
"    //[Constructor] You can add your own custom stuff here..\r\n"
"    //[/Constructor]\r\n"
"}\r\n"
"\r\n"
"%%className%%::~%%className%%()\r\n"
"{\r\n"
"    //[Destructor_pre]. You can add your own custom destruction code here..\r\n"
"    //[/Destructor_pre]\r\n"
"\r\n"
"    %%destructor%%\r\n"
"\r\n"
"    //[Destructor]. You can add your own custom destruction code here..\r\n"
"    //[/Destructor]\r\n"
"}\r\n"
"\r\n"
"//==============================================================================\r\n"
"%%methodDefinitions%%\r\n"
"\r\n"
"//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...\r\n"
"//[/MiscUserCode]\r\n"
"\r\n"
"\r\n"
"//==============================================================================\r\n"
"#if 0\r\n"
"/*  -- Introjucer information section --\r\n"
"\r\n"
"    This is where the Introjucer stores the metadata that describe this GUI layout, so \r\n"
"    make changes in here at your peril!\r\n"
"\r\n"
"BEGIN_JUCER_METADATA\r\n"
"\r\n"
"%%metadata%%\r\n"
"END_JUCER_METADATA\r\n"
"*/\r\n"
"#endif\r\n"
"\r\n"
"%%staticMemberDefinitions%%\r\n"
"//[EndFile] You can add extra defines here...\r\n"
"//[/EndFile]";

const char* jucer_ComponentTemplate_cpp = (const char*) temp_binary_data_6;

//================== jucer_ComponentTemplate.h ==================
static const unsigned char temp_binary_data_7[] =
"/*\r\n"
"  ==============================================================================\r\n"
"\r\n"
"  This is an automatically generated GUI class created by the Introjucer!\r\n"
"\r\n"
"  Be careful when adding custom code to these files, as only the code within\r\n"
"  the \"//[xyz]\" and \"//[/xyz]\" sections will be retained when the file is loaded\r\n"
"  and re-saved.\r\n"
"\r\n"
"  Created with Introjucer version: %%version%%\r\n"
"\r\n"
"  ------------------------------------------------------------------------------\r\n"
"\r\n"
"  The Introjucer is part of the JUCE library - \"Jules' Utility Class Extensions\"\r\n"
"  Copyright 2004-13 by Raw Material Software Ltd.\r\n"
"\r\n"
"  ==============================================================================\r\n"
"*/\r\n"
"\r\n"
"#ifndef %%headerGuard%%\r\n"
"#define %%headerGuard%%\r\n"
"\r\n"
"//[Headers]     -- You can add your own extra header files here --\r\n"
"#include \"JuceHeader.h\"\r\n"
"//[/Headers]\r\n"
"\r\n"
"%%includeFilesH%%\r\n"
"\r\n"
"//==============================================================================\r\n"
"/**\r\n"
"                                                                    //[Comments]\r\n"
"    An auto-generated component, created by the Introjucer.\r\n"
"\r\n"
"    Describe your class and how it works here!\r\n"
"                                                                    //[/Comments]\r\n"
"*/\r\n"
"%%classDeclaration%%\r\n"
"{\r\n"
"public:\r\n"
"    //==============================================================================\r\n"
"    %%className%% (%%constructorParams%%);\r\n"
"    ~%%className%%();\r\n"
"\r\n"
"    //==============================================================================\r\n"
"    //[UserMethods]     -- You can add your own custom methods in this section.\r\n"
"    //[/UserMethods]\r\n"
"\r\n"
"    %%publicMemberDeclarations%%\r\n"
"\r\n"
"private:\r\n"
"    //[UserVariables]   -- You can add your own custom variables in this section.\r\n"
"    //[/UserVariables]\r\n"
"\r\n"
"    //==============================================================================\r\n"
"    %%privateMemberDeclarations%%\r\n"
"\r\n"
"    //==============================================================================\r\n"
"    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (%%className%%)\r\n"
"};\r\n"
"\r\n"
"//[EndFile] You can add extra defines here...\r\n"
"//[/EndFile]\r\n"
"\r\n"
"#endif   // %%headerGuard%%";

const char* jucer_ComponentTemplate_h = (const char*) temp_binary_data_7;

//================== jucer_ContentCompTemplate.cpp ==================
static const unsigned char temp_binary_data_8[] =
"/*\r\n"
"  ==============================================================================\r\n"
"\r\n"
"    This file was auto-generated!\r\n"
"\r\n"
"  ==============================================================================\r\n"
"*/\r\n"
"\r\n"
"INCLUDE_CORRESPONDING_HEADER\r\n"
"\r\n"
"\r\n"
"//==============================================================================\r\n"
"CONTENTCOMPCLASS::CONTENTCOMPCLASS()\r\n"
"{\r\n"
"    setSize (600, 400);\r\n"
"}\r\n"
"\r\n"
"CONTENTCOMPCLASS::~CONTENTCOMPCLASS()\r\n"
"{\r\n"
"}\r\n"
"\r\n"
"void CONTENTCOMPCLASS::paint (Graphics& g)\r\n"
"{\r\n"
"    g.fillAll (Colour (0xff001F36));\r\n"
"\r\n"
"    g.setFont (Font (16.0f));\r\n"
"    g.setColour (Colours::white);\r\n"
"    g.drawText (\"Hello World!\", getLocalBounds(), Justification::centred, true);\r\n"
"}\r\n"
"\r\n"
"void CONTENTCOMPCLASS::resized()\r\n"
"{\r\n"
"    // This is called when the CONTENTCOMPCLASS is resized.\r\n"
"    // If you add any child components, this is where you should\r\n"
"    // update their positions.\r\n"
"}\r\n";

const char* jucer_ContentCompTemplate_cpp = (const char*) temp_binary_data_8;

//================== jucer_ContentCompTemplate.h ==================
static const unsigned char temp_binary_data_9[] =
"/*\r\n"
"  ==============================================================================\r\n"
"\r\n"
"    This file was auto-generated!\r\n"
"\r\n"
"  ==============================================================================\r\n"
"*/\r\n"
"\r\n"
"#ifndef HEADERGUARD\r\n"
"#define HEADERGUARD\r\n"
"\r\n"
"INCLUDE_JUCE\r\n"
"\r\n"
"\r\n"
"//==============================================================================\r\n"
"/*\r\n"
"    This component lives inside our window, and this is where you should put all\r\n"
"    your controls and content.\r\n"
"*/\r\n"
"class CONTENTCOMPCLASS   : public Component\r\n"
"{\r\n"
"public:\r\n"
"    //==============================================================================\r\n"
"    CONTENTCOMPCLASS();\r\n"
"    ~CONTENTCOMPCLASS();\r\n"
"\r\n"
"    void paint (Graphics&);\r\n"
"    void resized();\r\n"
"\r\n"
"private:\r\n"
"    //==============================================================================\r\n"
"    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CONTENTCOMPCLASS)\r\n"
"};\r\n"
"\r\n"
"\r\n"
"#endif  // HEADERGUARD\r\n";

const char* jucer_ContentCompTemplate_h = (const char*) temp_binary_data_9;

//================== jucer_InlineComponentTemplate.h ==================
static const unsigned char temp_binary_data_10[] =
"//==============================================================================\r\n"
"class COMPONENTCLASS    : public Component\r\n"
"{\r\n"
"public:\r\n"
"    COMPONENTCLASS()\r\n"
"    {\r\n"
"        // In your constructor, you should add any child components, and\r\n"
"        // initialise any special settings that your component needs.\r\n"
"\r\n"
"    }\r\n"
"\r\n"
"    ~COMPONENTCLASS()\r\n"
"    {\r\n"
"    }\r\n"
"\r\n"
"    void paint (Graphics& g)\r\n"
"    {\r\n"
"        // You should replace everything in this method with your own drawing code..\r\n"
"\r\n"
"        g.fillAll (Colours::white);   // clear the background\r\n"
"\r\n"
"        g.setColour (Colours::grey);\r\n"
"        g.drawRect (getLocalBounds(), 1);   // draw an outline around the component\r\n"
"\r\n"
"        g.setColour (Colours::lightblue);\r\n"
"        g.setFont (14.0f);\r\n"
"        g.drawText (\"COMPONENTCLASS\", getLocalBounds(),\r\n"
"                    Justification::centred, true);   // draw some placeholder text\r\n"
"    }\r\n"
"\r\n"
"    void resized()\r\n"
"    {\r\n"
"        // This method is where you should set the bounds of any child\r\n"
"        // components that your component contains..\r\n"
"\r\n"
"    }\r\n"
"\r\n"
"private:\r\n"
"    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (COMPONENTCLASS)\r\n"
"};\r\n";

const char* jucer_InlineComponentTemplate_h = (const char*) temp_binary_data_10;

//================== jucer_MainConsoleAppTemplate.cpp ==================
static const unsigned char temp_binary_data_11[] =
"/*\r\n"
"  ==============================================================================\r\n"
"\r\n"
"    This file was auto-generated!\r\n"
"\r\n"
"    It contains the basic startup code for a Juce application.\r\n"
"\r\n"
"  ==============================================================================\r\n"
"*/\r\n"
"\r\n"
"APPHEADERS\r\n"
"\r\n"
"\r\n"
"//==============================================================================\r\n"
"int main (int argc, char* argv[])\r\n"
"{\r\n"
"\r\n"
"    // ..your code goes here!\r\n"
"\r\n"
"\r\n"
"    return 0;\r\n"
"}\r\n";

const char* jucer_MainConsoleAppTemplate_cpp = (const char*) temp_binary_data_11;

//================== jucer_MainTemplate_NoWindow.cpp ==================
static const unsigned char temp_binary_data_12[] =
"/*\r\n"
"  ==============================================================================\r\n"
"\r\n"
"    This file was auto-generated by the Introjucer!\r\n"
"\r\n"
"    It contains the basic startup code for a Juce application.\r\n"
"\r\n"
"  ==============================================================================\r\n"
"*/\r\n"
"\r\n"
"APPHEADERS\r\n"
"\r\n"
"\r\n"
"//==============================================================================\r\n"
"class APPCLASSNAME  : public JUCEApplication\r\n"
"{\r\n"
"public:\r\n"
"    //==============================================================================\r\n"
"    APPCLASSNAME() {}\r\n"
"\r\n"
"    const String getApplicationName() override       { return ProjectInfo::projectName; }\r\n"
"    const String getApplicationVersion() override    { return ProjectInfo::versionString; }\r\n"
"    bool moreThanOneInstanceAllowed() override       { return ALLOWMORETHANONEINSTANCE; }\r\n"
"\r\n"
"    //==============================================================================\r\n"
"    void initialise (const String& commandLine) override\r\n"
"    {\r\n"
"        // Add your application's initialisation code here..\r\n"
"    }\r\n"
"\r\n"
"    void shutdown() override\r\n"
"    {\r\n"
"        // Add your application's shutdown code here..\r\n"
"    }\r\n"
"\r\n"
"    //==============================================================================\r\n"
"    void systemRequestedQuit() override\r\n"
"    {\r\n"
"        // This is called when the app is being asked to quit: you can ignore this\r\n"
"        // request and let the app carry on running, or call quit() to allow the app to close.\r\n"
"        quit();\r\n"
"    }\r\n"
"\r\n"
"    void anotherInstanceStarted (const String& commandLine) override\r\n"
"    {\r\n"
"        // When another instance of the app is launched while this one is running,\r\n"
"        // this method is invoked, and the commandLine parameter tells you what\r\n"
"        // the other instance's command-line arguments were.\r\n"
"    }\r\n"
"};\r\n"
"\r\n"
"//==============================================================================\r\n"
"// This macro generates the main() routine that launches the app.\r\n"
"START_JUCE_APPLICATION (APPCLASSNAME)\r\n";

const char* jucer_MainTemplate_NoWindow_cpp = (const char*) temp_binary_data_12;

//================== jucer_MainTemplate_SimpleWindow.cpp ==================
static const unsigned char temp_binary_data_13[] =
"/*\r\n"
"  ==============================================================================\r\n"
"\r\n"
"    This file was auto-generated by the Introjucer!\r\n"
"\r\n"
"    It contains the basic startup code for a Juce application.\r\n"
"\r\n"
"  ==============================================================================\r\n"
"*/\r\n"
"\r\n"
"APPHEADERS\r\n"
"\r\n"
"Component* createMainContentComponent();\r\n"
"\r\n"
"//==============================================================================\r\n"
"class APPCLASSNAME  : public JUCEApplication\r\n"
"{\r\n"
"public:\r\n"
"    //==============================================================================\r\n"
"    APPCLASSNAME() {}\r\n"
"\r\n"
"    const String getApplicationName() override       { return ProjectInfo::projectName; }\r\n"
"    const String getApplicationVersion() override    { return ProjectInfo::versionString; }\r\n"
"    bool moreThanOneInstanceAllowed() override       { return ALLOWMORETHANONEINSTANCE; }\r\n"
"\r\n"
"    //==============================================================================\r\n"
"    void initialise (const String& commandLine) override\r\n"
"    {\r\n"
"        // This method is where you should put your application's initialisation code..\r\n"
"\r\n"
"        mainWindow = new MainWindow (getApplicationName());\r\n"
"    }\r\n"
"\r\n"
"    void shutdown() override\r\n"
"    {\r\n"
"        // Add your application's shutdown code here..\r\n"
"\r\n"
"        mainWindow = nullptr; // (deletes our window)\r\n"
"    }\r\n"
"\r\n"
"    //==============================================================================\r\n"
"    void systemRequestedQuit() override\r\n"
"    {\r\n"
"        // This is called when the app is being asked to quit: you can ignore this\r\n"
"        // request and let the app carry on running, or call quit() to allow the app to close.\r\n"
"        quit();\r\n"
"    }\r\n"
"\r\n"
"    void anotherInstanceStarted (const String& commandLine) override\r\n"
"    {\r\n"
"        // When another instance of the app is launched while this one is running,\r\n"
"        // this method is invoked, and the commandLine parameter tells you what\r\n"
"        // the other instance's command-line arguments were.\r\n"
"    }\r\n"
"\r\n"
"    //==============================================================================\r\n"
"    /*\r\n"
"        This class implements the desktop window that contains an instance of\r\n"
"        our CONTENTCOMPCLASS class.\r\n"
"    */\r\n"
"    class MainWindow    : public DocumentWindow\r\n"
"    {\r\n"
"    public:\r\n"
"        MainWindow (String name)  : DocumentWindow (name,\r\n"
"                                                    Colours::lightgrey,\r\n"
"                                                    DocumentWindow::allButtons)\r\n"
"        {\r\n"
"            setUsingNativeTitleBar (true);\r\n"
"            setContentOwned (createMainContentComponent(), true);\r\n"
"            setResizable (true, true);\r\n"
"\r\n"
"            centreWithSize (getWidth(), getHeight());\r\n"
"            setVisible (true);\r\n"
"        }\r\n"
"\r\n"
"        void closeButtonPressed() override\r\n"
"        {\r\n"
"            // This is called when the user tries to close this window. Here, we'll just\r\n"
"            // ask the app to quit when this happens, but you can change this to do\r\n"
"            // whatever you need.\r\n"
"            JUCEApplication::getInstance()->systemRequestedQuit();\r\n"
"        }\r\n"
"\r\n"
"        /* Note: Be careful if you override any DocumentWindow methods - the base\r\n"
"           class uses a lot of them, so by overriding you might break its functionality.\r\n"
"           It's best to do all your work in your content component instead, but if\r\n"
"           you really have to override any DocumentWindow methods, make sure your\r\n"
"           subclass also calls the superclass's method.\r\n"
"        */\r\n"
"\r\n"
"    private:\r\n"
"        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)\r\n"
"    };\r\n"
"\r\n"
"private:\r\n"
"    ScopedPointer<MainWindow> mainWindow;\r\n"
"};\r\n"
"\r\n"
"//==============================================================================\r\n"
"// This macro generates the main() routine that launches the app.\r\n"
"START_JUCE_APPLICATION (APPCLASSNAME)\r\n";

const char* jucer_MainTemplate_SimpleWindow_cpp = (const char*) temp_binary_data_13;

//================== jucer_MainTemplate_Window.cpp ==================
static const unsigned char temp_binary_data_14[] =
"/*\r\n"
"  ==============================================================================\r\n"
"\r\n"
"    This file was auto-generated by the Introjucer!\r\n"
"\r\n"
"    It contains the basic startup code for a Juce application.\r\n"
"\r\n"
"  ==============================================================================\r\n"
"*/\r\n"
"\r\n"
"APPHEADERS\r\n"
"\r\n"
"\r\n"
"//==============================================================================\r\n"
"class APPCLASSNAME  : public JUCEApplication\r\n"
"{\r\n"
"public:\r\n"
"    //==============================================================================\r\n"
"    APPCLASSNAME() {}\r\n"
"\r\n"
"    const String getApplicationName() override       { return ProjectInfo::projectName; }\r\n"
"    const String getApplicationVersion() override    { return ProjectInfo::versionString; }\r\n"
"    bool moreThanOneInstanceAllowed() override       { return ALLOWMORETHANONEINSTANCE; }\r\n"
"\r\n"
"    //==============================================================================\r\n"
"    void initialise (const String& commandLine) override\r\n"
"    {\r\n"
"        // This method is where you should put your application's initialisation code..\r\n"
"\r\n"
"        mainWindow = new MainWindow (getApplicationName());\r\n"
"    }\r\n"
"\r\n"
"    void shutdown() override\r\n"
"    {\r\n"
"        // Add your application's shutdown code here..\r\n"
"\r\n"
"        mainWindow = nullptr; // (deletes our window)\r\n"
"    }\r\n"
"\r\n"
"    //==============================================================================\r\n"
"    void systemRequestedQuit() override\r\n"
"    {\r\n"
"        // This is called when the app is being asked to quit: you can ignore this\r\n"
"        // request and let the app carry on running, or call quit() to allow the app to close.\r\n"
"        quit();\r\n"
"    }\r\n"
"\r\n"
"    void anotherInstanceStarted (const String& commandLine) override\r\n"
"    {\r\n"
"        // When another instance of the app is launched while this one is running,\r\n"
"        // this method is invoked, and the commandLine parameter tells you what\r\n"
"        // the other instance's command-line arguments were.\r\n"
"    }\r\n"
"\r\n"
"    //==============================================================================\r\n"
"    /*\r\n"
"        This class implements the desktop window that contains an instance of\r\n"
"        our CONTENTCOMPCLASS class.\r\n"
"    */\r\n"
"    class MainWindow    : public DocumentWindow\r\n"
"    {\r\n"
"    public:\r\n"
"        MainWindow (String name)  : DocumentWindow (name,\r\n"
"                                                    Colours::lightgrey,\r\n"
"                                                    DocumentWindow::allButtons)\r\n"
"        {\r\n"
"            setUsingNativeTitleBar (true);\r\n"
"            setContentOwned (new CONTENTCOMPCLASS(), true);\r\n"
"\r\n"
"            centreWithSize (getWidth(), getHeight());\r\n"
"            setVisible (true);\r\n"
"        }\r\n"
"\r\n"
"        void closeButtonPressed() override\r\n"
"        {\r\n"
"            // This is called when the user tries to close this window. Here, we'll just\r\n"
"            // ask the app to quit when this happens, but you can change this to do\r\n"
"            // whatever you need.\r\n"
"            JUCEApplication::getInstance()->systemRequestedQuit();\r\n"
"        }\r\n"
"\r\n"
"        /* Note: Be careful if you override any DocumentWindow methods - the base\r\n"
"           class uses a lot of them, so by overriding you might break its functionality.\r\n"
"           It's best to do all your work in your content component instead, but if\r\n"
"           you really have to override any DocumentWindow methods, make sure your\r\n"
"           subclass also calls the superclass's method.\r\n"
"        */\r\n"
"\r\n"
"    private:\r\n"
"        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)\r\n"
"    };\r\n"
"\r\n"
"private:\r\n"
"    ScopedPointer<MainWindow> mainWindow;\r\n"
"};\r\n"
"\r\n"
"//==============================================================================\r\n"
"// This macro generates the main() routine that launches the app.\r\n"
"START_JUCE_APPLICATION (APPCLASSNAME)\r\n";

const char* jucer_MainTemplate_Window_cpp = (const char*) temp_binary_data_14;

//================== jucer_NewComponentTemplate.cpp ==================
static const unsigned char temp_binary_data_15[] =
"/*\r\n"
"  ==============================================================================\r\n"
"\r\n"
"    FILENAME\r\n"
"    Created: DATE\r\n"
"    Author:  AUTHOR\r\n"
"\r\n"
"  ==============================================================================\r\n"
"*/\r\n"
"\r\n"
"INCLUDE_JUCE\r\n"
"INCLUDE_CORRESPONDING_HEADER\r\n"
"\r\n"
"//==============================================================================\r\n"
"COMPONENTCLASS::COMPONENTCLASS()\r\n"
"{\r\n"
"    // In your constructor, you should add any child components, and\r\n"
"    // initialise any special settings that your component needs.\r\n"
"\r\n"
"}\r\n"
"\r\n"
"COMPONENTCLASS::~COMPONENTCLASS()\r\n"
"{\r\n"
"}\r\n"
"\r\n"
"void COMPONENTCLASS::paint (Graphics& g)\r\n"
"{\r\n"
"    /* This demo code just fills the component's background and\r\n"
"       draws some placeholder text to get you started.\r\n"
"\r\n"
"       You should replace everything in this method with your own\r\n"
"       drawing code..\r\n"
"    */\r\n"
"\r\n"
"    g.fillAll (Colours::white);   // clear the background\r\n"
"\r\n"
"    g.setColour (Colours::grey);\r\n"
"    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component\r\n"
"\r\n"
"    g.setColour (Colours::lightblue);\r\n"
"    g.setFont (14.0f);\r\n"
"    g.drawText (\"COMPONENTCLASS\", getLocalBounds(),\r\n"
"                Justification::centred, true);   // draw some placeholder text\r\n"
"}\r\n"
"\r\n"
"void COMPONENTCLASS::resized()\r\n"
"{\r\n"
"    // This method is where you should set the bounds of any child\r\n"
"    // components that your component contains..\r\n"
"\r\n"
"}\r\n";

const char* jucer_NewComponentTemplate_cpp = (const char*) temp_binary_data_15;

//================== jucer_NewComponentTemplate.h ==================
static const unsigned char temp_binary_data_16[] =
"/*\r\n"
"  ==============================================================================\r\n"
"\r\n"
"    FILENAME\r\n"
"    Created: DATE\r\n"
"    Author:  AUTHOR\r\n"
"\r\n"
"  ==============================================================================\r\n"
"*/\r\n"
"\r\n"
"#ifndef HEADERGUARD\r\n"
"#define HEADERGUARD\r\n"
"\r\n"
"INCLUDE_JUCE\r\n"
"\r\n"
"//==============================================================================\r\n"
"/*\r\n"
"*/\r\n"
"class COMPONENTCLASS    : public Component\r\n"
"{\r\n"
"public:\r\n"
"    COMPONENTCLASS();\r\n"
"    ~COMPONENTCLASS();\r\n"
"\r\n"
"    void paint (Graphics&);\r\n"
"    void resized();\r\n"
"\r\n"
"private:\r\n"
"    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (COMPONENTCLASS)\r\n"
"};\r\n"
"\r\n"
"\r\n"
"#endif  // HEADERGUARD\r\n";

const char* jucer_NewComponentTemplate_h = (const char*) temp_binary_data_16;

//================== jucer_NewCppFileTemplate.cpp ==================
static const unsigned char temp_binary_data_17[] =
"/*\r\n"
"  ==============================================================================\r\n"
"\r\n"
"    FILENAME\r\n"
"    Created: DATE\r\n"
"    Author:  AUTHOR\r\n"
"\r\n"
"  ==============================================================================\r\n"
"*/\r\n"
"\r\n"
"INCLUDE_CORRESPONDING_HEADER\r\n";

const char* jucer_NewCppFileTemplate_cpp = (const char*) temp_binary_data_17;

//================== jucer_NewCppFileTemplate.h ==================
static const unsigned char temp_binary_data_18[] =
"/*\r\n"
"  ==============================================================================\r\n"
"\r\n"
"    FILENAME\r\n"
"    Created: DATE\r\n"
"    Author:  AUTHOR\r\n"
"\r\n"
"  ==============================================================================\r\n"
"*/\r\n"
"\r\n"
"#ifndef HEADERGUARD\r\n"
"#define HEADERGUARD\r\n"
"\r\n"
"\r\n"
"\r\n"
"\r\n"
"\r\n"
"#endif  // HEADERGUARD\r\n";

const char* jucer_NewCppFileTemplate_h = (const char*) temp_binary_data_18;

//================== jucer_NewInlineComponentTemplate.h ==================
static const unsigned char temp_binary_data_19[] =
"/*\r\n"
"  ==============================================================================\r\n"
"\r\n"
"    FILENAME\r\n"
"    Created: DATE\r\n"
"    Author:  AUTHOR\r\n"
"\r\n"
"  ==============================================================================\r\n"
"*/\r\n"
"\r\n"
"#ifndef HEADERGUARD\r\n"
"#define HEADERGUARD\r\n"
"\r\n"
"INCLUDE_JUCE\r\n"
"\r\n"
"//==============================================================================\r\n"
"/*\r\n"
"*/\r\n"
"class COMPONENTCLASS    : public Component\r\n"
"{\r\n"
"public:\r\n"
"    COMPONENTCLASS()\r\n"
"    {\r\n"
"        // In your constructor, you should add any child components, and\r\n"
"        // initialise any special settings that your component needs.\r\n"
"\r\n"
"    }\r\n"
"\r\n"
"    ~COMPONENTCLASS()\r\n"
"    {\r\n"
"    }\r\n"
"\r\n"
"    void paint (Graphics& g)\r\n"
"    {\r\n"
"        /* This demo code just fills the component's background and\r\n"
"           draws some placeholder text to get you started.\r\n"
"\r\n"
"           You should replace everything in this method with your own\r\n"
"           drawing code..\r\n"
"        */\r\n"
"\r\n"
"        g.fillAll (Colours::white);   // clear the background\r\n"
"\r\n"
"        g.setColour (Colours::grey);\r\n"
"        g.drawRect (getLocalBounds(), 1);   // draw an outline around the component\r\n"
"\r\n"
"        g.setColour (Colours::lightblue);\r\n"
"        g.setFont (14.0f);\r\n"
"        g.drawText (\"COMPONENTCLASS\", getLocalBounds(),\r\n"
"                    Justification::centred, true);   // draw some placeholder text\r\n"
"    }\r\n"
"\r\n"
"    void resized()\r\n"
"    {\r\n"
"        // This method is where you should set the bounds of any child\r\n"
"        // components that your component contains..\r\n"
"\r\n"
"    }\r\n"
"\r\n"
"private:\r\n"
"    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (COMPONENTCLASS)\r\n"
"};\r\n"
"\r\n"
"\r\n"
"#endif  // HEADERGUARD\r\n";

const char* jucer_NewInlineComponentTemplate_h = (const char*) temp_binary_data_19;

//================== jucer_OpenGLComponentTemplate.cpp ==================
static const unsigned char temp_binary_data_20[] =
"/*\r\n"
"  ==============================================================================\r\n"
"\r\n"
"    This file was auto-generated!\r\n"
"\r\n"
"  ==============================================================================\r\n"
"*/\r\n"
"\r\n"
"#ifndef MAINCOMPONENT_H_INCLUDED\r\n"
"#define MAINCOMPONENT_H_INCLUDED\r\n"
"\r\n"
"INCLUDE_JUCE\r\n"
"\r\n"
"//==============================================================================\r\n"
"/*\r\n"
"    This component lives inside our window, and this is where you should put all\r\n"
"    your controls and content.\r\n"
"*/\r\n"
"class MainContentComponent   : public OpenGLAppComponent\r\n"
"{\r\n"
"public:\r\n"
"    //==============================================================================\r\n"
"    MainContentComponent()\r\n"
"    {\r\n"
"        setSize (800, 600);\r\n"
"    }\r\n"
"\r\n"
"    ~MainContentComponent()\r\n"
"    {\r\n"
"        shutdownOpenGL();\r\n"
"    }\r\n"
"\r\n"
"    void initialise() override\r\n"
"    {\r\n"
"    }\r\n"
"\r\n"
"    void shutdown() override\r\n"
"    {\r\n"
"    }\r\n"
"\r\n"
"    void render() override\r\n"
"    {\r\n"
"        OpenGLHelpers::clear (Colours::black);\r\n"
"\r\n"
"    }\r\n"
"\r\n"
"    void paint (Graphics& g) override\r\n"
"    {\r\n"
"        // You can add your component specific drawing code here!\r\n"
"        // This will draw over the top of the openGL background.\r\n"
"    }\r\n"
"\r\n"
"    void resized() override\r\n"
"    {\r\n"
"        // This is called when the MainContentComponent is resized.\r\n"
"        // If you add any child components, this is where you should\r\n"
"        // update their positions.\r\n"
"    }\r\n"
"\r\n"
"\r\n"
"private:\r\n"
"    //==============================================================================\r\n"
"\r\n"
"    // private member variables\r\n"
"\r\n"
"\r\n"
"\r\n"
"    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)\r\n"
"};\r\n"
"\r\n"
"\r\n"
"// (This function is called by the app startup code to create our main component)\r\n"
"Component* createMainContentComponent()    { return new MainContentComponent(); }\r\n"
"\r\n"
"\r\n"
"#endif  // MAINCOMPONENT_H_INCLUDED\r\n";

const char* jucer_OpenGLComponentTemplate_cpp = (const char*) temp_binary_data_20;

//================== AudioPluginXCodeScript.txt ==================
static const unsigned char temp_binary_data_21[] =
"\r\n"
"# This script takes the build product and copies it to the AU, VST, VST3, RTAS and AAX folders, depending on \r\n"
"# which plugin types you've built\r\n"
"\r\n"
"original=$CONFIGURATION_BUILD_DIR/$FULL_PRODUCT_NAME\r\n"
"\r\n"
"# this looks inside the binary to detect which platforms are needed.. \r\n"
"copyAU=`nm -g \"$CONFIGURATION_BUILD_DIR/$EXECUTABLE_PATH\" | grep -i 'AudioUnit' | wc -l`\r\n"
"copyVST=`nm -g \"$CONFIGURATION_BUILD_DIR/$EXECUTABLE_PATH\" | grep -i 'VSTPlugin' | wc -l`\r\n"
"copyVST3=`nm -g \"$CONFIGURATION_BUILD_DIR/$EXECUTABLE_PATH\" | grep -i 'GetPluginFactory' | wc -l`\r\n"
"copyRTAS=`nm -g \"$CONFIGURATION_BUILD_DIR/$EXECUTABLE_PATH\" | grep -i 'CProcess' | wc -l`\r\n"
"copyAAX=`nm -g \"$CONFIGURATION_BUILD_DIR/$EXECUTABLE_PATH\" | grep -i 'ACFStartup' | wc -l`\r\n"
"\r\n"
"if [ $copyAU -gt 0 ]; then\r\n"
"  echo \"Copying to AudioUnit folder...\"\r\n"
"  AU=~/Library/Audio/Plug-Ins/Components/$PRODUCT_NAME.component\r\n"
"  if [ -d \"$AU\" ]; then \r\n"
"    rm -r \"$AU\"\r\n"
"  fi\r\n"
"\r\n"
"  cp -r \"$original\" \"$AU\"\r\n"
"  sed -i \"\" -e 's/TDMwPTul/BNDLPTul/g' \"$AU/Contents/PkgInfo\"\r\n"
"  sed -i \"\" -e 's/TDMw/BNDL/g' \"$AU/Contents/$INFOPLIST_FILE\"\r\n"
"\r\n"
"  # Fix info.plist for AUs built with Xcode 3\r\n"
"  if [ -f \"$DEVELOPER_DIR/Library/Developer/CoreAudio/AudioUnits/AUPublic/AUBase/AUPlugInDispatch.cpp\" ]; then\r\n"
"    echo\r\n"
"  else\r\n"
"    echo \"Removing AudioComponents entry from Info.plist because this is not a new-format AU\"\r\n"
"    /usr/libexec/PlistBuddy -c \"Delete AudioComponents\" \"$AU/Contents/Info.plist\"\r\n"
"  fi\r\n"
"fi\r\n"
"\r\n"
"if [ $copyVST -gt 0 ]; then\r\n"
"  echo \"Copying to VST folder...\"\r\n"
"  VST=~/Library/Audio/Plug-Ins/VST/$PRODUCT_NAME.vst\r\n"
"  if [ -d \"$VST\" ]; then \r\n"
"    rm -r \"$VST\"\r\n"
"  fi\r\n"
"\r\n"
"  cp -r \"$original\" \"$VST\"\r\n"
"  sed -i \"\" -e 's/TDMwPTul/BNDLPTul/g' \"$VST/Contents/PkgInfo\"\r\n"
"  sed -i \"\" -e 's/TDMw/BNDL/g' \"$VST/Contents/$INFOPLIST_FILE\"\r\n"
"fi\r\n"
"\r\n"
"if [ $copyVST3 -gt 0 ]; then\r\n"
"  echo \"Copying to VST3 folder...\"\r\n"
"  VST3=~/Library/Audio/Plug-Ins/VST3/$PRODUCT_NAME.vst3\r\n"
"  if [ -d \"$VST3\" ]; then \r\n"
"    rm -r \"$VST3\"\r\n"
"  fi\r\n"
"\r\n"
"  cp -r \"$original\" \"$VST3\"\r\n"
"  sed -i \"\" -e 's/TDMwPTul/BNDLPTul/g' \"$VST3/Contents/PkgInfo\"\r\n"
"  sed -i \"\" -e 's/TDMw/BNDL/g' \"$VST3/Contents/$INFOPLIST_FILE\"\r\n"
"fi\r\n"
"\r\n"
"if [ $copyRTAS -gt 0 ]; then\r\n"
"  echo \"Copying to RTAS folder...\"\r\n"
"  RTAS=/Library/Application\\ Support/Digidesign/Plug-Ins/$PRODUCT_NAME.dpm\r\n"
"  if [ -d \"$RTAS\" ]; then\r\n"
"    rm -r \"$RTAS\"\r\n"
"  fi\r\n"
"\r\n"
"  cp -r \"$original\" \"$RTAS\"\r\n"
"fi\r\n"
"\r\n"
"if [ $copyAAX -gt 0 ]; then\r\n"
"  echo \"Copying to AAX folder...\"\r\n"
"\r\n"
"  if [ -d \"/Applications/ProTools_3PDev/Plug-Ins\" ]; then\r\n"
"    AAX1=\"/Applications/ProTools_3PDev/Plug-Ins/$PRODUCT_NAME.aaxplugin\"\r\n"
"\r\n"
"    if [ -d \"$AAX1\" ]; then\r\n"
"      rm -r \"$AAX1\"\r\n"
"    fi\r\n"
"\r\n"
"    cp -R -H \"$original\" \"$AAX1\"\r\n"
"  fi\r\n"
"\r\n"
"  if [ -d \"/Library/Application Support/Avid/Audio/Plug-Ins\" ]; then\r\n"
"    AAX2=\"/Library/Application Support/Avid/Audio/Plug-Ins/$PRODUCT_NAME.aaxplugin\"\r\n"
"\r\n"
"    if [ -d \"$AAX2\" ]; then\r\n"
"      rm -r \"$AAX2\"\r\n"
"    fi\r\n"
"\r\n"
"    cp -R -H \"$original\" \"$AAX2\"\r\n"
"  fi\r\n"
"fi\r\n";

const char* AudioPluginXCodeScript_txt = (const char*) temp_binary_data_21;

//================== background_tile.png ==================
static const unsigned char temp_binary_data_22[] =
{ 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,7,0,0,0,7,8,6,0,0,0,196,82,87,211,0,0,0,94,73,68,65,84,120,218,85,141,73,14,0,33,8,4,253,137,226,18,19,245,234,255,127,70,75,155,232,56,135,10,132,94,112,33,4,37,222,123,205,57,107,74,105,239,196,137,
8,72,239,29,99,12,204,57,209,90,227,237,19,45,113,161,209,12,234,172,18,49,70,88,229,134,34,103,173,245,159,60,134,82,10,238,79,166,223,106,238,91,100,229,73,191,80,92,47,179,68,223,148,158,98,226,0,0,0,0,73,69,78,68,174,66,96,130,0,0 };

const char* background_tile_png = (const char*) temp_binary_data_22;

//================== colourscheme_dark.xml ==================
static const unsigned char temp_binary_data_23[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
"\r\n"
"<COLOUR_SCHEME font=\"&lt;Monospaced&gt;; 13.0\">\r\n"
"  <COLOUR name=\"Main Window Bkgd\" colour=\"FF29292A\"/>\r\n"
"  <COLOUR name=\"Treeview Highlight\" colour=\"2BFFFEC3\"/>\r\n"
"  <COLOUR name=\"Code Background\" colour=\"FF222222\"/>\r\n"
"  <COLOUR name=\"Line Number Bkgd\" colour=\"44C1C1C1\"/>\r\n"
"  <COLOUR name=\"Line Numbers\" colour=\"E9B2B2B2\"/>\r\n"
"  <COLOUR name=\"Plain Text\" colour=\"FFCECECE\"/>\r\n"
"  <COLOUR name=\"Selected Text Bkgd\" colour=\"FF2859AC\"/>\r\n"
"  <COLOUR name=\"Caret\" colour=\"FFFFFFFF\"/>\r\n"
"  <COLOUR name=\"Preprocessor Text\" colour=\"FFF8F631\"/>\r\n"
"  <COLOUR name=\"Punctuation\" colour=\"FFCFBEFF\"/>\r\n"
"  <COLOUR name=\"Bracket\" colour=\"FF058202\"/>\r\n"
"  <COLOUR name=\"String\" colour=\"FFBC45DD\"/>\r\n"
"  <COLOUR name=\"Float\" colour=\"ff885500\"/>\r\n"
"  <COLOUR name=\"Integer\" colour=\"FF42C8C4\"/>\r\n"
"  <COLOUR name=\"Identifier\" colour=\"FFCFCFCF\"/>\r\n"
"  <COLOUR name=\"Operator\" colour=\"FFC4EB19\"/>\r\n"
"  <COLOUR name=\"Keyword\" colour=\"FFEE6F6F\"/>\r\n"
"  <COLOUR name=\"Comment\" colour=\"FF72D20C\"/>\r\n"
"  <COLOUR name=\"Error\" colour=\"FFE60000\"/>\r\n"
"</COLOUR_SCHEME>\r\n";

const char* colourscheme_dark_xml = (const char*) temp_binary_data_23;

//================== colourscheme_light.xml ==================
static const unsigned char temp_binary_data_24[] =
"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n"
"\r\n"
"<COLOUR_SCHEME font=\"&lt;Monospaced&gt;; 13.0\">\r\n"
"  <COLOUR name=\"Main Window Bkgd\" colour=\"FFE6E7E9\"/>\r\n"
"  <COLOUR name=\"Treeview Highlight\" colour=\"401111ee\"/>\r\n"
"  <COLOUR name=\"Code Background\" colour=\"ffffffff\"/>\r\n"
"  <COLOUR name=\"Line Number Bkgd\" colour=\"44999999\"/>\r\n"
"  <COLOUR name=\"Line Numbers\" colour=\"44000000\"/>\r\n"
"  <COLOUR name=\"Plain Text\" colour=\"ff000000\"/>\r\n"
"  <COLOUR name=\"Selected Text Bkgd\" colour=\"401111ee\"/>\r\n"
"  <COLOUR name=\"Caret\" colour=\"ff000000\"/>\r\n"
"  <COLOUR name=\"Preprocessor Text\" colour=\"ff660000\"/>\r\n"
"  <COLOUR name=\"Punctuation\" colour=\"ff004400\"/>\r\n"
"  <COLOUR name=\"Bracket\" colour=\"ff000055\"/>\r\n"
"  <COLOUR name=\"String\" colour=\"ff990099\"/>\r\n"
"  <COLOUR name=\"Float\" colour=\"ff885500\"/>\r\n"
"  <COLOUR name=\"Integer\" colour=\"ff880000\"/>\r\n"
"  <COLOUR name=\"Identifier\" colour=\"ff000000\"/>\r\n"
"  <COLOUR name=\"Operator\" colour=\"ff225500\"/>\r\n"
"  <COLOUR name=\"Keyword\" colour=\"ff0000cc\"/>\r\n"
"  <COLOUR name=\"Comment\" colour=\"ff00aa00\"/>\r\n"
"  <COLOUR name=\"Error\" colour=\"ffcc0000\"/>\r\n"
"</COLOUR_SCHEME>\r\n";

const char* colourscheme_light_xml = (const char*) temp_binary_data_24;

//================== juce_icon.png ==================
static const unsigned char temp_binary_data_25[] =
{ 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,128,0,0,0,128,8,6,0,0,0,195,62,97,203,0,0,0,9,112,72,89,115,0,0,11,19,0,0,11,19,1,0,154,156,24,0,0,67,120,105,84,88,116,88,77,76,58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,
112,97,99,107,101,116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,10,60,120,58,120,109,112,109,101,116,97,32,120,109,108,110,115,58,120,61,34,
97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,54,45,99,48,49,52,32,55,57,46,49,53,54,55,57,55,44,32,50,48,49,52,47,48,56,47,50,48,45,48,57,58,53,51,58,
48,50,32,32,32,32,32,32,32,32,34,62,10,32,32,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,34,104,116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,
120,45,110,115,35,34,62,10,32,32,32,32,32,32,60,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,34,10,32,32,32,32,32,32,32,32,32,32,32,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,
47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,34,10,32,32,32,32,32,32,32,32,32,32,32,32,120,109,108,110,115,58,100,99,61,34,104,116,116,112,58,47,47,112,117,114,108,46,111,114,103,47,100,99,47,101,108,101,109,101,110,116,
115,47,49,46,49,47,34,10,32,32,32,32,32,32,32,32,32,32,32,32,120,109,108,110,115,58,112,104,111,116,111,115,104,111,112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,112,104,111,116,111,115,104,111,112,47,49,46,48,47,34,
10,32,32,32,32,32,32,32,32,32,32,32,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,109,109,47,34,10,32,32,32,32,32,32,32,32,32,32,32,32,120,109,108,110,
115,58,115,116,69,118,116,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,115,111,117,114,99,101,69,118,101,110,116,35,34,10,32,32,32,32,32,32,32,32,32,32,32,32,120,109,
108,110,115,58,115,116,82,101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,115,111,117,114,99,101,82,101,102,35,34,10,32,32,32,32,32,32,32,32,32,32,32,32,120,109,
108,110,115,58,116,105,102,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,116,105,102,102,47,49,46,48,47,34,10,32,32,32,32,32,32,32,32,32,32,32,32,120,109,108,110,115,58,101,120,105,102,61,34,104,116,116,112,58,47,47,
110,115,46,97,100,111,98,101,46,99,111,109,47,101,120,105,102,47,49,46,48,47,34,62,10,32,32,32,32,32,32,32,32,32,60,120,109,112,58,67,114,101,97,116,111,114,84,111,111,108,62,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,67,32,50,48,49,
52,32,40,77,97,99,105,110,116,111,115,104,41,60,47,120,109,112,58,67,114,101,97,116,111,114,84,111,111,108,62,10,32,32,32,32,32,32,32,32,32,60,120,109,112,58,67,114,101,97,116,101,68,97,116,101,62,50,48,49,52,45,49,49,45,49,49,84,49,48,58,49,51,58,53,
56,90,60,47,120,109,112,58,67,114,101,97,116,101,68,97,116,101,62,10,32,32,32,32,32,32,32,32,32,60,120,109,112,58,77,111,100,105,102,121,68,97,116,101,62,50,48,49,52,45,49,49,45,49,51,84,49,49,58,50,51,58,51,54,90,60,47,120,109,112,58,77,111,100,105,
102,121,68,97,116,101,62,10,32,32,32,32,32,32,32,32,32,60,120,109,112,58,77,101,116,97,100,97,116,97,68,97,116,101,62,50,48,49,52,45,49,49,45,49,51,84,49,49,58,50,51,58,51,54,90,60,47,120,109,112,58,77,101,116,97,100,97,116,97,68,97,116,101,62,10,32,
32,32,32,32,32,32,32,32,60,100,99,58,102,111,114,109,97,116,62,105,109,97,103,101,47,112,110,103,60,47,100,99,58,102,111,114,109,97,116,62,10,32,32,32,32,32,32,32,32,32,60,112,104,111,116,111,115,104,111,112,58,67,111,108,111,114,77,111,100,101,62,51,
60,47,112,104,111,116,111,115,104,111,112,58,67,111,108,111,114,77,111,100,101,62,10,32,32,32,32,32,32,32,32,32,60,112,104,111,116,111,115,104,111,112,58,68,111,99,117,109,101,110,116,65,110,99,101,115,116,111,114,115,62,10,32,32,32,32,32,32,32,32,32,
32,32,32,60,114,100,102,58,66,97,103,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,114,100,102,58,108,105,62,120,109,112,46,100,105,100,58,101,98,102,56,101,56,50,55,45,99,99,101,50,45,52,99,48,53,45,97,97,98,54,45,52,102,100,55,54,50,52,52,51,
56,50,51,60,47,114,100,102,58,108,105,62,10,32,32,32,32,32,32,32,32,32,32,32,32,60,47,114,100,102,58,66,97,103,62,10,32,32,32,32,32,32,32,32,32,60,47,112,104,111,116,111,115,104,111,112,58,68,111,99,117,109,101,110,116,65,110,99,101,115,116,111,114,115,
62,10,32,32,32,32,32,32,32,32,32,60,120,109,112,77,77,58,73,110,115,116,97,110,99,101,73,68,62,120,109,112,46,105,105,100,58,49,56,101,97,99,100,52,49,45,55,102,54,52,45,52,48,97,57,45,56,99,102,51,45,51,100,51,55,97,101,54,51,57,50,97,52,60,47,120,109,
112,77,77,58,73,110,115,116,97,110,99,101,73,68,62,10,32,32,32,32,32,32,32,32,32,60,120,109,112,77,77,58,68,111,99,117,109,101,110,116,73,68,62,97,100,111,98,101,58,100,111,99,105,100,58,112,104,111,116,111,115,104,111,112,58,98,48,97,56,48,51,101,57,
45,97,98,98,51,45,49,49,55,55,45,98,102,54,100,45,102,56,98,101,56,51,57,100,48,101,51,102,60,47,120,109,112,77,77,58,68,111,99,117,109,101,110,116,73,68,62,10,32,32,32,32,32,32,32,32,32,60,120,109,112,77,77,58,79,114,105,103,105,110,97,108,68,111,99,
117,109,101,110,116,73,68,62,120,109,112,46,100,105,100,58,52,101,100,98,50,55,97,49,45,57,98,56,53,45,52,102,53,52,45,97,51,101,57,45,100,55,53,98,98,101,97,98,100,99,100,54,60,47,120,109,112,77,77,58,79,114,105,103,105,110,97,108,68,111,99,117,109,
101,110,116,73,68,62,10,32,32,32,32,32,32,32,32,32,60,120,109,112,77,77,58,72,105,115,116,111,114,121,62,10,32,32,32,32,32,32,32,32,32,32,32,32,60,114,100,102,58,83,101,113,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,114,100,102,58,108,105,
32,114,100,102,58,112,97,114,115,101,84,121,112,101,61,34,82,101,115,111,117,114,99,101,34,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,69,118,116,58,97,99,116,105,111,110,62,99,114,101,97,116,101,100,60,47,115,116,69,118,116,
58,97,99,116,105,111,110,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,69,118,116,58,105,110,115,116,97,110,99,101,73,68,62,120,109,112,46,105,105,100,58,52,101,100,98,50,55,97,49,45,57,98,56,53,45,52,102,53,52,45,97,51,101,57,
45,100,55,53,98,98,101,97,98,100,99,100,54,60,47,115,116,69,118,116,58,105,110,115,116,97,110,99,101,73,68,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,69,118,116,58,119,104,101,110,62,50,48,49,52,45,49,49,45,49,49,84,49,48,
58,49,51,58,53,56,90,60,47,115,116,69,118,116,58,119,104,101,110,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,69,118,116,58,115,111,102,116,119,97,114,101,65,103,101,110,116,62,65,100,111,98,101,32,80,104,111,116,111,115,104,
111,112,32,67,67,32,50,48,49,52,32,40,77,97,99,105,110,116,111,115,104,41,60,47,115,116,69,118,116,58,115,111,102,116,119,97,114,101,65,103,101,110,116,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,47,114,100,102,58,108,105,62,10,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,60,114,100,102,58,108,105,32,114,100,102,58,112,97,114,115,101,84,121,112,101,61,34,82,101,115,111,117,114,99,101,34,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,69,118,116,58,97,99,116,105,111,
110,62,115,97,118,101,100,60,47,115,116,69,118,116,58,97,99,116,105,111,110,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,69,118,116,58,105,110,115,116,97,110,99,101,73,68,62,120,109,112,46,105,105,100,58,52,54,55,53,52,52,56,
54,45,56,49,98,100,45,52,57,57,101,45,57,100,56,50,45,52,56,48,56,51,53,99,55,100,100,98,98,60,47,115,116,69,118,116,58,105,110,115,116,97,110,99,101,73,68,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,69,118,116,58,119,104,101,
110,62,50,48,49,52,45,49,49,45,49,51,84,49,49,58,48,55,58,53,57,90,60,47,115,116,69,118,116,58,119,104,101,110,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,69,118,116,58,115,111,102,116,119,97,114,101,65,103,101,110,116,62,65,
100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,67,32,50,48,49,52,32,40,77,97,99,105,110,116,111,115,104,41,60,47,115,116,69,118,116,58,115,111,102,116,119,97,114,101,65,103,101,110,116,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,60,115,116,69,118,116,58,99,104,97,110,103,101,100,62,47,60,47,115,116,69,118,116,58,99,104,97,110,103,101,100,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,47,114,100,102,58,108,105,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,
114,100,102,58,108,105,32,114,100,102,58,112,97,114,115,101,84,121,112,101,61,34,82,101,115,111,117,114,99,101,34,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,69,118,116,58,97,99,116,105,111,110,62,115,97,118,101,100,60,47,115,
116,69,118,116,58,97,99,116,105,111,110,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,69,118,116,58,105,110,115,116,97,110,99,101,73,68,62,120,109,112,46,105,105,100,58,102,48,102,100,48,57,99,49,45,53,97,49,98,45,52,55,99,97,
45,98,48,55,56,45,55,51,99,56,51,51,57,53,57,99,56,98,60,47,115,116,69,118,116,58,105,110,115,116,97,110,99,101,73,68,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,69,118,116,58,119,104,101,110,62,50,48,49,52,45,49,49,45,49,51,
84,49,49,58,48,57,58,49,56,90,60,47,115,116,69,118,116,58,119,104,101,110,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,69,118,116,58,115,111,102,116,119,97,114,101,65,103,101,110,116,62,65,100,111,98,101,32,80,104,111,116,111,
115,104,111,112,32,67,67,32,50,48,49,52,32,40,77,97,99,105,110,116,111,115,104,41,60,47,115,116,69,118,116,58,115,111,102,116,119,97,114,101,65,103,101,110,116,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,69,118,116,58,99,104,
97,110,103,101,100,62,47,60,47,115,116,69,118,116,58,99,104,97,110,103,101,100,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,47,114,100,102,58,108,105,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,114,100,102,58,108,105,32,114,100,102,
58,112,97,114,115,101,84,121,112,101,61,34,82,101,115,111,117,114,99,101,34,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,69,118,116,58,97,99,116,105,111,110,62,100,101,114,105,118,101,100,60,47,115,116,69,118,116,58,97,99,116,
105,111,110,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,69,118,116,58,112,97,114,97,109,101,116,101,114,115,62,99,111,110,118,101,114,116,101,100,32,102,114,111,109,32,105,109,97,103,101,47,112,110,103,32,116,111,32,97,112,
112,108,105,99,97,116,105,111,110,47,118,110,100,46,97,100,111,98,101,46,112,104,111,116,111,115,104,111,112,60,47,115,116,69,118,116,58,112,97,114,97,109,101,116,101,114,115,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,47,114,100,102,58,108,
105,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,114,100,102,58,108,105,32,114,100,102,58,112,97,114,115,101,84,121,112,101,61,34,82,101,115,111,117,114,99,101,34,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,69,118,
116,58,97,99,116,105,111,110,62,115,97,118,101,100,60,47,115,116,69,118,116,58,97,99,116,105,111,110,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,69,118,116,58,105,110,115,116,97,110,99,101,73,68,62,120,109,112,46,105,105,100,
58,52,102,51,49,54,52,102,99,45,56,100,53,101,45,52,99,99,102,45,97,50,50,50,45,101,57,51,54,53,55,55,49,55,55,54,99,60,47,115,116,69,118,116,58,105,110,115,116,97,110,99,101,73,68,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,
69,118,116,58,119,104,101,110,62,50,48,49,52,45,49,49,45,49,51,84,49,49,58,49,57,58,53,56,90,60,47,115,116,69,118,116,58,119,104,101,110,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,69,118,116,58,115,111,102,116,119,97,114,101,
65,103,101,110,116,62,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,67,32,50,48,49,52,32,40,77,97,99,105,110,116,111,115,104,41,60,47,115,116,69,118,116,58,115,111,102,116,119,97,114,101,65,103,101,110,116,62,10,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,60,115,116,69,118,116,58,99,104,97,110,103,101,100,62,47,60,47,115,116,69,118,116,58,99,104,97,110,103,101,100,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,47,114,100,102,58,108,105,62,10,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,60,114,100,102,58,108,105,32,114,100,102,58,112,97,114,115,101,84,121,112,101,61,34,82,101,115,111,117,114,99,101,34,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,69,118,116,58,97,99,116,105,111,110,62,100,
101,114,105,118,101,100,60,47,115,116,69,118,116,58,97,99,116,105,111,110,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,69,118,116,58,112,97,114,97,109,101,116,101,114,115,62,99,111,110,118,101,114,116,101,100,32,102,114,111,
109,32,97,112,112,108,105,99,97,116,105,111,110,47,118,110,100,46,97,100,111,98,101,46,112,104,111,116,111,115,104,111,112,32,116,111,32,105,109,97,103,101,47,112,110,103,60,47,115,116,69,118,116,58,112,97,114,97,109,101,116,101,114,115,62,10,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,60,47,114,100,102,58,108,105,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,114,100,102,58,108,105,32,114,100,102,58,112,97,114,115,101,84,121,112,101,61,34,82,101,115,111,117,114,99,101,34,62,10,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,69,118,116,58,97,99,116,105,111,110,62,115,97,118,101,100,60,47,115,116,69,118,116,58,97,99,116,105,111,110,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,69,118,116,58,105,
110,115,116,97,110,99,101,73,68,62,120,109,112,46,105,105,100,58,49,56,101,97,99,100,52,49,45,55,102,54,52,45,52,48,97,57,45,56,99,102,51,45,51,100,51,55,97,101,54,51,57,50,97,52,60,47,115,116,69,118,116,58,105,110,115,116,97,110,99,101,73,68,62,10,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,69,118,116,58,119,104,101,110,62,50,48,49,52,45,49,49,45,49,51,84,49,49,58,50,51,58,51,54,90,60,47,115,116,69,118,116,58,119,104,101,110,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,60,115,116,69,118,116,58,115,111,102,116,119,97,114,101,65,103,101,110,116,62,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,67,32,50,48,49,52,32,40,77,97,99,105,110,116,111,115,104,41,60,47,115,116,69,118,116,58,115,111,102,116,
119,97,114,101,65,103,101,110,116,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,69,118,116,58,99,104,97,110,103,101,100,62,47,60,47,115,116,69,118,116,58,99,104,97,110,103,101,100,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,60,47,114,100,102,58,108,105,62,10,32,32,32,32,32,32,32,32,32,32,32,32,60,47,114,100,102,58,83,101,113,62,10,32,32,32,32,32,32,32,32,32,60,47,120,109,112,77,77,58,72,105,115,116,111,114,121,62,10,32,32,32,32,32,32,32,32,32,60,120,109,112,77,77,
58,68,101,114,105,118,101,100,70,114,111,109,32,114,100,102,58,112,97,114,115,101,84,121,112,101,61,34,82,101,115,111,117,114,99,101,34,62,10,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,62,120,109,
112,46,105,105,100,58,52,102,51,49,54,52,102,99,45,56,100,53,101,45,52,99,99,102,45,97,50,50,50,45,101,57,51,54,53,55,55,49,55,55,54,99,60,47,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,62,10,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,
82,101,102,58,100,111,99,117,109,101,110,116,73,68,62,120,109,112,46,100,105,100,58,52,102,51,49,54,52,102,99,45,56,100,53,101,45,52,99,99,102,45,97,50,50,50,45,101,57,51,54,53,55,55,49,55,55,54,99,60,47,115,116,82,101,102,58,100,111,99,117,109,101,110,
116,73,68,62,10,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,82,101,102,58,111,114,105,103,105,110,97,108,68,111,99,117,109,101,110,116,73,68,62,120,109,112,46,100,105,100,58,52,101,100,98,50,55,97,49,45,57,98,56,53,45,52,102,53,52,45,97,51,101,57,
45,100,55,53,98,98,101,97,98,100,99,100,54,60,47,115,116,82,101,102,58,111,114,105,103,105,110,97,108,68,111,99,117,109,101,110,116,73,68,62,10,32,32,32,32,32,32,32,32,32,60,47,120,109,112,77,77,58,68,101,114,105,118,101,100,70,114,111,109,62,10,32,32,
32,32,32,32,32,32,32,60,116,105,102,102,58,79,114,105,101,110,116,97,116,105,111,110,62,49,60,47,116,105,102,102,58,79,114,105,101,110,116,97,116,105,111,110,62,10,32,32,32,32,32,32,32,32,32,60,116,105,102,102,58,88,82,101,115,111,108,117,116,105,111,
110,62,55,50,48,48,48,48,47,49,48,48,48,48,60,47,116,105,102,102,58,88,82,101,115,111,108,117,116,105,111,110,62,10,32,32,32,32,32,32,32,32,32,60,116,105,102,102,58,89,82,101,115,111,108,117,116,105,111,110,62,55,50,48,48,48,48,47,49,48,48,48,48,60,47,
116,105,102,102,58,89,82,101,115,111,108,117,116,105,111,110,62,10,32,32,32,32,32,32,32,32,32,60,116,105,102,102,58,82,101,115,111,108,117,116,105,111,110,85,110,105,116,62,50,60,47,116,105,102,102,58,82,101,115,111,108,117,116,105,111,110,85,110,105,
116,62,10,32,32,32,32,32,32,32,32,32,60,101,120,105,102,58,67,111,108,111,114,83,112,97,99,101,62,54,53,53,51,53,60,47,101,120,105,102,58,67,111,108,111,114,83,112,97,99,101,62,10,32,32,32,32,32,32,32,32,32,60,101,120,105,102,58,80,105,120,101,108,88,
68,105,109,101,110,115,105,111,110,62,49,50,56,60,47,101,120,105,102,58,80,105,120,101,108,88,68,105,109,101,110,115,105,111,110,62,10,32,32,32,32,32,32,32,32,32,60,101,120,105,102,58,80,105,120,101,108,89,68,105,109,101,110,115,105,111,110,62,49,50,
56,60,47,101,120,105,102,58,80,105,120,101,108,89,68,105,109,101,110,115,105,111,110,62,10,32,32,32,32,32,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,10,32,32,32,60,47,114,100,102,58,82,68,70,62,10,60,47,120,58,120,109,112,109,
101,116,97,62,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,
32,32,32,32,32,32,32,32,32,32,32,32,10,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,32,10,60,63,120,112,97,99,107,101,116,32,101,110,100,61,34,119,34,63,62,129,236,100,88,0,0,0,32,99,72,82,77,0,0,122,37,0,0,128,131,
0,0,249,255,0,0,128,233,0,0,117,48,0,0,234,96,0,0,58,152,0,0,23,111,146,95,197,70,0,1,0,149,73,68,65,84,120,1,0,255,255,0,0,1,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,2,0,0,0,2,0,0,0,3,0,0,0,2,0,0,0,2,0,0,0,2,0,0,0,2,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,255,0,0,0,
254,0,0,0,254,0,0,0,254,0,0,0,254,0,0,0,253,0,0,0,254,0,0,0,254,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,4,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,3,0,0,0,4,0,0,0,4,0,0,0,5,0,0,0,6,0,0,0,6,0,0,0,5,0,0,0,5,0,0,0,4,0,0,0,4,0,0,0,3,0,0,0,3,0,0,0,2,0,0,0,2,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,254,0,0,0,254,0,0,0,253,0,0,0,253,0,0,0,252,0,0,0,252,0,0,0,251,0,0,0,251,0,0,0,250,0,0,0,250,0,0,0,251,0,0,0,252,0,0,0,252,0,0,0,253,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,3,0,0,0,4,0,0,0,6,0,0,0,8,0,0,0,7,0,0,0,8,0,0,0,8,0,0,0,9,0,0,0,11,0,0,0,9,0,0,0,8,0,0,
0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,249,0,0,0,248,0,0,0,247,0,0,0,245,0,0,0,247,0,0,0,248,0,0,0,248,0,0,0,249,0,0,0,248,0,0,0,250,0,0,0,252,0,0,0,253,0,0,0,
255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,1,0,0,0,3,0,0,0,4,0,0,0,7,0,0,0,9,0,0,0,11,0,0,0,12,0,0,0,
13,0,0,0,13,0,0,0,12,0,0,0,4,0,0,0,0,0,0,0,21,0,0,0,23,0,0,0,22,0,0,0,20,0,0,0,18,0,0,0,16,0,0,0,13,255,255,255,12,255,0,0,9,0,0,1,7,0,0,0,4,0,0,0,0,0,0,0,252,0,0,255,249,1,0,0,247,0,0,0,244,1,1,1,243,0,0,0,240,0,0,0,238,0,0,0,236,0,0,0,234,0,0,0,233,
0,0,0,235,0,0,0,0,0,0,0,252,0,0,0,244,0,0,0,243,0,0,0,243,0,0,0,244,0,0,0,245,0,0,0,247,0,0,0,249,0,0,0,252,0,0,0,253,228,228,228,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,28,28,28,2,0,0,0,5,0,0,0,7,0,0,0,9,0,0,0,12,0,0,0,14,0,0,0,16,0,0,0,17,0,0,0,7,0,0,0,8,0,0,0,34,0,0,0,32,0,0,0,31,0,0,0,29,254,254,255,27,252,255,0,5,252,253,0,0,253,255,1,0,0,0,1,0,254,255,0,0,5,5,2,0,6,4,4,0,3,1,255,0,3,1,255,0,
255,0,0,0,0,0,0,0,0,0,0,5,255,255,1,5,253,255,1,12,251,254,255,21,251,249,251,0,254,255,0,0,2,1,0,0,2,0,254,0,4,3,0,0,4,2,0,251,2,2,1,229,1,0,0,227,0,0,0,225,0,0,0,224,0,0,0,222,0,0,0,248,0,0,0,249,0,0,0,239,0,0,0,240,0,0,0,242,0,0,0,244,0,0,0,247,28,
28,28,249,0,0,0,251,228,228,228,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,1,0,0,0,2,0,0,0,5,0,0,0,8,0,0,0,13,0,0,0,16,0,0,0,18,0,0,0,18,0,0,0,8,0,0,0,23,0,0,0,40,0,0,0,39,0,0,0,37,253,254,
255,27,249,252,0,0,251,254,1,0,3,2,2,0,13,11,5,0,26,11,255,0,30,19,5,0,32,14,248,0,31,21,11,0,24,6,242,0,23,11,251,0,18,15,4,0,10,4,255,0,8,2,251,0,255,0,0,0,0,0,0,0,1,0,0,0,251,255,2,0,246,252,5,0,243,247,252,0,234,241,251,0,232,252,16,0,226,236,250,
0,226,241,2,0,223,238,0,0,231,242,255,5,239,245,252,32,250,251,254,32,4,1,254,61,5,4,0,0,5,2,1,229,1,1,0,219,0,0,0,217,0,0,0,216,0,0,0,233,0,0,0,248,0,0,0,238,0,0,0,238,0,0,0,240,0,0,0,243,28,28,28,248,0,0,0,251,0,0,0,254,228,228,228,255,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,1,0,0,0,2,0,0,0,5,0,0,0,9,0,0,0,15,0,0,0,18,0,0,0,21,0,0,0,18,0,0,0,17,0,0,0,45,0,0,0,43,0,0,0,43,250,253,255,18,249,251,1,0,1,2,2,0,19,13,4,0,53,25,4,0,51,27,253,0,55,27,254,0,43,21,248,0,29,13,249,
0,0,9,253,0,0,2,251,0,0,0,235,0,0,1,0,0,0,255,255,0,0,254,238,0,0,0,1,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,255,0,0,1,1,0,0,1,1,0,0,1,243,0,0,255,255,0,0,255,7,0,0,249,1,0,238,246,8,0,218,237,3,0,204,231,6,0,203,228,3,0,209,230,254,0,228,239,
251,27,249,249,253,27,7,4,255,64,7,3,0,238,1,1,1,213,0,0,0,213,0,0,0,211,0,0,0,239,0,0,0,238,0,0,0,235,0,0,0,238,0,0,0,241,0,0,0,247,28,28,28,251,0,0,0,254,228,228,228,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,1,0,0,0,1,0,0,0,6,0,0,0,9,0,0,0,15,
0,0,0,19,0,0,0,24,0,0,0,14,0,0,0,39,0,0,0,49,0,0,0,48,253,254,255,30,247,250,0,0,1,4,4,0,31,19,5,0,60,32,1,0,72,38,253,0,57,27,246,0,19,16,249,0,0,4,251,0,0,255,253,0,0,253,0,0,0,253,1,0,0,255,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,1,0,0,0,1,255,0,0,4,0,0,0,2,1,0,0,254,5,0,253,246,7,0,205,231,8,0,188,222,6,0,188,221,1,0,214,230,253,0,245,248,250,0,8,4,255,0,7,4,1,226,0,0,0,208,0,0,0,207,0,0,0,
217,0,0,0,242,0,0,0,232,0,0,0,237,0,0,0,241,0,0,0,247,0,0,0,250,0,0,0,255,228,228,228,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,2,0,0,0,4,0,0,0,9,0,0,0,15,0,0,0,20,0,0,0,25,0,0,0,14,0,0,0,50,0,0,0,53,0,0,0,51,249,252,255,12,248,252,2,0,23,16,6,0,64,36,2,0,87,
44,252,0,67,32,245,0,2,11,247,0,0,0,252,0,0,253,0,0,0,253,1,0,0,255,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,1,0,0,0,1,0,0,0,0,255,0,0,2,255,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,255,255,0,0,254,0,0,0,0,2,0,0,255,0,0,0,255,0,0,0,0,1,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,2,255,0,0,4,0,0,0,2,2,0,0,249,8,0,214,233,10,0,174,215,7,0,178,213,0,0,218,232,251,0,1,255,252,0,10,6,0,244,1,1,1,205,0,0,0,203,0,0,0,206,0,0,0,242,0,0,0,231,0,0,0,236,0,0,0,241,0,0,0,247,0,0,0,252,228,
228,228,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,
0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,28,28,28,1,0,0,0,3,0,0,0,8,0,0,0,15,0,0,0,19,0,0,0,26,0,0,0,17,0,0,0,50,0,0,0,57,0,0,0,55,247,250,255,4,252,0,4,0,44,28,7,0,91,48,254,0,88,43,245,0,18,18,246,0,0,0,250,0,0,251,0,0,0,254,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,255,0,0,3,255,0,
0,2,0,0,0,2,2,0,0,255,0,0,0,255,6,0,0,250,1,0,254,249,255,0,242,254,11,0,246,252,2,0,247,250,255,0,0,0,0,0,0,0,0,0,0,0,0,0,5,3,0,0,11,6,1,0,12,2,246,0,7,6,253,0,0,7,2,0,0,2,249,0,0,1,255,0,0,255,254,0,0,253,0,0,0,253,1,0,0,255,1,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,1,255,0,0,4,255,0,0,3,2,0,0,246,10,0,184,220,12,0,158,206,5,0,192,220,251,0,249,248,250,0,11,6,0,252,2,2,1,201,0,0,0,199,0,0,0,206,0,0,0,239,0,0,0,230,0,0,0,237,0,0,0,241,0,0,0,248,0,0,0,253,228,228,228,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,2,0,0,0,5,0,0,0,12,0,0,0,19,0,0,0,27,0,0,0,24,0,
0,0,40,0,0,0,60,0,0,0,59,247,250,255,7,252,0,4,0,55,33,6,0,107,57,254,0,79,39,242,0,0,9,245,0,0,252,255,0,0,252,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,255,0,0,2,255,0,0,4,0,0,0,2,2,0,0,250,6,0,240,243,7,0,218,239,6,0,214,235,5,0,218,236,1,0,223,237,0,0,233,
246,3,0,236,241,246,0,248,254,7,0,249,253,0,0,250,252,255,0,0,0,1,0,255,0,0,0,0,255,0,0,4,2,0,0,7,5,0,0,8,1,251,0,17,13,5,0,21,12,3,0,28,12,250,0,38,23,5,0,40,19,248,0,42,20,253,0,27,15,249,0,0,8,249,0,0,0,253,0,0,253,255,0,0,253,1,0,0,255,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,1,254,0,0,6,0,0,0,255,8,0,211,227,12,0,149,200,8,0,179,213,254,0,244,246,249,0,12,7,255,249,1,1,1,197,0,0,0,196,0,0,0,216,0,0,0,232,0,0,0,229,0,0,0,237,0,0,0,244,0,0,0,251,228,228,228,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,1,0,0,0,3,0,0,0,8,0,0,0,16,0,0,0,23,0,0,0,30,0,0,0,29,0,0,0,62,0,0,0,63,249,252,255,
20,248,253,3,0,51,31,7,0,118,61,252,0,74,41,242,0,0,6,246,0,0,249,0,0,0,255,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,254,0,0,5,1,0,0,0,5,0,246,241,9,0,201,229,7,0,191,223,4,0,202,228,1,0,221,236,255,0,239,246,254,0,249,250,252,0,0,0,255,0,254,254,254,0,254,255,
0,0,0,1,3,0,2,1,255,0,10,8,6,0,10,6,0,0,5,0,250,0,2,0,254,0,254,0,3,0,250,254,5,0,246,252,0,0,246,248,251,0,253,254,255,0,0,255,254,0,3,2,1,0,2,2,1,0,5,4,3,0,13,9,3,0,30,17,2,0,47,25,1,0,61,31,253,0,62,30,249,0,26,18,247,0,0,6,248,0,0,253,255,0,0,251,
1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,255,0,0,6,255,0,0,1,6,0,220,230,14,0,139,197,10,0,173,208,250,0,253,251,250,0,11,7,1,236,0,0,0,193,0,0,0,194,0,0,0,227,0,0,0,226,0,0,0,233,0,0,0,240,0,0,0,248,0,0,0,253,228,228,228,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,1,0,0,0,5,0,0,0,11,0,0,0,20,0,0,0,28,0,0,0,24,0,0,0,57,0,0,0,65,254,255,255,44,243,249,1,0,33,23,10,
0,117,62,255,0,93,48,238,0,0,7,246,0,0,249,1,0,0,255,1,0,0,0,0,0,0,0,0,0,0,1,255,0,0,5,255,0,0,2,4,0,247,240,11,0,183,221,10,0,178,216,3,0,202,225,254,0,231,242,252,0,252,252,253,0,6,2,255,0,5,3,0,0,251,253,0,0,250,254,1,0,8,6,5,0,27,17,3,0,46,25,1,0,
47,26,0,0,39,18,248,0,32,17,0,0,20,11,1,0,12,0,244,0,5,0,249,0,251,1,8,0,243,254,11,0,237,246,255,0,223,239,1,0,216,236,6,0,207,229,0,0,211,231,255,0,230,241,252,0,250,251,253,0,6,2,254,0,1,2,1,0,251,253,1,0,0,0,2,0,15,12,5,0,44,24,2,0,71,38,255,0,81,
39,248,0,30,23,245,0,0,2,248,0,0,251,0,0,0,254,2,0,0,0,0,0,0,0,0,0,0,0,255,0,0,5,255,0,0,2,7,0,205,222,14,0,129,191,8,0,193,217,248,0,7,4,253,0,6,3,1,212,0,0,0,191,0,0,0,199,0,0,0,232,0,0,0,228,0,0,0,236,0,0,0,245,0,0,0,251,228,228,228,255,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,2,0,0,0,5,0,0,0,14,0,0,0,25,0,0,0,32,0,0,0,32,0,0,0,69,0,0,0,68,247,250,255,8,2,5,7,0,98,55,
4,0,129,63,243,0,8,15,241,0,0,249,255,0,0,255,2,0,0,0,0,0,0,0,0,0,0,1,255,0,0,6,255,0,0,253,8,0,207,226,13,0,163,209,8,0,184,215,254,0,229,240,250,0,1,255,253,0,8,5,0,0,4,2,1,0,254,254,255,0,246,251,0,0,3,4,6,0,43,26,4,0,73,39,0,0,70,35,249,0,49,23,247,
0,2,10,251,0,0,2,250,0,0,255,0,0,0,254,255,0,0,255,0,0,0,0,1,0,0,0,1,0,0,0,255,0,0,0,255,0,0,1,0,0,0,2,1,0,0,1,1,0,0,253,5,0,250,246,6,0,206,231,8,0,182,218,6,0,185,219,2,0,218,233,249,0,0,253,252,0,9,6,0,0,0,0,0,0,249,251,0,0,250,254,2,0,15,10,5,0,55,
33,5,0,91,46,252,0,82,40,243,0,0,11,245,0,0,252,254,0,0,252,2,0,0,0,0,0,0,0,0,0,0,0,255,0,0,6,255,0,0,254,9,0,167,206,17,0,132,189,1,0,226,237,248,0,14,7,255,248,1,1,1,188,0,0,0,187,0,0,0,224,0,0,0,224,0,0,0,231,0,0,0,242,0,0,0,251,228,228,228,254,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,3,0,0,0,9,0,0,0,16,0,0,0,25,0,0,0,34,0,0,0,50,0,0,0,70,255,255,255,48,242,249,2,0,47,30,8,0,142,74,
250,0,54,36,238,0,0,252,252,0,0,252,2,0,0,0,0,0,0,0,0,0,0,1,255,0,0,7,255,0,0,248,10,0,179,217,14,0,154,202,4,0,200,224,252,0,250,249,251,0,9,5,255,0,4,3,1,0,0,0,0,0,254,254,255,0,243,250,1,0,17,13,7,0,75,41,4,0,95,49,248,0,56,29,244,0,0,8,248,0,0,254,
254,0,0,252,0,0,0,254,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,2,255,0,0,4,0,0,0,2,3,0,0,246,10,0,186,222,12,0,159,206,5,0,189,217,250,0,250,250,250,0,11,6,0,0,1,1,1,0,254,255,255,0,247,250,
0,0,254,1,3,0,36,22,6,0,92,51,0,0,99,48,246,0,14,17,242,0,0,250,253,0,0,254,2,0,0,0,0,0,0,0,0,0,0,1,255,0,0,7,0,0,241,238,16,0,114,186,13,0,173,206,248,0,6,3,252,0,6,3,1,208,0,0,0,186,0,0,0,206,0,0,0,222,0,0,0,231,0,0,0,240,0,0,0,247,228,228,228,253,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,1,0,0,0,3,0,0,0,11,0,0,0,20,0,0,0,29,0,0,0,25,0,0,0,70,0,0,0,72,250,253,255,24,251,255,4,0,101,58,8,0,138,68,239,0,0,9,243,
0,0,249,1,0,0,0,1,0,0,0,0,0,0,0,255,0,0,7,255,0,0,251,10,0,176,214,15,0,145,197,4,0,206,225,248,0,3,0,253,0,9,5,0,0,1,1,1,0,0,0,0,0,0,0,0,0,245,249,255,0,10,9,7,0,90,51,4,0,111,55,246,0,28,23,243,0,0,255,250,0,0,251,1,0,0,255,1,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,1,255,0,0,1,255,0,0,2,255,0,0,2,2,0,0,1,0,0,0,255,255,0,0,254,255,0,0,254,1,0,0,255,1,0,0,255,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,254,0,0,6,0,0,0,255,7,0,209,225,14,0,137,197,9,0,184,213,250,0,2,255,250,0,8,4,1,0,0,0,0,0,0,0,0,0,249,
252,255,0,247,252,2,0,27,18,8,0,96,53,1,0,110,53,243,0,11,16,241,0,0,249,255,0,0,255,2,0,0,0,0,0,0,0,0,0,0,4,254,0,0,3,7,0,171,206,20,0,121,183,1,0,237,241,246,0,11,7,1,232,0,0,0,184,0,0,0,186,0,0,0,231,0,0,0,227,0,0,0,236,0,0,0,245,0,0,0,253,228,228,
228,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,1,0,0,0,4,0,0,0,11,0,0,0,24,0,0,0,34,0,0,0,27,0,0,0,75,0,0,0,76,246,249,255,3,17,14,8,0,143,77,255,0,78,48,235,0,0,250,252,0,0,254,2,0,0,
0,0,0,0,0,0,0,0,4,254,0,0,3,7,0,195,218,17,0,132,191,5,0,200,222,248,0,4,0,253,0,9,6,1,0,0,0,0,0,0,0,0,0,0,0,0,0,253,254,255,0,244,251,3,0,69,41,9,0,136,70,246,0,38,28,240,0,0,253,252,0,0,251,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,255,0,0,3,255,0,0,3,1,0,0,
0,3,0,0,249,6,0,241,245,3,0,234,249,11,0,235,240,245,0,249,248,249,0,10,9,9,0,19,15,9,0,21,7,245,0,15,12,253,0,0,6,250,0,0,0,253,0,0,253,255,0,0,252,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,255,0,0,6,255,0,0,0,8,0,187,214,17,0,121,185,3,0,217,231,247,0,14,7,255,
0,1,1,1,0,0,0,0,0,0,0,0,0,252,253,255,0,244,251,2,0,30,19,7,0,111,61,1,0,103,52,240,0,0,7,244,0,0,249,1,0,0,0,1,0,0,0,0,0,0,1,255,0,0,7,0,0,229,234,19,0,98,175,10,0,197,219,245,0,15,7,255,253,1,1,1,180,0,0,0,181,0,0,0,229,0,0,0,222,0,0,0,232,0,0,0,245,
0,0,0,252,228,228,228,255,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,0,0,0,0,28,28,28,1,0,0,0,4,0,0,0,12,0,0,0,25,0,0,0,36,0,0,0,35,0,0,0,78,0,0,0,64,241,248,0,0,49,32,11,0,162,83,246,0,32,25,237,0,0,248,0,0,0,0,1,0,0,0,0,
0,0,1,255,0,0,7,0,0,237,237,17,0,120,188,11,0,173,208,251,0,0,253,250,0,10,6,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,248,252,255,0,4,5,7,0,127,70,3,0,104,57,238,0,0,2,245,0,0,250,2,0,0,0,1,0,0,0,0,0,0,0,0,0,0,2,254,0,0,6,1,0,0,252,7,0,228,236,10,0,195,226,
8,0,196,225,2,0,213,233,0,0,230,240,252,0,245,251,3,0,247,250,251,0,253,252,253,0,4,4,3,0,9,6,5,0,9,4,253,0,29,18,4,0,41,21,254,0,64,34,0,0,62,30,247,0,23,19,246,0,0,2,249,0,0,251,0,0,0,254,2,0,0,0,0,0,0,0,0,0,0,0,255,0,0,8,255,0,254,244,13,0,109,183,
15,0,164,202,249,0,11,5,253,0,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,250,253,255,0,246,251,2,0,51,32,9,0,133,69,249,0,60,38,240,0,0,251,250,0,0,254,2,0,0,0,0,0,0,0,0,0,0,5,253,0,0,251,14,0,113,183,17,0,158,198,248,0,11,5,252,0,2,2,1,192,0,0,0,178,0,0,0,221,
0,0,0,220,0,0,0,231,0,0,0,244,0,0,0,252,228,228,228,255,0,0,0,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,28,28,28,1,0,0,0,4,0,0,0,12,0,0,0,26,0,0,0,37,0,0,0,41,0,0,0,79,255,255,0,55,241,249,1,0,78,48,11,0,164,81,240,0,2,9,241,0,0,250,2,
0,0,0,0,0,0,0,0,0,0,4,254,0,0,2,8,0,165,204,19,0,129,187,0,0,231,239,247,0,14,7,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,248,252,255,0,17,12,8,0,159,86,254,0,60,39,234,0,0,248,254,0,0,255,2,0,0,0,0,0,0,0,0,0,0,4,254,0,0,4,3,0,245,239,12,0,171,216,
11,0,174,213,2,0,207,227,253,0,240,246,252,0,3,0,254,0,4,3,255,0,5,2,0,0,1,1,0,0,2,1,1,0,0,0,0,0,255,0,0,0,255,255,255,0,255,255,1,0,251,254,255,0,251,253,1,0,0,0,2,0,18,13,5,0,53,29,2,0,88,46,253,0,82,40,244,0,0,11,245,0,0,250,254,0,0,254,2,0,0,0,0,
0,0,0,0,0,0,4,254,0,0,3,6,0,150,195,22,0,125,183,250,0,6,1,252,0,3,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,247,250,255,0,0,4,6,0,98,55,7,0,137,68,240,0,2,10,241,0,0,249,1,0,0,0,1,0,0,0,0,0,0,2,254,0,0,5,8,0,151,196,21,0,124,182,251,0,3,0,250,0,6,3,1,201,
0,0,0,177,0,0,0,215,0,0,0,219,0,0,0,230,0,0,0,244,0,0,0,252,228,228,228,255,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,
28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,28,28,28,1,0,0,0,4,0,0,0,12,0,0,0,26,0,0,0,37,0,0,0,45,0,0,0,82,255,255,255,48,241,249,3,0,102,60,9,0,142,76,236,0,0,0,246,0,0,
252,2,0,0,0,0,0,0,0,0,0,0,7,254,0,246,242,17,0,107,181,13,0,172,207,248,0,9,4,252,0,6,3,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,252,253,255,0,9,9,9,0,177,95,251,0,46,31,234,0,0,248,1,0,0,0,1,0,0,0,0,0,0,2,254,0,0,6,2,0,233,235,16,0,146,200,10,0,172,
210,254,0,230,239,250,0,5,2,254,0,8,5,0,0,2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,253,254,255,0,248,251,0,0,254,0,3,0,36,23,7,0,96,52,254,0,107,52,243,0,2,11,244,0,0,249,0,0,0,0,1,0,0,0,
0,0,0,1,255,0,0,8,2,0,167,202,24,0,108,174,251,0,7,2,252,0,2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,254,255,255,0,242,249,1,0,42,27,10,0,147,77,249,0,55,36,236,0,0,249,255,0,0,255,1,0,0,0,0,0,0,1,255,0,0,7,2,0,176,209,24,0,106,172,255,0,251,251,248,0,
7,4,1,208,0,0,0,174,0,0,0,211,0,0,0,219,0,0,0,230,0,0,0,244,0,0,0,252,228,228,228,255,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,11,0,0,0,25,0,0,0,37,0,0,0,45,0,0,0,84,254,255,255,48,244,250,3,0,119,69,9,0,142,78,234,0,0,254,246,0,0,252,2,0,0,0,0,0,0,1,255,
0,0,8,0,0,202,222,26,0,60,156,12,0,172,206,244,0,15,7,253,0,6,3,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,0,0,249,255,4,0,142,78,1,0,46,32,235,0,0,248,2,0,0,0,1,0,0,0,0,0,0,6,254,0,0,253,11,0,140,188,26,0,56,151,8,0,145,193,247,0,241,245,248,
0,15,8,255,0,10,6,1,0,2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,0,11,7,1,0,13,7,254,0,226,236,246,0,121,179,250,0,53,152,15,0,176,207,25,0,0,6,6,0,0,4,254,0,0,0,0,0,0,255,1,0,
0,247,254,0,89,54,233,0,116,66,7,0,245,250,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,0,16,8,0,0,221,231,245,0,77,159,3,0,145,192,26,0,0,5,7,0,0,4,254,0,0,0,0,0,0,255,1,0,0,248,254,0,80,48,231,0,169,92,255,0,12,10,9,0,248,252,255,48,0,0,0,84,0,
0,0,45,0,0,0,37,0,0,0,25,0,0,0,11,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,12,0,0,0,26,0,0,0,37,0,0,0,43,0,0,0,86,255,255,255,48,242,250,3,0,120,69,8,0,123,69,234,0,0,251,249,0,0,254,2,0,0,0,0,0,0,1,254,0,0,6,6,0,160,202,22,0,118,180,253,0,253,249,
249,0,9,6,1,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,0,0,0,0,0,0,0,0,0,1,1,1,0,245,249,255,0,77,48,10,0,162,90,235,0,0,249,250,0,0,0,1,0,0,0,0,0,0,7,255,0,224,231,18,0,96,175,10,0,203,222,247,0,13,6,254,0,4,3,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,248,252,255,0,251,254,4,0,88,51,7,0,153,76,242,0,0,8,241,0,0,251,2,0,0,0,0,0,0,1,254,0,0,1,11,0,70,157,18,0,205,223,
245,0,9,6,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,253,254,255,0,243,250,2,0,89,54,11,0,155,77,238,0,0,6,242,0,0,251,3,0,0,0,0,0,0,0,255,0,0,8,0,0,202,220,23,0,85,166,3,0,246,246,247,0,7,4,1,208,0,0,0,170,0,0,0,213,0,0,0,219,0,0,0,230,0,0,
0,244,0,0,0,253,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,2,0,0,0,9,0,0,0,20,0,0,0,33,0,0,0,35,0,0,0,79,255,255,255,48,242,250,2,0,118,68,9,0,123,69,234,0,0,250,249,0,0,254,2,0,0,0,0,0,0,2,254,0,0,4,8,0,126,183,23,0,111,173,250,0,3,253,249,0,9,6,1,0,0,0,0,0,250,253,255,0,243,249,0,0,241,
249,1,0,249,255,6,0,253,2,8,0,249,255,6,0,240,248,0,0,250,255,4,0,107,54,250,0,0,255,248,0,0,0,1,0,0,0,0,0,0,6,254,0,213,227,20,0,68,158,8,0,209,224,245,0,17,9,255,0,4,3,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,4,1,0,12,5,253,0,164,202,246,0,68,162,15,0,245,243,17,0,0,4,254,0,0,0,0,0,0,255,2,0,0,6,245,0,114,59,252,0,250,255,3,0,248,250,255,0,246,250,
255,0,247,250,255,0,250,253,255,0,255,255,255,0,0,0,0,0,0,0,0,0,3,2,1,0,16,8,255,0,167,201,244,0,70,161,17,0,241,241,18,0,0,6,254,0,0,0,0,0,0,0,1,0,0,248,0,0,54,36,233,0,170,89,253,0,8,9,8,0,250,253,255,48,0,0,0,79,0,0,0,35,0,0,0,33,0,0,0,20,0,0,0,9,
0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,1,0,0,0,6,0,0,0,21,0,0,0,36,0,0,0,37,0,0,0,90,0,0,0,64,240,248,1,0,102,59,9,0,142,80,235,0,0,250,248,0,0,255,2,0,0,0,0,0,0,4,254,0,0,253,11,0,105,177,19,0,163,202,246,0,15,7,254,0,1,1,1,0,246,250,255,0,254,2,4,0,35,20,5,0,62,34,1,0,54,28,252,0,
36,17,250,0,7,6,4,0,249,250,252,0,205,231,7,0,192,226,8,0,163,77,233,0,5,8,247,0,0,255,1,0,0,2,254,0,248,246,12,0,59,157,16,0,223,232,246,0,11,7,1,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,240,247,255,0,67,43,13,0,177,91,235,0,0,0,247,0,0,255,1,0,0,2,254,0,250,249,15,0,58,157,19,0,9,0,247,0,19,15,8,0,248,249,250,0,
232,242,254,0,232,242,255,0,245,248,252,0,8,4,254,0,7,4,1,0,0,0,0,0,0,0,0,0,244,249,0,0,36,25,10,0,175,89,246,0,29,24,237,0,0,249,2,0,0,0,0,0,0,0,0,0,0,8,255,0,188,213,25,0,90,167,255,0,3,254,249,0,3,2,1,192,0,0,0,166,0,0,0,219,0,0,0,220,0,0,0,235,0,
0,0,250,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
0,0,0,5,0,0,0,15,0,0,0,32,0,0,0,36,0,0,0,87,0,0,0,79,241,248,0,0,79,49,11,0,164,87,236,0,0,254,246,0,0,254,2,0,0,0,0,0,0,4,254,0,0,253,11,0,92,172,19,0,176,207,246,0,16,8,254,0,247,250,0,0,254,3,6,0,62,36,4,0,105,54,252,0,72,34,244,0,0,11,248,0,0,0,253,
0,0,254,255,0,0,255,255,0,0,1,1,0,0,2,1,0,0,250,9,0,0,254,249,0,0,0,255,0,0,0,0,0,0,7,255,0,157,200,27,0,109,171,248,0,18,10,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,239,248,2,0,157,86,3,0,88,53,233,0,0,249,1,0,0,0,0,0,0,0,2,0,241,247,13,0,15,8,247,0,0,5,0,0,0,253,254,0,254,251,7,0,215,237,6,0,185,
221,8,0,173,210,0,0,212,230,250,0,5,2,253,0,8,4,1,0,1,0,0,0,245,250,255,0,21,16,8,0,178,94,251,0,39,27,235,0,0,249,2,0,0,0,0,0,0,0,255,0,0,9,1,0,163,201,26,0,109,174,252,0,10,3,250,0,2,1,1,177,0,0,0,169,0,0,0,220,0,0,0,224,0,0,0,241,0,0,0,251,0,0,0,255,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,11,0,0,0,25,0,0,0,34,0,0,0,70,
0,0,0,76,245,249,255,0,50,32,10,0,164,80,239,0,0,0,246,0,0,254,2,0,0,0,0,0,0,2,254,0,0,253,11,0,94,172,19,0,176,207,246,0,13,6,254,0,241,248,2,0,51,32,9,0,125,66,251,0,67,39,236,0,0,2,248,0,0,251,0,0,0,248,2,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,1,247,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,14,5,252,0,3,2,1,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,232,242,253,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,254,0,15,11,239,0,0,255,1,0,0,253,0,0,0,0,0,0,2,6,250,0,43,28,0,0,43,31,2,0,114,57,9,0,176,217,14,0,142,196,8,0,207,225,247,0,11,5,254,0,11,
7,1,0,235,240,248,0,78,162,8,0,180,94,21,0,37,27,235,0,0,249,2,0,0,0,1,0,0,248,255,0,93,54,232,0,224,239,26,0,137,187,253,0,16,8,254,76,0,0,0,239,0,0,0,254,0,0,0,249,0,0,0,252,0,0,0,243,0,0,0,252,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,8,0,0,0,20,0,0,0,32,0,0,0,50,0,0,0,72,250,253,255,3,19,15,8,0,166,86,247,0,0,9,243,0,0,252,2,0,0,
0,0,0,0,2,254,0,0,3,9,0,101,176,20,0,174,207,245,0,7,2,253,0,1,4,8,0,129,75,9,0,192,100,238,0,67,41,232,0,0,252,249,0,0,249,2,0,0,254,2,0,0,0,0,0,0,0,255,0,0,2,255,0,0,4,254,0,0,1,255,0,0,0,0,0,0,255,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,253,255,0,68,33,248,
0,23,15,4,0,253,254,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,255,255,0,0,11,7,4,0,69,37,255,0,0,253,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,254,2,0,0,255,1,0,0,255,1,0,0,255,1,0,0,254,2,0,0,251,2,0,0,248,0,0,0,2,245,0,80,48,231,0,194,100,238,0,137,77,8,0,7,7,8,0,243,248,255,0,11,6,1,0,235,239,248,0,82,166,7,0,
233,236,19,0,0,6,254,0,0,0,0,0,0,255,1,0,0,250,252,0,125,68,236,0,79,48,10,0,243,248,0,3,0,0,0,72,0,0,0,50,0,0,0,32,0,0,0,20,0,0,0,8,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,5,0,0,0,16,0,0,0,28,0,0,0,32,0,0,0,70,254,255,255,24,253,1,6,0,148,80,0,0,27,22,237,0,0,250,2,0,0,0,0,0,0,1,255,0,0,5,7,0,123,185,22,0,167,203,245,0,1,0,255,0,37,
26,12,0,184,99,252,0,114,66,230,0,0,2,245,0,0,249,2,0,0,255,1,0,0,0,0,0,0,1,254,0,0,7,254,0,0,6,6,0,249,246,11,0,232,239,19,0,0,254,8,0,0,4,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,252,1,0,17,18,246,0,64,36,3,0,246,251,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,248,251,255,0,50,29,4,0,43,25,244,0,0,252,1,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,254,0,0,8,0,0,0,8,255,0,0,5,254,0,0,1,255,0,0,0,0,0,0,255,2,0,0,248,2,0,0,1,245,0,106,63,230,0,195,101,249,0,62,39,11,0,239,248,1,0,9,6,0,0,228,235,246,0,83,168,11,0,248,245,16,0,0,4,253,0,0,0,0,0,0,254,2,0,0,255,246,
0,158,79,242,0,37,26,11,0,247,252,255,24,0,0,0,70,0,0,0,32,0,0,0,28,0,0,0,16,0,0,0,5,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,12,0,0,0,23,0,0,0,24,0,0,0,69,0,0,0,48,242,249,1,0,105,60,6,0,72,44,236,0,0,248,1,0,0,0,0,0,0,0,0,0,0,7,1,0,156,198,21,0,145,191,246,0,0,255,255,0,78,48,12,0,195,100,243,0,46,30,234,0,0,247,254,0,0,254,2,0,0,0,0,0,0,
1,255,0,0,8,255,0,0,253,11,0,176,207,25,0,90,164,21,0,51,151,13,0,53,148,6,0,71,161,20,0,242,243,16,0,0,5,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,255,250,0,95,50,252,0,2,3,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,250,255,3,0,97,51,254,0,0,5,248,0,0,255,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,255,0,222,233,18,0,144,
190,24,0,161,199,21,0,219,231,20,0,0,3,8,0,0,7,254,0,0,1,255,0,0,0,0,0,0,255,2,0,0,247,0,0,30,24,237,0,178,94,238,0,117,67,8,0,246,253,4,0,8,4,0,0,211,225,245,0,89,171,14,0,0,254,11,0,0,2,254,0,0,0,0,0,0,252,2,0,7,12,239,0,149,78,250,0,2,3,6,0,254,254,
255,48,0,0,0,69,0,0,0,24,0,0,0,23,0,0,0,12,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,10,0,0,
0,26,0,0,0,43,0,0,0,65,0,0,0,101,247,250,255,8,44,30,11,0,193,99,239,0,0,3,244,0,0,254,2,0,0,0,0,0,0,7,254,0,197,219,22,0,79,160,0,0,245,248,254,0,108,64,10,0,139,74,236,0,0,0,246,0,0,252,2,0,0,0,0,0,0,0,0,0,0,5,254,0,0,253,9,0,169,210,17,0,143,195,1,
0,215,230,250,0,1,0,253,0,8,4,0,0,2,1,0,0,242,248,0,0,65,41,11,0,179,93,238,0,0,2,245,0,0,254,2,0,0,0,0,0,0,0,0,0,0,1,255,0,0,1,9,0,68,156,19,0,207,224,245,0,9,6,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,249,252,254,0,32,22,12,0,203,104,240,0,0,3,243,0,0,255,2,0,0,0,0,0,0,0,0,0,0,7,254,0,213,227,22,0,78,162,7,0,230,238,246,0,4,2,
1,0,12,9,5,0,53,31,4,0,106,54,251,0,72,40,241,0,0,4,247,0,0,250,1,0,0,0,1,0,0,0,0,0,0,2,254,0,0,6,4,0,167,203,23,0,114,178,254,0,253,251,248,0,246,252,2,0,110,64,11,0,134,75,232,0,0,249,251,0,0,0,1,0,0,0,0,0,0,8,0,0,134,186,27,0,134,186,247,0,16,8,255,
248,0,0,0,155,0,0,0,191,0,0,0,213,0,0,0,230,0,0,0,246,0,0,0,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,4,0,0,0,15,0,0,0,27,
0,0,0,29,0,0,0,65,254,254,255,8,5,6,7,0,146,75,249,0,0,8,244,0,0,254,2,0,0,0,0,0,0,5,254,0,240,239,17,0,102,173,5,0,243,247,254,0,121,71,10,0,139,76,234,0,0,254,246,0,0,252,2,0,0,0,0,0,0,0,255,0,0,8,255,0,223,233,20,0,78,162,20,0,100,169,251,0,220,233,
247,0,13,7,254,0,12,7,1,0,4,3,1,0,2,2,1,0,16,10,1,0,191,216,247,0,123,188,18,0,0,5,4,0,0,0,255,0,0,0,0,0,0,0,0,0,0,255,1,0,0,6,246,0,103,55,253,0,250,255,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,248,253,3,0,106,57,255,0,0,10,244,0,0,255,2,0,0,0,0,0,0,0,0,0,0,5,254,0,239,239,18,0,88,169,7,0,239,242,245,0,19,10,0,0,15,8,255,
0,1,254,249,0,195,217,245,0,87,164,254,0,77,162,20,0,209,225,22,0,0,8,2,0,0,1,254,0,0,0,0,0,0,254,2,0,0,248,253,0,89,54,231,0,167,90,1,0,9,10,10,0,8,2,254,0,149,194,248,0,171,205,20,0,0,8,0,0,0,0,0,0,0,0,0,0,0,250,254,0,122,63,238,0,54,33,8,0,246,250,
255,8,0,0,0,65,0,0,0,29,0,0,0,27,0,0,0,15,0,0,0,4,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,9,0,0,0,19,0,0,0,24,0,0,0,62,0,0,
0,44,242,250,1,0,102,58,3,0,47,33,238,0,0,249,1,0,0,0,0,0,0,1,255,0,0,0,9,0,104,179,13,0,214,229,248,0,115,66,9,0,126,70,232,0,0,252,248,0,0,254,2,0,0,0,0,0,0,2,254,0,0,7,3,0,168,203,26,0,63,155,6,0,194,217,245,0,15,7,254,0,8,4,1,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,15,8,254,0,150,197,251,0,189,215,21,0,0,8,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,249,1,0,85,42,241,0,43,27,7,0,248,252,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,249,252,255,0,39,24,9,0,97,47,241,0,0,250,255,0,0,0,0,0,0,0,0,0,0,1,254,0,0,253,11,0,98,176,13,0,226,233,245,0,11,6,1,0,0,0,0,0,0,0,0,0,2,1,1,0,11,7,1,0,13,6,253,
0,196,217,245,0,69,157,3,0,136,187,25,0,0,1,9,0,0,4,254,0,0,0,0,0,0,0,1,0,0,247,255,0,64,41,231,0,171,90,253,0,11,11,10,0,2,254,251,0,119,183,0,0,228,232,19,0,0,5,254,0,0,0,0,0,0,254,2,0,0,5,245,0,140,73,250,0,4,6,7,0,254,255,255,44,0,0,0,62,0,0,0,24,
0,0,0,19,0,0,0,9,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,6,0,0,0,15,0,0,0,26,0,0,0,40,0,0,0,63,249,252,255,0,40,26,10,0,128,62,243,
0,0,250,252,0,0,0,1,0,0,0,0,0,0,7,0,0,157,202,19,0,169,203,247,0,98,59,10,0,134,74,236,0,0,251,251,0,0,254,2,0,0,0,0,0,0,2,254,0,0,3,10,0,125,184,27,0,103,171,251,0,250,248,248,0,12,7,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,
0,3,255,250,0,118,183,4,0,251,244,16,0,0,2,254,0,0,0,0,0,0,0,0,0,0,255,1,0,0,9,246,0,107,57,254,0,249,253,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,248,253,3,0,109,59,254,0,0,10,244,0,0,254,2,0,0,0,0,0,0,0,255,0,0,7,3,0,138,194,18,0,186,214,245,0,13,8,1,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,7,1,0,3,254,249,0,131,184,248,
0,84,165,19,0,247,244,15,0,0,6,254,0,0,0,0,0,0,0,1,0,0,248,1,0,54,36,233,0,161,86,254,0,7,8,8,0,234,239,247,0,108,181,9,0,0,255,9,0,0,1,255,0,0,0,0,0,0,250,1,0,50,34,238,0,95,53,4,0,243,249,1,0,0,0,0,63,0,0,0,40,0,0,0,26,0,0,0,15,0,0,0,6,0,0,0,1,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,9,0,0,0,20,0,0,0,17,0,0,0,60,0,0,0,20,250,253,3,0,122,64,253,0,2,12,242,0,0,254,2,0,0,0,0,0,0,5,254,0,229,234,
19,0,121,181,254,0,59,38,7,0,157,79,240,0,0,255,247,0,0,254,1,0,0,0,0,0,0,2,254,0,0,2,10,0,103,172,20,0,142,189,245,0,12,5,252,0,4,3,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,4,1,0,217,231,246,0,121,189,14,0,0,6,4,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,250,255,0,93,45,241,0,42,26,7,0,248,252,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,247,250,255,0,48,29,7,0,95,48,240,0,0,249,0,0,0,0,0,0,0,0,0,0,0,6,255,0,206,222,21,0,140,191,252,0,14,6,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,0,17,9,255,0,185,213,246,0,64,160,15,0,230,236,20,0,0,7,254,0,
0,0,0,0,0,0,0,0,0,248,0,0,64,42,233,0,137,77,2,0,4,4,4,0,187,214,247,0,154,200,19,0,0,7,1,0,0,0,0,0,0,0,1,0,0,252,252,0,128,63,244,0,29,20,9,0,250,253,255,20,0,0,0,60,0,0,0,17,0,0,0,20,0,0,0,9,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,5,0,0,0,15,0,0,0,25,0,0,0,50,0,0,0,59,246,249,255,0,60,39,9,0,82,46,240,0,0,249,255,0,0,0,0,0,0,0,255,0,0,1,8,0,114,183,10,0,251,253,0,0,161,83,249,0,0,8,242,0,0,252,
2,0,0,0,0,0,0,2,254,0,0,0,10,0,101,173,19,0,165,202,245,0,17,8,255,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,7,255,0,160,201,251,0,201,220,20,0,0,6,254,0,0,0,0,0,0,0,0,0,0,254,2,0,0,9,246,0,112,
59,253,0,249,253,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,253,0,4,0,121,64,253,0,0,7,245,0,0,255,1,0,0,
0,0,0,0,2,254,0,0,249,12,0,111,181,4,0,253,251,249,0,3,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,16,8,1,0,218,229,244,0,70,160,8,0,217,229,21,0,0,7,254,0,0,0,0,0,0,0,1,0,0,248,255,0,95,55,235,0,101,59,8,0,254,
254,254,0,136,191,255,0,233,235,16,0,0,4,254,0,0,0,0,0,0,252,2,0,11,16,241,0,109,59,0,0,246,251,2,0,0,0,0,59,0,0,0,50,0,0,0,25,0,0,0,15,0,0,0,5,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,9,0,0,0,19,0,0,0,14,0,0,0,57,255,255,255,7,0,4,6,0,120,61,250,0,0,3,246,0,0,255,2,0,0,0,0,0,0,7,255,0,182,211,16,0,170,207,250,0,132,72,1,0,35,28,237,0,0,250,3,0,0,0,0,0,0,1,255,0,0,4,7,0,114,182,19,0,165,202,246,0,16,9,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,0,254,252,249,0,120,185,5,0,0,251,11,0,0,1,254,0,0,0,0,0,0,0,0,0,0,250,255,0,89,46,242,0,48,30,8,0,247,250,255,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,244,249,255,0,67,41,7,0,74,43,239,0,0,250,1,0,0,0,0,0,0,0,255,0,0,6,3,0,135,195,16,0,206,224,247,0,9,6,1,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,12,7,1,0,230,235,246,0,82,163,5,0,231,235,19,0,0,6,254,0,0,0,0,0,0,255,1,0,0,250,250,0,139,70,239,0,55,36,11,0,226,236,246,0,124,190,11,0,0,3,6,0,0,0,255,
0,0,0,0,0,0,250,254,0,105,51,241,0,41,26,8,0,249,252,255,7,0,0,0,57,0,0,0,14,0,0,0,19,0,0,0,9,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,0,0,0,15,0,0,0,24,0,0,0,50,
0,0,0,55,246,250,0,0,65,38,5,0,61,36,239,0,0,249,1,0,0,0,0,0,0,1,254,0,0,248,12,0,124,187,3,0,55,32,5,0,107,56,240,0,0,248,254,0,0,0,0,0,0,0,255,0,0,7,1,0,153,198,21,0,155,197,249,0,17,8,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,6,1,0,203,223,247,0,151,202,17,0,0,7,1,0,0,0,0,0,0,0,0,0,0,255,2,0,0,7,245,0,118,62,252,0,253,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,254,255,255,0,8,7,6,0,130,67,247,0,0,0,248,0,0,255,1,0,0,0,0,0,0,6,255,0,217,227,17,0,146,195,253,0,14,7,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,11,7,1,0,230,238,248,0,84,169,11,0,251,245,15,0,0,4,254,0,0,0,0,0,0,254,2,0,0,8,244,0,141,73,249,0,17,12,7,0,172,207,249,0,200,220,16,0,0,6,255,0,0,0,0,0,0,254,2,0,0,10,244,0,109,58,254,0,248,252,2,0,0,
0,0,55,0,0,0,50,0,0,0,24,0,0,0,15,0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,8,0,0,0,18,0,0,0,14,0,0,0,53,0,255,255,4,1,4,6,0,111,58,251,0,0,2,249,0,0,0,1,0,0,0,0,0,
0,7,1,0,157,205,15,0,207,228,254,0,129,67,252,0,0,6,245,0,0,255,2,0,0,0,0,0,0,6,255,0,211,225,19,0,128,185,254,0,12,5,252,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,12,5,254,0,142,193,255,0,238,237,17,0,0,4,254,0,0,0,0,0,0,0,0,0,0,250,1,0,77,43,240,0,58,37,7,0,246,249,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,242,249,1,0,94,53,3,0,43,30,239,0,0,251,2,0,0,0,0,0,0,1,255,0,0,254,9,0,119,186,9,0,247,248,249,0,3,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,
11,7,1,0,210,225,245,0,97,177,14,0,0,0,8,0,0,1,255,0,0,0,0,0,0,249,1,0,51,34,238,0,92,52,5,0,242,245,252,0,134,193,7,0,0,254,8,0,0,1,255,0,0,0,0,0,0,250,0,0,94,45,243,0,38,25,8,0,249,252,255,4,0,0,0,53,0,0,0,14,0,0,0,18,0,0,0,8,0,0,0,2,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,0,0,0,13,0,0,0,21,0,0,0,39,0,0,0,51,247,251,0,0,53,30,3,0,62,33,241,0,0,250,1,0,0,0,0,0,0,2,254,0,255,244,12,0,138,193,254,0,61,35,3,0,71,39,241,0,0,250,1,0,0,
0,0,0,0,1,254,0,0,251,10,0,115,182,8,0,248,248,248,0,3,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,3,1,0,232,240,248,0,128,193,12,0,0,3,6,0,0,0,255,0,
0,0,0,0,0,255,1,0,0,2,247,0,123,63,251,0,0,4,6,0,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,252,253,255,0,28,18,7,0,126,62,242,0,0,252,253,0,0,
0,0,0,0,0,0,0,0,7,0,0,168,206,16,0,189,216,248,0,12,7,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,8,0,0,176,209,248,0,157,201,21,0,0,7,1,0,0,0,0,0,0,0,
1,0,0,253,252,0,116,57,245,0,26,20,7,0,191,217,249,0,189,217,16,0,0,5,255,0,0,0,0,0,0,254,1,0,0,11,244,0,97,51,254,0,247,252,2,0,0,0,0,51,0,0,0,39,0,0,0,21,0,0,0,13,0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,1,0,0,0,7,0,0,0,16,0,0,0,18,0,0,0,49,0,0,0,12,250,255,3,0,99,53,254,0,0,5,247,0,0,255,1,0,0,0,0,0,0,6,1,0,155,207,13,0,218,232,253,0,103,54,254,0,0,6,247,0,0,255,1,0,0,0,0,0,0,7,1,0,166,208,15,0,198,221,248,0,11,7,1,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,7,0,0,166,204,250,0,206,222,15,0,0,6,255,0,0,0,0,0,0,0,0,0,0,250,1,0,59,36,239,0,74,43,5,0,245,249,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,244,251,2,0,118,62,255,0,12,17,244,0,0,252,2,0,0,0,0,0,0,4,255,0,249,241,11,0,132,190,1,0,8,4,253,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,4,253,0,133,190,254,0,235,236,15,0,0,4,254,0,0,0,0,0,0,251,2,0,27,24,242,0,81,44,2,0,246,250,252,0,146,199,5,0,0,254,
8,0,0,0,255,0,0,0,0,0,0,250,255,0,93,46,246,0,24,16,6,0,253,254,255,12,0,0,0,49,0,0,0,18,0,0,0,16,0,0,0,7,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,9,0,0,0,18,0,0,0,17,0,0,0,48,252,253,255,
0,33,21,6,0,83,39,244,0,0,251,0,0,0,0,0,0,0,1,254,0,0,248,11,0,156,203,2,0,33,22,4,0,82,39,245,0,0,250,255,0,0,0,0,0,0,1,255,0,0,247,9,0,147,199,2,0,8,4,253,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,253,251,250,0,129,191,8,0,0,255,9,0,0,0,255,0,0,0,0,0,0,0,1,0,0,255,249,0,128,64,247,0,10,9,7,0,254,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,246,250,255,0,56,33,7,0,98,53,240,0,0,249,0,0,0,0,0,0,0,0,0,0,0,4,2,0,132,194,14,0,224,236,249,0,7,3,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,3,1,0,235,241,248,0,137,196,11,0,0,3,3,0,0,0,0,0,0,0,0,0,0,254,252,0,97,49,250,0,17,11,4,0,195,221,249,0,202,224,14,0,0,6,255,0,0,0,0,0,0,254,2,0,15,18,244,0,74,42,2,0,246,250,0,0,0,0,0,
48,0,0,0,17,0,0,0,18,0,0,0,9,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,12,0,0,0,18,0,0,0,45,0,0,0,30,245,251,1,0,78,42,2,0,3,13,246,0,0,254,2,0,0,0,0,0,0,6,0,0,182,218,14,0,205,225,249,0,74,
40,2,0,9,15,245,0,0,254,1,0,0,0,0,0,0,6,255,0,185,220,14,0,214,230,250,0,8,4,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,
7,1,0,192,218,248,0,178,211,16,0,0,7,255,0,0,0,0,0,0,0,0,0,0,251,1,0,36,28,240,0,93,53,4,0,243,249,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,0,1,3,4,0,132,69,250,0,
0,6,247,0,0,255,1,0,0,0,0,0,0,6,254,0,218,226,14,0,157,202,254,0,14,7,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,6,255,
0,170,209,253,0,232,235,14,0,0,4,254,0,0,0,0,0,0,251,1,0,49,28,245,0,48,28,6,0,245,247,252,0,155,204,7,0,0,1,6,0,0,0,0,0,0,0,0,0,0,254,252,0,93,46,249,0,5,6,6,0,255,255,255,30,0,0,0,45,0,0,0,18,0,0,0,12,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,7,0,0,0,14,0,0,0,8,0,0,0,43,255,255,255,0,7,6,5,0,86,43,247,0,0,254,253,0,0,0,0,0,0,0,255,0,0,255,7,0,162,207,3,0,1,2,0,0,75,40,253,0,0,0,252,0,0,0,1,0,0,0,0,0,0,255,6,0,167,211,4,0,1,255,253,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,0,253,0,137,194,4,0,0,248,13,0,0,1,254,0,0,0,0,0,0,0,1,0,0,252,252,0,123,
61,243,0,26,18,8,0,252,253,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,244,249,1,0,85,48,6,0,56,37,241,0,0,249,1,0,0,0,0,0,0,1,255,0,0,0,7,0,126,190,10,0,246,248,250,0,2,1,1,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,0,238,243,251,0,166,212,10,0,0,4,1,0,0,0,0,0,0,255,1,0,0,6,248,0,75,40,255,0,3,2,1,0,
186,217,253,0,233,236,11,0,0,4,254,0,0,0,0,0,0,251,1,0,53,29,245,0,43,26,4,0,249,253,0,0,0,0,0,43,0,0,0,8,0,0,0,14,0,0,0,7,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,9,0,0,0,16,0,0,0,23,0,0,0,43,249,253,0,
0,45,27,4,0,46,26,246,0,0,251,1,0,0,0,0,0,0,4,255,0,226,232,11,0,192,219,251,0,31,18,4,0,58,27,247,0,0,251,0,0,0,0,0,0,0,2,254,0,244,241,10,0,198,223,255,0,9,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,6,1,0,209,226,248,0,152,204,15,0,0,7,1,0,0,0,0,0,0,0,0,0,0,252,2,0,14,17,241,0,112,60,0,0,245,251,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,253,253,255,0,20,15,7,0,132,66,245,0,0,253,250,0,0,0,1,0,0,0,0,0,0,6,255,0,183,212,15,0,184,214,250,0,12,7,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,4,254,0,178,214,1,0,0,249,9,0,0,1,255,0,0,0,0,0,0,253,254,0,69,35,250,0,12,9,5,0,236,242,249,0,171,214,10,0,0,4,1,0,0,0,0,0,0,255,1,0,0,7,248,0,75,39,255,0,
249,252,1,0,0,0,0,43,0,0,0,23,0,0,0,16,0,0,0,9,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,11,0,0,0,17,0,0,0,40,0,0,0,18,249,252,1,0,70,37,255,0,0,7,248,0,0,255,1,0,0,0,0,0,0,4,2,0,173,216,8,0,235,243,251,0,
44,25,2,0,15,14,248,0,0,254,1,0,0,0,0,0,0,5,0,0,192,226,9,0,233,241,250,0,3,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,10,5,253,0,147,196,255,0,247,241,15,0,0,2,254,0,0,0,0,0,0,0,0,0,0,249,255,0,105,54,240,0,49,31,9,0,247,250,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,243,251,3,0,115,63,1,0,20,
21,241,0,0,252,2,0,0,0,0,0,0,2,255,0,0,247,11,0,129,189,5,0,4,2,253,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,7,3,0,0,216,232,253,0,217,233,10,0,0,4,255,0,0,0,0,0,0,252,1,0,44,20,246,0,30,17,3,0,4,0,254,0,176,213,1,0,0,250,9,0,0,1,255,0,0,0,0,0,0,253,254,0,76,38,249,0,9,7,5,0,255,255,0,18,0,0,0,40,0,0,0,17,0,0,0,11,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,6,0,0,0,12,0,0,0,7,0,0,0,39,255,0,0,0,7,6,5,0,71,35,249,0,0,253,254,0,0,0,0,0,0,1,255,0,0,250,7,0,182,216,2,0,3,1,255,0,50,27,1,0,0,4,250,0,0,0,1,0,0,0,0,0,0,1,4,0,194,225,3,0,0,254,254,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,3,1,0,226,237,249,0,142,200,12,0,0,6,3,0,0,0,0,0,0,0,0,0,0,255,
2,0,0,7,245,0,127,67,252,0,254,2,6,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,246,250,255,0,54,34,9,0,106,55,240,0,0,249,254,0,0,0,0,0,0,0,0,0,0,6,0,0,145,201,14,0,212,229,249,0,8,4,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,245,248,251,0,192,225,6,0,0,3,1,0,0,0,0,0,0,255,1,0,0,10,251,0,43,25,1,0,1,
1,1,0,212,230,253,0,217,233,9,0,0,4,255,0,0,0,0,0,0,252,1,0,42,23,246,0,39,23,4,0,250,253,255,0,0,0,0,39,0,0,0,7,0,0,0,12,0,0,0,6,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,8,0,0,0,13,0,0,0,8,0,0,0,37,251,253,255,0,32,19,
3,0,50,24,248,0,0,252,0,0,0,0,0,0,0,4,255,0,220,234,10,0,212,231,253,0,7,5,3,0,46,22,249,0,0,255,254,0,0,0,0,0,0,1,255,0,0,249,4,0,207,228,0,0,5,4,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,6,254,0,156,201,255,0,235,236,15,0,0,4,254,0,0,0,0,0,0,0,0,0,0,249,1,0,67,40,238,0,75,44,5,0,244,249,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,0,1,4,6,0,135,69,250,0,0,6,245,0,0,255,2,0,0,0,0,0,0,4,254,0,238,237,16,0,147,197,1,0,12,6,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,1,254,0,201,227,2,0,0,255,5,0,0,0,0,0,0,0,0,0,0,1,252,0,46,23,254,0,253,0,1,0,245,248,250,0,181,219,8,0,0,3,1,0,0,0,0,0,0,255,1,0,0,8,249,0,62,33,0,
0,250,252,1,0,0,0,0,37,0,0,0,8,0,0,0,13,0,0,0,8,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,7,0,0,0,13,0,0,0,34,0,0,0,27,250,252,1,0,52,28,2,0,0,10,248,0,0,255,2,0,0,0,0,0,0,3,1,0,189,223,8,0,243,247,251,0,9,6,1,0,37,20,0,
0,0,254,0,0,0,0,0,0,0,1,255,0,242,246,9,0,225,239,254,0,5,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,3,2,1,0,236,242,248,0,136,195,10,0,0,4,4,0,0,0,0,0,0,0,0,0,0,0,1,0,0,255,249,0,134,67,247,0,14,12,8,0,253,254,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,242,249,1,0,94,54,6,0,55,36,236,0,0,249,1,0,0,0,0,0,0,0,
0,0,0,4,4,0,133,195,13,0,233,240,249,0,4,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,5,3,0,0,215,233,254,0,0,247,4,0,0,1,255,0,0,0,0,0,0,254,254,0,42,22,252,0,3,4,3,0,5,1,255,0,191,221,1,0,0,252,7,0,0,0,255,0,0,0,0,0,0,255,253,0,63,32,252,0,1,2,3,0,0,0,0,27,0,0,0,34,0,0,0,13,0,0,0,7,0,0,0,3,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,4,0,0,0,8,0,0,0,12,0,0,0,32,0,0,0,0,253,0,2,0,59,31,253,0,0,1,253,0,0,0,0,0,0,0,0,0,0,254,6,0,192,222,1,0,3,0,254,0,12,7,2,0,26,7,245,0,0,255,1,0,0,0,0,0,0,3,0,0,225,239,254,0,239,246,254,0,2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,13,7,255,0,168,206,253,0,222,229,15,0,0,5,254,0,0,0,0,0,0,0,0,0,0,251,
2,0,30,25,239,0,105,57,3,0,243,250,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,249,252,255,0,31,20,9,0,133,64,242,0,0,253,252,0,0,0,1,0,0,0,0,0,0,5,254,0,215,227,17,0,165,204,251,0,13,7,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,2,0,0,234,245,1,0,237,247,9,0,0,1,254,0,0,0,0,0,0,255,0,0,30,15,255,0,6,2,0,0,7,5,
0,0,212,231,253,0,236,239,9,0,0,4,255,0,0,0,0,0,0,251,0,0,51,25,249,0,19,13,4,0,254,254,255,0,0,0,0,32,0,0,0,12,0,0,0,8,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,8,0,0,0,4,0,0,0,31,255,255,0,0,11,6,3,0,52,26,250,0,0,253,
255,0,0,0,0,0,0,2,254,0,250,243,8,0,211,231,255,0,3,1,255,0,17,12,6,0,16,13,7,0,0,253,255,0,0,0,0,0,0,1,0,0,228,245,12,0,251,251,253,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,0,244,247,250,0,137,196,10,0,0,2,4,0,0,0,255,0,0,0,0,0,0,0,0,0,0,250,253,0,122,59,240,0,41,25,7,0,248,252,255,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,250,254,3,0,127,69,255,0,10,16,241,0,0,252,2,0,0,0,0,0,0,0,255,0,0,1,7,0,131,191,8,0,248,249,249,0,2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,0,242,246,251,0,230,240,250,0,0,3,2,0,0,0,0,0,0,255,1,0,20,6,245,0,9,7,2,0,1,0,0,0,239,244,252,0,203,231,8,0,0,3,0,0,0,0,0,0,0,255,1,0,19,14,248,0,35,20,2,0,
250,254,0,0,0,0,0,31,0,0,0,4,0,0,0,8,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,5,0,0,0,9,0,0,0,0,0,0,0,29,253,254,255,0,23,15,3,0,38,17,248,0,0,253,0,0,0,0,0,0,0,4,0,0,213,236,4,0,232,242,253,0,2,3,0,0,9,4,253,0,2,3,252,0,0,0,0,
0,0,0,0,0,0,1,0,0,239,249,3,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,12,7,255,0,175,211,252,0,212,226,17,0,0,6,255,0,0,0,0,0,0,0,0,0,0,254,2,0,0,10,243,0,127,69,255,0,252,254,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,244,249,255,0,72,43,8,0,85,49,237,0,0,249,0,0,0,0,0,0,0,0,0,0,0,7,255,0,195,219,15,0,183,
214,249,0,11,6,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,
252,254,255,0,237,250,12,0,0,1,255,0,0,0,0,0,0,254,255,0,14,10,7,0,13,9,6,0,254,255,0,0,253,254,254,0,200,228,2,0,0,1,3,0,0,0,0,0,0,255,1,0,0,5,251,0,45,24,0,0,251,252,0,0,0,0,0,29,0,0,0,0,0,0,0,9,0,0,0,5,0,0,0,1,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,
0,2,0,0,0,6,0,0,0,11,0,0,0,21,0,0,0,27,250,253,0,0,34,18,0,0,3,9,251,0,0,254,1,0,0,0,0,0,0,1,1,0,207,232,7,0,249,250,252,0,1,0,1,0,1,0,255,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,242,245,247,0,254,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,250,248,249,0,141,198,9,0,0,1,7,0,0,0,255,0,0,0,0,0,0,0,0,0,
0,249,1,0,76,43,237,0,73,43,6,0,244,249,255,0,0,0,0,0,0,0,0,0,253,254,255,0,14,12,8,0,140,70,246,0,0,255,247,0,0,255,1,0,0,0,0,0,0,1,255,0,0,253,9,0,135,192,7,0,1,254,252,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,244,253,5,0,0,0,0,0,0,0,0,0,0,255,0,0,6,4,0,0,6,3,0,0,254,0,0,0,5,2,255,0,208,230,
1,0,0,252,5,0,0,0,255,0,0,0,0,0,0,0,253,0,45,24,254,0,254,1,3,0,0,0,0,27,0,0,0,21,0,0,0,11,0,0,0,6,0,0,0,2,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,6,0,0,0,9,0,0,0,23,0,0,0,5,253,254,0,0,38,22,2,0,0,4,252,0,0,0,1,0,0,0,0,0,0,1,3,0,211,234,1,0,
2,0,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,7,0,0,180,212,251,0,205,222,15,0,0,6,255,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,249,0,137,70,247,0,13,12,8,0,253,254,255,0,0,0,0,0,243,250,2,0,117,65,3,0,29,26,239,0,0,251,3,0,0,0,0,0,0,0,0,0,
0,7,255,0,176,210,17,0,197,221,249,0,10,6,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,3,0,0,224,239,0,0,251,247,5,0,0,2,255,0,0,0,0,0,0,255,255,0,41,19,250,0,5,4,0,0,0,0,0,5,0,0,0,23,0,0,0,9,0,0,0,6,0,0,0,2,0,0,0,0,0,0,0,0,2,0,0,0,0,
0,0,0,0,0,0,0,3,0,0,0,5,0,0,0,8,0,0,0,22,0,0,0,0,254,1,3,0,39,18,249,0,0,0,254,0,0,0,0,0,0,0,255,0,0,250,3,0,217,233,255,0,4,4,0,0,0,0,0,0,255,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,16,12,8,0,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,251,250,249,0,140,196,6,0,0,0,7,0,0,0,255,0,0,0,0,0,0,
0,0,0,0,251,2,0,30,26,239,0,110,59,2,0,244,250,2,0,246,250,255,0,57,35,9,0,109,55,237,0,0,248,254,0,0,0,0,0,0,0,0,0,0,1,255,0,0,247,11,0,139,194,1,0,5,2,252,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,7,2,253,0,0,0,0,0,0,0,0,0,0,0,0,0,254,255,0,0,254,255,0,0,0,0,0,0,4,2,1,0,235,241,250,
0,226,241,4,0,0,3,0,0,0,0,0,0,0,254,255,0,31,17,254,0,15,10,6,0,254,254,255,0,0,0,0,22,0,0,0,8,0,0,0,5,0,0,0,3,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,5,0,0,0,7,0,0,0,20,0,0,0,0,4,3,0,0,33,19,5,0,0,254,255,0,0,0,0,0,0,1,255,0,255,251,9,0,232,
244,2,0,3,1,0,0,1,1,0,0,243,250,3,0,251,251,2,0,0,0,0,0,0,0,0,0,0,255,0,0,27,10,248,0,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,6,1,0,186,217,252,0,205,223,14,0,0,6,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,249,254,0,117,59,241,0,44,28,8,0,251,0,6,0,141,72,249,0,0,5,246,0,0,255,2,0,0,0,0,0,0,
0,0,0,0,7,0,0,160,208,15,0,209,228,250,0,8,4,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,3,2,1,0,23,6,244,0,0,0,0,0,0,0,0,0,0,3,0,0,243,247,250,0,244,247,251,0,3,1,0,0,1,0,0,0,247,253,2,0,224,241,7,0,0,1,255,0,0,0,0,0,0,254,1,0,23,9,249,0,14,5,251,0,254,255,0,0,0,0,0,20,0,0,0,7,0,0,0,5,0,0,
0,2,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,4,0,0,0,0,0,0,0,18,255,255,0,0,7,3,1,0,26,8,242,0,0,255,1,0,0,0,0,0,0,3,0,0,233,242,251,0,237,245,253,0,4,2,0,0,4,2,1,0,237,242,250,0,233,243,1,0,0,4,0,0,0,0,0,0,0,254,0,0,44,22,250,0,16,10,5,0,253,
254,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
1,1,0,0,254,251,250,0,137,194,7,0,0,255,8,0,0,1,255,0,0,0,0,0,0,0,0,0,0,254,2,0,0,8,243,0,126,66,253,0,111,64,7,0,46,32,237,0,0,250,1,0,0,0,0,0,0,0,0,0,0,2,254,0,255,244,12,0,146,197,2,0,9,5,253,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,254,254,255,0,17,10,5,0,32,18,2,0,0,252,0,0,0,0,0,0,0,1,
0,0,233,247,8,0,242,247,253,0,2,1,1,0,1,1,0,0,251,251,252,0,223,236,251,0,0,2,3,0,0,0,0,0,0,255,1,0,1,5,253,0,24,16,7,0,252,253,0,0,0,0,0,18,0,0,0,0,0,0,0,4,0,0,0,2,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,4,0,0,0,0,0,0,0,16,254,255,255,0,12,10,
7,0,20,14,6,0,0,253,255,0,0,0,0,0,0,1,0,0,234,248,10,0,246,249,252,0,1,0,1,0,2,1,0,0,244,248,254,0,210,236,9,0,0,2,0,0,0,0,0,0,0,252,1,0,17,17,246,0,68,39,3,0,235,245,1,0,249,252,255,0,254,255,255,0,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,6,1,0,189,218,252,0,198,222,16,0,0,5,255,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,249,1,0,73,39,235,0,132,67,234,0,0,250,254,0,0,0,1,0,0,0,0,0,0,0,0,0,0,7,2,0,149,205,15,0,218,232,248,0,7,3,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,249,251,0,0,40,23,1,0,15,14,247,0,0,254,1,0,0,0,0,0,0,2,0,0,218,238,5,0,250,252,0,0,1,1,0,0,0,0,0,0,3,0,255,0,232,250,15,0,0,0,1,0,0,
0,0,0,0,255,0,0,0,5,1,0,20,12,2,0,253,255,255,0,0,0,0,16,0,0,0,0,0,0,0,4,0,0,0,2,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,3,0,0,0,0,0,0,0,13,255,255,0,0,9,3,254,0,12,6,254,0,0,0,0,0,0,0,0,0,0,1,255,0,238,250,7,0,254,255,252,0,0,1,254,0,0,0,0,0,
0,255,252,0,187,220,6,0,74,46,3,0,0,0,255,0,0,0,1,0,0,252,250,0,144,70,240,0,9,4,16,0,199,225,255,0,232,241,252,0,246,251,1,0,250,249,251,0,0,0,255,0,2,1,1,0,1,0,0,0,1,0,254,0,1,0,0,0,1,1,0,0,1,0,0,0,1,3,1,0,1,1,0,0,0,0,0,0,2,0,0,0,0,0,255,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,254,0,1,0,0,0,0,0,0,0,0,0,254,254,255,0,0,254,0,0,0,0,0,0,254,0,255,0,245,249,249,0,133,192,11,0,181,31,8,0,0,1,249,0,0,255,1,0,0,0,0,0,0,0,1,0,0,255,253,0,0,1,253,0,0,255,1,0,0,0,0,0,0,
0,0,0,0,2,254,0,242,237,16,0,147,197,0,0,7,2,253,0,253,254,1,0,1,0,0,0,1,1,1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,0,251,254,0,0,246,254,4,0,94,50,255,0,0,3,249,0,0,252,1,0,0,0,0,0,0,1,3,0,200,226,4,0,255,254,247,0,0,0,0,0,0,0,0,0,1,1,0,0,233,239,247,0,0,254,254,0,0,0,1,0,0,0,1,0,0,1,250,0,14,1,242,0,254,0,2,
0,0,0,0,13,0,0,0,0,0,0,0,3,0,0,0,2,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,3,0,0,0,0,0,0,0,12,254,0,1,0,6,255,247,0,7,1,245,0,0,0,2,0,0,0,0,0,0,1,1,0,235,240,245,0,252,252,5,0,1,7,0,0,0,0,0,0,9,5,0,0,182,215,255,0,131,239,13,0,12,13,240,0,0,252,
2,0,0,254,2,0,11,11,244,0,146,82,247,0,0,247,7,0,228,241,7,0,225,242,3,0,227,239,254,0,235,251,15,0,233,240,249,0,236,241,246,0,247,0,11,0,245,252,6,0,247,252,0,0,250,252,254,0,246,247,250,0,245,247,248,0,2,1,0,0,251,0,5,0,254,0,4,0,0,0,255,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,2,0,254,0,5,0,250,0,0,0,0,0,0,255,255,0,13,14,11,0,6,4,3,0,2,1,0,0,11,6,3,0,0,0,253,0,9,5,0,0,235,242,10,0,21,14,248,0,0,255,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,2,0,168,213,
9,0,247,249,252,0,21,16,10,0,250,254,1,0,251,253,255,0,247,248,250,0,254,255,255,0,255,0,1,0,0,254,0,0,255,0,0,0,255,0,0,0,0,255,0,0,253,255,255,0,0,0,0,0,1,0,255,0,0,0,0,0,0,0,0,0,2,0,255,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,255,0,0,0,254,0,1,0,0,0,0,0,255,0,1,0,1,1,1,0,3,1,1,0,1,0,254,0,9,7,7,0,16,10,0,0,100,47,4,0,85,45,234,0,0,250,0,0,0,0,0,0,0,1,254,0,0,248,11,0,191,221,255,0,6,5,255,0,0,0,1,0,0,0,0,0,1,1,0,0,236,244,251,0,0,0,3,0,0,0,253,0,0,0,1,0,0,0,254,0,20,14,
8,0,0,0,251,0,0,0,255,12,0,0,0,0,0,0,0,3,0,0,0,1,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,2,0,0,0,0,0,0,0,9,255,255,0,0,10,8,4,0,0,2,254,0,0,0,0,0,0,0,0,0,0,1,1,0,242,246,252,0,253,255,3,0,0,0,0,0,0,0,0,0,3,2,1,0,232,240,248,0,133,195,13,0,135,
79,244,0,0,252,253,0,0,0,1,0,0,254,1,0,0,249,254,0,0,3,245,0,28,21,245,0,28,23,0,0,0,2,1,0,0,0,240,0,0,255,255,0,0,0,1,0,0,0,248,0,0,254,2,0,0,253,0,0,0,255,0,0,0,252,253,0,0,251,252,0,0,1,255,0,251,0,9,0,252,0,6,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,253,0,7,0,246,0,0,0,0,0,0,0,0,0,0,6,247,0,0,2,2,0,0,0,0,0,0,4,0,0,0,0,0,0,0,1,254,0,0,250,252,0,0,254,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,249,0,0,102,46,235,0,96,46,8,0,247,254,7,0,247,
253,2,0,247,250,254,0,243,244,246,0,247,252,4,0,247,1,11,0,244,252,5,0,250,254,1,0,243,248,255,0,252,252,253,0,237,239,242,0,0,255,255,0,244,0,13,0,254,0,4,0,0,0,255,0,244,253,6,0,253,255,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,
1,255,0,11,3,251,0,2,0,253,0,11,0,242,0,17,16,13,0,18,13,7,0,21,5,246,0,27,14,247,0,29,14,255,0,8,12,244,0,0,252,253,0,0,255,1,0,0,0,0,0,0,7,0,0,182,209,13,0,216,231,250,0,12,6,1,0,0,0,0,0,0,0,0,0,1,0,255,0,248,0,9,0,0,0,6,0,0,0,247,0,0,0,0,0,0,0,1,0,
13,11,6,0,0,0,251,0,0,0,0,9,0,0,0,0,0,0,0,2,0,0,0,1,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,2,0,0,0,0,0,0,0,7,254,254,255,0,12,13,11,0,0,4,242,0,0,255,249,0,0,0,1,0,0,0,1,0,246,1,13,0,1,0,249,0,0,0,0,0,0,0,0,0,0,0,0,0,12,6,0,0,155,199,249,0,40,
22,22,0,95,50,234,0,0,254,253,0,0,0,1,0,0,0,0,0,0,255,2,0,0,252,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,249,0,0,0,0,0,0,1,0,0,0,0,0,0,0,4,0,0,0,5,1,0,0,0,0,0,5,0,246,0,5,0,245,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,255,249,0,0,0,0,0,0,0,0,0,0,251,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,254,0,6,1,245,0,6,0,244,0,15,2,255,0,0,0,1,0,0,24,1,0,0,0,0,0,0,0,243,0,
0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,248,0,0,0,1,0,0,0,0,0,0,255,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,254,0,0,0,0,0,0,0,253,0,0,255,245,0,0,0,0,0,0,255,1,0,0,255,255,0,0,253,0,0,0,251,2,0,0,0,
1,0,0,0,0,0,0,2,254,0,0,248,12,0,136,194,4,0,4,2,250,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,255,0,247,253,5,0,0,254,1,0,0,0,0,0,0,0,0,0,0,0,0,0,11,5,2,0,0,255,0,0,0,0,0,7,0,0,0,0,0,0,0,2,0,0,0,1,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
4,0,0,0,0,255,255,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,0,0,252,251,251,0,147,198,9,0,90,44,13,0,5,6,239,0,0,254,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,255,0,0,
0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,255,1,0,0,0,1,0,0,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,253,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,255,0,0,253,9,0,108,182,17,0,210,226,247,0,12,7,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,249,253,255,0,21,17,10,0,171,88,245,0,5,9,239,0,0,251,2,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,255,0,202,225,18,0,165,205,254,0,9,5,254,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,252,0,0,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,250,253,3,
0,129,69,0,0,33,27,237,0,0,247,2,0,0,0,0,0,0,0,0,0,0,4,254,0,0,3,0,0,0,1,2,0,0,0,4,0,0,254,254,0,0,252,254,0,0,0,8,0,0,0,3,0,0,254,3,0,0,255,0,0,0,1,0,0,255,253,0,0,250,252,254,0,1,0,0,0,255,0,0,0,7,5,2,0,0,2,0,0,0,255,0,0,0,0,1,0,0,1,255,0,0,2,252,0,
0,0,0,0,0,0,251,0,0,255,251,0,0,4,2,0,0,2,1,0,0,1,2,0,0,0,0,0,0,1,254,0,0,0,255,0,0,0,254,0,0,255,255,0,0,0,0,0,0,255,0,0,0,0,0,0,0,252,1,0,0,254,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,255,0,0,1,255,0,0,0,255,
0,0,2,1,0,0,0,0,0,0,1,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,254,255,0,0,0,0,0,0,254,255,0,0,0,1,0,0,0,1,0,0,255,0,0,0,255,0,0,0,0,1,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,251,2,0,54,32,232,0,150,80,22,0,107,174,3,0,9,1,254,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,254,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,0,0,0,0,0,0,0,0,0,252,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,
0,0,0,0,0,0,255,0,0,0,254,0,0,0,0,0,0,0,249,2,3,1,5,241,241,244,0,0,251,251,0,0,5,1,0,0,255,0,0,0,0,255,0,11,255,242,0,255,0,9,0,0,0,255,0,0,0,0,0,249,252,255,0,45,28,7,0,90,47,239,0,0,248,255,0,0,0,1,0,0,0,0,0,0,5,0,0,236,238,16,0,200,231,10,0,219,233,
250,0,232,248,11,0,238,247,0,0,235,237,242,0,247,254,7,0,253,0,4,0,247,254,3,0,0,255,0,0,0,1,0,0,250,252,255,0,250,252,255,0,0,1,0,0,0,0,0,0,7,4,1,0,5,3,1,0,0,255,0,0,254,0,0,0,4,1,255,0,8,2,253,0,255,0,0,0,6,1,249,0,3,254,251,0,12,11,9,0,11,10,9,0,9,
6,0,0,8,3,1,0,10,5,250,0,8,0,249,0,10,1,246,0,18,17,15,0,13,8,3,0,12,6,255,0,8,0,248,0,49,22,245,0,11,13,249,0,0,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,251,10,0,243,251,8,0,254,0,1,0,247,252,255,0,254,253,0,0,248,
251,250,0,246,246,249,0,2,1,0,0,249,0,7,0,247,0,11,0,0,0,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,2,0,8,0,249,0,8,0,245,0,1,1,1,0,14,12,10,0,4,4,2,0,10,5,1,0,8,3,252,0,7,0,247,0,0,5,0,0,0,6,2,0,0,1,250,0,0,0,
254,0,0,0,0,0,0,253,255,0,0,253,1,0,0,255,1,0,0,0,0,0,0,251,254,0,89,54,237,0,188,221,23,0,173,206,246,0,12,7,0,0,0,0,0,0,0,0,0,0,255,0,1,0,9,2,252,0,0,1,254,0,0,0,0,0,0,0,0,0,0,0,0,0,242,250,254,0,0,1,0,0,0,0,0,249,0,0,0,0,0,0,0,254,0,0,0,255,0,0,0,
0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,254,0,0,0,0,0,0,0,247,1,0,0,12,249,252,1,0,0,255,5,0,0,0,249,0,0,0,1,0,0,255,255,0,16,13,9,0,3,2,2,0,0,0,253,0,0,0,0,0,248,252,2,0,80,42,255,0,0,7,246,0,0,248,2,0,0,0,0,0,0,6,254,0,201,222,24,0,86,167,4,0,240,
242,249,0,254,255,0,0,2,0,243,0,1,1,0,0,2,1,3,0,1,2,251,0,1,0,250,0,1,0,255,0,0,0,0,0,0,0,0,0,1,0,1,0,2,1,0,0,254,0,0,0,2,0,0,0,253,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,255,0,0,0,0,0,0,0,255,254,1,0,255,2,5,0,255,254,251,0,255,255,250,0,0,0,0,0,
255,255,0,0,254,0,1,0,0,0,0,0,255,0,9,0,3,2,252,0,2,2,252,0,254,253,255,0,9,8,7,0,203,106,254,0,11,2,247,0,0,254,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,255,0,181,218,16,0,105,176,3,0,10,5,255,0,251,253,255,0,255,255,0,0,250,251,
6,0,249,250,6,0,0,1,0,0,0,0,1,0,255,0,253,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,253,0,0,0,4,0,1,255,13,0,1,1,1,0,9,9,7,0,4,2,1,0,6,3,1,0,6,3,253,0,6,0,3,0,18,13,9,0,19,13,5,0,19,4,243,0,32,22,9,0,37,16,
249,0,46,24,253,0,33,20,245,0,0,1,249,0,0,254,1,0,0,255,1,0,0,254,250,0,114,59,247,0,179,212,5,0,255,255,250,0,1,1,1,0,0,0,0,0,255,0,2,0,8,0,246,0,0,0,250,0,0,0,1,0,0,0,0,0,0,0,255,0,241,243,246,0,0,0,0,0,0,0,0,247,0,0,0,0,0,0,0,254,0,0,0,255,0,0,0,0,
0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,253,0,0,0,0,0,0,0,244,2,0,255,21,250,0,9,0,247,255,254,0,9,0,248,0,0,0,1,0,0,255,255,0,19,15,9,0,4,4,251,0,255,255,253,0,0,0,0,0,8,6,2,0,70,34,248,0,0,253,255,0,0,0,1,0,0,1,255,0,0,252,9,0,124,186,5,0,240,241,
244,0,13,8,2,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,
0,0,0,253,0,0,0,0,0,240,251,0,0,106,59,2,0,29,25,241,0,0,248,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,255,0,0,0,0,0,0,255,1,0,0,0,0,0,0,0,0,0,0,251,0,0,76,36,240,0,131,12,15,0,218,230,247,0,8,6,1,0,1,0,0,0,0,1,0,0,7,4,1,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,252,248,247,0,251,0,0,0,246,255,0,0,0,0,0,0,254,0,1,0,224,238,246,0,253,253,0,0,254,0,1,0,254,255,247,0,2,1,1,0,10,8,6,0,56,33,11,0,166,82,239,0,9,
10,243,0,0,254,2,0,0,251,1,0,43,24,245,0,47,27,18,0,207,226,0,0,8,4,0,0,0,0,0,0,254,255,255,0,24,17,11,0,0,1,241,0,0,0,0,0,0,0,0,0,0,0,3,0,238,245,255,0,1,0,255,0,0,0,0,244,0,0,0,0,0,0,0,253,0,0,0,255,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,254,0,0,0,
253,0,0,0,0,0,0,0,243,2,1,0,0,246,251,0,0,242,248,0,0,0,2,0,0,0,0,0,0,0,255,1,0,20,4,245,0,3,0,253,0,0,0,0,0,253,254,255,0,24,16,6,0,39,19,250,0,0,253,0,0,0,0,0,0,0,4,255,0,215,232,11,0,209,229,251,0,12,7,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,252,254,255,0,27,19,9,0,111,54,244,0,0,252,253,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,254,0,0,253,11,0,0,254,9,0,0,1,255,0,0,0,0,0,0,0,0,0,0,255,1,0,0,10,246,0,113,60,255,0,252,0,4,0,3,2,1,0,2,2,1,0,2,1,1,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,
0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,2,2,1,0,3,2,1,0,4,3,1,0,4,3,1,0,6,3,0,0,8,4,1,0,11,7,1,0,13,7,0,0,15,8,255,0,13,7,254,0,1,254,249,0,181,210,245,0,147,203,14,0,0,6,2,0,0,0,0,0,0,255,1,0,0,6,250,0,56,32,2,0,251,252,1,0,0,0,0,0,0,0,0,0,254,255,
0,0,23,15,3,0,0,1,1,0,0,0,0,0,0,0,255,0,0,255,6,0,242,254,13,0,2,0,254,0,0,0,0,243,0,0,0,0,0,0,0,253,0,0,0,254,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,254,0,0,0,252,0,0,0,0,0,0,0,240,1,2,1,0,243,246,249,0,234,241,253,0,0,2,1,0,0,0,0,0,0,255,0,0,24,12,
251,0,14,10,6,0,254,255,255,0,252,255,1,0,24,11,250,0,6,9,250,0,0,255,1,0,0,0,0,0,0,2,0,0,204,231,5,0,247,250,253,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,244,250,2,0,103,56,0,0,15,18,241,0,0,252,2,0,0,0,0,0,0,0,0,0,0,0,255,0,0,7,3,0,115,187,19,0,139,197,19,0,0,7,1,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,249,0,0,88,45,242,0,46,28,6,0,248,252,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,1,1,0,13,6,255,0,188,220,1,0,0,249,6,0,0,1,255,0,0,0,0,0,0,255,254,0,44,19,247,0,0,1,2,0,0,0,0,0,0,0,0,0,0,1,1,0,25,6,242,0,0,0,254,0,0,0,0,0,0,1,0,0,0,249,253,0,233,242,250,0,3,3,1,0,0,0,0,240,0,0,0,0,0,
0,0,252,0,0,0,254,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,254,0,0,0,252,0,0,0,0,0,0,0,238,1,0,0,0,252,255,1,0,230,249,13,0,0,0,255,0,0,0,0,0,0,253,0,0,18,11,0,0,17,9,0,0,253,255,0,0,253,253,255,0,23,16,9,0,0,3,255,0,0,255,0,0,0,0,0,0,0,1,3,0,220,234,
253,0,255,255,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,250,253,255,0,33,22,8,0,108,53,243,0,0,250,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,255,0,206,222,19,0,159,200,247,0,148,198,253,0,232,234,17,0,0,5,254,0,0,0,0,0,0,0,0,0,0,255,1,0,0,9,246,0,107,58,254,0,249,252,2,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,4,0,0,228,239,253,0,247,
248,8,0,0,1,255,0,0,0,0,0,0,254,254,0,28,18,7,0,4,5,3,0,255,255,0,0,255,255,0,0,6,4,4,0,34,23,8,0,0,253,254,0,0,0,0,0,0,1,255,0,249,252,8,0,236,243,254,0,3,1,0,0,0,0,0,238,0,0,0,0,0,0,0,252,0,0,0,254,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,254,0,0,0,
251,0,0,0,249,0,0,0,236,0,0,0,0,252,252,254,0,220,233,248,0,0,2,1,0,0,0,0,0,0,0,1,0,0,6,249,0,28,17,1,0,252,253,0,0,255,0,0,0,14,8,3,0,0,2,1,0,0,0,0,0,0,0,0,0,0,255,3,0,241,254,12,0,4,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,247,252,3,0,112,59,255,0,6,13,242,0,0,254,2,0,0,0,0,0,0,0,0,
0,0,1,254,0,0,251,12,0,118,184,8,0,0,253,252,0,241,242,248,0,120,188,10,0,0,1,7,0,0,0,255,0,0,0,0,0,0,0,0,0,0,249,0,0,90,45,242,0,46,30,9,0,248,252,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,245,252,3,0,239,249,4,0,0,2,0,0,0,0,0,0,0,0,1,0,15,7,251,0,5,1,254,0,0,0,0,0,255,255,0,0,11,6,1,0,
32,12,246,0,0,255,1,0,0,0,0,0,0,2,0,0,233,243,2,0,240,248,1,0,2,2,0,0,0,0,0,236,0,0,0,249,0,0,0,251,0,0,0,254,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,253,0,0,0,251,0,0,0,248,0,0,0,234,0,0,0,0,2,0,254,0,220,241,9,0,0,255,2,0,0,0,0,0,0,255,1,0,0,4,254,
0,40,20,255,0,253,254,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,253,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,248,252,255,0,44,27,8,0,99,50,241,0,0,250,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,2,0,145,199,16,0,200,223,246,0,10,6,1,0,12,7,1,0,176,210,249,0,185,214,18,0,0,8,255,0,0,0,
0,0,0,0,0,0,0,255,1,0,0,8,246,0,115,60,254,0,251,253,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,0,252,253,255,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,254,0,0,0,0,0,0,0,0,0,251,254,255,0,24,15,3,0,27,15,254,0,0,253,0,0,0,0,0,0,0,2,0,0,222,240,7,0,245,249,251,0,2,1,1,0,0,0,0,234,0,0,0,248,0,
0,0,251,0,0,0,253,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,254,0,0,0,250,0,0,0,247,0,0,0,233,0,0,0,251,4,4,0,0,215,232,252,0,0,251,5,0,0,1,255,0,0,0,0,0,0,255,252,0,48,25,254,0,254,1,2,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,249,253,
3,0,117,62,253,0,0,9,244,0,0,254,2,0,0,0,0,0,0,0,0,0,0,5,253,0,226,231,17,0,140,193,254,0,13,5,254,0,0,0,0,0,0,1,0,0,4,0,252,0,126,187,3,0,0,246,12,0,0,2,254,0,0,0,0,0,0,0,0,0,0,250,255,0,82,43,240,0,49,31,7,0,247,250,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,255,0,0,0,0,0,0,0,0,0,0,0,252,253,0,0,36,20,0,0,0,7,249,0,0,255,1,0,0,0,0,0,0,2,2,0,213,235,2,0,252,253,255,0,0,0,0,251,0,0,0,233,0,0,0,247,0,0,0,250,0,0,0,254,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,254,0,0,0,250,0,0,0,245,0,0,0,235,0,0,0,229,5,
2,0,0,226,240,2,0,247,246,7,0,0,1,255,0,0,0,0,0,0,254,0,0,49,24,249,0,11,7,3,0,0,0,255,0,244,248,253,0,0,254,255,0,0,0,0,0,0,0,0,0,0,1,254,0,12,3,247,0,253,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,246,250,255,0,59,35,8,0,82,44,239,0,0,249,1,0,0,0,0,0,0,0,0,0,0,1,255,0,0,0,10,0,117,186,11,0,243,243,248,
0,3,2,1,0,0,0,0,0,0,0,0,0,8,4,1,0,213,229,248,0,136,197,17,0,0,6,2,0,0,0,0,0,0,0,0,0,0,255,2,0,0,6,245,0,115,61,252,0,252,0,5,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,6,4,1,0,11,7,1,0,0,254,255,0,0,0,0,0,0,0,255,0,246,252,6,0,253,255,2,0,0,0,0,0,252,255,2,0,51,26,255,0,0,3,252,0,0,0,1,0,0,0,0,0,0,
255,2,0,210,232,3,0,3,0,254,0,0,0,0,229,0,0,0,235,0,0,0,245,0,0,0,250,0,0,0,254,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,251,0,0,0,247,0,0,0,0,0,0,0,227,3,1,1,0,234,243,253,0,214,237,6,0,0,4,0,0,0,0,0,0,0,252,1,0,38,19,250,0,29,18,4,0,253,0,
1,0,234,240,246,0,0,252,254,0,0,1,0,0,0,0,0,0,0,0,253,0,26,13,252,0,0,1,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,0,0,0,3,4,0,123,65,251,0,0,5,247,0,0,255,1,0,0,0,0,0,0,0,0,0,0,7,0,0,170,207,17,0,182,213,248,0,12,8,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,13,6,254,0,149,196,252,0,224,230,16,0,0,5,254,0,0,0,0,0,
0,0,0,0,0,250,1,0,77,41,240,0,57,35,8,0,246,250,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,0,0,11,3,252,0,16,5,247,0,0,0,2,0,0,0,0,0,0,1,1,0,238,246,252,0,252,255,0,0,0,0,0,0,5,5,2,0,55,29,253,0,0,254,254,0,0,0,0,0,0,1,255,0,0,248,7,0,212,233,255,0,5,3,0,0,0,0,0,227,0,0,0,0,0,0,0,247,0,0,0,251,0,0,0,255,
0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,252,0,0,0,248,0,0,0,252,0,0,0,225,1,1,0,0,249,250,252,0,202,229,5,0,0,2,1,0,0,0,0,0,0,255,1,0,0,9,250,0,48,26,0,0,252,253,255,0,239,250,8,0,0,254,11,0,0,0,255,0,0,0,0,0,0,254,255,0,34,20,5,0,3,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,245,249,255,0,72,41,7,0,61,37,
239,0,0,249,1,0,0,0,0,0,0,0,0,0,0,4,254,0,246,240,15,0,129,188,1,0,8,2,252,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,0,242,245,249,0,123,190,12,0,0,1,8,0,0,0,255,0,0,0,0,0,0,255,1,0,0,5,247,0,117,60,250,0,254,2,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,251,253,0,0,25,17,6,0,2,7,253,0,0,255,0,0,0,0,0,0,0,2,2,
0,227,238,253,0,251,251,252,0,253,255,255,0,22,13,4,0,51,23,247,0,0,252,0,0,0,0,0,0,0,3,255,0,228,240,6,0,225,236,253,0,5,3,0,0,0,0,0,225,0,0,0,252,0,0,0,248,0,0,0,252,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,252,0,0,0,248,0,0,0,244,0,
0,0,224,0,0,0,0,4,1,254,0,197,226,2,0,0,254,5,0,0,0,255,0,0,0,0,0,0,1,251,0,66,34,252,0,2,2,3,0,235,243,252,0,237,244,254,0,0,3,0,0,0,0,0,0,0,254,0,0,38,16,247,0,11,8,4,0,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,254,254,255,0,9,9,8,0,128,65,248,0,0,255,249,0,0,0,1,0,0,0,0,0,0,0,255,0,0,4,4,0,122,190,12,0,226,236,248,0,
7,4,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,8,1,0,182,212,250,0,186,215,17,0,0,7,255,0,0,0,0,0,0,0,0,0,0,249,1,0,67,40,240,0,66,38,6,0,245,249,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,253,255,1,0,35,17,252,0,0,5,252,0,0,255,1,0,0,0,0,0,0,255,0,0,224,244,11,0,3,0,255,0,249,251,0,0,51,28,3,0,9,14,247,0,0,
254,1,0,0,0,0,0,0,3,0,0,199,229,8,0,243,247,252,0,2,1,1,0,0,0,0,224,0,0,0,244,0,0,0,248,0,0,0,252,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,253,0,0,0,249,0,0,0,243,0,0,0,222,0,0,0,229,7,4,0,0,207,227,255,0,248,243,9,0,0,2,255,0,0,0,0,0,
0,252,255,0,66,32,248,0,19,12,5,0,239,246,253,0,220,240,9,0,0,2,0,0,0,0,0,0,0,253,1,0,34,18,251,0,25,15,2,0,251,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,243,250,1,0,92,53,4,0,38,29,239,0,0,251,1,0,0,0,0,0,0,0,0,0,0,6,255,0,201,219,20,0,163,204,252,0,14,7,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,
3,254,250,0,129,189,4,0,0,248,13,0,0,2,254,0,0,0,0,0,0,0,1,0,0,1,248,0,122,63,252,0,3,6,6,0,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,253,255,1,0,49,26,255,0,0,1,253,0,0,0,0,0,0,0,255,0,0,255,5,0,217,232,252,0,3,2,255,0,252,0,2,0,70,38,253,0,0,2,250,0,0,0,1,0,0,0,0,0,0,0,4,0,192,223,3,0,1,0,253,0,0,0,0,229,0,0,0,222,0,
0,0,243,0,0,0,249,0,0,0,253,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,248,0,0,0,243,0,0,0,248,0,0,0,219,4,3,1,0,230,241,251,0,200,229,7,0,0,4,0,0,0,0,0,0,0,252,1,0,23,19,248,0,51,30,3,0,242,247,253,0,207,231,3,0,0,2,1,0,0,0,0,
0,0,255,0,0,0,9,250,0,48,27,2,0,250,252,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,252,253,
255,0,26,18,9,0,124,60,244,0,0,252,251,0,0,0,1,0,0,0,0,0,0,1,254,0,0,250,11,0,121,187,6,0,255,252,250,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,6,1,0,209,226,247,0,145,201,15,0,0,6,2,0,0,0,0,0,0,0,0,0,0,250,1,0,52,33,
238,0,78,44,5,0,244,250,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,0,0,8,6,4,0,53,
26,250,0,0,253,255,0,0,0,0,0,0,1,0,0,0,247,5,0,221,238,2,0,2,0,255,0,18,12,6,0,72,35,247,0,0,252,0,0,0,0,0,0,0,1,255,0,0,246,9,0,199,225,255,0,6,4,0,0,0,0,0,219,0,0,0,248,0,0,0,243,0,0,0,248,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,255,0,0,0,250,0,0,0,244,0,0,0,249,0,0,0,217,0,0,0,0,254,253,253,0,182,220,6,0,0,2,3,0,0,0,0,0,0,0,1,0,0,3,248,0,82,44,255,0,255,0,1,0,199,226,2,0,0,254,3,0,0,0,0,0,0,0,1,0,0,0,250,0,69,35,253,0,253,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,244,251,2,0,110,60,1,0,15,18,240,0,0,252,3,0,0,0,0,0,0,0,0,0,0,7,2,0,149,202,17,0,202,222,
246,0,10,6,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,5,254,0,146,195,255,0,236,236,16,0,0,4,254,0,0,0,0,0,0,0,1,0,0,255,250,0,123,62,247,0,11,8,7,0,254,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,251,254,255,0,31,18,3,0,47,23,249,0,0,252,1,0,0,0,0,0,0,4,255,0,218,239,8,0,229,240,253,0,252,254,1,0,
53,30,2,0,23,18,246,0,0,252,1,0,0,0,0,0,0,5,255,0,198,229,8,0,224,237,251,0,6,3,1,0,0,0,0,217,0,0,0,249,0,0,0,244,0,0,0,250,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,252,0,0,0,245,0,0,0,239,0,0,0,216,0,0,0,238,7,4,255,
0,190,220,0,0,0,246,10,0,0,1,254,0,0,0,0,0,0,250,0,0,78,37,246,0,29,18,6,0,203,226,255,0,251,245,9,0,0,2,254,0,0,0,0,0,0,253,0,0,69,33,246,0,21,14,5,0,253,254,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,248,252,255,0,45,30,9,0,107,53,239,0,0,249,254,0,0,0,0,0,0,0,0,0,0,4,254,0,233,235,17,0,141,192,254,0,12,6,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,3,1,0,234,241,248,0,130,194,11,0,0,3,4,0,0,0,255,0,0,0,0,0,0,251,1,0,35,27,239,0,93,53,4,0,243,249,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,249,251,0,0,62,34,1,0,0,11,248,0,0,255,1,0,0,0,0,0,0,3,1,0,194,224,5,0,245,248,251,0,253,1,3,0,85,44,253,0,0,2,250,0,0,0,1,0,0,0,0,0,0,2,3,0,177,216,6,0,251,
251,253,0,0,0,0,238,0,0,0,216,0,0,0,239,0,0,0,245,0,0,0,252,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,253,0,0,0,247,0,0,0,240,0,0,0,233,0,0,0,213,6,3,0,0,221,234,250,0,191,224,11,0,0,5,0,0,0,0,0,0,0,254,2,0,15,17,243,0,75,
41,2,0,218,234,251,0,197,227,9,0,0,5,0,0,0,0,0,0,0,252,2,0,18,19,246,0,64,36,3,0,247,251,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,251,254,3,0,126,64,251,0,0,8,244,0,0,255,2,0,0,0,0,0,0,0,255,0,0,3,6,0,123,189,10,0,237,242,248,0,4,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,14,7,255,0,168,205,252,
0,209,224,17,0,0,6,255,0,0,0,0,0,0,0,1,0,0,252,253,0,120,60,244,0,24,17,8,0,252,253,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,2,4,4,0,80,41,250,0,0,254,252,0,0,0,0,0,0,0,255,0,0,254,6,0,188,220,3,0,255,255,253,0,30,19,6,0,79,38,246,0,0,251,255,0,0,0,0,0,0,1,254,0,255,244,10,0,185,217,0,0,9,5,255,0,0,0,0,213,0,0,0,233,0,0,0,240,0,0,0,247,0,0,0,253,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,249,0,0,0,242,0,0,0,248,0,0,0,213,0,0,1,0,255,253,252,0,168,210,4,0,0,1,6,0,0,0,255,0,0,0,0,0,0,255,252,0,98,51,249,0,3,2,1,0,172,213,6,0,0,1,4,0,0,0,0,0,0,0,0,0,0,255,250,
0,99,50,249,0,1,3,4,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,244,249,255,0,74,44,9,0,73,43,239,0,0,250,1,0,0,0,
0,0,0,0,0,0,0,7,255,0,193,216,18,0,174,209,251,0,12,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,251,250,249,0,128,191,8,0,0,255,8,0,0,1,255,0,0,0,0,0,0,252,2,0,17,18,241,0,108,
59,2,0,244,251,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,248,252,255,0,42,24,7,0,62,32,245,0,0,251,1,0,0,0,0,0,0,
4,255,0,238,239,10,0,204,228,254,0,254,255,1,0,82,46,2,0,8,15,244,0,0,254,2,0,0,0,0,0,0,6,0,0,185,223,13,0,224,235,251,0,6,3,1,0,0,0,0,213,0,0,0,248,0,0,0,242,0,0,0,249,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,252,0,0,0,244,0,0,0,238,0,0,0,211,0,0,0,226,10,5,0,0,187,218,254,0,237,237,11,0,0,4,255,0,0,0,0,0,0,251,1,0,52,31,243,0,64,36,4,0,177,212,254,0,244,239,12,0,0,4,254,0,0,0,0,0,0,250,1,0,59,35,242,0,64,37,6,0,246,249,255,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,254,254,255,0,11,9,7,0,131,67,246,0,0,0,249,0,0,255,1,0,0,0,0,0,0,1,254,0,0,249,12,0,126,189,5,0,1,253,250,0,1,1,
1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,7,1,0,191,216,249,0,179,212,16,0,0,7,255,0,0,0,0,0,0,0,0,0,0,250,254,0,111,53,239,0,42,27,8,0,248,252,255,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,248,252,2,0,95,52,253,0,0,9,246,0,0,255,1,0,0,0,0,0,0,4,1,0,176,216,9,0,231,239,249,0,17,12,7,0,99,48,246,
0,0,253,253,0,0,0,0,0,0,0,255,0,0,255,6,0,162,207,2,0,1,255,252,0,0,0,0,226,0,0,0,211,0,0,0,238,0,0,0,244,0,0,0,252,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,253,0,0,0,247,0,0,0,238,0,0,0,239,0,0,0,208,2,
2,1,0,234,242,250,0,166,212,10,0,0,4,2,0,0,0,0,0,0,255,1,0,0,3,248,0,111,59,254,0,228,240,252,0,163,212,11,0,0,4,1,0,0,0,0,0,0,0,1,0,0,2,248,0,128,66,249,0,5,6,7,0,254,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,242,249,1,0,104,58,3,0,36,27,239,0,0,251,1,0,0,0,0,0,0,0,0,0,0,7,1,0,148,202,16,0,206,224,247,0,9,6,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,2,252,0,136,191,4,0,0,247,11,0,0,1,255,0,0,0,0,0,0,254,2,0,0,10,245,0,123,66,255,0,252,254,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,249,252,254,0,40,26,9,0,102,49,243,0,0,250,255,0,0,0,0,0,0,1,255,0,0,250,9,0,167,210,3,0,251,253,254,0,78,44,4,0,32,25,241,0,0,251,2,0,0,0,0,0,0,5,255,0,223,232,13,0,192,219,253,0,9,
6,0,0,0,0,0,208,0,0,0,239,0,0,0,238,0,0,0,247,0,0,0,253,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,249,0,0,0,240,0,0,0,238,0,0,0,207,0,0,0,244,9,4,254,0,163,206,2,0,0,246,10,0,0,1,254,0,0,0,0,0,0,
250,1,0,74,40,242,0,59,35,5,0,148,196,0,0,0,246,12,0,0,1,254,0,0,0,0,0,0,250,1,0,47,33,239,0,108,61,5,0,241,248,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,248,252,255,0,40,26,9,0,124,61,241,0,0,250,253,0,0,0,1,0,0,0,0,0,0,4,254,0,238,237,16,0,144,194,255,0,11,6,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,8,4,1,0,210,228,248,0,156,206,16,0,0,7,0,0,0,0,0,0,0,0,0,0,0,250,0,0,79,44,238,0,65,40,8,0,245,249,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,255,255,255,0,250,254,5,0,123,66,254,0,0,11,244,0,0,254,2,0,0,0,0,0,0,6,255,0,194,223,15,0,203,223,248,0,16,12,8,0,113,57,246,0,0,254,252,0,0,0,0,0,0,0,0,0,0,3,3,0,155,205,10,0,243,247,250,0,2,1,1,244,0,0,0,207,0,0,0,238,0,0,0,240,0,0,0,249,0,0,
0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,251,0,0,0,243,0,0,0,235,0,0,0,217,0,0,0,205,7,4,1,0,217,231,248,0,168,213,14,0,0,7,1,0,0,0,0,0,0,255,1,0,0,6,246,0,124,65,254,0,214,233,253,0,145,203,
16,0,0,7,2,0,0,0,0,0,0,0,1,0,0,250,252,0,126,65,236,0,63,40,9,0,243,248,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,0,0,252,0,4,0,132,70,253,0,1,10,243,
0,0,254,2,0,0,0,0,0,0,0,255,0,0,4,6,0,128,193,13,0,233,240,247,0,4,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,9,5,254,0,145,195,0,0,249,241,14,
0,0,2,254,0,0,0,0,0,0,255,1,0,0,1,247,0,132,68,247,0,7,7,7,0,254,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,241,248,1,0,85,50,7,0,81,46,236,0,0,249,
0,0,0,0,0,0,0,1,255,0,0,254,8,0,144,197,5,0,243,247,252,0,84,47,5,0,38,28,243,0,0,251,1,0,0,0,0,0,0,4,254,0,239,237,13,0,170,209,252,0,11,6,255,0,0,0,0,205,0,0,0,217,0,0,0,235,0,0,0,243,0,0,0,251,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,254,0,0,0,248,0,0,0,238,0,0,0,242,0,0,0,203,0,0,0,252,5,3,253,0,147,199,2,0,0,248,10,0,0,1,255,0,0,0,0,0,0,250,1,0,72,42,240,0,74,43,4,0,133,187,252,0,231,235,17,0,0,5,253,0,0,0,0,0,0,254,2,0,0,4,246,
0,166,83,244,0,35,25,11,0,246,249,255,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,244,249,255,0,81,46,7,0,72,43,238,0,0,249,1,0,0,0,0,0,0,0,0,0,0,6,254,0,210,225,16,0,168,206,
251,0,13,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,3,1,0,223,236,249,0,143,202,13,0,0,6,2,0,0,0,0,0,0,0,0,0,0,250,2,0,43,30,238,0,98,
55,4,0,242,249,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,245,249,255,0,45,31,11,0,151,75,242,0,0,252,249,0,0,255,1,0,0,0,0,0,0,7,0,0,184,214,17,0,186,215,246,0,
26,18,8,0,122,60,244,0,0,254,250,0,0,0,1,0,0,0,0,0,0,4,2,0,148,203,11,0,233,240,250,0,4,2,1,252,0,0,0,203,0,0,0,242,0,0,0,238,0,0,0,248,0,0,0,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,251,0,0,0,241,0,0,0,232,0,0,0,206,0,0,0,201,8,4,1,0,210,227,249,0,169,211,15,0,0,7,255,0,0,0,0,0,0,255,1,0,0,1,246,0,134,68,249,0,244,248,255,0,106,179,9,0,0,255,10,0,0,1,255,0,0,0,0,0,0,251,2,0,16,16,239,0,176,90,247,0,25,18,9,0,245,250,255,
0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,250,253,255,0,23,17,9,0,140,71,245,0,0,254,249,0,0,0,1,0,0,0,0,0,0,1,255,0,0,255,10,0,128,190,9,0,251,249,249,0,2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,5,254,0,156,201,255,0,239,237,15,0,0,4,254,0,0,0,0,0,0,0,0,0,0,250,253,0,126,62,243,0,33,22,9,0,249,252,255,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,246,250,255,0,21,16,9,0,170,86,246,0,5,11,241,0,0,254,2,0,0,0,0,0,0,4,254,0,254,244,14,0,125,187,3,0,248,250,254,0,104,59,4,0,28,25,241,0,0,
251,2,0,0,0,0,0,0,4,254,0,239,237,15,0,161,203,255,0,12,6,255,0,0,0,0,201,0,0,0,206,0,0,0,232,0,0,0,241,0,0,0,251,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,254,0,0,0,247,0,
0,0,237,0,0,0,242,0,0,0,199,0,0,0,249,8,3,253,0,141,195,2,0,0,246,12,0,0,2,254,0,0,0,0,0,0,250,3,0,47,33,239,0,107,61,5,0,176,206,248,0,136,190,18,0,0,7,2,0,0,0,255,0,0,0,0,0,0,250,2,0,30,24,236,0,179,95,249,0,26,18,10,0,244,249,255,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,247,252,3,0,129,70,1,0,19,19,239,0,0,252,2,0,0,0,0,0,0,0,0,0,0,6,255,0,183,213,15,0,190,217,247,0,11,7,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,3,1,0,232,240,248,0,143,201,13,0,0,4,3,0,0,0,0,0,0,0,0,0,0,254,2,0,8,15,241,0,129,67,255,0,250,253,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,245,250,255,0,17,13,8,0,174,93,250,0,29,23,237,0,0,250,2,0,0,0,0,0,0,1,255,0,0,4,7,0,112,187,16,0,209,225,246,0,50,31,10,0,123,58,240,0,0,250,253,0,0,0,0,0,0,0,0,0,0,4,3,0,138,198,12,0,233,
241,249,0,3,2,1,249,0,0,0,199,0,0,0,242,0,0,0,237,0,0,0,247,0,0,0,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,251,0,0,0,241,0,0,0,231,0,0,0,206,0,0,0,197,7,4,1,
0,214,230,247,0,144,201,16,0,0,6,2,0,0,0,0,0,0,0,0,0,0,250,252,0,132,66,243,0,44,30,8,0,131,186,251,0,177,209,24,0,0,9,1,0,0,0,0,0,0,0,0,0,0,249,2,0,34,25,237,0,186,96,246,0,43,30,12,0,241,248,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,243,248,255,0,78,47,9,0,89,51,236,0,0,249,0,0,0,0,0,0,0,0,0,0,0,1,255,0,0,249,11,0,135,193,4,0,3,254,252,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,13,6,255,0,163,204,252,0,232,234,14,0,0,4,254,0,0,0,0,0,0,0,0,0,0,248,0,0,82,49,237,0,76,47,9,0,243,248,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,243,249,255,0,26,17,9,0,178,95,251,0,44,30,236,0,0,249,1,0,0,0,0,0,0,0,0,0,0,8,1,0,162,202,22,0,161,198,246,0,11,9,4,0,135,72,253,0,0,11,243,0,0,254,2,0,0,0,0,0,0,6,254,0,219,228,17,0,161,203,252,0,13,6,255,0,0,0,0,197,0,0,0,206,0,0,0,231,
0,0,0,241,0,0,0,251,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,254,0,0,0,247,0,0,0,236,0,0,0,239,0,0,0,196,0,0,0,236,13,6,255,0,146,196,255,0,233,235,16,0,
0,4,254,0,0,0,0,0,0,254,2,0,7,12,240,0,142,75,253,0,1,0,255,0,105,173,254,0,198,219,23,0,0,8,254,0,0,0,0,0,0,0,0,0,0,249,1,0,26,22,238,0,191,97,242,0,82,50,12,0,240,249,2,0,252,253,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,249,252,
255,0,28,20,10,0,149,74,243,0,0,254,248,0,0,255,1,0,0,0,0,0,0,0,0,0,0,6,255,0,166,210,15,0,207,226,249,0,9,6,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,0,236,242,249,0,142,198,11,0,0,4,3,0,0,0,0,0,0,0,0,0,0,0,1,0,0,255,248,0,140,70,245,0,19,15,9,0,252,253,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,0,240,247,0,
0,52,33,11,0,189,100,247,0,42,29,235,0,0,248,1,0,0,0,0,0,0,0,0,0,0,8,254,0,197,218,24,0,119,179,252,0,249,252,253,0,97,56,8,0,67,41,237,0,0,249,1,0,0,0,0,0,0,1,255,0,0,0,8,0,123,188,10,0,246,247,249,0,2,2,1,236,0,0,0,196,0,0,0,239,0,0,0,236,0,0,0,247,
0,0,0,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,250,0,0,0,241,0,0,0,230,0,0,0,216,0,0,0,193,3,2,1,0,237,241,248,0,120,187,11,0,0,2,6,0,0,1,255,
0,0,0,0,0,0,249,1,0,69,43,235,0,110,63,9,0,234,241,249,0,93,167,2,0,205,222,24,0,0,8,255,0,0,0,255,0,0,0,0,0,0,250,2,0,5,8,240,0,161,87,234,0,141,79,7,0,6,8,8,0,243,248,255,0,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,254,254,255,0,254,2,6,0,146,79,
255,0,16,17,239,0,0,252,2,0,0,0,0,0,0,0,0,0,0,1,254,0,0,246,13,0,143,196,2,0,8,3,253,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,12,6,255,0,173,211,255,0,237,237,15,0,0,4,253,0,0,0,0,0,0,0,0,0,0,251,2,0,25,22,238,0,131,71,0,0,245,252,4,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,248,252,255,0,245,253,4,0,102,60,10,0,188,96,239,
0,23,19,238,0,0,249,2,0,0,0,0,0,0,0,0,0,0,8,255,0,207,222,22,0,94,168,1,0,243,245,248,0,49,31,10,0,141,68,239,0,0,251,250,0,0,0,1,0,0,0,0,0,0,7,0,0,174,208,18,0,185,214,248,0,12,7,1,0,0,0,0,193,0,0,0,216,0,0,0,230,0,0,0,241,0,0,0,250,0,0,0,255,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,247,0,0,0,237,0,0,0,232,0,0,0,194,0,0,0,212,12,7,0,0,175,210,249,0,178,211,20,0,0,7,255,0,0,0,0,0,0,0,
1,0,0,250,252,0,134,69,239,0,65,40,13,0,230,238,246,0,81,163,2,0,191,214,24,0,0,9,1,0,0,1,255,0,0,0,0,0,0,252,2,0,0,252,248,0,103,60,230,0,198,103,249,0,84,51,10,0,250,0,6,0,241,248,0,0,250,253,255,0,255,255,0,0,254,254,255,0,241,250,3,0,126,71,6,0,67,
41,232,0,0,249,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,1,0,159,210,15,0,215,231,248,0,7,4,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,2,1,0,234,241,248,0,151,203,11,0,0,4,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,248,0,0,91,52,236,0,91,52,7,0,241,247,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,249,252,255,0,240,248,1,0,34,23,11,0,168,91,3,0,142,80,233,0,0,4,244,
0,0,250,2,0,0,0,0,0,0,0,255,0,0,9,255,0,207,222,21,0,84,166,5,0,237,241,246,0,14,12,9,0,153,81,250,0,1,10,243,0,0,254,2,0,0,0,0,0,0,4,253,0,242,239,16,0,125,186,1,0,7,2,252,0,0,0,0,212,0,0,0,194,0,0,0,232,0,0,0,237,0,0,0,247,0,0,0,255,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,252,0,0,0,241,0,0,0,229,0,0,0,227,0,0,0,191,1,1,0,248,6,1,252,0,118,182,1,0,240,238,18,0,0,4,254,0,0,0,0,
0,0,254,2,0,0,5,243,0,160,81,244,0,29,20,11,0,241,242,247,0,89,166,0,0,162,201,27,0,0,6,5,0,0,2,254,0,0,0,0,0,0,255,1,0,0,248,0,0,28,21,237,0,157,87,235,0,190,100,251,0,101,59,10,0,25,17,9,0,250,255,3,0,249,0,7,0,120,69,9,0,111,61,232,0,0,249,254,0,0,
0,1,0,0,0,0,0,0,0,0,0,0,1,255,0,0,246,11,0,152,200,1,0,8,3,253,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,11,5,255,0,167,207,255,0,247,241,14,0,0,2,254,0,0,0,0,0,0,0,0,0,0,0,1,0,0,254,249,0,152,76,239,0,52,34,12,0,238,246,0,0,247,250,255,0,243,248,0,0,243,250,3,0,30,21,10,0,136,77,7,0,202,103,240,0,72,44,232,0,0,249,251,0,0,
254,2,0,0,0,0,0,0,1,255,0,0,9,1,0,187,211,24,0,80,163,4,0,240,242,246,0,255,2,5,0,136,75,2,0,46,31,236,0,0,250,2,0,0,0,0,0,0,1,255,0,0,2,8,0,112,184,12,0,228,236,247,0,6,4,1,248,0,0,0,191,0,0,0,227,0,0,0,229,0,0,0,241,0,0,0,252,0,0,0,255,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,15,0,0,0,32,0,0,0,38,0,0,0,90,0,0,0,76,243,249,255,0,76,47,12,0,165,90,234,0,
0,251,248,0,0,255,2,0,0,0,0,0,0,8,254,0,193,217,25,0,84,162,0,0,255,251,248,0,252,0,5,0,102,59,7,0,138,70,239,0,0,8,244,0,0,249,1,0,0,0,1,0,0,0,0,0,0,0,255,0,0,5,255,0,0,2,4,0,253,244,9,0,216,236,5,0,7,5,1,0,36,24,243,0,0,253,250,0,0,255,2,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,7,255,0,175,207,24,0,94,167,252,0,15,7,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,240,248,1,0,145,81,6,0,99,59,231,0,0,248,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,254,0,240,241,17,0,114,185,12,0,231,240,252,0,37,22,2,0,72,36,253,0,73,36,245,0,1,11,245,0,0,250,254,0,0,254,2,0,0,0,0,0,0,0,0,0,0,4,254,0,0,
2,8,0,151,198,21,0,126,183,251,0,1,253,249,0,246,252,3,0,110,63,9,0,134,76,233,0,0,250,251,0,0,255,1,0,0,0,0,0,0,8,255,0,165,202,27,0,106,172,250,0,12,6,253,0,1,0,0,180,0,0,0,166,0,0,0,218,0,0,0,224,0,0,0,241,0,0,0,252,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,6,0,0,0,21,0,0,0,37,0,0,0,45,0,0,0,97,0,0,0,48,241,248,1,0,132,77,9,0,111,63,230,0,
0,248,254,0,0,0,1,0,0,0,255,0,0,8,1,0,165,203,26,0,107,173,252,0,9,2,249,0,244,250,2,0,51,32,10,0,145,75,249,0,47,34,237,0,0,251,253,0,0,252,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,254,0,0,3,0,0,0,0,0,0,0,251,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,4,
7,0,78,160,20,0,196,218,246,0,10,6,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,247,250,255,0,50,32,10,0,187,99,237,0,0,0,248,0,0,255,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,254,0,0,4,6,0,0,254,3,0,0,3,250,0,0,0,253,0,0,251,0,0,0,254,2,0,0,0,0,0,0,0,0,0,0,1,255,0,0,7,0,0,247,240,14,0,108,184,15,0,172,206,247,0,11,4,253,0,242,
249,1,0,84,51,11,0,160,86,235,0,0,254,247,0,0,254,2,0,0,0,0,0,0,6,254,0,218,231,24,0,68,156,2,0,250,249,248,0,4,2,1,208,0,0,0,159,0,0,0,211,0,0,0,219,0,0,0,235,0,0,0,250,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,
228,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,251,0,0,0,240,0,0,0,228,0,0,0,224,0,0,0,186,0,0,0,232,12,6,254,0,132,186,252,0,194,217,22,0,0,8,0,0,0,0,0,0,0,0,1,0,0,248,255,0,91,
53,232,0,129,73,5,0,246,252,4,0,14,7,255,0,210,226,245,0,72,157,3,0,127,182,27,0,247,245,16,0,0,9,0,0,0,4,254,0,0,0,255,0,0,0,0,0,0,254,2,0,0,251,2,0,0,251,2,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,5,255,0,220,231,9,0,203,224,252,0,9,5,1,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,5,0,0,208,228,252,0,211,
230,11,0,0,4,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,254,2,0,0,250,251,0,0,252,248,0,0,249,255,0,0,249,2,0,0,254,2,0,0,0,0,0,0,0,255,0,0,6,254,0,0,4,7,0,178,209,25,0,55,154,12,0,172,204,245,0,13,6,253,0,243,250,1,0,66,41,10,0,160,81,238,0,0,0,247,0,0,
254,2,0,0,0,0,0,0,4,254,0,253,246,15,0,95,171,5,0,244,245,248,0,4,2,1,232,0,0,0,186,0,0,0,224,0,0,0,228,0,0,0,240,0,0,0,251,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,254,0,0,0,248,0,0,0,236,0,0,0,224,0,0,0,206,0,0,0,184,3,2,1,253,0,253,249,0,101,173,3,0,229,232,19,0,0,6,253,0,0,0,0,0,0,0,1,0,0,249,253,0,111,63,234,0,115,66,8,0,241,249,2,0,
9,5,0,0,1,255,249,0,142,189,246,0,53,151,11,0,144,191,26,0,239,241,17,0,0,7,5,0,0,7,255,0,0,5,254,0,0,4,254,0,0,4,254,0,0,6,254,0,0,7,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,201,232,12,0,243,248,253,0,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,0,248,250,252,0,188,224,8,0,0,3,1,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,1,254,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,255,0,0,2,254,0,0,7,254,0,0,6,6,0,209,224,21,0,84,162,21,0,94,165,251,0,231,238,247,0,14,8,0,0,241,248,0,0,62,39,10,0,178,90,242,0,0,6,244,0,0,252,2,0,0,0,0,0,0,2,254,0,0,0,9,0,94,176,16,0,212,228,246,
0,11,6,1,253,0,0,0,184,0,0,0,206,0,0,0,224,0,0,0,236,0,0,0,248,0,0,0,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,253,0,0,0,245,0,0,0,231,0,0,0,7,0,0,0,186,0,0,0,17,8,4,1,79,235,239,246,0,86,170,11,0,190,97,18,0,7,10,239,0,0,251,2,0,0,255,1,0,0,249,249,0,126,70,234,0,10,4,21,0,131,186,250,0,11,4,252,0,16,8,255,0,234,241,247,0,125,180,
2,0,148,195,9,0,63,36,11,0,69,36,254,0,44,20,248,0,24,12,251,0,2,0,0,0,227,241,3,0,208,239,17,0,95,43,234,0,0,3,246,0,0,255,1,0,0,254,0,0,41,19,249,0,6,6,4,0,254,255,249,0,2,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,253,255,0,0,247,251,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,254,10,0,0,7,248,0,0,2,253,0,
0,255,0,0,0,1,1,0,0,254,6,0,248,242,6,0,190,226,12,0,162,207,4,0,190,218,251,0,251,250,245,0,12,7,1,0,0,0,1,0,239,248,0,0,62,37,11,0,181,90,242,0,0,7,241,0,0,251,2,0,0,0,0,0,0,1,255,0,0,5,7,0,121,180,21,0,177,208,245,0,16,8,0,0,255,0,0,180,0,0,0,186,
0,0,0,222,0,0,0,231,0,0,0,245,0,0,0,253,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,6,0,0,0,21,0,0,0,36,0,0,0,37,0,0,0,90,1,0,0,64,240,248,255,0,65,41,12,0,178,90,239,0,0,5,243,0,0,252,2,0,0,0,0,0,0,1,255,0,0,6,5,0,151,195,21,0,123,183,253,0,4,0,250,0,6,3,1,0,0,0,0,0,253,254,255,0,247,251,0,0,255,1,3,0,
7,4,0,0,13,9,6,0,1,1,0,0,240,245,250,0,253,1,6,0,210,109,247,0,29,22,238,0,0,251,2,0,0,0,255,0,0,6,3,0,123,182,26,0,142,191,246,0,18,8,254,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,247,250,254,0,19,15,10,0,199,102,245,0,19,17,240,0,0,252,2,0,0,1,255,0,0,255,9,0,87,172,26,0,12,1,244,0,45,25,1,0,6,4,4,0,232,240,247,0,
219,240,9,0,210,229,253,0,229,239,251,0,1,255,253,0,10,6,0,0,1,1,1,0,255,255,0,0,241,249,1,0,77,46,10,0,167,84,241,0,0,6,241,0,0,252,2,0,0,0,0,0,0,1,255,0,0,6,3,0,143,192,24,0,126,182,249,0,14,6,254,0,1,1,0,192,0,0,0,166,0,0,0,219,0,0,0,220,0,0,0,235,
0,0,0,250,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
255,0,0,0,251,0,0,0,240,0,0,0,227,0,0,0,10,0,0,0,178,255,0,0,9,15,8,1,55,190,215,245,0,90,173,18,0,166,86,254,0,0,4,245,0,0,252,2,0,0,255,1,0,0,250,251,0,105,62,232,0,23,12,23,0,109,175,7,0,247,251,249,0,9,4,1,0,3,2,1,0,12,7,0,0,0,0,254,0,255,255,0,0,
246,247,247,0,0,0,0,0,7,2,1,0,244,249,248,0,172,217,11,0,113,26,240,0,0,251,0,0,0,0,1,0,0,251,252,0,133,70,235,0,246,251,21,0,155,197,248,0,12,5,252,0,4,3,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,245,249,255,0,18,14,10,0,167,88,251,0,19,20,241,0,0,249,2,0,0,0,0,0,0,4,255,0,222,232,9,0,190,214,244,0,248,249,250,0,170,2,2,0,2,0,252,0,254,
255,255,0,4,1,245,0,7,4,0,0,12,6,1,0,0,0,1,0,0,0,0,0,254,254,255,0,242,250,2,0,100,59,10,0,144,74,236,0,0,3,246,0,0,252,2,0,0,0,0,0,0,1,255,0,0,8,2,0,154,195,25,0,129,183,251,0,13,6,253,0,1,1,1,201,0,0,0,178,0,0,0,229,0,0,0,227,0,0,0,240,0,0,0,251,0,
0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,254,
0,0,0,247,0,0,0,236,0,0,0,223,0,0,0,221,0,0,0,177,0,0,0,208,16,8,0,0,179,208,245,0,96,172,19,0,0,252,12,0,0,5,254,0,0,0,0,0,0,255,1,0,0,248,254,0,82,50,231,0,182,98,255,0,28,19,10,0,243,248,255,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,0,0,3,2,1,0,3,2,1,0,1,1,0,
0,9,5,255,0,153,199,254,0,240,238,16,0,0,5,253,0,0,0,0,0,0,255,2,0,0,0,247,0,143,79,233,0,140,79,5,0,4,6,8,0,244,249,255,0,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,250,253,255,0,239,248,1,0,53,34,12,0,189,100,250,0,51,34,235,0,0,249,1,0,0,0,0,0,0,0,255,0,0,3,4,0,146,201,10,0,250,251,251,0,15,9,1,0,15,7,255,0,13,7,254,0,15,8,0,0,11,7,1,0,4,3,1,0,0,0,0,
0,0,0,0,0,248,252,255,0,250,1,7,0,136,78,9,0,144,79,233,0,0,1,246,0,0,252,2,0,0,0,0,0,0,1,255,0,0,7,2,0,158,198,25,0,121,182,251,0,9,4,250,0,2,1,1,208,0,0,0,177,0,0,0,221,0,0,0,223,0,0,0,236,0,0,0,247,0,0,0,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,253,0,0,0,245,0,0,0,231,0,0,
0,220,0,0,0,215,0,0,0,174,0,0,0,210,15,8,255,0,172,206,245,0,84,168,18,0,0,249,13,0,0,5,253,0,0,0,0,0,0,0,1,0,0,248,1,0,47,32,232,0,197,100,244,0,72,45,11,0,240,248,2,0,253,254,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,3,1,0,232,238,248,0,106,179,
12,0,0,0,10,0,0,2,254,0,0,0,0,0,0,254,2,0,0,251,250,0,104,61,231,0,194,102,250,0,73,44,10,0,247,254,6,0,245,249,255,0,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,249,252,255,0,240,248,1,0,21,15,9,0,140,79,8,0,186,97,237,0,32,22,235,0,0,249,1,0,0,0,0,0,0,0,0,0,0,9,255,0,187,212,20,0,164,204,251,0,12,6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,243,248,255,0,25,18,11,0,
174,94,255,0,108,63,230,0,0,251,250,0,0,254,2,0,0,0,0,0,0,1,254,0,0,7,6,0,150,194,26,0,114,177,252,0,6,0,250,0,3,2,1,210,0,0,0,174,0,0,0,215,0,0,0,220,0,0,0,231,0,0,0,245,228,228,228,253,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,28,28,28,1,0,0,0,4,0,0,0,12,0,0,0,
26,0,0,0,37,0,0,0,45,0,0,0,82,0,0,0,48,241,248,0,0,58,36,10,0,173,88,245,0,12,15,238,0,0,249,2,0,0,0,0,0,0,0,0,0,0,4,254,0,0,0,9,0,148,197,20,0,134,189,253,0,247,247,248,0,11,7,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,241,248,1,0,104,61,10,0,139,77,
235,0,0,254,247,0,0,252,2,0,0,0,0,0,0,0,0,0,0,5,254,0,0,0,8,0,197,221,14,0,146,198,6,0,193,220,253,0,245,248,250,0,8,4,255,0,6,3,0,0,1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,253,253,255,0,247,251,
0,0,1,2,4,0,37,24,5,0,92,50,1,0,102,48,243,0,9,16,242,0,0,249,255,0,0,255,2,0,0,0,0,0,0,0,255,0,0,8,255,0,215,227,21,0,77,161,5,0,242,245,248,0,6,3,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,250,253,255,0,247,252,3,0,81,48,9,0,148,74,244,0,14,16,238,
0,0,249,1,0,0,0,1,0,0,0,0,0,0,4,254,0,0,1,8,0,129,186,21,0,142,191,249,0,11,5,252,0,2,1,1,208,0,0,0,174,0,0,0,211,0,0,0,219,0,0,0,230,0,0,0,244,0,0,0,252,228,228,228,255,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,28,28,28,1,0,0,0,4,0,0,0,12,0,0,0,
26,0,0,0,37,0,0,0,41,0,0,0,79,0,0,0,55,244,249,0,0,37,25,10,0,165,86,249,0,38,28,235,0,0,248,0,0,0,0,1,0,0,0,0,0,0,1,255,0,0,7,1,0,215,227,20,0,114,182,7,0,195,219,248,0,12,5,253,0,4,3,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,241,249,1,0,86,52,10,0,157,
79,239,0,0,7,242,0,0,250,3,0,0,0,0,0,0,0,0,0,0,0,255,0,0,6,255,0,0,255,7,0,217,229,12,0,169,214,10,0,183,216,254,0,221,236,253,0,247,248,252,0,3,1,255,0,4,3,255,0,4,2,1,0,2,1,0,0,0,255,255,0,0,1,1,0,254,255,0,0,252,254,255,0,253,253,1,0,252,255,1,0,5,
5,2,0,27,15,5,0,60,34,1,0,81,42,252,0,70,33,243,0,0,11,247,0,0,250,254,0,0,254,2,0,0,0,0,0,0,0,0,0,0,1,254,0,0,8,2,0,197,218,23,0,97,170,2,0,238,243,247,0,9,4,1,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,255,255,0,242,249,1,0,25,17,8,0,127,69,0,0,91,52,
237,0,0,254,248,0,0,252,2,0,0,0,0,0,0,0,0,0,0,6,253,0,0,249,14,0,103,178,17,0,165,203,246,0,15,7,254,0,1,1,1,201,0,0,0,177,0,0,0,215,0,0,0,219,0,0,0,230,0,0,0,244,0,0,0,252,228,228,228,255,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,0,0,0,
0,28,28,28,1,0,0,0,4,0,0,0,12,0,0,0,25,0,0,0,36,0,0,0,35,0,0,0,78,1,0,0,64,246,250,255,0,12,11,9,0,148,81,0,0,77,46,233,0,0,250,252,0,0,254,2,0,0,0,0,0,0,0,0,0,0,6,254,0,0,251,10,0,156,203,18,0,139,194,0,0,229,238,247,0,13,6,255,0,3,2,1,0,0,0,0,0,0,0,
0,0,0,0,0,0,241,248,0,0,50,32,9,0,152,79,248,0,41,29,239,0,0,250,253,0,0,254,2,0,0,0,0,0,0,0,0,0,0,0,255,0,0,5,255,0,0,3,3,0,0,247,8,0,212,234,9,0,195,225,5,0,205,230,3,0,221,238,1,0,234,240,251,0,246,249,254,0,0,2,3,0,0,0,253,0,7,4,0,0,24,16,8,0,30,
13,252,0,48,28,0,0,56,28,251,0,52,23,248,0,6,14,247,0,0,0,252,0,0,250,1,0,0,255,1,0,0,0,0,0,0,0,0,0,0,0,255,0,0,7,255,0,0,251,10,0,147,199,19,0,133,187,252,0,253,252,249,0,8,4,1,0,255,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,246,249,255,0,2,5,6,0,88,49,5,0,132,
66,244,0,17,19,240,0,0,249,0,0,0,255,1,0,0,0,0,0,0,0,255,0,0,8,255,0,232,235,18,0,93,173,12,0,200,220,245,0,15,8,0,0,0,0,0,192,0,0,0,178,0,0,0,221,0,0,0,220,0,0,0,231,0,0,0,244,0,0,0,252,228,228,228,255,0,0,0,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,
228,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,1,0,0,0,4,0,0,0,11,0,0,0,24,0,0,0,34,0,0,0,27,0,0,0,75,0,0,0,76,250,253,255,3,252,0,6,0,111,63,7,0,127,65,236,0,0,5,244,0,0,250,3,0,0,0,0,0,0,0,0,0,0,1,254,0,0,7,2,0,240,238,15,0,135,196,12,0,165,205,253,0,241,245,
249,0,13,6,255,0,2,2,1,0,0,0,0,0,0,0,0,0,246,250,255,0,9,8,8,0,111,62,3,0,118,57,240,0,0,11,244,0,0,250,255,0,0,254,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,255,0,0,4,255,0,0,3,1,0,0,254,3,0,0,251,8,0,248,245,252,0,241,252,9,0,255,1,2,0,1,0,254,0,14,2,247,0,9,
12,4,0,0,5,248,0,0,2,254,0,0,254,254,0,0,252,0,0,0,254,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,6,255,0,0,1,6,0,205,222,14,0,125,190,10,0,195,218,246,0,12,5,254,0,3,2,1,0,0,0,0,0,0,0,0,0,255,255,0,0,247,251,255,0,254,0,3,0,62,38,8,0,124,64,249,0,54,35,
240,0,0,253,250,0,0,252,2,0,0,0,0,0,0,0,0,0,0,2,254,0,0,6,4,0,185,212,22,0,109,177,1,0,235,240,247,0,11,7,1,253,0,0,0,180,0,0,0,181,0,0,0,229,0,0,0,222,0,0,0,232,0,0,0,245,0,0,0,252,228,228,228,255,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,
228,228,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,1,0,0,0,3,0,0,0,11,0,0,0,20,0,0,0,29,0,0,0,25,0,0,0,70,0,0,0,72,254,255,255,24,242,249,2,0,60,37,9,0,149,77,248,0,35,26,237,0,0,249,254,0,0,255,2,0,0,0,0,0,0,0,0,0,0,4,254,0,0,4,4,0,224,231,16,0,142,
199,10,0,175,209,252,0,240,245,249,0,11,5,255,0,4,3,1,0,0,0,0,0,253,254,255,0,244,250,2,0,39,25,8,0,106,58,255,0,98,46,242,0,0,11,245,0,0,252,254,0,0,252,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,254,0,0,1,1,0,0,2,255,0,0,0,0,0,0,0,0,0,0,0,1,0,
0,253,255,0,0,255,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,254,0,0,5,0,0,0,254,8,0,211,226,12,0,143,199,8,0,179,212,252,0,253,252,250,0,10,6,1,0,0,0,0,0,0,0,0,0,254,255,255,0,246,250,0,0,1,3,4,0,56,32,6,0,114,60,252,0,69,38,241,0,0,4,247,0,0,250,
1,0,0,0,1,0,0,0,0,0,0,0,255,0,0,7,255,0,0,248,13,0,123,189,16,0,151,197,251,0,3,0,250,0,7,3,1,232,0,0,0,184,0,0,0,186,0,0,0,231,0,0,0,227,0,0,0,236,0,0,0,245,0,0,0,253,228,228,228,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,3,0,0,0,9,0,0,0,16,0,0,0,25,0,0,0,34,0,0,0,50,0,0,0,70,0,0,0,48,245,249,255,0,12,10,8,0,121,67,2,0,106,56,238,0,0,4,246,0,0,250,1,0,0,0,1,0,0,0,0,0,0,0,255,0,0,5,255,0,0,2,6,0,227,232,13,0,154,
205,9,0,173,211,254,0,227,238,251,0,5,1,253,0,8,5,0,0,2,1,1,0,249,252,255,0,248,253,3,0,39,24,6,0,88,47,1,0,88,44,245,0,28,21,247,0,0,3,249,0,0,253,255,0,0,252,1,0,0,255,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
255,0,0,1,0,0,0,3,255,0,0,3,0,0,0,0,4,0,249,242,9,0,183,223,13,0,164,207,3,0,194,220,251,0,250,250,250,0,11,6,0,0,1,1,1,0,255,255,0,0,250,253,255,0,247,252,2,0,15,10,5,0,64,38,4,0,103,51,251,0,62,35,241,0,0,5,247,0,0,250,1,0,0,255,1,0,0,0,0,0,0,0,0,0,
0,4,254,0,0,4,4,0,201,220,21,0,115,183,5,0,209,225,246,0,14,7,254,0,1,1,1,208,0,0,0,186,0,0,0,206,0,0,0,222,0,0,0,231,0,0,0,240,0,0,0,247,228,228,228,253,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,2,0,0,0,5,0,0,0,14,0,0,0,25,0,0,0,32,0,0,0,32,0,0,0,69,0,0,0,68,253,254,255,8,244,250,2,0,58,36,9,0,133,69,249,0,52,35,240,0,0,253,250,0,0,251,2,0,0,0,0,0,0,0,0,0,0,0,255,0,0,5,255,0,0,3,3,0,248,
240,10,0,174,217,12,0,169,211,3,0,204,226,253,0,242,246,251,0,5,2,254,0,7,4,0,0,252,254,0,0,248,252,2,0,16,11,5,0,53,31,3,0,74,38,253,0,65,32,250,0,35,19,248,0,0,8,251,0,0,1,251,0,0,255,0,0,0,0,0,0,0,254,255,0,0,0,255,0,0,0,1,0,0,2,1,0,0,0,0,0,0,1,0,
0,0,255,5,0,0,249,3,0,232,242,9,0,200,228,7,0,185,220,3,0,195,222,255,0,227,238,250,0,2,255,253,0,10,6,0,0,0,0,0,0,250,253,0,0,248,252,1,0,6,5,5,0,36,22,4,0,78,42,0,0,89,45,248,0,34,24,243,0,0,1,249,0,0,250,1,0,0,255,1,0,0,0,0,0,0,0,0,0,0,1,254,0,0,7,
1,0,245,240,15,0,127,193,13,0,161,202,252,0,254,251,249,0,9,6,1,248,0,0,0,188,0,0,0,187,0,0,0,224,0,0,0,224,0,0,0,231,0,0,0,242,0,0,0,251,228,228,228,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,1,0,0,0,5,0,0,0,11,0,0,0,20,0,0,0,28,0,0,0,24,0,0,0,57,0,0,0,65,0,0,0,44,246,249,255,0,3,6,7,0,88,50,4,0,123,62,245,0,24,22,241,0,0,251,253,0,0,252,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,
254,0,0,6,1,0,0,252,7,0,223,234,11,0,182,219,8,0,186,219,1,0,211,230,254,0,238,244,251,0,255,254,254,0,2,1,255,0,252,254,255,0,255,1,3,0,15,10,5,0,30,17,1,0,42,24,3,0,40,19,252,0,36,22,3,0,21,4,240,0,18,14,7,0,1,1,2,0,255,255,255,0,240,242,247,0,235,
252,16,0,222,238,254,0,217,235,3,0,215,234,254,0,226,239,0,0,238,244,251,0,0,254,253,0,5,4,255,0,1,0,1,0,251,253,0,0,253,0,2,0,12,7,5,0,35,22,3,0,62,33,0,0,78,40,250,0,56,27,246,0,0,10,246,0,0,253,254,0,0,251,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,255,0,0,7,
0,0,0,248,11,0,154,204,15,0,144,195,0,0,227,237,248,0,13,7,255,0,2,1,1,212,0,0,0,191,0,0,0,199,0,0,0,232,0,0,0,228,0,0,0,236,0,0,0,245,0,0,0,251,228,228,228,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
228,228,228,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,1,0,0,0,3,0,0,0,8,0,0,0,16,0,0,0,23,0,0,0,30,0,0,0,29,0,0,0,62,0,0,0,63,254,255,255,20,243,249,1,0,21,15,8,0,96,53,3,0,109,53,243,0,17,19,242,0,0,253,253,0,
0,251,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,255,0,0,4,255,0,0,3,1,0,0,252,7,0,236,239,9,0,202,231,7,0,197,225,4,0,206,229,255,0,225,239,0,0,236,244,251,0,246,250,255,0,251,251,253,0,252,255,0,0,255,0,2,0,4,1,255,0,4,2,0,0,9,8,6,0,2,2,2,0,255,255,0,0,248,249,
249,0,252,253,255,0,254,255,0,0,0,255,254,0,2,2,2,0,3,2,255,0,2,1,2,0,6,6,4,0,15,8,2,0,27,16,1,0,42,23,1,0,57,30,254,0,58,28,249,0,38,20,248,0,0,9,248,0,0,254,253,0,0,251,0,0,0,255,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,254,0,0,6,1,0,0,248,10,0,170,212,15,0,
146,198,3,0,209,227,248,0,9,4,253,0,6,3,1,236,0,0,0,193,0,0,0,194,0,0,0,227,0,0,0,226,0,0,0,233,0,0,0,240,0,0,0,248,0,0,0,253,228,228,228,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,2,0,0,0,5,0,0,0,12,0,0,0,19,0,0,0,27,0,0,0,24,0,0,0,40,0,0,0,60,0,0,0,59,253,254,255,7,244,250,1,0,24,17,8,0,87,47,1,0,103,51,246,0,29,24,244,0,0,0,250,
0,0,250,0,0,0,255,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,4,255,0,0,3,0,0,0,1,3,0,0,250,5,0,245,245,5,0,222,242,8,0,219,234,255,0,223,244,10,0,228,238,252,0,232,242,252,0,242,253,11,0,245,250,254,0,247,248,252,0,243,244,246,0,0,0,0,0,9,9,7,0,11,10,7,
0,9,4,2,0,12,5,251,0,18,7,248,0,31,21,13,0,29,10,243,0,37,23,4,0,35,15,248,0,23,14,251,0,0,7,249,0,0,1,253,0,0,253,255,0,0,252,1,0,0,255,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,5,255,0,0,3,2,0,251,243,11,0,166,214,14,0,157,203,1,0,209,225,249,0,5,3,253,
0,8,4,1,249,0,0,0,197,0,0,0,196,0,0,0,216,0,0,0,232,0,0,0,229,0,0,0,237,0,0,0,244,0,0,0,251,228,228,228,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,1,0,0,0,3,0,0,0,8,0,0,0,15,0,0,0,19,0,0,0,26,0,0,0,17,0,0,0,50,0,0,0,57,0,0,0,55,252,254,255,4,245,250,1,0,16,11,7,0,68,39,2,0,96,49,250,0,63,32,245,0,0,9,247,0,
0,253,254,0,0,252,1,0,0,255,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,255,0,0,1,255,0,0,3,0,0,0,2,0,0,0,1,3,0,0,255,255,0,0,255,5,0,0,254,4,0,0,252,0,0,0,253,254,0,253,249,250,0,0,0,0,0,3,5,4,0,0,4,4,0,0,3,0,0,0,3,254,0,0,1,250,0,0,2,255,0,0,0,255,0,0,
254,255,0,0,254,0,0,0,253,1,0,0,255,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,4,255,0,0,4,1,0,0,253,7,0,222,232,11,0,165,211,8,0,172,211,0,0,222,234,249,0,7,3,254,0,8,4,1,252,0,0,0,201,0,0,0,199,0,0,0,206,0,0,0,239,0,0,0,230,0,0,0,237,0,0,0,241,
0,0,0,248,0,0,0,253,228,228,228,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,
228,228,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,2,0,0,0,4,0,0,0,9,0,0,0,15,0,0,0,20,0,0,0,25,0,0,0,14,0,0,0,50,0,0,0,53,0,0,0,51,254,254,255,12,246,251,0,0,1,3,4,0,42,26,6,0,75,40,255,0,77,39,249,0,45,23,246,0,0,8,249,0,0,254,253,0,0,252,0,0,0,254,1,0,0,0,1,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,255,0,0,255,0,0,0,0,0,0,0,255,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,255,0,0,3,255,0,0,3,1,0,0,253,6,0,235,238,10,
0,183,221,8,0,176,214,3,0,201,224,253,0,242,246,250,0,9,5,255,0,6,3,1,244,0,0,0,205,0,0,0,203,0,0,0,206,0,0,0,242,0,0,0,231,0,0,0,236,0,0,0,241,0,0,0,247,0,0,0,252,228,228,228,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,1,0,0,0,1,0,0,0,6,0,0,0,9,0,0,0,15,0,0,
0,19,0,0,0,24,0,0,0,14,0,0,0,39,0,0,0,49,0,0,0,48,255,0,0,30,250,252,255,0,248,252,1,0,9,7,5,0,42,25,4,0,65,35,0,0,65,32,250,0,51,24,249,0,11,13,249,0,0,4,251,0,0,255,253,0,0,254,0,0,0,253,0,0,0,255,1,0,0,255,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,1,0,0,0,0,0,0,0,3,255,0,0,2,0,0,0,2,2,0,0,254,2,0,0,247,7,0,213,236,8,0,195,226,7,0,188,220,3,0,205,227,253,0,236,243,250,0,5,1,254,0,8,5,0,0,2,1,1,226,0,0,0,208,0,0,0,207,0,0,0,217,
0,0,0,242,0,0,0,232,0,0,0,237,0,0,0,241,0,0,0,247,0,0,0,250,0,0,0,255,228,228,228,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,1,0,0,0,2,0,0,0,5,0,0,0,9,0,0,0,15,0,0,0,18,0,0,0,21,0,0,0,18,0,0,0,17,0,0,0,45,0,0,0,43,0,0,0,43,255,255,255,18,
249,253,0,0,249,252,1,0,5,4,3,0,22,14,4,0,41,23,2,0,49,26,255,0,48,26,254,0,41,17,247,0,29,15,0,0,8,7,247,0,0,6,2,0,0,1,250,0,0,0,254,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,0,0,251,0,0,0,251,4,0,230,243,
5,0,220,239,6,0,211,234,4,0,205,229,2,0,211,230,254,0,227,240,254,0,245,248,252,0,5,2,254,0,7,4,0,0,3,2,1,238,0,0,0,213,0,0,0,213,0,0,0,211,0,0,0,239,0,0,0,238,0,0,0,235,0,0,0,238,0,0,0,241,0,0,0,247,0,0,0,251,0,0,0,254,228,228,228,255,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,
0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,1,0,0,0,2,0,0,0,5,0,0,0,8,0,0,0,13,0,0,0,16,0,0,0,18,0,0,0,18,0,0,0,8,0,0,0,23,0,0,0,40,0,0,0,39,0,0,0,37,0,0,0,27,252,254,255,0,250,252,0,0,251,254,1,0,3,2,2,0,10,7,3,0,18,10,2,0,
21,13,0,0,29,18,7,0,20,4,242,0,26,20,10,0,18,10,2,0,12,4,251,0,7,0,249,0,9,0,245,0,0,0,255,0,252,0,6,0,245,0,13,0,248,254,3,0,241,249,4,0,232,238,243,0,233,246,7,0,232,245,3,0,230,239,250,0,236,247,2,0,243,246,251,0,251,253,255,0,3,0,254,0,6,4,0,0,5,
2,1,0,1,1,0,229,0,0,0,219,0,0,0,217,0,0,0,216,0,0,0,233,0,0,0,248,0,0,0,238,0,0,0,238,0,0,0,240,0,0,0,243,0,0,0,248,0,0,0,251,0,0,0,254,228,228,228,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,28,28,28,2,0,0,0,5,0,0,0,7,0,0,0,9,0,0,0,12,0,0,0,14,0,0,0,16,0,0,0,17,0,0,0,7,0,0,0,8,0,0,0,34,0,0,0,32,0,0,0,31,0,0,0,29,0,0,0,27,255,255,255,5,253,254,0,0,252,255,0,0,253,253,0,0,254,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,254,0,0,0,3,0,254,0,3,1,0,0,3,3,0,0,4,1,0,0,2,2,1,251,0,0,0,229,0,0,0,227,0,0,0,225,0,0,0,224,0,0,0,222,0,0,0,248,0,0,0,249,0,0,0,239,0,0,0,240,0,0,0,242,0,0,0,244,0,0,0,247,0,0,0,249,0,0,0,251,228,228,228,254,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,254,0,0,0,249,28,28,28,250,0,0,0,246,0,0,0,241,0,0,0,236,0,0,0,9,0,0,0,11,0,0,0,12,0,0,0,13,0,0,0,232,0,0,0,210,
0,0,0,4,0,0,0,0,0,0,0,21,1,1,1,23,4,3,0,22,4,0,0,20,0,4,0,18,0,0,0,16,0,0,255,13,0,0,0,12,0,0,0,9,0,0,0,7,0,0,0,4,0,0,0,0,0,0,0,252,0,0,0,249,0,0,0,247,0,0,0,244,0,0,0,243,0,0,1,240,0,0,0,238,0,0,0,236,0,0,0,234,0,0,0,233,0,0,0,235,0,0,0,0,0,0,0,252,
0,0,0,244,0,0,0,243,0,0,0,243,0,0,0,244,0,0,0,245,0,0,0,247,0,0,0,249,0,0,0,252,0,0,0,253,228,228,228,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,255,255,255,0,0,0,0,0,28,28,28,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,3,0,0,0,4,0,0,0,6,0,0,0,8,0,0,0,7,0,0,0,8,0,0,0,8,0,0,0,9,0,0,0,11,0,0,0,9,0,0,0,8,0,0,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,249,0,0,0,248,0,0,0,247,0,0,0,245,0,0,0,247,0,0,0,248,0,0,0,248,0,0,0,249,0,0,0,248,0,0,0,250,0,0,0,252,0,0,0,253,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,228,228,228,0,0,0,0,0,4,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,252,0,0,0,252,0,0,0,249,0,0,0,244,0,0,0,3,0,0,0,4,0,0,0,4,0,0,0,5,0,0,0,6,0,0,0,6,0,0,0,5,0,0,0,5,0,0,0,4,0,0,0,4,0,0,0,3,0,0,0,3,0,0,0,2,
0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,254,0,0,0,254,0,0,0,253,0,0,0,253,0,0,0,252,0,0,0,252,0,0,0,251,0,0,0,251,0,0,0,250,0,0,0,250,0,0,0,251,0,0,0,252,0,0,0,252,0,0,0,253,0,0,0,255,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,254,0,0,0,251,0,0,0,0,0,0,0,1,0,0,0,
2,0,0,0,2,0,0,0,3,0,0,0,2,0,0,0,2,0,0,0,2,0,0,0,2,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,255,0,0,0,255,0,0,0,255,0,0,0,254,0,0,0,254,0,0,0,254,0,0,0,254,0,0,0,253,0,0,0,254,0,0,0,254,0,0,0,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,255,255,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,129,0,126,255,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,255,255,88,234,20,149,130,13,78,186,0,0,0,0,73,69,78,68,
174,66,96,130,0,0 };

const char* juce_icon_png = (const char*) temp_binary_data_25;

//================== projectIconAndroid.png ==================
static const unsigned char temp_binary_data_26[] =
{ 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,128,0,0,0,128,8,6,0,0,0,195,62,97,203,0,0,0,25,116,69,88,116,83,111,102,116,119,97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,3,134,105,84,88,116,88,77,76,
58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,112,97,99,107,101,116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,32,60,
120,58,120,109,112,109,101,116,97,32,120,109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,54,45,99,48,49,52,32,55,57,46,49,53,
54,55,57,55,44,32,50,48,49,52,47,48,56,47,50,48,45,48,57,58,53,51,58,48,50,32,32,32,32,32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,34,104,116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,
57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,
116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,109,109,47,34,32,120,109,108,110,115,58,115,116,82,101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,
115,84,121,112,101,47,82,101,115,111,117,114,99,101,82,101,102,35,34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,34,32,120,109,112,77,77,58,79,114,105,103,
105,110,97,108,68,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,51,51,101,53,51,101,51,102,45,98,98,100,52,45,52,48,99,99,45,98,54,100,55,45,53,100,52,100,102,52,50,56,99,56,52,54,34,32,120,109,112,77,77,58,68,111,99,117,109,101,
110,116,73,68,61,34,120,109,112,46,100,105,100,58,70,55,67,50,56,48,65,66,52,67,55,56,49,49,69,52,57,54,50,67,65,49,51,66,54,69,53,52,48,69,51,54,34,32,120,109,112,77,77,58,73,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,70,55,
67,50,56,48,65,65,52,67,55,56,49,49,69,52,57,54,50,67,65,49,51,66,54,69,53,52,48,69,51,54,34,32,120,109,112,58,67,114,101,97,116,111,114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,67,32,50,48,49,52,32,40,77,97,
99,105,110,116,111,115,104,41,34,62,32,60,120,109,112,77,77,58,68,101,114,105,118,101,100,70,114,111,109,32,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,54,49,99,99,53,50,57,52,45,98,55,101,57,45,52,56,
55,55,45,97,57,99,56,45,97,57,51,98,52,50,101,98,51,53,99,49,34,32,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,61,34,97,100,111,98,101,58,100,111,99,105,100,58,112,104,111,116,111,115,104,111,112,58,97,99,101,101,57,57,101,101,45,57,52,
100,101,45,49,49,55,55,45,97,53,100,98,45,56,53,99,49,100,48,98,53,54,97,53,50,34,47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,32,60,47,114,100,102,58,82,68,70,62,32,60,47,120,58,120,109,112,109,101,116,97,62,32,60,63,120,
112,97,99,107,101,116,32,101,110,100,61,34,114,34,63,62,105,25,181,0,0,0,27,230,73,68,65,84,120,218,236,93,9,152,84,213,149,62,111,173,181,215,42,122,111,26,186,217,183,113,101,208,108,24,220,141,163,95,76,102,81,99,28,77,226,56,147,113,190,108,126,73,
116,162,209,44,102,49,209,9,137,243,229,251,208,153,104,150,81,38,49,147,56,137,74,32,46,76,132,0,138,128,44,13,13,205,210,44,13,93,213,85,175,150,87,111,155,115,238,123,175,170,104,186,161,193,234,166,170,121,87,31,85,253,170,222,171,251,238,249,207,
122,207,61,151,179,44,11,188,118,238,54,222,27,2,15,0,94,243,0,224,53,15,0,94,243,0,224,53,15,0,94,243,0,224,53,15,0,94,243,0,224,181,115,163,137,165,186,209,120,68,20,57,142,203,255,150,105,154,236,149,231,121,118,140,87,63,220,62,224,47,129,97,20,250,
64,231,157,207,56,107,156,194,171,133,190,148,1,0,198,225,113,217,96,171,170,58,69,215,141,171,240,196,85,56,0,17,60,183,22,95,95,150,101,233,37,81,20,237,47,18,117,198,168,19,68,220,92,46,55,9,251,112,13,190,197,62,240,109,150,101,108,193,62,172,144,
36,233,55,120,104,99,220,135,146,63,80,217,75,0,66,58,113,124,38,147,185,193,178,184,199,4,65,152,50,244,183,117,93,123,26,65,112,183,207,231,75,141,37,1,176,15,23,153,166,245,148,32,136,243,134,246,193,48,244,255,21,4,254,211,129,64,96,247,120,128,160,
20,18,160,98,0,144,78,167,47,64,226,255,17,137,95,117,226,111,217,3,145,205,166,126,138,131,127,27,130,192,202,203,233,18,246,1,137,63,197,48,172,87,81,210,180,15,247,188,244,157,108,54,189,18,127,255,122,191,223,159,41,117,31,198,2,0,101,111,4,210,67,
234,186,206,227,192,223,207,243,5,226,11,188,15,100,193,135,164,231,217,24,211,88,200,178,255,22,69,81,110,55,240,203,199,33,163,4,125,64,177,47,160,216,127,20,57,63,79,124,158,147,236,62,112,66,158,9,176,15,31,196,62,220,98,89,102,69,168,128,138,240,
2,144,160,181,72,132,75,242,157,230,68,56,146,92,15,239,28,254,57,146,222,68,2,240,108,240,145,56,32,138,242,55,147,73,165,107,56,67,204,53,212,78,117,12,185,134,35,245,131,182,199,109,120,255,15,187,12,45,240,50,36,213,189,176,233,224,83,160,106,3,121,
16,32,72,201,56,92,162,170,57,186,214,242,0,80,130,134,4,104,194,193,172,102,156,78,60,143,4,95,179,247,235,240,187,109,31,131,45,135,158,4,73,144,28,35,209,4,20,189,13,8,152,71,179,217,12,211,193,46,65,137,136,154,166,17,39,243,72,204,73,217,108,182,
21,143,57,120,204,192,163,9,207,213,224,103,36,109,216,125,138,68,172,133,159,119,34,129,31,33,160,217,0,20,240,126,42,188,178,235,179,240,251,237,119,192,238,216,239,64,226,69,7,0,204,88,157,140,247,242,159,83,110,224,24,171,129,99,72,194,52,190,13,
90,76,220,115,48,181,254,26,216,27,95,9,235,247,127,15,90,107,222,11,147,194,243,65,51,84,246,253,96,48,116,131,162,36,239,66,99,237,25,36,218,249,72,144,25,120,44,196,235,166,225,221,66,72,211,73,248,42,227,109,106,200,126,196,191,19,100,223,225,113,
20,255,78,34,0,54,160,107,183,5,137,185,30,129,115,16,239,243,168,44,251,26,108,161,194,129,136,146,102,221,190,165,208,27,123,25,162,161,121,208,92,181,16,12,7,52,212,144,248,241,64,192,159,243,0,80,162,134,150,245,81,77,211,55,162,8,94,194,44,126,83,
131,185,77,183,195,254,193,87,96,71,255,127,195,159,122,191,10,215,206,126,6,129,33,34,247,25,204,47,247,251,3,143,34,211,127,65,16,184,201,2,233,6,199,36,40,150,240,174,150,64,96,212,217,127,231,109,54,114,241,232,243,24,222,99,64,20,165,46,247,187,
146,32,195,161,196,91,240,230,129,31,48,85,116,113,251,189,8,130,217,144,115,192,71,146,6,1,240,103,148,68,38,94,83,246,238,96,217,171,0,71,183,27,200,149,75,105,112,137,251,73,68,147,1,182,168,227,43,80,227,239,132,61,3,47,193,166,190,101,72,28,177,
200,24,147,131,120,116,34,24,138,64,206,136,154,63,108,27,145,43,250,219,58,206,94,64,125,94,135,247,232,114,213,8,137,126,205,200,192,27,8,56,69,237,131,105,209,27,97,118,227,205,160,153,154,3,84,1,226,241,88,31,222,231,57,188,174,34,162,43,21,97,3,
144,33,134,3,250,63,170,154,125,202,61,167,163,14,142,134,102,192,69,237,159,99,70,224,134,3,143,161,97,184,9,197,179,47,15,2,58,232,51,2,139,232,120,13,62,209,135,64,177,15,50,228,68,60,232,125,241,103,244,93,226,238,161,238,45,1,236,237,131,63,70,209,
191,2,234,2,211,96,209,228,251,17,62,130,13,72,148,58,154,150,131,195,135,15,61,210,220,220,180,185,98,198,182,18,226,0,212,79,228,126,43,149,74,181,226,219,21,168,143,103,185,196,37,222,124,113,251,157,168,10,158,131,206,200,117,112,237,172,103,144,
24,68,64,142,125,70,220,153,206,29,66,139,125,63,30,251,64,201,237,135,108,46,134,190,3,26,132,122,130,129,67,18,194,120,141,4,33,185,9,194,114,27,84,251,39,67,216,215,2,1,49,138,159,163,88,183,128,169,142,195,201,55,225,55,239,124,4,178,104,245,95,54,
237,113,152,223,124,27,168,186,234,24,127,60,244,244,236,122,62,28,14,125,188,165,165,37,69,30,128,227,5,148,117,28,160,98,66,193,52,192,162,40,30,80,148,212,127,74,146,252,77,27,116,38,139,7,44,234,184,31,142,40,27,152,81,182,249,208,83,48,183,249,99,
176,47,190,22,14,196,95,129,254,212,102,136,103,118,34,161,98,104,168,229,144,152,90,62,68,192,57,97,2,114,37,109,58,241,76,42,8,156,15,66,8,128,250,192,12,104,172,186,8,218,106,63,0,213,190,201,40,250,31,134,148,122,16,102,54,252,13,204,105,188,5,245,
190,150,239,91,50,153,76,231,114,234,143,162,209,142,4,130,83,196,115,102,37,132,131,43,65,2,48,181,175,170,26,160,107,119,15,250,249,15,224,224,214,23,127,33,32,251,96,115,223,47,96,229,174,123,192,47,214,67,80,110,68,162,119,131,97,230,152,222,230,
57,217,145,22,220,40,98,67,22,251,207,180,116,52,232,114,236,189,95,172,67,137,208,6,3,233,109,76,50,124,120,222,11,248,218,148,55,252,138,172,255,183,13,67,255,52,74,129,215,36,73,226,188,72,96,137,136,159,205,170,33,85,205,61,45,203,254,199,209,208,
202,19,95,96,226,219,7,7,6,223,132,3,137,215,81,119,7,64,55,82,16,79,119,227,103,50,234,245,42,60,23,100,250,220,142,24,142,102,192,88,164,129,93,79,170,129,238,97,160,189,17,75,111,103,247,49,77,29,182,247,47,71,85,114,212,137,2,22,134,208,231,243,47,
64,245,244,187,116,58,125,23,218,3,174,248,231,60,9,112,134,125,115,130,48,45,186,110,252,4,57,127,73,129,153,56,28,124,25,137,208,15,111,238,127,28,182,30,249,25,90,231,73,70,176,177,247,74,12,180,43,82,104,4,206,128,11,90,255,5,102,52,252,53,227,35,
2,137,139,88,219,21,84,31,242,251,125,15,58,243,18,99,226,14,78,228,201,32,54,96,170,170,214,107,154,241,2,138,211,69,5,159,93,96,81,183,221,3,47,195,255,237,121,16,57,115,43,35,188,27,138,29,183,240,52,18,156,212,68,103,228,67,112,233,148,175,162,59,
218,158,87,9,46,8,80,101,125,35,20,10,222,135,30,204,152,168,131,137,10,0,242,250,104,206,221,143,98,255,89,228,252,235,221,113,99,162,28,31,122,195,254,199,88,4,144,8,64,98,255,44,70,41,144,232,73,36,126,23,44,238,250,30,180,215,189,23,61,139,28,128,
19,173,52,12,3,208,117,253,116,85,85,248,135,104,192,150,28,4,19,18,0,206,236,31,164,211,153,111,161,181,127,111,222,88,65,125,79,81,219,213,187,191,204,44,125,137,15,13,225,122,11,69,115,134,121,6,28,11,215,6,28,189,255,238,27,121,14,186,35,226,201,
238,32,207,163,184,233,102,154,73,161,15,116,61,10,51,39,221,136,30,71,1,4,154,166,169,248,255,181,53,53,53,43,121,59,117,201,244,0,112,242,254,88,104,68,221,200,243,226,115,28,103,71,99,200,146,199,79,224,149,157,159,71,125,255,52,14,118,117,222,133,
115,221,56,210,205,237,181,151,177,216,124,60,179,11,93,194,151,144,112,166,125,237,187,104,186,153,69,139,191,21,166,214,95,137,132,247,195,222,216,42,244,6,182,14,145,60,200,237,150,74,166,35,44,158,246,125,116,19,111,66,73,80,136,15,100,50,233,110,
158,231,22,135,195,225,62,206,166,154,229,1,96,132,7,202,100,50,205,134,97,190,230,198,223,201,202,22,121,9,57,255,1,120,179,111,41,248,208,42,63,222,176,182,112,240,53,22,149,59,191,245,238,124,172,127,235,225,229,240,106,207,231,153,27,119,166,146,
128,244,124,125,112,38,92,57,115,25,190,118,178,115,233,92,12,86,116,255,19,236,139,175,60,65,253,80,156,129,98,8,87,207,122,18,38,215,46,62,206,38,72,38,19,203,80,21,124,2,141,194,146,169,130,9,229,6,186,162,31,45,254,251,4,161,104,242,5,137,191,245,
240,207,96,99,223,19,104,249,135,79,240,170,72,52,79,10,205,135,121,77,159,192,247,38,139,204,229,12,29,102,53,124,132,205,18,26,200,193,103,46,250,117,152,223,252,73,136,132,58,217,125,233,8,200,117,204,250,39,55,209,26,34,205,233,28,169,131,63,238,
252,28,74,161,61,44,204,156,143,85,4,130,119,42,138,114,45,69,52,203,42,192,86,78,157,65,171,127,1,207,11,119,184,204,65,49,249,99,233,29,176,166,247,107,44,84,59,28,39,147,232,15,72,147,80,231,75,140,96,238,57,98,142,144,220,156,159,219,63,19,3,143,
236,14,10,15,155,102,177,84,0,22,104,98,118,192,48,82,143,226,14,9,117,15,155,161,68,217,148,79,86,65,79,134,192,123,63,170,55,191,7,128,17,184,223,48,172,79,34,0,2,110,215,104,0,215,238,125,4,82,218,17,198,93,195,53,10,221,146,78,78,170,125,44,54,64,
3,46,33,113,84,61,13,135,149,55,25,17,207,212,19,53,81,164,247,37,214,224,111,216,70,40,217,19,50,154,20,135,146,111,160,148,81,142,11,2,21,3,135,130,71,61,199,94,128,29,253,191,100,18,204,85,145,126,127,96,17,2,224,131,142,20,224,60,0,20,53,180,150,
155,208,96,186,201,253,155,178,124,246,199,95,133,61,177,23,79,26,224,33,194,208,68,207,107,61,247,194,64,166,155,233,252,100,110,31,254,253,37,56,150,218,124,130,197,126,58,141,60,137,45,135,150,193,219,7,127,138,162,61,201,116,252,206,163,47,34,40,
191,237,24,151,220,200,209,68,4,199,219,125,255,14,89,54,225,36,184,82,0,109,46,248,123,148,116,101,19,29,44,19,35,208,2,69,73,221,140,62,255,79,221,96,172,192,139,240,251,109,119,64,207,192,11,142,238,63,149,181,158,1,191,20,129,176,220,2,25,237,40,
164,114,125,142,145,246,238,198,154,212,10,233,122,202,59,160,56,196,96,182,7,207,25,142,68,58,249,51,107,70,10,62,56,253,223,96,118,195,223,50,131,208,201,110,62,102,24,218,194,104,52,218,243,110,35,132,19,194,8,180,3,38,164,100,185,107,92,98,145,190,
63,150,222,6,7,18,171,81,132,142,46,208,67,196,166,233,221,163,200,245,25,173,159,233,226,82,72,89,34,58,249,254,131,232,90,210,124,128,27,11,24,21,221,240,231,187,251,127,197,50,152,200,126,33,30,65,47,32,130,210,110,17,5,137,60,21,224,26,86,134,142,
190,29,55,223,5,180,128,111,246,197,87,161,30,31,56,173,16,47,17,75,68,95,253,204,245,254,200,148,36,85,66,199,233,184,148,34,231,135,35,104,135,196,51,61,96,39,38,89,44,107,8,1,255,151,100,243,120,0,200,3,192,108,71,253,63,155,249,253,44,168,98,194,
193,196,27,224,196,129,42,182,17,120,85,61,142,198,232,122,4,15,151,151,120,178,44,47,204,100,50,210,57,5,128,66,222,189,251,119,65,135,33,0,230,81,214,87,97,208,6,89,52,143,175,112,0,56,97,42,230,165,20,155,72,232,233,76,65,53,208,238,100,13,13,51,70,
227,103,35,138,227,65,120,154,25,211,117,141,67,66,79,71,15,168,3,57,157,167,245,22,60,207,145,33,180,11,63,159,227,206,149,144,197,156,206,29,6,213,136,179,208,106,165,55,146,98,137,108,47,24,166,238,216,1,22,218,1,114,227,224,96,188,37,155,205,244,
90,22,119,190,101,153,17,2,8,142,71,22,199,101,11,170,137,163,180,208,213,78,128,29,219,204,98,113,172,9,159,205,170,211,80,223,221,138,127,211,106,222,153,232,30,213,81,124,220,94,76,105,162,69,108,190,133,95,143,56,15,107,7,132,140,65,80,181,152,157,
77,97,233,21,13,0,211,201,73,164,9,37,87,165,9,130,104,73,146,252,117,77,51,171,112,44,230,161,68,144,136,233,157,49,217,173,105,250,59,56,10,203,101,89,250,47,84,23,153,34,107,214,170,8,0,56,179,96,68,252,219,241,1,31,17,69,169,177,152,192,5,113,39,
68,220,92,255,194,124,191,253,90,229,107,103,65,158,50,79,168,57,181,125,99,230,208,61,173,179,163,130,32,185,25,196,124,93,93,253,251,139,159,219,29,19,28,143,169,248,22,15,235,58,85,213,110,199,113,252,108,32,16,160,133,42,28,140,65,98,73,201,227,0,
14,241,121,36,254,247,241,97,238,113,185,189,56,64,66,134,30,203,188,179,70,202,155,180,179,169,198,83,23,142,169,29,96,89,35,154,91,44,53,213,93,92,202,102,53,205,161,17,210,24,122,73,119,132,195,161,231,157,233,228,124,166,113,217,205,6,82,127,200,
167,79,165,210,247,161,136,251,218,241,126,186,143,125,174,234,73,22,98,101,57,123,98,21,51,142,220,185,118,119,72,136,91,76,83,133,9,211,136,179,57,255,16,194,243,44,218,73,30,15,197,47,232,153,69,62,204,242,12,105,82,203,205,94,182,25,42,167,32,8,46,
175,174,174,94,83,60,157,92,150,211,193,233,116,250,189,136,246,151,81,175,249,221,197,156,34,45,167,74,174,99,179,122,148,190,77,41,218,62,177,30,26,171,46,132,217,13,183,226,235,121,160,25,118,18,5,37,121,82,8,248,165,29,159,2,137,5,115,42,187,81,248,
152,212,217,135,230,60,139,42,45,96,47,93,99,113,10,19,118,29,251,53,236,60,250,60,196,50,59,152,170,160,239,181,215,46,134,57,141,31,131,160,52,9,180,162,60,195,76,38,253,103,73,18,175,8,133,66,131,174,42,40,171,117,1,238,26,122,180,242,31,148,36,193,
239,250,244,68,252,77,7,159,132,55,122,31,98,72,167,164,10,82,3,233,92,63,244,43,27,97,103,255,47,225,146,41,15,194,220,166,219,28,16,0,147,16,228,9,72,66,104,66,0,192,14,73,23,210,218,12,51,13,175,244,124,30,118,244,47,119,194,222,182,173,147,202,29,
132,3,131,175,34,40,126,13,75,166,47,133,104,120,62,232,44,167,128,60,7,255,197,138,146,184,211,231,243,125,207,41,133,83,126,70,32,234,171,249,200,249,239,119,165,1,17,127,215,177,23,224,245,221,95,102,15,46,139,181,5,157,207,1,3,131,110,169,240,90,
207,23,33,40,55,192,212,250,171,11,2,146,205,190,73,21,15,0,139,77,43,139,69,207,197,193,107,123,30,128,109,71,126,206,214,27,20,27,185,246,18,182,0,28,75,111,129,21,221,119,195,245,115,150,67,64,106,96,234,128,34,136,104,43,220,156,201,100,126,88,85,
85,85,50,253,88,178,64,16,185,124,168,255,47,224,121,123,254,147,92,30,85,75,192,186,125,223,201,63,220,137,6,159,197,108,1,188,18,191,247,93,102,31,144,173,59,81,119,48,144,120,1,246,15,174,134,109,253,191,64,21,88,59,162,135,67,211,201,199,82,91,96,
243,193,101,200,36,5,18,33,8,166,33,0,186,202,50,18,104,175,220,133,203,10,70,159,0,135,148,117,16,163,69,26,188,255,20,3,67,168,223,10,253,169,141,192,79,224,202,133,164,178,123,99,47,162,10,200,156,114,78,129,230,52,122,227,43,28,166,16,216,248,162,
248,175,49,12,227,2,103,34,137,43,59,0,160,228,143,184,134,9,253,155,84,123,81,7,102,143,75,224,28,97,104,240,90,29,18,153,221,19,152,248,232,210,153,22,139,10,142,102,142,131,36,38,217,4,52,151,192,65,161,252,140,170,170,205,166,105,150,159,4,64,189,
47,32,0,130,199,131,226,244,162,120,148,89,59,129,249,159,249,248,100,237,115,163,102,94,74,120,213,243,188,78,188,133,70,118,109,89,2,160,52,34,137,131,137,223,184,119,203,104,37,93,133,85,74,9,0,94,27,39,207,162,28,1,224,181,113,35,127,73,197,164,7,
128,115,188,121,0,240,0,224,53,15,0,94,243,0,224,53,15,0,94,243,0,224,53,15,0,94,243,0,224,53,15,0,94,243,0,224,53,15,0,94,243,0,224,53,15,0,94,243,0,224,53,15,0,94,243,0,224,53,15,0,94,243,0,112,210,198,121,67,58,214,141,43,79,0,80,197,15,124,209,79,
236,172,229,225,39,255,116,199,239,91,120,234,102,157,144,66,110,89,150,94,202,101,243,37,5,0,213,68,46,62,39,139,225,252,162,134,209,52,218,239,103,226,54,139,85,14,151,133,26,86,197,252,212,223,182,216,106,98,145,173,40,182,138,1,16,231,75,184,124,
170,100,119,114,10,65,236,116,59,107,224,107,52,180,128,173,129,163,37,209,39,125,88,75,103,3,19,9,205,129,137,154,93,78,227,66,235,30,27,194,231,195,104,182,12,160,34,215,145,224,28,8,72,245,78,205,97,182,250,218,224,121,110,47,45,20,45,75,0,96,91,153,
7,128,169,33,65,103,195,212,250,107,33,103,36,78,122,45,237,186,209,21,185,30,234,2,211,89,97,230,137,170,8,104,105,216,180,232,13,172,234,40,85,54,29,233,73,237,42,228,28,204,105,188,21,165,64,190,172,12,213,8,232,71,226,111,40,75,0,216,245,109,248,
183,77,211,56,236,22,177,32,48,44,156,252,37,182,247,30,21,133,24,90,94,157,36,3,157,111,170,90,8,23,79,190,215,94,245,50,129,149,0,149,152,13,251,154,225,210,41,15,177,125,5,52,83,57,193,70,162,122,2,84,71,97,65,203,93,48,5,153,71,55,10,149,66,20,69,
121,43,20,10,31,130,18,214,10,42,233,210,48,73,146,246,235,186,190,172,32,198,52,86,110,253,234,153,63,129,25,147,62,202,186,172,25,10,147,8,244,74,22,209,204,134,191,131,171,103,253,4,130,82,35,219,248,97,162,55,42,130,209,25,185,6,159,249,63,152,138,
164,29,73,72,2,218,99,146,130,0,218,65,151,78,125,24,46,233,120,144,1,134,152,134,184,63,157,78,27,40,1,158,172,169,169,78,59,165,227,74,210,74,90,32,130,42,87,136,162,240,93,85,85,175,243,249,124,127,97,239,244,173,178,93,56,47,159,241,99,86,30,230,
112,114,29,100,181,99,224,151,162,140,243,39,133,207,99,96,214,77,21,206,141,102,177,109,238,39,215,93,198,74,228,244,37,222,96,85,205,169,8,68,149,175,3,90,107,222,195,118,32,163,45,111,221,130,81,196,253,251,247,239,123,38,26,141,254,22,153,172,164,
174,123,73,1,128,29,229,253,126,127,12,69,213,93,185,156,250,130,44,251,35,246,138,88,149,185,51,141,225,11,161,25,31,218,174,137,103,239,201,107,56,187,115,158,107,141,64,32,240,65,182,23,81,87,228,74,71,69,208,120,88,199,109,53,67,250,126,239,222,222,
117,178,44,62,140,0,32,195,65,112,92,238,178,11,4,177,242,101,84,202,44,20,10,174,209,117,237,198,116,58,181,213,222,134,157,103,68,38,46,167,135,179,183,117,81,109,174,231,96,194,148,131,59,93,155,137,109,66,233,140,7,29,244,158,234,35,209,103,36,77,
73,130,246,244,236,122,81,211,114,31,239,232,152,178,139,54,209,114,136,111,149,35,0,220,7,179,4,65,20,170,170,170,94,199,183,75,6,7,227,79,160,254,26,100,110,16,2,129,14,66,181,123,208,90,119,77,211,204,115,11,4,150,69,133,52,93,230,24,58,38,84,1,164,
191,191,191,183,187,123,251,87,124,62,249,214,174,174,174,173,120,94,44,37,231,143,137,10,128,66,253,58,19,59,44,84,87,87,31,204,102,213,127,84,148,228,82,69,73,92,142,24,88,68,251,235,34,24,124,232,45,232,8,140,119,80,74,172,170,169,169,187,188,185,
185,249,134,114,169,161,63,214,156,79,59,198,32,103,127,29,153,36,137,182,210,21,162,40,181,34,35,136,40,53,149,84,42,181,6,223,111,144,101,105,101,71,71,199,238,112,56,76,99,42,20,237,70,110,149,51,0,142,139,12,146,132,241,251,125,28,30,239,32,226,233,
248,65,38,147,169,193,7,149,105,28,106,107,107,148,104,52,146,197,111,79,45,101,213,139,10,8,10,113,200,217,235,35,145,250,95,225,115,63,134,70,115,216,48,116,1,13,104,173,177,177,33,129,118,148,225,108,55,43,56,213,195,141,98,6,171,4,0,184,146,192,114,
42,91,114,244,64,120,0,21,58,164,243,142,43,195,217,53,133,115,194,132,230,250,19,227,249,164,227,133,64,32,64,34,159,98,251,3,238,152,56,149,212,105,60,172,34,194,143,9,241,199,84,2,12,7,134,252,120,216,251,3,59,246,130,64,101,210,135,132,128,185,137,
196,238,246,78,35,110,61,96,171,176,47,128,163,255,205,130,208,228,160,212,70,94,57,0,96,88,64,20,84,133,237,21,185,167,105,128,104,243,39,218,41,204,174,176,93,217,41,11,244,12,52,31,194,234,36,22,213,83,70,66,171,69,134,239,89,243,131,249,179,203,28,
150,187,177,226,234,194,36,146,14,181,129,233,16,9,206,6,221,72,67,229,151,139,87,161,173,246,50,144,120,57,31,213,203,100,210,7,241,181,155,47,131,162,136,101,177,107,152,32,240,155,208,24,74,185,190,177,44,4,225,162,246,47,176,157,191,244,97,226,229,
149,33,249,77,200,234,49,104,174,190,148,21,127,214,205,130,58,207,102,51,61,168,255,123,97,12,234,255,87,130,10,56,1,3,104,16,237,65,119,241,15,248,250,87,110,148,108,114,221,98,184,98,230,143,97,109,239,55,32,150,233,102,49,115,123,172,10,18,129,74,
170,58,246,210,168,126,136,98,238,246,212,52,55,162,192,166,170,166,84,185,116,52,209,73,50,238,168,95,199,207,236,89,172,79,180,215,225,244,232,135,225,61,83,30,98,53,129,41,232,101,111,145,103,0,186,122,191,153,52,137,69,245,248,179,13,128,114,216,
56,146,153,191,138,162,156,199,243,226,42,4,65,109,126,227,104,193,7,170,22,135,67,201,245,112,72,89,203,184,202,182,168,41,124,104,194,246,35,203,33,171,29,29,69,229,77,27,56,51,27,62,202,136,49,82,95,73,36,31,74,172,135,3,131,171,71,181,227,40,137,
247,230,154,69,208,82,189,16,92,55,150,234,30,83,98,75,107,205,251,32,26,156,139,63,43,176,239,49,110,19,69,138,233,175,69,219,247,166,201,147,59,14,56,54,192,89,221,56,178,28,36,128,229,204,33,188,133,156,241,175,72,132,31,184,187,140,144,36,16,145,
203,59,234,151,192,212,200,146,227,125,4,252,103,111,124,21,219,143,71,56,5,0,44,39,181,234,252,214,127,134,186,96,59,140,20,114,160,173,253,54,244,62,1,189,177,21,163,2,0,113,126,123,205,98,88,56,245,30,24,90,20,85,55,237,253,130,44,103,207,35,34,126,
44,22,27,68,160,223,55,115,230,244,253,84,89,117,44,34,123,149,8,0,70,35,28,32,30,245,226,82,69,73,5,125,62,223,131,178,236,11,16,8,104,235,86,211,208,135,136,94,148,156,156,126,218,59,131,211,134,207,57,29,242,28,57,180,249,192,231,204,74,142,150,179,
56,118,47,34,62,197,242,71,146,42,100,232,246,247,31,217,139,199,23,58,59,59,87,138,162,36,56,110,241,89,55,110,202,198,199,34,113,70,83,157,225,112,232,219,104,36,93,157,72,12,190,158,203,229,76,119,16,199,194,98,182,107,240,115,37,127,14,55,166,111,
239,137,172,40,61,61,59,159,139,199,99,55,76,155,54,237,217,80,40,52,102,59,128,85,180,4,112,8,77,209,66,161,182,182,246,213,116,58,125,165,162,36,46,213,52,237,66,228,160,69,40,33,102,213,212,212,206,194,129,229,74,49,116,4,40,228,200,125,193,96,176,
58,16,8,214,148,194,22,178,183,118,201,40,131,131,241,29,170,170,110,70,9,181,17,127,103,77,93,93,205,91,245,245,145,20,79,229,190,109,149,103,121,0,24,57,98,72,220,35,132,195,225,44,114,203,31,116,93,251,3,122,8,144,76,38,23,225,199,175,149,170,207,
4,128,190,190,3,79,181,181,77,94,20,10,133,175,44,197,68,20,221,19,251,185,43,149,82,174,107,106,106,58,134,42,77,163,240,183,19,218,21,156,208,120,217,16,191,220,0,80,12,2,119,67,36,182,83,154,36,201,52,187,168,230,131,71,37,8,14,209,125,252,254,128,
33,203,146,81,202,226,203,130,192,3,114,123,6,165,152,65,132,119,8,110,14,125,70,15,0,167,0,65,145,135,224,196,206,75,111,3,32,184,74,202,141,246,246,175,188,133,154,204,44,138,241,151,117,59,215,215,6,158,243,53,238,189,197,161,30,0,188,230,1,192,107,
30,0,42,83,129,151,198,119,47,117,31,42,41,193,181,98,0,80,180,27,93,190,235,180,233,228,168,175,31,198,117,180,211,213,134,82,139,59,45,240,12,179,187,41,87,148,253,228,1,160,116,0,224,146,232,175,39,88,206,0,219,113,212,7,245,129,89,163,90,78,70,83,
192,126,169,30,124,66,109,126,254,192,201,202,73,227,167,89,55,14,64,47,97,95,235,40,183,172,117,250,16,156,205,22,116,184,231,114,185,220,0,186,128,122,165,72,1,190,66,136,79,254,245,110,211,52,55,185,3,77,75,173,91,106,222,51,170,235,105,214,142,150,
160,133,228,70,54,185,68,247,83,213,172,134,47,180,29,251,106,247,123,134,105,66,67,248,34,8,72,209,252,246,237,35,53,2,94,16,239,55,41,188,128,93,103,3,200,162,100,143,63,249,124,254,108,41,215,239,121,18,0,216,196,141,129,220,187,201,229,86,205,212,
161,179,254,90,182,190,208,94,126,206,13,43,248,137,144,18,31,134,5,205,159,194,63,237,252,11,138,46,34,0,142,250,253,254,237,130,192,111,118,231,242,137,168,181,129,41,48,187,241,102,150,60,50,114,152,192,2,221,72,193,156,198,219,160,202,215,194,126,
131,64,229,44,246,216,226,243,201,149,50,172,149,1,0,34,186,61,115,7,207,35,8,44,123,23,78,131,37,142,126,160,235,59,80,227,239,98,203,204,77,54,247,110,7,247,236,101,87,10,19,249,180,28,155,146,54,116,103,123,122,186,126,112,112,112,85,85,85,213,0,222,
247,207,134,161,239,116,69,182,129,192,186,176,237,51,48,61,122,19,100,245,56,155,238,117,151,181,211,43,253,173,234,131,108,85,243,121,173,119,163,116,177,167,170,237,121,128,196,94,73,146,254,84,202,245,251,30,0,138,212,128,44,203,175,228,114,218,139,
174,122,165,185,251,104,104,46,124,104,206,179,48,11,9,66,105,88,180,216,148,210,180,40,75,168,9,137,78,75,207,231,53,125,156,45,203,102,122,27,137,163,40,137,172,174,231,150,85,87,87,27,72,176,1,195,48,150,186,146,133,150,100,83,90,216,146,233,63,130,
75,58,30,96,54,1,129,72,55,211,12,136,85,254,118,4,212,195,112,217,180,199,17,70,18,3,154,155,234,53,48,48,240,84,52,26,221,67,2,171,98,198,181,12,82,194,70,221,87,20,213,150,162,164,230,241,188,176,10,9,23,117,127,147,229,221,227,107,60,179,11,226,217,
110,38,146,195,114,43,68,130,115,65,20,124,121,226,219,73,167,0,221,221,219,191,209,214,214,250,64,109,45,101,159,89,150,170,170,50,2,107,185,223,31,184,206,189,39,237,216,45,242,34,40,185,126,24,72,191,3,57,148,6,62,177,14,141,190,185,104,75,68,152,
10,114,75,223,96,95,160,183,119,207,106,158,231,62,218,209,209,113,200,241,4,204,241,96,138,115,10,0,116,232,186,102,38,147,202,77,178,236,123,90,146,228,64,97,73,25,199,202,169,240,206,160,80,119,72,167,187,86,191,155,80,178,107,87,247,242,112,56,124,
87,91,91,91,204,89,133,67,24,48,83,169,84,7,226,235,183,8,130,121,246,163,184,64,16,241,190,66,62,125,215,48,13,71,213,184,83,215,34,28,60,216,183,51,153,28,188,101,250,244,233,107,105,42,187,104,29,159,7,128,49,144,88,180,156,12,65,144,188,10,7,127,
105,32,16,156,102,115,246,240,155,42,187,217,68,233,116,90,71,46,253,81,117,117,213,55,145,248,84,198,134,119,103,236,232,158,8,36,83,81,148,14,93,55,30,15,6,67,55,144,170,24,233,158,110,214,143,174,235,148,228,185,50,151,83,191,216,217,217,185,142,146,
89,198,106,17,167,7,128,162,40,13,113,46,234,92,3,65,208,129,162,251,51,40,130,111,66,215,171,205,94,102,198,131,11,8,210,203,217,108,54,29,139,13,172,77,167,83,79,52,54,54,254,58,18,137,228,224,196,213,182,108,77,30,98,192,200,100,50,114,42,149,190,
19,239,245,15,180,146,153,196,187,123,79,59,126,96,144,175,15,137,68,98,91,60,30,123,186,170,42,188,172,165,165,229,176,187,124,123,60,19,62,206,69,0,228,85,129,195,181,6,17,131,196,55,18,110,1,158,127,31,207,139,51,209,170,151,144,59,99,72,252,215,69,
81,220,18,14,7,55,70,34,209,65,36,166,224,72,5,107,24,46,229,156,197,153,196,217,164,18,106,80,106,92,128,18,225,98,188,238,98,36,124,8,15,21,253,252,117,56,240,111,161,11,185,177,190,190,254,128,147,227,199,143,55,241,207,101,0,156,0,4,60,12,167,208,
4,250,247,42,113,50,19,239,62,159,207,36,14,38,181,108,57,54,96,81,152,214,58,153,132,33,135,0,9,110,17,199,227,61,9,20,130,227,137,208,210,109,96,185,137,54,225,79,117,79,15,0,227,164,22,184,226,129,41,238,207,144,216,188,117,166,247,28,250,156,103,
139,240,165,4,64,57,166,132,157,54,246,220,165,230,39,27,156,211,4,232,176,247,44,193,125,203,174,137,229,132,198,114,231,150,74,124,238,9,19,9,244,154,7,0,175,121,0,240,154,7,0,175,121,0,240,154,7,0,175,121,0,240,154,7,0,175,189,235,246,255,2,12,0,158,
137,39,54,252,6,9,64,0,0,0,0,73,69,78,68,174,66,96,130,0,0 };

const char* projectIconAndroid_png = (const char*) temp_binary_data_26;

//================== projectIconCodeblocks.png ==================
static const unsigned char temp_binary_data_27[] =
{ 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,128,0,0,0,128,8,6,0,0,0,195,62,97,203,0,0,0,25,116,69,88,116,83,111,102,116,119,97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,3,134,105,84,88,116,88,77,76,
58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,112,97,99,107,101,116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,32,60,
120,58,120,109,112,109,101,116,97,32,120,109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,54,45,99,48,49,52,32,55,57,46,49,53,
54,55,57,55,44,32,50,48,49,52,47,48,56,47,50,48,45,48,57,58,53,51,58,48,50,32,32,32,32,32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,34,104,116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,
57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,
116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,109,109,47,34,32,120,109,108,110,115,58,115,116,82,101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,
115,84,121,112,101,47,82,101,115,111,117,114,99,101,82,101,102,35,34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,34,32,120,109,112,77,77,58,79,114,105,103,
105,110,97,108,68,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,98,54,49,53,55,56,57,51,45,102,48,97,51,45,52,56,56,55,45,98,99,52,50,45,54,100,50,49,55,51,50,97,99,98,100,97,34,32,120,109,112,77,77,58,68,111,99,117,109,101,110,
116,73,68,61,34,120,109,112,46,100,105,100,58,70,55,67,50,56,48,65,70,52,67,55,56,49,49,69,52,57,54,50,67,65,49,51,66,54,69,53,52,48,69,51,54,34,32,120,109,112,77,77,58,73,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,70,55,67,50,
56,48,65,69,52,67,55,56,49,49,69,52,57,54,50,67,65,49,51,66,54,69,53,52,48,69,51,54,34,32,120,109,112,58,67,114,101,97,116,111,114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,67,32,50,48,49,52,32,40,77,97,99,105,
110,116,111,115,104,41,34,62,32,60,120,109,112,77,77,58,68,101,114,105,118,101,100,70,114,111,109,32,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,54,48,97,49,51,52,53,100,45,97,99,57,50,45,52,48,99,98,
45,97,55,57,53,45,53,100,48,55,55,99,100,56,102,98,99,55,34,32,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,61,34,97,100,111,98,101,58,100,111,99,105,100,58,112,104,111,116,111,115,104,111,112,58,102,54,55,54,101,48,101,99,45,57,52,100,
102,45,49,49,55,55,45,97,53,100,98,45,56,53,99,49,100,48,98,53,54,97,53,50,34,47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,32,60,47,114,100,102,58,82,68,70,62,32,60,47,120,58,120,109,112,109,101,116,97,62,32,60,63,120,112,
97,99,107,101,116,32,101,110,100,61,34,114,34,63,62,104,107,124,19,0,0,42,157,73,68,65,84,120,218,236,125,89,204,44,201,149,86,156,200,204,218,254,125,185,75,187,221,238,118,47,86,187,229,102,140,103,176,133,48,54,242,136,153,198,6,143,152,145,224,101,
120,0,193,3,235,3,210,136,71,94,64,72,60,33,129,16,226,1,36,16,26,48,30,75,192,72,48,48,96,132,133,102,198,96,141,192,99,187,109,26,247,116,247,244,118,125,251,222,255,254,75,85,229,22,135,56,177,101,100,100,100,85,253,203,253,239,149,169,236,206,251,
215,146,85,21,25,231,196,57,223,89,3,16,145,173,143,255,127,15,190,158,130,53,3,172,143,53,3,172,143,53,3,172,143,53,3,172,143,53,3,172,143,53,3,172,143,53,3,172,143,53,3,172,143,53,3,172,143,53,3,172,143,53,3,172,143,53,3,172,143,53,3,172,143,53,3,172,
143,31,199,35,61,207,197,151,204,29,128,199,224,126,209,222,7,0,180,238,203,127,254,24,140,245,194,19,29,220,199,213,50,192,5,137,14,134,113,192,103,32,249,248,161,79,178,156,12,12,38,6,205,107,216,199,160,52,174,112,108,143,96,172,232,141,89,60,54,18,
224,188,171,93,8,193,229,228,209,9,242,49,216,201,165,27,188,198,73,5,75,120,58,57,231,66,158,136,122,32,118,210,185,28,159,29,47,152,49,63,18,6,176,227,244,198,235,152,247,50,146,225,202,24,96,69,53,160,38,144,38,180,174,235,196,158,52,177,138,17,228,
201,224,26,164,44,137,118,73,108,78,147,169,9,47,146,36,169,233,164,199,118,210,237,184,170,170,74,105,156,2,5,87,204,32,240,218,84,1,202,255,18,158,200,1,51,197,164,105,146,86,52,78,203,20,196,8,184,194,228,63,14,42,64,17,223,18,157,38,85,142,10,73,
142,201,231,80,11,145,216,149,118,29,171,74,254,144,34,124,74,4,151,175,201,241,100,244,219,52,185,150,1,44,163,202,191,137,124,5,177,34,137,69,76,81,43,38,184,252,88,33,178,120,193,27,39,67,245,52,97,181,100,2,90,29,88,179,218,205,147,28,43,49,107,221,
243,69,215,43,1,150,113,152,17,247,138,240,52,217,149,20,173,103,211,105,82,78,167,227,162,174,211,186,170,18,97,164,1,49,6,51,248,192,61,166,239,183,140,174,31,235,247,244,151,219,9,176,175,53,207,245,245,173,201,33,81,47,79,181,226,51,121,14,54,54,
166,27,147,73,133,101,153,249,42,194,174,126,81,34,62,56,125,48,41,171,98,80,87,70,114,9,26,171,134,15,4,99,90,48,13,2,200,6,173,137,104,174,111,127,210,190,0,234,118,157,216,87,210,73,174,252,164,26,100,131,98,115,180,53,75,38,156,38,133,46,41,233,26,
122,255,113,199,0,106,50,229,196,233,21,5,80,157,190,255,222,179,247,190,254,95,255,234,176,40,110,75,162,215,114,98,228,93,33,183,196,14,208,150,55,175,230,145,86,23,122,250,132,208,4,215,204,67,74,155,222,160,153,108,72,97,103,21,205,210,146,215,73,
17,32,10,196,244,116,99,227,173,249,231,62,247,143,110,61,251,236,107,101,89,14,136,49,20,195,214,164,150,80,252,207,183,127,235,75,175,102,223,249,133,10,202,140,190,70,254,0,151,239,130,94,162,200,21,32,51,132,148,63,202,220,232,45,127,250,2,29,52,
179,106,197,45,28,115,32,67,163,204,133,252,102,244,185,93,208,183,75,150,21,76,74,160,103,230,207,125,227,75,131,47,255,19,14,28,164,66,34,105,64,146,12,32,96,242,199,14,4,26,9,192,229,74,207,230,156,79,203,55,223,250,137,143,191,254,195,191,48,188,
113,147,228,175,94,213,86,138,248,235,87,248,11,218,220,35,189,166,230,71,104,44,76,138,68,136,230,117,117,134,207,209,49,83,11,103,100,25,123,247,173,183,216,91,123,187,255,251,240,185,231,94,173,202,114,98,129,41,45,198,60,207,211,255,113,244,155,191,
152,124,182,254,18,38,150,84,204,16,185,25,27,178,134,240,24,145,201,150,192,138,125,204,115,193,106,197,57,194,253,171,31,213,222,243,230,42,100,37,43,216,221,163,59,47,126,246,222,31,249,215,251,59,123,71,37,43,83,146,100,6,64,63,214,32,80,77,168,22,
157,2,242,162,24,158,158,158,222,26,78,38,4,0,154,41,35,162,209,141,136,144,80,194,188,111,166,215,39,232,42,56,24,160,45,158,57,52,114,57,149,183,154,36,88,79,103,123,243,60,31,200,65,38,14,225,75,6,152,206,166,27,243,122,182,181,37,198,140,39,137,99,
128,197,206,1,108,189,107,63,97,25,64,223,49,145,53,137,146,159,155,71,150,21,184,250,171,153,78,174,252,209,7,247,239,222,220,217,218,57,150,178,1,60,11,101,33,14,56,47,131,240,243,18,127,201,9,254,89,230,249,240,248,222,189,91,18,130,107,6,80,114,85,
152,191,193,170,181,167,125,191,117,178,46,51,88,134,137,90,244,193,234,119,182,158,4,161,179,217,86,46,49,128,111,161,72,76,194,231,243,249,168,168,139,137,226,27,11,41,12,49,128,249,170,202,190,103,153,139,123,211,232,57,151,88,91,189,97,71,82,132,
143,137,57,154,223,173,202,106,244,163,179,31,221,146,24,22,140,5,101,231,181,119,254,31,186,4,32,238,90,196,97,100,75,59,16,72,43,76,130,62,56,62,190,41,209,152,239,135,107,240,20,176,182,184,238,157,33,140,160,233,64,0,251,224,145,121,146,128,55,210,
33,145,146,7,231,179,77,169,255,83,57,163,41,97,18,163,2,160,40,242,97,89,231,155,250,211,220,49,65,67,118,112,218,155,222,199,128,220,76,189,38,156,36,16,110,133,241,70,178,169,71,150,117,56,211,224,31,29,171,37,74,22,208,103,18,121,93,53,60,134,163,
3,57,66,86,99,195,0,203,104,240,40,49,128,91,249,180,186,20,24,44,138,17,148,229,13,37,126,173,254,119,160,137,27,31,23,70,8,205,218,171,223,49,137,247,60,196,11,161,132,112,147,212,224,13,73,113,198,139,124,163,148,8,127,40,213,131,21,171,100,44,22,
101,57,172,69,62,162,143,113,67,22,22,89,247,22,3,180,89,18,205,123,22,5,128,195,139,194,176,0,51,234,160,145,40,86,70,112,163,10,192,92,43,12,227,36,163,25,206,14,200,12,37,48,234,153,206,87,106,10,94,57,8,180,196,39,114,228,243,249,166,68,87,135,108,
48,136,172,116,225,144,122,148,248,190,56,183,79,29,40,244,153,163,71,10,216,223,114,76,39,167,148,152,112,62,27,151,53,249,38,82,237,19,38,6,144,159,47,36,94,169,235,124,192,148,46,78,220,106,102,29,91,211,39,178,207,28,202,149,99,222,105,88,64,175,
105,171,76,184,34,51,184,199,168,228,70,109,164,4,56,86,146,175,242,122,240,64,28,223,46,139,50,133,33,192,195,242,157,92,105,52,208,227,80,168,201,25,52,159,111,236,205,102,55,64,24,244,206,152,71,192,112,165,135,160,207,94,239,49,8,196,66,37,184,140,
35,205,119,146,123,5,217,124,122,54,34,19,208,119,85,211,123,211,179,233,164,204,139,180,81,0,220,145,73,191,194,189,213,155,184,43,154,43,193,187,190,253,73,31,71,64,199,178,176,114,0,140,218,209,167,24,215,252,100,247,232,86,89,84,169,189,56,22,167,
120,156,172,0,23,248,33,9,160,214,79,81,108,60,41,196,142,182,0,124,9,16,172,216,16,189,163,181,22,176,193,13,125,8,28,3,88,208,97,113,51,169,160,197,110,49,155,141,201,65,5,156,23,88,85,41,89,236,169,92,112,243,233,108,67,200,201,182,68,99,17,35,144,
49,8,188,63,141,36,0,3,227,26,213,33,2,182,64,143,196,232,49,68,3,49,193,103,130,177,196,205,7,229,97,121,191,76,6,91,89,45,180,15,196,2,237,43,115,5,95,181,4,80,171,138,233,65,226,108,58,221,29,15,135,35,103,1,56,244,207,26,27,222,71,254,76,120,86,0,
107,94,67,140,16,24,34,82,0,187,224,17,253,155,37,169,148,143,42,35,1,188,89,147,216,112,62,193,90,36,74,107,152,21,223,252,215,144,81,159,150,88,73,71,54,132,114,192,146,148,71,95,15,109,140,70,34,36,10,13,212,59,211,249,116,2,74,3,60,28,21,112,238,
124,128,85,204,13,37,172,229,133,167,247,238,221,20,116,47,44,4,114,254,138,143,96,128,216,114,118,12,195,34,234,194,251,188,197,12,78,255,155,147,27,243,74,2,83,105,161,12,204,189,184,168,159,52,3,55,144,139,132,5,171,189,249,183,199,143,239,44,132,
70,38,160,49,233,52,145,107,143,228,216,90,255,93,38,169,157,68,73,228,127,243,42,223,60,41,78,118,110,177,155,247,125,226,247,249,2,46,98,29,164,87,141,1,84,100,77,254,229,114,48,156,76,64,105,110,181,137,107,29,227,61,132,111,89,122,62,94,192,104,180,
111,249,160,132,2,128,134,43,165,48,42,135,162,170,6,228,82,37,151,181,242,195,203,119,165,25,56,22,3,76,26,147,15,60,129,31,88,20,10,188,181,29,63,208,130,137,224,76,60,112,6,37,122,127,125,241,239,51,81,99,95,164,242,191,138,149,155,211,100,186,37,
37,141,16,218,20,228,87,237,13,188,50,63,128,23,239,151,115,174,252,1,60,41,138,3,72,120,162,1,32,116,23,56,176,136,13,207,226,204,129,6,16,162,240,152,2,26,53,18,115,1,211,99,231,131,160,172,31,114,72,85,35,172,156,4,176,193,22,9,87,202,137,180,19,19,
203,156,237,149,15,173,149,222,118,4,249,6,161,240,220,64,214,3,80,59,66,163,89,237,13,62,96,158,12,176,108,165,177,4,169,151,26,196,86,206,102,59,114,160,4,46,18,59,230,171,244,5,92,185,4,80,186,138,194,87,69,49,16,69,113,168,137,198,218,78,159,69,32,
30,123,196,191,93,205,44,32,246,66,201,0,129,223,64,210,191,172,7,181,148,0,194,34,106,5,75,16,36,46,24,51,51,118,112,235,53,198,2,161,107,8,60,15,64,155,53,152,51,252,186,66,31,220,231,218,113,5,240,216,71,240,98,227,180,62,57,20,149,28,23,87,57,11,
124,65,124,249,209,50,128,231,170,84,147,88,228,249,104,243,236,236,118,34,234,246,10,5,232,70,81,156,137,7,17,32,231,233,117,31,12,58,128,24,56,129,124,75,2,188,248,130,121,67,148,213,72,142,109,172,135,172,179,147,72,122,229,243,124,34,198,164,2,176,
133,247,99,132,247,191,175,97,22,116,104,95,56,51,182,45,77,66,196,239,107,127,29,60,178,207,180,75,168,218,40,39,39,195,147,3,81,10,128,1,176,199,217,12,132,150,175,154,36,128,100,128,39,165,4,200,162,129,26,108,3,64,183,124,2,85,225,175,244,80,228,
199,176,65,139,9,172,25,233,71,138,145,149,101,49,146,99,155,160,183,146,36,22,72,103,243,217,182,52,15,1,23,71,155,60,190,133,32,12,212,126,7,156,59,8,3,198,129,206,149,109,220,224,161,133,93,41,3,118,230,135,85,94,67,182,145,182,98,45,143,165,25,200,
156,103,141,137,217,108,182,185,145,101,155,29,34,70,221,189,184,192,255,143,241,232,160,79,124,17,9,20,33,118,178,53,76,86,210,64,72,113,239,231,40,170,228,149,178,216,176,33,105,244,162,246,24,209,83,16,121,13,35,134,105,243,105,136,176,81,236,21,108,
185,151,211,36,101,165,40,246,243,34,247,29,87,87,42,1,174,42,26,232,59,41,56,229,223,77,31,60,216,175,132,24,183,137,236,173,220,142,7,208,243,20,10,236,218,241,214,132,108,49,14,152,107,49,194,80,97,44,65,94,10,202,79,49,170,139,98,34,172,67,69,210,
187,150,19,91,150,213,38,5,94,172,0,71,47,74,135,45,63,63,182,200,212,228,8,180,125,82,93,243,49,100,34,223,239,199,188,200,98,35,17,18,41,160,79,139,147,253,89,61,27,201,239,18,222,60,63,186,104,224,2,198,176,129,32,121,111,92,176,227,227,3,185,180,
134,141,62,6,79,103,131,97,61,8,124,2,126,50,8,235,81,5,49,215,49,182,177,132,85,23,192,26,23,52,145,150,48,159,16,25,86,210,18,208,181,0,148,111,198,84,6,147,168,71,196,76,244,209,218,69,244,109,176,183,54,154,217,18,85,120,209,65,240,174,110,51,9,70,
88,169,27,205,4,207,114,208,191,87,155,239,38,83,112,202,166,187,57,159,79,228,239,29,251,56,139,18,68,30,9,6,88,192,4,54,10,40,241,11,199,52,207,247,229,72,135,154,240,193,106,116,22,19,182,147,229,124,191,191,37,96,104,226,133,207,25,107,71,21,133,
199,4,190,235,217,74,25,174,28,68,216,198,44,218,180,66,47,109,195,129,70,230,39,171,112,71,40,193,218,49,139,70,22,216,199,194,187,170,29,93,180,65,229,230,119,88,139,161,172,18,210,190,128,122,183,100,210,68,85,214,181,112,150,128,113,6,224,181,51,
192,50,39,16,61,166,120,123,149,231,55,229,40,7,45,179,205,183,207,125,80,216,151,38,17,234,241,142,254,15,128,160,207,16,16,16,95,185,152,41,176,202,201,55,128,177,220,58,160,84,69,23,205,111,86,40,186,216,128,136,234,244,246,170,110,171,17,63,73,172,
9,28,135,247,10,45,121,97,175,32,75,160,132,124,119,90,158,237,144,122,234,36,41,60,134,126,0,181,186,10,201,0,131,211,211,155,153,20,173,206,17,19,66,165,150,184,14,99,248,61,160,80,193,75,17,1,135,44,194,12,44,154,91,128,148,124,201,208,47,76,81,191,
202,19,46,180,203,66,40,231,13,26,2,104,145,44,90,115,223,228,5,98,43,35,0,13,147,120,104,39,80,3,177,36,179,208,82,64,15,160,1,203,119,102,187,167,249,233,30,74,129,175,150,83,19,113,125,108,204,192,86,190,154,74,5,171,42,126,43,207,111,140,41,254,62,
24,4,121,0,158,52,16,145,212,173,168,69,224,185,117,67,19,208,254,21,162,241,37,132,94,65,39,1,72,203,144,46,7,132,8,91,130,118,10,57,18,162,209,199,220,169,2,104,37,134,176,78,90,72,179,230,109,182,95,44,183,16,91,57,4,139,19,197,196,141,106,43,63,158,
239,138,66,50,225,16,90,85,86,87,17,15,184,10,9,128,190,35,136,68,43,229,215,237,50,182,155,182,34,125,204,115,246,88,175,94,232,2,198,136,103,16,219,42,193,247,10,10,11,44,133,39,129,61,241,109,167,72,104,203,130,60,170,2,76,153,88,171,80,148,84,2,175,
53,70,21,164,119,61,243,168,29,200,133,78,36,18,156,211,167,201,7,110,16,129,240,254,245,213,75,104,68,134,150,133,253,158,100,156,192,188,154,238,215,105,37,89,55,99,143,212,17,212,99,110,128,23,15,80,232,244,236,248,120,71,226,128,45,37,254,67,127,
62,195,120,52,23,252,247,33,78,124,22,152,135,16,132,151,25,246,184,147,133,147,30,2,84,21,129,159,94,162,109,17,174,242,242,229,215,217,172,221,70,212,131,193,1,126,148,176,137,236,181,215,51,235,24,140,93,236,223,198,6,200,48,240,30,248,40,33,149,198,
224,209,252,232,176,28,84,217,128,13,106,3,2,163,206,160,71,22,13,116,49,0,26,148,52,1,241,244,116,143,178,111,123,245,63,243,60,116,128,11,192,96,219,142,111,28,133,24,137,43,132,110,98,214,85,3,142,51,98,197,21,96,106,180,208,100,232,177,86,188,142,
123,46,27,107,8,134,153,40,216,210,248,221,149,47,162,176,48,166,0,26,48,73,190,128,19,60,57,168,152,202,12,170,188,112,240,227,3,2,61,7,5,85,92,86,105,89,110,39,66,108,40,147,75,96,220,85,230,86,38,196,131,57,173,252,62,214,117,238,56,135,17,118,189,
140,49,151,176,41,40,145,75,93,89,96,17,95,28,210,112,107,79,5,160,151,203,135,65,120,8,58,33,98,214,97,134,152,7,0,61,117,129,29,227,49,156,29,50,5,51,150,243,249,65,205,235,76,10,167,57,21,174,250,25,194,143,60,28,108,137,111,147,65,9,138,213,121,46,
33,0,78,90,182,56,11,252,0,78,8,219,52,241,112,229,138,174,104,87,34,223,83,9,78,5,68,174,117,165,101,230,185,202,74,18,26,54,176,118,9,182,98,8,165,7,192,84,232,212,46,40,203,61,188,207,29,54,239,50,0,180,18,69,177,229,51,20,129,15,177,43,45,186,46,
100,116,4,226,44,199,252,32,47,231,19,57,215,39,118,158,213,72,56,191,180,47,224,42,37,128,26,84,85,215,0,84,13,84,148,27,206,2,96,145,192,15,4,240,123,217,109,68,227,255,145,194,17,196,168,62,117,140,192,129,117,145,28,49,19,9,47,222,98,0,77,10,174,
214,63,56,127,128,232,129,131,208,114,11,55,214,64,83,246,17,2,193,198,96,20,81,68,96,199,127,186,117,114,99,90,78,183,228,101,239,217,175,191,42,53,112,85,126,0,176,174,213,92,50,192,222,108,118,176,83,85,137,75,7,95,74,88,22,215,229,225,155,34,32,46,
122,82,1,35,249,6,126,45,129,181,70,168,168,20,64,120,225,96,197,17,4,2,229,233,106,249,124,146,10,151,221,139,173,76,158,118,13,145,104,71,242,90,114,0,91,198,97,168,2,250,204,64,97,236,143,234,137,124,47,127,123,190,1,34,90,135,252,248,56,130,168,17,
3,73,128,177,16,187,19,235,180,233,211,83,177,202,202,208,137,227,219,250,190,217,136,30,46,104,121,5,35,137,34,162,157,76,2,186,49,68,29,168,54,146,254,74,185,212,174,110,79,123,255,173,21,32,92,181,80,147,193,195,188,156,224,38,227,39,4,134,194,201,
133,182,85,208,118,6,55,242,202,247,67,106,150,73,54,147,73,81,23,91,140,106,68,224,106,35,184,87,202,0,105,154,86,85,81,76,230,69,177,71,213,184,206,22,103,16,120,249,32,48,34,177,29,11,112,129,21,240,69,116,4,220,137,254,0,17,250,17,196,134,113,80,
215,123,139,144,49,149,47,128,219,198,37,162,55,80,10,65,0,215,127,206,189,28,31,108,49,6,122,254,127,110,234,134,181,143,49,81,255,250,30,0,93,29,132,206,244,84,133,162,233,209,236,232,32,57,224,212,217,228,241,115,5,91,48,149,14,6,5,20,197,126,125,
255,254,45,151,149,139,16,184,61,61,100,222,237,156,96,136,203,61,43,193,3,141,246,9,247,164,128,167,71,155,149,110,65,36,52,140,80,202,53,76,64,16,20,2,100,16,250,94,73,137,165,146,52,73,97,114,115,209,65,64,238,224,31,247,8,207,61,18,129,115,29,183,
5,156,112,129,32,108,73,132,134,41,218,30,67,95,82,52,242,69,240,58,189,47,238,61,145,14,210,202,6,129,174,170,79,192,85,73,0,234,195,35,104,98,135,27,27,179,227,170,122,239,248,251,223,101,195,209,144,85,9,176,20,74,198,135,114,122,192,107,10,97,45,
10,82,19,94,226,38,114,179,142,44,19,216,212,110,47,128,164,249,7,91,36,68,79,244,107,39,105,173,153,66,212,146,182,37,43,79,24,251,222,221,154,205,182,54,41,77,153,181,170,8,229,192,210,52,17,213,73,193,126,248,59,111,176,233,248,73,38,117,153,179,120,
192,148,124,89,162,0,54,13,75,156,33,227,20,128,69,105,96,74,190,161,65,245,96,50,133,193,4,151,104,14,140,17,196,141,165,68,133,9,20,233,77,228,153,166,53,203,179,41,131,19,156,111,139,157,59,73,154,170,198,53,87,217,36,226,74,24,64,117,222,74,18,65,
137,160,135,59,59,103,243,95,248,249,191,253,141,217,233,116,242,245,95,255,249,237,147,7,27,48,76,24,121,90,7,146,40,169,92,133,148,39,152,214,149,90,145,116,130,17,229,42,76,3,232,12,3,110,68,189,75,156,128,32,140,192,65,137,79,97,90,13,208,227,130,
94,179,126,166,12,84,0,37,145,218,104,42,56,123,117,244,28,43,14,14,155,196,63,151,134,32,9,53,72,48,159,113,246,157,175,254,46,59,59,253,145,252,109,147,165,47,172,86,215,42,200,117,180,177,134,143,145,50,142,36,158,196,3,69,76,112,230,51,79,36,59,36,
244,90,194,146,36,97,60,77,229,95,206,210,76,62,151,143,179,20,212,88,137,240,48,146,191,206,231,236,134,56,248,225,207,236,255,209,127,240,185,79,126,254,223,212,80,13,83,158,86,126,131,171,71,206,0,166,131,21,245,175,169,133,196,0,117,81,12,159,124,
250,153,215,55,254,250,47,253,210,15,94,250,196,111,28,253,139,127,254,87,118,95,251,193,75,116,179,37,200,27,150,211,159,16,19,212,146,41,104,117,202,149,198,5,157,52,217,77,214,47,168,83,18,15,26,226,235,244,13,52,140,0,170,105,11,49,2,5,108,106,208,
127,21,3,128,126,142,169,252,141,129,156,212,148,147,149,199,70,35,149,174,174,64,160,87,101,161,86,36,197,136,74,90,177,146,9,54,242,169,126,205,165,162,27,181,36,12,225,233,127,98,12,97,180,60,232,80,178,213,66,78,73,16,225,85,248,89,45,113,121,143,
92,159,9,173,116,164,222,116,42,237,75,210,93,62,23,74,16,170,86,73,212,10,104,6,245,203,227,79,253,219,63,254,252,23,255,254,39,158,255,196,183,48,195,140,24,222,118,57,123,100,12,208,151,122,164,24,64,8,78,64,112,62,155,141,54,228,12,190,252,39,190,
252,149,55,94,248,216,119,222,253,202,47,255,197,244,235,255,249,79,110,204,231,195,98,144,169,90,17,34,54,77,0,40,7,77,77,9,79,70,92,107,228,174,141,51,116,117,122,224,199,146,204,138,66,131,35,104,229,19,209,133,33,124,165,164,130,124,46,185,7,229,
9,114,181,161,4,254,167,148,20,106,156,40,30,118,209,205,162,136,1,114,57,166,105,198,178,50,109,28,61,20,56,168,193,180,51,224,13,67,8,179,242,91,185,195,38,175,31,26,0,8,134,131,137,17,56,141,69,50,62,167,21,15,230,76,168,106,217,148,155,113,26,123,
201,246,202,189,55,190,240,225,207,255,227,207,191,248,185,175,236,221,216,191,63,199,249,230,16,134,185,156,219,82,158,4,4,109,32,235,250,163,129,177,31,48,3,225,102,66,69,154,101,101,53,159,143,234,179,179,237,15,63,251,236,107,155,127,249,175,253,
205,255,251,210,203,223,124,227,171,255,242,47,141,223,124,243,121,98,245,138,100,157,226,250,68,225,0,205,0,194,121,245,192,76,8,216,136,157,239,125,3,237,203,85,30,94,221,8,74,25,108,36,178,137,17,42,208,82,64,80,15,0,186,78,254,158,208,61,165,232,
26,227,1,228,141,208,166,0,17,117,135,155,201,21,121,170,165,148,91,209,106,72,202,110,144,175,37,77,74,155,11,99,24,67,145,235,215,141,91,193,136,44,77,120,245,48,49,12,144,104,41,144,82,159,43,121,61,53,133,227,153,46,166,225,85,82,190,180,241,226,
127,122,229,229,159,253,135,31,123,250,99,255,139,15,56,159,229,179,205,209,104,52,247,87,189,109,109,123,237,174,224,165,129,160,54,46,80,83,149,159,158,110,13,229,253,191,240,133,47,252,234,219,207,60,253,253,255,243,181,175,253,249,242,183,126,243,
149,52,159,143,107,201,4,164,175,33,105,162,117,232,217,242,104,171,122,125,107,16,108,121,161,169,161,1,45,31,84,131,5,90,253,160,171,241,20,67,40,169,160,171,131,40,20,76,147,94,83,199,5,174,76,190,198,29,204,53,58,171,230,82,21,205,19,195,0,90,191,
43,198,36,98,41,237,196,155,128,144,195,39,122,124,164,18,180,176,50,4,7,45,246,173,148,160,239,35,92,161,108,10,185,242,185,122,174,27,160,81,103,149,109,177,253,214,103,158,248,244,63,251,236,203,127,232,107,219,59,219,39,243,122,190,157,85,89,49,28,
14,115,95,220,251,249,151,87,145,22,118,101,158,64,211,30,206,53,135,148,167,107,107,82,228,249,176,144,0,241,224,230,237,119,199,127,246,207,253,221,239,62,251,220,239,188,255,31,127,237,23,241,157,119,62,10,89,38,137,150,105,164,140,198,68,50,113,2,
221,170,13,156,65,32,116,63,39,183,242,104,130,85,69,178,245,198,131,6,130,104,56,6,141,116,208,86,161,96,3,73,204,68,163,123,193,77,219,88,133,97,40,207,130,114,67,167,200,210,194,212,251,218,213,45,140,3,8,161,43,238,13,16,5,51,64,110,254,170,215,204,
169,138,82,233,229,84,3,64,200,64,73,2,165,248,51,65,76,80,63,51,126,250,191,125,246,99,159,253,167,207,63,243,252,183,37,143,164,167,179,211,237,129,52,169,165,184,87,139,139,230,211,180,184,21,87,9,0,47,141,1,76,211,95,213,23,144,106,238,203,178,74,
147,36,203,7,131,100,38,81,1,225,239,1,34,31,152,199,195,60,47,134,35,62,16,191,239,167,127,246,95,189,253,236,11,223,250,238,127,248,181,63,253,222,127,255,230,43,229,131,211,61,202,213,115,170,213,160,109,97,136,106,165,110,109,124,243,104,175,1,237,
50,65,99,42,106,149,224,187,27,192,101,34,209,42,219,28,103,236,176,170,49,161,150,177,205,100,74,11,134,250,69,73,140,157,151,76,228,247,181,3,19,27,192,103,138,134,53,168,99,154,224,10,213,27,6,179,14,36,213,190,208,72,1,83,114,216,48,1,17,63,53,152,
64,157,136,187,233,205,239,125,234,169,207,252,202,167,158,255,201,127,191,179,187,123,44,88,149,14,178,108,54,28,14,230,131,193,80,174,254,65,158,73,117,58,144,127,37,67,148,85,197,228,66,42,229,67,9,20,60,12,227,99,129,107,197,0,38,50,149,120,253,128,
217,91,111,126,255,51,103,211,119,127,138,38,177,174,203,148,218,173,18,7,35,82,235,211,154,83,119,14,20,117,178,179,151,229,47,124,233,19,191,119,240,28,251,193,230,131,119,63,173,165,40,120,125,164,154,174,89,194,133,2,180,238,179,126,91,52,177,0,97,
106,191,117,126,47,119,94,99,69,36,133,80,181,190,124,235,251,119,216,59,239,207,51,205,59,220,69,16,36,14,17,69,57,7,44,19,182,191,243,69,54,220,60,144,120,212,239,83,96,124,19,104,204,65,104,208,191,243,118,66,195,48,204,72,45,213,29,141,162,207,220,
4,161,236,95,234,88,87,227,241,110,181,245,157,122,246,52,251,214,171,245,207,213,226,78,146,101,4,242,18,42,89,54,221,77,185,52,249,168,175,113,130,147,81,122,252,201,151,247,191,126,235,214,225,187,52,159,182,103,192,101,37,194,101,84,0,232,40,171,
94,253,146,246,245,241,241,221,15,205,230,223,248,59,47,190,184,253,7,40,46,104,21,55,116,243,168,228,220,230,144,102,227,34,249,242,103,164,77,80,68,242,37,35,25,192,173,148,41,108,133,94,219,233,229,205,119,56,129,45,5,209,111,127,243,183,79,127,229,
43,27,95,221,63,188,241,122,93,85,67,219,140,89,160,72,63,241,194,139,191,254,214,15,171,151,118,119,255,198,167,39,79,236,178,170,16,14,132,106,16,215,78,85,176,104,159,247,100,175,117,194,16,230,111,109,60,213,242,115,27,105,42,94,185,147,23,63,39,
102,10,92,96,171,118,6,209,19,184,0,101,49,173,238,124,240,206,223,251,51,127,106,231,111,85,53,31,74,194,39,118,252,143,12,3,152,46,219,170,223,94,41,165,231,124,126,116,251,240,240,254,199,119,119,7,114,186,242,36,172,125,235,122,212,139,52,244,200,
118,163,65,225,99,236,186,127,91,225,64,17,92,111,211,136,18,182,247,196,232,238,31,126,229,103,126,249,137,39,158,124,171,40,230,19,41,94,11,149,196,90,22,195,207,252,228,31,252,47,119,126,111,240,210,171,111,12,63,125,231,237,148,44,82,150,112,67,108,
174,25,128,27,141,194,205,107,182,251,12,231,77,63,10,223,8,112,94,75,239,142,185,112,89,132,105,157,179,45,132,145,6,181,61,241,44,203,112,101,185,157,189,251,126,254,242,116,122,58,206,6,155,204,52,183,182,18,224,145,96,0,151,157,74,162,189,40,80,130,
191,179,221,167,158,34,157,47,22,16,147,7,132,1,214,223,232,39,70,112,8,158,139,86,206,88,60,3,79,55,118,154,205,78,111,28,61,120,231,5,196,159,248,182,49,255,172,73,37,168,67,200,187,119,222,127,177,78,114,249,218,86,139,189,200,158,1,209,196,180,208,
147,0,126,225,147,107,71,100,53,5,118,178,14,58,181,172,216,243,188,85,80,109,44,14,1,147,221,179,179,187,27,219,233,100,234,183,141,243,51,132,175,179,83,168,151,9,132,82,13,148,233,201,201,209,237,209,104,56,8,186,63,120,39,247,102,128,179,158,182,
95,145,213,31,4,147,122,94,195,160,218,174,253,219,137,20,201,103,27,167,103,239,125,172,170,106,181,114,236,61,200,199,213,217,217,116,251,238,253,147,23,43,41,167,57,196,162,127,241,54,199,173,108,53,22,228,167,176,254,10,182,190,130,167,152,159,13,
141,148,41,203,122,235,232,232,120,223,118,15,245,210,241,47,92,27,120,217,216,162,107,10,73,82,224,244,244,254,109,196,140,175,0,29,122,147,32,250,51,70,98,24,0,23,126,115,231,151,121,197,78,78,239,124,100,122,54,31,81,235,117,35,193,72,153,214,71,15,
142,14,167,179,252,80,136,126,181,26,118,177,109,21,158,134,4,93,20,169,142,84,183,177,24,150,240,152,139,238,109,150,215,155,71,15,170,125,80,37,142,34,185,138,172,160,115,49,128,14,110,112,102,99,210,141,10,208,187,155,36,201,217,129,22,132,98,9,161,
25,91,220,249,121,213,215,241,92,140,38,237,106,246,193,253,187,79,158,156,28,239,167,73,90,218,213,67,238,235,119,222,121,247,185,178,170,55,161,87,250,53,76,192,88,123,117,219,52,69,33,186,43,93,68,138,160,23,181,72,238,83,13,186,127,53,108,76,103,
124,151,20,87,51,247,2,44,93,46,130,5,46,37,1,108,103,112,13,4,171,44,73,138,27,109,88,17,3,126,200,122,82,132,47,65,112,92,202,14,90,2,36,236,228,248,248,214,241,233,209,33,165,128,121,25,182,236,238,221,15,158,169,107,49,114,197,69,44,168,92,143,172,
112,198,186,125,47,93,55,123,219,27,51,36,244,2,105,16,147,8,232,1,65,206,113,50,159,215,251,36,165,172,249,125,217,20,177,203,90,1,182,122,82,228,69,190,9,188,184,5,48,97,77,181,76,44,11,8,88,127,239,108,140,228,138,225,2,203,0,34,168,127,193,120,229,
236,87,101,53,40,139,98,72,155,68,200,147,235,110,97,85,146,23,229,152,186,134,51,172,52,209,132,243,252,234,64,96,80,73,226,170,209,124,180,111,26,146,9,47,161,41,180,208,195,174,120,97,79,43,92,176,62,242,42,27,31,159,242,3,33,42,176,221,247,46,171,
6,46,106,5,128,143,64,201,65,83,149,249,104,103,187,184,193,249,100,9,83,226,5,69,253,42,175,133,12,17,209,225,214,102,4,151,7,100,34,194,80,11,105,165,35,152,40,67,172,182,196,210,222,75,79,180,197,79,206,63,32,60,66,251,153,240,208,15,242,132,136,79,
65,8,129,235,106,59,59,155,237,222,168,170,156,243,100,16,109,25,115,109,45,98,154,150,165,218,29,39,7,53,185,253,132,216,53,49,160,208,35,179,210,10,93,110,5,96,143,201,24,182,114,137,247,142,213,33,6,114,56,114,219,121,179,201,9,80,161,8,29,143,8,197,
111,88,151,18,195,6,171,232,248,206,247,198,246,195,192,136,124,180,210,134,143,37,139,78,14,165,197,197,205,214,119,215,11,2,187,156,171,76,64,21,161,57,59,123,176,55,25,103,99,182,50,46,239,179,247,217,2,245,192,86,120,157,245,56,159,140,249,218,20,
141,182,182,237,210,161,132,166,208,52,172,67,69,182,92,111,163,151,161,46,206,1,246,98,86,69,167,200,9,181,51,106,54,47,247,102,179,249,68,199,183,46,223,51,232,162,126,0,103,108,171,166,16,82,115,206,102,199,210,132,74,135,113,162,138,5,43,27,151,156,
140,245,215,215,135,44,0,11,153,66,237,63,69,241,61,14,162,227,97,80,82,193,110,91,21,89,173,94,91,99,29,231,104,254,210,235,181,5,125,94,177,178,16,109,70,232,35,190,8,190,27,89,91,106,160,87,211,114,124,86,239,157,77,197,38,249,133,130,29,90,174,215,
19,216,50,1,149,230,59,145,22,0,203,206,183,130,151,217,253,231,5,125,184,80,226,24,149,21,239,14,66,73,173,216,36,164,44,178,211,45,48,180,213,109,22,12,58,86,71,182,112,95,204,62,219,31,177,139,153,125,243,147,171,70,167,176,83,20,201,6,167,29,13,219,
24,224,66,185,1,231,174,13,164,100,70,161,89,211,108,177,74,78,32,224,105,154,239,75,189,148,45,215,245,203,128,223,34,144,216,5,120,184,68,239,251,175,9,93,74,14,44,222,172,212,102,164,180,235,78,188,125,39,208,55,80,68,191,134,11,59,223,70,187,227,
123,59,231,48,232,246,209,100,97,87,29,98,128,84,221,239,86,89,138,77,45,137,27,21,144,36,201,181,154,129,110,95,0,122,82,150,69,138,76,245,4,74,186,118,254,170,168,31,207,33,9,22,93,211,253,94,155,74,166,34,113,140,155,68,144,118,19,89,85,253,163,226,
196,232,10,145,16,226,145,62,224,139,251,84,199,202,33,253,225,181,218,30,26,161,22,109,154,30,89,211,211,121,182,115,124,90,31,160,168,232,94,46,189,137,196,165,172,0,51,96,204,243,50,219,152,228,183,178,52,104,202,16,13,10,133,239,183,131,57,253,81,
191,240,187,113,137,212,195,46,155,8,230,41,121,223,225,175,227,113,232,229,36,178,8,10,239,52,39,11,28,59,49,96,23,11,246,56,61,239,221,190,211,249,190,99,73,116,43,231,242,252,96,251,108,58,62,16,162,100,166,43,255,165,152,224,34,24,160,181,57,148,
150,0,101,118,235,86,113,48,28,166,17,98,173,162,18,112,133,207,116,95,195,222,207,196,59,123,42,55,191,234,100,217,148,35,235,220,58,181,97,115,141,74,173,98,167,59,93,232,192,241,155,144,51,104,171,10,139,15,24,46,215,249,45,105,225,177,182,159,10,
1,194,139,58,210,248,147,221,180,172,223,223,171,235,146,118,58,105,209,227,90,93,193,205,143,114,49,157,158,110,142,39,108,171,235,3,88,20,207,95,197,57,20,91,253,125,196,95,14,4,53,6,160,186,64,46,66,108,67,175,57,43,39,226,134,93,68,192,78,44,191,
94,188,29,34,99,193,102,38,61,141,84,195,94,88,182,24,102,58,205,247,41,253,206,154,226,143,202,12,180,189,235,197,217,217,131,253,170,132,13,237,158,196,37,196,246,9,43,86,100,144,85,226,3,43,236,100,98,236,116,173,255,109,153,25,184,116,34,179,239,
97,127,239,15,92,46,111,252,107,252,221,111,162,1,37,193,22,183,82,102,221,109,147,104,76,71,15,138,253,162,196,1,0,250,13,186,46,20,18,62,119,52,208,138,25,187,65,36,165,75,0,76,119,16,235,113,191,199,111,21,134,64,182,106,104,24,25,246,196,24,22,142,
94,218,234,104,74,7,227,102,160,74,40,161,204,228,160,135,37,46,128,25,157,174,118,177,110,53,145,40,31,46,120,45,26,59,48,140,147,200,117,127,54,231,7,117,197,135,202,117,37,240,82,18,224,220,102,96,184,57,164,64,42,172,156,237,73,96,61,105,155,106,
176,32,176,179,170,217,135,43,248,12,160,199,237,28,113,5,91,6,0,64,128,246,125,113,181,111,0,82,227,232,78,74,23,91,224,107,100,44,210,224,4,151,155,132,173,239,14,51,135,34,183,194,161,9,11,215,181,216,171,106,28,82,247,99,63,51,232,34,117,2,23,226,
30,31,117,214,85,13,117,61,187,33,129,207,36,110,179,139,75,196,1,22,1,191,48,100,6,75,200,132,138,1,4,34,195,30,76,65,24,161,174,177,241,14,71,242,251,206,227,217,88,230,0,194,158,198,102,81,135,182,183,249,201,116,150,30,78,167,229,182,53,36,109,235,
184,107,51,3,125,6,40,138,154,75,9,112,107,56,192,193,98,159,254,121,212,193,74,242,104,133,88,128,255,156,51,157,233,13,206,19,24,122,4,133,114,106,97,148,175,250,26,154,54,161,132,229,196,95,6,210,99,193,33,223,210,0,231,11,184,121,112,54,5,82,187,
30,174,16,15,159,1,236,86,107,62,19,228,121,153,236,239,23,7,219,219,98,5,15,31,246,136,238,213,220,186,24,105,211,188,26,163,96,3,190,48,222,39,80,153,129,20,35,180,133,169,176,64,252,199,222,135,184,238,94,93,189,70,62,135,193,235,96,44,12,118,176,
85,148,201,150,32,103,16,186,182,177,23,202,13,184,140,4,80,137,149,179,217,124,56,28,21,187,131,1,239,49,251,216,57,86,122,159,120,183,115,12,17,177,191,44,245,188,185,77,33,180,167,55,74,28,93,159,217,20,166,122,226,223,218,252,54,253,219,39,148,159,
18,238,51,71,184,226,1,226,76,180,136,105,124,141,238,3,69,158,102,131,179,233,108,15,245,170,191,148,39,240,34,173,98,193,212,3,40,79,118,145,159,109,230,243,106,87,127,85,24,168,129,115,136,245,176,119,156,191,122,113,133,21,14,61,184,160,65,84,216,
232,127,191,148,202,219,85,72,152,234,163,246,42,247,29,63,139,90,244,216,228,144,24,19,180,246,178,130,5,4,103,221,182,74,225,95,26,226,189,123,243,91,130,252,218,102,183,214,139,134,134,47,42,1,140,184,33,167,202,124,3,145,0,73,194,186,9,28,253,171,
185,203,24,176,224,253,85,59,164,247,93,167,49,146,232,179,149,193,116,32,172,177,69,64,127,226,99,171,159,69,84,1,4,173,144,252,102,68,108,21,235,34,16,106,16,241,68,82,80,232,228,140,31,72,233,207,47,187,103,196,121,93,193,208,142,65,211,254,11,243,
77,206,11,201,0,212,19,176,234,33,254,101,192,30,139,170,2,92,233,243,109,105,128,58,11,32,22,14,38,118,38,253,175,226,1,190,216,239,19,227,81,194,247,255,244,226,17,66,12,246,183,31,187,244,51,34,90,38,193,119,197,14,132,222,76,18,47,19,15,72,47,186,
250,117,70,48,217,164,179,253,36,169,119,186,98,251,162,40,126,209,231,219,13,216,176,85,100,226,227,7,30,7,145,202,203,7,181,215,28,194,163,159,218,70,166,217,118,152,55,182,247,34,208,214,209,96,61,110,9,53,42,191,71,118,176,121,138,175,50,90,27,170,
134,140,103,32,236,217,148,221,144,0,124,52,26,225,137,221,82,246,218,48,128,121,6,117,69,195,155,29,142,198,184,221,175,123,113,137,206,95,166,215,251,63,3,157,221,54,98,217,65,141,76,21,162,29,145,117,146,0,117,159,64,213,139,128,186,197,240,166,14,
176,215,246,247,247,186,12,204,52,27,16,114,129,30,222,222,6,201,119,14,249,152,2,34,122,159,7,82,70,151,203,115,118,58,61,184,57,159,87,147,157,29,113,162,210,242,86,215,147,151,243,4,50,83,13,68,15,242,188,226,227,73,190,183,191,143,131,243,139,119,
88,162,46,96,197,239,131,128,236,109,230,193,214,222,63,200,192,245,141,240,124,1,186,206,158,204,64,221,149,0,226,96,111,161,105,7,93,155,223,223,192,164,133,27,150,33,255,8,16,116,104,6,180,190,170,217,135,14,242,252,119,55,148,235,186,217,69,228,90,
36,128,243,5,20,101,197,211,116,190,183,189,197,89,60,19,248,34,250,126,145,229,192,123,36,11,68,228,112,119,39,95,99,87,163,107,240,226,152,192,58,83,192,37,124,180,178,128,122,136,213,217,44,18,187,82,195,134,114,93,243,40,12,16,126,31,147,69,60,233,
190,235,56,73,178,201,116,54,223,214,153,110,215,36,1,236,208,172,9,88,85,197,48,207,167,55,24,27,178,118,242,199,50,83,45,166,235,87,92,94,43,93,31,67,82,220,184,129,153,0,206,99,193,32,157,20,76,61,3,35,32,16,34,120,32,244,188,135,121,0,204,182,13,
130,126,173,6,145,41,242,85,11,68,188,145,244,183,170,197,224,222,253,153,170,112,242,83,195,30,186,4,112,27,22,169,185,44,135,117,61,221,111,124,0,112,14,145,31,147,18,176,0,200,117,87,119,151,169,248,98,255,131,209,211,212,213,212,118,214,0,215,43,
136,41,35,128,65,35,1,162,142,155,8,216,67,111,133,67,184,217,9,180,71,229,154,136,96,183,193,68,216,73,215,254,158,43,73,247,164,135,124,152,30,61,128,27,6,188,250,27,73,61,92,51,208,248,157,121,173,178,84,203,97,154,230,123,203,37,64,223,138,199,37,
118,83,31,38,128,30,119,70,95,120,152,27,71,144,96,131,193,48,79,179,172,32,194,55,29,54,128,73,56,61,85,55,97,202,195,195,28,203,150,30,239,251,25,108,202,201,44,98,111,241,75,224,20,98,65,243,8,22,168,8,215,103,192,126,23,111,46,75,7,44,203,75,118,
168,83,203,154,26,199,235,48,3,155,114,176,106,190,197,121,37,37,192,36,96,128,85,9,199,86,0,126,139,164,4,44,248,188,47,17,116,243,119,234,178,116,120,248,228,219,59,59,187,247,104,199,112,221,35,128,169,2,215,167,158,122,242,245,52,77,207,132,168,55,
56,95,226,166,229,81,225,210,180,142,241,29,76,17,87,72,75,172,135,14,169,158,223,117,210,194,97,20,72,166,83,60,172,202,50,163,78,168,22,151,93,151,25,8,40,80,84,245,108,103,60,174,247,251,87,239,121,45,129,243,232,121,88,241,218,70,94,115,200,216,225,
193,135,95,223,220,220,152,10,81,82,143,128,90,119,165,21,252,230,205,155,239,221,56,220,121,87,94,121,19,120,163,207,91,141,205,3,98,59,54,13,164,3,68,134,133,61,142,206,86,186,249,162,101,226,237,91,163,123,37,103,236,248,108,251,22,181,223,27,167,
195,162,109,162,63,100,87,48,29,117,77,173,120,139,205,195,3,138,3,172,74,156,190,14,30,141,152,94,236,210,133,115,156,60,120,94,179,225,96,227,244,96,255,169,31,100,89,170,186,110,82,95,128,132,218,114,203,247,183,183,55,31,60,253,145,155,175,170,150,
174,166,221,11,53,49,5,163,14,18,207,50,176,18,130,155,142,111,246,58,110,123,9,37,102,149,122,159,73,204,251,246,187,237,247,218,6,84,60,248,110,223,205,204,195,254,67,106,49,14,88,89,63,113,88,213,212,49,140,93,111,151,48,211,30,80,2,169,124,123,255,
128,0,0,6,122,120,25,19,44,210,249,176,2,102,88,197,213,220,6,133,181,144,12,48,220,186,43,37,192,15,185,68,129,212,115,151,186,132,232,123,97,48,26,14,231,31,253,200,19,175,190,125,175,110,251,1,60,61,13,61,67,107,245,11,98,93,147,45,38,250,209,183,
235,89,183,153,84,184,150,57,235,74,137,36,29,110,151,69,53,72,120,114,124,81,6,184,144,4,160,216,57,213,215,205,230,167,135,89,54,134,197,174,219,101,65,32,22,68,16,23,5,119,96,117,39,123,112,155,101,137,108,52,60,120,255,246,237,15,191,73,237,108,36,
241,43,146,0,70,10,8,82,7,207,63,255,209,239,75,109,90,96,192,103,176,200,186,132,126,25,7,97,96,201,59,59,142,161,224,154,69,178,208,50,104,81,214,147,211,179,98,123,48,200,138,107,145,0,198,19,168,208,243,96,32,167,177,58,185,13,108,131,233,146,192,
186,101,115,47,223,40,121,145,219,183,47,159,16,122,192,223,50,6,75,89,93,145,104,191,249,230,225,225,225,93,2,80,68,120,98,100,208,29,23,21,67,63,249,228,173,55,118,119,143,143,6,67,141,3,90,209,57,92,16,156,12,115,0,89,219,53,140,139,92,92,216,13,250,
248,166,165,255,219,232,51,37,117,22,207,97,235,193,113,114,48,28,102,249,60,175,199,23,105,24,121,94,21,96,246,6,72,235,44,131,138,32,244,201,233,29,41,90,19,70,113,1,110,88,93,181,76,5,179,53,154,176,59,132,216,30,54,24,236,18,219,76,139,67,209,168,
149,90,226,122,4,55,155,51,145,179,150,67,184,65,147,191,131,8,111,229,81,171,125,183,228,7,62,248,128,163,168,63,254,27,91,91,10,0,82,143,192,74,131,64,11,158,16,63,116,123,247,205,143,126,248,131,111,125,240,193,252,143,9,97,76,91,180,114,218,58,253,
133,110,15,107,242,11,154,98,62,101,143,49,189,243,152,142,60,130,113,219,90,11,137,3,180,54,156,245,169,15,118,131,57,179,11,9,130,183,155,48,182,57,65,7,173,50,150,242,153,180,0,18,65,109,228,147,138,13,46,82,24,114,46,219,81,245,211,173,170,65,85,
149,89,89,8,246,163,15,94,255,56,138,111,127,81,26,132,131,34,23,96,60,82,92,59,220,84,195,92,87,138,213,12,206,238,216,211,108,178,98,74,179,108,119,87,98,26,245,36,225,54,213,73,39,109,8,87,168,161,55,169,214,123,182,152,156,126,131,128,85,164,79,51,
150,170,0,146,160,142,90,174,242,162,218,124,127,52,250,228,191,147,18,224,71,196,16,212,123,87,167,130,171,251,74,203,178,76,235,186,96,175,189,118,231,147,223,123,109,240,211,149,100,15,41,226,120,173,146,45,140,5,70,29,190,193,171,49,246,140,0,253,
190,208,155,79,177,86,74,57,232,248,2,96,87,64,8,167,64,56,232,162,114,68,163,238,193,178,23,52,243,71,190,11,206,148,180,98,48,132,131,189,242,205,159,250,253,163,95,221,221,221,125,64,237,110,169,175,48,157,15,141,1,76,71,112,154,172,140,218,195,74,
158,46,103,51,44,139,162,164,70,209,25,245,218,177,217,41,189,34,196,22,107,154,213,174,87,119,219,67,210,8,123,223,159,239,125,30,89,60,239,202,73,18,189,189,3,33,124,146,86,164,239,199,227,65,53,28,226,144,56,135,58,132,6,253,247,169,135,177,188,175,
106,192,185,40,80,76,165,121,93,200,123,172,51,211,4,203,166,94,197,241,0,178,139,229,228,118,228,124,187,171,36,250,219,210,113,221,190,154,26,91,147,244,74,179,180,28,100,163,186,22,195,17,189,47,113,64,105,113,205,67,197,0,214,124,210,12,81,39,163,
81,61,200,50,150,80,137,184,156,43,142,200,151,230,169,65,183,36,110,213,56,241,50,175,130,98,9,206,237,242,129,132,152,64,62,79,229,2,161,166,203,181,6,125,106,219,21,87,150,68,190,84,106,33,167,91,178,86,169,192,201,16,248,136,46,79,104,65,175,150,
116,9,216,99,237,175,180,14,35,187,105,183,121,77,229,48,168,68,76,218,128,41,69,76,178,170,166,251,224,234,126,12,158,121,232,253,1,208,243,161,51,234,84,38,127,55,149,115,89,5,221,42,174,116,143,251,11,154,170,22,179,8,227,246,21,218,122,233,246,220,
247,67,195,182,127,144,215,133,19,206,43,41,31,226,61,97,59,126,161,221,217,254,94,2,15,21,3,96,131,70,152,63,65,33,209,31,19,6,104,37,125,216,201,241,136,143,193,60,248,39,15,61,107,143,227,61,133,204,224,191,247,48,29,65,246,135,148,9,101,203,197,46,
202,84,15,91,2,120,13,161,41,237,191,111,130,252,4,17,176,224,208,111,140,253,56,221,147,207,16,11,238,233,234,37,192,250,248,241,59,248,122,10,214,12,176,62,214,12,176,62,214,12,176,62,214,12,176,62,214,12,176,62,214,12,176,62,214,12,176,62,214,12,176,
62,214,12,176,62,214,12,176,62,214,12,176,62,214,12,176,62,214,12,176,62,214,12,176,62,126,28,143,255,39,192,0,238,147,31,89,162,25,31,21,0,0,0,0,73,69,78,68,174,66,96,130,0,0 };

const char* projectIconCodeblocks_png = (const char*) temp_binary_data_27;

//================== projectIconLinuxMakefile.png ==================
static const unsigned char temp_binary_data_28[] =
{ 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,110,0,0,0,128,8,6,0,0,0,234,21,92,9,0,0,10,65,105,67,67,80,73,67,67,32,80,114,111,102,105,108,101,0,0,72,13,157,150,119,84,83,217,22,135,207,189,55,189,208,18,34,32,37,244,26,122,9,32,210,59,72,21,4,
81,137,73,128,80,2,134,132,38,118,68,5,70,20,17,41,86,100,84,192,1,71,135,34,99,69,20,11,131,130,98,215,9,242,16,80,198,193,81,68,69,229,221,140,107,9,239,173,53,243,222,154,253,199,89,223,217,231,183,215,217,103,239,125,215,186,0,80,252,130,4,194,116,
88,1,128,52,161,88,20,238,235,193,92,18,19,203,196,247,2,24,16,1,14,88,1,192,225,102,102,4,71,248,68,2,212,252,189,61,153,153,168,72,198,179,246,238,46,128,100,187,219,44,191,80,38,115,214,255,127,145,34,55,67,36,6,0,10,69,213,54,60,126,38,23,229,2,148,
83,179,197,25,50,255,4,202,244,149,41,50,134,49,50,22,161,9,162,172,34,227,196,175,108,246,167,230,43,187,201,152,151,38,228,161,26,89,206,25,188,52,158,140,187,80,222,154,37,225,163,140,4,161,92,152,37,224,103,163,124,7,101,189,84,73,154,0,229,247,40,
211,211,248,156,76,0,48,20,153,95,204,231,38,161,108,137,50,69,20,25,238,137,242,2,0,8,148,196,57,188,114,14,139,249,57,104,158,0,120,166,103,228,138,4,137,73,98,166,17,215,152,105,229,232,200,102,250,241,179,83,249,98,49,43,148,195,77,225,136,120,76,
207,244,180,12,142,48,23,128,175,111,150,69,1,37,89,109,153,104,145,237,173,28,237,237,89,214,230,104,249,191,217,223,30,126,83,253,61,200,122,251,85,241,38,236,207,158,65,140,158,89,223,108,236,172,47,189,22,0,246,36,90,155,29,179,190,149,85,0,180,109,
6,64,229,225,172,79,239,32,0,242,5,0,180,222,156,243,30,134,108,94,146,196,226,12,39,11,139,236,236,108,115,1,159,107,46,43,232,55,251,159,130,111,202,191,134,57,247,153,203,238,251,86,59,166,23,63,129,35,73,21,51,101,69,229,166,167,166,75,68,204,204,
12,14,151,207,100,253,247,16,255,227,192,57,105,205,201,195,44,156,159,192,23,241,133,232,85,81,232,148,9,132,137,104,187,133,60,129,88,144,46,100,10,132,127,213,225,127,24,54,39,7,25,126,157,107,20,104,117,95,0,125,133,57,80,184,73,7,200,111,61,0,67,
35,3,36,110,63,122,2,125,235,91,16,49,10,200,190,188,104,173,145,175,115,143,50,122,254,231,250,31,11,92,138,110,225,76,65,34,83,230,246,12,143,100,114,37,162,44,25,163,223,132,108,193,2,18,144,7,116,160,10,52,129,46,48,2,44,96,13,28,128,51,112,3,222,
32,0,132,128,72,16,3,150,3,46,72,2,105,64,4,178,65,62,216,0,10,65,49,216,1,118,131,106,112,0,212,129,122,208,4,78,130,54,112,6,92,4,87,192,13,112,11,12,128,71,64,10,134,193,75,48,1,222,129,105,8,130,240,16,21,162,65,170,144,22,164,15,153,66,214,16,27,
90,8,121,67,65,80,56,20,3,197,67,137,144,16,146,64,249,208,38,168,24,42,131,170,161,67,80,61,244,35,116,26,186,8,93,131,250,160,7,208,32,52,6,253,1,125,132,17,152,2,211,97,13,216,0,182,128,217,176,59,28,8,71,194,203,224,68,120,21,156,7,23,192,219,225,
74,184,22,62,14,183,194,23,225,27,240,0,44,133,95,194,147,8,64,200,8,3,209,70,88,8,27,241,68,66,144,88,36,1,17,33,107,145,34,164,2,169,69,154,144,14,164,27,185,141,72,145,113,228,3,6,135,161,97,152,24,22,198,25,227,135,89,140,225,98,86,97,214,98,74,48,
213,152,99,152,86,76,23,230,54,102,16,51,129,249,130,165,98,213,177,166,88,39,172,63,118,9,54,17,155,141,45,196,86,96,143,96,91,176,151,177,3,216,97,236,59,28,14,199,192,25,226,28,112,126,184,24,92,50,110,53,174,4,183,15,215,140,187,128,235,195,13,225,
38,241,120,188,42,222,20,239,130,15,193,115,240,98,124,33,190,10,127,28,127,30,223,143,31,198,191,39,144,9,90,4,107,130,15,33,150,32,36,108,36,84,16,26,8,231,8,253,132,17,194,52,81,129,168,79,116,34,134,16,121,196,92,98,41,177,142,216,65,188,73,28,38,
78,147,20,73,134,36,23,82,36,41,153,180,129,84,73,106,34,93,38,61,38,189,33,147,201,58,100,71,114,24,89,64,94,79,174,36,159,32,95,37,15,146,63,80,148,40,38,20,79,74,28,69,66,217,78,57,74,185,64,121,64,121,67,165,82,13,168,110,212,88,170,152,186,157,90,
79,189,68,125,74,125,47,71,147,51,151,243,151,227,201,173,147,171,145,107,149,235,151,123,37,79,148,215,151,119,151,95,46,159,39,95,33,127,74,254,166,252,184,2,81,193,64,193,83,129,163,176,86,161,70,225,180,194,61,133,73,69,154,162,149,98,136,98,154,
98,137,98,131,226,53,197,81,37,188,146,129,146,183,18,79,169,64,233,176,210,37,165,33,26,66,211,165,121,210,184,180,77,180,58,218,101,218,48,29,71,55,164,251,211,147,233,197,244,31,232,189,244,9,101,37,101,91,229,40,229,28,229,26,229,179,202,82,6,194,
48,96,248,51,82,25,165,140,147,140,187,140,143,243,52,230,185,207,227,207,219,54,175,105,94,255,188,41,149,249,42,110,42,124,149,34,149,102,149,1,149,143,170,76,85,111,213,20,213,157,170,109,170,79,212,48,106,38,106,97,106,217,106,251,213,46,171,141,
207,167,207,119,158,207,157,95,52,255,228,252,135,234,176,186,137,122,184,250,106,245,195,234,61,234,147,26,154,26,190,26,25,26,85,26,151,52,198,53,25,154,110,154,201,154,229,154,231,52,199,180,104,90,11,181,4,90,229,90,231,181,94,48,149,153,238,204,
84,102,37,179,139,57,161,173,174,237,167,45,209,62,164,221,171,61,173,99,168,179,88,103,163,78,179,206,19,93,146,46,91,55,65,183,92,183,83,119,66,79,75,47,88,47,95,175,81,239,161,62,81,159,173,159,164,191,71,191,91,127,202,192,208,32,218,96,139,65,155,
193,168,161,138,161,191,97,158,97,163,225,99,35,170,145,171,209,42,163,90,163,59,198,56,99,182,113,138,241,62,227,91,38,176,137,157,73,146,73,141,201,77,83,216,212,222,84,96,186,207,180,207,12,107,230,104,38,52,171,53,187,199,162,176,220,89,89,172,70,
214,160,57,195,60,200,124,163,121,155,249,43,11,61,139,88,139,157,22,221,22,95,44,237,44,83,45,235,44,31,89,41,89,5,88,109,180,234,176,250,195,218,196,154,107,93,99,125,199,134,106,227,99,179,206,166,221,230,181,173,169,45,223,118,191,237,125,59,154,
93,176,221,22,187,78,187,207,246,14,246,34,251,38,251,49,7,61,135,120,135,189,14,247,216,116,118,40,187,132,125,213,17,235,232,225,184,206,241,140,227,7,39,123,39,177,211,73,167,223,157,89,206,41,206,13,206,163,11,12,23,240,23,212,45,24,114,209,113,225,
184,28,114,145,46,100,46,140,95,120,112,161,212,85,219,149,227,90,235,250,204,77,215,141,231,118,196,109,196,221,216,61,217,253,184,251,43,15,75,15,145,71,139,199,148,167,147,231,26,207,11,94,136,151,175,87,145,87,175,183,146,247,98,239,106,239,167,62,
58,62,137,62,141,62,19,190,118,190,171,125,47,248,97,253,2,253,118,250,221,243,215,240,231,250,215,251,79,4,56,4,172,9,232,10,164,4,70,4,86,7,62,11,50,9,18,5,117,4,195,193,1,193,187,130,31,47,210,95,36,92,212,22,2,66,252,67,118,133,60,9,53,12,93,21,250,
115,24,46,44,52,172,38,236,121,184,85,120,126,120,119,4,45,98,69,68,67,196,187,72,143,200,210,200,71,139,141,22,75,22,119,70,201,71,197,69,213,71,77,69,123,69,151,69,75,151,88,44,89,179,228,70,140,90,140,32,166,61,22,31,27,21,123,36,118,114,169,247,210,
221,75,135,227,236,226,10,227,238,46,51,92,150,179,236,218,114,181,229,169,203,207,174,144,95,193,89,113,42,30,27,31,29,223,16,255,137,19,194,169,229,76,174,244,95,185,119,229,4,215,147,187,135,251,146,231,198,43,231,141,241,93,248,101,252,145,4,151,
132,178,132,209,68,151,196,93,137,99,73,174,73,21,73,227,2,79,65,181,224,117,178,95,242,129,228,169,148,144,148,163,41,51,169,209,169,205,105,132,180,248,180,211,66,37,97,138,176,43,93,51,61,39,189,47,195,52,163,48,67,186,202,105,213,238,85,19,162,64,
209,145,76,40,115,89,102,187,152,142,254,76,245,72,140,36,155,37,131,89,11,179,106,178,222,103,71,101,159,202,81,204,17,230,244,228,154,228,110,203,29,201,243,201,251,126,53,102,53,119,117,103,190,118,254,134,252,193,53,238,107,14,173,133,214,174,92,
219,185,78,119,93,193,186,225,245,190,235,143,109,32,109,72,217,240,203,70,203,141,101,27,223,110,138,222,212,81,160,81,176,190,96,104,179,239,230,198,66,185,66,81,225,189,45,206,91,14,108,197,108,21,108,237,221,102,179,173,106,219,151,34,94,209,245,
98,203,226,138,226,79,37,220,146,235,223,89,125,87,249,221,204,246,132,237,189,165,246,165,251,119,224,118,8,119,220,221,233,186,243,88,153,98,89,94,217,208,174,224,93,173,229,204,242,162,242,183,187,87,236,190,86,97,91,113,96,15,105,143,100,143,180,
50,168,178,189,74,175,106,71,213,167,234,164,234,129,26,143,154,230,189,234,123,183,237,157,218,199,219,215,191,223,109,127,211,1,141,3,197,7,62,30,20,28,188,127,200,247,80,107,173,65,109,197,97,220,225,172,195,207,235,162,234,186,191,103,127,95,127,
68,237,72,241,145,207,71,133,71,165,199,194,143,117,213,59,212,215,55,168,55,148,54,194,141,146,198,177,227,113,199,111,253,224,245,67,123,19,171,233,80,51,163,185,248,4,56,33,57,241,226,199,248,31,239,158,12,60,217,121,138,125,170,233,39,253,159,246,
182,208,90,138,90,161,214,220,214,137,182,164,54,105,123,76,123,223,233,128,211,157,29,206,29,45,63,155,255,124,244,140,246,153,154,179,202,103,75,207,145,206,21,156,155,57,159,119,126,242,66,198,133,241,139,137,23,135,58,87,116,62,186,180,228,210,157,
174,176,174,222,203,129,151,175,94,241,185,114,169,219,189,251,252,85,151,171,103,174,57,93,59,125,157,125,189,237,134,253,141,214,30,187,158,150,95,236,126,105,233,181,239,109,189,233,112,179,253,150,227,173,142,190,5,125,231,250,93,251,47,222,246,186,
125,229,142,255,157,27,3,139,6,250,238,46,190,123,255,94,220,61,233,125,222,253,209,7,169,15,94,63,204,122,56,253,104,253,99,236,227,162,39,10,79,42,158,170,63,173,253,213,248,215,102,169,189,244,236,160,215,96,207,179,136,103,143,134,184,67,47,255,149,
249,175,79,195,5,207,169,207,43,70,180,70,234,71,173,71,207,140,249,140,221,122,177,244,197,240,203,140,151,211,227,133,191,41,254,182,247,149,209,171,159,126,119,251,189,103,98,201,196,240,107,209,235,153,63,74,222,168,190,57,250,214,246,109,231,100,
232,228,211,119,105,239,166,167,138,222,171,190,63,246,129,253,161,251,99,244,199,145,233,236,79,248,79,149,159,141,63,119,124,9,252,242,120,38,109,102,230,223,247,132,243,251,50,58,89,126,0,0,0,9,112,72,89,115,0,0,11,19,0,0,11,19,1,0,154,156,24,0,0,
53,161,73,68,65,84,120,1,237,125,7,152,28,197,181,238,233,158,153,157,205,121,87,90,173,194,174,34,202,2,33,36,129,64,194,96,242,35,7,131,237,251,12,56,2,182,177,253,192,143,96,44,27,219,23,108,227,135,237,11,215,198,15,184,228,100,131,201,225,145,51,
40,24,37,16,146,80,206,187,210,230,60,211,117,255,255,204,212,104,52,154,213,102,105,120,31,245,109,109,245,84,87,85,159,58,127,157,83,167,66,87,139,124,225,190,224,192,23,28,248,130,3,95,112,160,11,14,56,93,220,255,60,222,182,117,98,104,226,42,16,127,
29,23,253,249,188,180,149,252,124,82,31,161,154,117,176,158,49,225,72,244,94,255,121,223,7,79,240,226,253,94,137,62,79,63,62,207,192,145,118,23,62,17,40,198,231,192,7,225,61,248,86,248,38,248,120,103,243,242,254,231,82,18,63,143,192,89,166,91,192,2,112,
19,195,225,240,209,142,227,28,10,32,42,140,49,165,8,179,225,153,166,30,241,155,225,215,34,254,3,164,123,23,113,235,225,173,163,36,218,178,108,220,23,97,63,115,128,76,182,110,176,235,186,255,11,126,145,207,231,51,61,240,77,72,251,52,252,217,182,32,132,
148,92,250,47,220,0,112,192,130,150,13,176,126,1,198,239,72,0,43,140,223,29,240,33,120,94,199,123,198,211,123,240,198,239,247,171,199,245,219,240,39,199,209,234,143,187,254,226,178,31,56,160,210,0,134,207,3,104,43,201,252,168,39,72,244,246,119,151,33,
202,240,160,54,67,160,41,28,7,224,125,248,93,20,165,211,54,144,126,32,123,224,138,248,60,16,73,26,41,41,223,64,248,56,152,94,140,144,140,183,125,93,183,84,28,242,9,250,55,25,49,98,132,83,94,94,238,162,191,115,26,27,27,195,12,225,166,194,159,11,32,63,
244,60,111,19,202,230,51,83,218,104,33,129,169,236,72,31,85,222,57,8,31,132,39,88,52,36,168,210,122,100,88,65,82,5,160,200,216,177,99,101,234,212,169,50,108,216,48,1,80,110,67,67,131,180,183,183,135,113,191,8,32,126,29,207,250,0,233,214,160,252,148,6,
47,149,117,58,37,137,32,85,192,223,1,79,199,223,189,106,108,148,56,186,252,252,124,25,58,116,168,74,31,128,18,0,230,172,93,187,214,15,0,67,72,19,68,220,227,72,54,19,254,99,120,109,56,8,83,206,117,75,205,28,76,170,33,1,215,225,249,133,240,29,240,189,6,
141,210,70,151,145,145,33,80,145,178,109,219,54,161,180,81,18,179,178,178,4,160,177,17,243,25,217,120,230,93,8,57,14,100,67,233,145,100,35,253,1,113,41,73,20,106,206,6,69,78,143,1,19,151,34,76,135,103,159,211,43,122,41,109,236,223,178,179,179,229,240,
195,15,71,49,34,91,183,110,149,218,218,90,5,177,163,163,67,239,235,141,72,255,233,135,228,93,3,176,111,66,92,74,74,93,170,170,74,5,8,210,112,38,24,71,208,122,173,34,9,6,85,162,117,80,139,82,93,93,45,205,205,205,26,79,80,173,143,166,81,45,132,184,31,227,
247,125,240,91,224,109,67,138,38,57,248,65,175,84,207,129,34,27,192,221,128,103,141,134,167,244,245,73,173,19,28,74,22,165,44,20,10,9,36,89,213,100,2,104,172,26,27,13,27,74,54,238,213,3,244,55,112,205,103,239,65,31,63,14,182,235,19,51,6,136,120,203,164,
65,96,218,33,209,103,244,27,157,22,48,74,97,188,36,38,212,197,170,228,11,16,159,9,159,114,125,93,191,49,36,161,226,125,249,169,76,195,252,99,25,90,252,136,104,65,150,145,177,114,41,41,189,113,4,139,224,5,131,65,149,184,78,192,211,198,131,103,140,71,218,
47,71,159,147,82,188,74,41,98,226,129,128,97,48,44,250,155,106,50,134,18,25,77,67,195,170,187,238,0,200,52,4,11,227,54,5,171,173,173,77,251,56,170,78,52,144,206,36,143,207,165,59,43,18,164,150,170,76,85,227,132,204,28,154,8,10,153,79,70,231,228,228,72,
83,83,83,204,156,167,73,79,115,63,49,61,25,158,150,150,166,96,49,61,29,199,112,211,166,77,83,224,95,124,241,69,33,136,148,62,130,152,144,95,27,11,232,152,141,108,84,151,205,240,140,75,137,190,46,101,37,14,12,226,212,22,157,50,138,204,197,12,135,2,198,
177,216,145,71,30,41,199,30,123,172,2,70,233,35,120,137,46,61,61,93,90,90,90,52,79,89,89,153,12,31,62,92,45,202,45,91,182,200,200,145,35,229,178,203,46,147,194,194,66,5,143,13,34,193,41,112,0,179,18,247,38,70,239,237,251,144,132,76,7,234,103,42,90,149,
218,170,193,176,211,224,143,66,139,55,144,26,151,0,96,142,81,61,165,103,247,238,221,50,121,242,100,149,160,213,171,87,171,26,68,82,229,27,67,130,70,147,127,214,172,89,114,234,169,167,42,208,135,29,118,152,28,122,232,161,146,153,153,41,143,60,242,136,
140,27,55,78,38,76,152,32,139,23,47,214,252,118,144,30,101,62,233,160,81,194,49,221,91,240,28,79,146,95,86,133,226,242,224,185,148,85,149,144,32,78,63,197,0,56,241,196,19,229,152,99,142,209,65,52,251,184,101,203,150,201,221,119,223,45,103,156,113,6,39,
142,101,195,134,13,170,70,237,64,187,190,190,94,239,29,127,252,241,66,105,179,198,8,239,183,182,182,234,156,229,173,183,222,42,115,231,206,149,241,227,199,203,39,159,124,162,106,149,247,19,29,26,208,84,196,113,76,151,18,160,145,190,148,5,14,160,193,150,
240,171,212,140,30,61,90,206,57,231,28,153,56,113,162,2,71,181,118,220,113,199,169,180,92,117,213,85,42,117,172,12,37,134,179,35,4,109,250,244,233,242,205,111,126,83,211,80,181,178,127,4,0,218,183,177,63,99,153,148,200,155,110,186,73,42,43,43,153,61,
126,246,68,127,227,159,170,75,208,50,54,26,145,18,253,155,37,46,213,66,237,71,0,218,173,67,134,12,33,163,66,191,251,221,239,76,85,85,21,248,183,199,1,36,3,201,49,96,60,211,24,204,246,235,90,92,52,143,121,234,169,167,12,164,103,79,134,132,43,222,91,181,
106,149,57,225,132,19,52,127,81,81,145,129,148,39,174,231,113,101,130,113,11,83,141,73,169,72,143,2,87,80,80,240,59,128,167,192,173,92,185,82,217,254,209,71,31,153,199,30,123,204,96,218,42,6,195,166,77,155,148,241,152,40,54,80,137,122,13,233,52,117,117,
117,177,52,219,183,111,55,15,63,252,176,185,229,150,91,204,219,111,191,109,8,186,117,47,188,240,130,230,129,181,153,8,26,127,43,112,0,244,147,84,100,84,170,209,164,6,19,12,145,223,128,48,115,242,201,39,135,96,77,154,37,75,150,196,24,12,80,205,162,69,
139,148,247,48,82,12,44,67,189,103,165,141,18,106,29,239,163,31,211,251,23,92,112,129,33,168,4,209,58,74,242,33,135,28,162,247,115,115,115,19,193,179,192,125,148,106,76,74,25,243,54,145,49,104,237,237,140,3,51,181,239,121,229,149,87,52,201,247,191,255,
125,185,246,218,107,229,222,123,239,213,120,142,211,102,207,226,242,153,200,172,153,71,104,152,1,139,210,186,55,223,124,83,222,120,227,13,25,51,102,140,212,212,212,8,84,163,60,240,192,3,186,172,195,52,121,121,121,114,254,249,231,107,114,94,3,80,155,53,
22,162,111,220,25,253,161,125,94,236,198,65,188,72,89,224,192,64,5,14,161,195,65,50,39,135,233,158,127,254,121,5,147,179,252,219,177,166,70,3,230,195,149,107,245,94,93,11,151,211,224,124,123,108,174,29,59,118,104,20,13,147,151,94,122,73,158,125,246,89,
121,250,233,167,101,233,82,90,247,176,206,144,159,3,122,58,52,22,13,227,254,89,20,119,197,197,165,196,101,202,2,7,203,175,158,28,122,239,189,247,116,134,132,3,102,186,55,222,124,75,62,251,108,141,212,236,174,18,39,16,148,134,250,70,49,107,63,149,121,
99,33,12,155,222,144,113,72,179,3,191,27,26,34,51,37,129,64,4,196,112,216,211,241,27,140,22,45,103,243,230,205,26,210,252,167,117,73,151,48,142,211,184,232,63,59,70,72,25,137,219,211,52,227,201,76,129,235,93,187,118,213,141,26,53,10,32,125,166,179,29,
52,239,233,166,77,28,43,191,253,237,239,164,108,210,209,82,187,234,53,25,108,94,144,229,79,139,4,210,10,161,230,154,48,205,50,88,90,27,111,149,198,247,33,141,19,174,145,142,112,68,138,178,130,33,25,156,219,44,217,21,99,100,233,199,171,133,101,211,113,
53,28,253,165,94,219,105,49,253,17,249,103,129,202,141,198,89,9,140,75,242,197,165,229,128,114,26,106,235,116,76,81,145,81,97,244,73,198,131,249,254,227,43,127,200,223,102,48,224,122,240,234,114,179,224,86,49,139,111,23,211,241,126,161,49,75,128,219,
191,196,44,188,77,204,35,151,151,152,237,79,138,193,158,101,179,252,145,35,204,57,179,57,109,230,131,215,225,133,193,84,151,105,106,106,84,251,100,211,230,205,90,38,13,19,244,151,157,25,39,17,100,45,133,41,16,166,172,196,161,79,170,227,64,26,206,249,
47,204,144,92,116,225,133,114,195,47,111,148,182,234,13,114,92,217,63,229,132,147,178,100,213,170,161,226,194,134,241,249,118,2,189,18,113,124,5,50,126,50,22,172,157,42,105,237,40,19,55,203,39,19,199,124,40,247,220,156,38,63,93,149,35,11,151,109,149,
38,255,57,114,201,181,191,134,218,204,82,246,191,255,30,119,164,71,140,148,36,18,167,247,240,143,243,166,148,58,18,68,41,36,216,7,213,89,85,112,80,137,72,120,56,37,46,140,217,145,195,193,157,5,131,74,138,101,203,214,109,242,207,167,159,149,25,163,74,
100,237,115,23,203,196,201,43,164,160,4,175,6,24,188,207,193,212,152,100,22,223,24,172,83,231,33,231,42,92,131,191,80,145,198,195,77,227,2,80,166,195,222,159,80,27,250,177,57,82,223,58,65,188,252,147,101,117,125,137,204,58,106,142,34,145,153,149,141,
169,48,187,0,16,163,136,0,145,71,173,24,203,29,131,126,119,1,174,149,190,88,138,131,116,65,34,82,205,209,96,226,32,185,208,117,228,91,18,110,243,181,117,132,205,224,237,15,58,197,13,127,147,201,135,85,73,97,9,164,197,68,152,108,96,54,232,154,170,175,
12,217,240,35,188,5,58,18,251,72,176,219,0,91,132,112,207,133,120,224,158,41,64,194,44,196,46,151,160,187,72,130,173,143,72,251,186,187,196,105,17,121,127,77,166,228,167,55,75,115,7,243,236,229,248,147,134,73,26,244,234,155,240,75,113,77,158,29,244,57,
203,84,180,42,85,13,113,114,63,59,205,105,172,107,10,201,143,190,156,46,231,156,16,144,169,71,149,74,225,32,74,26,65,35,233,220,232,67,228,176,123,220,129,68,121,53,202,83,199,137,242,150,59,238,124,195,196,241,151,66,234,10,145,54,83,60,147,141,164,
131,196,151,61,84,70,77,27,38,223,185,40,3,121,176,244,131,242,210,246,195,13,168,238,136,117,132,167,35,195,65,119,251,33,245,224,210,54,42,71,154,107,91,76,195,180,193,142,28,51,165,221,12,31,233,151,130,2,140,229,60,11,26,233,139,242,208,5,152,94,
3,252,86,196,89,129,64,213,220,97,17,245,9,80,140,7,117,233,237,22,55,163,73,54,111,172,150,39,254,177,77,218,234,183,73,97,65,187,28,138,251,205,200,30,140,234,31,142,249,232,163,78,47,160,1,230,69,127,83,2,99,55,163,113,7,60,72,69,224,20,141,225,121,
186,226,220,48,40,31,134,67,190,99,114,242,219,33,53,137,60,99,82,72,26,199,234,186,229,223,10,3,210,185,229,240,249,10,173,23,170,7,16,235,144,182,90,22,188,235,202,11,47,137,76,157,140,156,89,70,170,33,164,255,194,29,142,193,91,89,60,28,23,102,57,166,
227,224,28,78,65,2,144,92,33,152,192,8,184,47,128,139,240,97,223,255,51,135,18,56,167,177,185,133,123,76,124,152,12,65,27,131,69,239,89,108,226,179,120,187,0,94,27,98,216,14,145,192,29,1,155,164,68,153,239,200,78,113,211,86,75,93,77,179,60,252,119,191,
236,192,228,213,197,95,55,50,114,44,210,133,130,152,250,226,202,183,145,172,52,116,102,232,185,208,143,233,250,29,103,81,56,99,3,240,28,24,38,92,102,200,68,220,156,232,99,15,122,131,79,213,225,128,51,255,117,9,165,99,98,228,173,13,62,89,187,169,221,228,
47,247,203,232,137,105,146,95,10,233,234,112,0,10,155,61,85,26,44,74,149,43,7,76,247,196,56,5,218,151,57,190,6,132,59,165,5,243,147,139,151,248,100,203,22,71,102,31,97,100,196,40,100,12,65,188,194,153,178,107,251,118,121,241,53,66,145,33,38,212,46,33,
204,162,112,238,147,107,125,148,184,15,63,252,80,214,172,89,163,88,113,13,16,146,120,38,126,220,1,31,149,77,189,117,80,254,69,181,250,65,121,246,254,30,74,186,76,65,102,224,172,230,246,142,241,217,69,211,188,82,119,135,187,105,109,135,248,211,50,209,
47,117,136,27,4,104,49,234,161,185,32,3,78,26,226,252,132,179,65,106,170,183,202,191,22,180,202,251,31,248,165,108,136,200,113,243,60,128,14,192,66,133,18,110,175,150,237,235,27,229,131,229,199,201,194,214,57,178,100,201,34,72,22,54,12,65,228,168,38,
167,76,153,162,147,207,124,163,167,178,178,146,51,55,14,222,51,112,0,222,48,0,250,15,208,86,13,31,21,239,253,85,99,224,238,197,170,62,112,143,232,85,201,202,148,180,204,156,147,176,88,58,109,220,17,199,121,5,229,199,184,57,109,31,74,245,150,14,89,183,
49,27,210,213,46,173,205,142,164,65,211,249,144,186,3,130,184,121,19,230,41,183,117,200,155,111,182,203,166,77,62,25,82,238,147,99,142,242,100,40,12,27,7,175,133,55,238,174,150,77,107,26,228,147,85,19,228,179,182,203,164,125,216,73,146,149,157,35,159,
174,88,46,59,170,119,235,234,57,55,36,21,23,23,11,22,86,85,109,114,117,2,215,14,182,54,112,137,39,13,106,179,10,224,189,129,90,29,84,224,82,85,85,42,218,96,20,153,35,153,160,50,99,232,20,217,184,243,26,25,86,123,151,20,250,119,72,67,109,46,172,195,70,
201,204,113,176,215,68,164,13,192,109,223,38,50,14,147,205,115,142,116,164,148,115,29,65,244,99,29,249,210,88,21,150,173,235,182,201,134,29,179,36,60,226,98,201,61,124,130,140,200,43,16,15,234,49,11,3,239,25,179,102,203,250,77,143,233,158,21,62,239,253,
247,223,151,65,131,6,233,43,89,156,132,230,171,89,88,31,116,184,59,12,170,244,98,36,249,51,60,199,30,52,82,240,144,3,239,82,17,56,50,67,7,184,48,8,160,228,200,25,0,81,148,47,109,121,211,100,245,167,223,150,182,170,127,202,228,130,101,114,248,244,124,
201,205,111,4,251,162,188,99,109,148,149,216,103,217,22,148,166,170,76,169,218,182,83,214,163,155,106,44,190,65,74,231,158,32,131,134,12,151,156,172,12,9,192,98,196,58,184,170,196,217,179,103,11,86,214,117,89,135,125,28,87,11,184,134,119,244,209,71,235,
246,61,190,142,133,183,123,32,108,110,24,210,86,129,240,34,132,183,225,105,108,88,7,165,191,75,101,224,216,159,240,248,11,50,212,205,205,203,23,46,144,114,163,207,122,57,71,90,150,167,73,83,195,34,25,49,54,67,114,243,2,186,4,231,121,46,12,12,88,131,216,
202,87,191,187,81,182,110,104,148,106,239,12,201,153,118,145,140,26,55,85,138,11,11,36,15,118,127,16,229,216,113,26,23,79,185,63,243,136,35,142,80,99,132,170,145,247,248,26,214,19,79,60,161,111,175,98,123,132,170,77,90,152,160,137,36,253,8,254,126,248,
58,248,131,34,117,169,8,28,91,177,135,86,125,54,24,88,132,107,190,100,239,35,131,75,74,74,212,120,8,133,58,100,171,115,134,180,111,202,149,170,93,175,73,110,78,139,248,209,91,115,241,154,187,235,128,155,52,154,185,226,86,158,42,131,39,206,146,193,101,
229,82,84,0,233,68,25,104,4,40,50,226,104,250,227,57,50,105,210,36,221,28,75,43,146,139,170,92,234,177,27,112,23,44,224,244,36,150,141,160,143,161,54,93,208,196,55,87,71,33,223,143,0,226,124,220,58,40,82,151,106,192,145,9,180,239,243,192,156,31,145,169,
112,14,183,215,97,63,137,246,59,108,241,45,173,45,156,204,148,237,129,19,101,231,238,73,18,220,85,39,105,88,245,14,164,5,85,154,50,134,151,75,225,176,113,50,104,112,153,74,89,9,38,170,179,97,132,160,76,150,23,115,252,77,240,56,208,62,229,148,83,116,11,
195,163,143,62,170,134,9,183,57,16,228,232,48,128,160,217,252,138,60,242,254,4,5,61,0,191,26,158,113,7,84,101,166,26,112,202,89,48,242,122,0,52,2,106,43,140,5,85,223,140,25,51,116,7,51,152,163,0,114,249,37,20,10,11,154,191,212,99,87,114,8,163,114,50,
56,35,51,67,242,160,234,10,96,76,20,228,229,194,58,44,132,148,150,234,171,194,204,155,204,89,240,40,205,151,95,126,185,16,56,2,73,208,216,72,104,101,210,197,129,78,26,217,184,248,202,241,205,0,244,108,92,83,127,30,80,149,185,71,111,224,201,7,217,113,
10,35,4,166,205,3,195,238,224,158,126,244,51,14,250,31,135,155,131,184,109,156,140,36,64,52,32,216,215,209,113,107,66,86,70,80,242,161,226,8,86,33,84,98,17,250,178,210,210,82,248,65,154,79,19,118,241,143,192,208,146,228,26,224,171,175,190,170,67,2,110,
123,239,196,81,21,208,34,26,15,173,176,6,82,203,85,3,10,129,118,128,8,7,220,165,10,112,164,131,173,120,16,252,51,0,39,31,64,121,144,44,247,158,123,238,209,45,226,182,63,34,71,248,94,128,149,20,74,7,95,190,167,81,65,179,157,128,23,23,151,40,227,217,79,
117,199,177,172,248,70,193,29,100,216,2,168,214,37,159,219,137,227,13,7,247,143,132,127,20,215,220,205,196,122,116,154,1,247,250,205,165,2,112,84,49,218,82,161,122,120,84,197,116,72,75,24,155,88,125,55,223,124,179,96,47,100,236,77,28,171,174,24,166,3,
20,134,4,142,210,72,224,200,108,14,158,25,82,50,123,234,88,94,94,126,158,124,240,193,7,250,110,2,203,180,170,50,73,89,164,59,140,60,185,144,186,49,0,239,33,252,86,48,147,164,253,255,50,74,27,15,42,127,51,188,1,211,41,121,102,206,156,57,102,253,250,245,
224,7,102,32,227,118,30,107,68,244,31,38,129,117,107,58,119,42,115,227,43,183,164,247,135,187,237,182,219,8,128,193,160,59,113,15,74,178,223,118,211,236,149,81,116,82,205,110,24,144,70,163,149,132,164,93,8,111,32,37,30,164,141,210,103,254,254,247,191,
119,11,3,24,7,6,243,139,157,130,219,173,66,162,137,88,22,29,119,73,147,6,110,105,71,127,154,12,172,248,56,61,216,13,244,183,129,254,25,81,46,165,130,38,27,16,192,88,168,173,220,36,84,186,129,192,65,218,104,82,235,139,24,152,94,82,38,118,38,109,122,179,
159,255,217,103,193,64,49,103,157,117,150,130,7,213,27,15,82,103,215,122,16,28,52,198,114,208,143,85,93,117,58,150,137,94,247,123,48,160,133,239,135,90,237,31,112,63,0,192,238,66,200,202,134,208,186,149,30,190,11,71,35,227,64,59,246,113,104,11,58,8,39,
13,116,52,132,24,215,133,99,35,228,192,124,34,234,115,75,23,105,251,229,246,193,2,78,159,139,22,250,11,212,130,234,37,4,11,208,207,237,226,28,79,241,149,95,48,160,203,10,146,161,241,190,203,12,221,72,64,240,232,142,58,234,40,13,249,18,100,55,13,29,59,
28,248,54,104,255,26,50,83,229,15,88,127,119,48,128,35,34,156,198,58,10,76,250,223,202,29,157,142,140,0,197,193,54,221,126,198,80,209,44,145,65,49,25,109,125,236,70,23,23,4,155,227,64,78,38,115,134,100,231,206,157,88,104,221,130,165,160,77,177,119,20,
248,226,227,165,151,94,170,187,168,105,165,246,208,221,138,244,21,240,52,180,6,132,199,3,214,34,64,112,50,103,85,164,15,204,99,229,98,191,237,128,26,150,156,50,139,91,207,57,135,216,89,107,199,235,81,250,82,62,199,110,4,130,64,67,130,117,224,205,48,209,
113,202,138,91,17,56,15,25,239,9,30,77,126,222,167,231,208,130,239,137,83,242,207,62,251,108,185,243,206,59,117,222,146,207,96,3,97,217,188,166,79,226,248,96,130,85,4,169,187,21,229,157,137,107,74,30,235,153,52,3,226,123,229,186,214,71,189,42,182,211,
76,170,78,80,249,31,130,9,151,32,21,141,17,212,49,178,191,131,251,249,9,22,95,7,182,140,228,224,154,51,37,100,90,188,227,224,26,211,97,250,214,13,37,134,233,56,91,210,25,208,4,150,96,115,214,127,227,198,141,178,110,221,58,225,139,31,44,3,47,65,234,139,
37,4,145,135,2,208,243,189,114,206,143,190,254,250,235,178,98,197,10,237,115,89,6,233,226,96,157,227,199,253,128,71,176,56,171,178,25,105,184,23,137,124,254,220,2,199,214,72,160,120,98,208,253,240,214,250,114,8,2,37,142,167,34,140,25,61,70,215,200,40,
73,22,52,251,14,55,242,198,28,91,62,87,169,9,84,101,101,165,206,101,146,153,157,57,166,99,131,224,236,191,245,148,46,198,219,70,65,64,120,109,79,213,227,59,231,164,131,107,117,108,20,52,84,120,80,41,29,193,38,13,54,111,194,115,9,18,91,218,76,0,247,32,
194,122,120,214,191,223,192,59,144,18,167,132,163,178,215,161,178,39,161,18,33,84,202,199,138,147,225,4,142,51,244,84,83,60,193,149,76,99,171,39,227,40,145,157,57,222,39,176,218,250,193,42,238,56,73,230,248,28,150,67,230,51,207,224,193,120,117,4,158,
243,147,156,109,97,35,225,125,210,193,180,219,240,238,29,203,229,254,19,188,110,140,247,20,86,169,228,115,54,165,162,162,66,103,103,216,47,242,185,4,48,193,145,8,170,76,206,170,164,35,205,115,184,102,220,231,14,56,214,140,234,99,8,152,114,7,124,22,43,
12,70,113,3,142,26,9,108,201,23,93,116,145,30,95,193,107,74,7,1,37,19,247,231,20,48,36,96,186,206,64,139,207,175,233,153,22,222,74,33,135,30,148,94,134,4,139,125,33,1,164,74,229,89,40,52,84,120,46,10,129,198,251,232,122,220,6,27,21,143,225,160,186,165,
164,118,2,30,137,63,12,245,120,22,105,182,226,186,223,84,102,231,77,57,190,182,125,191,230,115,184,56,122,57,252,233,232,39,194,104,229,62,50,137,224,176,239,224,235,188,4,142,12,33,83,201,224,174,64,35,89,10,24,210,119,215,105,250,104,226,120,208,9,
34,85,40,45,72,54,26,210,196,190,142,253,30,223,205,227,27,172,60,200,134,210,201,97,11,251,85,2,201,60,188,78,66,43,137,98,215,192,3,110,138,225,57,17,221,111,238,64,0,199,10,80,218,210,32,97,183,97,122,170,148,149,229,230,27,50,135,158,86,221,31,254,
240,7,85,145,172,89,119,65,99,218,190,184,120,208,237,51,9,32,193,227,138,59,37,143,82,199,113,37,27,20,215,234,216,215,17,76,210,77,117,202,186,80,197,210,82,221,143,212,77,64,154,151,33,117,27,65,111,191,72,221,129,0,78,45,73,16,206,35,158,126,0,226,
13,152,224,82,210,56,184,165,85,247,235,95,255,90,37,142,12,176,12,236,11,32,189,201,107,37,198,62,159,128,80,125,210,138,108,110,105,86,117,249,206,59,239,232,9,68,52,166,172,97,67,105,163,163,186,76,226,172,212,177,171,200,69,217,143,33,100,92,159,
251,186,125,122,213,36,15,239,107,148,214,8,173,241,66,110,54,133,97,224,209,16,136,238,156,210,150,252,149,175,124,69,59,126,203,180,190,62,176,47,249,45,128,44,131,192,241,12,231,124,108,84,202,206,201,214,147,138,24,79,131,134,82,201,62,142,18,200,
254,144,113,28,182,36,1,208,242,248,108,72,243,44,100,39,63,250,44,48,182,80,210,51,16,142,229,147,208,225,168,144,78,254,161,143,112,9,32,91,50,221,141,55,222,168,39,217,165,2,104,74,80,194,63,90,145,60,106,35,47,55,79,15,116,227,109,106,10,130,68,199,
70,72,218,105,216,84,84,84,36,211,24,49,169,3,15,46,211,76,159,3,137,211,134,129,150,118,22,136,206,7,19,194,232,248,29,246,17,180,198,184,159,145,231,142,164,186,163,17,69,79,0,231,205,155,167,125,155,165,153,253,51,61,27,35,207,194,164,138,229,117,
188,228,34,173,21,144,211,113,93,9,207,198,108,227,112,217,115,215,167,204,93,60,142,45,141,99,25,46,239,159,207,86,73,139,140,33,85,11,29,55,231,80,197,48,46,161,162,122,63,85,254,145,62,58,90,155,231,158,123,174,94,179,14,236,159,177,136,171,191,57,
166,99,31,93,89,73,92,128,202,222,99,59,43,117,121,136,191,72,19,68,250,186,232,101,207,131,129,6,142,99,165,25,144,182,35,169,90,208,106,125,172,48,39,115,57,86,227,49,134,159,7,199,70,133,58,40,169,28,215,209,17,24,30,131,79,233,98,221,184,201,136,
150,37,193,237,196,17,60,186,175,194,243,232,35,246,21,54,14,151,61,115,3,14,28,90,171,74,27,42,68,233,211,254,129,225,197,23,95,28,219,114,151,202,210,70,90,227,29,13,18,58,74,27,45,203,120,218,217,40,57,156,160,179,64,235,143,200,63,242,154,95,96,226,
1,221,199,70,227,123,205,255,94,103,140,35,40,217,101,76,53,160,2,103,177,114,168,144,253,0,145,166,231,160,150,173,214,170,161,100,133,164,82,156,5,136,175,94,157,116,210,73,58,159,105,13,20,91,7,238,247,180,70,151,141,75,168,67,196,34,19,249,31,9,241,
61,254,217,249,172,108,143,139,218,43,3,27,4,55,209,156,133,112,36,42,237,161,53,186,108,133,156,161,167,9,205,105,164,222,58,29,91,193,192,161,33,112,160,156,5,142,115,157,180,36,233,108,63,102,65,226,196,51,193,164,79,98,160,104,22,254,67,250,227,17,
176,144,38,120,54,242,30,143,235,6,66,226,172,180,49,252,46,60,9,53,28,183,113,208,77,119,218,105,167,41,120,250,163,7,255,44,131,216,151,172,95,183,190,7,57,251,47,41,235,64,131,138,142,198,8,93,60,93,172,167,5,89,111,238,253,79,249,141,251,99,160,129,
38,71,111,145,79,61,118,3,1,156,150,137,74,93,8,106,102,66,202,60,12,100,125,28,231,216,138,206,156,57,51,214,106,247,83,201,78,43,195,57,196,237,59,34,214,92,167,137,6,232,6,165,140,199,223,211,89,85,201,107,198,115,26,172,190,174,62,89,255,198,36,214,
169,186,132,214,176,150,89,74,0,103,165,141,75,25,63,37,40,8,13,135,1,84,143,28,239,208,241,228,113,58,219,82,245,71,15,254,113,187,1,85,209,129,118,150,94,46,176,210,89,13,194,107,214,149,52,213,214,213,42,112,251,105,144,170,22,113,255,80,230,131,235,
177,154,100,166,254,150,56,45,15,173,239,187,32,108,10,202,103,235,242,113,246,129,253,145,157,215,99,7,223,23,199,190,196,74,175,101,102,95,202,235,110,94,251,44,206,85,210,17,28,11,16,251,93,174,243,209,91,3,165,147,114,85,194,80,214,88,220,207,132,
231,56,163,199,82,215,159,192,177,44,2,197,14,224,106,120,74,148,18,68,19,217,182,78,238,227,224,204,123,111,156,101,18,39,125,109,121,189,41,167,175,121,44,29,182,28,107,164,112,168,64,237,66,103,227,108,154,184,80,121,142,50,216,122,243,163,241,7,21,
56,125,56,8,190,2,68,113,176,67,16,241,51,210,54,108,200,105,33,90,102,125,113,173,45,123,230,10,251,82,78,111,243,90,201,179,249,45,144,236,14,108,191,103,227,108,154,36,97,9,226,40,113,189,114,253,37,113,4,141,64,241,104,192,189,166,116,56,4,224,192,
212,74,8,7,173,246,186,39,20,91,102,177,31,97,95,105,213,85,79,202,232,107,90,11,6,215,233,232,72,19,227,72,15,119,133,209,0,227,192,156,142,245,238,196,197,75,215,65,7,78,27,0,0,153,134,138,140,36,193,232,113,29,31,142,191,163,163,249,78,117,73,199,
202,90,16,52,162,135,255,216,127,132,194,161,88,121,61,204,222,47,201,217,16,233,88,15,219,8,43,42,42,212,56,225,194,43,53,138,5,185,139,7,118,138,110,23,249,250,109,167,173,109,69,65,30,217,196,227,149,10,32,123,60,98,137,93,111,115,125,53,42,50,92,
105,225,202,0,153,111,129,236,138,192,196,251,204,27,198,219,168,150,97,137,247,15,196,111,43,77,4,135,75,60,116,148,56,123,112,183,141,35,141,236,34,8,176,205,131,164,180,34,201,47,154,216,28,128,247,202,245,215,204,137,182,28,168,177,205,229,69,190,
186,45,53,146,215,184,147,244,144,70,60,98,247,118,25,53,186,65,151,69,168,234,226,42,209,99,162,153,151,224,117,179,69,247,184,252,238,100,224,243,233,56,197,197,5,85,46,247,176,94,203,151,47,143,169,76,90,190,118,229,128,180,82,181,83,165,226,90,129,
3,152,27,80,68,173,22,20,97,84,244,178,123,65,127,1,71,98,208,178,100,37,104,92,131,203,233,183,124,95,188,47,207,196,89,173,104,83,127,184,95,228,241,87,215,203,164,67,252,168,64,100,1,178,123,228,37,79,69,70,244,5,252,228,165,238,47,22,213,67,229,34,
206,81,213,207,107,2,70,75,146,146,197,213,1,106,19,26,95,28,231,49,196,135,40,52,237,194,133,11,85,50,169,66,1,158,7,250,121,122,195,71,40,130,135,220,176,155,209,134,143,176,219,174,223,128,123,237,53,241,131,159,161,171,47,144,127,93,254,85,153,62,
188,2,52,80,111,130,172,195,38,250,229,201,87,27,229,194,27,106,100,250,17,88,10,137,235,35,8,66,79,28,199,111,84,63,108,225,3,239,8,22,233,131,7,157,188,106,105,11,201,130,69,139,245,209,148,34,238,114,254,244,211,79,245,55,105,251,248,227,143,213,115,
172,106,23,95,175,190,250,106,61,67,229,245,215,95,151,226,130,76,183,3,198,77,91,135,60,23,21,220,158,49,64,159,212,79,255,176,241,76,39,237,86,220,17,152,214,246,182,175,198,44,247,25,111,161,207,243,22,164,153,240,130,0,54,97,99,45,117,73,134,249,
231,205,233,228,132,121,250,169,39,161,41,34,14,146,99,47,247,27,218,116,48,12,204,67,15,61,100,176,113,71,211,219,248,253,102,238,205,205,40,93,164,206,107,173,49,13,235,94,54,77,107,30,55,139,94,189,207,148,161,219,18,25,108,242,179,121,178,186,232,
203,143,144,186,216,187,115,104,88,26,207,123,244,151,92,114,137,249,214,183,190,197,107,72,22,121,224,223,126,202,60,156,179,31,113,7,13,56,160,162,141,81,234,95,243,189,103,86,249,0,86,48,100,22,151,0,176,17,240,35,77,120,97,145,49,11,161,108,62,202,
49,15,254,60,82,153,39,30,223,243,198,41,250,140,30,177,246,153,103,158,49,248,100,139,230,25,16,224,162,160,181,55,215,153,150,21,183,24,111,81,169,9,127,128,131,214,223,67,29,22,136,217,254,82,150,57,110,58,235,225,51,69,121,201,223,88,37,144,80,141,
10,220,160,65,165,230,136,35,102,152,49,67,10,67,39,206,16,115,250,145,242,87,98,54,127,126,239,103,174,250,172,42,1,8,85,100,199,199,119,187,87,229,148,152,89,210,144,22,114,125,197,48,167,56,59,130,254,12,135,89,187,56,11,217,132,176,47,31,103,38,159,
119,114,174,188,181,52,77,206,58,251,92,185,251,174,255,43,95,255,183,139,49,125,21,249,198,169,29,164,71,26,226,190,255,129,148,26,37,28,232,54,53,247,218,32,219,183,224,125,98,200,111,188,153,211,190,75,220,154,159,136,147,55,27,95,21,25,135,122,64,
177,24,236,232,42,219,38,83,198,172,149,87,22,225,28,103,156,90,27,73,189,119,33,164,85,135,13,96,78,113,142,11,85,185,65,94,249,243,110,247,216,89,69,24,206,56,151,152,112,245,107,193,163,4,95,103,210,227,90,122,220,199,233,248,107,239,71,118,255,23,
90,140,223,57,92,58,174,199,161,117,69,197,206,13,28,130,27,201,71,39,132,247,201,28,28,82,13,208,212,57,24,215,248,7,137,23,78,19,127,122,189,204,255,94,139,124,121,230,8,185,248,146,111,202,205,55,253,38,246,2,5,164,167,91,15,47,31,90,46,165,56,120,
134,174,167,125,100,183,30,16,81,32,210,214,184,59,178,145,206,195,184,205,224,52,163,48,14,110,246,117,224,156,48,79,214,210,4,67,109,57,111,222,153,174,11,3,209,97,69,174,172,88,179,75,174,254,74,149,204,155,85,224,56,25,195,188,64,254,8,191,47,80,
246,208,174,23,101,24,112,245,76,47,36,175,183,192,57,175,1,52,0,167,22,194,41,191,247,63,80,58,12,199,139,135,124,97,199,197,57,132,60,145,92,219,33,43,71,135,26,240,232,120,95,169,120,168,104,105,89,171,252,246,242,173,136,175,144,235,174,255,153,124,
231,59,223,209,23,11,237,152,135,173,53,153,179,32,17,180,190,78,84,39,43,63,49,174,106,195,50,105,215,97,90,51,142,160,106,65,35,65,195,130,20,174,92,186,65,62,93,8,233,131,38,108,229,104,44,137,163,93,86,0,54,236,214,177,186,39,223,56,29,162,149,129,
198,219,17,112,165,13,31,24,204,192,196,59,206,76,213,172,63,79,82,64,23,81,221,2,142,186,152,173,130,98,77,192,80,166,57,54,10,218,67,223,245,253,215,164,41,6,26,223,160,163,202,192,247,44,35,51,231,251,62,23,237,18,234,211,245,97,31,98,155,193,55,114,
60,185,243,218,141,72,54,82,95,168,56,246,75,95,210,113,16,193,161,239,12,60,150,75,243,123,64,87,191,241,124,186,250,218,6,105,215,217,45,160,195,163,241,189,13,56,14,248,51,217,190,1,111,254,144,229,0,53,157,218,83,83,239,249,199,223,4,45,12,223,84,
231,200,221,87,134,101,252,88,76,77,134,51,80,55,156,86,228,64,22,113,198,52,54,3,204,209,92,143,117,42,180,123,10,77,184,218,47,112,4,44,42,89,158,51,95,48,254,16,207,2,246,167,179,3,179,238,191,212,247,220,244,89,242,63,179,113,102,39,212,164,207,113,
121,134,191,226,154,240,24,254,68,117,8,170,131,26,179,102,120,242,5,39,57,114,244,228,181,56,114,173,66,86,227,53,38,126,101,152,175,52,209,17,188,238,170,78,205,208,159,255,208,66,119,86,239,146,109,216,141,166,163,14,126,231,192,224,51,48,33,188,4,
210,228,195,38,33,71,206,158,29,121,224,46,224,80,132,106,229,98,70,47,7,30,39,48,74,17,230,208,119,35,190,30,195,235,127,63,199,200,201,199,161,33,164,225,16,128,48,190,192,21,66,125,195,187,48,158,105,151,156,2,89,162,165,156,183,15,246,93,214,134,
92,78,234,64,187,242,14,55,217,241,12,126,240,66,201,207,46,243,205,66,91,57,14,13,126,68,48,232,29,61,116,184,72,249,72,0,10,251,67,12,154,30,140,144,88,191,150,180,84,160,229,226,60,72,131,85,226,80,131,100,229,57,242,215,235,125,50,225,130,245,50,
118,100,153,172,90,187,77,240,133,70,185,253,246,219,117,23,24,7,172,4,175,43,163,37,233,163,122,25,73,73,175,134,40,124,178,106,53,194,157,210,206,174,212,180,130,102,28,212,141,70,201,175,127,102,97,58,111,198,33,34,191,68,149,31,124,71,100,101,100,
94,121,175,39,206,29,41,114,62,228,105,238,81,70,138,74,129,100,71,13,140,51,28,138,74,7,201,149,80,62,142,37,46,174,210,227,157,127,17,137,238,201,255,164,192,69,65,51,127,58,57,112,232,161,243,188,239,149,143,48,167,22,20,56,67,176,153,87,130,200,209,
129,158,173,5,42,36,152,1,109,128,15,254,66,175,1,48,16,199,111,0,168,133,75,145,74,230,172,212,229,195,148,194,39,195,58,140,140,31,35,242,31,63,241,201,21,183,108,147,49,195,113,10,172,228,233,217,145,252,188,243,79,127,250,83,221,194,103,213,38,91,
210,64,59,206,252,239,4,112,187,176,103,178,185,25,111,19,65,216,32,42,84,1,208,103,70,248,45,165,124,216,94,60,3,250,80,144,83,6,5,178,165,26,19,233,144,176,102,244,103,25,144,184,2,236,97,26,135,70,93,81,137,23,2,71,186,0,27,55,217,252,245,37,92,178,
220,11,73,102,157,191,165,218,124,13,63,126,38,167,129,135,243,53,5,126,118,207,237,3,220,124,168,71,106,169,191,94,232,159,51,251,75,222,171,147,103,10,223,181,133,120,163,64,76,236,131,250,112,0,255,3,217,4,44,10,26,159,229,128,218,78,251,183,120,98,
40,117,216,52,106,144,222,171,83,134,124,245,84,124,100,251,25,159,124,240,105,29,196,186,73,119,128,253,249,207,127,150,215,48,29,115,223,125,247,197,246,120,16,192,129,6,143,243,144,52,227,219,96,46,118,160,133,182,129,231,232,189,149,5,172,69,22,200,
46,64,119,165,147,66,144,56,52,92,41,199,80,218,78,228,112,255,80,6,20,79,118,62,226,71,64,117,230,161,149,115,172,237,167,165,29,105,216,198,49,174,131,47,143,228,228,174,251,198,234,187,90,255,4,203,188,138,147,24,206,249,202,229,120,102,117,122,13,
46,238,237,230,70,68,70,166,30,97,190,61,249,72,128,134,115,61,165,13,35,210,14,168,10,204,237,154,144,227,55,33,140,60,81,25,0,7,98,208,196,92,172,250,186,104,122,252,150,141,70,238,93,230,222,191,144,137,210,137,244,142,139,226,33,117,249,200,126,243,
149,76,229,74,73,190,167,103,253,87,84,84,168,177,194,87,139,249,137,76,186,129,6,141,207,224,180,149,31,94,183,70,160,15,192,244,35,0,228,195,81,51,144,238,167,68,225,220,218,34,168,208,146,65,248,62,25,48,41,68,88,50,4,214,114,57,150,255,135,66,10,
135,137,140,128,170,44,25,140,51,53,3,184,240,35,210,133,168,146,63,16,2,158,221,32,237,217,33,95,241,200,161,69,195,229,31,124,46,65,195,235,119,128,189,123,110,31,224,100,110,36,99,17,182,249,171,214,11,25,72,92,129,235,248,43,241,220,81,240,35,163,
225,104,212,114,20,60,8,227,151,164,56,110,235,137,83,9,45,68,21,192,13,48,230,232,233,34,55,124,3,199,173,110,198,171,61,37,62,89,191,126,189,110,131,163,245,200,83,126,158,124,242,201,158,148,222,235,180,180,88,121,102,115,26,214,15,93,0,72,85,217,
214,10,212,240,167,30,228,98,138,82,138,32,117,165,0,107,16,192,26,2,92,134,0,172,114,168,199,97,149,34,67,43,49,232,46,197,210,83,0,137,56,166,213,198,76,93,137,204,49,231,249,165,41,43,84,48,108,220,209,205,111,202,205,140,198,75,185,97,59,125,24,75,
214,201,197,190,192,189,17,73,137,237,129,208,7,240,46,148,130,139,38,166,18,133,94,217,129,135,129,17,243,24,159,69,44,201,72,190,110,255,103,235,211,129,122,54,6,230,48,64,32,132,23,159,21,201,93,211,132,53,125,124,161,136,155,139,104,160,208,241,29,
58,187,76,98,251,188,72,234,254,253,79,73,227,228,177,15,0,114,46,15,198,159,180,182,0,53,240,156,216,89,23,192,164,16,251,186,82,72,28,193,27,28,245,148,194,108,178,135,178,67,62,197,114,197,231,142,150,194,15,220,181,229,152,140,193,227,175,174,127,
53,240,23,198,82,242,48,27,5,185,222,191,219,23,184,104,122,24,84,70,218,240,48,210,12,221,24,113,108,49,201,124,244,118,143,2,114,2,82,234,150,170,213,232,193,216,169,168,20,249,199,111,68,26,208,202,135,160,173,80,228,121,116,5,223,181,230,226,228,
54,188,186,59,208,142,234,152,71,219,135,177,202,30,238,104,197,162,45,37,142,11,161,9,79,38,249,224,13,85,39,142,130,86,207,107,198,97,180,142,127,252,209,165,22,226,128,21,45,35,203,228,12,25,247,157,134,215,242,223,90,122,187,20,112,54,202,96,181,
133,93,122,194,83,99,63,247,1,174,106,130,34,35,117,59,205,230,54,74,29,88,43,94,21,136,129,62,235,188,28,77,216,171,127,42,189,248,76,38,251,229,144,35,167,206,21,57,15,126,13,38,86,202,139,112,148,6,154,182,221,95,194,45,16,3,233,98,146,12,238,239,
218,181,91,234,170,209,80,192,186,118,44,229,16,192,189,171,15,134,83,37,17,36,50,159,63,233,53,17,216,202,126,95,5,71,35,121,163,51,23,1,175,45,35,156,93,81,57,103,226,225,67,222,218,241,188,76,113,142,229,161,110,40,185,19,240,246,1,238,188,143,35,
143,111,170,117,222,173,171,199,67,177,115,68,12,24,198,239,179,69,110,117,70,64,47,227,161,83,80,73,199,197,153,203,152,136,14,194,106,187,230,82,22,133,73,105,127,150,100,103,166,203,134,13,27,180,236,24,99,123,249,164,238,102,163,148,191,250,250,155,
152,221,90,0,186,112,202,17,63,209,137,118,181,199,145,47,232,34,92,244,237,250,185,51,88,42,14,38,213,217,111,107,67,100,215,2,179,210,206,213,238,201,216,217,21,154,7,212,102,189,47,228,230,150,77,44,29,62,118,73,219,123,185,215,35,49,206,129,73,14,
222,62,192,253,34,90,116,211,198,240,154,232,118,10,20,202,100,0,79,85,38,126,246,171,35,19,104,101,14,86,67,133,159,130,59,116,138,39,255,231,135,217,178,121,123,157,84,97,6,99,206,156,163,229,143,127,252,163,158,205,220,175,143,142,43,76,155,118,148,
75,47,191,252,178,60,120,207,93,146,159,139,241,166,11,130,208,136,246,200,13,36,140,32,249,134,194,163,131,195,252,43,105,87,3,141,161,195,223,48,74,186,86,147,113,79,183,151,198,143,238,9,155,106,242,76,218,208,202,27,27,223,40,124,10,119,146,130,7,
11,33,185,107,169,135,158,221,235,22,127,237,29,179,215,237,190,254,80,43,19,189,61,231,4,165,72,190,118,114,181,184,67,255,93,142,57,225,84,41,47,27,132,189,28,96,72,212,245,247,176,128,160,89,224,248,193,248,31,252,224,7,96,126,1,6,206,237,48,50,240,
90,112,116,183,90,228,241,52,170,32,81,106,148,69,249,161,195,32,196,219,79,197,88,66,123,21,26,172,66,123,198,169,73,243,178,74,11,79,49,239,238,158,224,28,41,203,112,42,21,165,39,38,247,157,2,135,79,180,5,212,50,210,135,83,202,96,70,169,232,15,4,120,
40,51,106,101,114,58,204,132,92,193,65,230,114,242,148,26,41,27,51,94,178,51,246,28,120,214,223,160,217,41,53,150,187,108,217,178,216,57,149,35,128,77,90,160,89,63,56,8,91,69,191,152,165,13,151,86,245,62,210,212,191,60,1,45,17,5,201,121,95,108,67,37,
4,231,41,14,123,254,237,163,42,203,182,70,6,129,67,143,18,95,150,110,215,36,81,180,144,104,150,239,147,124,79,73,253,113,165,3,84,244,29,6,115,72,78,185,12,11,254,86,22,189,240,55,193,216,191,223,7,223,4,140,82,102,151,146,56,78,228,185,93,234,178,74,
100,92,126,141,228,100,184,250,205,30,154,254,145,70,12,30,114,218,138,147,14,3,169,125,96,166,73,192,113,189,166,150,134,230,157,178,67,105,138,218,30,17,2,247,69,194,7,139,155,84,73,81,177,111,86,33,13,35,46,240,114,144,165,211,53,253,221,191,89,50,
108,8,67,197,225,17,135,176,135,48,147,158,94,48,88,202,219,46,147,247,94,127,213,38,232,83,104,85,34,11,33,96,108,216,27,54,172,151,249,243,231,203,153,103,158,41,5,216,137,60,172,20,51,64,77,85,82,146,135,85,0,84,155,210,166,192,105,155,37,31,128,226,
222,230,101,159,104,138,207,76,227,20,101,135,176,228,227,151,188,102,124,182,124,219,175,178,78,144,173,136,247,57,243,117,84,29,75,174,98,24,253,229,27,205,145,116,110,68,52,199,79,148,227,36,27,64,53,122,56,91,31,228,199,8,238,95,181,16,163,68,153,
193,134,65,14,145,169,176,191,67,45,50,106,164,200,138,103,142,147,143,138,150,201,180,169,147,84,74,108,30,50,62,153,35,64,137,142,105,227,211,211,82,197,222,21,185,226,138,43,52,41,207,202,220,186,117,27,186,41,76,78,66,233,228,96,8,70,192,2,24,163,
113,19,54,27,83,68,210,20,193,196,226,251,244,155,228,130,182,48,118,70,184,146,101,252,225,221,181,205,173,85,219,126,149,61,175,253,183,184,71,178,99,125,155,125,144,5,142,212,224,171,162,226,15,230,170,170,244,99,38,32,157,31,147,85,141,224,192,178,
226,56,78,251,56,50,139,126,95,230,216,66,123,31,218,178,89,2,174,209,223,209,114,155,52,102,187,60,254,31,87,72,240,170,191,97,65,114,204,94,197,39,130,148,8,144,77,76,213,200,83,18,248,205,83,90,141,63,255,249,207,245,22,167,184,184,55,146,39,210,114,
142,210,207,143,232,194,13,67,31,199,1,53,65,243,99,86,93,235,174,75,87,188,203,186,147,214,190,185,72,41,142,167,182,77,48,228,243,106,155,164,118,123,237,163,235,55,84,93,55,253,155,130,79,191,236,213,214,246,122,24,129,35,5,10,220,49,199,139,255,154,
255,148,182,167,127,29,152,130,81,247,151,35,111,40,67,187,115,28,23,222,140,84,48,131,241,33,88,85,155,250,180,120,70,147,140,190,58,219,154,109,89,84,153,219,37,35,103,136,28,146,255,134,92,121,254,137,114,201,181,55,201,236,89,179,244,184,166,196,
83,15,236,211,185,52,195,205,169,244,124,231,156,103,77,126,186,242,83,185,247,190,123,99,31,171,229,158,71,110,58,34,152,60,44,71,39,149,81,64,59,250,83,186,2,116,101,17,208,208,118,124,168,39,165,91,223,26,179,180,69,210,245,230,63,199,212,90,154,139,
209,97,160,13,115,150,13,178,125,221,238,247,151,46,109,248,229,137,215,201,243,44,243,185,63,73,16,143,196,124,82,114,23,15,156,91,140,217,39,36,243,198,140,49,249,144,60,124,159,196,54,45,62,6,173,63,12,53,66,139,138,11,166,92,123,99,191,167,6,5,139,
97,86,164,211,16,129,186,158,84,18,105,85,162,57,201,23,205,135,226,60,140,75,59,66,91,49,105,91,42,51,135,172,147,175,224,147,45,116,151,94,122,169,238,20,230,187,118,124,15,129,146,199,105,49,130,197,227,123,185,49,213,174,166,107,134,232,63,238,48,
102,255,70,64,249,130,6,175,233,233,136,15,38,73,164,4,93,25,55,92,251,80,45,126,236,83,169,137,253,227,5,61,235,218,149,139,79,195,53,49,228,194,76,139,227,195,203,15,130,165,244,214,122,255,103,203,119,109,88,177,194,220,124,198,175,228,126,108,0,111,
157,127,229,103,249,59,183,212,134,78,249,129,130,198,2,244,201,137,79,138,7,206,105,199,225,168,72,224,34,68,39,163,73,249,53,74,114,82,153,170,170,150,210,103,48,11,28,3,137,29,0,106,169,150,22,129,180,215,44,26,62,166,94,147,62,159,37,199,185,136,
113,18,23,129,57,67,128,135,175,48,26,108,29,24,62,36,79,38,85,230,201,242,117,27,245,144,235,248,116,201,174,169,54,249,162,33,37,147,142,239,142,115,190,147,32,243,158,5,204,230,13,2,36,2,55,21,99,107,124,202,90,219,32,169,214,169,71,229,31,197,17,
30,249,85,2,147,243,52,90,28,211,177,107,162,245,138,245,48,116,83,142,139,73,79,105,50,94,99,131,111,229,202,246,142,143,151,201,127,158,247,7,193,228,114,241,230,175,157,221,156,29,174,91,148,57,255,86,238,100,209,130,173,36,36,101,156,5,78,31,182,
249,99,13,210,255,242,183,208,250,223,92,231,174,201,171,112,70,99,161,154,114,135,129,20,192,180,69,40,225,172,4,9,227,204,2,128,100,28,193,244,136,56,193,164,100,70,165,147,67,9,53,110,44,45,72,178,143,99,69,65,179,54,10,251,160,200,2,37,214,139,177,
112,9,131,33,216,40,121,186,252,159,129,197,203,28,116,200,88,126,71,191,68,16,108,95,199,133,80,190,92,65,233,227,203,244,156,190,34,96,22,44,62,150,233,19,29,159,24,237,222,176,15,18,99,56,114,6,145,4,13,31,136,68,72,198,179,175,39,95,121,147,117,98,
155,142,47,139,25,64,172,206,48,177,46,228,77,27,158,7,141,23,110,13,55,215,118,248,182,111,54,206,71,75,156,119,238,252,125,240,166,231,36,240,225,156,57,225,244,226,80,117,209,253,143,171,132,145,161,241,5,198,95,227,214,30,71,10,98,238,197,21,226,
157,62,83,130,183,191,46,213,147,167,185,223,56,126,166,247,232,208,65,206,144,14,20,215,30,210,186,83,203,171,138,70,229,93,85,37,208,47,28,227,240,83,151,12,213,59,156,144,102,37,235,192,113,86,142,149,68,167,161,139,137,108,202,209,246,66,178,20,112,
130,214,140,180,152,53,161,74,38,199,192,92,110,15,192,199,25,35,82,71,206,194,69,180,90,187,212,226,211,208,45,224,81,103,46,14,28,205,153,248,27,249,44,83,108,200,71,170,75,71,187,163,218,36,105,156,92,230,212,95,176,197,96,101,27,196,168,99,145,0,
3,75,149,160,40,18,165,214,58,65,35,88,4,55,194,131,80,71,200,52,213,123,94,205,14,199,247,241,114,183,105,241,71,206,239,127,246,130,251,176,12,111,107,63,190,184,173,104,213,50,105,123,187,78,19,71,107,24,45,174,139,32,218,174,34,178,132,105,81,121,
234,3,9,159,54,91,178,190,119,107,104,21,36,231,212,59,254,205,251,110,81,161,57,30,135,222,141,194,199,124,157,160,109,104,160,23,93,139,231,3,237,252,22,183,31,195,47,152,207,244,152,205,199,247,185,225,121,205,205,53,142,11,80,40,149,80,121,145,25,
24,220,80,240,16,232,163,45,208,12,233,240,150,39,234,222,0,12,27,97,234,182,225,218,78,242,70,121,139,6,228,224,120,165,72,234,253,252,79,38,226,201,114,81,125,196,28,56,232,96,21,210,225,118,4,110,120,229,246,133,38,98,4,13,19,204,104,70,227,33,157,
140,96,125,44,112,81,237,3,245,16,198,250,98,59,182,32,54,214,27,111,87,149,227,214,110,119,124,159,172,150,5,15,191,32,55,190,180,53,180,114,230,56,201,104,192,249,3,47,47,86,117,197,103,91,208,24,218,107,92,238,117,205,223,49,23,15,156,87,11,254,64,
75,152,103,222,147,208,188,41,18,92,42,29,213,223,190,87,174,7,145,127,185,106,174,55,30,223,214,62,50,63,75,14,11,166,155,225,89,25,206,80,0,233,234,88,7,56,16,64,12,27,13,64,52,4,23,187,209,156,116,0,205,253,23,233,216,206,22,76,231,105,231,4,145,173,
17,156,136,39,79,201,1,63,129,4,85,19,140,66,84,58,226,227,193,131,202,50,232,214,153,147,235,230,201,0,136,85,44,238,130,28,165,117,70,79,142,51,63,243,178,238,236,252,64,45,246,217,224,31,247,145,208,53,130,68,108,87,48,240,14,37,158,235,109,42,233,
232,6,240,225,44,52,200,54,212,5,243,152,209,57,76,74,102,8,51,187,29,237,88,194,196,83,154,27,29,54,186,112,67,173,227,219,182,5,171,250,235,229,175,87,63,238,222,137,82,91,15,27,30,200,93,177,174,163,173,17,39,37,226,81,22,52,62,57,254,58,254,55,73,
218,199,217,202,179,18,251,248,137,67,36,109,80,161,4,222,173,18,175,117,135,118,104,108,98,254,25,69,254,225,71,79,50,35,115,50,205,208,130,92,103,70,118,186,169,8,4,157,210,236,116,41,226,52,153,130,137,202,250,131,98,96,117,155,140,44,0,8,207,29,82,
0,83,239,211,90,83,137,1,5,172,56,61,23,43,169,30,155,97,255,16,56,130,166,192,97,250,178,29,59,192,215,110,22,247,199,143,33,159,43,141,105,142,172,4,175,54,129,30,164,150,70,168,194,22,140,213,216,42,172,111,66,156,5,140,33,117,24,65,164,136,176,174,
212,217,217,232,251,120,20,97,25,194,17,233,174,25,217,212,33,101,147,74,157,65,63,58,89,28,172,223,154,172,60,172,249,99,122,18,244,43,237,56,237,73,27,41,186,214,168,218,6,221,40,149,128,181,66,177,52,129,26,30,233,213,218,32,190,205,91,100,247,146,
85,230,87,183,188,225,189,52,178,44,144,145,134,198,176,178,154,53,84,208,8,156,245,108,80,246,218,134,140,35,189,73,157,5,142,149,33,40,86,254,25,178,114,10,212,112,88,220,144,180,0,118,96,251,195,248,238,235,114,156,216,142,123,81,38,16,38,9,30,85,
238,13,159,61,198,84,66,165,86,148,228,200,12,124,103,182,34,15,199,26,243,184,45,206,62,164,1,64,0,167,32,66,34,29,170,87,72,104,172,95,177,192,209,16,32,3,168,34,163,210,230,181,55,137,187,27,221,223,63,22,152,109,207,125,226,60,51,36,71,158,221,218,
16,94,133,231,178,146,116,108,161,244,214,217,107,214,43,222,251,49,116,208,6,10,160,88,55,222,163,211,186,227,31,198,223,166,176,169,221,84,220,118,161,115,101,249,32,41,77,207,6,120,248,160,49,53,7,247,154,112,80,110,65,99,195,83,105,3,39,184,227,185,
5,189,1,214,122,195,29,141,226,219,184,205,172,126,234,93,231,218,103,214,132,87,30,86,30,200,222,88,35,237,213,205,29,29,60,230,11,234,151,188,139,7,200,94,39,134,182,30,74,100,252,63,11,28,227,88,9,86,202,130,167,21,196,111,5,143,33,233,54,120,111,
163,32,32,129,162,204,128,31,70,149,31,218,1,195,7,227,110,216,141,117,164,8,49,200,103,130,99,178,77,233,89,51,205,212,193,88,136,199,251,224,51,243,115,164,48,7,45,55,0,197,4,99,208,139,50,193,165,49,99,251,42,50,193,26,3,148,186,38,168,27,244,47,190,
170,221,102,221,179,255,146,187,159,90,225,190,115,88,73,104,227,226,42,113,209,90,2,104,186,201,42,102,235,196,48,254,190,11,208,98,117,1,112,172,31,127,91,143,45,246,198,87,1,26,63,171,113,107,126,113,138,119,217,196,10,249,186,63,93,66,233,152,161,
231,54,60,54,182,136,65,182,55,205,218,23,182,193,188,106,130,240,129,222,13,219,204,71,151,63,228,94,133,84,85,83,112,58,200,210,205,84,211,220,141,26,3,204,242,42,17,168,248,223,20,142,78,157,173,164,77,16,171,24,34,246,1,14,113,241,247,181,194,209,
22,236,43,196,38,223,252,28,195,25,36,167,30,253,247,166,90,18,73,85,101,210,70,103,154,146,115,143,52,51,202,139,157,99,240,9,210,195,11,115,37,131,42,147,79,64,122,15,13,128,214,170,210,66,181,131,22,105,176,187,138,91,15,221,205,59,204,226,7,222,118,
175,126,111,75,104,237,225,195,36,107,225,38,128,22,105,181,182,146,201,42,152,88,47,219,40,45,205,26,178,62,113,146,199,56,55,15,245,216,213,42,222,184,124,147,243,195,147,204,159,202,138,157,225,216,143,21,166,138,164,180,161,161,57,236,239,248,0,182,
10,244,201,248,98,36,222,6,192,216,55,12,37,189,105,167,188,249,253,71,156,107,145,162,113,82,177,4,150,67,53,34,43,247,28,17,44,11,152,13,227,193,180,245,97,152,172,78,136,222,227,18,43,200,59,241,149,180,173,210,134,177,10,35,29,227,152,214,198,225,
26,111,162,4,34,93,64,14,222,144,43,203,54,126,42,234,85,53,32,56,228,176,239,9,158,54,206,27,58,27,135,195,22,231,153,99,178,50,157,169,185,24,37,4,193,16,214,132,18,71,110,112,220,138,33,152,108,173,150,255,247,195,199,220,235,209,90,235,38,150,4,50,
86,84,117,180,65,234,177,145,32,166,170,45,3,162,57,145,63,218,0,244,42,242,47,190,62,150,86,5,41,42,129,122,141,164,76,231,226,133,87,183,178,64,124,235,106,156,182,243,15,245,70,29,51,214,92,87,152,227,76,204,128,166,136,169,246,40,215,52,192,147,217,
191,213,54,74,77,85,173,115,239,53,79,57,247,80,186,198,228,7,124,171,107,21,52,190,160,68,58,45,173,137,215,137,224,117,9,26,171,149,12,56,27,207,10,37,3,44,190,242,182,210,54,180,64,2,192,0,6,98,176,58,177,43,19,221,131,91,142,195,243,208,50,205,170,
93,180,236,176,125,74,2,57,151,206,246,70,143,27,44,71,194,58,29,231,247,153,161,200,156,3,203,174,45,20,114,182,236,110,52,207,255,236,25,239,9,208,224,141,41,18,223,106,228,3,239,104,125,176,226,172,156,101,128,101,72,98,133,89,183,68,208,44,125,182,
14,246,55,213,40,175,53,15,173,201,74,76,182,175,171,215,193,104,198,117,39,120,115,75,242,156,153,48,168,6,251,92,147,131,16,237,7,4,120,210,20,246,156,237,181,205,102,225,219,31,187,175,188,248,89,104,227,224,108,73,135,132,153,141,141,248,152,33,232,
4,166,241,116,146,70,11,148,141,183,191,25,178,1,118,203,117,6,156,205,108,43,102,43,26,31,218,123,241,33,203,99,26,203,52,203,12,254,118,178,81,163,76,236,51,45,207,15,56,91,155,59,194,59,208,97,35,158,222,55,42,51,144,63,105,184,9,214,54,57,29,111,
108,234,216,133,184,214,161,185,146,201,17,194,166,58,173,44,43,109,1,179,33,43,111,175,45,3,88,121,75,147,109,120,246,183,165,159,191,109,156,189,182,161,3,181,232,64,93,59,149,144,154,117,205,88,198,109,231,248,69,159,3,219,50,16,60,108,16,135,173,
98,222,223,161,125,23,102,25,148,190,224,228,33,226,223,176,75,194,245,109,251,165,215,2,102,67,210,205,58,244,200,117,5,156,45,204,86,146,21,79,188,182,21,142,15,45,112,10,24,242,216,144,229,233,51,75,240,25,111,26,43,57,96,18,102,41,188,197,155,117,
140,165,21,56,180,76,159,33,27,208,79,226,117,37,198,89,112,8,74,60,88,54,222,198,105,126,62,4,46,158,206,120,192,108,29,226,233,181,215,137,116,11,233,44,204,193,219,14,160,179,166,67,194,107,49,52,66,217,164,195,228,193,58,30,85,172,239,90,152,93,13,
18,222,1,122,163,35,84,166,177,222,210,22,31,242,218,254,238,182,148,33,79,204,41,19,99,191,186,190,176,21,179,149,183,97,103,21,103,60,243,216,231,216,112,175,39,97,14,216,96,249,11,147,193,232,47,0,34,38,248,205,78,88,104,136,178,158,149,179,215,54,
180,21,231,239,253,85,158,207,76,6,28,227,45,221,54,100,92,162,71,148,206,186,226,248,139,200,104,93,87,182,240,76,188,163,104,118,67,35,128,94,75,67,124,104,233,100,104,105,181,33,227,250,228,72,100,111,93,98,197,227,127,39,86,222,254,78,124,22,227,
45,211,25,38,250,206,24,97,211,37,150,215,213,111,11,80,98,104,233,99,188,189,102,72,103,195,200,175,200,255,206,104,238,140,94,27,111,243,197,151,213,171,235,100,68,245,170,32,100,138,175,48,175,227,153,192,50,237,179,108,200,56,58,91,25,11,198,254,
194,72,142,254,249,31,79,111,60,173,241,241,124,82,34,189,246,233,251,163,179,223,129,178,15,181,97,103,68,217,251,253,17,198,63,35,254,218,150,109,129,235,236,183,141,63,16,97,60,125,157,93,199,211,219,217,245,128,211,250,223,12,238,70,210,82,169,25,
10,0,0,0,0,73,69,78,68,174,66,96,130,0,0 };

const char* projectIconLinuxMakefile_png = (const char*) temp_binary_data_28;

//================== projectIconVisualStudio.png ==================
static const unsigned char temp_binary_data_29[] =
{ 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,128,0,0,0,128,8,6,0,0,0,195,62,97,203,0,0,0,1,115,82,71,66,0,174,206,28,233,0,0,4,166,105,84,88,116,88,77,76,58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,120,58,120,109,112,109,101,
116,97,32,120,109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,88,77,80,32,67,111,114,101,32,53,46,52,46,48,34,62,10,32,32,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,
100,102,61,34,104,116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,10,32,32,32,32,32,32,60,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,
32,114,100,102,58,97,98,111,117,116,61,34,34,10,32,32,32,32,32,32,32,32,32,32,32,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,109,109,47,34,10,32,32,
32,32,32,32,32,32,32,32,32,32,120,109,108,110,115,58,115,116,82,101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,115,111,117,114,99,101,82,101,102,35,34,10,32,
32,32,32,32,32,32,32,32,32,32,32,120,109,108,110,115,58,116,105,102,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,116,105,102,102,47,49,46,48,47,34,10,32,32,32,32,32,32,32,32,32,32,32,32,120,109,108,110,115,58,120,109,
112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,34,62,10,32,32,32,32,32,32,32,32,32,60,120,109,112,77,77,58,68,101,114,105,118,101,100,70,114,111,109,32,114,100,102,58,112,97,114,115,101,84,121,
112,101,61,34,82,101,115,111,117,114,99,101,34,62,10,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,62,120,109,112,46,105,105,100,58,100,98,101,99,56,57,51,56,45,56,49,54,56,45,52,52,102,101,45,97,55,
50,102,45,101,51,48,55,48,102,100,99,55,101,51,53,60,47,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,62,10,32,32,32,32,32,32,32,32,32,32,32,32,60,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,62,97,100,111,98,101,58,100,111,
99,105,100,58,112,104,111,116,111,115,104,111,112,58,55,100,55,51,53,51,48,56,45,57,52,100,102,45,49,49,55,55,45,97,53,100,98,45,56,53,99,49,100,48,98,53,54,97,53,50,60,47,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,62,10,32,32,32,32,32,
32,32,32,32,60,47,120,109,112,77,77,58,68,101,114,105,118,101,100,70,114,111,109,62,10,32,32,32,32,32,32,32,32,32,60,120,109,112,77,77,58,68,111,99,117,109,101,110,116,73,68,62,120,109,112,46,100,105,100,58,49,51,54,56,69,69,54,67,52,67,55,57,49,49,69,
52,57,54,50,67,65,49,51,66,54,69,53,52,48,69,51,54,60,47,120,109,112,77,77,58,68,111,99,117,109,101,110,116,73,68,62,10,32,32,32,32,32,32,32,32,32,60,120,109,112,77,77,58,73,110,115,116,97,110,99,101,73,68,62,120,109,112,46,105,105,100,58,49,51,49,68,
69,70,50,65,52,67,55,57,49,49,69,52,57,54,50,67,65,49,51,66,54,69,53,52,48,69,51,54,60,47,120,109,112,77,77,58,73,110,115,116,97,110,99,101,73,68,62,10,32,32,32,32,32,32,32,32,32,60,120,109,112,77,77,58,79,114,105,103,105,110,97,108,68,111,99,117,109,
101,110,116,73,68,62,97,100,111,98,101,58,100,111,99,105,100,58,112,104,111,116,111,115,104,111,112,58,51,97,99,50,101,99,98,55,45,57,52,100,102,45,49,49,55,55,45,97,53,100,98,45,56,53,99,49,100,48,98,53,54,97,53,50,60,47,120,109,112,77,77,58,79,114,
105,103,105,110,97,108,68,111,99,117,109,101,110,116,73,68,62,10,32,32,32,32,32,32,32,32,32,60,116,105,102,102,58,79,114,105,101,110,116,97,116,105,111,110,62,49,60,47,116,105,102,102,58,79,114,105,101,110,116,97,116,105,111,110,62,10,32,32,32,32,32,
32,32,32,32,60,120,109,112,58,67,114,101,97,116,111,114,84,111,111,108,62,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,67,32,50,48,49,52,32,40,77,97,99,105,110,116,111,115,104,41,60,47,120,109,112,58,67,114,101,97,116,111,114,84,111,
111,108,62,10,32,32,32,32,32,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,10,32,32,32,60,47,114,100,102,58,82,68,70,62,10,60,47,120,58,120,109,112,109,101,116,97,62,10,181,212,82,31,0,0,23,34,73,68,65,84,120,1,237,93,9,152,84,
213,149,62,221,93,93,91,239,27,2,141,40,224,2,35,160,40,26,53,162,99,148,209,68,65,109,149,65,176,49,40,102,198,4,197,104,62,51,126,25,53,126,201,231,18,162,142,27,78,136,138,34,42,162,81,65,34,8,10,74,80,135,168,24,81,65,140,226,2,210,108,246,222,181,
87,247,252,255,173,122,221,85,213,85,45,93,253,94,219,93,220,203,247,168,183,221,229,157,243,159,115,207,61,247,220,219,34,58,105,10,104,10,104,10,104,10,104,10,104,10,104,10,104,10,104,10,104,10,104,10,104,10,104,10,104,10,104,10,104,10,104,10,104,10,
104,10,104,10,104,10,104,10,104,10,104,10,104,10,104,10,104,10,104,10,100,32,5,178,210,249,166,182,182,54,27,242,29,140,35,27,71,91,58,101,232,60,166,82,128,124,168,201,202,202,106,233,110,169,100,100,58,105,168,215,235,125,7,25,157,56,90,211,41,64,231,
49,141,2,20,98,155,221,110,175,198,239,179,221,45,53,93,0,16,113,249,56,236,221,173,80,191,111,25,5,210,226,37,25,153,78,10,35,83,32,157,140,58,143,101,20,8,165,83,114,186,0,72,203,118,72,167,129,58,143,181,20,72,75,109,116,213,164,220,220,92,201,201,
201,233,234,21,253,172,7,20,8,135,195,18,12,6,123,80,66,124,86,211,1,192,198,249,124,62,129,69,26,95,147,190,234,49,5,48,250,50,93,184,76,3,0,25,238,112,56,100,217,178,101,178,104,209,34,41,42,42,82,141,101,163,117,234,25,5,72,219,214,214,86,169,171,
171,147,169,83,167,74,85,85,149,210,2,188,215,211,100,42,0,216,208,154,154,26,89,191,126,189,12,24,48,64,108,54,155,104,0,244,148,69,162,180,41,85,255,174,93,187,100,194,132,9,146,157,157,174,233,214,185,45,166,1,192,40,218,229,114,73,121,121,185,148,
150,150,106,13,96,16,165,135,191,20,44,2,32,20,10,137,219,237,238,97,105,241,217,205,131,82,180,92,45,241,241,4,54,251,202,108,250,154,14,0,179,63,88,151,103,45,5,52,0,172,165,111,159,47,93,3,160,207,179,200,218,6,106,0,88,75,223,62,95,186,6,64,159,103,
145,181,13,212,0,176,150,190,125,190,116,13,128,62,207,34,107,27,168,1,96,45,125,251,124,233,26,0,125,158,69,214,54,80,3,192,90,250,246,249,210,53,0,250,60,139,172,109,160,6,128,181,244,237,243,165,107,0,244,121,22,89,219,64,211,167,131,147,53,151,33,
98,140,18,170,173,173,85,129,13,156,222,212,41,57,5,56,219,71,250,148,149,149,137,211,233,84,211,192,201,223,52,231,174,229,0,224,199,180,180,180,72,69,69,133,76,156,56,17,65,34,185,10,4,230,52,63,243,74,97,176,71,56,28,146,141,27,55,202,238,221,187,
133,241,21,86,166,94,1,64,67,67,131,140,30,61,90,230,204,153,35,249,249,5,0,0,163,202,117,74,70,1,2,192,227,241,200,141,55,222,40,91,183,110,85,1,32,102,132,126,37,171,139,247,44,7,0,43,201,206,206,82,209,44,12,15,108,69,100,139,207,231,239,87,65,163,
140,196,233,173,228,206,139,68,252,4,81,167,17,250,101,101,151,217,43,0,64,84,155,162,31,37,63,28,110,85,96,176,242,163,122,139,89,86,212,19,14,129,70,12,246,236,165,96,90,61,10,176,130,139,253,168,76,13,128,126,196,44,43,154,170,1,96,5,85,251,81,153,
26,0,253,136,89,86,52,85,3,192,10,170,246,163,50,53,0,250,17,179,172,104,170,6,128,21,84,237,71,101,106,0,244,35,102,89,209,84,13,0,43,168,218,143,202,212,0,232,71,204,178,162,169,26,0,86,80,181,31,149,153,209,0,48,123,37,109,63,226,235,126,55,53,99,
1,192,25,72,155,77,239,85,244,93,72,232,165,217,192,239,106,134,121,207,41,245,217,57,217,98,203,181,169,41,231,28,27,103,31,91,165,173,181,45,114,96,99,83,61,19,217,65,239,140,2,0,153,159,67,230,219,193,124,78,65,35,254,32,43,139,96,136,156,115,42,154,
193,21,10,12,120,87,3,161,151,2,66,58,240,102,229,89,60,243,219,251,255,232,30,85,100,118,164,75,200,81,243,237,12,76,105,13,227,97,244,121,52,100,193,202,6,246,201,178,51,66,3,48,118,130,146,159,107,207,85,68,110,103,126,12,201,99,239,229,32,236,138,
54,66,91,78,27,64,208,26,13,192,96,12,198,129,167,21,250,61,0,200,52,246,249,93,49,63,6,7,113,0,97,200,21,143,28,148,113,160,118,15,189,2,0,171,250,218,174,152,207,103,118,167,93,108,217,54,9,183,97,119,77,95,80,90,219,90,227,250,125,190,195,100,116,
15,232,68,84,204,34,187,6,101,43,224,185,178,37,50,56,138,221,210,97,32,9,75,66,114,247,208,8,8,204,163,36,153,23,81,251,52,248,34,234,91,113,51,122,238,116,57,37,216,18,148,173,27,62,23,79,157,87,120,237,116,59,149,182,48,24,31,251,190,186,135,50,35,
218,196,6,141,98,147,28,14,35,209,85,100,114,178,68,3,144,217,70,120,179,223,239,87,235,2,154,155,187,253,183,12,82,210,221,96,190,26,234,129,253,177,12,229,185,11,140,14,249,194,178,248,150,151,100,245,130,55,101,204,105,71,200,132,169,199,203,191,156,
122,132,148,14,46,86,155,220,7,67,1,9,5,147,132,167,43,187,16,64,136,118,15,44,47,224,55,111,111,222,148,31,245,61,61,48,29,0,36,28,55,53,228,162,6,174,108,153,62,125,186,112,3,233,205,155,183,136,215,235,145,60,119,94,143,62,85,49,223,134,161,29,118,
33,37,208,82,49,255,201,155,151,202,138,121,175,163,27,200,149,77,171,62,145,143,95,255,84,134,142,174,148,227,39,29,45,199,79,30,43,149,35,7,74,174,205,46,161,214,144,4,3,193,136,1,24,51,20,48,202,205,210,26,160,123,252,226,42,32,110,23,123,234,169,
167,202,85,87,93,165,86,3,17,20,219,182,109,19,123,174,67,168,17,34,221,65,247,202,229,219,16,70,168,101,24,124,0,84,228,58,210,135,27,231,84,241,148,252,39,111,126,81,150,223,187,70,138,74,11,196,93,200,229,85,173,18,240,6,229,203,15,190,145,207,223,
253,90,94,131,86,24,119,246,81,114,226,133,227,100,196,184,67,196,149,239,194,159,61,193,59,190,128,234,178,98,219,23,1,28,134,142,8,215,182,58,25,160,179,186,158,216,242,77,213,0,236,239,185,97,244,204,153,51,229,186,235,174,147,202,202,74,213,255,83,
35,12,31,62,92,188,88,31,24,240,7,34,210,6,233,237,78,82,140,160,181,15,15,31,83,44,177,120,78,230,135,189,97,121,234,150,165,242,215,40,243,157,249,142,118,53,111,119,229,10,143,48,212,126,211,158,102,121,229,161,55,228,173,37,239,201,200,31,14,151,
19,47,56,86,70,255,235,145,146,63,0,139,50,66,240,24,118,224,170,59,77,236,151,239,154,6,0,50,33,16,8,200,228,201,147,229,210,75,47,21,252,13,27,181,32,148,160,160,68,113,117,13,127,29,184,207,243,48,220,179,49,26,183,75,226,197,51,191,179,218,87,204,
167,228,223,210,33,249,100,62,199,248,70,162,247,143,137,134,93,94,169,91,220,197,46,241,183,248,229,189,229,31,201,123,43,62,150,17,39,12,149,89,247,76,149,67,143,30,34,62,175,207,200,150,241,191,166,141,2,200,36,30,37,37,37,106,147,104,174,111,227,
117,162,58,37,211,109,246,28,117,144,186,124,167,171,196,231,202,189,75,201,79,209,231,135,149,218,143,151,252,88,230,199,150,207,242,12,48,56,11,156,82,62,164,68,236,14,155,108,125,243,115,217,243,229,62,52,15,13,236,158,114,138,45,190,223,157,155,166,
1,140,47,55,250,248,88,198,27,207,248,171,64,129,95,46,25,87,154,33,24,66,191,27,15,20,227,125,197,252,46,12,190,142,62,127,41,250,252,215,84,159,159,40,249,70,89,201,126,9,132,86,48,219,238,176,43,71,82,174,211,116,114,36,171,182,79,221,51,77,3,24,95,
101,48,158,204,203,206,201,82,203,155,29,46,59,92,238,29,146,78,161,87,207,97,28,210,131,71,9,79,76,17,230,67,83,68,103,245,120,109,36,158,187,220,46,137,72,62,213,126,247,153,111,148,197,95,150,23,91,126,236,179,76,63,239,76,121,147,190,216,225,114,
160,191,119,200,87,31,238,144,186,111,26,196,237,114,11,135,84,49,124,84,68,39,96,20,8,208,55,27,140,224,59,236,171,105,240,81,37,199,50,135,231,100,190,191,37,32,79,220,248,188,252,245,190,181,82,12,107,191,59,146,111,210,39,102,68,49,166,2,128,204,
161,39,141,155,26,4,91,66,178,226,193,55,228,247,231,223,47,15,92,177,80,190,249,100,151,56,29,78,53,140,75,100,40,41,201,153,58,122,223,216,1,43,15,95,10,107,159,204,247,53,5,228,209,235,23,203,242,7,35,67,61,205,252,244,177,104,42,0,148,177,150,147,
43,255,252,191,47,228,190,203,30,149,199,111,120,78,60,251,124,242,209,218,79,229,174,105,127,150,77,107,182,40,173,192,62,55,17,4,188,166,93,224,128,227,38,55,151,145,60,241,106,153,207,35,204,247,131,249,207,200,171,143,188,37,101,3,138,197,145,103,
87,227,252,244,73,112,96,231,52,13,0,84,229,244,1,188,183,114,147,220,62,109,158,252,125,217,7,82,80,146,39,37,7,21,74,197,144,82,217,249,201,110,185,103,250,35,178,18,227,239,172,54,104,9,72,178,74,29,93,123,123,151,192,41,24,240,59,46,185,41,249,141,
100,254,18,89,179,0,204,63,168,68,104,91,164,178,246,217,221,240,89,107,55,134,155,113,21,30,32,23,166,1,0,60,67,202,146,175,54,237,144,61,95,127,43,229,149,165,17,199,11,25,128,84,86,89,34,33,111,72,30,187,254,89,28,75,164,105,111,139,234,42,148,171,
53,134,217,17,205,208,113,131,215,156,200,241,54,129,249,191,122,6,204,127,19,204,47,142,148,29,51,206,87,149,68,255,99,55,20,128,141,208,82,239,85,147,57,134,97,26,251,142,62,143,80,192,60,0,68,41,202,161,89,129,221,173,164,153,195,59,38,50,145,210,
88,84,81,40,121,133,110,121,25,62,250,123,103,62,42,59,182,212,168,249,130,28,168,252,8,227,163,133,68,127,120,175,67,237,27,146,95,44,185,240,232,37,147,124,50,154,204,111,216,219,44,126,111,64,206,158,125,154,140,60,101,132,120,27,189,73,203,143,175,
237,192,188,50,29,0,36,99,187,252,66,43,196,198,223,209,37,236,44,112,72,249,224,18,249,96,245,22,153,59,101,190,108,124,249,67,204,17,216,209,247,119,182,11,226,153,79,201,47,73,201,124,21,225,3,192,236,219,81,39,246,60,155,204,248,227,133,50,237,119,
231,1,112,78,241,53,7,226,28,82,7,38,171,147,127,181,37,0,48,170,162,68,182,130,41,45,245,30,53,153,66,233,164,228,82,226,7,12,41,147,221,159,239,147,123,103,44,144,165,119,173,146,54,204,184,178,159,135,168,42,105,53,152,191,64,245,249,6,243,109,200,
223,14,47,163,26,37,245,1,95,72,246,110,175,149,161,71,13,146,57,11,47,151,137,179,38,168,153,62,79,147,79,249,35,218,95,214,39,113,20,176,20,0,28,171,15,25,117,144,140,159,52,86,154,190,109,81,99,119,130,192,112,197,150,85,22,227,92,100,209,111,94,148,
135,127,249,140,52,236,105,130,202,135,159,30,64,240,54,250,148,193,199,153,187,210,118,201,143,103,190,161,242,91,26,60,82,183,183,65,78,158,114,156,92,255,204,149,114,244,25,163,196,31,244,171,105,105,221,255,199,241,187,211,133,165,190,79,95,179,95,
92,240,183,207,188,231,98,201,47,205,147,151,31,128,211,6,118,128,11,19,53,156,162,109,131,52,23,148,229,65,173,219,100,245,195,127,147,221,159,237,149,153,119,79,145,146,193,69,178,240,215,207,201,154,199,223,86,204,231,44,94,98,159,111,204,211,215,
213,52,72,118,110,182,76,249,239,115,100,242,117,19,161,254,115,165,190,174,65,114,49,156,140,120,24,227,65,211,137,2,7,248,13,203,0,192,65,1,251,101,79,131,79,108,240,177,87,223,94,37,217,240,235,191,124,255,90,197,204,188,34,184,114,1,2,50,214,233,
182,75,197,224,82,217,178,238,51,153,55,107,33,0,80,44,31,174,249,68,74,7,20,41,107,63,145,249,212,34,225,64,88,106,119,214,73,197,176,114,153,246,251,243,228,135,83,198,139,31,179,145,141,181,77,106,50,71,75,254,254,33,219,50,0,196,86,239,247,250,197,
93,238,2,8,46,80,1,26,47,220,254,138,10,188,160,159,32,2,2,120,16,1,142,210,33,197,178,243,211,61,178,253,227,26,165,25,24,252,17,199,124,160,138,193,37,212,44,245,251,26,101,236,143,70,74,245,29,85,50,124,220,80,105,193,236,99,0,129,159,134,49,24,91,
191,62,79,77,129,94,1,0,213,181,47,224,83,253,253,148,155,206,21,55,164,127,241,77,203,164,225,219,102,41,68,23,64,195,206,176,11,242,75,92,145,81,4,52,183,49,140,100,243,149,68,195,168,108,194,16,143,65,37,103,95,117,154,176,172,194,138,124,105,108,
104,84,239,146,249,58,117,143,2,189,2,0,54,137,32,240,54,113,60,46,50,105,206,153,202,54,88,248,95,127,145,230,90,143,228,67,19,48,112,132,137,207,59,37,48,158,142,166,134,221,141,24,70,218,229,146,219,38,203,196,43,39,96,198,8,247,234,26,241,8,255,248,
142,78,221,166,64,239,0,32,202,84,170,111,63,162,109,218,192,236,51,47,63,5,210,159,47,79,98,4,208,136,16,45,250,7,58,28,8,9,223,1,135,18,135,115,71,156,52,92,166,252,246,28,25,117,242,8,241,160,156,0,38,133,12,99,48,33,135,190,220,79,10,88,58,12,76,
108,3,113,64,73,229,58,1,166,67,198,86,74,94,137,91,197,237,81,138,147,38,220,166,71,48,20,8,201,65,35,202,101,216,209,7,227,53,148,129,110,64,167,158,83,160,119,52,64,180,157,148,124,174,220,45,40,200,151,79,55,108,147,71,230,60,35,53,91,247,72,1,98,
244,146,185,130,85,54,160,134,86,127,62,108,133,183,22,191,43,123,182,237,149,234,59,171,100,216,49,48,252,90,60,0,2,22,157,88,222,247,199,207,76,246,156,236,201,75,32,13,248,175,55,83,239,0,128,82,12,53,206,249,254,124,48,127,211,107,91,100,254,236,
167,193,204,125,82,6,183,48,212,66,71,231,143,83,106,131,88,64,240,156,32,112,229,59,229,67,196,247,207,253,247,249,50,253,119,231,43,199,79,16,101,122,154,49,233,195,92,200,107,118,98,221,92,131,192,53,4,177,109,50,187,30,163,60,78,149,115,87,117,118,
151,169,251,68,227,237,158,255,246,74,23,160,152,143,88,126,50,127,195,139,239,203,3,51,31,151,189,95,214,74,197,193,165,146,133,176,49,53,2,0,243,200,100,122,15,155,106,35,171,136,98,37,155,239,228,56,114,84,158,250,157,141,242,208,207,158,80,35,137,
144,39,44,69,37,133,81,12,153,47,61,29,93,147,249,101,119,102,31,23,169,98,217,122,130,65,108,37,240,44,215,0,100,28,37,40,39,43,71,94,127,226,109,21,36,226,111,10,170,104,92,62,227,199,25,67,188,198,125,205,24,17,184,165,226,144,82,169,249,231,94,248,
12,92,136,32,226,250,194,8,241,13,160,148,14,44,82,243,11,207,223,185,82,182,127,82,35,151,222,118,129,12,62,252,32,105,106,110,86,113,255,157,9,219,189,59,84,36,156,195,160,20,50,198,97,245,234,213,242,220,115,207,169,176,54,134,170,225,145,101,137,
75,215,185,136,117,251,246,237,42,194,186,157,62,22,213,104,41,0,232,234,117,22,58,196,153,7,34,194,213,203,8,33,134,225,150,12,42,82,82,207,143,35,145,201,224,111,119,212,74,241,192,2,184,130,47,150,161,99,42,229,193,89,143,203,230,191,125,38,3,14,46,
83,147,57,6,8,216,69,210,121,228,46,118,171,153,193,191,47,253,135,236,130,11,185,250,142,11,100,220,89,163,225,111,240,171,225,166,25,244,34,48,121,124,253,245,215,178,242,149,149,82,84,8,207,36,130,88,173,148,72,3,92,133,133,133,106,170,220,88,87,97,
198,247,36,43,195,50,0,80,72,232,2,166,26,127,233,127,94,149,151,254,248,154,100,35,18,168,0,142,27,195,187,199,168,97,90,247,251,224,210,61,116,204,16,153,121,215,197,50,250,244,35,85,59,255,99,222,116,128,96,33,86,247,126,161,64,64,191,190,161,26,249,
2,203,160,65,89,129,89,69,26,146,247,86,47,144,170,95,159,37,63,158,125,58,34,145,242,177,72,5,35,13,37,169,61,55,12,184,198,177,188,172,28,198,107,129,90,150,102,37,0,212,199,71,255,139,253,222,216,251,102,158,91,106,3,208,227,247,213,63,118,202,210,
59,95,197,42,235,44,41,40,239,96,62,25,234,243,248,21,243,199,159,51,86,174,127,122,150,98,190,31,30,67,175,215,139,89,196,65,50,251,209,203,176,100,235,8,204,241,215,170,233,228,88,155,128,68,96,151,64,9,45,197,172,34,67,191,22,253,102,41,140,203,167,
164,97,103,147,184,157,46,60,235,152,121,236,9,209,12,77,208,23,126,123,242,29,201,242,90,166,1,140,202,66,254,144,90,161,203,69,23,148,90,18,145,46,91,26,122,94,143,79,126,252,243,211,100,234,45,231,97,182,208,141,181,131,240,20,70,251,123,158,87,30,
57,80,174,6,8,254,244,159,79,202,198,85,155,165,2,97,102,54,204,252,181,119,7,168,132,210,200,174,134,78,37,187,43,32,107,31,123,91,190,217,188,75,102,252,225,66,172,251,59,76,5,141,38,139,33,48,218,183,63,191,148,68,46,103,51,150,183,245,150,6,48,218,
70,154,209,56,100,253,102,215,109,41,0,216,216,28,123,54,6,104,29,210,202,143,169,197,20,110,14,150,135,77,195,80,238,220,57,103,32,64,4,179,134,88,58,78,139,155,207,153,8,4,46,39,175,24,90,38,87,205,175,150,121,176,250,223,95,245,17,64,80,166,84,191,
209,141,168,151,241,31,237,2,7,166,141,25,139,248,217,187,95,201,125,51,22,200,180,59,206,87,35,11,14,63,217,183,70,139,54,178,236,247,47,87,59,213,215,215,171,46,136,43,147,205,102,194,119,53,196,0,0,255,240,38,181,163,153,201,116,0,144,49,36,144,193,
72,246,195,236,138,41,245,148,220,189,80,231,12,4,153,113,231,133,114,242,197,199,97,131,166,16,52,65,231,37,227,42,63,50,242,131,203,176,126,239,231,127,174,150,249,191,120,90,222,93,190,9,190,3,196,5,98,61,95,162,100,179,124,14,37,203,49,188,108,193,
28,195,130,107,150,168,56,4,46,6,77,39,241,59,40,121,199,28,115,140,92,123,237,181,106,68,16,25,159,167,83,90,207,242,176,45,92,122,127,194,9,39,152,170,9,76,7,64,8,4,107,12,98,56,231,207,83,171,117,84,63,13,230,7,209,21,212,214,212,201,225,39,12,83,
65,31,71,252,96,152,4,67,65,181,57,67,59,88,18,105,164,84,7,65,224,81,81,197,191,120,120,134,60,114,205,98,89,191,228,29,41,27,132,248,64,71,231,191,66,106,72,39,153,30,240,32,50,184,206,131,174,161,123,78,28,2,150,109,98,89,148,254,99,143,61,86,198,
143,31,159,216,186,239,229,154,109,226,159,225,53,43,153,6,0,50,186,21,155,49,141,255,201,88,249,244,237,109,242,254,203,155,85,100,110,81,121,1,86,242,64,133,214,54,202,137,85,227,228,167,127,184,8,227,252,50,241,249,125,237,54,65,151,31,163,122,132,
72,23,81,8,35,242,202,7,46,81,93,202,58,130,96,32,86,246,38,137,22,82,90,7,132,82,140,199,133,1,138,46,235,73,241,144,64,232,216,227,40,197,75,189,120,187,39,223,146,172,153,166,1,128,133,251,177,195,198,33,71,13,150,107,31,187,66,214,62,254,150,10,255,
222,241,89,141,56,224,70,157,116,245,25,50,229,230,72,44,128,209,143,165,148,252,36,45,165,125,64,59,129,198,226,172,7,167,73,22,130,69,214,61,189,1,11,79,176,58,40,197,2,17,179,136,213,27,195,177,36,159,220,43,183,210,5,0,181,100,210,228,197,52,173,
3,115,246,63,185,250,71,114,36,166,109,95,184,103,165,12,27,51,84,206,191,238,223,84,236,158,215,3,35,38,98,231,37,205,223,213,77,130,128,229,231,33,104,228,103,15,76,83,49,5,175,204,95,39,37,8,29,99,88,25,13,65,157,186,71,129,116,1,192,124,249,169,170,
242,33,4,140,227,252,17,199,13,149,107,30,190,28,86,62,134,110,248,231,79,98,236,165,42,163,171,251,4,145,11,241,254,151,205,189,72,245,213,43,254,244,134,148,32,216,148,30,199,196,209,65,87,229,100,216,51,4,84,116,63,165,11,0,90,33,111,227,224,150,95,
195,113,196,129,129,170,157,210,200,61,129,184,196,59,140,217,173,16,54,130,72,84,249,184,110,128,154,222,134,252,76,137,122,129,90,134,127,181,121,48,142,74,28,237,137,229,208,135,192,85,72,151,97,188,207,156,43,1,130,98,65,196,49,64,96,178,38,96,27,
62,199,209,128,131,68,78,169,253,240,236,251,72,32,71,150,13,116,220,151,78,229,233,2,96,39,156,18,147,48,36,202,197,138,222,51,80,249,28,84,126,124,108,3,200,36,146,138,123,236,241,92,93,199,190,32,242,37,46,111,66,57,43,241,140,169,147,87,18,198,151,
31,110,216,209,40,255,110,188,219,169,124,95,20,4,52,44,185,68,124,217,61,171,149,255,192,141,80,244,253,5,1,234,85,109,83,155,71,195,144,77,146,246,161,254,235,49,26,88,143,125,143,184,162,53,233,75,73,242,245,214,45,69,59,76,184,213,166,83,97,90,0,
64,141,220,51,237,219,104,133,79,194,168,123,11,68,154,141,251,87,224,183,40,182,33,36,112,146,244,14,238,255,10,251,8,172,75,242,44,241,214,250,230,230,230,89,0,219,35,120,16,55,22,99,217,10,4,88,60,202,101,96,140,44,126,97,238,43,138,69,236,34,82,117,
7,108,18,241,198,89,55,6,148,48,148,156,219,201,210,163,152,36,133,80,79,125,105,105,41,53,0,143,140,74,157,164,46,157,175,3,35,191,192,113,3,172,229,105,200,191,22,71,82,74,70,203,126,30,204,156,182,159,204,87,89,242,243,243,55,161,236,203,113,177,33,
90,70,251,143,2,1,12,67,6,136,78,189,121,146,84,221,112,150,52,54,54,99,61,130,87,57,133,218,95,196,9,223,229,4,20,29,72,30,44,24,173,135,71,178,25,203,214,134,142,29,34,23,221,120,142,12,63,118,168,114,76,37,180,62,7,249,156,177,229,100,210,121,82,241,
236,201,7,54,54,54,150,193,93,58,27,154,224,74,148,19,219,119,51,136,111,30,84,233,173,216,73,172,62,157,58,224,0,57,18,64,120,8,121,79,79,204,143,250,176,192,20,93,52,166,155,95,156,187,82,158,189,109,5,166,83,237,146,143,105,99,122,8,41,245,116,70,
181,128,241,33,127,88,197,34,142,154,48,66,198,159,59,6,235,11,70,41,71,83,48,12,199,20,247,53,142,55,71,106,0,128,106,0,246,181,196,58,51,225,218,116,0,24,68,65,183,112,10,152,242,75,92,87,129,128,52,246,110,5,17,31,196,121,143,162,57,1,130,195,0,130,
255,69,185,103,24,117,25,191,6,8,24,124,242,226,221,171,100,241,111,151,169,109,105,184,45,93,83,93,179,218,119,104,208,225,3,212,218,193,227,206,25,35,135,159,56,76,77,84,193,133,37,1,48,94,109,38,97,20,214,241,187,11,26,171,26,182,200,171,29,183,50,
231,204,50,0,144,68,212,6,48,78,102,128,233,187,193,252,167,204,34,27,65,0,31,253,67,40,247,204,196,50,9,2,181,77,188,2,193,106,89,116,235,11,226,180,57,100,212,73,135,201,15,170,142,198,244,242,72,25,56,162,66,101,227,62,193,33,196,13,68,52,68,74,82,
52,163,204,75,242,242,242,150,39,214,149,9,215,41,191,186,175,127,28,52,204,161,96,204,189,104,231,228,196,182,26,154,0,130,45,235,150,108,80,59,140,140,61,125,164,228,161,59,160,121,18,8,6,212,78,165,124,15,32,74,204,30,123,189,11,239,172,130,6,248,
45,0,252,69,236,131,76,57,239,242,235,251,250,71,98,55,210,33,104,227,125,56,46,72,108,43,153,203,105,96,70,243,50,133,219,184,43,56,230,211,57,212,163,137,154,250,203,233,78,252,0,199,90,28,127,1,227,223,3,72,252,56,207,200,148,154,12,253,228,115,1,
2,26,154,247,227,232,4,2,126,130,33,225,4,68,87,9,239,209,78,89,131,95,170,250,181,153,42,241,137,52,232,247,0,224,7,97,158,124,16,24,55,23,167,211,19,63,240,59,174,131,120,254,37,142,229,200,191,12,221,202,251,209,241,254,119,100,203,156,199,25,1,0,
178,3,6,103,57,188,146,115,193,200,159,238,7,123,106,33,237,27,241,30,125,18,43,96,225,111,71,62,58,183,14,184,148,49,0,32,231,8,2,140,58,232,54,174,78,193,201,109,96,244,106,12,35,95,114,187,221,235,112,142,221,36,14,236,148,81,0,32,43,33,217,3,49,68,
60,9,167,156,184,49,164,154,223,25,2,227,183,193,73,245,49,24,79,213,175,147,166,128,166,128,166,128,166,128,166,128,166,128,166,128,166,128,166,128,166,128,166,128,166,128,166,128,166,128,166,128,166,128,166,128,166,128,166,128,166,128,166,192,129,64,
129,255,7,47,12,150,8,60,209,161,194,0,0,0,0,73,69,78,68,174,66,96,130,0,0 };

const char* projectIconVisualStudio_png = (const char*) temp_binary_data_29;

//================== projectIconXcode.png ==================
static const unsigned char temp_binary_data_30[] =
{ 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,128,0,0,0,128,8,6,0,0,0,195,62,97,203,0,0,0,25,116,69,88,116,83,111,102,116,119,97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,3,40,105,84,88,116,88,77,76,58,
99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,112,97,99,107,101,116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,32,60,120,
58,120,109,112,109,101,116,97,32,120,109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,54,45,99,48,49,52,32,55,57,46,49,53,54,
55,57,55,44,32,50,48,49,52,47,48,56,47,50,48,45,48,57,58,53,51,58,48,50,32,32,32,32,32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,34,104,116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,57,
57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,
58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,109,109,47,34,32,120,
109,108,110,115,58,115,116,82,101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,115,84,121,112,101,47,82,101,115,111,117,114,99,101,82,101,102,35,34,32,120,109,112,58,67,114,101,97,116,111,
114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,67,32,50,48,49,52,32,40,77,97,99,105,110,116,111,115,104,41,34,32,120,109,112,77,77,58,73,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,49,51,
54,56,69,69,54,70,52,67,55,57,49,49,69,52,57,54,50,67,65,49,51,66,54,69,53,52,48,69,51,54,34,32,120,109,112,77,77,58,68,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,49,51,54,56,69,69,55,48,52,67,55,57,49,49,69,52,57,54,50,67,65,
49,51,66,54,69,53,52,48,69,51,54,34,62,32,60,120,109,112,77,77,58,68,101,114,105,118,101,100,70,114,111,109,32,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,49,51,54,56,69,69,54,68,52,67,55,57,49,49,69,
52,57,54,50,67,65,49,51,66,54,69,53,52,48,69,51,54,34,32,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,49,51,54,56,69,69,54,69,52,67,55,57,49,49,69,52,57,54,50,67,65,49,51,66,54,69,53,52,48,69,51,54,34,
47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,32,60,47,114,100,102,58,82,68,70,62,32,60,47,120,58,120,109,112,109,101,116,97,62,32,60,63,120,112,97,99,107,101,116,32,101,110,100,61,34,114,34,63,62,9,144,57,89,0,0,67,215,73,
68,65,84,120,218,236,125,9,188,93,85,117,247,218,251,156,59,189,41,19,153,19,2,97,8,132,36,24,32,8,40,115,65,171,88,81,180,22,1,91,43,159,210,138,165,98,171,182,180,95,173,126,85,106,21,75,213,90,252,172,84,40,173,56,97,157,80,64,20,48,130,65,166,144,
64,18,8,153,200,60,190,228,13,119,56,103,239,111,173,181,199,115,19,126,95,127,242,46,190,103,121,249,221,220,247,238,61,247,156,115,247,94,123,13,255,245,95,107,11,173,53,252,170,63,141,70,3,86,175,94,13,47,230,28,47,255,188,248,159,217,179,103,195,
196,137,19,127,165,207,166,47,230,194,27,54,108,128,87,189,234,85,160,148,58,212,219,226,55,104,140,71,181,132,223,116,211,77,112,249,229,151,191,244,2,64,19,63,48,48,208,62,233,194,14,152,104,19,4,49,74,38,82,31,66,80,197,11,76,180,136,94,211,109,191,
143,154,159,86,171,245,43,127,54,125,177,23,151,82,58,13,224,6,50,1,33,74,160,5,62,131,164,67,94,80,0,200,116,8,241,146,205,189,144,169,72,123,166,84,240,154,194,93,63,27,216,89,215,42,211,209,61,169,82,42,242,84,66,150,183,84,179,169,33,63,196,120,169,
23,16,166,95,203,143,120,17,99,152,142,212,61,216,137,78,232,118,230,94,254,149,47,214,166,29,119,170,86,13,161,113,168,116,174,133,202,64,39,85,193,35,38,5,189,6,198,119,144,230,211,244,37,120,52,115,109,212,7,30,100,92,11,51,198,90,217,103,252,156,
72,131,60,209,235,244,253,249,125,43,130,42,83,32,18,17,132,12,236,181,146,84,166,221,147,43,218,45,108,252,76,171,127,123,67,235,76,9,105,142,81,90,231,147,106,98,168,170,242,65,53,184,125,143,104,110,219,156,236,125,118,85,115,199,211,143,175,95,253,
196,242,157,251,6,247,225,135,75,145,32,168,177,96,38,94,42,1,192,129,209,165,218,228,185,199,244,204,94,116,76,222,24,118,211,7,57,78,92,95,77,66,119,69,192,214,3,185,153,119,97,39,217,9,2,29,173,89,26,104,33,134,73,70,161,160,69,42,203,2,114,92,146,
210,222,53,31,34,33,18,16,205,231,210,237,250,70,91,1,16,248,94,150,249,223,89,131,85,250,252,177,218,126,126,0,207,181,159,180,90,247,60,212,112,0,229,57,25,76,174,12,235,215,92,180,254,201,236,217,59,111,125,224,135,95,191,101,243,246,61,187,240,35,
101,124,100,145,70,248,31,41,0,65,245,155,1,233,214,89,93,233,172,129,191,54,121,69,243,82,193,129,157,214,155,194,148,158,4,54,236,172,67,53,21,160,98,11,155,153,185,103,85,205,2,96,221,8,97,4,128,231,38,51,218,67,229,224,87,189,80,58,152,106,17,107,
4,59,165,36,16,185,153,92,153,136,200,218,43,43,52,25,107,2,62,78,27,13,36,18,141,95,134,254,206,65,55,50,24,198,11,174,221,215,16,107,178,137,139,102,77,191,226,31,46,185,246,188,119,172,184,243,115,31,186,247,167,15,220,21,9,1,140,38,179,240,223,54,
225,35,168,1,156,0,116,105,165,82,158,64,154,12,28,116,122,224,194,131,41,221,40,0,189,137,147,9,176,243,13,164,126,5,10,4,253,147,206,158,217,213,45,98,127,193,41,139,48,223,65,189,211,229,26,202,42,100,97,236,162,54,231,17,180,218,149,213,18,116,79,
246,120,17,9,32,153,28,115,31,218,8,129,213,80,2,157,1,242,115,74,137,196,47,167,224,249,29,253,240,245,167,202,11,143,186,232,47,238,184,252,237,151,190,207,158,161,108,23,147,24,107,209,143,28,193,115,9,59,8,85,28,240,68,90,27,44,34,251,61,165,79,66,
34,205,228,248,1,79,160,56,81,198,2,152,99,236,164,145,0,201,146,61,159,123,143,151,187,21,26,43,100,194,10,140,247,137,180,123,15,39,49,53,239,187,243,179,20,230,164,21,140,0,122,85,198,147,46,88,235,240,217,181,61,119,154,224,61,36,80,174,36,144,202,
28,190,185,108,103,165,247,148,119,124,230,79,174,254,227,15,218,111,80,26,139,66,48,210,62,128,245,3,104,33,11,72,74,18,109,191,134,38,218,239,33,180,221,199,76,73,97,211,222,156,127,23,96,85,62,249,102,218,168,221,102,110,78,80,78,141,67,152,181,140,
239,128,46,164,17,24,60,62,207,140,253,166,143,86,75,230,184,225,58,250,8,40,40,101,252,27,231,9,234,45,197,102,66,209,36,39,177,150,64,213,46,141,80,101,185,50,215,86,198,60,209,235,37,60,111,78,145,1,125,30,125,128,28,143,83,116,83,120,125,214,58,74,
66,43,67,227,160,36,223,248,205,119,61,11,215,189,245,205,127,123,237,53,66,222,112,227,231,63,25,57,135,217,88,113,12,211,14,104,20,153,55,112,16,235,56,178,82,3,250,124,112,194,225,21,168,225,208,28,117,88,10,179,198,37,176,165,191,6,83,251,18,232,
197,55,251,135,52,172,221,221,130,129,97,5,167,29,89,129,221,67,10,150,111,105,242,128,79,69,159,97,106,175,132,190,138,132,129,166,98,193,153,61,62,129,33,20,140,253,168,238,151,173,111,66,29,159,47,57,177,10,164,112,158,217,213,98,1,91,56,189,4,21,
252,102,221,168,152,43,40,132,93,120,237,6,10,195,126,60,247,30,188,78,130,234,102,114,183,228,201,222,61,152,243,103,183,236,203,96,195,158,22,244,97,164,50,185,75,64,23,222,91,111,89,3,90,45,20,134,28,246,14,181,96,168,158,194,97,120,226,198,144,128,
125,253,120,190,189,25,220,189,108,53,124,244,157,87,254,141,76,202,181,79,221,240,153,143,89,33,32,113,105,141,5,33,72,71,216,4,88,108,0,204,234,39,167,13,87,243,27,23,85,225,228,217,101,152,128,81,64,29,87,240,239,157,36,96,23,14,252,238,65,141,147,
163,241,181,4,154,221,9,156,115,76,5,246,13,107,56,128,194,67,171,253,154,115,122,1,23,35,12,83,48,142,171,113,0,159,55,236,201,97,2,78,92,95,93,194,115,61,25,100,53,1,151,44,238,194,243,41,104,225,235,107,118,100,112,24,10,205,235,78,168,65,25,87,255,
96,131,62,75,43,154,110,208,204,197,0,10,205,174,129,156,5,96,209,204,18,236,27,34,161,73,225,151,155,48,66,217,147,193,101,75,186,96,206,196,20,14,52,114,212,94,10,26,168,5,178,172,4,131,40,0,27,118,40,216,39,74,208,151,148,97,155,170,64,179,113,0,110,
185,235,41,248,219,63,253,147,15,182,114,149,220,120,227,141,31,137,198,117,212,71,8,105,7,206,41,36,174,188,4,7,95,227,26,160,9,255,248,93,7,80,189,107,184,238,194,62,246,206,175,251,94,63,212,40,164,35,207,27,255,166,21,88,66,245,253,253,39,135,57,
50,72,173,255,240,224,186,221,208,200,204,228,187,233,107,41,243,25,58,162,140,199,165,168,101,46,189,121,23,191,78,90,163,132,175,221,246,240,32,220,254,232,144,1,122,200,20,144,199,79,102,70,27,225,36,179,148,185,176,209,134,126,244,76,2,67,170,255,
253,223,106,224,121,208,36,177,41,81,108,94,240,83,24,138,182,160,213,172,227,247,26,130,188,62,4,186,94,199,231,28,106,155,214,195,229,231,31,7,215,190,255,154,15,160,195,88,254,204,103,62,243,215,209,120,140,106,33,232,132,0,160,4,72,10,230,2,232,131,
191,80,20,48,136,43,120,106,143,228,137,171,165,1,53,118,185,164,164,34,204,43,218,56,99,45,114,210,200,78,59,80,71,144,135,41,89,0,180,67,17,53,77,58,62,232,66,82,179,243,86,43,73,27,18,226,23,100,128,199,92,139,222,211,40,5,252,54,159,71,217,89,113,
192,16,254,141,231,84,12,8,1,251,19,244,5,248,122,116,12,122,140,101,124,104,124,100,58,197,195,49,162,81,41,236,219,219,15,63,125,98,19,204,63,98,50,188,247,189,239,125,95,146,36,229,79,127,250,211,127,17,37,201,70,173,16,200,142,156,84,154,137,55,33,
93,8,201,232,98,93,37,225,61,111,243,160,137,48,19,156,112,144,111,67,60,156,40,154,120,233,163,6,105,206,35,76,228,96,162,1,19,243,179,183,46,69,120,22,102,85,155,247,180,189,142,245,230,181,240,231,113,152,1,223,166,138,66,195,22,73,172,118,32,165,
193,38,116,136,44,232,11,72,84,113,66,38,44,72,101,148,192,7,158,220,8,185,29,206,171,174,186,234,61,31,249,200,71,110,68,65,168,218,240,184,4,16,157,238,55,93,0,220,162,226,129,183,3,156,224,138,175,183,180,1,250,32,196,224,57,58,119,82,154,129,165,
215,100,73,50,210,199,147,110,198,58,76,36,189,230,66,70,25,194,61,23,198,105,59,233,86,36,204,231,100,152,112,97,143,101,161,2,135,23,132,123,117,211,99,238,193,197,163,225,59,240,195,134,158,60,116,36,148,146,4,160,4,207,110,220,13,219,247,14,67,95,
79,55,223,199,219,223,254,246,43,62,113,253,245,55,85,170,213,190,56,58,26,109,66,208,57,1,208,17,200,99,7,153,6,158,212,63,105,6,103,199,19,210,8,41,240,67,59,100,61,23,222,44,104,103,166,85,49,173,72,43,207,96,1,225,98,82,138,98,60,15,17,24,100,79,
196,202,200,2,65,124,188,50,90,193,92,91,91,173,96,145,71,165,15,194,247,24,176,114,90,137,84,17,10,0,174,116,216,55,88,135,231,182,238,133,222,158,46,168,86,171,124,141,55,93,124,241,37,159,250,135,79,221,220,221,221,61,201,222,86,217,106,4,249,27,45,
0,20,6,170,40,18,230,201,195,175,223,196,1,37,8,88,186,85,232,84,60,136,34,170,7,118,114,242,224,163,9,7,27,90,161,242,19,66,194,148,25,211,161,99,191,206,171,116,17,180,130,23,144,88,67,132,172,38,88,129,97,252,192,33,153,78,104,148,176,96,149,53,107,
214,36,57,80,43,71,103,241,153,77,187,161,90,171,177,0,212,240,153,190,203,107,95,251,154,215,222,112,195,13,183,77,156,56,113,150,253,138,177,38,16,191,145,2,64,171,154,84,62,163,129,90,216,149,6,176,31,67,188,82,74,94,191,8,240,189,140,16,58,105,162,
7,143,204,225,115,138,234,56,113,171,78,10,171,154,193,128,65,214,31,208,46,37,234,132,37,137,84,186,93,237,14,58,166,137,230,76,161,140,51,145,96,205,79,36,144,140,254,133,41,162,251,242,215,182,121,9,35,25,210,251,57,27,183,237,101,85,86,169,84,10,
66,112,238,185,231,158,249,185,207,125,238,246,89,179,102,29,29,65,199,201,11,166,201,199,186,0,176,157,77,35,184,151,38,46,145,60,94,45,155,6,22,22,127,167,9,202,155,22,182,149,198,94,120,219,158,24,83,160,242,0,23,187,8,65,68,99,151,148,141,176,145,
105,73,24,234,179,16,112,148,106,54,230,65,24,21,175,193,107,29,17,57,122,44,36,137,17,50,201,247,129,247,172,132,123,43,228,31,200,100,128,100,186,131,72,140,38,144,120,236,238,254,33,179,196,209,39,136,133,128,126,78,62,249,228,197,40,4,223,152,59,
119,238,43,192,232,182,81,145,63,232,144,45,50,203,68,72,109,87,179,89,154,4,235,166,9,248,201,96,79,221,66,188,2,66,214,207,227,250,110,177,43,237,39,83,145,121,105,1,68,110,156,11,38,253,106,118,154,135,237,59,59,31,70,99,168,150,246,194,197,54,95,
219,227,164,21,42,167,121,68,208,30,108,14,84,236,112,26,1,73,203,238,59,24,213,149,224,23,219,143,126,0,135,165,105,10,105,90,212,4,116,254,121,243,230,29,251,217,207,126,246,235,11,22,44,56,211,134,134,37,171,9,126,109,66,32,59,53,253,70,37,139,16,
62,57,205,32,130,87,110,38,192,56,128,206,4,152,84,189,153,56,74,227,210,235,73,85,178,208,144,130,72,43,194,58,114,16,209,49,204,241,76,236,17,177,7,106,87,107,102,38,92,70,166,196,152,15,235,229,107,27,126,170,144,112,242,201,36,252,12,105,24,103,239,
157,95,225,179,213,50,246,96,204,135,73,219,165,135,16,2,114,22,143,60,242,200,89,40,4,95,59,243,204,51,223,96,133,160,252,235,20,2,217,65,5,96,86,153,85,215,110,210,8,214,85,22,181,115,206,28,171,208,68,68,118,220,120,227,206,145,11,156,27,97,7,216,
126,46,129,16,222,217,228,147,191,174,117,36,201,142,179,121,160,140,96,4,40,113,120,105,49,139,24,39,144,78,112,193,250,18,113,88,235,4,219,29,99,36,58,8,135,59,189,48,41,228,67,9,1,253,61,121,242,228,9,215,95,127,253,45,23,92,112,193,229,191,110,33,
72,59,229,3,184,137,160,65,215,81,172,78,88,128,178,118,157,157,42,165,189,93,215,214,227,118,206,92,98,209,66,86,195,222,150,147,0,9,99,94,132,9,203,98,143,63,208,81,3,198,199,166,35,58,7,56,191,66,182,221,51,203,152,13,9,117,49,138,177,20,166,144,250,
68,129,210,210,106,56,113,40,48,204,156,144,38,252,80,63,125,125,125,93,31,251,216,199,110,234,233,233,25,127,199,29,119,124,193,10,65,235,165,206,36,118,68,0,28,153,131,198,128,236,58,79,180,214,252,119,201,250,0,137,5,120,84,136,255,12,46,128,2,67,
137,27,86,231,188,50,77,206,222,76,74,64,235,116,108,171,61,75,72,251,149,74,51,170,61,243,68,120,68,208,133,154,4,246,232,76,27,152,56,49,172,33,127,31,210,42,199,220,248,14,116,125,130,135,181,53,23,218,57,183,209,13,196,56,133,11,79,255,127,66,128,
247,145,94,119,221,117,159,158,48,97,66,239,151,191,252,229,79,253,58,210,201,29,17,0,237,66,174,136,152,201,104,43,133,130,13,109,57,127,80,96,245,120,175,63,55,147,78,78,150,178,92,192,164,44,89,149,43,55,161,58,120,240,42,15,240,179,75,60,120,202,
159,136,236,145,163,17,57,201,241,142,165,243,242,181,191,7,86,237,148,23,72,77,30,129,35,7,27,234,177,117,81,218,71,43,206,159,96,33,111,215,132,255,61,33,128,171,175,190,250,35,104,30,122,63,255,249,207,127,52,50,162,47,73,254,160,35,2,96,66,48,233,
109,177,155,236,225,150,182,57,2,167,138,181,119,14,188,214,22,80,72,8,5,187,26,224,99,17,59,140,96,133,73,26,184,88,171,96,78,252,103,109,92,143,226,100,222,199,255,92,50,80,88,19,37,19,105,9,165,138,63,171,33,124,214,11,172,104,203,75,196,78,143,14,
106,191,93,8,200,249,163,7,133,135,36,225,213,114,9,127,199,215,236,241,67,67,67,112,229,149,87,126,0,133,164,251,198,27,111,252,240,75,153,73,236,140,9,208,6,122,13,115,108,84,254,129,186,102,82,168,201,198,133,213,168,163,133,42,180,243,17,76,136,72,
160,146,182,7,72,179,48,205,10,148,129,24,42,173,48,40,239,237,59,239,60,100,26,249,82,76,239,2,131,28,234,72,64,68,48,21,38,198,119,78,157,100,98,40,77,131,19,74,103,119,140,70,8,9,42,18,160,193,225,6,60,180,252,57,24,170,55,96,184,222,140,3,3,78,51,
87,112,226,107,149,18,244,118,149,161,187,90,134,158,90,10,19,198,247,225,223,21,232,63,48,0,239,124,231,31,92,85,198,159,27,110,184,225,131,121,158,215,33,16,75,58,38,4,29,115,2,121,242,210,128,233,211,133,118,13,105,142,2,82,74,177,58,6,176,100,198,
149,135,105,153,244,105,85,56,209,191,148,229,3,58,56,87,74,75,37,19,33,215,224,22,97,146,186,72,129,249,253,62,2,241,181,5,78,179,224,57,77,32,97,77,134,137,5,3,243,152,57,129,206,124,73,227,159,147,32,228,224,145,77,37,2,200,68,255,104,85,175,222,184,
3,46,186,230,179,144,181,154,40,192,89,160,188,187,36,19,227,5,18,74,105,2,189,181,42,76,28,215,13,179,167,77,132,227,143,156,10,139,143,153,14,115,167,245,193,187,175,124,231,31,214,106,213,210,71,63,250,177,107,236,228,235,78,106,130,206,8,128,69,220,
148,178,20,111,101,84,60,153,112,28,39,118,244,50,75,236,96,158,30,81,196,173,157,181,35,101,1,23,163,138,181,93,241,78,83,8,135,247,131,161,143,83,173,0,199,234,150,204,233,76,131,211,64,230,115,22,208,177,102,2,172,41,210,110,38,249,56,229,209,73,54,
13,153,209,66,102,182,37,115,9,2,124,104,76,129,179,77,198,44,152,201,77,165,225,135,122,1,240,154,78,187,122,20,216,55,48,12,187,246,15,194,211,235,183,195,157,15,62,133,154,161,2,71,207,154,12,23,158,122,20,188,254,245,111,188,162,89,31,26,188,254,
147,159,254,112,20,99,116,132,98,150,118,202,182,240,148,43,90,189,54,140,183,254,64,133,86,116,100,223,217,223,178,30,191,81,251,1,136,225,115,228,218,39,147,124,136,103,39,128,163,77,10,9,203,118,112,85,28,171,107,143,116,176,231,238,136,235,20,97,
228,202,99,8,32,130,31,97,160,93,171,17,172,32,230,13,197,14,168,118,60,2,86,65,210,57,50,172,41,248,115,84,9,135,15,137,246,199,132,146,138,77,139,142,194,68,9,81,72,169,205,224,235,8,35,89,243,252,46,88,177,126,23,252,112,217,90,248,163,55,93,120,213,
213,127,52,176,251,115,95,184,233,122,27,34,58,77,48,162,2,208,177,180,164,180,194,111,194,51,163,186,137,218,69,92,139,180,28,212,191,180,158,52,13,100,214,52,112,173,3,2,248,119,95,54,230,146,75,14,1,212,81,133,143,246,116,241,66,173,97,244,167,67,
23,217,52,85,101,1,130,134,216,79,16,96,75,26,237,177,101,233,51,138,44,108,96,4,128,159,21,9,12,78,60,218,30,122,200,180,4,162,84,65,161,47,227,163,130,130,83,197,123,198,191,211,10,63,11,122,46,133,223,5,14,132,76,202,120,157,18,62,151,0,205,63,140,
235,169,194,17,189,13,248,248,45,247,67,101,238,217,127,241,250,223,190,240,109,81,238,96,196,249,4,157,9,3,89,253,83,38,79,4,162,134,197,255,93,228,37,236,202,116,121,124,118,148,50,59,201,202,132,101,236,224,165,194,66,197,214,148,200,112,78,114,250,
164,203,210,197,252,1,79,244,208,62,18,241,224,148,22,62,139,72,166,195,107,29,105,28,68,83,10,170,141,214,128,128,248,105,79,74,49,56,6,213,190,74,156,244,180,82,70,115,86,9,230,58,179,160,133,214,150,136,10,177,20,70,101,106,246,57,55,78,143,171,114,
34,19,121,194,172,110,120,118,239,16,124,233,71,171,229,91,78,123,243,223,31,179,102,205,202,103,214,174,95,97,113,2,61,146,254,64,199,194,64,118,0,51,231,157,227,202,199,241,73,251,12,75,151,126,231,201,119,3,97,49,252,82,69,120,147,73,33,149,78,149,
79,192,4,228,71,251,9,54,170,217,240,0,77,49,169,17,38,158,84,31,93,152,236,159,182,254,136,51,13,236,108,166,210,168,105,17,21,152,70,174,187,240,180,38,11,70,89,170,153,214,134,14,38,75,41,126,55,92,193,9,174,116,190,241,132,87,56,115,13,149,85,175,
113,249,155,136,56,15,174,86,81,217,115,162,170,164,115,180,136,193,140,231,155,58,65,192,134,254,58,220,245,244,192,164,211,126,235,178,255,179,105,227,245,151,213,91,28,25,228,48,130,37,104,29,17,0,162,131,115,17,135,173,207,163,80,78,224,164,84,108,
193,71,174,131,142,38,225,112,28,63,151,227,19,62,89,40,162,156,66,228,0,138,224,13,50,25,163,165,125,138,89,75,221,22,142,26,164,79,104,233,41,99,206,46,27,184,217,56,101,30,150,148,161,154,41,111,69,0,79,102,52,129,210,218,230,29,72,250,80,133,167,
25,11,145,194,149,79,106,221,73,189,71,18,116,49,81,228,82,145,70,83,41,35,44,172,49,204,239,173,44,135,180,218,5,125,189,37,188,70,2,141,186,130,181,106,206,249,11,151,156,249,198,135,127,254,211,175,90,83,48,98,229,233,157,203,5,56,96,206,229,236,75,
70,32,168,58,184,148,82,20,96,110,63,177,89,57,199,194,17,73,80,231,62,190,207,139,231,245,56,191,93,98,169,211,28,150,228,9,73,96,4,185,101,232,4,140,94,34,138,122,119,89,242,204,19,83,185,108,193,32,123,195,94,221,27,36,51,32,132,84,17,100,32,229,132,
225,34,137,54,60,173,145,25,193,201,111,161,29,231,50,166,188,128,44,130,141,84,156,22,51,69,168,246,187,74,83,64,67,192,148,44,147,38,203,81,232,20,84,123,106,232,45,227,185,187,75,80,193,24,121,251,1,128,163,231,93,112,85,247,163,75,127,52,88,111,237,
143,176,129,23,29,21,116,68,0,18,107,111,149,50,54,218,140,173,128,129,97,227,21,151,241,170,92,60,172,3,247,206,171,121,187,218,157,58,246,4,78,25,105,13,207,221,11,30,180,112,145,133,108,163,144,5,228,151,39,165,11,39,126,225,180,18,60,185,181,9,215,
156,211,7,55,255,252,0,108,223,159,65,149,132,40,10,89,153,235,39,9,49,52,30,189,49,109,36,28,146,233,224,124,189,212,114,19,209,73,145,232,200,249,149,236,253,216,8,218,116,17,129,142,240,114,202,115,148,140,138,99,243,130,146,222,76,114,56,160,107,
48,125,114,13,146,205,3,24,82,162,128,100,25,236,42,31,181,184,103,250,188,211,6,215,173,184,219,206,91,238,60,150,209,23,6,58,86,79,20,3,209,252,214,81,59,82,229,15,199,222,100,87,69,88,29,206,44,178,182,109,153,76,160,112,1,191,5,116,140,41,176,159,
139,112,1,7,8,185,44,158,165,163,152,56,221,106,74,119,59,131,24,214,253,206,162,30,174,83,188,224,184,42,44,153,93,134,247,124,117,55,236,29,206,185,134,128,24,62,60,145,22,239,7,198,2,164,41,34,212,46,213,107,169,12,54,221,45,73,50,80,27,144,29,119,
61,6,60,6,16,117,159,209,81,166,210,84,79,211,12,68,13,48,72,19,224,156,14,0,250,0,147,169,119,1,1,71,248,26,122,187,77,140,28,242,169,167,156,15,235,86,252,196,46,147,17,65,8,59,19,6,218,213,152,150,101,112,128,169,226,7,205,192,222,97,109,28,95,17,
216,181,224,18,44,118,197,167,190,1,132,240,126,1,43,7,21,160,93,38,136,68,36,13,173,130,106,45,228,14,92,166,206,46,68,170,20,186,117,217,32,124,224,188,94,120,110,87,198,16,245,71,47,26,207,43,61,48,153,165,103,255,250,254,87,202,214,190,178,208,10,
227,240,73,19,2,82,52,128,241,31,30,95,1,42,5,224,208,143,194,187,196,134,125,20,22,82,8,72,97,95,90,14,97,97,153,222,171,154,71,153,30,93,144,84,186,249,185,171,187,23,195,85,243,72,171,125,104,230,186,161,107,246,226,147,40,147,220,198,36,26,133,140,
32,155,46,213,49,247,14,255,174,161,51,72,5,155,74,7,245,77,142,86,214,176,37,224,73,196,195,119,144,178,19,20,176,252,0,101,5,33,10,10,92,242,201,225,0,133,60,13,211,186,66,116,66,142,232,234,29,25,44,219,208,52,101,107,247,236,135,211,142,168,192,69,
11,106,140,81,184,18,115,71,23,43,85,13,145,68,250,123,177,92,64,233,84,63,97,0,104,255,211,148,31,70,32,74,102,162,147,146,41,30,73,12,239,221,189,103,142,71,39,47,49,241,63,189,71,127,211,103,74,149,10,236,207,203,176,109,16,223,47,163,25,40,147,64,
116,163,96,212,160,122,216,220,89,73,165,247,48,8,212,242,23,77,32,145,29,178,0,161,26,40,21,30,3,160,151,40,23,192,245,122,84,243,199,169,94,91,27,32,69,129,66,158,164,161,126,192,211,187,173,61,77,164,40,104,22,136,210,207,158,20,2,129,191,87,170,24,
225,32,220,128,34,20,250,252,237,143,13,195,171,142,172,48,73,245,150,135,73,35,140,131,233,227,82,31,254,25,219,30,120,127,34,9,204,31,10,31,77,101,144,52,19,44,13,32,100,4,32,225,94,2,36,73,50,53,88,1,135,140,73,252,25,247,57,35,20,137,21,28,122,173,
140,207,143,108,86,240,147,231,114,232,170,149,13,88,132,26,130,180,75,218,59,181,47,233,26,127,24,248,126,76,163,85,3,88,155,159,166,194,23,96,176,70,192,119,182,13,24,168,149,38,152,169,214,156,41,20,30,144,145,182,110,207,165,125,35,184,61,112,240,
146,182,112,209,230,29,152,0,162,116,84,121,36,124,24,22,4,5,189,126,60,199,210,231,154,240,196,230,22,92,177,164,27,254,253,145,33,232,71,111,252,207,207,31,199,247,108,194,82,237,23,153,65,32,101,160,161,218,92,133,249,82,38,79,32,64,134,116,49,72,
203,130,14,140,97,35,80,50,112,18,45,61,221,31,207,69,38,166,238,240,189,103,141,131,5,51,171,208,66,103,51,73,131,118,72,43,93,53,252,92,79,155,6,24,157,38,32,20,237,216,170,29,252,127,63,14,242,79,214,182,160,98,87,188,214,34,100,4,163,246,61,206,214,
7,226,101,168,5,76,236,228,211,192,103,22,124,35,97,74,219,8,25,210,179,124,3,107,135,72,38,105,197,56,113,84,120,250,181,199,135,225,156,163,42,48,185,55,129,143,222,181,31,78,71,141,240,219,243,107,172,21,204,106,215,158,32,226,39,77,68,154,64,90,109,
32,12,44,108,114,4,137,77,102,201,144,36,114,43,158,171,153,18,198,36,226,46,39,78,202,25,196,194,145,58,97,122,5,142,159,130,78,31,24,161,48,166,35,97,51,144,118,79,234,30,245,38,192,1,32,190,170,214,170,104,10,147,183,247,135,36,137,147,126,1,224,203,
186,60,93,76,134,194,146,144,255,21,33,214,135,160,29,192,37,115,36,4,150,15,4,190,191,141,206,88,224,200,215,164,82,244,46,20,134,7,214,54,96,115,127,14,151,157,212,5,43,183,103,240,221,149,195,240,167,103,247,193,172,9,41,155,41,176,89,96,111,22,164,
97,248,248,186,4,75,82,229,22,52,37,179,234,205,106,79,252,138,15,170,43,82,103,96,11,75,165,49,23,194,214,32,208,50,33,186,217,126,12,145,135,115,147,62,102,205,99,147,38,236,60,86,122,202,5,169,30,141,26,192,169,100,233,104,224,182,92,156,50,129,147,
123,165,143,176,56,127,239,226,99,136,34,2,16,62,42,48,38,93,251,122,62,23,50,154,170,161,192,242,117,147,99,30,142,26,166,11,62,4,245,26,184,120,81,13,142,152,148,194,129,33,205,149,74,183,62,60,4,191,125,124,21,142,198,215,110,198,232,96,235,254,28,
254,4,133,128,24,198,108,158,210,152,123,232,106,8,164,215,6,50,18,14,183,218,157,250,151,110,146,165,51,7,214,87,40,5,19,226,207,149,152,123,87,185,128,29,3,10,170,196,89,20,238,92,194,155,152,194,106,24,173,217,64,199,232,245,166,128,178,128,40,209,
199,79,79,225,164,89,41,78,132,48,89,64,221,230,241,203,80,78,110,86,191,246,116,49,55,233,177,179,232,35,134,168,28,32,20,112,72,203,68,50,172,34,210,208,51,199,167,112,218,156,18,76,236,146,240,230,83,186,224,168,41,41,124,251,201,6,108,220,155,195,
57,71,87,96,31,10,197,77,15,14,194,217,104,22,206,61,182,10,25,184,137,179,14,166,211,66,46,169,228,38,86,10,175,193,184,55,145,178,20,51,136,83,197,210,23,180,10,123,94,233,254,22,214,44,48,149,92,194,166,125,26,54,236,83,76,160,141,5,43,114,112,71,
44,35,216,49,62,64,112,188,140,141,167,129,57,237,240,20,118,13,90,251,110,33,82,231,164,185,100,13,153,80,10,13,181,11,239,82,59,249,153,9,231,210,82,244,245,85,72,16,8,151,109,115,180,18,247,159,205,75,15,214,53,188,239,213,85,152,135,147,190,104,122,
137,19,47,151,44,236,131,135,214,55,225,123,43,234,112,249,146,26,220,243,76,3,30,69,199,240,174,213,117,184,246,236,94,88,142,191,239,29,204,3,77,92,152,130,48,37,131,243,41,124,255,3,83,85,44,101,59,84,109,89,200,82,114,212,195,29,81,108,78,79,115,
158,201,28,164,108,249,59,117,78,249,206,202,6,167,206,107,101,105,252,36,233,213,28,20,147,221,163,84,3,184,16,206,241,239,93,28,62,185,71,194,164,158,80,171,47,69,168,208,117,199,123,154,151,8,182,215,81,182,211,52,208,175,165,101,249,10,219,136,194,
225,125,113,235,122,167,13,90,40,40,103,205,69,199,106,106,10,159,248,241,32,220,242,200,48,220,246,68,29,174,191,247,0,44,156,81,226,65,167,14,35,23,47,168,242,133,191,188,108,152,63,247,251,75,186,124,93,99,12,60,9,31,42,10,240,44,87,187,82,57,125,
45,165,175,64,230,242,52,171,37,18,219,115,16,108,89,185,193,19,140,26,75,236,10,111,161,9,88,56,189,140,90,42,69,43,104,35,133,168,143,65,161,40,114,180,10,128,47,202,53,237,121,109,129,7,192,129,134,134,241,85,81,76,236,136,98,223,64,22,160,196,76,
182,11,195,185,60,43,17,133,116,179,20,49,115,72,251,9,50,78,90,48,45,84,136,114,206,220,18,156,130,218,231,225,77,25,28,104,82,46,66,66,21,87,215,62,212,10,255,241,216,16,92,116,66,5,6,240,222,206,71,51,48,169,59,129,126,252,253,147,63,25,128,55,47,
234,98,1,105,217,116,51,200,56,212,179,68,151,184,17,133,141,108,28,12,45,18,7,34,5,129,33,130,137,140,177,6,215,231,192,254,77,207,39,160,134,234,174,26,191,73,104,17,133,147,133,46,136,163,156,17,36,163,178,43,27,226,80,77,64,119,89,88,46,158,246,37,
90,206,111,112,72,94,30,165,56,60,13,79,182,215,231,133,234,98,239,60,70,77,38,233,15,162,25,28,214,45,225,29,75,170,176,118,87,14,63,88,213,128,11,143,45,195,165,39,85,185,77,28,213,114,78,193,9,39,166,242,63,255,124,144,147,88,167,163,143,64,131,242,
228,214,12,190,247,84,29,174,126,117,47,116,87,100,212,156,34,80,195,101,91,191,1,25,119,63,113,213,197,73,226,7,194,125,7,173,33,162,150,7,27,79,218,190,138,227,115,204,228,212,39,145,124,118,84,201,23,213,21,252,37,22,128,8,168,137,90,188,82,207,69,
234,17,68,72,156,182,12,95,136,18,56,238,33,165,8,43,45,194,7,92,134,143,208,60,149,135,46,161,238,61,25,5,17,116,189,22,30,119,197,41,85,78,67,47,70,231,243,98,92,233,119,63,219,132,181,187,115,120,219,43,106,112,241,194,42,171,253,199,209,214,255,215,
147,117,248,241,51,77,35,0,212,32,26,231,237,63,31,31,134,217,19,18,120,195,130,46,162,139,66,34,99,92,66,248,22,49,161,54,80,88,141,39,11,42,91,68,52,115,136,194,195,80,168,106,5,3,167,163,134,23,158,53,161,100,250,37,137,8,133,148,35,105,249,95,130,
40,64,123,111,87,120,231,136,234,2,74,9,132,149,162,163,50,112,25,72,63,254,125,159,213,11,221,195,56,89,68,97,84,98,66,205,60,202,137,233,72,144,40,231,63,111,106,2,23,206,43,195,99,207,103,208,143,33,223,9,211,82,120,255,171,107,176,16,159,191,177,
188,14,75,14,47,193,124,252,253,17,124,159,60,242,175,61,58,12,139,103,150,225,60,52,5,196,190,236,199,251,189,121,217,16,188,251,244,46,152,57,33,69,193,139,48,9,17,248,140,126,66,45,54,19,23,149,136,184,51,137,51,29,50,216,254,184,67,133,19,4,106,140,
233,28,64,111,206,100,220,23,97,148,11,128,171,186,117,158,184,202,139,233,94,167,194,149,141,2,124,120,19,37,110,60,53,76,68,108,205,66,56,46,108,134,81,248,34,82,207,72,178,231,184,242,149,53,158,120,226,31,80,87,210,13,24,238,237,67,181,127,238,81,
101,248,223,191,213,13,199,162,170,253,171,239,13,192,211,168,238,39,161,73,88,191,87,193,143,86,55,224,210,197,53,198,44,168,157,205,61,207,54,88,99,92,253,234,110,40,227,164,84,202,214,121,107,155,10,33,226,213,236,112,127,17,18,86,82,20,89,79,174,
207,177,12,62,133,180,108,169,28,12,80,229,206,69,99,148,183,160,35,165,33,157,137,2,146,160,178,181,229,231,57,178,70,221,50,125,13,226,23,76,133,249,59,56,112,206,116,186,202,98,159,24,142,28,70,55,168,142,0,226,146,65,148,229,187,100,81,5,213,121,
10,159,91,58,12,219,7,20,28,49,49,225,9,39,129,122,22,39,116,74,175,132,109,7,20,172,217,147,195,210,141,166,136,131,162,129,127,195,8,160,15,29,176,197,232,252,53,149,17,180,207,254,108,0,78,157,93,130,11,230,85,216,140,37,73,148,21,108,23,133,168,27,
137,176,42,223,99,2,182,168,36,86,251,206,63,114,62,69,102,169,8,189,21,233,122,79,216,197,97,16,199,49,33,0,42,98,190,152,126,65,134,250,197,245,129,100,191,195,14,13,197,86,111,186,45,243,87,172,195,224,210,240,16,101,20,125,4,247,76,43,104,230,248,
4,222,115,122,13,190,255,116,19,142,64,27,62,136,66,119,235,163,13,216,188,95,193,113,83,18,56,122,146,100,98,202,223,221,51,8,91,251,53,244,148,141,62,167,46,163,207,239,83,240,99,212,2,255,235,180,46,126,157,108,241,38,124,237,43,24,58,94,117,70,23,
140,175,25,56,217,11,36,155,177,196,132,119,58,152,42,23,162,122,99,16,23,145,68,53,17,197,160,222,56,192,36,4,105,18,204,13,113,3,203,93,73,212,212,102,148,11,128,35,123,184,200,73,218,28,201,158,97,195,191,43,69,33,96,92,235,1,194,49,129,156,32,25,
79,190,48,144,142,78,173,124,201,79,168,66,182,97,223,59,78,169,112,191,225,79,220,51,204,190,192,226,153,37,56,227,136,20,126,182,174,5,255,250,112,157,87,248,173,191,172,195,186,221,138,87,125,96,36,9,168,225,202,251,183,95,14,179,250,63,227,240,50,
195,199,20,5,220,189,166,9,195,168,134,207,63,182,18,229,28,34,202,154,69,6,25,221,81,1,53,148,145,231,222,82,46,111,33,124,87,50,79,138,113,81,1,62,103,248,126,165,148,120,31,193,85,73,11,57,102,162,0,240,28,124,98,245,208,35,197,47,179,181,95,113,215,
240,90,41,146,228,56,215,11,161,4,156,158,166,244,8,56,231,200,18,80,157,101,158,71,172,112,199,234,245,221,155,140,208,145,202,62,250,48,116,252,142,45,193,39,127,58,12,115,38,74,152,62,78,194,39,239,27,102,40,250,236,185,37,56,27,207,247,20,39,126,
26,208,149,22,123,255,208,4,16,32,244,60,106,133,31,174,106,194,91,78,172,50,137,149,14,161,202,230,155,31,30,98,255,128,178,135,190,253,156,4,219,75,216,230,62,40,51,89,178,121,0,95,87,32,120,69,207,159,94,182,197,173,206,167,9,9,61,215,178,198,129,
142,61,101,23,41,133,246,55,162,160,85,70,179,19,24,131,52,142,162,133,95,132,237,167,0,31,78,57,2,105,92,69,236,104,88,180,199,0,169,235,19,166,75,254,156,230,14,99,130,87,81,211,170,73,202,216,13,103,154,87,57,77,16,201,213,7,207,173,193,79,159,203,
96,217,166,28,174,56,185,2,223,126,178,9,175,193,216,127,251,128,70,53,222,128,249,24,25,220,138,207,67,45,65,253,40,89,99,208,249,72,192,104,181,155,18,118,1,183,63,222,96,199,113,193,52,244,5,240,64,234,63,252,224,250,22,59,132,239,70,243,192,117,90,
74,176,96,145,150,34,225,163,207,183,50,67,120,161,246,244,117,46,44,22,48,128,2,124,220,148,18,10,84,141,209,235,36,21,145,249,51,131,197,125,21,109,93,35,251,73,145,178,103,82,141,160,202,169,145,159,171,180,83,2,32,227,86,174,194,72,189,100,7,205,
214,11,40,219,227,199,190,79,131,70,136,91,217,86,20,147,140,76,239,53,43,114,114,159,128,73,93,18,122,203,130,53,8,13,118,255,160,226,158,254,83,186,205,202,39,36,239,136,9,18,230,162,179,247,225,31,12,193,155,22,148,97,35,218,110,114,216,142,57,76,
194,255,93,214,128,15,157,83,131,95,160,195,183,106,71,14,199,77,53,251,21,84,113,4,168,13,61,237,65,48,13,29,67,250,123,24,7,122,0,255,190,107,117,19,222,188,160,10,203,183,14,178,0,150,48,190,191,13,67,197,27,223,216,135,161,101,5,238,127,174,9,227,
170,9,127,134,0,46,194,239,41,212,37,129,156,142,90,130,204,203,222,97,197,66,242,154,227,42,220,130,190,142,161,100,102,199,167,146,250,64,55,128,93,182,60,190,29,85,33,86,147,246,21,178,163,92,0,60,19,80,135,60,63,105,2,34,130,164,142,167,31,37,77,
104,226,79,158,149,192,5,199,148,216,71,32,167,141,42,136,142,64,21,62,174,42,224,134,139,186,112,64,165,47,54,161,202,226,6,10,18,77,26,197,234,100,46,104,18,142,69,245,255,197,95,212,25,196,89,60,35,129,207,254,188,1,87,157,94,129,187,158,105,113,236,
63,163,79,194,51,59,51,184,245,237,189,208,93,50,142,24,77,26,157,147,234,3,8,170,38,141,66,239,81,210,138,206,127,60,10,202,235,142,47,195,143,214,180,120,215,179,221,120,12,129,73,239,63,187,27,46,59,169,6,19,186,4,151,187,243,121,172,107,66,247,78,
27,92,144,102,161,239,187,3,163,141,105,120,237,241,232,123,92,119,65,47,244,160,224,145,134,163,161,33,146,204,55,158,168,195,206,1,179,43,9,119,71,151,194,251,22,2,2,40,146,36,35,238,3,118,70,0,50,234,199,111,54,227,66,181,108,28,189,52,209,76,116,
32,13,64,254,77,110,203,184,92,247,150,205,251,113,96,159,201,96,207,144,230,150,178,207,161,131,246,103,103,85,97,229,246,28,238,125,54,131,113,232,125,247,211,106,178,97,18,153,18,154,176,221,67,38,239,127,213,105,85,118,210,254,227,209,38,252,229,
249,53,248,33,78,250,34,20,2,154,132,173,120,238,79,189,190,6,255,244,179,97,206,7,156,56,61,195,207,42,158,224,76,25,210,7,97,18,253,77,211,199,144,118,13,33,83,64,231,163,213,252,246,87,84,97,35,250,5,235,247,229,92,87,112,231,234,22,99,9,59,135,20,
250,26,117,222,128,98,160,110,247,28,192,85,205,27,82,12,26,7,147,38,122,199,126,52,71,167,118,193,7,207,233,129,245,187,51,212,76,57,107,133,157,120,76,131,132,29,191,148,51,139,84,88,196,194,77,37,239,137,140,28,63,209,145,40,160,99,77,162,184,237,
171,20,33,36,180,68,254,3,77,179,82,28,248,145,101,198,177,217,57,8,176,165,223,32,56,36,32,175,152,158,192,172,241,168,254,51,9,27,246,74,94,165,147,186,18,219,47,0,24,212,153,130,171,239,149,179,83,152,136,207,111,59,177,12,31,248,222,16,156,127,76,
153,5,131,108,245,159,157,85,131,127,126,168,1,239,90,66,59,145,224,138,196,129,125,245,156,18,219,151,113,181,196,236,11,160,92,91,96,205,19,70,197,33,91,48,92,148,120,252,89,71,166,40,60,138,53,202,162,105,9,172,219,147,115,42,151,38,254,54,244,17,
46,123,69,133,253,148,181,123,0,182,13,100,144,104,194,241,19,60,119,201,132,164,182,15,209,94,156,232,25,118,183,52,34,123,44,223,146,177,0,36,73,200,138,66,212,254,70,138,208,76,211,107,0,29,154,109,140,122,1,224,184,63,238,137,45,108,107,23,124,94,
191,199,172,58,25,181,145,167,193,103,18,105,57,236,19,72,153,186,141,123,53,204,161,34,73,180,229,100,46,217,89,211,198,1,108,224,239,53,50,23,40,80,215,162,166,248,38,58,123,207,237,81,112,237,153,101,248,151,95,224,228,44,174,194,253,235,51,152,51,
94,194,146,217,9,124,232,251,67,140,8,42,138,22,90,134,160,146,89,65,109,178,227,134,39,182,29,76,114,219,222,110,39,254,189,106,167,130,233,125,45,56,239,168,18,44,221,208,226,80,150,60,244,123,215,54,225,204,35,74,240,174,83,107,240,109,140,40,166,
118,75,184,7,67,69,34,189,146,176,209,108,146,182,35,19,64,154,166,156,42,254,158,116,223,137,44,230,56,32,204,63,47,24,250,92,189,21,252,40,199,141,148,82,140,120,20,208,65,31,32,36,101,226,68,13,77,146,82,34,202,24,154,10,25,87,7,232,96,223,231,80,
80,14,176,115,38,224,89,52,7,213,196,36,99,204,150,47,198,127,216,129,239,159,62,167,204,234,242,11,184,210,175,56,169,12,43,208,100,80,248,56,9,181,194,215,150,231,240,15,175,171,193,151,208,1,92,181,51,103,167,139,122,245,20,174,5,33,165,156,171,224,
176,118,85,52,172,217,105,132,245,219,43,155,236,159,156,126,120,10,223,69,245,79,110,27,249,33,79,161,63,113,212,196,50,131,77,95,69,141,64,175,175,65,7,83,233,176,191,1,77,58,201,86,87,197,68,51,20,141,80,84,81,73,53,40,40,134,181,238,135,142,235,173,
70,57,5,89,204,179,140,137,48,208,209,163,220,239,180,167,35,169,212,203,22,151,76,97,72,148,43,144,110,27,152,136,33,76,168,220,44,140,225,155,153,249,189,92,178,217,56,59,94,52,49,39,76,149,240,251,39,151,217,217,163,173,104,200,9,252,57,122,249,111,
58,161,4,119,224,164,93,178,160,132,166,69,195,157,79,183,208,177,147,12,64,85,74,102,85,146,15,65,59,139,133,228,148,193,42,18,219,208,138,236,48,133,116,244,153,77,253,10,110,127,162,1,191,115,124,5,205,142,228,207,254,225,41,85,216,51,64,185,131,38,
28,137,206,234,132,46,201,247,78,247,74,219,217,149,233,122,137,241,244,19,75,74,109,100,218,11,190,182,57,126,33,139,137,32,122,108,71,167,113,114,79,98,57,131,129,38,39,58,160,1,58,135,4,10,139,153,131,1,130,40,175,95,207,73,125,130,165,133,7,226,72,
230,242,5,92,194,109,85,83,2,124,236,80,203,52,115,148,224,132,197,76,30,9,192,187,78,173,192,51,148,231,199,9,126,15,254,254,147,117,25,78,70,194,42,148,38,227,252,163,75,112,19,154,3,154,80,198,27,32,130,87,165,223,18,0,116,155,42,118,169,93,90,137,
20,147,87,240,243,223,121,186,201,78,219,229,139,43,112,229,41,21,120,4,157,201,71,54,231,158,77,228,154,98,248,60,8,132,126,201,244,218,14,252,44,221,215,140,190,196,147,98,157,141,151,82,20,118,45,33,31,166,86,114,168,167,163,207,119,32,23,220,49,36,
80,4,70,47,57,66,110,219,23,42,200,216,51,4,188,170,29,89,196,77,132,3,83,154,45,51,240,228,133,147,74,166,247,9,84,161,240,108,152,67,63,147,166,61,121,86,138,234,55,129,127,92,218,128,55,47,44,115,108,189,98,187,226,213,255,195,53,25,78,82,21,190,179,
162,9,171,182,102,166,47,129,227,85,40,83,29,148,101,130,169,98,116,45,138,44,72,77,83,56,106,0,29,243,160,152,125,0,35,151,93,196,210,197,207,239,69,159,96,238,4,180,245,207,182,56,42,249,203,115,107,76,44,161,72,228,45,39,86,184,194,152,170,206,201,
177,165,115,209,138,39,155,223,210,130,163,27,242,87,8,153,228,132,143,50,33,109,43,119,253,18,66,68,164,116,48,153,110,255,37,1,113,157,195,104,247,1,162,118,174,82,134,213,64,14,84,238,188,94,91,31,72,95,182,175,91,192,52,28,200,241,53,231,44,26,188,
128,18,47,36,0,23,29,151,242,138,160,243,141,199,21,183,27,207,115,233,43,74,240,61,92,249,253,117,128,55,204,47,193,223,223,95,135,183,160,202,95,177,45,71,83,32,249,179,119,60,157,113,10,151,38,185,100,213,77,220,7,128,194,61,98,8,167,86,8,89,179,104,
179,171,9,1,59,21,219,214,246,112,156,180,243,143,78,225,9,60,55,85,16,189,231,149,85,6,145,8,82,38,135,243,68,140,88,174,56,169,2,255,133,2,183,175,65,155,98,104,142,80,232,252,148,248,34,44,99,10,131,76,130,73,42,132,29,80,186,151,128,45,18,206,61,
24,85,144,16,185,144,47,87,209,122,143,247,43,2,49,54,112,0,15,98,216,58,62,55,230,42,218,209,67,89,181,78,216,127,13,39,235,117,199,37,108,63,9,68,217,143,43,124,11,198,238,132,238,209,224,30,137,171,142,86,38,173,34,202,217,95,138,126,196,14,116,166,
110,125,172,9,127,126,118,5,99,254,12,38,214,36,28,55,89,194,77,203,154,240,55,231,85,97,37,58,99,127,124,122,153,7,141,38,115,114,183,137,201,99,53,223,91,49,171,175,219,214,23,144,144,145,198,34,243,99,0,39,243,160,149,187,229,128,169,251,39,128,136,
142,126,108,115,6,55,254,108,24,239,187,12,139,103,166,44,212,127,253,91,93,140,42,146,86,112,63,180,107,169,11,255,40,156,252,93,12,29,47,152,87,102,13,71,223,103,8,191,216,138,109,25,236,220,144,153,98,63,187,111,98,195,22,164,72,81,4,255,70,218,8,
116,172,87,48,136,224,11,184,110,158,228,213,63,176,62,247,181,131,108,22,202,198,44,220,184,180,101,58,132,90,202,23,225,232,199,78,145,24,1,100,240,239,143,183,120,117,18,166,126,244,36,1,23,30,147,194,231,31,106,194,41,104,6,200,219,95,138,131,247,
151,231,84,224,235,43,50,120,203,194,18,135,141,255,180,212,56,134,52,1,180,18,201,236,12,56,219,42,141,223,65,130,70,128,77,217,98,243,21,27,146,130,101,20,145,230,104,102,218,135,159,202,180,8,64,143,191,201,161,231,245,175,235,230,144,240,190,117,
45,142,50,222,56,191,12,15,63,159,193,202,109,45,248,254,170,38,155,1,186,30,33,155,27,119,231,112,205,153,93,40,216,57,124,7,29,212,196,246,75,160,99,72,224,202,73,24,63,210,20,137,12,62,64,204,119,28,51,185,128,152,191,202,197,160,2,120,162,135,45,
234,229,108,191,147,234,212,150,132,49,224,81,66,53,140,131,190,26,99,240,149,219,13,150,222,208,198,107,191,26,87,245,131,27,114,120,20,29,176,79,189,190,10,55,63,218,132,11,81,61,179,10,197,143,207,70,117,253,241,123,27,184,178,4,172,219,163,60,63,
193,161,135,14,72,114,80,52,11,86,67,219,64,80,248,176,53,226,153,122,230,183,243,152,8,40,162,112,150,252,144,175,252,178,1,95,197,8,129,224,98,250,153,136,218,236,111,239,174,251,210,50,101,171,147,232,220,68,84,121,26,195,84,242,13,72,232,168,205,
80,218,182,196,201,208,145,227,59,99,156,233,118,46,74,157,115,0,59,136,3,224,68,55,140,221,141,251,63,144,93,158,84,51,212,172,36,122,207,109,0,17,84,136,225,12,220,191,46,231,137,162,85,62,190,10,112,202,236,132,243,3,127,121,215,48,92,246,138,50,172,
221,77,144,43,192,123,150,160,70,120,176,9,87,189,178,12,223,90,209,130,245,56,57,61,149,216,123,214,133,206,44,96,187,137,184,29,67,18,123,125,87,109,20,135,175,133,14,31,214,191,161,48,239,107,79,52,249,59,205,30,47,225,189,103,212,120,114,111,95,222,
132,183,45,42,195,155,23,148,57,227,88,182,244,117,210,237,21,188,151,255,196,215,8,2,161,207,199,187,161,197,125,164,18,105,28,70,130,147,57,138,138,27,100,142,88,111,176,142,227,0,24,11,87,44,47,48,248,133,92,19,64,206,147,210,80,192,181,11,50,30,119,
20,161,20,50,122,226,39,205,76,224,109,39,150,224,77,243,83,248,215,95,182,216,121,58,101,150,68,149,223,130,75,23,17,66,151,115,154,151,212,49,105,7,195,167,139,57,151,34,20,161,56,138,89,18,202,202,28,248,36,146,98,223,1,33,139,108,21,23,178,209,225,
251,208,25,36,223,224,156,163,74,60,249,55,63,210,68,181,223,130,181,40,124,239,56,185,10,11,166,165,188,210,93,9,28,117,61,105,218,104,199,51,153,68,208,48,177,246,36,144,137,34,157,112,195,225,123,140,29,66,8,132,22,47,142,219,71,94,190,243,184,99,
14,191,20,194,195,158,82,28,156,82,38,59,126,250,225,9,19,53,190,251,84,11,254,224,228,20,238,69,237,64,91,203,83,202,248,209,45,10,237,111,10,95,126,164,133,170,63,236,87,92,88,191,174,0,85,138,130,138,63,168,190,208,246,46,104,167,154,185,19,145,57,
154,137,190,197,7,207,169,241,10,39,8,122,90,175,73,57,19,124,253,45,140,4,136,131,248,254,179,106,48,17,157,193,86,94,228,62,200,136,217,234,38,52,110,133,111,198,41,116,58,113,91,221,134,250,201,49,130,4,166,73,188,237,171,81,121,83,186,165,217,55,
48,45,238,12,170,33,80,168,93,236,235,32,209,211,102,75,56,123,110,194,241,255,167,30,104,193,169,179,83,86,161,247,161,51,249,214,5,41,220,137,17,192,197,56,249,196,219,251,25,190,70,239,201,168,166,0,34,90,53,68,92,190,152,204,25,239,43,33,10,96,182,
240,5,172,126,27,27,43,148,183,163,99,250,47,15,25,158,33,169,236,139,142,43,177,160,62,137,161,226,210,245,25,227,8,215,188,186,102,242,13,162,77,179,104,40,144,97,227,58,10,186,119,114,78,183,246,231,193,239,136,37,112,172,64,193,14,107,247,245,13,
64,248,188,33,78,208,63,183,101,12,20,233,125,126,122,200,227,166,126,66,148,38,62,26,237,62,169,68,202,250,17,252,123,203,227,25,156,54,43,225,213,78,175,191,106,78,2,183,225,107,105,27,9,197,147,75,161,232,148,198,197,29,174,174,208,67,216,177,233,
136,150,167,235,91,64,2,176,5,67,193,205,232,205,83,116,241,228,214,156,191,219,214,3,26,206,58,178,196,128,21,1,69,95,248,69,157,57,14,175,63,222,132,124,113,31,227,120,197,199,85,47,116,12,69,34,132,101,44,223,106,114,23,133,141,52,198,138,9,16,174,
234,199,174,112,101,27,59,81,216,83,41,104,5,17,154,50,89,148,206,103,15,241,63,98,247,144,186,167,216,122,205,46,5,127,255,218,178,1,87,240,156,71,161,80,252,199,242,12,222,120,124,9,238,192,240,239,137,45,57,71,9,46,121,82,44,39,8,166,40,236,22,94,
132,174,61,28,91,164,40,122,96,211,149,123,209,57,19,109,252,19,18,184,141,251,76,244,241,12,134,129,4,21,83,8,71,218,138,34,5,98,33,17,92,125,88,143,240,232,159,239,131,32,14,50,241,222,4,82,253,226,254,58,248,220,71,39,183,18,235,216,182,113,113,173,
159,3,128,250,168,75,104,18,77,180,245,248,147,168,188,203,193,181,244,251,243,253,10,87,124,194,30,253,194,105,84,184,161,217,230,127,228,188,50,39,104,142,154,36,97,92,197,204,229,107,231,149,56,244,138,87,185,108,219,249,59,54,52,34,108,21,28,118,
5,21,109,142,105,155,48,184,29,71,125,193,167,69,23,73,237,159,122,120,10,191,124,222,168,109,210,20,68,38,93,186,49,103,92,224,242,147,42,28,218,77,236,22,108,26,221,247,110,247,1,220,46,246,19,80,200,187,92,20,163,138,161,245,152,40,13,115,3,236,74,
181,74,182,186,119,216,230,199,253,106,143,119,254,0,183,207,176,245,152,113,50,95,127,92,2,221,104,87,127,138,14,223,221,207,230,112,24,14,32,217,89,138,193,127,23,237,255,213,167,149,216,164,204,234,19,156,7,72,108,133,176,235,49,236,119,45,111,203,
189,251,38,20,34,218,75,74,67,180,145,85,216,68,66,20,2,65,104,203,227,11,46,230,36,237,67,248,195,225,19,36,196,95,171,134,223,227,182,199,155,76,71,163,36,210,37,11,202,94,43,6,83,21,101,2,165,137,0,22,78,79,216,17,212,174,56,244,224,242,147,209,45,
0,142,212,25,135,129,4,111,146,106,172,36,34,84,14,67,216,44,218,23,209,82,113,4,70,15,51,112,82,23,224,234,39,72,246,153,221,26,190,177,50,135,127,94,214,98,52,144,212,46,9,83,217,146,45,110,121,44,99,32,40,9,251,60,132,213,172,3,209,66,171,162,143,
160,189,105,16,81,206,34,216,127,87,182,85,104,241,19,103,13,45,127,143,114,28,228,132,158,55,55,101,138,151,115,112,73,133,147,217,35,200,154,76,193,137,211,211,66,116,114,176,73,55,23,39,254,99,201,22,206,198,218,106,12,153,0,17,86,75,228,188,42,11,
183,178,19,104,7,189,176,223,158,8,118,144,6,238,118,180,241,244,188,1,195,63,74,160,60,177,77,195,87,112,178,95,115,108,2,51,208,20,252,24,181,194,223,220,211,226,99,136,237,19,51,144,98,170,185,47,178,148,69,190,130,219,84,68,41,167,146,227,148,112,
196,100,242,234,58,50,15,50,0,74,20,14,254,96,85,6,139,112,130,123,184,174,191,72,222,32,184,153,82,188,228,163,176,153,146,193,5,105,159,87,242,45,136,64,90,16,188,88,32,199,132,19,40,98,96,179,40,12,206,83,215,2,66,46,220,142,132,107,50,173,237,234,
57,101,166,100,6,13,37,134,40,101,123,220,100,1,31,58,179,196,54,242,31,151,182,96,61,106,2,242,3,136,108,154,128,40,182,230,141,66,38,199,177,75,162,36,85,232,76,98,174,197,144,173,46,86,251,21,232,237,58,168,108,191,25,149,125,141,132,143,0,32,226,
15,206,159,156,216,52,179,161,192,159,59,151,72,48,101,120,104,83,198,32,216,164,30,9,245,150,201,66,182,59,158,116,186,46,180,18,68,132,17,237,9,0,209,25,37,208,209,30,65,70,162,129,85,58,49,119,91,121,100,71,117,176,207,42,108,31,234,109,44,77,202,
201,51,77,238,156,114,239,139,209,9,124,235,252,132,195,173,229,219,20,220,183,94,177,154,164,80,145,38,128,87,136,22,190,199,32,97,16,142,156,145,235,104,219,183,176,107,131,95,2,82,135,182,246,241,94,131,174,47,144,142,118,149,243,88,129,115,224,18,
237,203,209,239,93,155,113,65,42,113,17,201,124,189,105,190,9,11,191,248,112,19,38,163,6,59,117,22,250,45,103,84,152,241,188,245,128,66,13,150,153,108,169,243,66,173,111,192,77,52,92,47,2,29,76,21,140,149,100,144,91,85,153,219,17,197,198,220,76,19,79,
138,199,170,104,115,73,7,152,36,130,56,250,38,123,72,42,241,99,231,149,160,7,157,191,235,239,207,96,106,15,177,114,3,98,72,126,69,34,3,7,1,146,98,129,166,167,96,41,240,59,138,24,39,92,248,157,68,180,136,242,17,186,136,77,11,223,21,76,23,177,152,168,43,
57,21,171,92,124,66,137,11,78,169,80,229,10,92,241,196,34,254,225,154,22,252,98,83,206,254,202,73,51,36,223,51,53,200,160,227,127,176,170,101,32,235,130,99,106,190,75,79,197,148,166,115,201,89,42,138,224,148,24,3,132,16,13,80,64,244,160,205,4,164,182,
227,69,110,219,195,130,46,218,109,42,185,154,217,107,136,24,244,243,236,30,141,225,84,14,155,113,213,44,223,102,124,137,106,42,34,80,76,135,208,179,112,46,151,221,211,126,247,176,130,75,15,17,184,18,117,151,240,253,254,35,115,38,163,45,104,252,70,20,
246,56,114,76,215,162,163,122,198,156,132,193,170,183,206,40,193,87,30,109,194,234,93,102,135,210,185,19,4,92,186,168,204,224,206,39,239,171,115,129,72,119,57,160,131,89,22,239,146,98,55,218,140,157,82,29,37,140,198,66,101,144,176,118,95,218,189,242,
180,10,142,93,89,66,148,9,51,161,152,178,91,190,185,137,163,227,78,158,33,56,71,190,116,163,130,47,61,146,51,60,74,33,160,140,237,178,138,125,13,59,58,50,236,88,22,11,2,191,38,116,129,127,175,33,102,217,70,37,235,160,195,169,227,234,100,29,64,98,162,
125,185,38,146,4,12,17,63,144,132,148,38,238,202,37,37,248,61,156,240,183,44,208,108,178,166,246,8,206,92,18,85,141,252,3,106,66,21,171,126,79,255,6,67,139,35,106,58,245,7,112,249,0,223,181,184,3,136,80,199,52,128,86,81,79,67,219,231,55,203,225,160,186,
120,143,28,38,230,147,20,255,159,63,87,192,219,22,37,240,175,56,241,63,88,163,216,123,238,46,57,103,210,109,176,0,197,125,134,35,187,76,19,109,183,228,9,222,188,221,163,64,71,250,91,68,176,117,28,155,107,87,231,223,182,187,117,204,209,79,236,238,102,
238,126,136,109,108,10,59,77,244,66,0,22,105,131,39,112,213,19,64,244,212,246,220,39,182,32,152,124,147,124,74,220,86,50,230,117,202,151,84,146,32,108,29,164,3,116,74,3,152,1,39,254,126,26,65,191,74,155,108,154,107,33,163,245,33,136,35,76,2,21,240,216,
22,197,170,223,81,184,221,132,145,195,200,133,27,82,123,130,71,156,241,115,43,94,56,234,153,173,180,116,4,20,99,114,132,223,78,206,101,102,164,8,8,36,88,45,19,170,113,180,3,239,66,71,82,87,167,105,175,229,38,144,28,86,10,83,201,214,127,225,193,38,59,
134,36,192,229,164,40,64,49,196,172,93,46,0,204,121,154,89,104,136,21,51,130,59,145,11,232,24,35,168,90,54,180,174,184,91,22,141,121,57,177,197,143,194,168,89,173,61,71,211,116,2,149,164,78,53,172,216,97,106,254,202,9,20,120,113,50,41,218,110,215,110,
14,116,177,75,24,8,240,59,137,135,230,212,81,154,56,218,131,72,184,46,160,242,96,71,166,208,140,58,218,246,37,78,210,248,141,169,1,152,103,72,72,229,135,126,88,231,216,159,4,66,68,142,163,223,50,6,218,145,208,176,185,141,115,150,253,70,165,109,76,171,
49,161,1,180,223,216,33,212,223,185,9,98,74,116,28,142,153,173,249,124,232,67,234,148,251,237,218,12,156,178,198,218,169,251,36,41,94,77,218,102,17,34,222,20,188,96,47,163,77,164,197,193,106,95,251,30,204,214,38,183,229,8,130,169,113,93,78,139,231,113,
31,167,251,39,134,210,182,253,218,231,254,197,33,198,166,176,145,84,196,0,150,16,136,179,149,212,54,138,4,56,168,114,104,76,104,0,55,161,185,18,22,205,50,3,75,108,215,70,203,240,251,12,103,32,232,195,68,132,176,80,138,182,61,23,69,49,94,118,19,44,147,
192,242,117,30,191,136,106,17,221,184,229,182,99,137,49,61,186,184,202,33,218,152,26,138,152,68,12,25,20,246,33,210,197,200,193,161,138,137,48,219,215,59,155,161,227,238,206,81,24,25,218,202,128,239,132,238,180,9,71,57,174,129,166,8,173,117,199,20,16,
228,230,202,177,115,136,6,222,178,246,155,86,48,109,145,90,88,65,174,247,158,101,236,202,200,190,203,40,119,46,139,46,121,65,173,135,140,158,117,206,162,144,73,70,88,129,179,171,228,35,184,54,51,162,221,68,68,64,80,156,225,164,144,77,90,193,83,186,168,
89,188,121,208,109,2,165,163,13,236,69,177,149,110,172,17,132,173,91,160,66,24,21,169,124,237,251,229,141,124,109,96,199,162,0,186,223,136,13,229,155,31,180,148,235,237,223,22,79,219,15,58,213,47,226,73,134,104,155,54,8,228,8,165,28,216,108,95,179,230,
66,64,0,149,28,50,24,231,2,10,96,160,22,126,79,130,66,186,213,155,137,112,48,179,151,147,112,175,129,243,32,124,248,232,170,207,132,138,226,122,9,49,227,197,239,114,202,251,16,70,68,79,109,177,6,135,152,22,160,117,93,100,22,141,234,92,128,140,188,21,
29,227,216,194,132,55,198,35,54,85,51,237,85,15,162,144,199,143,153,189,197,228,146,95,213,246,197,56,157,91,224,36,68,61,254,115,21,175,186,54,14,98,16,165,2,46,4,5,8,57,36,149,28,233,68,229,174,99,104,200,64,138,200,68,21,168,104,113,10,24,196,193,
230,197,46,148,146,171,101,20,162,208,65,109,108,249,0,86,93,43,237,32,88,179,122,8,186,85,58,2,131,100,224,141,23,188,222,130,221,141,226,242,67,168,65,215,95,199,247,231,131,184,166,30,236,86,115,118,149,70,189,6,85,180,159,160,246,118,60,96,9,78,115,
105,221,230,221,66,240,228,147,164,232,76,138,8,62,244,145,131,40,250,26,225,52,145,79,210,102,231,185,84,61,10,55,164,56,120,1,140,106,31,160,221,129,35,141,72,144,41,225,226,220,47,47,115,201,128,226,38,24,194,218,111,95,185,91,72,196,184,190,130,218,
227,230,45,21,200,154,89,30,146,58,194,122,216,34,210,42,166,179,152,221,203,72,216,182,180,34,56,111,34,236,84,83,248,6,137,253,60,197,231,132,109,112,163,9,123,47,220,222,54,210,20,206,55,116,197,39,110,99,44,25,33,122,174,80,165,8,133,153,66,90,178,
255,180,72,220,66,137,70,101,140,85,6,217,65,35,239,155,138,58,102,99,124,76,137,156,185,19,5,156,60,195,148,126,75,97,160,97,105,118,104,101,219,71,95,156,24,181,132,152,141,171,154,247,232,117,130,135,221,68,236,107,8,230,9,238,29,2,56,98,60,112,82,
229,121,12,189,38,213,240,51,21,19,22,238,26,50,208,49,13,34,37,105,182,15,106,152,61,78,112,238,62,181,89,196,84,218,142,28,220,108,202,148,175,143,167,68,76,9,56,25,69,19,78,200,36,237,118,186,183,174,97,201,44,227,196,174,219,171,97,98,205,117,243,
4,139,85,104,190,15,7,78,165,118,222,92,71,19,170,121,204,181,233,72,70,8,33,245,40,116,120,63,193,191,116,143,68,150,161,251,57,12,223,167,38,23,68,55,175,103,218,131,103,157,176,3,29,235,16,162,162,237,119,41,251,117,252,97,0,11,167,0,204,236,5,184,
98,145,33,73,58,53,237,26,39,56,76,159,212,53,173,132,221,195,118,187,87,101,124,7,154,176,61,56,81,83,169,158,14,117,39,77,250,107,143,150,240,244,78,83,122,253,150,249,102,68,155,185,89,83,187,135,12,248,68,19,252,179,141,38,54,63,247,72,193,19,208,
202,227,240,48,20,175,210,158,6,156,97,180,176,237,222,186,81,209,148,147,56,227,240,132,191,203,55,87,102,208,87,54,181,0,115,198,11,230,39,72,203,254,73,133,105,128,165,180,235,12,106,206,229,198,130,132,138,190,15,17,72,57,85,110,191,122,45,53,130,
73,130,75,61,7,22,78,77,120,12,239,91,159,113,65,170,28,75,124,0,215,236,89,89,111,126,235,128,134,245,253,0,143,108,5,184,124,33,192,77,143,152,186,121,167,114,115,229,236,92,112,195,92,101,110,28,82,106,27,38,73,155,237,161,149,247,240,230,220,167,
132,87,218,246,44,185,14,154,195,69,36,174,169,196,99,91,205,117,114,21,32,107,119,17,186,103,42,214,204,84,112,218,232,92,196,59,160,201,161,114,240,92,25,95,166,133,179,243,243,77,10,22,76,149,92,169,236,218,213,57,187,231,118,43,215,16,21,157,136,
32,208,153,138,240,8,208,222,95,162,206,100,127,117,158,128,7,112,226,151,174,87,172,41,211,14,237,21,208,113,31,192,154,120,254,114,4,108,152,150,41,38,60,116,106,173,156,154,166,82,220,82,30,66,92,78,19,70,239,233,98,212,239,193,30,167,61,104,66,232,
188,52,73,196,194,105,240,214,176,218,191,206,126,130,189,22,245,18,108,229,96,241,8,115,70,122,45,142,68,41,205,204,43,57,49,164,78,18,22,90,181,195,182,189,93,26,119,246,194,159,107,78,79,225,91,79,229,240,253,213,57,171,117,37,98,175,63,14,228,131,
127,64,223,171,12,193,161,116,206,168,243,65,73,96,103,244,74,188,150,235,103,36,138,137,161,81,143,3,232,54,127,0,140,141,215,218,242,1,162,228,139,11,183,146,196,149,95,11,191,151,64,33,7,110,125,132,68,22,17,146,139,230,73,84,199,0,119,172,50,219,
172,189,245,4,9,71,79,50,43,233,171,79,42,216,124,64,195,25,179,105,203,90,128,85,104,42,166,245,80,67,73,1,199,29,102,26,71,124,111,181,226,234,96,233,55,152,2,56,21,109,253,155,230,39,92,203,72,130,66,171,245,75,191,204,80,195,152,228,148,243,15,201,
87,32,142,227,225,227,12,129,195,237,94,86,8,22,28,74,9,224,55,189,212,174,41,86,180,249,53,143,153,213,132,68,112,165,156,130,217,244,82,20,106,3,197,88,32,132,20,90,196,58,147,96,195,187,220,82,156,104,37,41,155,182,21,81,38,207,173,198,68,4,47,219,
13,64,10,80,64,8,7,51,227,107,76,233,70,191,160,91,192,187,78,18,156,54,190,111,189,153,244,249,147,5,172,223,7,248,44,209,1,212,232,131,80,7,112,193,44,99,42,227,38,71,240,91,43,131,64,113,27,55,92,233,167,206,164,182,180,0,95,88,150,113,73,251,153,
71,72,184,224,232,4,150,163,9,40,69,223,137,142,37,7,113,83,191,225,44,86,29,27,73,132,253,139,93,218,218,224,22,246,230,19,93,40,248,245,225,38,185,48,153,233,82,74,20,248,208,22,230,96,178,205,152,72,7,183,237,226,238,179,127,126,83,8,27,187,75,27,
235,58,127,32,46,148,136,83,111,28,115,71,182,155,22,221,172,62,116,46,167,154,118,44,107,112,98,111,91,158,195,150,253,192,205,33,136,136,49,161,166,225,152,73,134,185,251,253,53,26,254,238,126,197,228,146,171,150,72,184,125,133,105,232,92,182,177,124,
142,75,242,221,248,58,57,137,31,190,43,99,178,41,93,103,220,118,197,93,198,226,118,245,14,157,163,251,37,199,77,66,12,109,71,249,4,89,100,42,233,182,12,100,1,149,180,191,83,83,137,106,42,10,215,235,20,45,160,163,148,48,165,219,38,220,62,68,140,228,65,
128,83,217,211,77,244,65,38,196,21,143,122,251,31,49,117,72,237,111,57,0,112,221,143,21,59,131,139,80,24,174,62,213,52,145,254,47,180,203,164,230,191,182,82,115,45,193,115,123,169,164,76,192,181,231,74,120,240,121,205,225,92,197,66,187,100,227,95,131,
17,197,60,52,15,159,184,159,120,124,52,9,102,162,201,204,124,107,165,242,29,66,92,226,9,236,189,164,50,84,124,250,4,146,138,167,75,31,52,115,142,21,237,56,0,30,238,181,77,177,148,142,43,148,139,40,229,152,241,1,220,228,43,21,160,83,170,123,203,20,28,
148,186,13,57,30,225,119,10,215,186,152,182,117,197,28,198,103,48,145,196,166,126,154,84,96,149,127,44,174,244,87,206,20,240,115,156,220,47,62,170,208,15,32,42,152,134,109,7,140,118,57,115,142,128,223,69,31,225,199,235,20,220,185,198,109,97,103,106,22,
79,64,243,112,246,17,102,242,7,154,198,81,163,46,35,239,58,217,148,164,173,222,69,20,116,211,61,52,87,97,98,18,25,109,113,15,109,124,3,93,212,8,46,101,29,231,255,189,143,160,194,251,121,244,123,188,16,164,28,51,56,192,193,25,56,14,159,164,89,177,49,171,
54,102,201,20,49,239,40,77,44,205,231,101,52,192,50,218,137,139,156,184,75,142,23,176,31,39,238,134,7,21,172,221,27,162,8,106,245,118,56,198,234,111,61,65,160,159,0,236,31,252,232,217,104,247,50,155,96,57,13,29,191,239,162,67,216,223,48,145,193,32,158,
235,242,19,165,233,56,186,66,177,90,190,246,85,9,220,250,120,14,203,158,215,140,7,144,80,53,50,155,242,246,221,81,131,199,175,116,123,5,80,68,5,106,203,62,130,173,138,18,185,113,44,101,167,116,254,75,233,4,198,3,225,251,244,88,228,79,29,226,120,125,136,
191,67,137,132,134,2,117,95,7,231,242,169,93,2,109,191,1,86,52,24,224,137,219,201,250,162,84,128,135,112,210,86,108,7,56,107,14,192,95,159,45,225,254,13,26,126,138,194,64,220,75,138,34,158,223,79,199,4,28,254,117,199,72,152,143,90,225,147,15,228,133,
154,198,75,78,72,24,111,32,192,200,212,3,0,119,55,247,85,69,17,92,29,247,65,138,211,196,34,34,177,22,82,208,161,154,13,6,91,112,16,25,100,76,241,1,68,187,195,7,134,12,34,45,50,214,178,89,180,66,189,67,59,21,219,130,38,137,12,121,126,41,66,66,136,86,234,
145,227,9,104,162,73,16,140,49,56,30,64,154,20,7,141,158,31,223,166,97,229,78,179,218,201,20,28,142,145,193,82,20,132,33,140,81,47,56,74,114,219,246,153,24,127,47,152,106,52,192,103,31,50,221,61,73,160,136,236,249,237,167,21,58,143,9,124,252,130,4,158,
218,105,64,35,242,25,68,236,179,68,169,95,29,243,18,116,155,75,224,106,19,219,55,152,210,6,127,232,42,71,93,66,196,161,179,161,163,86,0,148,237,188,153,200,208,245,82,139,34,186,231,83,177,162,152,72,241,27,68,138,80,34,118,144,105,208,38,212,162,129,
58,124,156,17,128,99,38,154,149,187,125,80,248,213,170,109,79,98,202,23,80,149,49,65,187,164,250,31,222,66,14,33,65,199,2,150,160,207,240,205,167,52,252,4,253,130,11,142,18,176,17,125,138,59,159,209,92,135,216,178,17,2,229,43,114,124,190,31,53,70,63,
74,49,249,11,4,255,206,193,248,159,144,197,237,131,1,32,42,164,182,221,54,243,142,254,165,161,141,107,216,174,231,205,253,146,51,75,205,52,124,91,24,8,96,208,152,208,0,50,74,4,57,27,72,130,64,136,26,217,88,128,98,91,22,104,235,38,70,100,10,157,188,64,
131,68,175,29,4,59,148,247,174,55,43,113,207,112,128,124,195,249,77,183,207,239,174,54,222,122,57,117,184,59,112,136,247,197,71,12,13,141,176,247,187,215,105,184,119,157,137,231,133,77,82,49,18,137,3,95,207,77,181,18,253,77,130,65,147,238,64,45,247,125,
211,56,29,221,214,217,141,125,160,60,56,183,208,150,93,22,158,161,102,34,33,82,255,32,224,16,219,230,142,149,40,0,14,102,176,58,65,24,110,5,252,61,142,22,226,205,18,226,98,204,118,240,35,166,146,147,9,248,209,90,71,57,119,27,81,68,225,163,133,148,247,
212,133,157,176,208,46,206,249,34,238,167,108,255,72,109,24,26,53,15,243,223,64,90,232,24,32,28,19,227,30,16,225,22,34,226,249,199,196,144,66,17,75,251,152,89,97,162,76,228,222,225,182,250,137,177,214,40,50,6,55,180,181,255,101,155,15,24,104,69,27,69,
199,155,62,199,181,252,16,146,48,78,30,28,72,148,202,162,57,73,109,131,73,247,190,178,57,250,196,35,143,118,75,214,220,109,59,175,3,141,60,234,81,148,235,40,204,180,36,85,16,224,51,151,46,151,159,41,115,12,245,252,111,230,65,104,93,164,66,188,132,82,
18,42,127,169,233,131,107,84,233,252,33,179,231,128,209,94,78,115,105,139,46,186,42,42,170,34,174,37,197,60,193,216,114,2,237,128,81,248,69,233,224,201,93,230,249,125,75,66,122,215,37,114,220,100,186,238,221,52,0,142,51,224,200,30,77,43,68,148,63,223,
107,83,197,79,239,38,155,14,112,88,13,224,202,197,230,156,131,56,112,247,109,0,120,20,85,245,226,105,6,39,152,80,17,92,122,93,225,235,153,201,241,8,160,178,37,89,173,48,161,132,28,186,247,135,178,64,58,33,218,55,9,10,149,166,63,180,9,224,13,243,4,188,
114,150,225,1,56,243,70,152,193,119,87,41,254,78,139,103,160,159,49,67,50,48,85,181,123,21,184,212,51,105,195,125,195,166,52,142,28,77,250,62,196,75,152,217,103,146,90,148,226,94,246,188,178,66,49,134,146,65,74,199,182,218,124,201,85,187,240,245,137,
38,191,78,213,189,228,232,48,195,198,18,38,90,42,80,193,105,160,105,112,104,176,27,150,87,64,152,66,201,106,131,129,38,248,76,222,160,205,221,211,196,253,98,179,37,112,212,9,158,53,131,70,222,58,145,65,142,28,175,57,28,108,229,38,107,152,217,32,189,110,
181,145,203,28,210,15,13,248,142,65,109,1,25,211,213,164,156,152,110,36,84,6,78,0,82,211,166,154,105,191,32,234,48,62,216,48,215,230,142,232,246,126,9,205,165,4,18,77,234,220,9,230,125,66,249,92,250,155,206,75,215,37,97,39,252,159,62,71,190,199,130,169,
70,155,81,111,4,231,68,139,168,109,252,232,119,2,163,80,135,51,108,218,192,181,68,240,160,213,244,179,141,70,109,171,54,130,102,59,56,34,69,177,239,176,138,184,247,142,61,91,178,125,135,41,86,191,103,93,40,59,115,236,33,26,235,109,3,130,227,124,215,8,
218,215,225,181,133,86,204,32,142,248,3,142,150,38,162,164,149,243,55,192,10,205,170,221,26,86,236,140,170,146,173,149,119,81,1,29,74,144,51,229,41,138,102,95,251,251,8,147,107,132,207,165,158,151,111,211,156,176,138,125,34,61,194,13,99,71,82,0,10,157,
81,10,217,46,235,49,147,132,63,177,205,168,118,120,1,71,47,238,152,217,158,19,72,100,97,32,66,189,157,157,52,10,243,218,9,165,46,133,156,200,16,66,130,173,86,58,40,186,108,171,193,246,141,172,218,170,137,147,40,118,45,91,150,179,46,248,117,193,209,243,
41,112,155,70,86,170,152,228,210,81,197,19,251,61,168,165,54,237,55,254,7,117,20,209,177,183,92,116,172,245,136,45,214,17,22,2,93,104,200,24,197,246,90,5,10,148,251,201,219,154,38,197,43,211,161,135,113,157,124,124,94,25,61,199,4,210,104,3,14,207,251,
139,67,79,13,194,87,10,123,169,85,49,108,43,60,47,161,208,109,52,186,166,214,1,186,19,1,1,46,134,173,81,18,71,68,59,128,20,90,207,129,40,180,141,245,187,134,196,209,83,177,48,84,143,70,19,16,19,119,242,82,181,183,82,65,199,175,149,20,165,190,189,39,46,
143,97,110,202,161,29,4,75,15,19,127,155,155,115,161,225,161,42,98,148,45,36,73,147,67,39,164,220,245,148,58,24,158,54,161,155,240,25,56,95,189,163,99,115,19,232,221,90,31,164,138,219,206,117,80,239,137,67,212,22,192,65,125,140,116,187,22,212,47,16,250,
218,106,36,153,114,125,121,62,26,77,128,178,55,150,173,249,201,173,119,111,95,117,196,166,172,149,83,85,127,154,229,198,137,117,164,16,1,135,224,246,139,118,53,218,86,78,173,219,38,49,154,48,41,95,88,0,218,127,47,12,124,97,53,191,240,218,10,247,170,15,
50,109,193,124,196,26,76,31,114,137,28,98,135,184,131,175,81,104,88,194,229,225,44,231,82,138,166,72,74,195,3,59,55,108,115,202,115,36,180,193,72,106,0,199,246,111,61,122,251,199,111,197,103,244,123,97,28,1,111,54,36,239,104,103,242,223,224,31,154,104,
10,48,7,240,177,219,62,231,246,161,94,172,16,164,35,52,249,218,222,16,5,104,131,246,188,37,123,131,131,86,0,196,203,115,249,43,143,45,129,195,24,67,65,221,142,103,221,46,182,81,165,1,98,1,80,246,38,43,246,26,226,101,1,120,81,166,85,217,177,173,71,130,
144,65,49,171,62,42,124,128,150,21,134,204,222,164,235,223,249,242,228,191,120,45,144,217,69,214,178,143,28,70,0,19,24,233,40,32,111,211,6,241,202,127,89,8,126,245,113,133,200,222,143,136,237,31,17,1,160,141,152,149,82,135,18,130,67,181,194,125,89,0,
94,156,0,232,23,120,134,102,179,249,235,17,128,9,19,38,192,59,223,249,206,152,182,172,169,75,67,169,119,178,110,237,223,246,242,196,143,188,48,28,50,88,157,63,127,254,175,124,82,161,71,186,231,200,203,63,99,234,231,229,216,252,127,248,207,255,19,96,0,
221,83,18,25,240,8,112,38,0,0,0,0,73,69,78,68,174,66,96,130,0,0 };

const char* projectIconXcode_png = (const char*) temp_binary_data_30;

//================== projectIconXcodeIOS.png ==================
static const unsigned char temp_binary_data_31[] =
{ 137,80,78,71,13,10,26,10,0,0,0,13,73,72,68,82,0,0,0,128,0,0,0,128,8,6,0,0,0,195,62,97,203,0,0,0,25,116,69,88,116,83,111,102,116,119,97,114,101,0,65,100,111,98,101,32,73,109,97,103,101,82,101,97,100,121,113,201,101,60,0,0,3,134,105,84,88,116,88,77,76,
58,99,111,109,46,97,100,111,98,101,46,120,109,112,0,0,0,0,0,60,63,120,112,97,99,107,101,116,32,98,101,103,105,110,61,34,239,187,191,34,32,105,100,61,34,87,53,77,48,77,112,67,101,104,105,72,122,114,101,83,122,78,84,99,122,107,99,57,100,34,63,62,32,60,
120,58,120,109,112,109,101,116,97,32,120,109,108,110,115,58,120,61,34,97,100,111,98,101,58,110,115,58,109,101,116,97,47,34,32,120,58,120,109,112,116,107,61,34,65,100,111,98,101,32,88,77,80,32,67,111,114,101,32,53,46,54,45,99,48,49,52,32,55,57,46,49,53,
54,55,57,55,44,32,50,48,49,52,47,48,56,47,50,48,45,48,57,58,53,51,58,48,50,32,32,32,32,32,32,32,32,34,62,32,60,114,100,102,58,82,68,70,32,120,109,108,110,115,58,114,100,102,61,34,104,116,116,112,58,47,47,119,119,119,46,119,51,46,111,114,103,47,49,57,
57,57,47,48,50,47,50,50,45,114,100,102,45,115,121,110,116,97,120,45,110,115,35,34,62,32,60,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,32,114,100,102,58,97,98,111,117,116,61,34,34,32,120,109,108,110,115,58,120,109,112,77,77,61,34,104,116,
116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,109,109,47,34,32,120,109,108,110,115,58,115,116,82,101,102,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,
115,84,121,112,101,47,82,101,115,111,117,114,99,101,82,101,102,35,34,32,120,109,108,110,115,58,120,109,112,61,34,104,116,116,112,58,47,47,110,115,46,97,100,111,98,101,46,99,111,109,47,120,97,112,47,49,46,48,47,34,32,120,109,112,77,77,58,79,114,105,103,
105,110,97,108,68,111,99,117,109,101,110,116,73,68,61,34,120,109,112,46,100,105,100,58,51,51,101,53,51,101,51,102,45,98,98,100,52,45,52,48,99,99,45,98,54,100,55,45,53,100,52,100,102,52,50,56,99,56,52,54,34,32,120,109,112,77,77,58,68,111,99,117,109,101,
110,116,73,68,61,34,120,109,112,46,100,105,100,58,49,51,54,56,69,69,55,52,52,67,55,57,49,49,69,52,57,54,50,67,65,49,51,66,54,69,53,52,48,69,51,54,34,32,120,109,112,77,77,58,73,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,49,51,
54,56,69,69,55,51,52,67,55,57,49,49,69,52,57,54,50,67,65,49,51,66,54,69,53,52,48,69,51,54,34,32,120,109,112,58,67,114,101,97,116,111,114,84,111,111,108,61,34,65,100,111,98,101,32,80,104,111,116,111,115,104,111,112,32,67,67,32,50,48,49,52,32,40,77,97,
99,105,110,116,111,115,104,41,34,62,32,60,120,109,112,77,77,58,68,101,114,105,118,101,100,70,114,111,109,32,115,116,82,101,102,58,105,110,115,116,97,110,99,101,73,68,61,34,120,109,112,46,105,105,100,58,97,101,56,50,49,51,51,49,45,49,102,97,102,45,52,
54,50,98,45,98,55,53,49,45,100,52,97,48,53,100,55,52,102,56,54,100,34,32,115,116,82,101,102,58,100,111,99,117,109,101,110,116,73,68,61,34,97,100,111,98,101,58,100,111,99,105,100,58,112,104,111,116,111,115,104,111,112,58,55,51,55,51,98,57,100,51,45,57,
52,100,100,45,49,49,55,55,45,97,53,100,98,45,56,53,99,49,100,48,98,53,54,97,53,50,34,47,62,32,60,47,114,100,102,58,68,101,115,99,114,105,112,116,105,111,110,62,32,60,47,114,100,102,58,82,68,70,62,32,60,47,120,58,120,109,112,109,101,116,97,62,32,60,63,
120,112,97,99,107,101,116,32,101,110,100,61,34,114,34,63,62,236,65,16,56,0,0,66,207,73,68,65,84,120,218,236,125,7,124,28,213,157,255,247,189,153,45,146,44,201,189,247,130,113,5,108,156,64,232,112,148,4,114,144,0,151,127,192,164,28,185,75,15,23,184,75,
46,151,92,146,203,125,46,201,229,18,72,114,185,112,164,83,19,122,66,9,189,184,226,130,141,193,54,238,189,219,146,37,89,109,87,187,51,239,255,126,175,204,188,93,175,100,201,182,44,147,67,254,140,87,154,157,157,153,157,95,251,254,234,99,66,8,28,237,79,
54,155,197,186,117,235,112,44,231,120,247,231,216,127,70,140,24,129,190,125,251,30,213,103,253,99,185,240,182,109,219,112,206,57,231,32,12,195,82,111,179,191,160,103,124,82,115,248,93,119,221,133,217,179,103,159,120,6,32,194,55,53,53,21,19,157,153,7,
198,138,24,129,157,36,132,20,37,24,149,181,67,104,230,236,19,69,191,159,52,63,185,92,238,168,63,235,31,235,197,57,231,86,3,216,7,233,129,177,4,4,147,175,224,116,72,187,12,64,166,131,177,19,70,123,198,125,230,247,26,152,146,215,100,246,250,249,166,3,25,
17,230,133,115,79,97,194,103,129,207,145,15,114,97,91,155,64,80,226,121,133,237,48,83,143,252,176,99,120,134,254,241,186,7,67,104,143,110,103,236,236,187,127,81,54,248,212,247,136,48,203,132,124,84,34,16,44,204,67,120,105,166,158,24,103,180,15,26,59,
112,253,105,250,18,234,105,6,66,171,15,121,144,134,22,250,25,139,208,188,202,207,49,63,230,39,218,79,223,95,189,111,88,48,204,135,96,30,139,153,12,230,90,158,207,253,138,1,41,97,5,91,126,38,215,176,47,43,68,62,100,92,31,19,10,17,244,43,99,45,233,48,104,
14,155,247,29,100,109,123,119,121,117,27,215,182,237,95,179,98,235,186,55,223,58,80,223,92,47,63,156,112,24,33,124,39,152,137,19,197,0,242,193,136,68,217,128,177,19,122,141,152,62,33,200,182,90,242,33,144,132,171,42,227,168,72,49,236,105,12,52,221,153,
33,178,101,4,58,90,40,110,32,65,140,137,44,153,130,132,148,39,25,2,41,146,220,220,181,58,132,195,97,16,161,206,37,138,245,141,48,12,192,228,123,249,124,244,187,210,96,169,170,232,88,97,62,223,36,207,117,136,180,90,197,68,169,225,128,228,168,60,6,164,
90,197,229,87,109,93,153,223,248,204,189,243,158,125,248,158,93,251,14,214,200,143,36,229,150,119,52,194,255,73,6,136,85,191,126,32,21,34,159,9,69,62,43,127,109,83,18,173,68,69,62,216,193,149,62,6,246,242,176,237,64,6,105,159,33,116,45,108,94,211,94,
169,106,197,0,6,70,48,205,0,138,54,121,173,61,194,0,145,212,179,80,196,166,154,185,26,193,144,148,24,34,208,196,229,30,115,172,125,104,152,38,175,52,129,58,78,104,13,196,60,33,191,12,253,29,64,100,243,104,149,23,220,84,159,101,235,243,125,167,15,31,114,
211,127,93,123,235,197,31,91,245,204,207,190,250,242,171,243,158,119,152,0,39,147,89,232,180,9,63,142,26,192,50,64,185,8,67,95,17,144,136,33,31,58,109,82,240,48,176,66,50,64,165,103,121,2,134,222,32,245,203,36,67,208,63,110,237,153,145,110,230,226,5,
171,44,98,122,199,234,157,46,151,13,141,66,102,218,46,10,125,30,70,210,30,26,45,65,247,100,142,103,14,3,146,201,209,247,33,52,19,24,13,197,36,24,32,156,147,240,184,252,114,33,118,238,111,192,195,111,39,167,141,187,234,107,143,207,190,225,163,95,52,103,
72,26,97,98,239,52,239,135,31,199,115,49,243,16,210,242,129,123,220,216,96,230,216,239,129,85,28,30,215,196,137,30,184,135,66,66,105,11,160,143,49,68,35,6,226,9,115,62,251,158,18,119,195,52,134,201,152,97,152,8,19,9,251,158,36,162,175,223,183,231,87,
92,24,144,86,208,12,24,169,50,69,116,166,180,142,58,187,48,231,246,61,121,15,30,146,41,15,62,15,240,232,146,3,169,202,51,63,118,199,151,190,240,185,175,152,111,144,120,39,50,193,241,198,0,6,7,144,32,51,120,9,46,109,191,64,155,180,223,45,210,118,79,24,
232,99,71,93,160,126,103,48,42,159,176,153,208,106,183,45,208,39,72,250,26,16,230,115,26,59,72,8,169,25,70,30,31,228,181,253,166,143,166,19,250,184,214,140,196,8,146,81,146,242,111,73,39,100,114,161,50,19,33,17,217,115,181,132,84,237,92,51,85,62,8,245,
181,67,109,158,104,127,66,158,55,32,207,128,62,47,49,64,32,143,11,233,166,228,245,149,214,9,57,114,121,105,28,66,174,110,252,183,207,111,196,215,175,255,240,191,221,122,11,227,183,255,228,127,126,224,128,195,252,59,5,24,250,221,160,81,120,144,149,15,
49,35,159,44,23,144,152,15,83,70,166,80,38,31,205,184,254,62,134,87,123,216,221,80,134,65,85,30,42,229,155,13,45,2,155,106,115,104,106,13,113,214,152,20,106,91,66,188,181,187,77,61,240,65,18,51,12,170,228,168,74,113,52,181,133,138,113,70,244,246,208,
34,25,227,144,84,247,75,182,182,33,35,95,175,61,45,13,82,56,27,106,114,138,193,166,13,73,32,37,191,89,133,84,204,41,201,132,229,242,218,89,201,12,135,228,185,15,202,235,120,82,221,12,168,224,138,216,181,205,129,250,236,238,250,60,182,29,204,161,74,122,
42,3,202,25,202,229,189,85,38,5,164,213,146,204,16,160,174,37,135,150,140,143,254,242,196,217,22,134,250,6,121,190,186,60,94,88,178,14,223,249,228,167,190,197,189,100,217,15,111,191,227,223,13,19,16,187,228,222,9,76,224,31,103,19,96,98,3,208,210,79,160,
77,74,243,213,211,211,152,57,34,137,62,210,11,200,72,9,254,127,51,24,106,228,131,175,109,22,146,56,66,238,243,208,86,225,225,194,9,41,212,183,10,52,74,230,33,105,191,229,194,74,72,97,68,43,57,227,82,26,155,228,235,182,131,1,250,72,194,85,101,56,54,247,
202,35,95,198,112,237,25,229,242,124,33,114,114,255,250,253,121,244,151,76,243,129,41,101,72,74,233,111,206,210,103,73,162,233,6,53,45,154,36,211,212,52,5,138,1,166,15,75,160,190,133,152,198,199,235,59,164,135,114,48,143,27,103,149,99,84,95,31,141,217,
64,106,175,16,89,169,5,242,249,4,154,37,3,108,219,31,162,158,37,80,229,37,177,55,76,161,45,219,136,123,158,127,27,255,246,15,95,250,74,46,8,189,159,252,228,39,223,118,158,235,73,239,33,248,221,112,78,198,165,228,121,242,225,11,41,3,68,240,239,62,223,
40,213,187,192,215,47,171,82,232,252,235,79,53,160,140,92,58,66,222,242,111,146,192,132,84,223,79,175,108,85,158,129,111,240,195,107,91,106,145,205,107,226,91,242,229,66,253,25,58,34,41,143,243,165,150,249,232,111,107,212,126,210,26,9,185,239,254,165,
205,120,112,121,139,14,244,144,41,32,196,79,102,70,104,230,36,179,148,183,110,163,113,253,232,149,24,134,84,255,151,31,203,202,243,72,147,164,76,73,168,204,139,252,148,116,69,115,200,181,101,228,247,106,65,144,105,129,200,100,228,107,128,178,29,91,49,
251,146,83,113,235,151,111,185,77,2,198,228,29,119,220,241,175,206,243,56,169,153,160,59,24,64,114,0,39,103,46,14,250,200,95,200,11,104,150,18,60,168,23,87,132,43,243,227,168,177,205,37,121,41,166,247,8,13,198,114,4,210,200,78,219,160,14,35,132,201,21,
3,8,27,69,20,68,116,185,209,133,184,80,224,173,44,193,141,75,40,191,160,10,240,232,107,209,123,66,114,129,122,91,157,39,52,84,177,129,33,249,183,60,103,168,2,66,80,120,130,190,128,186,30,29,35,17,99,82,110,66,110,121,225,203,195,165,71,19,250,168,175,
107,192,171,111,238,192,228,209,3,240,249,207,127,254,139,158,231,37,127,244,163,31,125,205,73,146,157,180,76,192,187,229,164,92,19,94,187,116,177,75,70,23,43,79,176,8,121,235,141,8,161,9,236,41,39,223,184,120,146,80,68,120,30,121,13,92,159,135,105,207,
65,123,3,218,231,87,104,157,179,248,149,105,169,214,239,9,115,29,131,230,5,139,206,99,99,6,234,54,67,199,53,204,17,199,10,27,164,212,177,9,17,123,22,244,5,184,84,113,140,123,138,145,146,146,3,231,173,220,142,192,60,206,207,124,230,51,159,254,246,183,
191,253,19,201,8,105,227,30,39,0,231,116,127,233,12,96,133,74,61,120,243,128,61,41,241,153,156,208,129,62,196,62,120,32,193,29,231,250,193,210,62,158,224,42,210,167,136,174,159,117,76,72,218,103,93,70,30,187,123,214,141,19,134,232,134,37,244,231,120,
76,112,102,142,85,76,5,27,47,136,239,213,146,71,223,131,245,71,227,239,160,54,227,122,170,71,71,76,201,137,1,18,216,184,189,22,251,234,90,81,213,171,66,221,199,13,55,220,112,211,247,190,255,253,187,82,233,116,149,235,29,157,108,76,208,125,12,32,156,32,
143,121,200,244,224,73,253,147,102,176,118,220,35,141,224,67,109,194,70,214,3,22,153,5,97,205,116,88,152,86,36,201,211,177,128,248,98,156,179,66,127,30,78,48,200,156,72,41,35,19,8,82,199,135,90,43,232,107,11,163,21,76,228,49,20,135,197,247,84,192,202,
106,37,82,69,146,1,164,164,163,190,57,131,205,123,234,80,217,171,28,233,116,90,93,227,67,215,92,115,237,15,255,235,135,191,173,168,168,232,103,110,43,105,52,2,255,139,102,0,114,3,67,199,19,86,196,147,95,191,77,62,80,10,1,115,43,133,86,197,131,21,70,245,
96,136,19,196,24,141,217,176,161,97,170,136,32,196,76,121,109,58,132,139,235,34,149,206,98,173,16,49,136,171,33,226,172,38,12,195,168,248,129,141,100,90,166,9,153,9,86,25,179,102,76,146,13,106,5,18,44,110,216,81,139,116,89,153,98,128,50,249,74,223,229,
138,43,46,191,226,246,219,111,191,191,111,223,190,195,205,87,116,53,1,251,139,100,0,146,106,82,249,42,26,40,152,145,52,224,144,116,241,18,62,161,126,22,135,239,185,19,161,227,218,123,136,34,115,242,213,151,234,216,179,82,199,153,81,205,208,193,32,131,
7,132,77,137,90,102,241,28,149,110,164,221,134,142,137,208,42,83,200,221,76,36,140,249,113,24,82,69,255,98,18,209,125,69,215,54,121,9,205,25,60,194,57,219,247,214,41,85,150,74,165,10,152,224,162,139,46,58,239,103,63,251,217,131,195,135,15,31,239,132,
142,189,118,211,228,93,252,105,167,32,167,39,189,0,253,176,148,212,8,109,187,185,199,213,243,202,153,52,176,146,121,227,34,4,57,104,109,224,49,99,199,13,54,247,180,41,176,201,31,37,229,204,18,59,78,2,121,73,141,13,116,118,80,159,131,176,131,16,214,101,
100,176,31,177,192,175,176,214,195,102,2,205,125,48,173,145,66,226,144,64,187,144,214,210,48,3,4,9,34,82,185,3,243,180,38,224,242,75,215,54,52,171,243,86,86,86,66,170,125,85,50,151,145,174,34,21,205,204,154,53,235,12,201,4,143,220,122,235,173,31,223,
188,121,243,27,134,9,114,208,122,14,163,71,143,22,201,100,18,109,109,109,71,204,239,7,65,16,21,129,208,177,246,90,180,63,33,241,8,49,132,122,6,148,195,144,127,119,116,62,191,123,20,139,126,184,140,91,233,211,162,73,97,93,223,51,234,29,134,57,12,106,103,
38,237,171,81,62,43,192,18,148,241,99,38,23,16,102,69,204,44,70,157,107,242,57,121,4,206,34,60,161,222,229,90,5,132,197,169,228,8,245,81,86,145,233,116,113,232,228,18,40,20,28,88,198,48,218,197,104,33,63,73,9,46,102,56,146,43,6,63,212,162,137,242,230,
138,21,88,248,218,107,216,179,103,15,202,203,203,113,202,41,167,96,242,228,201,152,49,99,198,41,119,222,121,231,195,183,221,118,219,39,87,173,90,53,215,9,29,7,253,251,247,199,79,127,250,83,65,175,173,173,173,237,18,205,247,125,117,94,42,199,35,236,65,
4,62,237,180,211,176,110,237,90,28,60,120,16,35,71,141,82,159,39,70,34,45,52,102,204,24,245,122,98,53,64,164,146,153,2,95,194,213,12,44,70,229,42,234,35,12,0,52,118,62,84,17,100,253,62,61,124,101,22,210,60,2,123,44,197,148,70,16,110,226,213,160,250,32,
31,42,109,16,3,4,125,13,197,88,70,43,69,230,37,20,198,199,23,6,4,198,26,37,82,48,66,19,59,74,47,91,240,106,235,24,12,179,209,47,94,34,137,129,189,24,110,249,210,23,112,223,253,191,87,196,176,63,100,18,102,206,156,137,171,175,190,154,204,193,240,223,252,
230,55,15,125,237,107,95,251,187,151,94,122,233,73,203,4,175,191,254,122,32,221,71,252,238,119,191,19,189,123,247,70,75,75,75,73,38,32,162,211,70,210,205,13,0,141,10,156,142,194,20,248,221,168,0,244,195,226,154,9,236,67,165,176,46,69,214,148,137,224,
136,129,154,5,118,66,68,40,93,171,107,22,87,24,26,198,224,158,208,102,193,115,188,3,147,124,130,136,3,75,164,105,72,250,121,16,155,27,29,64,130,49,17,49,222,176,231,38,77,64,166,75,129,63,207,42,232,168,52,65,51,129,101,110,242,68,228,121,37,223,225,
125,19,251,97,199,107,15,226,158,87,95,61,236,113,144,122,94,184,112,161,218,198,141,27,135,15,126,240,131,125,62,247,185,207,221,83,93,93,125,203,19,79,60,113,95,62,79,241,74,240,183,222,122,43,148,251,241,163,31,253,72,12,25,50,4,205,205,205,37,237,
189,85,241,170,130,201,252,30,165,74,79,6,16,104,137,105,165,5,44,70,224,20,11,8,173,93,55,25,62,173,189,227,116,175,253,172,2,127,70,234,172,202,214,96,157,69,238,101,4,204,28,215,47,70,254,113,204,65,69,1,45,179,145,196,7,135,223,51,99,177,148,171,
247,77,80,42,58,214,137,39,40,76,67,196,207,11,12,27,80,133,244,129,165,120,165,4,241,139,127,54,109,218,132,31,255,248,199,248,236,103,63,91,46,255,252,229,53,215,92,243,208,229,151,95,126,153,244,18,136,221,194,69,139,22,137,235,174,187,142,45,93,186,
84,97,137,119,102,28,32,140,226,36,234,193,43,155,111,226,240,9,131,1,20,170,102,142,51,36,52,14,160,232,159,34,122,16,71,4,85,48,8,49,162,143,136,235,32,120,197,32,194,197,1,60,246,223,13,168,227,60,118,39,41,216,67,140,160,204,16,115,98,16,194,68,14,
61,3,242,24,51,140,104,181,128,214,38,154,97,164,251,39,193,252,41,125,242,120,93,21,7,117,254,103,255,254,253,120,236,177,199,240,252,243,207,127,80,218,235,103,174,191,254,250,23,102,207,158,253,137,137,19,39,246,150,54,94,220,116,211,77,120,77,226,
136,170,170,170,119,30,3,8,235,114,121,214,21,212,196,37,19,117,40,43,76,205,31,139,142,139,2,60,158,245,177,161,64,22,17,133,106,0,21,202,246,221,114,174,200,147,143,24,69,17,215,115,36,95,56,197,35,145,173,142,25,34,138,38,250,110,20,194,122,35,92,
135,178,125,253,190,5,155,113,181,146,62,121,78,222,199,153,163,164,203,87,179,20,219,118,238,61,170,103,117,232,208,33,188,242,202,43,248,213,175,126,117,158,52,1,191,61,247,220,115,151,222,124,243,205,223,146,192,110,220,45,183,220,130,151,95,126,89,
1,201,119,86,46,192,99,38,198,175,37,43,52,68,106,205,9,147,35,128,99,219,227,232,157,181,245,204,85,233,140,21,72,187,213,0,156,91,70,51,72,220,134,119,141,219,103,189,2,91,113,108,85,182,214,6,194,184,132,214,244,24,162,115,30,49,67,4,92,109,46,195,
220,88,156,151,128,42,34,185,104,124,2,43,151,47,138,98,17,71,91,162,77,46,156,100,0,220,119,223,125,227,37,104,252,182,100,132,55,36,86,248,221,115,207,61,119,30,153,13,138,41,188,115,76,128,64,4,252,44,33,73,181,55,102,132,42,10,213,217,184,184,238,
46,162,186,249,156,102,26,77,64,10,42,217,50,95,110,8,8,83,2,174,131,61,177,137,160,232,99,24,88,155,206,34,149,239,170,111,197,156,96,5,173,43,81,82,137,199,196,182,132,6,119,253,127,68,39,11,37,120,31,232,31,194,134,165,207,99,213,219,27,10,241,196,
49,48,2,197,2,38,76,152,64,113,129,202,1,3,6,124,92,2,197,185,210,76,204,221,188,121,243,199,36,99,84,146,71,161,163,150,39,117,58,88,63,76,178,157,22,160,210,133,106,90,132,242,2,124,66,231,182,2,152,171,138,171,72,69,171,162,79,83,181,77,229,95,161,
169,7,180,193,27,37,249,78,255,17,115,218,78,60,223,162,121,85,223,31,217,254,168,183,192,154,39,121,78,93,138,104,60,3,232,207,68,149,199,170,38,208,154,12,174,99,118,100,107,2,68,145,77,226,207,81,213,1,214,47,88,133,108,59,193,27,119,95,103,251,39,
233,51,228,195,147,237,31,59,118,172,10,248,72,76,112,222,239,127,255,251,243,250,244,233,243,205,179,207,62,251,62,249,222,147,210,255,127,11,113,213,209,73,198,0,208,126,117,104,220,56,229,206,65,215,97,38,19,26,232,229,77,97,135,170,211,163,18,241,
80,196,182,56,10,251,234,24,130,242,209,121,164,8,116,70,80,104,2,210,131,37,156,160,162,129,166,152,211,154,134,40,250,103,65,129,169,10,178,45,7,220,198,248,141,70,144,119,172,25,81,232,96,147,200,235,64,149,174,95,231,170,150,192,154,149,68,194,71,
185,124,122,43,222,124,171,211,132,165,243,86,86,247,65,42,93,142,134,186,3,138,208,165,142,163,239,36,93,67,69,124,250,123,248,240,225,170,1,180,166,166,102,220,75,47,189,244,173,214,214,214,111,13,26,52,232,205,145,35,71,254,90,190,62,156,76,36,246,
230,74,156,171,231,226,0,86,186,66,146,94,83,169,109,98,20,41,146,104,129,56,69,203,161,154,62,136,192,54,240,195,152,115,142,192,137,206,9,199,107,96,186,239,76,249,236,73,99,243,67,39,8,101,19,248,92,167,112,133,45,92,87,230,37,212,62,190,99,26,52,
211,112,85,156,42,28,70,12,178,161,202,9,8,91,71,192,169,208,149,99,104,31,31,245,111,189,132,218,250,166,46,212,73,48,36,123,15,130,72,244,66,101,144,69,109,77,77,187,12,96,253,123,171,17,232,71,106,0,72,179,160,130,68,59,119,238,60,109,241,226,197,
63,149,38,226,235,101,233,244,195,146,73,126,157,72,38,87,116,213,60,116,27,3,112,35,241,214,223,231,92,151,118,81,173,133,159,148,186,171,77,152,200,32,21,219,234,232,94,190,45,84,238,161,151,50,5,58,57,91,252,97,170,139,60,68,161,89,21,182,117,77,129,
219,94,38,68,65,240,70,55,2,177,232,198,40,178,168,202,190,173,218,119,178,190,186,231,128,71,218,192,75,114,221,165,68,102,37,208,193,159,16,73,140,74,236,193,220,215,94,42,32,90,177,218,63,12,232,201,239,223,90,187,11,101,149,213,104,168,175,87,251,
250,245,235,135,94,189,122,97,239,222,189,42,96,100,51,147,54,224,195,109,33,12,116,19,40,105,6,10,7,83,120,153,76,132,252,220,160,199,30,127,252,11,190,231,125,122,198,204,153,191,28,50,100,200,29,18,71,108,108,235,164,70,232,22,6,208,234,159,50,121,
44,46,212,48,78,63,183,136,158,91,162,8,83,245,67,154,192,184,141,161,137,238,155,136,161,138,223,91,83,194,227,115,18,232,227,54,75,231,214,15,68,133,30,70,221,187,193,41,83,120,66,140,69,166,35,210,58,42,61,109,35,127,38,60,140,56,226,39,76,81,10,157,
174,119,101,10,193,182,185,216,178,125,143,10,197,146,100,82,210,135,98,240,132,230,219,99,4,218,215,220,212,168,54,251,67,4,149,170,28,147,38,77,130,4,122,104,104,104,80,175,68,96,174,180,77,16,167,157,205,70,223,157,174,71,191,15,29,58,20,195,134,13,
163,208,115,98,197,138,21,159,123,227,141,55,110,184,242,202,43,191,51,126,252,248,59,58,131,59,186,205,13,140,130,39,208,42,62,215,42,84,34,136,170,116,41,145,165,136,111,187,125,242,90,12,19,41,29,3,16,70,10,20,97,13,162,143,138,60,172,91,103,64,38,
108,18,201,48,69,84,20,18,67,255,168,57,37,170,1,48,226,206,125,74,53,115,211,84,194,226,54,79,227,119,234,235,113,181,41,55,82,186,47,82,205,226,194,222,27,177,114,254,147,234,112,202,196,17,114,167,168,29,37,114,164,74,86,127,219,80,109,41,38,112,55,
34,248,188,121,243,176,100,201,18,149,180,153,62,125,58,36,17,241,200,35,143,96,203,150,45,145,139,72,155,213,10,238,70,146,78,154,131,174,75,249,134,49,99,198,244,150,159,189,125,213,170,85,223,164,123,58,18,19,116,79,65,72,62,78,236,208,13,144,43,71,
132,77,153,160,74,32,98,184,168,10,71,120,236,57,16,243,248,190,174,27,180,46,155,46,202,140,85,186,27,224,81,89,65,235,54,134,69,104,219,168,126,93,184,195,11,195,229,70,105,144,231,192,173,6,48,69,35,220,179,26,201,4,138,76,45,161,4,27,232,143,90,148,
237,91,140,183,223,94,171,206,105,115,254,214,94,83,208,134,166,117,208,102,125,247,246,152,193,101,8,74,30,81,28,96,249,242,229,138,169,136,49,136,17,44,30,176,76,208,30,51,144,121,32,13,68,222,195,140,25,51,240,226,139,47,254,219,161,67,135,206,37,
13,213,35,110,160,193,80,17,160,242,18,42,181,174,186,131,19,190,78,160,168,206,28,147,254,181,85,56,42,26,24,37,115,16,213,3,184,231,181,7,232,191,37,195,164,88,97,174,223,139,43,130,192,29,147,96,42,149,169,68,189,34,201,85,60,129,42,149,147,38,181,
108,110,56,82,247,186,189,45,174,77,8,67,142,65,126,61,230,191,244,50,178,185,188,34,20,61,96,75,124,215,221,35,127,157,54,82,241,68,152,206,152,7,251,217,179,206,58,11,117,117,117,74,170,173,45,183,239,187,102,160,88,155,168,74,106,201,8,116,93,98,192,
133,11,23,222,54,101,202,148,249,39,190,34,200,216,219,48,116,170,109,228,127,77,173,218,69,75,250,142,83,110,9,199,11,213,176,173,199,211,121,3,166,107,7,77,152,22,70,43,184,1,26,85,243,31,37,157,204,3,49,24,209,208,85,225,138,114,73,248,51,135,167,
84,191,192,231,47,168,194,128,10,15,153,54,17,39,130,204,185,40,8,164,43,145,226,234,173,132,188,113,105,254,165,235,183,82,221,91,169,232,156,27,4,34,34,17,96,43,54,15,71,138,11,144,102,33,187,78,218,132,8,90,74,250,59,210,8,100,18,8,92,110,219,182,
237,125,242,247,222,61,144,14,182,89,190,56,220,70,68,200,72,149,74,157,63,194,20,95,112,203,201,102,24,132,125,38,249,156,205,4,138,8,226,51,91,196,97,63,231,196,5,108,64,200,101,10,1,75,60,97,29,74,117,254,102,233,214,253,245,244,94,170,79,241,210,
83,211,152,53,34,137,79,255,161,22,117,173,129,234,33,160,36,144,66,253,38,222,15,21,11,144,96,76,94,119,72,149,143,218,215,254,136,3,117,141,145,132,119,68,72,151,17,172,121,32,166,33,169,38,141,64,64,174,148,247,64,90,195,130,60,218,92,111,128,94,75,
73,127,241,62,210,76,146,121,250,73,151,113,160,188,207,250,19,31,10,102,84,53,195,11,50,119,190,52,3,117,173,66,97,0,43,173,204,77,176,24,137,247,163,170,29,230,212,252,64,5,139,220,92,127,84,92,98,180,136,112,184,40,10,1,131,69,113,4,250,147,36,255,
222,37,205,184,237,226,74,108,174,201,43,183,243,59,87,245,150,151,103,78,37,51,143,170,127,109,141,69,200,210,24,201,118,97,249,188,63,171,191,73,253,119,54,244,235,18,201,154,7,42,250,32,173,64,46,32,105,9,151,17,172,84,19,35,20,75,186,187,207,149,
250,98,109,64,199,73,38,104,148,12,211,208,3,245,0,6,205,219,136,156,177,215,101,82,141,83,195,102,40,226,134,12,170,7,204,59,101,94,136,203,241,163,148,171,245,2,20,234,15,13,35,56,181,3,54,249,20,213,30,176,194,212,164,197,16,116,28,1,209,117,251,243,
88,178,173,77,183,173,189,120,8,103,141,78,225,170,169,101,42,70,97,91,204,21,16,144,91,130,170,145,164,86,232,159,110,67,126,253,211,216,180,101,151,34,88,169,50,43,87,2,59,98,132,98,243,64,234,154,24,194,50,2,17,207,18,186,248,181,61,102,40,222,168,
152,68,158,115,161,188,212,190,19,207,0,78,182,143,251,174,93,214,21,65,170,95,143,122,254,242,20,194,53,189,1,22,128,133,113,35,137,237,31,136,170,134,140,27,224,113,86,160,89,224,164,159,163,44,35,226,250,189,68,74,51,7,161,122,242,80,232,243,15,190,
209,138,115,198,164,84,145,234,61,75,73,35,84,99,72,181,111,204,77,92,237,163,157,71,31,31,24,184,17,111,205,121,52,114,253,44,186,238,12,177,59,99,30,200,52,80,60,129,54,235,255,219,192,143,187,21,19,190,148,70,160,31,98,128,81,163,70,253,134,222,63,
241,12,96,108,190,239,179,168,1,67,251,247,12,123,155,116,168,149,8,172,139,66,52,200,179,1,25,110,250,246,220,180,111,65,70,207,182,120,197,137,185,40,239,160,226,1,161,112,58,143,152,157,59,227,48,138,68,253,242,28,11,54,183,225,205,93,57,220,52,171,
2,247,45,107,65,67,38,196,63,93,82,173,238,89,71,31,133,137,255,39,209,59,216,133,218,213,47,98,229,234,245,138,56,100,203,93,219,223,145,228,119,197,60,208,70,204,69,204,64,102,130,52,130,85,231,29,49,65,49,51,16,190,144,231,216,48,116,232,208,167,123,
134,1,56,67,220,180,99,186,118,168,248,65,62,228,87,54,73,55,197,72,188,16,44,206,8,58,245,30,214,214,199,133,151,113,47,160,103,136,79,132,37,176,168,92,73,95,199,14,10,190,152,173,51,140,147,7,170,200,196,79,113,197,84,212,120,250,208,138,86,92,56,
46,133,1,149,30,190,243,252,33,156,45,53,194,251,39,151,41,173,160,9,35,84,30,104,72,191,10,188,177,108,41,90,36,48,35,226,251,6,164,116,133,224,157,49,15,244,67,231,183,146,108,77,141,245,28,74,105,133,82,90,128,98,8,99,199,142,253,149,212,82,153,158,
169,7,48,4,138,186,106,141,138,38,55,120,95,67,168,30,108,84,159,167,234,235,17,69,240,108,185,152,13,200,196,82,108,58,124,195,184,206,32,46,241,98,81,129,39,227,69,69,38,38,251,167,59,138,153,194,154,212,138,94,46,153,97,222,166,44,118,53,4,184,113,
70,57,86,239,203,227,201,213,173,248,7,233,26,14,239,227,43,51,165,25,201,67,159,240,0,150,189,190,52,170,193,239,12,97,59,139,5,14,175,166,18,135,17,152,142,181,140,64,166,199,18,189,148,102,32,15,67,30,211,32,25,224,94,218,119,164,186,132,238,41,9,
51,42,153,219,50,112,211,46,78,153,192,1,149,60,242,176,84,254,158,9,39,126,239,132,233,194,24,205,11,196,149,194,214,101,212,93,67,113,229,167,78,56,113,179,197,21,192,46,134,160,89,3,215,76,47,195,232,126,62,26,91,132,234,84,186,119,105,11,222,63,41,
141,241,114,223,111,165,119,176,231,80,128,47,73,38,160,10,99,106,12,25,214,55,129,218,229,143,96,95,109,67,228,250,117,69,250,143,198,60,20,171,116,75,96,58,150,250,0,136,25,138,1,163,61,150,74,204,164,237,127,88,50,234,158,30,173,9,180,237,92,214,70,
147,90,157,52,196,199,140,225,190,36,4,211,101,97,162,8,241,59,65,28,91,75,104,235,9,45,209,93,176,24,121,12,110,64,40,234,24,230,81,37,18,129,76,26,235,51,172,183,143,179,70,37,208,183,156,227,195,103,150,99,220,64,31,127,92,153,197,246,186,0,23,142,
79,161,94,50,197,93,175,53,227,2,105,22,46,58,37,141,28,101,253,248,46,188,254,202,19,234,156,132,216,93,130,117,85,250,59,99,30,58,242,2,236,102,205,3,49,163,237,252,177,199,200,247,194,73,147,38,221,213,25,233,239,214,116,112,12,188,180,141,39,87,236,
172,145,62,106,154,141,125,55,197,28,22,164,217,30,0,202,1,144,107,40,172,123,103,155,70,242,218,157,243,19,78,21,169,45,230,48,229,101,186,6,192,96,4,251,31,215,247,208,156,17,248,226,185,105,76,148,68,159,62,36,1,46,247,93,59,173,10,139,182,182,225,
169,85,25,204,158,85,134,23,55,100,177,92,2,195,231,215,101,240,229,243,43,177,181,161,13,109,171,158,193,250,77,219,34,201,43,21,246,45,70,245,71,170,8,42,117,172,187,223,18,221,106,52,27,8,42,126,181,1,31,122,165,26,1,138,0,142,30,61,122,174,116,43,
95,167,64,82,143,105,0,235,194,193,196,241,173,31,62,160,23,71,191,94,78,109,61,139,59,116,237,241,81,153,151,129,242,81,229,47,180,87,97,27,65,56,143,187,120,184,169,3,136,152,202,101,64,202,163,75,70,57,127,108,10,147,6,249,248,222,75,205,184,103,89,
43,238,127,51,131,239,191,220,136,105,67,19,106,92,13,77,24,185,102,106,90,93,248,55,75,90,145,151,143,230,182,179,115,88,244,220,131,42,112,69,210,111,193,95,103,84,254,209,154,7,85,225,84,132,236,59,138,11,216,92,131,173,32,154,58,117,234,157,150,73,
221,90,130,19,238,6,90,20,239,249,60,74,219,54,102,5,122,167,89,97,98,135,21,206,13,84,12,228,105,98,219,190,1,238,179,120,76,140,241,247,57,115,43,135,68,20,25,228,166,98,215,154,22,106,68,185,112,108,2,103,74,237,179,116,71,30,141,109,148,139,224,72,
39,57,234,165,86,120,224,141,22,92,53,37,133,38,121,111,151,72,51,208,175,194,67,131,252,253,206,197,1,246,174,154,139,149,43,150,41,41,179,174,95,41,162,117,197,246,119,134,65,138,137,94,10,244,217,125,54,11,72,219,192,129,3,55,15,31,62,252,73,171,254,
59,99,2,186,205,11,224,214,167,231,113,254,158,122,2,42,146,204,212,226,197,45,90,22,55,216,72,94,224,116,237,68,101,120,220,121,160,34,110,2,133,163,146,221,33,147,244,7,149,25,244,175,224,248,216,172,52,54,213,4,248,243,218,44,46,59,37,137,143,206,
72,171,49,113,205,25,154,94,234,169,74,229,159,47,108,86,73,172,179,37,70,160,176,112,125,115,22,119,253,230,110,228,164,59,96,115,254,71,146,234,174,16,189,35,112,88,74,3,180,183,143,84,61,49,1,189,78,159,62,253,215,82,75,181,30,169,50,233,4,96,128,
56,80,99,103,254,210,11,205,92,164,25,65,30,215,42,219,43,74,224,48,167,118,46,234,197,51,109,219,194,57,159,26,63,199,109,205,64,220,88,26,37,111,77,167,79,78,30,119,211,57,105,149,134,62,67,130,207,113,253,61,188,176,177,13,67,164,39,242,145,211,203,
176,122,79,14,127,53,33,137,21,210,238,255,105,101,70,205,23,36,6,120,126,51,67,159,198,85,120,104,238,11,234,65,82,188,190,35,187,222,85,76,208,158,237,47,54,1,197,233,95,215,246,219,191,41,226,71,12,80,93,93,221,120,202,41,167,220,77,191,219,132,81,
143,153,0,11,242,108,107,149,106,239,146,87,162,190,128,132,105,9,211,21,91,113,143,159,173,4,98,204,121,63,202,234,197,211,195,84,178,136,10,76,60,237,106,18,96,140,122,248,28,70,162,156,255,196,65,30,46,155,152,196,27,59,243,104,144,46,223,148,193,
62,190,124,110,25,166,201,215,71,222,202,96,214,200,4,38,203,223,151,201,247,169,191,255,161,229,173,18,32,38,49,91,122,8,155,23,60,130,61,53,245,42,42,231,130,191,142,84,127,71,230,161,51,154,194,22,173,180,23,242,45,54,1,4,250,104,35,0,56,109,218,180,
71,165,166,218,229,50,101,143,121,1,182,15,47,52,225,216,48,64,60,141,27,49,208,11,133,112,202,183,53,5,195,144,69,133,32,206,12,9,43,254,17,184,39,230,9,37,99,249,166,103,32,12,227,66,17,107,66,62,245,222,50,69,120,170,63,40,147,39,218,38,221,189,65,
149,30,46,26,151,196,251,164,164,211,72,218,111,60,213,164,146,66,253,164,73,216,90,23,74,13,17,226,202,161,7,241,141,103,31,141,92,63,27,155,111,79,146,143,69,3,20,239,179,26,128,164,187,56,205,235,166,130,233,213,34,127,201,164,226,244,211,79,255,95,
98,12,123,76,41,237,114,194,24,192,86,33,233,178,42,17,119,235,114,13,202,236,192,168,184,239,62,182,219,65,32,162,182,43,29,22,54,128,49,140,107,194,172,123,201,5,115,240,131,197,4,84,119,16,226,250,211,83,82,157,251,248,234,211,205,24,221,135,227,125,
163,53,193,9,136,110,172,13,48,83,154,132,181,251,3,172,63,24,96,71,125,136,94,9,61,123,248,143,107,4,182,207,253,147,42,204,36,63,219,5,127,71,82,253,199,106,30,236,126,203,0,197,185,127,187,143,54,139,254,201,246,207,156,57,115,126,255,254,253,23,219,
193,18,150,65,58,83,34,222,45,38,32,116,134,55,232,121,65,186,244,75,245,7,82,251,86,188,66,67,225,168,55,81,148,249,19,5,109,0,170,180,60,246,50,226,54,240,168,78,208,212,27,14,235,237,225,211,103,151,225,233,53,109,146,248,30,154,37,211,221,187,60,
139,93,135,66,156,58,208,195,248,126,92,21,166,252,199,139,205,216,211,32,208,43,105,84,141,4,20,85,188,25,143,254,225,190,72,250,75,197,253,187,162,230,143,198,20,116,166,30,192,74,63,197,39,206,62,251,236,59,173,198,136,123,41,89,167,204,64,247,84,
5,27,183,142,179,216,27,32,176,118,176,85,215,223,37,28,23,208,237,245,0,67,97,53,177,208,72,62,254,14,142,185,8,69,220,51,232,184,136,164,97,62,118,102,74,205,27,254,222,139,173,10,11,156,49,44,33,53,128,143,249,91,114,248,245,210,12,170,210,28,247,
190,158,193,150,218,80,197,0,236,72,153,84,170,28,125,235,151,98,249,210,133,138,240,46,248,235,136,168,199,122,76,113,86,176,189,210,47,155,36,34,38,32,6,160,184,127,239,222,189,183,213,212,212,60,97,167,134,184,254,127,143,186,129,182,6,95,152,137,
27,190,188,153,61,13,161,154,26,94,150,112,48,191,155,235,133,169,238,53,146,76,35,87,46,28,147,64,166,77,219,245,168,59,72,216,170,94,17,23,144,82,7,141,36,226,120,137,244,47,59,37,129,31,188,218,138,81,125,57,134,84,115,252,96,78,171,10,69,95,48,54,
129,11,228,249,222,86,137,159,172,106,237,178,207,72,122,206,24,63,128,99,253,171,15,160,161,185,77,17,223,150,119,31,139,116,119,85,3,184,24,160,148,244,211,126,43,253,4,78,37,19,252,118,253,250,245,205,214,77,45,46,21,235,17,6,96,110,144,198,218,66,
41,101,228,6,122,214,190,179,184,128,212,246,240,217,128,16,217,125,90,99,128,212,245,148,33,92,125,78,168,9,99,76,77,15,167,191,169,170,152,50,118,173,121,161,164,156,90,207,137,175,190,114,81,25,94,221,156,199,146,29,1,110,154,153,194,31,87,182,225,
114,233,251,239,107,18,184,123,89,22,147,165,103,112,175,124,109,201,49,154,71,169,52,134,58,63,79,98,120,126,3,230,189,240,164,186,15,59,157,163,35,130,29,43,225,139,53,128,181,217,237,85,249,88,166,32,215,143,24,96,224,192,129,205,203,151,47,191,219,
13,27,119,213,4,248,221,197,0,156,23,78,242,32,16,71,82,74,133,161,129,157,224,229,128,69,34,38,229,232,147,166,163,152,30,197,144,74,29,162,29,80,197,208,175,156,163,82,170,107,210,32,36,233,13,205,161,154,233,63,176,66,75,62,69,242,8,236,141,237,235,
225,159,255,220,130,15,77,77,98,123,125,168,34,136,19,250,115,252,114,73,22,95,189,176,12,139,183,231,21,248,59,117,144,94,175,32,237,67,173,61,80,81,89,134,3,115,30,197,182,221,7,148,244,219,122,255,206,2,185,163,241,12,74,29,99,53,64,49,250,183,199,
218,130,82,51,33,236,79,91,229,15,73,127,113,66,169,24,184,246,64,32,40,174,6,178,95,140,10,65,124,91,167,207,99,183,141,8,63,115,184,135,75,39,36,20,70,32,208,70,29,68,163,165,10,175,78,51,220,126,85,185,10,222,168,26,66,146,124,105,255,179,146,145,
104,225,136,134,140,46,50,37,66,158,34,213,255,47,22,103,212,216,247,51,134,122,248,239,133,89,124,230,236,20,158,223,144,83,190,255,208,42,142,13,7,242,184,247,134,74,84,36,52,106,84,253,138,33,181,108,53,225,242,219,126,175,238,135,154,43,156,245,16,
143,11,35,116,180,207,149,90,203,0,197,184,192,30,67,51,7,9,249,143,25,51,70,108,218,180,233,87,170,241,198,36,132,218,235,29,56,225,12,144,167,121,252,122,49,46,169,150,53,208,243,61,90,229,67,151,134,83,48,40,16,206,236,32,249,178,235,144,192,11,27,
242,56,216,34,212,72,217,205,18,160,253,227,249,105,172,222,23,224,229,141,121,84,151,49,233,211,135,202,4,132,166,161,132,92,186,218,22,205,237,159,57,43,45,205,0,240,192,242,54,252,203,37,101,120,86,18,125,186,100,2,82,239,123,228,185,127,120,101,25,
126,58,191,85,229,3,78,27,146,151,159,13,21,3,209,251,213,149,189,48,102,231,147,88,179,102,181,178,171,164,1,58,202,234,117,133,168,93,101,160,226,206,96,87,3,16,6,32,6,160,247,251,244,233,179,116,206,156,57,11,172,83,228,2,191,158,215,0,198,119,167,
144,111,188,46,131,14,245,53,182,105,137,181,141,27,249,188,246,251,15,52,3,187,27,52,231,19,131,156,62,196,195,240,222,82,253,231,57,182,213,113,37,249,253,202,61,51,47,0,42,168,51,176,156,225,189,35,124,244,149,175,31,57,45,137,219,158,106,193,37,19,
146,138,49,54,73,95,255,31,207,47,195,207,23,101,113,243,44,90,137,36,148,184,2,56,119,84,66,217,151,234,50,221,209,147,149,64,96,100,159,60,30,253,249,221,10,40,246,233,87,165,60,0,91,137,211,89,223,255,120,152,7,55,14,80,172,1,136,192,68,124,178,253,
131,7,15,70,109,109,237,239,234,235,235,219,52,254,213,189,240,197,154,164,51,205,161,221,19,8,242,77,48,200,153,222,17,154,128,207,214,131,66,1,56,238,140,145,167,136,160,42,34,77,198,235,4,82,166,110,123,157,192,168,62,12,219,164,45,167,132,34,73,43,
49,15,125,62,43,127,47,35,115,33,25,234,86,169,41,30,149,96,111,243,193,16,183,158,151,196,255,46,206,226,198,51,210,152,187,53,143,81,189,57,102,141,240,240,213,167,91,84,68,48,36,111,33,167,11,84,200,148,248,233,50,240,221,11,177,120,254,156,168,76,
187,84,206,255,88,84,125,103,181,132,27,227,119,143,183,230,136,24,128,180,192,144,33,67,246,204,159,63,159,26,20,168,58,34,40,198,9,93,73,7,119,107,50,8,44,142,210,217,33,93,68,164,48,100,78,198,80,55,117,70,225,95,19,246,221,44,25,133,180,5,161,245,
141,210,28,164,169,178,135,219,37,95,52,126,216,47,223,63,123,84,82,45,38,117,167,148,244,155,102,36,177,74,154,12,114,31,251,73,173,240,208,91,1,254,235,3,101,248,149,4,128,107,15,4,170,39,64,71,211,244,181,136,9,38,14,101,152,243,196,221,18,84,102,
84,63,29,153,128,174,132,125,143,87,132,208,61,166,216,4,168,248,134,180,251,180,81,123,89,46,151,123,124,231,206,157,84,239,31,53,180,187,94,68,103,165,191,91,221,64,237,211,199,191,211,154,142,231,143,241,165,100,38,116,14,128,199,209,31,110,151,129,
113,42,132,105,237,159,225,210,135,111,203,235,223,147,9,22,133,135,213,146,114,242,248,41,131,56,62,62,51,169,192,30,45,69,67,32,112,161,68,249,31,154,146,192,227,171,219,112,237,212,132,52,45,2,207,172,201,73,208,199,85,0,42,149,208,203,200,16,134,
72,167,83,24,17,108,192,130,23,158,80,15,144,30,238,241,116,239,142,38,80,100,203,187,138,187,127,169,214,143,212,255,176,97,195,178,107,214,172,249,147,60,52,229,38,145,138,221,211,98,183,240,196,71,2,153,25,250,8,51,122,149,98,244,1,133,93,97,202,194,
227,194,145,124,96,87,3,225,241,80,41,201,219,116,44,1,53,10,40,153,230,110,165,73,136,120,196,0,55,191,39,133,13,148,231,151,4,254,180,252,253,149,45,121,140,145,110,32,1,77,98,164,75,198,39,112,151,52,7,106,234,8,119,70,202,153,156,194,160,234,36,246,
45,121,4,91,165,48,185,174,223,241,38,124,103,63,99,99,255,46,225,233,111,114,251,40,248,147,76,38,80,94,150,158,191,97,195,134,141,208,211,198,81,204,4,197,177,128,158,137,4,178,184,162,151,220,44,187,236,11,53,100,28,108,129,146,106,91,44,226,25,117,
76,33,95,242,231,219,114,58,96,68,129,29,2,126,244,62,173,249,71,1,159,86,229,250,65,185,126,148,204,25,39,137,253,227,5,89,124,120,90,18,52,184,123,213,190,80,73,255,179,235,243,248,212,153,105,60,177,170,13,107,247,228,245,92,2,187,120,72,168,187,131,
50,121,31,163,18,181,88,240,231,223,171,253,212,154,229,246,210,31,47,105,238,234,49,197,12,64,91,99,99,163,82,255,67,135,143,196,246,125,117,47,74,19,32,74,213,19,184,94,67,103,195,193,221,131,1,226,5,189,34,73,87,67,16,168,49,20,182,254,79,68,35,255,
170,42,24,6,87,112,244,46,179,96,81,199,11,122,151,105,181,127,213,169,190,10,31,43,66,73,160,88,43,207,243,209,211,19,120,74,74,126,67,6,248,224,228,4,254,115,110,6,215,73,149,191,106,111,32,77,1,87,159,125,124,77,94,74,141,142,36,38,140,186,209,14,
137,196,8,189,202,144,219,240,56,222,92,185,74,73,190,117,253,78,180,205,47,246,4,44,3,184,129,31,98,0,90,140,123,228,25,151,98,238,130,69,13,136,23,155,40,208,30,165,64,96,143,120,1,156,199,21,65,30,143,219,195,67,103,69,143,208,168,117,138,253,151,
73,98,125,224,84,79,217,122,10,0,29,146,18,190,91,250,238,20,221,59,77,186,131,99,250,112,165,29,40,70,64,57,251,143,74,28,177,191,73,224,222,55,218,240,79,23,164,164,207,159,71,223,50,142,83,7,112,220,181,164,13,223,186,56,141,213,251,3,124,238,236,
164,82,251,84,134,54,128,34,134,249,120,197,233,164,31,226,115,55,220,163,122,5,6,247,175,86,89,181,206,128,191,227,77,248,226,207,184,27,105,36,235,250,13,234,223,23,189,166,125,24,205,47,45,228,165,113,215,225,245,2,61,86,18,22,23,115,176,104,153,119,
85,17,36,209,250,188,173,65,212,59,168,204,66,82,155,133,159,44,200,233,41,226,208,37,95,25,73,139,83,6,114,233,1,228,113,223,138,156,178,251,77,210,235,29,223,143,225,178,9,62,254,103,81,27,206,148,102,128,208,254,130,109,121,252,203,133,41,60,188,42,
143,235,166,37,148,219,248,211,5,26,24,210,178,177,180,28,44,153,29,98,40,10,37,251,169,50,140,108,152,135,197,11,230,42,194,19,248,59,154,114,239,238,136,16,186,26,128,202,187,8,252,229,178,45,152,120,230,135,112,168,122,18,173,152,209,110,109,98,169,
10,163,30,203,5,184,171,120,171,102,80,6,69,232,214,92,168,212,186,181,253,54,191,239,155,150,48,213,82,150,16,72,75,6,88,119,32,196,106,105,215,201,93,163,25,211,180,30,240,23,164,84,191,182,45,192,242,93,1,126,120,101,26,191,93,222,134,203,198,251,
42,34,72,231,25,33,61,135,239,190,156,149,174,33,195,150,131,97,84,159,96,163,135,20,73,28,51,64,96,253,227,247,160,161,169,85,181,102,147,235,231,174,187,211,217,26,255,238,208,18,110,237,31,1,63,53,247,167,34,141,161,103,125,20,155,90,57,60,214,190,
244,151,194,40,61,150,14,110,205,198,4,182,68,32,187,60,180,146,171,196,143,91,200,201,189,184,2,72,63,9,157,4,154,187,37,192,65,105,14,72,202,39,72,201,191,110,154,175,242,3,63,156,223,134,27,79,79,98,83,109,136,154,102,224,194,177,190,196,3,121,220,
112,90,2,143,173,202,97,171,36,60,49,11,229,21,104,163,60,1,73,62,5,130,82,169,36,134,181,173,197,146,87,158,82,15,141,192,95,87,64,92,119,164,131,219,99,50,106,242,204,180,74,233,159,254,30,100,6,191,15,249,108,166,228,218,163,46,232,115,193,95,15,102,
3,229,67,79,233,185,187,238,16,70,234,9,160,12,92,232,44,16,17,141,111,1,10,150,148,87,19,69,36,19,16,242,159,49,198,195,172,225,28,211,7,115,252,250,245,156,202,10,158,41,255,254,143,87,179,152,45,25,97,129,212,8,148,230,165,202,30,210,14,212,248,201,
220,233,240,230,106,20,249,27,88,157,192,129,133,143,96,203,142,189,168,146,170,159,74,190,219,171,210,61,30,53,126,93,61,134,164,159,108,63,73,62,165,125,83,82,56,38,92,112,3,118,181,149,195,103,153,118,74,240,60,85,187,96,23,140,42,88,69,164,199,10,
66,16,143,120,177,107,55,16,202,247,157,130,207,104,230,31,139,39,123,115,118,120,74,153,236,248,217,35,61,236,108,16,120,242,237,28,62,49,211,199,203,91,2,181,180,60,165,140,151,239,14,113,245,100,31,191,89,150,147,170,223,41,36,133,179,196,0,105,33,
150,192,48,190,15,139,159,127,72,253,77,182,63,30,31,215,185,218,253,163,61,174,171,26,128,108,63,185,126,227,38,76,132,63,225,253,104,205,180,154,73,40,37,164,216,247,21,241,105,163,223,105,115,51,132,61,18,9,244,61,119,217,87,237,19,14,148,174,158,
90,55,208,47,92,25,52,150,210,56,68,172,92,32,234,39,28,193,113,193,88,79,249,255,63,156,151,195,123,70,248,170,120,115,142,4,147,215,79,245,241,140,244,0,174,145,196,167,194,206,249,114,31,189,199,153,179,108,140,83,156,82,89,158,68,118,237,51,88,181,
250,109,85,236,105,193,223,145,202,181,78,164,121,176,179,129,201,245,163,35,166,94,116,61,246,243,193,82,85,7,49,39,183,99,2,136,1,72,19,88,109,96,235,4,122,132,1,108,92,159,71,75,1,80,124,30,202,37,163,127,158,23,151,120,57,229,125,17,43,16,38,163,
121,66,148,38,30,47,237,62,5,127,40,235,71,225,223,123,86,228,113,214,112,79,73,59,237,63,103,148,135,251,229,62,159,23,47,39,19,179,85,32,125,207,97,233,102,188,249,194,253,200,72,36,72,182,159,30,82,87,164,250,104,106,252,186,90,47,104,179,126,196,
4,67,7,246,69,223,153,215,163,190,41,27,47,178,209,142,9,80,89,84,163,5,44,241,233,245,72,177,128,110,234,13,100,145,20,11,51,47,144,154,56,40,68,155,42,208,10,44,30,202,4,68,19,185,84,246,80,254,71,213,61,164,238,155,164,251,184,190,38,196,127,94,145,
84,46,29,49,204,56,201,20,15,188,149,199,213,147,18,120,92,186,127,111,238,14,20,240,211,229,103,113,151,144,101,190,84,50,133,244,158,133,88,182,120,161,122,56,196,0,197,43,110,30,107,99,103,87,247,149,58,198,70,254,72,51,157,113,254,7,80,223,107,18,
88,144,61,66,220,133,71,140,96,77,0,109,244,61,123,132,1,192,10,123,253,108,0,168,138,166,132,122,14,161,13,226,183,141,159,204,9,215,210,239,59,27,66,41,241,18,220,201,207,77,27,76,141,27,66,217,252,111,95,156,196,14,249,222,184,126,28,213,41,221,50,
112,197,196,132,242,46,220,25,19,209,2,35,160,72,99,128,13,115,238,71,237,161,22,85,241,99,167,124,117,101,156,203,209,48,72,87,204,67,129,235,39,129,207,168,243,111,194,129,70,51,77,149,193,105,123,71,73,208,106,215,20,116,153,224,72,12,208,125,11,71,
242,216,213,163,209,176,94,160,123,2,252,226,134,15,27,58,54,253,131,180,38,32,237,151,90,15,87,158,234,161,66,106,233,87,36,224,163,200,223,149,19,125,233,199,11,149,82,254,27,105,255,169,164,187,78,250,255,195,171,24,158,93,31,168,168,35,55,101,99,
204,100,22,149,25,74,36,81,221,176,18,79,207,121,54,154,238,125,36,212,223,213,152,192,241,242,30,8,252,145,22,152,54,227,28,180,13,61,7,193,193,12,60,20,46,108,213,145,247,85,156,17,236,177,222,64,238,128,57,157,221,99,10,200,169,5,35,120,156,35,176,
139,69,123,158,221,199,36,1,25,134,74,162,78,149,210,79,196,220,80,43,240,200,234,0,63,95,146,83,209,192,237,245,58,49,68,227,126,41,91,120,207,27,121,21,8,242,226,117,30,226,78,35,169,97,250,148,115,236,93,242,48,182,239,220,167,10,62,138,75,190,142,
36,249,39,194,60,16,177,8,245,147,235,71,105,235,73,23,207,198,254,108,57,18,44,116,82,231,157,99,0,123,62,119,85,209,19,108,2,88,100,127,93,127,60,20,122,98,184,2,129,102,81,167,104,229,79,196,136,221,142,149,125,80,218,120,122,221,38,221,191,62,18,
0,190,185,87,224,110,73,236,203,79,241,48,84,154,130,151,54,6,248,214,139,57,117,140,154,63,204,11,207,163,34,107,60,129,126,109,59,177,236,229,199,84,53,16,169,127,171,26,59,51,190,237,68,154,7,178,253,20,254,29,55,126,28,82,147,174,66,75,107,166,160,
89,214,230,88,58,27,139,233,209,178,112,75,9,59,164,217,50,131,69,234,194,52,120,234,234,28,179,66,152,29,49,15,29,25,60,115,24,87,245,252,148,24,162,247,79,29,192,240,169,25,62,250,72,32,248,227,5,57,229,81,16,14,88,185,55,84,169,102,59,129,52,114,149,
228,249,43,202,61,180,174,124,26,235,215,173,87,160,136,198,178,211,67,38,16,72,89,64,91,110,213,209,56,247,227,105,30,218,11,11,211,61,17,3,208,207,149,215,124,4,153,202,161,64,163,1,0,118,65,204,35,106,222,35,175,90,114,130,74,194,156,11,168,21,216,
117,146,67,45,29,143,120,176,147,5,106,182,68,204,142,124,97,134,1,102,74,6,160,194,15,170,8,62,67,130,192,235,39,123,216,211,40,240,150,36,248,156,173,161,82,149,228,42,146,244,43,9,17,44,154,49,168,6,79,72,73,31,148,104,198,242,151,255,160,92,191,84,
90,55,85,18,19,208,56,118,138,2,18,30,176,243,122,221,53,121,219,35,118,41,102,56,82,223,255,145,246,91,215,143,76,64,223,62,213,184,113,246,77,248,229,214,0,241,98,102,113,17,75,71,132,63,26,70,232,158,116,176,233,254,161,236,107,96,87,245,52,222,64,
162,104,253,2,235,17,68,157,65,74,19,8,52,102,117,246,144,194,193,255,126,113,66,117,239,126,127,110,30,131,122,1,7,90,226,136,33,225,10,143,199,225,99,120,238,44,1,15,141,111,60,138,149,43,94,87,43,125,20,171,95,122,232,180,217,225,205,238,56,247,163,
209,10,71,59,20,202,14,119,164,247,207,189,232,10,76,158,118,42,252,45,45,138,153,225,199,106,159,185,81,173,118,24,161,163,194,211,19,151,14,142,252,211,162,12,161,49,1,190,9,105,6,65,140,212,99,187,77,49,123,134,97,210,198,143,172,214,39,216,120,80,
96,233,206,0,187,26,67,41,253,26,75,164,205,234,35,142,27,17,205,250,215,113,4,142,1,233,60,54,45,122,12,117,13,77,72,166,203,218,245,159,41,232,66,139,54,213,214,214,42,144,72,90,225,68,153,7,186,6,169,126,114,255,42,210,62,198,93,120,19,26,36,243,39,
184,147,81,141,135,169,149,140,4,186,61,5,197,141,38,61,162,1,152,177,253,122,122,151,30,214,104,129,93,50,90,188,201,230,1,116,13,0,156,185,127,116,220,204,161,76,117,248,44,216,30,226,87,203,2,85,36,66,57,1,238,182,157,133,133,88,67,251,124,102,50,
156,159,64,197,193,215,177,254,205,69,224,126,226,136,128,201,78,224,36,211,80,95,95,175,204,3,105,5,219,34,222,93,230,129,126,232,122,116,237,201,103,158,141,214,161,231,163,94,226,158,202,20,143,38,159,219,133,79,24,43,237,13,184,12,80,74,242,59,98,
132,110,211,0,209,58,63,209,80,7,61,37,60,90,242,181,56,114,232,233,79,82,252,255,146,177,12,31,153,238,225,215,146,240,127,94,175,83,187,21,9,235,89,152,47,25,162,112,89,56,7,125,210,91,85,41,96,207,139,15,97,231,158,125,240,18,233,46,33,231,98,243,
64,166,129,152,225,120,155,7,218,103,93,63,18,140,201,23,223,128,154,160,18,101,94,168,34,166,110,206,180,35,4,104,207,231,150,146,217,253,61,179,102,144,113,249,168,194,202,119,66,191,161,169,197,183,35,100,162,153,62,110,225,136,42,2,101,120,99,119,
168,84,191,45,225,182,196,38,192,24,132,186,159,192,43,92,2,56,10,60,209,186,190,229,45,187,240,250,107,207,72,213,233,29,54,54,165,83,56,198,49,15,251,246,237,83,230,129,92,72,98,4,74,36,117,214,60,116,196,12,116,14,178,253,116,141,9,99,71,163,98,234,
95,99,127,107,155,20,2,63,90,230,94,8,7,1,116,48,95,184,148,9,232,49,47,128,174,159,78,234,178,46,22,207,121,86,146,153,244,96,34,118,122,13,62,33,226,53,125,212,36,80,249,220,151,237,18,88,181,95,127,153,164,23,191,15,196,99,229,34,73,11,163,53,98,34,
96,168,178,141,7,182,98,223,238,157,82,2,252,35,218,235,206,152,7,34,182,53,15,196,0,132,19,58,99,30,58,194,3,145,235,39,247,79,59,255,26,212,167,71,74,238,111,150,130,226,43,176,28,173,146,238,226,168,78,106,128,246,204,193,9,211,0,34,90,216,65,152,
229,227,98,2,169,210,236,104,76,140,48,203,187,26,208,200,116,243,70,24,198,147,195,67,179,192,148,85,247,133,90,205,204,29,52,46,164,213,2,34,159,65,174,196,188,220,174,44,232,108,147,69,54,162,102,143,39,149,173,138,53,140,121,160,205,14,145,238,172,
121,32,233,183,57,255,129,253,42,49,232,172,143,98,71,107,104,39,219,74,179,199,17,173,166,44,58,54,1,246,62,221,49,49,157,101,248,238,171,9,52,4,165,24,128,142,102,233,254,63,234,233,203,230,116,125,159,111,214,6,212,31,18,81,32,40,10,231,194,229,254,
104,1,130,130,53,132,237,210,241,204,174,248,197,204,114,50,229,253,245,250,123,205,117,237,38,68,58,122,72,4,202,72,194,105,173,94,146,84,146,124,194,4,110,185,181,107,30,232,88,50,15,116,77,183,193,163,189,235,90,215,143,174,51,101,214,197,200,244,
63,67,2,143,76,20,49,77,219,1,154,44,158,153,200,186,128,1,220,54,177,19,31,10,70,76,43,238,204,251,201,25,251,77,18,172,87,17,119,241,141,118,1,184,237,40,118,24,159,51,119,80,148,93,108,42,126,175,208,27,212,217,160,176,239,4,140,153,56,13,249,92,91,
167,212,188,43,53,244,32,201,222,79,153,50,69,5,137,236,42,94,118,46,159,11,226,172,121,32,6,217,182,109,27,182,110,221,170,76,5,17,182,189,144,51,237,35,13,66,174,95,47,137,246,198,94,48,27,245,185,4,173,79,162,190,51,53,194,132,174,202,23,49,80,42,
69,79,59,57,164,212,235,17,177,78,119,17,159,179,194,197,190,149,251,38,116,85,46,227,113,107,88,60,24,26,209,90,131,156,199,235,254,57,117,34,113,243,104,36,241,206,36,49,163,13,132,26,16,30,160,145,149,99,202,21,159,70,223,202,180,146,224,174,216,124,
218,198,140,25,19,13,107,160,229,219,105,141,94,59,48,178,120,25,22,55,243,70,132,221,181,107,151,26,51,71,177,5,210,18,165,18,51,196,36,116,95,19,166,156,6,127,220,37,200,211,113,76,175,81,100,35,166,5,161,117,39,80,86,74,3,184,68,119,87,18,235,17,13,
192,29,180,226,46,249,170,236,187,167,129,160,82,131,96,69,223,212,153,20,202,28,34,23,5,147,224,6,154,204,206,104,137,25,115,190,64,62,197,252,164,107,113,229,39,190,130,114,63,68,214,89,163,239,72,241,116,178,231,20,8,82,193,25,169,210,55,110,220,136,
154,154,26,140,31,63,62,106,222,236,200,123,160,141,136,187,127,255,126,197,8,59,118,236,136,204,7,85,233,144,22,161,191,201,245,155,116,193,71,80,207,251,192,67,62,50,125,9,219,203,232,44,126,121,36,12,80,44,249,157,45,14,237,62,12,96,26,67,116,49,136,
110,12,33,155,79,161,91,59,52,66,8,68,171,129,195,44,5,19,161,94,225,158,203,174,29,40,74,170,65,59,76,90,8,235,76,106,164,121,40,195,80,125,217,55,241,145,190,195,177,232,225,59,176,113,195,90,53,90,38,145,76,117,88,40,161,6,71,72,34,217,169,219,164,
254,237,108,158,206,186,89,174,121,32,91,79,128,143,152,138,250,16,72,75,208,249,70,143,24,134,222,167,127,24,187,179,121,216,53,46,237,153,117,77,67,108,3,98,161,40,125,191,118,144,148,59,80,162,51,109,226,221,150,12,42,6,112,100,141,40,151,79,121,124,
50,3,176,90,212,13,24,89,98,154,101,100,66,103,53,81,187,10,152,25,61,29,77,19,205,25,15,67,229,30,130,120,173,98,117,14,249,64,106,154,57,210,103,252,29,46,158,250,65,156,190,226,9,172,121,249,247,120,123,197,98,52,171,110,219,100,201,53,0,72,122,73,
69,211,15,1,188,129,3,7,42,245,79,210,236,2,172,174,6,151,72,163,208,166,204,133,100,140,241,239,189,10,181,229,227,209,214,212,170,132,133,26,105,201,254,147,144,88,65,113,158,74,187,105,0,87,242,139,23,169,236,97,13,160,147,65,189,211,192,8,233,37,
81,34,103,108,95,134,153,67,117,235,55,103,58,52,204,245,10,173,202,246,209,23,39,41,165,176,111,117,90,191,71,251,41,60,236,153,115,214,103,153,170,19,172,107,1,70,247,134,234,3,216,121,72,160,95,153,252,76,74,187,133,53,45,58,116,76,15,177,78,114,221,
222,150,193,56,237,234,191,199,37,31,250,24,176,117,14,22,60,117,63,158,125,238,121,236,222,179,79,161,209,68,162,176,130,150,236,55,33,123,2,119,59,119,238,140,2,63,116,140,173,182,61,218,224,18,17,171,79,117,5,174,190,126,54,248,24,249,125,179,158,
154,92,74,197,50,116,191,253,203,153,26,114,49,184,146,169,57,198,54,120,214,158,29,112,109,126,87,102,4,118,163,6,208,85,189,118,113,40,234,202,153,212,31,152,54,16,24,86,9,220,52,93,175,30,110,251,3,168,65,211,141,233,83,180,143,36,161,182,213,44,247,
26,106,236,64,65,162,131,25,201,72,1,69,8,185,34,250,21,227,57,214,28,208,171,128,95,55,89,35,203,182,64,171,207,218,22,29,124,162,89,128,243,183,75,21,41,63,59,117,124,26,3,78,189,28,239,185,240,114,92,52,123,21,158,253,211,131,88,246,210,99,216,178,
113,173,74,25,39,146,233,200,134,219,24,189,85,167,182,254,190,84,178,165,43,204,144,147,234,127,242,121,23,97,252,233,103,203,207,169,126,105,148,249,186,35,154,24,151,74,221,166,13,242,212,51,156,179,53,175,150,217,225,172,99,55,208,53,1,174,134,234,
49,13,160,2,56,166,130,101,79,147,192,214,6,96,217,30,96,246,52,224,174,101,66,197,3,108,123,120,16,90,59,39,162,47,169,6,65,134,133,46,165,48,237,221,220,68,75,8,76,46,221,21,68,41,97,234,8,14,109,10,218,104,14,235,145,216,161,18,43,246,230,213,117,
232,74,201,228,84,164,223,63,21,151,94,240,121,52,173,122,26,235,94,145,230,225,141,69,168,59,212,12,47,145,140,164,182,212,50,177,93,137,41,20,19,171,151,116,242,199,156,119,35,238,92,238,33,215,218,106,190,187,150,126,154,116,246,141,139,25,230,73,
194,47,216,26,42,77,233,243,142,243,1,238,84,81,119,60,92,71,177,136,19,130,1,108,98,134,190,28,5,54,8,221,82,0,136,120,222,170,181,164,175,135,74,133,65,92,61,4,211,200,73,239,137,130,44,168,19,236,49,218,131,152,135,206,75,233,211,182,128,41,243,225,
113,17,237,87,56,193,92,139,122,4,233,119,29,143,160,89,131,57,181,175,153,15,70,114,230,205,56,99,230,141,56,125,219,60,172,127,229,1,172,152,255,172,50,3,62,153,135,78,52,88,116,150,17,242,242,154,163,198,140,69,98,252,37,72,72,233,167,238,104,91,5,
109,61,38,98,88,234,161,244,61,59,207,136,21,38,134,58,96,128,226,22,241,30,209,0,238,53,163,85,187,13,160,83,245,0,81,37,80,156,16,242,60,91,14,102,230,255,211,205,187,57,112,131,17,60,238,156,84,30,115,213,68,142,42,41,37,143,175,13,21,193,175,159,
194,49,190,159,150,164,63,172,12,177,171,81,224,125,35,104,201,90,96,173,52,21,131,123,209,64,73,134,83,251,235,193,17,79,173,11,209,148,205,33,47,205,67,45,75,34,28,118,41,174,255,198,165,184,61,185,6,143,63,250,48,30,126,248,97,172,95,187,6,45,242,
96,210,4,157,5,128,237,153,7,30,6,152,116,209,223,160,181,124,8,88,182,205,228,55,140,7,99,52,33,21,184,86,166,76,50,40,140,195,128,237,217,118,23,4,22,15,138,236,49,19,80,252,234,25,247,46,48,37,78,228,18,134,54,108,235,100,242,108,125,136,103,10,75,
221,101,64,252,162,0,83,115,94,75,207,192,10,137,11,42,24,110,158,193,84,218,120,206,86,77,244,201,3,24,182,214,67,190,114,140,168,22,18,131,48,188,111,36,83,85,198,125,203,244,74,230,143,173,118,25,42,64,166,37,192,184,106,31,3,135,78,130,127,233,55,
113,225,228,207,226,156,117,127,198,214,185,15,96,241,162,5,168,107,144,230,193,79,192,79,36,186,228,9,40,219,47,113,197,144,193,131,48,248,172,27,112,48,136,195,214,214,255,167,81,138,76,126,39,178,249,253,43,76,87,19,47,93,108,211,145,9,232,249,92,
0,92,23,38,214,10,161,67,104,59,61,36,78,18,137,8,15,184,171,126,193,241,133,73,75,216,42,98,83,45,133,225,85,18,92,14,98,170,99,104,189,36,236,253,111,5,216,125,8,106,56,196,32,137,164,251,148,9,76,232,167,91,197,159,94,47,240,31,115,67,85,92,242,153,
89,28,15,174,18,42,99,153,52,235,14,81,9,249,223,203,253,253,203,129,175,60,157,83,147,77,153,55,0,179,46,249,56,110,184,225,6,252,238,79,243,176,127,209,3,88,185,224,25,236,220,181,91,85,29,81,76,161,179,230,33,148,12,112,234,89,151,35,215,127,18,88,
115,174,160,192,67,56,9,31,26,135,147,246,99,215,175,32,139,216,9,16,232,78,11,233,25,19,128,40,182,83,72,112,179,49,55,146,23,125,9,179,20,156,39,14,51,33,182,121,212,77,246,216,26,64,82,251,187,27,129,175,191,20,42,48,56,93,50,195,23,222,163,135,72,
255,105,93,160,212,252,67,171,133,234,37,216,92,71,45,101,12,183,94,196,241,218,78,129,45,117,194,20,94,232,248,196,229,210,163,152,40,205,195,247,230,6,242,111,34,2,169,211,28,254,106,132,135,71,222,246,80,59,228,98,12,189,233,98,140,189,114,45,106,
94,127,4,171,94,125,4,171,87,175,146,146,45,109,121,42,213,97,241,69,46,215,134,129,253,171,48,230,146,191,67,67,142,21,44,114,41,156,106,102,102,134,98,133,194,93,12,131,65,116,16,10,44,165,1,220,81,51,61,134,1,162,2,141,48,238,215,163,117,251,242,161,
51,34,222,9,1,155,244,87,180,82,184,13,239,186,76,100,83,164,158,167,61,137,29,13,68,84,40,149,127,138,148,244,247,14,99,88,40,137,251,139,229,161,196,1,76,37,88,246,54,106,237,114,222,40,134,191,145,24,225,165,45,33,158,89,111,151,176,211,61,139,83,
164,121,184,96,180,38,126,83,155,6,160,52,101,228,230,153,92,49,202,250,3,33,198,245,9,177,95,186,150,135,42,78,69,217,165,223,192,197,23,124,6,127,91,243,28,158,126,244,126,44,95,60,31,181,245,141,170,252,140,220,197,40,177,68,213,208,217,12,170,202,
124,124,224,19,255,140,220,240,115,33,90,219,34,83,23,229,53,194,56,165,29,56,191,187,130,192,57,235,20,3,184,217,200,30,139,4,70,18,46,16,125,41,2,128,36,177,145,4,195,233,36,62,172,247,205,73,19,115,253,121,238,48,151,125,24,244,133,105,246,207,181,
147,24,14,73,194,221,254,90,136,77,117,177,23,145,145,146,60,178,55,147,224,144,73,156,0,133,15,158,219,232,172,94,102,18,44,103,13,103,120,82,2,66,42,200,84,158,129,60,215,236,211,184,158,56,186,42,84,106,249,214,115,60,220,187,34,192,146,157,82,125,
83,208,138,245,199,45,55,222,136,212,233,31,193,232,197,11,176,247,181,7,176,74,154,135,61,187,119,161,45,175,23,198,73,75,183,103,210,180,169,56,235,186,47,65,156,246,183,168,207,4,202,75,137,38,169,137,56,43,163,108,126,160,155,103,120,39,74,193,220,
212,53,225,11,187,100,156,101,130,30,205,5,184,230,192,206,232,9,77,200,150,51,167,158,19,133,221,67,197,127,243,8,9,196,160,194,93,40,154,206,247,118,13,147,182,63,212,195,39,160,3,79,42,95,19,53,165,2,139,164,86,88,181,15,56,127,20,240,175,23,112,204,
221,38,240,170,100,6,170,189,36,47,98,231,33,58,38,142,195,127,96,2,199,100,169,21,126,48,47,40,232,105,188,118,138,167,226,13,180,8,38,3,85,244,248,216,219,226,161,110,232,5,24,114,195,5,24,253,254,13,56,180,105,1,90,14,108,147,232,222,67,213,80,169,
45,198,159,135,186,196,32,100,165,170,241,76,245,171,45,98,117,153,192,98,2,250,83,66,132,195,138,65,218,227,5,59,19,192,157,49,236,174,49,220,115,185,0,215,254,67,23,131,112,152,97,77,1,162,165,97,153,195,45,78,234,91,29,155,15,227,204,152,70,203,113,
66,136,36,117,76,111,10,52,209,144,73,166,98,12,76,119,82,40,47,195,125,104,244,186,98,175,192,234,3,90,218,201,20,140,148,158,193,2,201,8,45,210,71,189,116,28,151,15,61,196,48,233,127,79,29,164,53,192,127,47,10,149,121,32,134,202,72,130,252,113,77,40,
193,163,135,239,94,234,225,237,3,122,157,3,114,41,61,154,133,147,19,104,148,247,208,88,49,1,254,140,9,232,101,152,81,58,21,82,51,73,66,103,218,162,26,198,120,142,98,188,131,57,46,175,180,22,42,188,109,171,130,25,43,157,13,181,63,35,71,142,196,196,137,
19,143,138,78,221,194,0,170,228,203,72,123,104,107,254,88,97,116,47,26,7,227,160,96,215,37,180,238,158,199,75,152,6,85,39,47,212,131,26,89,173,25,96,66,95,45,185,251,154,89,36,173,194,204,36,166,124,1,117,25,83,110,128,84,255,210,221,4,8,41,116,204,48,
75,98,134,71,223,22,120,69,226,130,75,199,49,108,151,152,226,153,13,66,245,33,230,140,135,64,249,138,64,190,206,149,26,163,65,114,49,225,5,106,79,27,85,205,240,198,30,33,175,105,170,155,84,65,67,14,212,206,31,56,120,200,51,75,230,21,214,51,148,10,155,
233,251,37,48,75,195,52,108,189,132,93,37,13,96,199,157,86,221,214,25,100,19,65,214,206,19,35,208,172,190,134,108,97,145,135,219,20,98,61,7,70,32,200,59,172,84,160,224,23,50,13,4,40,95,222,170,37,241,96,107,28,242,141,207,207,20,202,127,114,157,246,28,
146,230,219,150,169,101,98,128,95,44,211,101,104,20,123,127,97,139,192,203,91,132,46,88,49,73,42,21,137,148,15,62,19,232,110,37,250,155,24,131,136,110,131,90,246,251,250,110,58,154,29,110,18,131,32,6,183,5,90,146,69,147,117,149,70,160,115,145,250,7,43,
44,2,101,172,123,52,117,183,185,129,197,21,172,150,17,90,109,61,64,145,183,0,81,88,12,226,82,95,20,121,24,246,92,100,2,158,219,100,75,206,237,66,20,142,251,104,164,239,96,134,25,130,197,129,23,139,69,34,59,106,254,240,141,27,42,10,43,177,212,55,224,38,
156,172,203,181,29,204,130,248,59,69,125,16,118,165,84,203,224,60,174,142,17,37,66,58,110,249,27,181,197,213,181,22,245,79,176,238,97,130,110,5,129,66,196,160,38,107,212,41,169,224,166,92,81,169,24,43,196,0,118,102,16,197,247,125,103,220,139,13,18,209,
62,215,156,248,102,192,164,125,159,152,129,222,242,162,200,163,126,37,41,228,44,198,9,188,104,70,81,32,28,55,211,20,169,194,152,46,59,243,32,52,173,107,244,86,202,215,141,44,150,105,173,167,66,117,9,9,47,94,229,148,198,211,217,65,149,22,15,209,117,232,
188,164,189,172,230,18,76,167,194,109,23,85,70,186,177,101,94,97,158,224,29,193,0,174,118,167,47,78,238,23,165,131,7,148,235,215,47,206,138,211,187,54,145,99,137,217,22,232,34,15,122,0,182,102,192,22,123,180,25,38,162,185,193,117,38,85,188,166,150,108,
58,208,191,12,248,212,25,250,156,205,242,193,205,217,6,44,151,170,250,140,193,58,78,208,39,197,36,176,210,215,77,152,220,106,20,1,12,245,57,91,115,49,65,245,144,73,253,126,75,62,46,58,161,193,148,196,40,91,235,5,22,237,0,62,56,145,225,189,195,245,36,
84,107,222,214,213,8,60,185,54,84,223,233,140,161,18,103,12,229,42,48,149,54,107,21,216,185,201,164,13,235,91,117,107,28,1,77,250,62,52,252,122,88,149,78,106,81,138,123,201,206,208,48,5,235,14,250,119,19,8,20,174,173,214,95,114,109,141,220,47,9,81,151,
209,221,189,4,116,8,19,216,37,96,114,97,92,253,75,15,154,30,14,61,236,172,169,43,32,111,198,54,76,54,153,66,95,229,175,231,180,244,16,225,22,239,210,175,116,141,154,22,253,208,8,173,239,107,22,210,91,16,202,29,204,5,58,107,152,55,177,234,140,209,70,9,
103,90,41,61,240,253,205,194,4,100,244,84,147,164,7,179,200,148,14,32,181,153,84,243,166,131,148,232,162,85,206,244,181,213,68,116,115,191,20,205,93,189,95,40,162,142,237,163,223,167,40,159,77,127,211,121,233,186,196,236,20,255,167,207,17,246,152,58,
72,107,51,154,141,96,65,52,227,246,249,176,147,159,1,184,227,234,168,48,168,208,225,90,42,240,32,105,154,191,93,171,109,55,95,192,156,9,161,81,176,135,21,206,29,182,118,214,186,83,68,204,132,153,59,76,190,250,139,91,226,182,51,91,61,68,207,122,111,19,
83,126,126,88,208,66,117,184,107,165,102,11,56,245,3,182,44,141,57,73,43,139,55,96,152,102,109,173,192,170,3,113,87,50,51,86,222,247,98,143,70,69,18,107,69,145,217,23,209,125,196,196,213,204,215,106,86,86,123,107,175,80,9,43,23,19,57,240,228,164,99,0,
129,66,76,23,215,5,26,196,76,28,254,230,94,173,218,209,14,208,139,218,161,75,164,149,61,94,240,32,76,117,113,92,244,65,110,94,113,65,169,77,33,123,60,118,33,97,186,149,14,243,46,139,122,176,237,66,87,238,250,7,202,20,56,190,107,210,84,57,139,2,92,23,
3,189,40,5,110,202,223,195,176,48,201,37,156,142,39,133,123,164,150,218,113,72,227,15,26,171,43,92,180,92,8,172,197,113,19,214,227,204,4,194,29,213,198,28,223,94,152,248,64,65,24,83,20,122,9,174,100,218,232,161,219,39,239,158,151,59,175,110,1,169,112,
104,237,243,88,115,68,253,5,208,211,68,93,36,111,179,140,140,89,2,178,130,101,141,89,209,53,133,136,67,119,12,177,43,87,224,173,58,73,156,40,184,197,11,219,188,237,208,7,230,132,63,221,72,169,189,130,147,113,20,39,163,9,112,11,119,130,68,186,50,149,146,
192,47,231,21,114,61,99,69,18,78,251,72,141,251,113,8,150,54,237,127,235,155,179,174,97,201,142,24,211,72,226,123,165,19,82,246,122,182,226,166,216,229,10,109,99,9,143,239,201,74,123,232,192,110,55,33,229,106,160,194,115,21,206,37,118,93,214,195,92,94,
86,186,120,198,213,34,162,196,119,161,198,23,51,235,32,56,25,77,64,104,110,44,191,254,149,123,95,216,183,118,244,142,124,46,160,174,126,95,34,120,5,98,109,81,8,67,137,218,126,86,172,70,11,19,69,16,69,68,116,8,86,42,220,45,68,161,59,90,236,67,187,9,40,
33,14,55,73,165,43,156,196,97,166,45,54,31,174,6,19,37,69,68,160,253,62,79,33,10,3,93,154,121,84,123,184,226,115,206,89,27,243,18,173,77,7,182,237,181,202,243,120,104,131,227,169,1,108,181,127,110,249,131,223,189,87,190,210,52,70,90,135,173,204,184,228,
221,58,153,252,47,248,135,8,77,14,102,147,220,106,205,107,96,182,240,88,153,192,63,78,196,23,230,134,200,65,107,54,231,77,152,27,108,54,12,192,222,165,229,81,63,91,10,14,83,249,112,198,60,207,140,17,182,147,74,3,184,12,16,154,155,76,153,107,176,119,25,
224,152,76,107,104,158,109,198,97,132,60,10,179,234,39,5,6,200,25,102,200,155,155,180,243,59,223,37,254,177,107,129,188,17,178,156,217,2,28,135,152,192,241,246,2,130,34,109,224,74,254,187,76,112,244,207,21,142,189,63,46,182,255,184,48,128,237,73,43,193,
4,97,9,162,191,203,0,199,198,0,162,157,87,213,105,220,35,12,64,131,146,62,249,201,79,186,117,103,130,86,133,76,84,14,16,185,67,123,223,37,252,241,103,134,146,206,234,228,201,147,143,250,164,172,171,29,174,239,254,252,101,253,188,235,155,255,31,255,249,
255,2,12,0,235,154,52,248,249,240,115,28,0,0,0,0,73,69,78,68,174,66,96,130,0,0 };

const char* projectIconXcodeIOS_png = (const char*) temp_binary_data_31;

//================== RecentFilesMenuTemplate.nib ==================
static const unsigned char temp_binary_data_32[] =
{ 98,112,108,105,115,116,48,48,212,0,1,0,2,0,3,0,4,0,5,0,6,1,53,1,54,88,36,118,101,114,115,105,111,110,88,36,111,98,106,101,99,116,115,89,36,97,114,99,104,105,118,101,114,84,36,116,111,112,18,0,1,134,160,175,16,74,0,7,0,8,0,31,0,35,0,36,0,42,0,46,0,50,
0,53,0,57,0,74,0,77,0,78,0,86,0,87,0,97,0,112,0,113,0,114,0,119,0,120,0,121,0,124,0,128,0,129,0,132,0,143,0,144,0,145,0,149,0,153,0,162,0,163,0,164,0,169,0,173,0,180,0,181,0,182,0,185,0,192,0,193,0,200,0,201,0,208,0,209,0,216,0,217,0,224,0,225,0,226,
0,229,0,230,0,232,0,249,1,11,1,29,1,30,1,31,1,32,1,33,1,34,1,35,1,36,1,37,1,38,1,39,1,40,1,41,1,42,1,43,1,44,1,47,1,50,85,36,110,117,108,108,219,0,9,0,10,0,11,0,12,0,13,0,14,0,15,0,16,0,17,0,18,0,19,0,20,0,21,0,22,0,23,0,24,0,25,0,26,0,27,0,28,0,29,0,
29,95,16,16,78,83,86,105,115,105,98,108,101,87,105,110,100,111,119,115,93,78,83,79,98,106,101,99,116,115,75,101,121,115,86,78,83,82,111,111,116,92,78,83,79,105,100,115,86,97,108,117,101,115,86,36,99,108,97,115,115,90,78,83,79,105,100,115,75,101,121,115,
93,78,83,67,111,110,110,101,99,116,105,111,110,115,95,16,15,78,83,79,98,106,101,99,116,115,86,97,108,117,101,115,95,16,25,78,83,65,99,99,101,115,115,105,98,105,108,105,116,121,67,111,110,110,101,99,116,111,114,115,95,16,23,78,83,65,99,99,101,115,115,
105,98,105,108,105,116,121,79,105,100,115,75,101,121,115,95,16,25,78,83,65,99,99,101,115,115,105,98,105,108,105,116,121,79,105,100,115,86,97,108,117,101,115,128,5,128,9,128,2,128,55,128,73,128,54,128,7,128,53,128,71,128,72,128,72,210,0,13,0,32,0,33,0,
34,91,78,83,67,108,97,115,115,78,97,109,101,128,4,128,3,93,78,83,65,112,112,108,105,99,97,116,105,111,110,210,0,37,0,38,0,39,0,40,90,36,99,108,97,115,115,110,97,109,101,88,36,99,108,97,115,115,101,115,94,78,83,67,117,115,116,111,109,79,98,106,101,99,
116,162,0,39,0,41,88,78,83,79,98,106,101,99,116,210,0,13,0,43,0,44,0,45,90,78,83,46,111,98,106,101,99,116,115,128,6,160,210,0,37,0,38,0,47,0,48,92,78,83,77,117,116,97,98,108,101,83,101,116,163,0,47,0,49,0,41,85,78,83,83,101,116,210,0,13,0,43,0,51,0,52,
128,8,160,210,0,37,0,38,0,54,0,55,94,78,83,77,117,116,97,98,108,101,65,114,114,97,121,163,0,54,0,56,0,41,87,78,83,65,114,114,97,121,210,0,13,0,43,0,58,0,59,128,52,174,0,60,0,61,0,62,0,63,0,64,0,65,0,66,0,67,0,68,0,69,0,70,0,71,0,72,0,73,128,10,128,12,
128,45,128,15,128,39,128,25,128,28,128,30,128,33,128,35,128,43,128,41,128,47,128,50,210,0,13,0,32,0,33,0,76,128,4,128,11,93,78,83,65,112,112,108,105,99,97,116,105,111,110,212,0,79,0,13,0,80,0,81,0,82,0,83,0,84,0,85,91,78,83,77,101,110,117,73,116,101,
109,115,86,78,83,78,97,109,101,87,78,83,84,105,116,108,101,128,14,128,38,128,49,128,13,89,65,77,97,105,110,77,101,110,117,210,0,13,0,43,0,51,0,89,128,8,167,0,63,0,65,0,64,0,71,0,70,0,62,0,72,128,15,128,25,128,39,128,41,128,43,128,45,128,47,216,0,98,0,
99,0,100,0,13,0,101,0,102,0,103,0,81,0,104,0,61,0,106,0,107,0,108,0,109,0,110,0,111,95,16,17,78,83,75,101,121,69,113,117,105,118,77,111,100,77,97,115,107,86,78,83,77,101,110,117,89,78,83,79,110,73,109,97,103,101,90,78,83,75,101,121,69,113,117,105,118,
93,78,83,77,110,101,109,111,110,105,99,76,111,99,92,78,83,77,105,120,101,100,73,109,97,103,101,18,0,16,0,0,128,12,128,18,128,24,128,17,18,127,255,255,255,128,22,128,16,91,100,101,108,109,101,65,112,112,75,105,116,80,211,0,13,0,115,0,32,0,116,0,117,0,
118,94,78,83,82,101,115,111,117,114,99,101,78,97,109,101,128,21,128,20,128,19,87,78,83,73,109,97,103,101,95,16,15,78,83,77,101,110,117,67,104,101,99,107,109,97,114,107,210,0,37,0,38,0,122,0,123,95,16,16,78,83,67,117,115,116,111,109,82,101,115,111,117,
114,99,101,162,0,122,0,41,211,0,13,0,115,0,32,0,116,0,126,0,118,128,21,128,23,128,19,95,16,16,78,83,77,101,110,117,77,105,120,101,100,83,116,97,116,101,210,0,37,0,38,0,130,0,131,90,78,83,77,101,110,117,73,116,101,109,162,0,130,0,41,218,0,133,0,98,0,134,
0,99,0,100,0,13,0,101,0,102,0,103,0,81,0,135,0,104,0,66,0,61,0,106,0,107,0,108,0,109,0,110,0,142,88,78,83,65,99,116,105,111,110,89,78,83,83,117,98,109,101,110,117,128,27,128,28,128,12,128,18,128,24,128,17,128,22,128,26,84,70,105,108,101,94,115,117,98,
109,101,110,117,65,99,116,105,111,110,58,211,0,79,0,13,0,81,0,146,0,83,0,142,128,29,128,38,128,26,210,0,13,0,43,0,51,0,151,128,8,161,0,67,128,30,218,0,133,0,98,0,134,0,99,0,100,0,13,0,101,0,102,0,103,0,81,0,154,0,104,0,68,0,66,0,106,0,107,0,108,0,109,
0,110,0,161,128,32,128,33,128,28,128,18,128,24,128,17,128,22,128,31,91,79,112,101,110,32,82,101,99,101,110,116,94,115,117,98,109,101,110,117,65,99,116,105,111,110,58,212,0,79,0,13,0,80,0,81,0,165,0,83,0,167,0,161,128,34,128,38,128,37,128,31,210,0,13,
0,43,0,51,0,171,128,8,161,0,69,128,35,216,0,98,0,99,0,100,0,13,0,101,0,102,0,103,0,81,0,104,0,68,0,106,0,107,0,108,0,109,0,110,0,179,128,33,128,18,128,24,128,17,128,22,128,36,90,67,108,101,97,114,32,77,101,110,117,95,16,22,95,78,83,82,101,99,101,110,
116,68,111,99,117,109,101,110,116,115,77,101,110,117,210,0,37,0,38,0,183,0,184,86,78,83,77,101,110,117,162,0,183,0,41,216,0,98,0,99,0,100,0,13,0,101,0,102,0,103,0,81,0,104,0,61,0,106,0,107,0,108,0,109,0,110,0,191,128,12,128,18,128,24,128,17,128,22,128,
40,84,69,100,105,116,215,0,99,0,100,0,13,0,101,0,102,0,103,0,81,0,61,0,106,0,107,0,108,0,109,0,110,0,199,128,12,128,18,128,24,128,17,128,22,128,42,86,70,111,114,109,97,116,216,0,98,0,99,0,100,0,13,0,101,0,102,0,103,0,81,0,104,0,61,0,106,0,107,0,108,0,
109,0,110,0,207,128,12,128,18,128,24,128,17,128,22,128,44,84,86,105,101,119,216,0,98,0,99,0,100,0,13,0,101,0,102,0,103,0,81,0,104,0,61,0,106,0,107,0,108,0,109,0,110,0,215,128,12,128,18,128,24,128,17,128,22,128,46,86,87,105,110,100,111,119,215,0,99,0,
100,0,13,0,101,0,102,0,103,0,81,0,61,0,106,0,107,0,108,0,109,0,110,0,223,128,12,128,18,128,24,128,17,128,22,128,48,84,72,101,108,112,91,95,78,83,77,97,105,110,77,101,110,117,210,0,13,0,32,0,33,0,228,128,4,128,51,93,78,83,70,111,110,116,77,97,110,97,103,
101,114,210,0,37,0,38,0,56,0,231,162,0,56,0,41,210,0,13,0,43,0,58,0,234,128,52,174,0,22,0,22,0,61,0,61,0,61,0,61,0,65,0,66,0,67,0,68,0,61,0,61,0,61,0,22,128,2,128,2,128,12,128,12,128,12,128,12,128,25,128,28,128,30,128,33,128,12,128,12,128,12,128,2,210,
0,13,0,43,0,58,0,251,128,52,175,16,15,0,22,0,60,0,61,0,62,0,63,0,64,0,65,0,66,0,67,0,68,0,69,0,70,0,71,0,72,0,73,128,2,128,10,128,12,128,45,128,15,128,39,128,25,128,28,128,30,128,33,128,35,128,43,128,41,128,47,128,50,210,0,13,0,43,0,58,1,13,128,52,175,
16,15,1,14,1,15,1,16,1,17,1,18,1,19,1,20,1,21,1,22,1,23,1,24,1,25,1,26,1,27,1,28,128,56,128,57,128,58,128,59,128,60,128,61,128,62,128,63,128,64,128,65,128,66,128,67,128,68,128,69,128,70,17,2,22,17,2,23,17,2,24,17,2,25,17,2,26,17,2,27,17,2,28,17,2,29,
17,2,30,17,2,31,17,2,32,17,2,33,17,2,34,17,2,35,17,2,36,210,0,13,0,43,0,51,1,46,128,8,160,210,0,13,0,43,0,58,1,49,128,52,160,210,0,37,0,38,1,51,1,52,94,78,83,73,66,79,98,106,101,99,116,68,97,116,97,162,1,51,0,41,95,16,15,78,83,75,101,121,101,100,65,114,
99,104,105,118,101,114,209,1,55,1,56,93,73,66,46,111,98,106,101,99,116,100,97,116,97,128,1,0,8,0,25,0,34,0,43,0,53,0,58,0,63,0,214,0,220,1,9,1,28,1,42,1,49,1,62,1,69,1,80,1,94,1,112,1,140,1,166,1,194,1,196,1,198,1,200,1,202,1,204,1,206,1,208,1,210,1,
212,1,214,1,216,1,225,1,237,1,239,1,241,1,255,2,8,2,19,2,28,2,43,2,48,2,57,2,66,2,77,2,79,2,80,2,89,2,102,2,109,2,115,2,124,2,126,2,127,2,136,2,151,2,158,2,166,2,175,2,177,2,206,2,208,2,210,2,212,2,214,2,216,2,218,2,220,2,222,2,224,2,226,2,228,2,230,
2,232,2,234,2,243,2,245,2,247,3,5,3,22,3,34,3,41,3,49,3,51,3,53,3,55,3,57,3,67,3,76,3,78,3,93,3,95,3,97,3,99,3,101,3,103,3,105,3,107,3,140,3,160,3,167,3,177,3,188,3,202,3,215,3,220,3,222,3,224,3,226,3,228,3,233,3,235,3,237,3,249,3,250,4,7,4,22,4,24,4,
26,4,28,4,36,4,54,4,63,4,82,4,87,4,100,4,102,4,104,4,106,4,125,4,134,4,145,4,150,4,191,4,200,4,210,4,212,4,214,4,216,4,218,4,220,4,222,4,224,4,226,4,231,4,246,5,3,5,5,5,7,5,9,5,18,5,20,5,23,5,25,5,66,5,68,5,70,5,72,5,74,5,76,5,78,5,80,5,82,5,94,5,109,
5,126,5,128,5,130,5,132,5,134,5,143,5,145,5,148,5,150,5,183,5,185,5,187,5,189,5,191,5,193,5,195,5,206,5,231,5,240,5,247,5,252,6,29,6,31,6,33,6,35,6,37,6,39,6,41,6,46,6,75,6,77,6,79,6,81,6,83,6,85,6,87,6,94,6,127,6,129,6,131,6,133,6,135,6,137,6,139,6,
144,6,177,6,179,6,181,6,183,6,185,6,187,6,189,6,196,6,225,6,227,6,229,6,231,6,233,6,235,6,237,6,242,6,254,7,7,7,9,7,11,7,25,7,34,7,39,7,48,7,50,7,79,7,81,7,83,7,85,7,87,7,89,7,91,7,93,7,95,7,97,7,99,7,101,7,103,7,105,7,107,7,116,7,118,7,151,7,153,7,155,
7,157,7,159,7,161,7,163,7,165,7,167,7,169,7,171,7,173,7,175,7,177,7,179,7,181,7,190,7,192,7,225,7,227,7,229,7,231,7,233,7,235,7,237,7,239,7,241,7,243,7,245,7,247,7,249,7,251,7,253,7,255,8,2,8,5,8,8,8,11,8,14,8,17,8,20,8,23,8,26,8,29,8,32,8,35,8,38,8,
41,8,44,8,53,8,55,8,56,8,65,8,67,8,68,8,77,8,92,8,97,8,115,8,120,8,134,0,0,0,0,0,0,2,2,0,0,0,0,0,0,1,57,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8,136,0,0 };

const char* RecentFilesMenuTemplate_nib = (const char*) temp_binary_data_32;

//================== wizard_AnimatedApp.svg ==================
static const unsigned char temp_binary_data_33[] =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
"<!-- Generator: Adobe Illustrator 18.0.0, SVG Export Plug-In . SVG Version: 6.00 Build 0)  -->\r\n"
"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\r\n"
"<svg version=\"1.1\" id=\"Layer_1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\r\n"
"\t viewBox=\"0 0 136.9 114.8\" enable-background=\"new 0 0 136.9 114.8\" xml:space=\"preserve\">\r\n"
"<g id=\"Layer_1_18_\">\r\n"
"\t<rect x=\"16.3\" y=\"10.9\" fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" width=\"103.6\" height=\"67.5\"/>\r\n"
"</g>\r\n"
"<rect x=\"16.3\" y=\"10.9\" fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" width=\"103.6\" height=\"2.4\"/>\r\n"
"<circle fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" cx=\"80.3\" cy=\"45\" r=\"20.8\"/>\r\n"
"<g>\r\n"
"\t<g>\r\n"
"\t\t<path fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" d=\"M97.8,40.1c0.9,3.2,0,8-2.1,8.3\r\n"
"\t\t\tc-2,0.2-10.5-2.2-14.8-3.6c2.9-3.4,8.9-10,10.8-10.8C93.5,33.2,96.9,36.9,97.8,40.1z\"/>\r\n"
"\t</g>\r\n"
"\t<g>\r\n"
"\t\t<path fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" d=\"M93.3,57.7c-2.3,2.4-7,4-8.2,2.4\r\n"
"\t\t\tc-1.2-1.6-3.4-10.2-4.3-14.6c4.4,0.8,13.1,2.8,14.7,4C97.2,50.5,95.6,55.3,93.3,57.7z\"/>\r\n"
"\t</g>\r\n"
"\t<g>\r\n"
"\t\t<path fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" d=\"M75.7,62.6c-3.2-0.8-7-4.1-6.1-6\r\n"
"\t\t\tc0.8-1.9,7.2-8,10.5-11.1c1.5,4.3,4.1,12.7,3.9,14.8C83.9,62.4,79,63.4,75.7,62.6z\"/>\r\n"
"\t</g>\r\n"
"\t<g>\r\n"
"\t\t<path fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" d=\"M62.9,49.9c-0.9-3.2,0-8,2.1-8.3\r\n"
"\t\t\tc2-0.2,10.5,2.2,14.8,3.6c-2.9,3.4-8.9,10-10.8,10.8C67.2,56.8,63.8,53.1,62.9,49.9z\"/>\r\n"
"\t</g>\r\n"
"\t<g>\r\n"
"\t\t<path fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" d=\"M67.4,32.3c2.3-2.4,7-4,8.2-2.4\r\n"
"\t\t\tc1.2,1.6,3.4,10.2,4.3,14.6c-4.4-0.8-13.1-2.8-14.7-4C63.5,39.4,65.1,34.7,67.4,32.3z\"/>\r\n"
"\t</g>\r\n"
"\t<g>\r\n"
"\t\t<path fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" d=\"M84.8,27.4c3.2,0.8,7,4.1,6.1,6\r\n"
"\t\t\tc-0.8,1.9-7.2,8-10.5,11.1c-1.5-4.3-4.1-12.7-3.9-14.8C76.7,27.6,81.6,26.6,84.8,27.4z\"/>\r\n"
"\t</g>\r\n"
"</g>\r\n"
"<path fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" d=\"M130,114.8H6.9c-3.8,0-6.9-3.1-6.9-6.9V6.9\r\n"
"\tC0,3.1,3.1,0,6.9,0H130c3.8,0,6.9,3.1,6.9,6.9v101.1C136.9,111.7,133.9,114.8,130,114.8z\"/>\r\n"
"<path opacity=\"0.8\" fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" d=\"M62.6,61.4\r\n"
"\tc-9.1-9.1-9.1-23.7,0-32.8\"/>\r\n"
"<path opacity=\"0.5\" fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" d=\"M55.9,57.3\r\n"
"\tc-4.7-7.4-4.7-16.9-0.1-24.4\"/>\r\n"
"<line opacity=\"0.7\" fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" x1=\"57.7\" y1=\"41.5\" x2=\"37.8\" y2=\"41.5\"/>\r\n"
"<line opacity=\"0.7\" fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" x1=\"57.7\" y1=\"48.4\" x2=\"34.2\" y2=\"48.4\"/>\r\n"
"</svg>\r\n";

const char* wizard_AnimatedApp_svg = (const char*) temp_binary_data_33;

//================== wizard_AudioApp.svg ==================
static const unsigned char temp_binary_data_34[] =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
"<!-- Generator: Adobe Illustrator 18.0.0, SVG Export Plug-In . SVG Version: 6.00 Build 0)  -->\r\n"
"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\r\n"
"<svg version=\"1.1\" id=\"Layer_1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\r\n"
"\t viewBox=\"0 0 136.9 114.8\" enable-background=\"new 0 0 136.9 114.8\" xml:space=\"preserve\">\r\n"
"<path fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" d=\"M130,114.8H6.9c-3.8,0-6.9-3.1-6.9-6.9V6.9\r\n"
"\tC0,3.1,3.1,0,6.9,0H130c3.8,0,6.9,3.1,6.9,6.9v101.1C136.9,111.7,133.9,114.8,130,114.8z\"/>\r\n"
"<rect x=\"16.5\" y=\"10.9\" fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.3412\" stroke-miterlimit=\"10\" width=\"102.7\" height=\"2.4\"/>\r\n"
"<rect x=\"16.5\" y=\"10.9\" fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" width=\"102.9\" height=\"67.5\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"17.1\" y1=\"43.8\" x2=\"17.1\" y2=\"44.5\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"18.8\" y1=\"43.6\" x2=\"18.8\" y2=\"44.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"20.4\" y1=\"43.8\" x2=\"20.4\" y2=\"44.5\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"22.1\" y1=\"43.6\" x2=\"22.1\" y2=\"44.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"23.8\" y1=\"43.1\" x2=\"23.8\" y2=\"45.2\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"25.4\" y1=\"42.3\" x2=\"25.4\" y2=\"46\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"27.1\" y1=\"43.9\" x2=\"27.1\" y2=\"44.4\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"28.8\" y1=\"43.3\" x2=\"28.8\" y2=\"45\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"30.4\" y1=\"29.3\" x2=\"30.4\" y2=\"59\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"32.1\" y1=\"40.9\" x2=\"32.1\" y2=\"47.4\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"33.8\" y1=\"42.7\" x2=\"33.8\" y2=\"45.6\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"35.4\" y1=\"36.6\" x2=\"35.4\" y2=\"51.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"37.1\" y1=\"33.3\" x2=\"37.1\" y2=\"55.1\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"38.8\" y1=\"42.6\" x2=\"38.8\" y2=\"45.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"40.4\" y1=\"37.4\" x2=\"40.4\" y2=\"50.9\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"42.1\" y1=\"27.4\" x2=\"42.1\" y2=\"60.9\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"43.8\" y1=\"39.3\" x2=\"43.8\" y2=\"49\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"45.4\" y1=\"41.1\" x2=\"45.4\" y2=\"47.2\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"47.1\" y1=\"42.6\" x2=\"47.1\" y2=\"45.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"48.8\" y1=\"36.2\" x2=\"48.8\" y2=\"52.1\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"50.4\" y1=\"42.5\" x2=\"50.4\" y2=\"45.9\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"52.1\" y1=\"39.3\" x2=\"52.1\" y2=\"49\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"53.8\" y1=\"34.8\" x2=\"53.8\" y2=\"53.5\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"55.4\" y1=\"42.2\" x2=\"55.4\" y2=\"46.1\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"57.1\" y1=\"41\" x2=\"57.1\" y2=\"47.3\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"58.8\" y1=\"35.4\" x2=\"58.8\" y2=\"53\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"60.4\" y1=\"32.6\" x2=\"60.4\" y2=\"55.8\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"62.1\" y1=\"37.4\" x2=\"62.1\" y2=\"50.9\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"63.8\" y1=\"38.1\" x2=\"63.8\" y2=\"50.2\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"65.4\" y1=\"42.2\" x2=\"65.4\" y2=\"46.1\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"67.1\" y1=\"38.9\" x2=\"67.1\" y2=\"49.4\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"68.8\" y1=\"43.6\" x2=\"68.8\" y2=\"44.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"70.4\" y1=\"41.4\" x2=\"70.4\" y2=\"47\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"72.1\" y1=\"39\" x2=\"72.1\" y2=\"49.3\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"73.8\" y1=\"40.9\" x2=\"73.8\" y2=\"47.5\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"75.4\" y1=\"40.6\" x2=\"75.4\" y2=\"47.8\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"77.1\" y1=\"38.4\" x2=\"77.1\" y2=\"49.9\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"78.8\" y1=\"42\" x2=\"78.8\" y2=\"46.3\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"80.4\" y1=\"42.2\" x2=\"80.4\" y2=\"46.1\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"82.1\" y1=\"43.6\" x2=\"82.1\" y2=\"44.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"83.8\" y1=\"42.6\" x2=\"83.8\" y2=\"45.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"85.4\" y1=\"43.3\" x2=\"85.4\" y2=\"45\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"87.1\" y1=\"39.9\" x2=\"87.1\" y2=\"48.4\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"88.8\" y1=\"42.9\" x2=\"88.8\" y2=\"45.4\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"90.4\" y1=\"43.6\" x2=\"90.4\" y2=\"44.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"92.1\" y1=\"43.6\" x2=\"92.1\" y2=\"44.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"93.8\" y1=\"42.7\" x2=\"93.8\" y2=\"45.6\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"95.4\" y1=\"41.1\" x2=\"95.4\" y2=\"47.2\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"97.1\" y1=\"42.7\" x2=\"97.1\" y2=\"45.6\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"98.8\" y1=\"43.6\" x2=\"98.8\" y2=\"44.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"100.4\" y1=\"43.1\" x2=\"100.4\" y2=\"45.2\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"102.1\" y1=\"43.6\" x2=\"102.1\" y2=\"44.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"103.8\" y1=\"42\" x2=\"103.8\" y2=\"46.3\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"105.4\" y1=\"43.6\" x2=\"105.4\" y2=\"44.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"107.1\" y1=\"43.8\" x2=\"107.1\" y2=\"44.5\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"108.8\" y1=\"43.6\" x2=\"108.8\" y2=\"44.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"110.4\" y1=\"43.6\" x2=\"110.4\" y2=\"44.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"112.1\" y1=\"42.6\" x2=\"112.1\" y2=\"45.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"113.8\" y1=\"43.1\" x2=\"113.8\" y2=\"45.2\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"115.4\" y1=\"43.6\" x2=\"115.4\" y2=\"44.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"117.1\" y1=\"43.8\" x2=\"117.1\" y2=\"44.5\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"118.8\" y1=\"43.8\" x2=\"118.8\" y2=\"44.5\"/>\r\n"
"</svg>\r\n";

const char* wizard_AudioApp_svg = (const char*) temp_binary_data_34;

//================== wizard_AudioPlugin.svg ==================
static const unsigned char temp_binary_data_35[] =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
"<!-- Generator: Adobe Illustrator 18.0.0, SVG Export Plug-In . SVG Version: 6.00 Build 0)  -->\r\n"
"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\r\n"
"<svg version=\"1.1\" id=\"Layer_1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\r\n"
"\t viewBox=\"0 0 136.9 114.8\" enable-background=\"new 0 0 136.9 114.8\" xml:space=\"preserve\">\r\n"
"<g id=\"Layer_1_23_\">\r\n"
"\t<rect x=\"16.3\" y=\"10.9\" fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" width=\"103.6\" height=\"67.5\"/>\r\n"
"</g>\r\n"
"<rect x=\"16.3\" y=\"10.9\" fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" width=\"103.6\" height=\"2.4\"/>\r\n"
"<path fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" d=\"M130,114.8H6.9c-3.8,0-6.9-3.1-6.9-6.9V6.9\r\n"
"\tC0,3.1,3.1,0,6.9,0H130c3.8,0,6.9,3.1,6.9,6.9v101.1C136.9,111.7,133.9,114.8,130,114.8z\"/>\r\n"
"<g id=\"Layer_1_8_\">\r\n"
"\t<rect x=\"35.7\" y=\"23.6\" fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" width=\"64.4\" height=\"42.3\"/>\r\n"
"</g>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"36\" y1=\"44.1\" x2=\"36\" y2=\"44.6\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"37.1\" y1=\"44\" x2=\"37.1\" y2=\"44.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"38.1\" y1=\"44.1\" x2=\"38.1\" y2=\"44.6\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"39.2\" y1=\"44\" x2=\"39.2\" y2=\"44.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"40.2\" y1=\"43.7\" x2=\"40.2\" y2=\"45\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"41.3\" y1=\"43.2\" x2=\"41.3\" y2=\"45.5\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"42.3\" y1=\"44.2\" x2=\"42.3\" y2=\"44.5\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"43.3\" y1=\"43.8\" x2=\"43.3\" y2=\"44.9\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"44.4\" y1=\"35.1\" x2=\"44.4\" y2=\"53.6\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"45.4\" y1=\"42.3\" x2=\"45.4\" y2=\"46.4\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"46.5\" y1=\"43.4\" x2=\"46.5\" y2=\"45.3\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"47.5\" y1=\"39.6\" x2=\"47.5\" y2=\"49.1\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"48.6\" y1=\"37.5\" x2=\"48.6\" y2=\"51.2\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"49.6\" y1=\"43.4\" x2=\"49.6\" y2=\"45.3\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"50.6\" y1=\"40.1\" x2=\"50.6\" y2=\"48.6\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"51.7\" y1=\"33.9\" x2=\"51.7\" y2=\"54.9\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"52.7\" y1=\"41.3\" x2=\"52.7\" y2=\"47.4\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"53.8\" y1=\"42.5\" x2=\"53.8\" y2=\"46.3\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"54.8\" y1=\"43.4\" x2=\"54.8\" y2=\"45.3\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"55.9\" y1=\"39.4\" x2=\"55.9\" y2=\"49.3\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"56.9\" y1=\"43.3\" x2=\"56.9\" y2=\"45.4\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"57.9\" y1=\"41.3\" x2=\"57.9\" y2=\"47.4\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"59\" y1=\"38.5\" x2=\"59\" y2=\"50.2\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"60\" y1=\"43.1\" x2=\"60\" y2=\"45.6\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"61.1\" y1=\"42.4\" x2=\"61.1\" y2=\"46.3\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"62.1\" y1=\"38.8\" x2=\"62.1\" y2=\"49.9\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"63.2\" y1=\"37.1\" x2=\"63.2\" y2=\"51.6\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"64.2\" y1=\"40.1\" x2=\"64.2\" y2=\"48.6\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"65.2\" y1=\"40.6\" x2=\"65.2\" y2=\"48.1\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"66.3\" y1=\"43.1\" x2=\"66.3\" y2=\"45.6\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"67.3\" y1=\"41.1\" x2=\"67.3\" y2=\"47.6\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"68.4\" y1=\"44\" x2=\"68.4\" y2=\"44.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"69.4\" y1=\"42.6\" x2=\"69.4\" y2=\"46.1\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"70.5\" y1=\"41.1\" x2=\"70.5\" y2=\"47.6\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"71.5\" y1=\"42.3\" x2=\"71.5\" y2=\"46.4\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"72.5\" y1=\"42.1\" x2=\"72.5\" y2=\"46.6\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"73.6\" y1=\"40.8\" x2=\"73.6\" y2=\"47.9\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"74.6\" y1=\"43\" x2=\"74.6\" y2=\"45.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"75.7\" y1=\"43.1\" x2=\"75.7\" y2=\"45.6\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"76.7\" y1=\"44\" x2=\"76.7\" y2=\"44.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"77.8\" y1=\"43.4\" x2=\"77.8\" y2=\"45.3\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"78.8\" y1=\"43.8\" x2=\"78.8\" y2=\"44.9\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"79.9\" y1=\"41.7\" x2=\"79.9\" y2=\"47\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"80.9\" y1=\"43.6\" x2=\"80.9\" y2=\"45.2\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"81.9\" y1=\"44\" x2=\"81.9\" y2=\"44.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"83\" y1=\"44\" x2=\"83\" y2=\"44.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"84\" y1=\"43.4\" x2=\"84\" y2=\"45.3\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"85.1\" y1=\"42.5\" x2=\"85.1\" y2=\"46.3\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"86.1\" y1=\"43.5\" x2=\"86.1\" y2=\"45.2\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"87.2\" y1=\"44\" x2=\"87.2\" y2=\"44.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"88.2\" y1=\"43.7\" x2=\"88.2\" y2=\"45\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"89.2\" y1=\"44\" x2=\"89.2\" y2=\"44.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"90.3\" y1=\"43\" x2=\"90.3\" y2=\"45.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"91.3\" y1=\"44\" x2=\"91.3\" y2=\"44.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"92.4\" y1=\"44.2\" x2=\"92.4\" y2=\"44.6\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"93.4\" y1=\"44\" x2=\"93.4\" y2=\"44.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"94.5\" y1=\"44\" x2=\"94.5\" y2=\"44.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"95.5\" y1=\"43.4\" x2=\"95.5\" y2=\"45.3\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"96.5\" y1=\"43.7\" x2=\"96.5\" y2=\"45\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"97.6\" y1=\"44\" x2=\"97.6\" y2=\"44.7\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"98.6\" y1=\"44.2\" x2=\"98.6\" y2=\"44.6\"/>\r\n"
"<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-linecap=\"round\" stroke-miterlimit=\"10\" x1=\"99.7\" y1=\"44.2\" x2=\"99.7\" y2=\"44.6\"/>\r\n"
"<g>\r\n"
"\t<g>\r\n"
"\t\t<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-miterlimit=\"10\" x1=\"16.3\" y1=\"35.1\" x2=\"35.8\" y2=\"35.1\"/>\r\n"
"\t\t<g>\r\n"
"\t\t\t<circle fill=\"#F29300\" cx=\"35.7\" cy=\"35.1\" r=\"1.7\"/>\r\n"
"\t\t</g>\r\n"
"\t</g>\r\n"
"</g>\r\n"
"<g>\r\n"
"\t<g>\r\n"
"\t\t<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-miterlimit=\"10\" x1=\"16.3\" y1=\"54.4\" x2=\"35.8\" y2=\"54.4\"/>\r\n"
"\t\t<g>\r\n"
"\t\t\t<circle fill=\"#F29300\" cx=\"35.7\" cy=\"54.4\" r=\"1.7\"/>\r\n"
"\t\t</g>\r\n"
"\t</g>\r\n"
"</g>\r\n"
"<g>\r\n"
"\t<g>\r\n"
"\t\t<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-miterlimit=\"10\" x1=\"119.7\" y1=\"54.4\" x2=\"100.2\" y2=\"54.4\"/>\r\n"
"\t\t<g>\r\n"
"\t\t\t<circle fill=\"#F29300\" cx=\"100.3\" cy=\"54.4\" r=\"1.7\"/>\r\n"
"\t\t</g>\r\n"
"\t</g>\r\n"
"</g>\r\n"
"<g>\r\n"
"\t<g>\r\n"
"\t\t<line fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.5\" stroke-miterlimit=\"10\" x1=\"119.7\" y1=\"35.1\" x2=\"100.2\" y2=\"35.1\"/>\r\n"
"\t\t<g>\r\n"
"\t\t\t<circle fill=\"#F29300\" cx=\"100.3\" cy=\"35.1\" r=\"1.7\"/>\r\n"
"\t\t</g>\r\n"
"\t</g>\r\n"
"</g>\r\n"
"</svg>\r\n";

const char* wizard_AudioPlugin_svg = (const char*) temp_binary_data_35;

//================== wizard_ConsoleApp.svg ==================
static const unsigned char temp_binary_data_36[] =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
"<!-- Generator: Adobe Illustrator 18.0.0, SVG Export Plug-In . SVG Version: 6.00 Build 0)  -->\r\n"
"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\r\n"
"<svg version=\"1.1\" id=\"Layer_1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\r\n"
"\t viewBox=\"0 0 136.9 114.8\" enable-background=\"new 0 0 136.9 114.8\" xml:space=\"preserve\">\r\n"
"<path fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" d=\"M130,114.8H6.9c-3.8,0-6.9-3.1-6.9-6.9V6.9\r\n"
"\tC0,3.1,3.1,0,6.9,0H130c3.8,0,6.9,3.1,6.9,6.9v101.1C136.9,111.7,133.9,114.8,130,114.8z\"/>\r\n"
"<g id=\"Layer_1_22_\">\r\n"
"\t<rect x=\"16.3\" y=\"10.9\" fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" width=\"102.9\" height=\"67.5\"/>\r\n"
"</g>\r\n"
"<g id=\"Layer_3_3_\">\r\n"
"\t<g opacity=\"0.8\">\r\n"
"\t\t<path fill=\"#F29300\" d=\"M40.6,28.3l-14.4-6.6v-2.3l17.3,7.8v2.1l-17.3,7.8v-2.3L40.6,28.3z\"/>\r\n"
"\t\t<path fill=\"#F29300\" d=\"M62,39.5v1.7H45v-1.7H62z\"/>\r\n"
"\t</g>\r\n"
"\t<g>\r\n"
"\t\t<path fill=\"#F29300\" d=\"M40.6,28.3l-14.4-6.6v-2.3l17.3,7.8v2.1l-17.3,7.8v-2.3L40.6,28.3z\"/>\r\n"
"\t\t<path fill=\"#F29300\" d=\"M62,39.5v1.7H45v-1.7H62z\"/>\r\n"
"\t</g>\r\n"
"</g>\r\n"
"</svg>\r\n";

const char* wizard_ConsoleApp_svg = (const char*) temp_binary_data_36;

//================== wizard_DLL.svg ==================
static const unsigned char temp_binary_data_37[] =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
"<!-- Generator: Adobe Illustrator 18.0.0, SVG Export Plug-In . SVG Version: 6.00 Build 0)  -->\r\n"
"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\r\n"
"<svg version=\"1.1\" id=\"Layer_1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\r\n"
"\t viewBox=\"0 0 136.9 114.8\" enable-background=\"new 0 0 136.9 114.8\" xml:space=\"preserve\">\r\n"
"<path fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" d=\"M130,114.9H6.9c-3.8,0-6.9-3.1-6.9-6.9V7\r\n"
"\tc0-3.8,3.1-6.9,6.9-6.9H130c3.8,0,6.9,3.1,6.9,6.9V108C136.9,111.8,133.9,114.9,130,114.9z\"/>\r\n"
"<g id=\"Layer_1_19_\">\r\n"
"\t<rect x=\"16.3\" y=\"11\" fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" width=\"102.9\" height=\"67.5\"/>\r\n"
"</g>\r\n"
"<g>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4307\" stroke-miterlimit=\"10\" x1=\"39.6\" y1=\"25\" x2=\"39.6\" y2=\"62.3\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4307\" stroke-miterlimit=\"10\" x1=\"41.2\" y1=\"25\" x2=\"41.2\" y2=\"62.3\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4307\" stroke-miterlimit=\"10\" x1=\"42.8\" y1=\"25\" x2=\"42.8\" y2=\"62.3\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4307\" stroke-miterlimit=\"10\" x1=\"44.5\" y1=\"25\" x2=\"44.5\" y2=\"62.3\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4307\" stroke-miterlimit=\"10\" x1=\"46.1\" y1=\"25\" x2=\"46.1\" y2=\"62.3\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4307\" stroke-miterlimit=\"10\" x1=\"47.7\" y1=\"25\" x2=\"47.7\" y2=\"62.3\"/>\r\n"
"</g>\r\n"
"<path fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.0853\" stroke-miterlimit=\"10\" d=\"M49.9,23.9v38.6c0,0.9-0.7,1.6-1.6,1.6H39\r\n"
"\tc-0.9,0-1.6-0.7-1.6-1.6V23.9\"/>\r\n"
"<g>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4307\" stroke-miterlimit=\"10\" x1=\"54.8\" y1=\"27.5\" x2=\"67.4\" y2=\"62.6\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4307\" stroke-miterlimit=\"10\" x1=\"56.3\" y1=\"27\" x2=\"69\" y2=\"62\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4307\" stroke-miterlimit=\"10\" x1=\"57.8\" y1=\"26.4\" x2=\"70.5\" y2=\"61.5\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4307\" stroke-miterlimit=\"10\" x1=\"59.4\" y1=\"25.9\" x2=\"72.1\" y2=\"60.9\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4307\" stroke-miterlimit=\"10\" x1=\"60.9\" y1=\"25.3\" x2=\"73.6\" y2=\"60.4\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4307\" stroke-miterlimit=\"10\" x1=\"62.4\" y1=\"24.8\" x2=\"75.1\" y2=\"59.8\"/>\r\n"
"</g>\r\n"
"<path fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.0853\" stroke-miterlimit=\"10\" d=\"M64.1,23l13.1,36.3c0.3,0.8-0.1,1.7-0.9,2\r\n"
"\tl-8.8,3.2c-0.8,0.3-1.7-0.1-2-0.9L52.4,27.2\"/>\r\n"
"<g>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4307\" stroke-miterlimit=\"10\" x1=\"72.5\" y1=\"30.6\" x2=\"93.5\" y2=\"61.4\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4307\" stroke-miterlimit=\"10\" x1=\"73.8\" y1=\"29.7\" x2=\"94.9\" y2=\"60.5\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4307\" stroke-miterlimit=\"10\" x1=\"75.2\" y1=\"28.8\" x2=\"96.2\" y2=\"59.6\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4307\" stroke-miterlimit=\"10\" x1=\"76.5\" y1=\"27.9\" x2=\"97.6\" y2=\"58.6\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4307\" stroke-miterlimit=\"10\" x1=\"77.9\" y1=\"27\" x2=\"98.9\" y2=\"57.7\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4307\" stroke-miterlimit=\"10\" x1=\"79.2\" y1=\"26\" x2=\"100.3\" y2=\"56.8\"/>\r\n"
"</g>\r\n"
"<path fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.0853\" stroke-miterlimit=\"10\" d=\"M80.4,23.9l21.8,31.9c0.5,0.7,0.3,1.7-0.4,2.2\r\n"
"\tl-7.7,5.3c-0.7,0.5-1.7,0.3-2.2-0.4L70.1,31\"/>\r\n"
"</svg>\r\n";

const char* wizard_DLL_svg = (const char*) temp_binary_data_37;

//================== wizard_GUI.svg ==================
static const unsigned char temp_binary_data_38[] =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
"<!-- Generator: Adobe Illustrator 18.0.0, SVG Export Plug-In . SVG Version: 6.00 Build 0)  -->\r\n"
"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\r\n"
"<svg version=\"1.1\" id=\"Layer_1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\r\n"
"\t viewBox=\"0 0 136.9 114.8\" enable-background=\"new 0 0 136.9 114.8\" xml:space=\"preserve\">\r\n"
"<rect x=\"16.3\" y=\"10.9\" fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" width=\"103.6\" height=\"67.5\"/>\r\n"
"<path fill=\"none\" stroke=\"#F29300\" stroke-width=\"0.7078\" stroke-miterlimit=\"10\" d=\"M55.2,62.5H23.4c-0.4,0-0.7-0.4-0.7-0.9l0,0\r\n"
"\tc0-0.5,0.3-0.9,0.7-0.9h31.8c0.4,0,0.7,0.4,0.7,0.9l0,0C55.9,62.1,55.5,62.5,55.2,62.5z\"/>\r\n"
"<path fill=\"#F29300\" d=\"M38.1,62.5H23.4c-0.4,0-0.7-0.4-0.7-0.9l0,0c0-0.5,0.3-0.9,0.7-0.9h14.7c0.4,0,0.7,0.4,0.7,0.9l0,0\r\n"
"\tC38.7,62.1,38.4,62.5,38.1,62.5z\"/>\r\n"
"<path fill=\"none\" stroke=\"#F29300\" stroke-width=\"0.7078\" stroke-miterlimit=\"10\" d=\"M55.2,67.8H23.4c-0.4,0-0.7-0.4-0.7-0.9l0,0\r\n"
"\tc0-0.5,0.3-0.9,0.7-0.9h31.8c0.4,0,0.7,0.4,0.7,0.9l0,0C55.9,67.4,55.5,67.8,55.2,67.8z\"/>\r\n"
"<path fill=\"#F29300\" d=\"M44.2,67.8H23.4c-0.4,0-0.7-0.4-0.7-0.9l0,0c0-0.5,0.3-0.9,0.7-0.9h20.8c0.4,0,0.7,0.4,0.7,0.9l0,0\r\n"
"\tC44.8,67.4,44.6,67.8,44.2,67.8z\"/>\r\n"
"<path fill=\"none\" stroke=\"#F29300\" stroke-width=\"0.7078\" stroke-miterlimit=\"10\" d=\"M55.2,73H23.4c-0.4,0-0.7-0.4-0.7-0.9l0,0\r\n"
"\tc0-0.5,0.3-0.9,0.7-0.9h31.8c0.4,0,0.7,0.4,0.7,0.9l0,0C55.9,72.6,55.5,73,55.2,73z\"/>\r\n"
"<path fill=\"#F29300\" d=\"M49.4,73h-26c-0.4,0-0.7-0.4-0.7-0.9l0,0c0-0.5,0.3-0.9,0.7-0.9h26c0.4,0,0.7,0.4,0.7,0.9l0,0\r\n"
"\tC50.1,72.6,49.8,73,49.4,73z\"/>\r\n"
"<rect x=\"16.3\" y=\"10.9\" fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" width=\"103.6\" height=\"2.4\"/>\r\n"
"<circle fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" cx=\"69.3\" cy=\"39.6\" r=\"13.9\"/>\r\n"
"<g>\r\n"
"\t<g>\r\n"
"\t\t<path fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.2523\" stroke-miterlimit=\"10\" d=\"M79.8,33.5c1.1,1.9,1.3,5.2,0,5.7\r\n"
"\t\t\tc-1.3,0.5-7.2,0.3-10.2,0.2c1.3-2.7,4.1-7.9,5.2-8.8C75.9,29.7,78.7,31.6,79.8,33.5z\"/>\r\n"
"\t</g>\r\n"
"\t<g>\r\n"
"\t\t<path fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.2523\" stroke-miterlimit=\"10\" d=\"M79.8,45.6c-1.1,1.9-3.8,3.7-4.9,2.9\r\n"
"\t\t\tc-1.1-0.9-3.9-6.1-5.2-8.8c3-0.2,8.9-0.4,10.2,0.1C81.2,40.4,80.9,43.7,79.8,45.6z\"/>\r\n"
"\t</g>\r\n"
"\t<g>\r\n"
"\t\t<path fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.2523\" stroke-miterlimit=\"10\" d=\"M69.3,51.7c-2.2,0-5.2-1.5-5-2.8\r\n"
"\t\t\ts3.3-6.4,5-8.9c1.7,2.5,4.8,7.5,5,8.9S71.5,51.7,69.3,51.7z\"/>\r\n"
"\t</g>\r\n"
"\t<g>\r\n"
"\t\t<path fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.2523\" stroke-miterlimit=\"10\" d=\"M58.9,45.6c-1.1-1.9-1.3-5.2,0-5.7\r\n"
"\t\t\tc1.3-0.5,7.2-0.3,10.2-0.2c-1.3,2.7-4.1,7.9-5.2,8.8C62.8,49.4,60,47.6,58.9,45.6z\"/>\r\n"
"\t</g>\r\n"
"\t<g>\r\n"
"\t\t<path fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.2523\" stroke-miterlimit=\"10\" d=\"M58.9,33.5c1.1-1.9,3.8-3.7,4.9-2.9\r\n"
"\t\t\tc1.1,0.9,3.9,6.1,5.2,8.8c-3,0.2-8.9,0.4-10.2-0.1C57.5,38.8,57.8,35.5,58.9,33.5z\"/>\r\n"
"\t</g>\r\n"
"\t<g>\r\n"
"\t\t<path fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.2523\" stroke-miterlimit=\"10\" d=\"M69.3,27.5c2.2,0,5.2,1.5,5,2.8\r\n"
"\t\t\ts-3.3,6.4-5,8.9c-1.7-2.5-4.8-7.5-5-8.9S67.1,27.5,69.3,27.5z\"/>\r\n"
"\t</g>\r\n"
"</g>\r\n"
"<path fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" d=\"M130,114.8H6.9c-3.8,0-6.9-3.1-6.9-6.9V6.9\r\n"
"\tC0,3.1,3.1,0,6.9,0H130c3.8,0,6.9,3.1,6.9,6.9v101.1C136.9,111.7,133.9,114.8,130,114.8z\"/>\r\n"
"</svg>\r\n";

const char* wizard_GUI_svg = (const char*) temp_binary_data_38;

//================== wizard_Highlight.svg ==================
static const unsigned char temp_binary_data_39[] =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
"<!-- Generator: Adobe Illustrator 18.0.0, SVG Export Plug-In . SVG Version: 6.00 Build 0)  -->\r\n"
"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\r\n"
"<svg version=\"1.1\" id=\"Layer_1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\r\n"
"\t viewBox=\"0 0 136.9 114.8\" enable-background=\"new 0 0 136.9 114.8\" xml:space=\"preserve\">\r\n"
"<path opacity=\"0.2\" fill=\"#F29300\" d=\"M130.1,114.8H6.9c-3.8,0-6.9-3.1-6.9-6.9V6.9C0,3.1,3.1,0,6.9,0h123.2c3.8,0,6.9,3.1,6.9,6.9\r\n"
"\tV108C136.9,111.8,133.9,114.8,130.1,114.8z\"/>\r\n"
"</svg>\r\n";

const char* wizard_Highlight_svg = (const char*) temp_binary_data_39;

//================== wizard_Openfile.svg ==================
static const unsigned char temp_binary_data_40[] =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
"<!-- Generator: Adobe Illustrator 18.0.0, SVG Export Plug-In . SVG Version: 6.00 Build 0)  -->\r\n"
"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\r\n"
"<svg version=\"1.1\" id=\"Layer_1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\r\n"
"\t viewBox=\"0 0 177.9 114.8\" enable-background=\"new 0 0 177.9 114.8\" xml:space=\"preserve\">\r\n"
"<path fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" d=\"M164.7,71.4H13.2c-3.8,0-6.9-3.1-6.9-6.9V43.6\r\n"
"\tc0-3.8,3.1-6.9,6.9-6.9h151.5c3.8,0,6.9,3.1,6.9,6.9v20.9C171.6,68.3,168.5,71.4,164.7,71.4z\"/>\r\n"
"</svg>\r\n";

const char* wizard_Openfile_svg = (const char*) temp_binary_data_40;

//================== wizard_OpenGL.svg ==================
static const unsigned char temp_binary_data_41[] =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
"<!-- Generator: Adobe Illustrator 18.0.0, SVG Export Plug-In . SVG Version: 6.00 Build 0)  -->\r\n"
"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\r\n"
"<svg version=\"1.1\" id=\"Layer_1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\r\n"
"\t viewBox=\"0 0 136.9 114.8\" enable-background=\"new 0 0 136.9 114.8\" xml:space=\"preserve\">\r\n"
"<rect x=\"16.3\" y=\"10.9\" fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" width=\"103.6\" height=\"67.5\"/>\r\n"
"<rect x=\"16.3\" y=\"10.9\" fill=\"none\" stroke=\"#F29300\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" width=\"103.6\" height=\"2.4\"/>\r\n"
"<path fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" d=\"M75.8,31.5c8.7,4.7,14.8,13.2,13.5,20\r\n"
"\tc-1.6,8.5-13.8,11.4-26.8,4.3S45.3,36.9,51.6,30.9C56.7,26.1,67.1,26.7,75.8,31.5z\"/>\r\n"
"<path fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" d=\"M86.1,43.6c1,2.4,0.5,5.5-1.3,5\r\n"
"\tc-1.9-0.5-9.8-5-13.8-7.4c2.9-1.2,8.2-3.2,9.8-3.2C82.2,38.2,85.2,41.4,86.1,43.6z\"/>\r\n"
"<path fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" d=\"M83,55.3c-2.3,1.3-7.3,1.2-8.6-0.7\r\n"
"\tc-1.2-1.9-2.9-9.5-3.5-12.9c4.2,2.1,12.2,6.3,13.8,7.7C86.3,50.7,85.2,54,83,55.3z\"/>\r\n"
"<path fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" d=\"M63.6,53.5c-3.6-2-7.3-6.3-5.9-7.6\r\n"
"\tc1.3-1.2,9-3.5,12.5-4.4c1.2,3.5,3.4,11.2,3.1,13C73,56.3,67.3,55.5,63.6,53.5z\"/>\r\n"
"<path fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" d=\"M51.9,38.3c-0.1-2.5,2-5.2,4.1-4.6\r\n"
"\tc2.1,0.6,9.9,5.1,13.9,7.5c-3.2,1.3-10.5,4.1-12.8,4.1C54.9,45.1,52,40.9,51.9,38.3z\"/>\r\n"
"<path fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" d=\"M60,29.3c2.4-0.4,6.6,0.3,7.5,1.5\r\n"
"\tc0.9,1.2,2.1,6.8,2.6,9.9c-4.2-2.1-12.3-6.3-13.7-7.6C55,31.8,57.4,29.8,60,29.3z\"/>\r\n"
"<path fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" d=\"M75.2,32.5c2.5,1.4,5.6,4.3,5,5\r\n"
"\tc-0.7,0.8-6.3,2.5-9.5,3.3c-1.1-3.2-2.7-8.9-2.4-9.9C68.6,30,72.6,31.1,75.2,32.5z\"/>\r\n"
"<path fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" d=\"M130,114.8H6.9c-3.8,0-6.9-3.1-6.9-6.9V6.9\r\n"
"\tC0,3.1,3.1,0,6.9,0H130c3.8,0,6.9,3.1,6.9,6.9v101.1C136.9,111.7,133.9,114.8,130,114.8z\"/>\r\n"
"<path fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" d=\"M87.4,55.5c-6.6,10.4-20.4,14.6-30.8,8\r\n"
"\ts-13.4-19.6-6.9-30\"/>\r\n"
"</svg>\r\n";

const char* wizard_OpenGL_svg = (const char*) temp_binary_data_41;

//================== wizard_StaticLibrary.svg ==================
static const unsigned char temp_binary_data_42[] =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
"<!-- Generator: Adobe Illustrator 18.0.0, SVG Export Plug-In . SVG Version: 6.00 Build 0)  -->\r\n"
"<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\r\n"
"<svg version=\"1.1\" id=\"Layer_1\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" x=\"0px\" y=\"0px\"\r\n"
"\t viewBox=\"0 0 136.9 114.8\" enable-background=\"new 0 0 136.9 114.8\" xml:space=\"preserve\">\r\n"
"<path fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" d=\"M130,114.8H6.9c-3.8,0-6.9-3.1-6.9-6.9V6.9\r\n"
"\tC0,3,3.1,0,6.9,0H130c3.8,0,6.9,3.1,6.9,6.9v101.1C136.9,111.7,133.9,114.8,130,114.8z\"/>\r\n"
"<g id=\"Layer_1_21_\">\r\n"
"\t<rect x=\"16.3\" y=\"10.9\" fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.3469\" stroke-miterlimit=\"10\" width=\"102.9\" height=\"67.5\"/>\r\n"
"</g>\r\n"
"<g>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4942\" stroke-miterlimit=\"10\" x1=\"43.1\" y1=\"22.1\" x2=\"43.1\" y2=\"64.9\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4942\" stroke-miterlimit=\"10\" x1=\"45\" y1=\"22.1\" x2=\"45\" y2=\"64.9\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4942\" stroke-miterlimit=\"10\" x1=\"46.8\" y1=\"22.1\" x2=\"46.8\" y2=\"64.9\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4942\" stroke-miterlimit=\"10\" x1=\"48.7\" y1=\"22.1\" x2=\"48.7\" y2=\"64.9\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4942\" stroke-miterlimit=\"10\" x1=\"50.6\" y1=\"22.1\" x2=\"50.6\" y2=\"64.9\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4942\" stroke-miterlimit=\"10\" x1=\"52.5\" y1=\"22.1\" x2=\"52.5\" y2=\"64.9\"/>\r\n"
"</g>\r\n"
"<path fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.2453\" stroke-miterlimit=\"10\" d=\"M54.9,20.8v44.3c0,1-0.8,1.8-1.8,1.8H42.4\r\n"
"\tc-1,0-1.8-0.8-1.8-1.8V20.8\"/>\r\n"
"<g>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4942\" stroke-miterlimit=\"10\" x1=\"64.5\" y1=\"22.1\" x2=\"64.5\" y2=\"64.9\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4942\" stroke-miterlimit=\"10\" x1=\"66.4\" y1=\"22.1\" x2=\"66.4\" y2=\"64.9\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4942\" stroke-miterlimit=\"10\" x1=\"68.3\" y1=\"22.1\" x2=\"68.3\" y2=\"64.9\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4942\" stroke-miterlimit=\"10\" x1=\"70.1\" y1=\"22.1\" x2=\"70.1\" y2=\"64.9\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4942\" stroke-miterlimit=\"10\" x1=\"72\" y1=\"22.1\" x2=\"72\" y2=\"64.9\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4942\" stroke-miterlimit=\"10\" x1=\"73.9\" y1=\"22.1\" x2=\"73.9\" y2=\"64.9\"/>\r\n"
"</g>\r\n"
"<path fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.2453\" stroke-miterlimit=\"10\" d=\"M76.4,20.8v44.3c0,1-0.8,1.8-1.8,1.8H63.8\r\n"
"\tc-1,0-1.8-0.8-1.8-1.8V20.8\"/>\r\n"
"<g>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4942\" stroke-miterlimit=\"10\" x1=\"85.9\" y1=\"22.1\" x2=\"85.9\" y2=\"64.9\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4942\" stroke-miterlimit=\"10\" x1=\"87.8\" y1=\"22.1\" x2=\"87.8\" y2=\"64.9\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4942\" stroke-miterlimit=\"10\" x1=\"89.7\" y1=\"22.1\" x2=\"89.7\" y2=\"64.9\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4942\" stroke-miterlimit=\"10\" x1=\"91.6\" y1=\"22.1\" x2=\"91.6\" y2=\"64.9\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4942\" stroke-miterlimit=\"10\" x1=\"93.4\" y1=\"22.1\" x2=\"93.4\" y2=\"64.9\"/>\r\n"
"\t<line fill=\"none\" stroke=\"#F29100\" stroke-width=\"0.4942\" stroke-miterlimit=\"10\" x1=\"95.3\" y1=\"22.1\" x2=\"95.3\" y2=\"64.9\"/>\r\n"
"</g>\r\n"
"<path fill=\"none\" stroke=\"#F29100\" stroke-width=\"1.2453\" stroke-miterlimit=\"10\" d=\"M97.8,20.8v44.3c0,1-0.8,1.8-1.8,1.8H85.3\r\n"
"\tc-1,0-1.8-0.8-1.8-1.8V20.8\"/>\r\n"
"</svg>\r\n";

const char* wizard_StaticLibrary_svg = (const char*) temp_binary_data_42;


const char* getNamedResource (const char*, int&) throw();
const char* getNamedResource (const char* resourceNameUTF8, int& numBytes) throw()
{
    unsigned int hash = 0;
    if (resourceNameUTF8 != 0)
        while (*resourceNameUTF8 != 0)
            hash = 31 * hash + (unsigned int) *resourceNameUTF8++;

    switch (hash)
    {
        case 0x6cf2645e:  numBytes = 1949; return jucer_AnimatedComponentTemplate_cpp;
        case 0xafccbd3f:  numBytes = 2991; return jucer_AudioComponentTemplate_cpp;
        case 0x27c5a93a:  numBytes = 1180; return jucer_AudioPluginEditorTemplate_cpp;
        case 0x4d0721bf:  numBytes = 1012; return jucer_AudioPluginEditorTemplate_h;
        case 0x51b49ac5:  numBytes = 5039; return jucer_AudioPluginFilterTemplate_cpp;
        case 0x488afa0a:  numBytes = 2727; return jucer_AudioPluginFilterTemplate_h;
        case 0xabad7041:  numBytes = 2083; return jucer_ComponentTemplate_cpp;
        case 0xfc72fe86:  numBytes = 2156; return jucer_ComponentTemplate_h;
        case 0x0b66646c:  numBytes = 886; return jucer_ContentCompTemplate_cpp;
        case 0x6fa10171:  numBytes = 924; return jucer_ContentCompTemplate_h;
        case 0x28d496ad:  numBytes = 1143; return jucer_InlineComponentTemplate_h;
        case 0x8905395b:  numBytes = 470; return jucer_MainConsoleAppTemplate_cpp;
        case 0x5e5ea047:  numBytes = 2010; return jucer_MainTemplate_NoWindow_cpp;
        case 0xda2391f8:  numBytes = 3866; return jucer_MainTemplate_SimpleWindow_cpp;
        case 0x400bc026:  numBytes = 3778; return jucer_MainTemplate_Window_cpp;
        case 0xf4842835:  numBytes = 1389; return jucer_NewComponentTemplate_cpp;
        case 0xe7bf237a:  numBytes = 648; return jucer_NewComponentTemplate_h;
        case 0x02a2a077:  numBytes = 262; return jucer_NewCppFileTemplate_cpp;
        case 0x0842c43c:  numBytes = 308; return jucer_NewCppFileTemplate_h;
        case 0x36e634a1:  numBytes = 1626; return jucer_NewInlineComponentTemplate_h;
        case 0x7fbac252:  numBytes = 1827; return jucer_OpenGLComponentTemplate_cpp;
        case 0x44be9398:  numBytes = 2922; return AudioPluginXCodeScript_txt;
        case 0x4a0cfd09:  numBytes = 151; return background_tile_png;
        case 0x763d39dc:  numBytes = 1050; return colourscheme_dark_xml;
        case 0xe8b08520:  numBytes = 1050; return colourscheme_light_xml;
        case 0x154a7275:  numBytes = 83091; return juce_icon_png;
        case 0x507a15c7:  numBytes = 8150; return projectIconAndroid_png;
        case 0xe8e2796f:  numBytes = 11917; return projectIconCodeblocks_png;
        case 0x90374ad6:  numBytes = 16444; return projectIconLinuxMakefile_png;
        case 0x20236af2:  numBytes = 7194; return projectIconVisualStudio_png;
        case 0xecc12a3d:  numBytes = 18281; return projectIconXcode_png;
        case 0x9d3ae124:  numBytes = 18111; return projectIconXcodeIOS_png;
        case 0xa41e649d:  numBytes = 2842; return RecentFilesMenuTemplate_nib;
        case 0x1f3b6d2f:  numBytes = 2963; return wizard_AnimatedApp_svg;
        case 0x60296d04:  numBytes = 9802; return wizard_AudioApp_svg;
        case 0x1115ccda:  numBytes = 10809; return wizard_AudioPlugin_svg;
        case 0x1d65d363:  numBytes = 1204; return wizard_ConsoleApp_svg;
        case 0xba5a4595:  numBytes = 3588; return wizard_DLL_svg;
        case 0x683e4e6c:  numBytes = 3448; return wizard_GUI_svg;
        case 0x2e6bf065:  numBytes = 638; return wizard_Highlight_svg;
        case 0x52a8dfdf:  numBytes = 686; return wizard_Openfile_svg;
        case 0x58e2ae48:  numBytes = 2497; return wizard_OpenGL_svg;
        case 0xb1da6f9e:  numBytes = 3563; return wizard_StaticLibrary_svg;
        default: break;
    }

    numBytes = 0;
    return 0;
}

const char* namedResourceList[] =
{
    "jucer_AnimatedComponentTemplate_cpp",
    "jucer_AudioComponentTemplate_cpp",
    "jucer_AudioPluginEditorTemplate_cpp",
    "jucer_AudioPluginEditorTemplate_h",
    "jucer_AudioPluginFilterTemplate_cpp",
    "jucer_AudioPluginFilterTemplate_h",
    "jucer_ComponentTemplate_cpp",
    "jucer_ComponentTemplate_h",
    "jucer_ContentCompTemplate_cpp",
    "jucer_ContentCompTemplate_h",
    "jucer_InlineComponentTemplate_h",
    "jucer_MainConsoleAppTemplate_cpp",
    "jucer_MainTemplate_NoWindow_cpp",
    "jucer_MainTemplate_SimpleWindow_cpp",
    "jucer_MainTemplate_Window_cpp",
    "jucer_NewComponentTemplate_cpp",
    "jucer_NewComponentTemplate_h",
    "jucer_NewCppFileTemplate_cpp",
    "jucer_NewCppFileTemplate_h",
    "jucer_NewInlineComponentTemplate_h",
    "jucer_OpenGLComponentTemplate_cpp",
    "AudioPluginXCodeScript_txt",
    "background_tile_png",
    "colourscheme_dark_xml",
    "colourscheme_light_xml",
    "juce_icon_png",
    "projectIconAndroid_png",
    "projectIconCodeblocks_png",
    "projectIconLinuxMakefile_png",
    "projectIconVisualStudio_png",
    "projectIconXcode_png",
    "projectIconXcodeIOS_png",
    "RecentFilesMenuTemplate_nib",
    "wizard_AnimatedApp_svg",
    "wizard_AudioApp_svg",
    "wizard_AudioPlugin_svg",
    "wizard_ConsoleApp_svg",
    "wizard_DLL_svg",
    "wizard_GUI_svg",
    "wizard_Highlight_svg",
    "wizard_Openfile_svg",
    "wizard_OpenGL_svg",
    "wizard_StaticLibrary_svg"
};

}
