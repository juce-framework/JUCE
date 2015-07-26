/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifndef JUCE_MOUNTEDVOLUMELISTCHANGEDETECTOR_H_INCLUDED
#define JUCE_MOUNTEDVOLUMELISTCHANGEDETECTOR_H_INCLUDED

#if JUCE_MAC || JUCE_WINDOWS || defined (DOXYGEN)

//==============================================================================
/**
    An instance of this class will provide callbacks when drives are
    mounted or unmounted on the system.

    Just inherit from this class and implement the pure virtual method
    to get the callbacks, there's no need to do anything else.

    @see File::findFileSystemRoots()
*/
class JUCE_API  MountedVolumeListChangeDetector
{
public:
    MountedVolumeListChangeDetector();
    virtual ~MountedVolumeListChangeDetector();

    /** This method is called when a volume is mounted or unmounted. */
    virtual void mountedVolumeListChanged() = 0;

private:
    JUCE_PUBLIC_IN_DLL_BUILD (struct Pimpl)
    friend struct ContainerDeletePolicy<Pimpl>;
    ScopedPointer<Pimpl> pimpl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MountedVolumeListChangeDetector)
};

#endif

#endif   // JUCE_MOUNTEDVOLUMELISTCHANGEDETECTOR_H_INCLUDED
