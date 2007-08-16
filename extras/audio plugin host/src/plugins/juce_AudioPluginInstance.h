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

#include "../../../audio plugins/wrapper/juce_AudioFilterBase.h"
class AudioPluginInstance;


//==============================================================================
class AudioPluginParameterListener
{
public:
    //==============================================================================
    /** Destructor. */
    virtual ~AudioPluginParameterListener() {}

    //==============================================================================
    /** Receives a callback when a parameter is changed. */
    virtual void audioPluginParameterChanged (AudioPluginInstance* plugin,
                                              int parameterIndex,
                                              float newValue) = 0;

    /** Called to indicate that something else in the plugin has changed, like its
        program, number of parameters, etc.
    */
    virtual void audioPluginChanged (AudioPluginInstance* plugin) = 0;

    /** Indicates that a parameter change gesture has started.

        E.g. if the user is dragging a slider, this would be called when they first 
        press the mouse button, and audioPluginParameterChangeGestureEnd would be
        called when they release it.

        @see audioPluginParameterChangeGestureEnd
    */
    virtual void audioPluginParameterChangeGestureBegin (AudioPluginInstance* plugin,
                                                         int parameterIndex);

    /** Indicates that a parameter change gesture has finished.

        E.g. if the user is dragging a slider, this would be called when they release 
        the mouse button.
        @see audioPluginParameterChangeGestureStart
    */
    virtual void audioPluginParameterChangeGestureEnd (AudioPluginInstance* plugin,
                                                       int parameterIndex);
};


//==============================================================================
/**
    Base class for an active instance of a plugin.

    This derives from the same AudioFilterBase object that is used in the
    plugin wrapper code, and most of its functionality is exposed by that interface,
    with just a few extra methods here for using it from the host side.

    @see AudioFilterBase, AudioPluginFormat
*/
class AudioPluginInstance   : public AudioFilterBase,
                              private AudioFilterBase::HostCallbacks
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

    /** Returns the plugin's latency.

        This may not always be fixed, but should be correct just after the filter
        has been prepared for playback.
    */
    virtual int getSamplesLatency() const = 0;


    //==============================================================================
    /** Adds a listener that will be called when one of this plugin's parameters changes. */
    void addListener (AudioPluginParameterListener* const newListener) throw();

    /** Removes a previously added listener. */
    void removeListener (AudioPluginParameterListener* const listenerToRemove) throw();

    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    VoidArray listeners;
    CriticalSection changedParamLock;
    Array <int> changedParams;
    Array <float> changedParamValues;

    class InternalAsyncUpdater : public Timer
    {
    public:
        InternalAsyncUpdater (AudioPluginInstance& owner);
        ~InternalAsyncUpdater() {}

        void timerCallback();

        juce_UseDebuggingNewOperator

    private:
        AudioPluginInstance& owner;
    };

    InternalAsyncUpdater* internalAsyncUpdater;
    void internalAsyncCallback();
    void queueChangeMessage (const int index, const float value) throw();

    AudioPluginInstance();

    bool JUCE_CALLTYPE getCurrentPositionInfo (AudioFilterBase::CurrentPositionInfo& info);
    void JUCE_CALLTYPE informHostOfParameterChange (int index, float newValue);
    void JUCE_CALLTYPE informHostOfParameterGestureBegin (int index);
    void JUCE_CALLTYPE informHostOfParameterGestureEnd (int index);
    void JUCE_CALLTYPE informHostOfStateChange();
};


#endif   // __JUCE_AUDIOPLUGININSTANCE_JUCEHEADER__
