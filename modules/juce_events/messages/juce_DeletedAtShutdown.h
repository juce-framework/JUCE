/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCE_DELETEDATSHUTDOWN_JUCEHEADER__
#define __JUCE_DELETEDATSHUTDOWN_JUCEHEADER__


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
    DeletedAtShutdown();

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
    static Array <DeletedAtShutdown*>& getObjects();

    JUCE_DECLARE_NON_COPYABLE (DeletedAtShutdown)
};

#endif   // __JUCE_DELETEDATSHUTDOWN_JUCEHEADER__
