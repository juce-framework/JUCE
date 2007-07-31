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

#include "jucedemo_headers.h"
#include "MainDemoWindow.h"


//==============================================================================
class JUCEDemoApplication : public JUCEApplication
{
    /* Important! NEVER embed objects directly inside your JUCEApplication class! Use
       ONLY pointers to objects, which you should create during the initialise() method
       (NOT in the constructor!) and delete in the shutdown() method (NOT in the
       destructor!)

       This is because the application object gets created before Juce has been properly
       initialised, so any embedded objects would also get constructed too soon.
   */
    MainDemoWindow* theMainWindow;

public:
    //==============================================================================
    JUCEDemoApplication()
        : theMainWindow (0)
    {
        // NEVER do anything in here that could involve any Juce function being called
        // - leave all your startup tasks until the initialise() method.
    }

    ~JUCEDemoApplication()
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
        theMainWindow = new MainDemoWindow();
        theMainWindow->centreWithSize (700, 600);
        theMainWindow->setVisible (true);

        // this little function just demonstrates a few system info calls
        Logger::outputDebugString (collectSomeSystemInfo());

        /*  on return from this method, the app will go into its the main event
            dispatch loop, and this will run until something calls
            JUCEAppliction::quit().

            In this case, JUCEAppliction::quit() will be called by the
            demo window when the user clicks on its close button.
        */
    }

    void shutdown()
    {
        delete theMainWindow;
        theMainWindow = 0;
    }

    //==============================================================================
    const String getApplicationName()
    {
        return T("JUCE Demo");
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
        // This will get called if the user launches another copy of the app, but
        // there's nothing that the demo app needs to do here.
    }

private:
    //==============================================================================
    // this little function just demonstrates a few system info calls
    static const String collectSomeSystemInfo()
    {
        String systemInfo;

        systemInfo
          << T("Time and date: ") << Time::getCurrentTime().toString (true, true)
          << T("\nOperating system: ") << SystemStats::getOperatingSystemName()
          << T("\nCPU vendor: ") << SystemStats::getCpuVendor()
          << T("\nCPU speed: ") << SystemStats::getCpuSpeedInMegaherz() << T("MHz\n")
          << T("\nNumber of CPUs: ") << SystemStats::getNumCpus()
          << T("\nCPU has MMX: ") << (SystemStats::hasMMX() ? T("yes") : T("no"))
          << T("\nCPU has SSE: ") << (SystemStats::hasSSE() ? T("yes") : T("no"))
          << T("\nCPU has SSE2: ") << (SystemStats::hasSSE2() ? T("yes") : T("no"))
          << T("\nCPU has 3DNOW: ") << (SystemStats::has3DNow() ? T("yes") : T("no"))
          << T("\nMemory size: ") << SystemStats::getMemorySizeInMegabytes() << T("MB\n");

        int64 macAddresses[8];
        const int numAddresses = SystemStats::getMACAddresses (macAddresses, 8);

        for (int i = 0; i < numAddresses; ++i)
        {
            systemInfo
              << T("Found network card MAC address: ")
              << String::formatted (T("%02x-%02x-%02x-%02x-%02x-%02x\n"),
                                    0xff & (int) (macAddresses [i] >> 40),
                                    0xff & (int) (macAddresses [i] >> 32),
                                    0xff & (int) (macAddresses [i] >> 24),
                                    0xff & (int) (macAddresses [i] >> 16),
                                    0xff & (int) (macAddresses [i] >> 8),
                                    0xff & (int) macAddresses [i]);
        }

        systemInfo
          << T("Current executable file: ")
          << File::getSpecialLocation (File::currentExecutableFile).getFullPathName()
          << T("\nCurrent application file: ")
          << File::getSpecialLocation (File::currentApplicationFile).getFullPathName()
          << T("\nUser home directory: ")
          << File::getSpecialLocation (File::userHomeDirectory).getFullPathName()
          << T("\nUser documents directory: ")
          << File::getSpecialLocation (File::userDocumentsDirectory).getFullPathName()
          << T("\nUser application data directory: ")
          << File::getSpecialLocation (File::userApplicationDataDirectory).getFullPathName()
          << T("\nCommon application data directory: ")
          << File::getSpecialLocation (File::commonApplicationDataDirectory).getFullPathName()
          << T("\nTemp directory: ")
          << File::getSpecialLocation (File::tempDirectory).getFullPathName()
          << T("\n\n");

        return systemInfo;
    }
};


//==============================================================================
/*
    This macro creates the application's main() function..
*/
START_JUCE_APPLICATION (JUCEDemoApplication)
