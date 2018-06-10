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

#include "../JuceLibraryCode/JuceHeader.h"
#include "DemoContentComponent.h"

//==============================================================================
class MainComponent    : public Component
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

    //==============================================================================
    SidePanel& getSidePanel()              { return demosPanel; }

    //==============================================================================
    void homeButtonClicked();
    void settingsButtonClicked();

    //==============================================================================
    StringArray getRenderingEngines()      { return renderingEngines; }
    int getCurrentRenderingEngine()        { return currentRenderingEngineIdx; }
    void setRenderingEngine (int index);

private:
    void parentHierarchyChanged() override;
    void updateRenderingEngine (int index);

    //==============================================================================
    std::unique_ptr<DemoContentComponent> contentComponent;
    SidePanel demosPanel  { "Demos", 250, true };

    OpenGLContext openGLContext;
    ComponentPeer* peer = nullptr;
    StringArray renderingEngines;
    int currentRenderingEngineIdx = -1;

    TextButton showDemosButton      { "Browse Demos" };

    bool isShowingHeavyweightDemo = false;
    int sidePanelWidth = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
