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

// (This file gets included by juce_mac_NativeCode.mm, rather than being
// compiled on its own).
#ifdef JUCE_INCLUDED_FILE


//==============================================================================
ScopedAutoReleasePool::ScopedAutoReleasePool()
{
    pool = [[NSAutoreleasePool alloc] init];
}

ScopedAutoReleasePool::~ScopedAutoReleasePool()
{
    [((NSAutoreleasePool*) pool) release];
}

//==============================================================================
void PlatformUtilities::beep()
{
    //xxx
    //AudioServicesPlaySystemSound ();
}

//==============================================================================
void PlatformUtilities::addItemToDock (const File& file)
{
}


//==============================================================================
#if ! JUCE_ONLY_BUILD_CORE_LIBRARY

bool AlertWindow::showNativeDialogBox (const String& title,
                                       const String& bodyText,
                                       bool isOkCancel)
{
    const ScopedAutoReleasePool pool;

    UIAlertView *alert = [[[UIAlertView alloc] initWithTitle: juceStringToNS (title)
                                                     message: juceStringToNS (title)
                                                    delegate: nil
                                           cancelButtonTitle: @"OK"
                                           otherButtonTitles: (isOkCancel ? @"Cancel" : nil), nil] autorelease];
    alert.cancelButtonIndex = alert.firstOtherButtonIndex;
    [alert show];
    
    // xxx need to use a delegate to find which button was clicked
    return false;
}

//==============================================================================
bool DragAndDropContainer::performExternalDragDropOfFiles (const StringArray& files, const bool canMoveFiles)
{
    jassertfalse    // not implemented!
    return false;
}

bool DragAndDropContainer::performExternalDragDropOfText (const String& text)
{
    jassertfalse    // not implemented!
    return false;
}

//==============================================================================
bool Desktop::canUseSemiTransparentWindows() throw()
{
    return true;
}

void Desktop::getMousePosition (int& x, int& y) throw()
{
    x = 0;
    y = 0;
}

void Desktop::setMousePosition (int x, int y) throw()
{
}

//==============================================================================
void Desktop::setScreenSaverEnabled (const bool isEnabled) throw()
{
	[[UIApplication sharedApplication] setIdleTimerDisabled: ! isEnabled];
}

bool Desktop::isScreenSaverEnabled() throw()
{
    return ! [[UIApplication sharedApplication] isIdleTimerDisabled];
}


//==============================================================================
void juce_updateMultiMonitorInfo (Array <Rectangle>& monitorCoords, const bool clipToWorkArea) throw()
{
    const ScopedAutoReleasePool pool;
    monitorCoords.clear();

    CGRect r = clipToWorkArea ? [[UIScreen mainScreen] applicationFrame]
                              : [[UIScreen mainScreen] bounds];

    monitorCoords.add (Rectangle ((int) r.origin.x,
                                  (int) r.origin.y,
                                  (int) r.size.width,
                                  (int) r.size.height));
}


#endif

#endif
