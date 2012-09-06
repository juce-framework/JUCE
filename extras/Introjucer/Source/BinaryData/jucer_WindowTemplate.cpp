/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic structure for a simple desktop window.

  ==============================================================================
*/

INCLUDE_CORRESPONDING_HEADER


//==============================================================================
WINDOWCLASS::WINDOWCLASS()
    : DocumentWindow ("WINDOWCLASS",
                      Colours::lightgrey,
                      DocumentWindow::allButtons)
{
    // At this point you should call setContentOwned() to give the window
    // a component containing the content you want to show..


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
