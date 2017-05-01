/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../JuceDemoHeader.h"

//==============================================================================
class DemoBackgroundThread  : public ThreadWithProgressWindow
{
public:
    DemoBackgroundThread()
        : ThreadWithProgressWindow ("busy doing some important things...", true, true)
    {
        setStatusMessage ("Getting ready...");
    }

    void run() override
    {
        setProgress (-1.0); // setting a value beyond the range 0 -> 1 will show a spinning bar..
        setStatusMessage ("Preparing to do some stuff...");
        wait (2000);

        const int thingsToDo = 10;

        for (int i = 0; i < thingsToDo; ++i)
        {
            // must check this as often as possible, because this is
            // how we know if the user's pressed 'cancel'
            if (threadShouldExit())
                return;

            // this will update the progress bar on the dialog box
            setProgress (i / (double) thingsToDo);

            setStatusMessage (String (thingsToDo - i) + " things left to do...");

            wait (500);
        }

        setProgress (-1.0); // setting a value beyond the range 0 -> 1 will show a spinning bar..
        setStatusMessage ("Finishing off the last few bits and pieces!");
        wait (2000);
    }

    // This method gets called on the message thread once our thread has finished..
    void threadComplete (bool userPressedCancel) override
    {
        if (userPressedCancel)
        {
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              "Progress window",
                                              "You pressed cancel!");
        }
        else
        {
            // thread finished normally..
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              "Progress window",
                                              "Thread finished ok!");
        }

        // ..and clean up by deleting our thread object..
        delete this;
    }
};


