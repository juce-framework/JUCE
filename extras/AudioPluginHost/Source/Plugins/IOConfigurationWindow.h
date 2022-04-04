/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

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
