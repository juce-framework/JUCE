/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

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
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        DialogsDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
struct MessageBoxOwnerComponent : public Component
{
    ScopedMessageBox messageBox;
};

//==============================================================================
class DemoBackgroundThread final : public ThreadWithProgressWindow
{
public:
    explicit DemoBackgroundThread (MessageBoxOwnerComponent& comp)
        : ThreadWithProgressWindow ("busy doing some important things...", true, true),
          owner (&comp)
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
        const String messageString (userPressedCancel ? "You pressed cancel!" : "Thread finished ok!");

        if (owner != nullptr)
        {
            owner->messageBox = AlertWindow::showScopedAsync (MessageBoxOptions()
                                                                  .withIconType (MessageBoxIconType::InfoIcon)
                                                                  .withTitle ("Progress window")
                                                                  .withMessage (messageString)
                                                                  .withButton ("OK"),
                                                              nullptr);
        }

        // ..and clean up by deleting our thread object..
        delete this;
    }

    Component::SafePointer<MessageBoxOwnerComponent> owner;
};


//==============================================================================
class DialogsDemo final : public MessageBoxOwnerComponent
{
public:
    enum DialogType
    {
        plainAlertWindow,
        warningAlertWindow,
        infoAlertWindow,
        questionAlertWindow,
        yesNoCancelAlertWindow,
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

        StringArray windowNames { "Plain Alert Window",
                                  "Alert Window With Warning Icon",
                                  "Alert Window With Info Icon",
                                  "Alert Window With Question Icon",
                                  "Yes No Cancel Alert Window",
                                  "Alert Window With Extra Components",
                                  "CalloutBox",
                                  "Thread With Progress Window",
                                  "'Load' File Browser",
                                  "'Load' File Browser With Image Preview",
                                  "'Choose Directory' File Browser",
                                  "'Save' File Browser",
                                  "Share Text",
                                  "Share Files",
                                  "Share Images" };

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

