/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

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
        : fileChooser ("movie", File::nonexistent, true, false, false,
                       "*", String::empty, "(choose a video file to play)")
    {
        addAndMakeVisible (&qtComp);

        addAndMakeVisible (&fileChooser);
        fileChooser.addListener (this);
        fileChooser.setBrowseButtonText ("browse");
    }

    ~QuickTimeWindowWithFileBrowser()
    {
    }

    void resized()
    {
        qtComp.setBounds (0, 0, getWidth(), getHeight() - 30);
        fileChooser.setBounds (0, getHeight() - 24, getWidth(), 24);
    }

    void filenameComponentChanged (FilenameComponent*)
    {
        // this is called when the user changes the filename in the file chooser box
        if (qtComp.loadMovie (fileChooser.getCurrentFile(), true))
        {
            // loaded the file ok, so let's start it playing..

            qtComp.play();
        }
        else
        {
            AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                         "Couldn't load the file!",
                                         "Sorry, QuickTime didn't manage to load that file!");
        }
    }

private:
    QuickTimeMovieComponent qtComp;
    FilenameComponent fileChooser;
};


//==============================================================================
class QuickTimeDemo  : public Component
{
public:
    //==============================================================================
    QuickTimeDemo()
    {
        setName ("QuickTime");

        // add a movie component..
        addAndMakeVisible (&qtComp1);
        addAndMakeVisible (&qtComp2);
    }

    ~QuickTimeDemo()
    {
        qtComp1.setVisible (false);
        qtComp2.setVisible (false);
    }

    void resized()
    {
        qtComp1.setBoundsRelative (0.05f, 0.05f, 0.425f, 0.9f);
        qtComp2.setBoundsRelative (0.525f, 0.05f, 0.425f, 0.9f);
    }

private:
    //==============================================================================
    QuickTimeWindowWithFileBrowser qtComp1, qtComp2;
};


//==============================================================================
Component* createQuickTimeDemo()
{
    return new QuickTimeDemo();
}

#endif
