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

#ifndef __JUCE_DELETEDATSHUTDOWN_JUCEHEADER__
#define __JUCE_DELETEDATSHUTDOWN_JUCEHEADER__

#include "../../juce_core/containers/juce_Array.h"


//==============================================================================
/**
    Classes derived from this will be automatically deleted when the application exits.

    After JUCEApplication::shutdown() has been called, any objects derived from
    DeletedAtShutdown which are still in existence will be deleted in the reverse
    order to that in which they were created.

    So if you've got a singleton and don't want to have to explicitly delete it, just
    inherit from this and it'll be taken care of.
*/
class JUCE_API  DeletedAtShutdown
{
protected:
    /** Creates a DeletedAtShutdown object. */
    DeletedAtShutdown() throw();

    /** Destructor.

        It's ok to delete these objects explicitly - it's only the ones left
        dangling at the end that will be deleted automatically.
    */
    virtual ~DeletedAtShutdown();


public:
    /** Deletes all extant objects.

        This shouldn't be used by applications, as it's called automatically
        in the shutdown code of the JUCEApplication class.
    */
    static void deleteAll();

private:
    DeletedAtShutdown (const DeletedAtShutdown&);
    const DeletedAtShutdown& operator= (const DeletedAtShutdown&);
};

#endif   // __JUCE_DELETEDATSHUTDOWN_JUCEHEADER__
