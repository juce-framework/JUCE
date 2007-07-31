/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "jucer_Headers.h"
#include "ui/jucer_MainWindow.h"

ApplicationCommandManager* commandManager = 0;


//==============================================================================
class JucerApplication : public JUCEApplication
{
    MainWindow* theMainWindow;

public:
    //==============================================================================
    JucerApplication()
        : theMainWindow (0)
    {
    }

    ~JucerApplication()
    {
    }

    //==============================================================================
    void initialise (const String& commandLine)
    {
        commandManager = new ApplicationCommandManager();

        theMainWindow = new MainWindow();
        theMainWindow->setVisible (true);

        ImageCache::setCacheTimeout (30 * 1000);

        if (commandLine.trim().isNotEmpty()
              && ! commandLine.trim().startsWithChar (T('-')))
            anotherInstanceStarted (commandLine);
    }

    void shutdown()
    {
        delete theMainWindow;
        theMainWindow = 0;

        deleteAndZero (commandManager);
    }

    //==============================================================================
    void systemRequestedQuit()
    {
        if (theMainWindow == 0 || theMainWindow->closeAllDocuments())
        {
            deleteAndZero (theMainWindow);

            StoredSettings::deleteInstance();

            quit (false);
        }
    }

    //==============================================================================
    const String getApplicationName()
    {
        return T("The Jucer");
    }

    const String getApplicationVersion()
    {
        return String (JUCER_MAJOR_VERSION) + T(".") + String (JUCER_MINOR_VERSION);
    }

    bool moreThanOneInstanceAllowed()
    {
#ifndef JUCE_LINUX
        return false;
#else
        return true; //xxx should be false but doesn't work on linux..
#endif
    }

    void anotherInstanceStarted (const String& commandLine)
    {
        if (theMainWindow != 0)
            theMainWindow->openFile (commandLine.unquoted());
    }
};


START_JUCE_APPLICATION(JucerApplication)
