/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic startup code for a JUCE application.

  ==============================================================================
*/

%%app_headers%%

//==============================================================================
class %%app_class_name%%  : public JUCEApplication
{
public:
    //==============================================================================
    %%app_class_name%%() {}

    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override       { return %%allow_more_than_one_instance%%; }

    //==============================================================================
    void initialise (const String& commandLine) override
    {
        // Add your application's initialisation code here..
    }

    void shutdown() override
    {
        // Add your application's shutdown code here..
    }

    //==============================================================================
    void systemRequestedQuit() override
    {
        // This is called when the app is being asked to quit: you can ignore this
        // request and let the app carry on running, or call quit() to allow the app to close.
        quit();
    }

    void anotherInstanceStarted (const String& commandLine) override
    {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.
    }
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (%%app_class_name%%)
