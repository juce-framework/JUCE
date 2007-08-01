/*
  ==============================================================================

   Demonstration "Hello World" application in JUCE
   Copyright 2004 by Julian Storer.

  ==============================================================================
*/

#include "../../../juce.h"

//==============================================================================
/** This is the component that sits inside the "hello world" window, filling its
    content area. In this example, we'll just write "hello world" inside it.
*/
class HelloWorldContentComponent    : public Component
{
public:
    HelloWorldContentComponent()
    {
    }

    ~HelloWorldContentComponent()
    {
    }

    void paint (Graphics& g)
    {
        // clear the background with solid white
        g.fillAll (Colours::white);

        // set our drawing colour to black..
        g.setColour (Colours::black);

        // choose a suitably sized font
        g.setFont (20.0f, Font::bold);

        // and draw the text, centred in this component
        g.drawText (T("Hello World!"),
                    0, 0, getWidth(), getHeight(),
                    Justification::centred, false);
    }
};

//==============================================================================
/** This is the top-level window that we'll pop up. Inside it, we'll create and
    show a HelloWorldContentComponent component.
*/
class HelloWorldWindow  : public DocumentWindow
{
public:
    //==============================================================================
    HelloWorldWindow()
        : DocumentWindow (T("Hello World"),
                          Colours::lightgrey, 
                          DocumentWindow::allButtons, 
                          true)
    {
        setContentComponent (new HelloWorldContentComponent());

        setVisible (true);

        // centre the window on the desktop with this size
        centreWithSize (400, 200);
    }

    ~HelloWorldWindow()
    {
        // (the content component will be deleted automatically, so no need to do it here)
    }

    //==============================================================================
    void closeButtonPressed()
    {
        // When the user presses the close button, we'll tell the app to quit. This 
        // window will be deleted by the app object as it closes down.
        JUCEApplication::quit();
    }
};


//==============================================================================
/** This is the application object that is started up when Juce starts. It handles
    the initialisation and shutdown of the whole application.
*/
class JUCEHelloWorldApplication : public JUCEApplication
{
    /* Important! NEVER embed objects directly inside your JUCEApplication class! Use
       ONLY pointers to objects, which you should create during the initialise() method
       (NOT in the constructor!) and delete in the shutdown() method (NOT in the
       destructor!)

       This is because the application object gets created before Juce has been properly
       initialised, so any embedded objects would also get constructed too soon.
   */
    HelloWorldWindow* helloWorldWindow;

public:
    //==============================================================================
    JUCEHelloWorldApplication()
        : helloWorldWindow (0)
    {
        // NEVER do anything in here that could involve any Juce function being called
        // - leave all your startup tasks until the initialise() method.
    }

    ~JUCEHelloWorldApplication()
    {
        // Your shutdown() method should already have done all the things necessary to
        // clean up this app object, so you should never need to put anything in
        // the destructor.

        // Making any Juce calls in here could be very dangerous...
    }

    //==============================================================================
    void initialise (const String& commandLine)
    {
        // just create the main window...
        helloWorldWindow = new HelloWorldWindow();

        /*  ..and now return, which will fall into to the main event
            dispatch loop, and this will run until something calls
            JUCEAppliction::quit().

            In this case, JUCEAppliction::quit() will be called by the
            hello world window being clicked.
        */
    }

    void shutdown()
    {
        // clear up..

        if (helloWorldWindow != 0)
            delete helloWorldWindow;
    }

    //==============================================================================
    const String getApplicationName()
    {
        return T("Hello World for JUCE");
    }

    const String getApplicationVersion()
    {
        return T("1.0");
    }

    bool moreThanOneInstanceAllowed()
    {
        return true;
    }

    void anotherInstanceStarted (const String& commandLine)
    {
    }
};


//==============================================================================
// This macro creates the application's main() function..
START_JUCE_APPLICATION (JUCEHelloWorldApplication)
