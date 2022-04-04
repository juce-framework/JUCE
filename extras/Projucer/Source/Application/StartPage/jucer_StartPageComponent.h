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

struct ContentComponent;
struct ProjectTemplatesAndExamples;

//==============================================================================
class StartPageComponent  : public Component
{
public:
    StartPageComponent (std::function<void (std::unique_ptr<Project>&&)>&& newProjectCb,
                        std::function<void (const File&)>&& exampleCb);

    void paint (Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    std::unique_ptr<ContentComponent> content;
    std::unique_ptr<ProjectTemplatesAndExamples> tabs;

    TextButton openExistingButton { "Open Existing Project..." };

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StartPageComponent)
};
