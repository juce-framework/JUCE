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

/** A type used to store years with double precision. */
using Years = std::chrono::duration<double, std::ratio<31556952>>;

/** A type used to store months with double precision. */
using Months = std::chrono::duration<double, std::ratio<2629746>>;

/** A type used to store weeks with double precision. */
using Weeks = std::chrono::duration<double, std::ratio<604800>>;

/** A type used to store days with double precision. */
using Days = std::chrono::duration<double, std::ratio<86400>>;

/** A type used to store hours with double precision. */
using Hours = std::chrono::duration<double, std::ratio<3600>>;

/** A type used to store minutes with double precision. */
using Minutes = std::chrono::duration<double, std::ratio<60>>;

/** A type used to store seconds with double precision. */
using Seconds = std::chrono::duration<double, std::ratio<1>>;

/** A type used to store milliseconds with double precision. */
using Milliseconds = std::chrono::duration<double, std::milli>;

/** A type used to store microseconds with double precision. */
using Microseconds = std::chrono::duration<double, std::micro>;

/** A type used to store nanoseconds with double precision. */
using Nanoseconds = std::chrono::duration<double, std::nano>;

} // namespace juce
