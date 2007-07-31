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

#ifndef __JUCE_AUDIOFILTEREDITOR_JUCEHEADER__
#define __JUCE_AUDIOFILTEREDITOR_JUCEHEADER__

#ifdef _MSC_VER
  #pragma pack (push, 8)
#endif

#include "../../../juce.h"
class AudioFilterBase;


//==============================================================================
/**
    Base class for the component that forms a plugin's GUI.

    Derive your editor component from this class, and create an instance of it
    by overriding the AudioFilterBase::createEditor() method.

*/
class AudioFilterEditor  : public Component
{
public:
    //==============================================================================
    /** Creates a filter editor.

        You'll need to pass in the filter that's creating it.
    */
    AudioFilterEditor (AudioFilterBase* const ownerFilter);

    /** Destructor. */
    ~AudioFilterEditor();


    //==============================================================================
    /** Returns a pointer to the filter that owns this editor. */
    AudioFilterBase* getOwnerFilter() const throw()         { return ownerFilter; }


private:
    //==============================================================================
    AudioFilterBase* const ownerFilter;
};

#ifdef _MSC_VER
  #pragma pack (pop)
#endif

#endif   // __JUCE_AUDIOFILTEREDITOR_JUCEHEADER__
