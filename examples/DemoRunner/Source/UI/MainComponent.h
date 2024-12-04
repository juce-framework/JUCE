/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DemoContentComponent.h"

//==============================================================================
class MainComponent final : public Component
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
