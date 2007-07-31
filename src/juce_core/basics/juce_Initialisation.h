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

#ifndef __JUCE_INITIALISATION_JUCEHEADER__
#define __JUCE_INITIALISATION_JUCEHEADER__


//==============================================================================
/** Initialises Juce's GUI classes.

    If you're embedding Juce into an application that uses its own event-loop rather
    than using the START_JUCE_APPLICATION macro, call this function before making any
    Juce calls, to make sure things are initialised correctly.

    Note that if you're creating a Juce DLL for Windows, you may also need to call the
    PlatformUtilities::setCurrentModuleInstanceHandle() method.

    @see shutdownJuce_GUI(), initialiseJuce_NonGUI()
*/
void JUCE_PUBLIC_FUNCTION  initialiseJuce_GUI();

/** Clears up any static data being used by Juce's GUI classes.

    If you're embedding Juce into an application that uses its own event-loop rather
    than using the START_JUCE_APPLICATION macro, call this function in your shutdown
    code to clean up any juce objects that might be lying around.

    @see initialiseJuce_GUI(), initialiseJuce_NonGUI()
*/
void JUCE_PUBLIC_FUNCTION  shutdownJuce_GUI();


//==============================================================================
/** Initialises the core parts of Juce.

    If you're embedding Juce into either a command-line program, call this function
    at the start of your main() function to make sure that Juce is initialised correctly.

    Note that if you're creating a Juce DLL for Windows, you may also need to call the
    PlatformUtilities::setCurrentModuleInstanceHandle() method.

    @see shutdownJuce_NonGUI, initialiseJuce_GUI
*/
void JUCE_PUBLIC_FUNCTION  initialiseJuce_NonGUI();

/** Clears up any static data being used by Juce's non-gui core classes.

    If you're embedding Juce into either a command-line program, call this function
    at the end of your main() function if you want to make sure any Juce objects are
    cleaned up correctly.

    @see initialiseJuce_NonGUI, initialiseJuce_GUI
*/
void JUCE_PUBLIC_FUNCTION  shutdownJuce_NonGUI();



#endif   // __JUCE_INITIALISATION_JUCEHEADER__