        RuntimePermissions::request (RuntimePermissions::readExternalStorage, [ptr = Component::SafePointer (this)] (bool granted)
        {
            if (granted || ptr == nullptr)
               return;

            ptr->messageBox = AlertWindow::showScopedAsync (MessageBoxOptions()
                                                                .withIconType (MessageBoxIconType::WarningIcon)
                                                                .withTitle ("Permissions warning")
                                                                .withMessage ("External storage access permission not granted, some files"
                                                                              " may be inaccessible.")
                                                                .withButton ("OK"),
                                                            nullptr);
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

    auto getAlertBoxResultChosen()
    {
        return [ptr = Component::SafePointer (this)] (int result)
        {
            if (ptr != nullptr)
                ptr->messageBox = AlertWindow::showScopedAsync (MessageBoxOptions()
                                                                    .withIconType (MessageBoxIconType::InfoIcon)
                                                                    .withTitle ("Alert Box")
                                                                    .withMessage ("Result code: " + String (result))
                                                                    .withButton ("OK"),
                                                                nullptr);
        };
    }

    auto getAsyncAlertBoxResultChosen()
    {
        return [ptr = Component::SafePointer (this)] (int result)
        {
            if (ptr == nullptr)
                return;

            auto& aw = *ptr->asyncAlertWindow;

            aw.exitModalState (result);
            aw.setVisible (false);

            if (result == 0)
            {
                ptr->getAlertBoxResultChosen() (result);
                return;
            }

            auto optionIndexChosen = aw.getComboBoxComponent ("option")->getSelectedItemIndex();
            auto text = aw.getTextEditorContents ("text");

            ptr->messageBox = AlertWindow::showScopedAsync (MessageBoxOptions()
                                                                .withIconType (MessageBoxIconType::InfoIcon)
                                                                .withTitle ("Alert Box")
                                                                .withMessage ("Result code: " + String (result) + newLine
                                                                              + "Option index chosen: " + String (optionIndexChosen) + newLine
                                                                              + "Text: " + text)
                                                                .withButton ("OK"),
                                                            nullptr);
        };
    }

    void showWindow (Component& button, DialogType type)
    {
        if (type >= plainAlertWindow && type <= questionAlertWindow)
        {
            MessageBoxIconType icon = MessageBoxIconType::NoIcon;

            if (type == warningAlertWindow)   icon = MessageBoxIconType::WarningIcon;
            if (type == infoAlertWindow)      icon = MessageBoxIconType::InfoIcon;
            if (type == questionAlertWindow)  icon = MessageBoxIconType::QuestionIcon;

            auto options = MessageBoxOptions::makeOptionsOk (icon,
                                                             "This is an AlertWindow",
                                                             "And this is the AlertWindow's message. "
                                                             "Blah blah blah blah blah blah blah blah blah blah blah blah blah.");
            messageBox = AlertWindow::showScopedAsync (options, nullptr);
        }
        else if (type == yesNoCancelAlertWindow)
        {
            auto options = MessageBoxOptions::makeOptionsYesNoCancel (MessageBoxIconType::QuestionIcon,
                                                                      "This is a yes/no/cancel AlertWindow",
                                                                      "And this is the AlertWindow's message. "
                                                                      "Blah blah blah blah blah blah blah blah blah blah blah blah blah.");
            messageBox = AlertWindow::showScopedAsync (options, getAlertBoxResultChosen());
        }
        else if (type == calloutBoxWindow)
        {
            auto colourSelector = std::make_unique<ColourSelector>();

            colourSelector->setName ("background");
            colourSelector->setCurrentColour (findColour (TextButton::buttonColourId));
            colourSelector->setColour (ColourSelector::backgroundColourId, Colours::transparentBlack);
            colourSelector->setSize (300, 400);

            CallOutBox::launchAsynchronously (std::move (colourSelector), button.getScreenBounds(), nullptr);
        }
        else if (type == extraComponentsAlertWindow)
        {
            asyncAlertWindow = std::make_unique<AlertWindow> ("AlertWindow demo..",
                                                              "This AlertWindow has a couple of extra components added to show how to add drop-down lists and text entry boxes.",
                                                              MessageBoxIconType::QuestionIcon);

            asyncAlertWindow->addTextEditor ("text", "enter some text here", "text field:");
            asyncAlertWindow->addComboBox ("option", { "option 1", "option 2", "option 3", "option 4" }, "some options");
            asyncAlertWindow->addButton ("OK",     1, KeyPress (KeyPress::returnKey, 0, 0));
            asyncAlertWindow->addButton ("Cancel", 0, KeyPress (KeyPress::escapeKey, 0, 0));

            asyncAlertWindow->enterModalState (true, ModalCallbackFunction::create (getAsyncAlertBoxResultChosen()));
        }
        else if (type == progressWindow)
        {
            // This will launch our ThreadWithProgressWindow in a modal state. (Our subclass
            // will take care of deleting the object when the task has finished)
            (new DemoBackgroundThread (*this))->launchThread();
        }
        else if (type >= loadChooser && type <= saveChooser)
        {
            auto useNativeVersion = nativeButton.getToggleState();

            if (type == loadChooser)
            {
                fc.reset (new FileChooser ("Choose a file to open...", File::getCurrentWorkingDirectory(),
                                           "*", useNativeVersion));

                fc->launchAsync (FileBrowserComponent::canSelectMultipleItems
                                 | FileBrowserComponent::openMode
                                 | FileBrowserComponent::canSelectFiles,
                                 [this] (const FileChooser& chooser)
                                 {
                                     String chosen;
                                     auto results = chooser.getURLResults();

                                     for (auto result : results)
                                         chosen << (result.isLocalFile() ? result.getLocalFile().getFullPathName()
                                                                         : result.toString (false)) << "\n";

                                     messageBox = AlertWindow::showScopedAsync (MessageBoxOptions()
                                                                                    .withIconType (MessageBoxIconType::InfoIcon)
                                                                                    .withTitle ("File Chooser...")
                                                                                    .withMessage ("You picked: " + chosen)
                                                                                    .withButton ("OK"),
                                                                                nullptr);
                                 });
            }
            else if (type == loadWithPreviewChooser)
            {
                imagePreview.setSize (200, 200);

                fc.reset (new FileChooser ("Choose an image to open...", File::getCurrentWorkingDirectory(),
                                           "*.jpg;*.jpeg;*.png;*.gif", useNativeVersion));

                fc->launchAsync (FileBrowserComponent::openMode
                                 | FileBrowserComponent::canSelectFiles
                                 | FileBrowserComponent::canSelectMultipleItems,
                                 [this] (const FileChooser& chooser)
                                 {
                                     String chosen;
                                     auto results = chooser.getURLResults();

                                     for (auto result : results)
                                         chosen << (result.isLocalFile() ? result.getLocalFile().getFullPathName()
                                                                         : result.toString (false)) << "\n";

                                     messageBox = AlertWindow::showScopedAsync (MessageBoxOptions()
                                                                                    .withIconType (MessageBoxIconType::InfoIcon)
                                                                                    .withTitle ("File Chooser...")
                                                                                    .withMessage ("You picked: " + chosen)
                                                                                    .withButton ("OK"),
                                                                                nullptr);
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
                        if (auto inStream = createAssetInputStream ("juce_icon.png"))
                            outStream.writeFromInputStream (*inStream, -1);
                }

                fc.reset (new FileChooser ("Choose a file to save...",
                                           File::getCurrentWorkingDirectory().getChildFile (fileToSave.getFileName()),
                                           "*",  useNativeVersion));

                fc->launchAsync (FileBrowserComponent::saveMode | FileBrowserComponent::canSelectFiles,
                                 [this, fileToSave] (const FileChooser& chooser)
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
                                             [[maybe_unused]] auto numWritten = wo->writeFromInputStream (*wi, -1);
                                             jassert (numWritten > 0);
                                             wo->flush();
                                         }
                                     }
                                   #endif

                                     messageBox = AlertWindow::showScopedAsync (MessageBoxOptions()
                                                                                    .withIconType (MessageBoxIconType::InfoIcon)
                                                                                    .withTitle ("File Chooser...")
                                                                                    .withMessage ("You picked: " + name)
                                                                                    .withButton ("OK"),
                                                                                nullptr);
                                 });
            }
            else if (type == directoryChooser)
            {
                fc.reset (new FileChooser ("Choose a directory...",
                                           File::getCurrentWorkingDirectory(),
                                           "*",
                                           useNativeVersion));

                fc->launchAsync (FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories,
                                 [this] (const FileChooser& chooser)
                                 {
                                     auto result = chooser.getURLResult();
                                     auto name = result.isLocalFile() ? result.getLocalFile().getFullPathName()
                                                                      : result.toString (true);

                                     messageBox = AlertWindow::showScopedAsync (MessageBoxOptions()
                                                                                    .withIconType (MessageBoxIconType::InfoIcon)
                                                                                    .withTitle ("File Chooser...")
                                                                                    .withMessage ("You picked: " + name)
                                                                                    .withButton ("OK"),
                                                                                nullptr);
                                 });
            }
        }
        else if (type == shareText)
        {
            messageBox = ContentSharer::shareTextScoped ("I love JUCE!", [ptr = Component::SafePointer (this)] (bool success, const String& error)
            {
                if (ptr == nullptr)
                    return;

                auto resultString = success ? String ("success") : ("failure\n (error: " + error + ")");

                ptr->messageBox = AlertWindow::showScopedAsync (MessageBoxOptions()
                                                                    .withIconType (MessageBoxIconType::InfoIcon)
                                                                    .withTitle ("Sharing Text Result")
                                                                    .withMessage ("Sharing text finished\nwith " + resultString)
                                                                    .withButton ("OK"),
                                                                nullptr);
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
                urls.add (URL (fileToSave));

                messageBox = ContentSharer::shareFilesScoped (urls, [ptr = Component::SafePointer (this)] (bool success, const String& error)
                {
                    if (ptr == nullptr)
                        return;

                    auto resultString = success ? String ("success") : ("failure\n (error: " + error + ")");

                    ptr->messageBox = AlertWindow::showScopedAsync (MessageBoxOptions()
                                                                        .withIconType (MessageBoxIconType::InfoIcon)
                                                                        .withTitle ("Sharing Files Result")
                                                                        .withMessage ("Sharing files finished\nwith " + resultString)
                                                                        .withButton ("OK"),
                                                                    nullptr);
                });

            }
        }
        else if (type == shareImage)
        {
            auto myImage = getImageFromAssets ("juce_icon.png");

            Image myImage2 (Image::RGB, 500, 500, true);

            {
                Graphics g (myImage2);
                g.setColour (Colours::green);
                ColourGradient gradient (Colours::yellow, 170, 170, Colours::cyan, 170, 20, true);
                g.setGradientFill (gradient);
                g.fillEllipse (20, 20, 300, 300);
            }

            Array<Image> images { myImage, myImage2 };

            messageBox = ContentSharer::shareImagesScoped (images, nullptr, [ptr = Component::SafePointer (this)] (bool success, const String& error)
            {
                if (ptr == nullptr)
                    return;

                String resultString = success ? String ("success")
                                              : ("failure\n (error: " + error + ")");

                ptr->messageBox = AlertWindow::showScopedAsync (MessageBoxOptions()
                                                                    .withIconType (MessageBoxIconType::InfoIcon)
                                                                    .withTitle ("Sharing Images Result")
                                                                    .withMessage ("Sharing images finished\nwith " + resultString)
                                                                    .withButton ("OK"),
                                                                nullptr);
            });
        }
    }

    ImagePreviewComponent imagePreview;
    std::unique_ptr<FileChooser> fc;
    std::unique_ptr<AlertWindow> asyncAlertWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DialogsDemo)
};
