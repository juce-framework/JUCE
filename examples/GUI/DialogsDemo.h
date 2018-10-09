/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2017 - ROLI Ltd.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             DialogsDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Displays different types of dialog windows.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2017, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        DialogsDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

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

        int thingsToDo = 10;

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
class DialogsDemo  : public Component
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
        shareText,
        shareFile,
        shareImage,
        numDialogs
    };

    DialogsDemo()
    {
        setOpaque (true);

        addAndMakeVisible (nativeButton);
        nativeButton.setButtonText ("Use Native Windows");
        nativeButton.onClick = [this] { getLookAndFeel().setUsingNativeAlertWindows (nativeButton.getToggleState()); };

        StringArray windowNames { "Plain Alert Window", "Alert Window With Warning Icon", "Alert Window With Info Icon", "Alert Window With Question Icon",
                                  "OK Cancel Alert Window", "Alert Window With Extra Components", "CalloutBox", "Thread With Progress Window",
                                  "'Load' File Browser", "'Load' File Browser With Image Preview", "'Choose Directory' File Browser", "'Save' File Browser",
                                  "Share Text", "Share Files", "Share Images"  };

        // warn in case we add any windows
        jassert (windowNames.size() == numDialogs);

        for (auto windowName : windowNames)
        {
            auto* newButton = new TextButton();

            addAndMakeVisible (windowButtons.add (newButton));
            newButton->setButtonText (windowName);

            auto index = windowNames.indexOf (windowName);
            newButton->onClick = [this, index, newButton] { showWindow (*newButton, static_cast<DialogType> (index)); };
        }

        setSize (500, 500);

        RuntimePermissions::request (RuntimePermissions::readExternalStorage,
                                     [] (bool granted)
                                     {
                                         if (! granted)
                                         {
                                             AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                                                               "Permissions warning",
                                                                               "External storage access permission not granted, some files"
                                                                               " may be inaccessible.");
                                         }
                                     });
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced (5, 15);
        Rectangle<int> topRow;

        for (auto* b : windowButtons)
        {
            auto index = windowButtons.indexOf (b);

            if (topRow.getWidth() < 10 || index == loadChooser)
                topRow = area.removeFromTop (26);

            if (index == progressWindow)
                area.removeFromTop (20);

            b->setBounds (topRow.removeFromLeft (area.getWidth() / 2).reduced (4, 2));
        }

        area.removeFromTop (15);
        nativeButton.setBounds (area.removeFromTop (24));
    }

