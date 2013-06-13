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

#ifndef __MAINDEMOWINDOW_JUCEHEADER__
#define __MAINDEMOWINDOW_JUCEHEADER__


//==============================================================================
class MainDemoWindow  : public DocumentWindow
{
public:
    //==============================================================================
    MainDemoWindow();
    ~MainDemoWindow();

    //==============================================================================
    // called when the close button is pressed or esc is pushed
    void closeButtonPressed();

    // the command manager object used to dispatch command events
    ApplicationCommandManager commandManager;

private:
    ScopedPointer<Component> taskbarIcon;
};


#endif   // __MAINDEMOWINDOW_JUCEHEADER__
