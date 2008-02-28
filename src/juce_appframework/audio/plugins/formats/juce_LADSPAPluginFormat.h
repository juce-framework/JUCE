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

#ifndef __JUCE_LADSPAPLUGINFORMAT_JUCEHEADER__
#define __JUCE_LADSPAPLUGINFORMAT_JUCEHEADER__

#include "../juce_AudioPluginFormat.h"

#if JUCE_PLUGINHOST_LADSPA && JUCE_LINUX


//   Sorry, this file is just a placeholder at the moment!...


//==============================================================================
/**
    Implements a plugin format manager for DirectX plugins.
*/
class JUCE_API  LADSPAPluginFormat   : public AudioPluginFormat
{
public:
    //==============================================================================
    LADSPAPluginFormat();
    ~LADSPAPluginFormat();

    //==============================================================================
    const String getName() const                { return "LADSPA"; }
    void findAllTypesForFile (OwnedArray <PluginDescription>& results, const File& file);
    AudioPluginInstance* createInstanceFromDescription (const PluginDescription& desc);
    bool fileMightContainThisPluginType (const File& file);
    const FileSearchPath getDefaultLocationsToSearch();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    LADSPAPluginFormat (const LADSPAPluginFormat&);
    const LADSPAPluginFormat& operator= (const LADSPAPluginFormat&);
};

#endif

#endif   // __JUCE_LADSPAPLUGINFORMAT_JUCEHEADER__
