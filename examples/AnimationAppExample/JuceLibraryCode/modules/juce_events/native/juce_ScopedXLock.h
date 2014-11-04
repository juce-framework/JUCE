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

#ifndef JUCE_SCOPEDXLOCK_H_INCLUDED
#define JUCE_SCOPEDXLOCK_H_INCLUDED


//==============================================================================
#if JUCE_LINUX || DOXYGEN

/** A handy class that uses XLockDisplay and XUnlockDisplay to lock the X server
    using RAII (Only available in Linux!).
*/
class ScopedXLock
{
public:
    /** Creating a ScopedXLock object locks the X display.
        This uses XLockDisplay() to grab the display that Juce is using.
    */
    ScopedXLock();

    /** Deleting a ScopedXLock object unlocks the X display.
        This calls XUnlockDisplay() to release the lock.
    */
    ~ScopedXLock();
};

#endif
#endif   // JUCE_SCOPEDXLOCK_H_INCLUDED
