/*
  ==============================================================================

    FILENAME
    Created: DATE
    Author:  AUTHOR

  ==============================================================================
*/

#pragma once

INCLUDE_JUCE

//==============================================================================
/*
*/
class COMPONENTCLASS    : public Component
{
public:
    COMPONENTCLASS();
    ~COMPONENTCLASS();

    void paint (Graphics&) override;
    void resized() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (COMPONENTCLASS)
};