private:
    OwnedArray<TextButton> windowButtons;
    ToggleButton nativeButton;

    static void alertBoxResultChosen (int result, DialogsDemo*)
    {
        AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon, "Alert Box",
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

            AlertWindow::showMessageBoxAsync (icon, "This is an AlertWindow",
                                              "And this is the AlertWindow's message. Blah blah blah blah blah blah blah blah blah blah blah blah blah.",
                                              "OK");
        }
        else if (type == okCancelAlertWindow)
        {
            AlertWindow::showOkCancelBox (AlertWindow::QuestionIcon, "This is an ok/cancel AlertWindow",
                                          "And this is the AlertWindow's message. Blah blah blah blah blah blah blah blah blah blah blah blah blah.",
                                          {}, {}, {},
                                          ModalCallbackFunction::forComponent (alertBoxResultChosen, this));
        }
        else if (type == calloutBoxWindow)
        {
            auto* colourSelector = new ColourSelector();

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
            w.addComboBox ("option", { "option 1", "option 2", "option 3", "option 4" }, "some options");
            w.addButton ("OK",     1, KeyPress (KeyPress::returnKey, 0, 0));
            w.addButton ("Cancel", 0, KeyPress (KeyPress::escapeKey, 0, 0));

            if (w.runModalLoop() != 0) // is they picked 'ok'
            {
                // this is the item they chose in the drop-down list..
                auto optionIndexChosen = w.getComboBoxComponent ("option")->getSelectedItemIndex();
                ignoreUnused (optionIndexChosen);

                // this is the text they entered..
                auto text = w.getTextEditorContents ("text");
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
            auto useNativeVersion = nativeButton.getToggleState();

            if (type == loadChooser)
            {
                fc.reset (new FileChooser ("Choose a file to open...", File::getCurrentWorkingDirectory(),
                                           "*", useNativeVersion));

                fc->launchAsync (FileBrowserComponent::canSelectMultipleItems | FileBrowserComponent::openMode
                                     | FileBrowserComponent::canSelectFiles,
                                 [] (const FileChooser& chooser)
                                 {
                                     String chosen;
                                     auto results = chooser.getURLResults();

                                     for (auto result : results)
                                         chosen << (result.isLocalFile() ? result.getLocalFile().getFullPathName()
                                                                         : result.toString (false)) << "\n";

                                     AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon,
                                                                       "File Chooser...",
                                                                       "You picked: " + chosen);
                                 });
            }
            else if (type == loadWithPreviewChooser)
            {
                imagePreview.setSize (200, 200);

                fc.reset (new FileChooser ("Choose an image to open...", File::getCurrentWorkingDirectory(),
                                           "*.jpg;*.jpeg;*.png;*.gif", useNativeVersion));

                fc->launchAsync (FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles
                                    | FileBrowserComponent::canSelectMultipleItems,
                                 [] (const FileChooser& chooser)
                                 {
                                     String chosen;
                                     auto results = chooser.getURLResults();

                                     for (auto result : results)
                                         chosen << (result.isLocalFile() ? result.getLocalFile().getFullPathName()
                                                                         : result.toString (false)) << "\n";

                                     AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon,
                                                                       "File Chooser...",
                                                                       "You picked: " + chosen);
                                 },
                                 &imagePreview);
            }
            else if (type == saveChooser)
            {
                auto fileToSave = File::createTempFile ("saveChooserDemo");

                if (fileToSave.createDirectory().wasOk())
                {
                    fileToSave = fileToSave.getChildFile ("JUCE.png");
                    fileToSave.deleteFile();

                    FileOutputStream outStream (fileToSave);

                    if (outStream.openedOk())
                        if (auto inStream = std::unique_ptr<InputStream> (createAssetInputStream ("juce_icon.png")))
                            outStream.writeFromInputStream (*inStream, -1);
                }

                fc.reset (new FileChooser ("Choose a file to save...",
                                           File::getCurrentWorkingDirectory().getChildFile (fileToSave.getFileName()),
                                           "*",  useNativeVersion));

                fc->launchAsync (FileBrowserComponent::saveMode | FileBrowserComponent::canSelectFiles,
                                 [fileToSave] (const FileChooser& chooser)
                                 {
                                     auto result = chooser.getURLResult();
                                     auto name = result.isEmpty() ? String()
                                                                  : (result.isLocalFile() ? result.getLocalFile().getFullPathName()
                                                                                          : result.toString (true));

                                     // Android and iOS file choosers will create placeholder files for chosen
                                     // paths, so we may as well write into those files.
                                   #if JUCE_ANDROID || JUCE_IOS
                                     if (! result.isEmpty())
                                     {
                                         std::unique_ptr<InputStream>  wi (fileToSave.createInputStream());
                                         std::unique_ptr<OutputStream> wo (result.createOutputStream());

                                         if (wi.get() != nullptr && wo.get() != nullptr)
                                         {
                                             auto numWritten = wo->writeFromInputStream (*wi, -1);
                                             jassert (numWritten > 0);
                                             ignoreUnused (numWritten);
                                             wo->flush();
                                         }
                                     }
                                   #endif

                                     AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon,
                                                                       "File Chooser...",
                                                                       "You picked: " + name);
                                 });
            }
            else if (type == directoryChooser)
            {
                fc.reset (new FileChooser ("Choose a directory...",
                                           File::getCurrentWorkingDirectory(),
                                           "*",
                                           useNativeVersion));

                fc->launchAsync (FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories,
                                 [] (const FileChooser& chooser)
                                 {
                                     auto result = chooser.getURLResult();
                                     auto name = result.isLocalFile() ? result.getLocalFile().getFullPathName()
                                                                      : result.toString (true);

                                     AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon,
                                                                       "File Chooser...",
                                                                       "You picked: " + name);
                                 });
            }
        }
        else if (type == shareText)
        {
            ContentSharer::getInstance()->shareText ("I love JUCE!",
                                                     [] (bool success, const String& error)
                {
                    auto resultString = success ? String ("success") : ("failure\n (error: " + error + ")");

                    AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon, "Sharing Text Result",
                                                      "Sharing text finished\nwith " + resultString);
                });
        }
        else if (type == shareFile)
        {
            File fileToSave = File::createTempFile ("DialogsDemoSharingTest");

            if (fileToSave.createDirectory().wasOk())
            {
                fileToSave = fileToSave.getChildFile ("SharingDemoFile.txt");
                fileToSave.replaceWithText ("Make it fast!");

                Array<URL> urls;
                urls.add ({ fileToSave.getFullPathName() });

                ContentSharer::getInstance()->shareFiles (urls,
                    [] (bool success, const String& error)
                    {
                        auto resultString = success ? String ("success") : ("failure\n (error: " + error + ")");

                        AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon,
                                                          "Sharing Files Result",
                                                          "Sharing files finished\nwith " + resultString);
                    });

            }
        }
        else if (type == shareImage)
        {
            auto myImage = getImageFromAssets ("juce_icon.png");

            Image myImage2 (Image::RGB, 500, 500, true);
            Graphics g (myImage2);
            g.setColour (Colours::green);
            ColourGradient gradient (Colours::yellow, 170, 170, Colours::cyan, 170, 20, true);
            g.setGradientFill (gradient);
            g.fillEllipse (20, 20, 300, 300);

            Array<Image> images { myImage, myImage2 };

            ContentSharer::getInstance()->shareImages (images,
                                                       [] (bool success, const String& error)
                                                       {
                                                           String resultString = success ? String ("success")
                                                                                         : ("failure\n (error: " + error + ")");

                                                           AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon, "Sharing Images Result",
                                                                                             "Sharing images finished\nwith " + resultString);
                                                       });
        }
    }

    ImagePreviewComponent imagePreview;
    std::unique_ptr<FileChooser> fc;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DialogsDemo)
};
