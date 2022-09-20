/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

class MainHostWindow;
class GraphDocumentComponent;


//==============================================================================
class IOConfigurationWindow   : public  AudioProcessorEditor
{
public:
    IOConfigurationWindow (AudioProcessor&);
    ~IOConfigurationWindow() override;

    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;

private:
    class InputOutputConfig;

    AudioProcessor::BusesLayout currentLayout;
    Label title;
    std::unique_ptr<InputOutputConfig> inConfig, outConfig;

    InputOutputConfig* getConfig (bool isInput) noexcept    { return isInput ? inConfig.get() : outConfig.get(); }
    void update();

    MainHostWindow* getMainWindow() const;
    GraphDocumentComponent* getGraphEditor() const;
    AudioProcessorGraph* getGraph() const;
    AudioProcessorGraph::NodeID getNodeID() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IOConfigurationWindow)
};
