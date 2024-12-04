/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

class ChangeBroadcaster;

//==============================================================================
/**
    Receives change event callbacks that are sent out by a ChangeBroadcaster.

    A ChangeBroadcaster keeps a set of listeners to which it broadcasts a message when
    the ChangeBroadcaster::sendChangeMessage() method is called. A subclass of
    ChangeListener is used to receive these callbacks.

    Note that the major difference between an ActionListener and a ChangeListener
    is that for a ChangeListener, multiple changes will be coalesced into fewer
    callbacks, but ActionListeners perform one callback for every event posted.

    @see ChangeBroadcaster, ActionListener

    @tags{Events}
*/
class JUCE_API  ChangeListener
{
public:
    /** Destructor. */
    virtual ~ChangeListener() = default;

    /** Your subclass should implement this method to receive the callback.
        @param source the ChangeBroadcaster that triggered the callback.
    */
    virtual void changeListenerCallback (ChangeBroadcaster* source) = 0;
};

} // namespace juce
