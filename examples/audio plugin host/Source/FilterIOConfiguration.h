/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
 */

#ifndef FILTERIOCONFIGURATION_H_INCLUDED
#define FILTERIOCONFIGURATION_H_INCLUDED

#include "FilterGraph.h"

class FilterIOConfigurationWindow       : public  AudioProcessorEditor
{
public:
    class InputOutputConfig;

    //==============================================================================
    FilterIOConfigurationWindow (AudioProcessor* const p);
    ~FilterIOConfigurationWindow();

    //==============================================================================
    void paint (Graphics& g) override;
    void resized() override;

    //==============================================================================
    InputOutputConfig* getConfig (bool isInput) noexcept { return isInput ? inConfig : outConfig; }
    void update();
private:

    //==============================================================================
    MainHostWindow* getMainWindow() const;
    GraphDocumentComponent* getGraphEditor() const;
    AudioProcessorGraph* getGraph() const;
    int32 getNodeId() const;

    //==============================================================================
    friend class InputOutputConfig;

    AudioProcessor::BusesLayout currentLayout;
    Label title;
    ScopedPointer<InputOutputConfig> inConfig, outConfig;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilterIOConfigurationWindow)
};


#endif  // FILTERIOCONFIGURATION_H_INCLUDED
