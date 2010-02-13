/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "../jucedemo_headers.h"

#if JUCE_QUICKTIME && ! JUCE_LINUX

//==============================================================================
// so that we can easily have two QT windows each with a file browser, wrap this up as a class..
class QuickTimeWindowWithFileBrowser  : public Component,
                                        public FilenameComponentListener
{
public:
    QuickTimeWindowWithFileBrowser()
    {
        addAndMakeVisible (qtComp = new QuickTimeMovieComponent());

        // and a file-chooser..
        addAndMakeVisible (fileChooser = new FilenameComponent (T("movie"),
                                                                File::nonexistent,
                                                                true, false, false,
                                                                T("*.*"),
                                                                String::empty,
                                                                T("(choose a video file to play)")));
        fileChooser->addListener (this);
        fileChooser->setBrowseButtonText (T("browse"));
    }

    ~QuickTimeWindowWithFileBrowser()
    {
        deleteAllChildren();
    }

    void resized()
    {
        qtComp->setBounds (0, 0, getWidth(), getHeight() - 30);
        fileChooser->setBounds (0, getHeight() - 24, getWidth(), 24);
    }

    void filenameComponentChanged (FilenameComponent*)
    {
        // this is called when the user changes the filename in the file chooser box
        if (qtComp->loadMovie (fileChooser->getCurrentFile(), true))
        {
            // loaded the file ok, so let's start it playing..

            qtComp->play();
        }
        else
        {
            AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                         T("Couldn't load the file!"),
                                         T("Sorry, QuickTime didn't manage to load that file!"));
        }
    }

private:
    QuickTimeMovieComponent* qtComp;
    FilenameComponent* fileChooser;
};


//==============================================================================
class QuickTimeDemo  : public Component
{
public:
    //==============================================================================
    QuickTimeDemo()
    {
        setName (T("QuickTime"));

        // add a movie component..
        addAndMakeVisible (qtComp1 = new QuickTimeWindowWithFileBrowser());
        addAndMakeVisible (qtComp2 = new QuickTimeWindowWithFileBrowser());
    }

    ~QuickTimeDemo()
    {
        deleteAllChildren();
    }

    void resized()
    {
        qtComp1->setBoundsRelative (0.05f, 0.05f, 0.425f, 0.9f);
        qtComp2->setBoundsRelative (0.525f, 0.05f, 0.425f, 0.9f);
    }

private:
    //==============================================================================
    QuickTimeWindowWithFileBrowser* qtComp1;
    QuickTimeWindowWithFileBrowser* qtComp2;
};


//==============================================================================
Component* createQuickTimeDemo()
{
    return new QuickTimeDemo();
}

#endif
