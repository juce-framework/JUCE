/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

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
