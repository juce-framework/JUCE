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

namespace juce::LinuxEventLoop
{

    /** Registers a callback that will be called when a file descriptor is ready for I/O.

        This will add the given file descriptor to the internal set of file descriptors
        that will be passed to the poll() call. When this file descriptor has data to read
        the readCallback will be called.

        @param fd            the file descriptor to be monitored
        @param readCallback  a callback that will be called when the file descriptor has
                             data to read. The file descriptor will be passed as an argument
        @param eventMask     a bit mask specifying the events you are interested in for the
                             file descriptor. The possible values for this are defined in
                             <poll.h>
    */
    void registerFdCallback (int fd, std::function<void (int)> readCallback, short eventMask = 1 /*POLLIN*/);

    /** Unregisters a previously registered file descriptor.

        @see registerFdCallback
    */
    void unregisterFdCallback (int fd);

} // namespace juce::LinuxEventLoop
