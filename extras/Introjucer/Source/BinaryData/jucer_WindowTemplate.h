/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic outline for a simple desktop window.

  ==============================================================================
*/

#ifndef HEADERGUARD
#define HEADERGUARD

INCLUDES


//==============================================================================
class WINDOWCLASS   : public DocumentWindow
{
public:
    //==============================================================================
    WINDOWCLASS();
    ~WINDOWCLASS();

    void closeButtonPressed();


    /* Note: Be careful when overriding DocumentWindow methods - the base class
       uses a lot of them, so by overriding you might break its functionality.
       It's best to do all your work in you content component instead, but if
       you really have to override any DocumentWindow methods, make sure your
       implementation calls the superclass's method.
    */

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WINDOWCLASS)
};


#endif  // HEADERGUARD
