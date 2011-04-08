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

#include "jucedemo_headers.h"
#include "MainDemoWindow.h"


//==============================================================================
class JUCEDemoApplication : public JUCEApplication
{
public:
    //==============================================================================
    JUCEDemoApplication()
    {
    }

    ~JUCEDemoApplication()
    {
    }

    //==============================================================================
    void initialise (const String& /*commandLine*/)
    {
      #if JUCE_IOS || JUCE_ANDROID
        theMainWindow.setVisible (true);
        theMainWindow.setFullScreen (true);
      #else
        theMainWindow.centreWithSize (700, 600);
        theMainWindow.setVisible (true);
      #endif

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
        // This method is where your app should do any cleaning-up that's needed
        // before being shut down.
    }

    //==============================================================================
    const String getApplicationName()
    {
        // When you use the Jucer to auto-generate a project, it puts the project's name and version in
        // this constant, so we can use that here as our return value. Alternatively you can return
        // your own string here, of course.
        return ProjectInfo::projectName;
    }

    const String getApplicationVersion()
    {
        // When you use the Jucer to auto-generate a project, it puts the project's name and version in
        // this constant, so we can use that here as our return value. Alternatively you can return
        // your own string here, of course.
        return ProjectInfo::versionString;
    }

    bool moreThanOneInstanceAllowed()
    {
        return true;
    }

    void anotherInstanceStarted (const String& /*commandLine*/)
    {
        // This will get called if the user launches another copy of the app, but
        // there's nothing that the demo app needs to do here.
    }

private:
    // This is the main demo window component.
    MainDemoWindow theMainWindow;

    //==============================================================================
    // this little function just demonstrates a few system info calls
    static const String collectSomeSystemInfo()
    {
        String systemInfo;

        systemInfo
          << "Time and date: " << Time::getCurrentTime().toString (true, true)
          << "\nUser logon name: " << SystemStats::getLogonName()
          << "\nFull user name: " << SystemStats::getFullUserName()
          << "\nHost name: " << SystemStats::getComputerName()
          << "\nOperating system: " << SystemStats::getOperatingSystemName()
          << "\nCPU vendor: " << SystemStats::getCpuVendor()
          << "\nCPU speed: " << SystemStats::getCpuSpeedInMegaherz() << "MHz"
          << "\nNumber of CPUs: " << SystemStats::getNumCpus()
          << "\nCPU has MMX: " << (SystemStats::hasMMX() ? "yes" : "no")
          << "\nCPU has SSE: " << (SystemStats::hasSSE() ? "yes" : "no")
          << "\nCPU has SSE2: " << (SystemStats::hasSSE2() ? "yes" : "no")
          << "\nCPU has 3DNOW: " << (SystemStats::has3DNow() ? "yes" : "no")
          << "\nMemory size: " << SystemStats::getMemorySizeInMegabytes() << "MB"
          << "\nFound network card MAC addresses: " << getMacAddressList()
          << "\nCurrent executable file: " << File::getSpecialLocation (File::currentExecutableFile).getFullPathName()
          << "\nCurrent application file: " << File::getSpecialLocation (File::currentApplicationFile).getFullPathName()
          << "\nCurrent working directory: " << File::getCurrentWorkingDirectory().getFullPathName()
          << "\nUser home directory: " << File::getSpecialLocation (File::userHomeDirectory).getFullPathName()
          << "\nUser documents directory: " << File::getSpecialLocation (File::userDocumentsDirectory).getFullPathName()
          << "\nUser application data directory: " << File::getSpecialLocation (File::userApplicationDataDirectory).getFullPathName()
          << "\nCommon application data directory: " << File::getSpecialLocation (File::commonApplicationDataDirectory).getFullPathName()
          << "\nTemp directory: " << File::getSpecialLocation (File::tempDirectory).getFullPathName()
          << "\n\n";

        return systemInfo;
    }

    static const String getMacAddressList()
    {
        Array <MACAddress> macAddresses;
        MACAddress::findAllAddresses (macAddresses);

        StringArray addressStrings;
        for (int i = 0; i < macAddresses.size(); ++i)
            addressStrings.add (macAddresses[i].toString());

        return addressStrings.joinIntoString (", ");
    }
};


//==============================================================================
/*
    This macro creates the application's main() function..
*/
START_JUCE_APPLICATION (JUCEDemoApplication)
