/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic outline for a simple desktop window.

  ==============================================================================
*/

INCLUDES


//==============================================================================
WINDOWCLASS::WINDOWCLASS()
    : DocumentWindow (JUCEApplication::getInstance()->getApplicationName(),
                      Colours::lightgrey,
                      DocumentWindow::allButtons)
{
    centreWithSize (500, 400);
    setVisible (true);
}

WINDOWCLASS::~WINDOWCLASS()
{
}

void WINDOWCLASS::closeButtonPressed()
{
    JUCEApplication::getInstance()->systemRequestedQuit();
}
