/*
  ==============================================================================

    %%filename%%
    Created: %%date%%
    Author:  %%author%%

  ==============================================================================
*/

#pragma once

%%include_juce%%

//==============================================================================
/*
*/
class %%component_class%%  : public juce::Component
{
public:
    %%component_class%%();
    ~%%component_class%%() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (%%component_class%%)
};
