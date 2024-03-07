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

static constexpr bool graphemeBreakTable[14][14] =
{
    //                Other  cr     lf     Control Extend  ri     Prepend  SpacingMark  L       V       T       LV      LVT     zwj
    /*Other*/       { true,  true,  true,  true,   false,  true,  true,    false,       true,   true,   true,   true,   true,   true  },
    /*cr*/          { true,  true,  false, true,   true,   true,  true,    true,        true,   true,   true,   true,   true,   true  },
    /*lf*/          { true,  true,  true,  true,   true,   true,  true,    true,        true,   true,   true,   true,   true,   true  },
    /*Control*/     { true,  true,  true,  true,   true,   true,  true,    true,        true,   true,   true,   true,   true,   true  },
    /*Extend*/      { true,  true,  true,  true,   false,  true,  true,    false,       true,   true,   true,   true,   true,   true  },
    /*ri*/          { true,  true,  true,  true,   false,  false, true,    false,       true,   true,   true,   true,   true,   true  },
    /*Prepend*/     { false, true,  true,  true,   false,  false, false,   false,       false,  false,  false,  false,  false,  false },
    /*SpacingMark*/ { true,  true,  true,  true,   false,  true,  true,    false,       true,   true,   true,   true,   true,   true  },
    /*L*/           { true,  true,  true,  true,   false,  true,  true,    false,       false,  false,  true,   false,  false,  true  },
    /*V*/           { true,  true,  true,  true,   false,  true,  true,    false,       true,   false,  false,  true,   true,   true  },
    /*T*/           { true,  true,  true,  true,   false,  true,  true,    false,       true,   true,   false,  true,   true,   true  },
    /*LV*/          { true,  true,  true,  true,   false,  true,  true,    false,       true,   false,  false,  true,   true,   true  },
    /*LVT*/         { true,  true,  true,  true,   false,  true,  true,    false,       true,   true,   false,  true,   true,   true  },
    /*zwj*/         { true,  true,  true,  true,   false,  true,  true,    false,       true,   true,   true,   true,   true,   true  }
};

static inline bool isGraphemeBreak (GraphemeBreakType g1, GraphemeBreakType g2)
{
    return graphemeBreakTable[(int) g1][(int) g2];
}

} // namespace juce
