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

#ifndef __JUCE_AUDIOPROCESSORLISTENER_JUCEHEADER__
#define __JUCE_AUDIOPROCESSORLISTENER_JUCEHEADER__

class AudioProcessor;

//==============================================================================
/**
    Base class for listeners that want to know about changes to an AudioProcessor.

    Use AudioProcessor::addListener() to register your listener with an AudioProcessor.

    @see AudioProcessor
*/
class JUCE_API  AudioProcessorListener
{
public:
    //==============================================================================
    /** Destructor. */
    virtual ~AudioProcessorListener() {}

    //==============================================================================
    /** Receives a callback when a parameter is changed.

        IMPORTANT NOTE: this will be called synchronously when a parameter changes, and
        many audio processors will change their parameter during their audio callback.
        This means that not only has your handler code got to be completely thread-safe,
        but it's also got to be VERY fast, and avoid blocking. If you need to handle
        this event on your message thread, use this callback to trigger an AsyncUpdater
        or ChangeBroadcaster which you can respond to on the message thread.
    */
    virtual void audioProcessorParameterChanged (AudioProcessor* processor,
                                                 int parameterIndex,
                                                 float newValue) = 0;

    /** Called to indicate that something else in the plugin has changed, like its
        program, number of parameters, etc.

        IMPORTANT NOTE: this will be called synchronously, and many audio processors will
        call it during their audio callback. This means that not only has your handler code
        got to be completely thread-safe, but it's also got to be VERY fast, and avoid
        blocking. If you need to handle this event on your message thread, use this callback
        to trigger an AsyncUpdater or ChangeBroadcaster which you can respond to later on the
        message thread.
    */
    virtual void audioProcessorChanged (AudioProcessor* processor) = 0;

    /** Indicates that a parameter change gesture has started.

        E.g. if the user is dragging a slider, this would be called when they first
        press the mouse button, and audioProcessorParameterChangeGestureEnd would be
        called when they release it.

        IMPORTANT NOTE: this will be called synchronously, and many audio processors will
        call it during their audio callback. This means that not only has your handler code
        got to be completely thread-safe, but it's also got to be VERY fast, and avoid
        blocking. If you need to handle this event on your message thread, use this callback
        to trigger an AsyncUpdater or ChangeBroadcaster which you can respond to later on the
        message thread.

        @see audioProcessorParameterChangeGestureEnd
    */
    virtual void audioProcessorParameterChangeGestureBegin (AudioProcessor* processor,
                                                            int parameterIndex);

    /** Indicates that a parameter change gesture has finished.

        E.g. if the user is dragging a slider, this would be called when they release
        the mouse button.

        IMPORTANT NOTE: this will be called synchronously, and many audio processors will
        call it during their audio callback. This means that not only has your handler code
        got to be completely thread-safe, but it's also got to be VERY fast, and avoid
        blocking. If you need to handle this event on your message thread, use this callback
        to trigger an AsyncUpdater or ChangeBroadcaster which you can respond to later on the
        message thread.

        @see audioProcessorParameterChangeGestureBegin
    */
    virtual void audioProcessorParameterChangeGestureEnd (AudioProcessor* processor,
                                                          int parameterIndex);
};

#endif   // __JUCE_AUDIOPROCESSORLISTENER_JUCEHEADER__
