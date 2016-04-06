/*
  ==============================================================================

    FILENAME
    Created: DATE
    Author:  AUTHOR

  ==============================================================================
*/

#ifndef HEADERGUARD
#define HEADERGUARD

INCLUDE_JUCE

//==============================================================================
/*
*/
class COMPONENTCLASS    : public Component
{
public:
    COMPONENTCLASS();
    ~COMPONENTCLASS();

    void paint (Graphics&);
    void resized();

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (COMPONENTCLASS)
};


#endif  // HEADERGUARD
