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

#include <JuceHeader.h>
#include "DemoContentComponent.h"

//==============================================================================
class MainComponent    : public Component
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

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