//==============================================================================
class DialogsDemo  : public Component,
                     private Button::Listener
{
public:
    enum DialogType
    {
        plainAlertWindow,
        warningAlertWindow,
        infoAlertWindow,
        questionAlertWindow,
        okCancelAlertWindow,
        extraComponentsAlertWindow,
        calloutBoxWindow,
        progressWindow,
        loadChooser,
        loadWithPreviewChooser,
        directoryChooser,
        saveChooser,
        numDialogs
    };

    DialogsDemo()
    {
        setOpaque (true);

        addAndMakeVisible (nativeButton);
        nativeButton.setButtonText ("Use Native Windows");
        nativeButton.addListener (this);

        static const char* windowNames[] =
        {
            "Plain Alert Window",
            "Alert Window With Warning Icon",
            "Alert Window With Info Icon",
            "Alert Window With Question Icon",
            "OK Cancel Alert Window",
            "Alert Window With Extra Components",
            "CalloutBox",
            "Thread With Progress Window",
            "'Load' File Browser",
            "'Load' File Browser With Image Preview",
            "'Choose Directory' File Browser",
            "'Save' File Browser"
        };

        // warn in case we add any windows
        jassert (numElementsInArray (windowNames) == numDialogs);

        for (int i = 0; i < numDialogs; ++i)
        {
            TextButton* newButton = new TextButton();
            windowButtons.add (newButton);
            addAndMakeVisible (newButton);
            newButton->setButtonText (windowNames[i]);
            newButton->addListener (this);
        }
    }

    ~DialogsDemo()
    {
        nativeButton.removeListener (this);

        for (int i = windowButtons.size(); --i >= 0;)
            if (TextButton* button = windowButtons.getUnchecked (i))
                button->removeListener (this);
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

    void resized() override
    {
        Rectangle<int> area (getLocalBounds().reduced (5, 15));
        Rectangle<int> topRow;

        for (int i = 0; i < windowButtons.size(); ++i)
        {
            if (topRow.getWidth() < 10 || i == loadChooser)
                topRow = area.removeFromTop (26);

            if (i == progressWindow)
                area.removeFromTop (20);

            windowButtons.getUnchecked (i)
                ->setBounds (topRow.removeFromLeft (area.getWidth() / 2).reduced (4, 2));
        }

        area.removeFromTop (15);
        nativeButton.setBounds (area.removeFromTop (24));
    }

private:
    OwnedArray<TextButton> windowButtons;
    ToggleButton nativeButton;

    static void alertBoxResultChosen (int result, DialogsDemo*)
    {
        AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon,
                                          "Alert Box",
                                          "Result code: " + String (result));
    }

    void showWindow (Component& button, DialogType type)
    {
        if (type >= plainAlertWindow && type <= questionAlertWindow)
        {
            AlertWindow::AlertIconType icon = AlertWindow::NoIcon;

            switch (type)
            {
                case warningAlertWindow:    icon = AlertWindow::WarningIcon;    break;
                case infoAlertWindow:       icon = AlertWindow::InfoIcon;       break;
                case questionAlertWindow:   icon = AlertWindow::QuestionIcon;   break;
                default: break;
            }

            AlertWindow::showMessageBoxAsync (icon,
                                              "This is an AlertWindow",
                                              "And this is the AlertWindow's message. Blah blah blah blah blah blah blah blah blah blah blah blah blah.",
                                              "OK");
        }
        else if (type == okCancelAlertWindow)
        {
            AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon,
                                          "This is an ok/cancel AlertWindow",
                                          "And this is the AlertWindow's message. Blah blah blah blah blah blah blah blah blah blah blah blah blah.",
                                          String(),
                                          String(),
                                          0,
                                          ModalCallbackFunction::forComponent (alertBoxResultChosen, this));
        }
        else if (type == calloutBoxWindow)
        {
            ColourSelector* colourSelector = new ColourSelector();
            colourSelector->setName ("background");
            colourSelector->setCurrentColour (findColour (TextButton::buttonColourId));
            colourSelector->setColour (ColourSelector::backgroundColourId, Colours::transparentBlack);
            colourSelector->setSize (300, 400);

            CallOutBox::launchAsynchronously (colourSelector, button.getScreenBounds(), nullptr);
        }
        else if (type == extraComponentsAlertWindow)
        {
           #if JUCE_MODAL_LOOPS_PERMITTED
            AlertWindow w ("AlertWindow demo..",
                           "This AlertWindow has a couple of extra components added to show how to add drop-down lists and text entry boxes.",
                           AlertWindow::QuestionIcon);

            w.addTextEditor ("text", "enter some text here", "text field:");

            const char* options[] = { "option 1", "option 2", "option 3", "option 4", nullptr };
            w.addComboBox ("option", StringArray (options), "some options");

            w.addButton ("OK",     1, KeyPress (KeyPress::returnKey, 0, 0));
            w.addButton ("Cancel", 0, KeyPress (KeyPress::escapeKey, 0, 0));

            if (w.runModalLoop() != 0) // is they picked 'ok'
            {
                // this is the item they chose in the drop-down list..
                const int optionIndexChosen = w.getComboBoxComponent ("option")->getSelectedItemIndex();
                ignoreUnused (optionIndexChosen);

                // this is the text they entered..
                String text = w.getTextEditorContents ("text");
            }
           #endif
        }
        else if (type == progressWindow)
        {
            // This will launch our ThreadWithProgressWindow in a modal state. (Our subclass
            // will take care of deleting the object when the task has finished)
            (new DemoBackgroundThread())->launchThread();
        }
        else if (type >= loadChooser && type <= saveChooser)
        {
           #if JUCE_MODAL_LOOPS_PERMITTED
            const bool useNativeVersion = nativeButton.getToggleState();

            if (type == loadChooser)
            {
                FileChooser fc ("Choose a file to open...",
                                File::getCurrentWorkingDirectory(),
                                "*",
                                useNativeVersion);

                if (fc.browseForMultipleFilesToOpen())
                {
                    String chosen;
                    for (int i = 0; i < fc.getResults().size(); ++i)
                        chosen << fc.getResults().getReference(i).getFullPathName() << "\n";

                    AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon,
                                                      "File Chooser...",
                                                      "You picked: " + chosen);
                }
            }
            else if (type == loadWithPreviewChooser)
            {
                ImagePreviewComponent imagePreview;
                imagePreview.setSize (200, 200);

                FileChooser fc ("Choose an image to open...",
                                File::getSpecialLocation (File::userPicturesDirectory),
                                "*.jpg;*.jpeg;*.png;*.gif",
                                useNativeVersion);

                if (fc.browseForMultipleFilesToOpen (&imagePreview))
                {
                    String chosen;
                    for (int i = 0; i < fc.getResults().size(); ++i)
                        chosen << fc.getResults().getReference (i).getFullPathName() << "\n";

                    AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon,
                                                      "File Chooser...",
                                                      "You picked: " + chosen);
                }
            }
            else if (type == saveChooser)
            {
                FileChooser fc ("Choose a file to save...",
                                File::getCurrentWorkingDirectory(),
                                "*",
                                useNativeVersion);

                if (fc.browseForFileToSave (true))
                {
                    File chosenFile = fc.getResult();

                    AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon,
                                                      "File Chooser...",
                                                      "You picked: " + chosenFile.getFullPathName());
                }
            }
            else if (type == directoryChooser)
            {
                FileChooser fc ("Choose a directory...",
                                File::getCurrentWorkingDirectory(),
                                "*",
                                useNativeVersion);

                if (fc.browseForDirectory())
                {
                    File chosenDirectory = fc.getResult();

                    AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon,
                                                      "File Chooser...",
                                                      "You picked: " + chosenDirectory.getFullPathName());
                }
            }
           #endif
        }
    }

    void buttonClicked (Button* button) override
    {
        if (button == &nativeButton)
        {
            getLookAndFeel().setUsingNativeAlertWindows (nativeButton.getToggleState());

            return;
        }

        for (int i = windowButtons.size(); --i >= 0;)
            if (button == windowButtons.getUnchecked (i))
                return showWindow (*button, static_cast<DialogType> (i));
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DialogsDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<DialogsDemo> demo ("10 Components: Dialog Boxes");
