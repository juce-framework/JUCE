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

#ifndef __JUCE_AUDIOPLUGININSTANCE_JUCEHEADER__
#define __JUCE_AUDIOPLUGININSTANCE_JUCEHEADER__


//==============================================================================
/**
    Base class for an active instance of a plugin.

    This derives from the AudioProcessor class, and adds some extra functionality
    that helps when wrapping dynamically loaded plugins.

    @see AudioProcessor, AudioPluginFormat
*/
class AudioPluginInstance   : public AudioProcessor
{
public:
    //==============================================================================
    /** Destructor.

        Make sure that you delete any UI components that belong to this plugin before
        deleting the plugin.
    */
    virtual ~AudioPluginInstance();

    //==============================================================================
    /** Returns the plugin's name. */
    virtual const String getName() const = 0;

    /** Asks the plugin to supply a manufacturer name. */
    virtual const String getManufacturer() const = 0;

    /** Asks the plugin for its version number. */
    virtual const String getVersion() const = 0;

    /** Returns true if the plugin is an instrument rather than an effect. */
    virtual bool isInstrument() const = 0;

    /** Returns a category description for the plugin.

        E.g. "Dynamics", "Reverbs", etc.
    */
    virtual const String getCategory() const = 0;

     /** Returns the class of plugin to which this belongs.

        E.g. "VST", "AU", etc
    */
    virtual const String getFormatName() const = 0;

    /** Returns the binary file containing the plugin.

        This is normally the DLL or bundle file.
    */
    virtual const File getFile() const = 0;

    /** Returns a unique identifier for the plugin.

        (Note that this may not be unique across different plugin formats).
    */
    virtual int getUID() const = 0;


    //==============================================================================
    /** Returns true if the plugin wants midi messages. */
    virtual bool acceptsMidi() const = 0;

    /** Returns true if the plugin produces midi messages. */
    virtual bool producesMidi() const = 0;


    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    AudioPluginInstance();
};


#endif   // __JUCE_AUDIOPLUGININSTANCE_JUCEHEADER__
