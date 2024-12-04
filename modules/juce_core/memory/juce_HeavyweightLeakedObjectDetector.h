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

//==============================================================================
/**
    This class is a useful way of tracking down hard to find memory leaks when the
    regular LeakedObjectDetector isn't enough.

    As well as firing when any instances of the OwnerClass type are leaked, it will
    print out a stack trace showing where the leaked object was created. This is obviously
    quite a heavyweight task so, unlike the LeakedObjectDetector which should be always
    be added to your classes, you should only use this object temporarily when you are
    debugging and remove it when finished.

    To use it, use the JUCE_HEAVYWEIGHT_LEAK_DETECTOR macro as a simple way to put
    one in your class declaration.

    @tags{Core}
*/
template <class OwnerClass>
class HeavyweightLeakedObjectDetector
{
public:
    //==============================================================================
    HeavyweightLeakedObjectDetector() noexcept                                           { getBacktraceMap()[this] = SystemStats::getStackBacktrace(); }
    HeavyweightLeakedObjectDetector (const HeavyweightLeakedObjectDetector&) noexcept    { getBacktraceMap()[this] = SystemStats::getStackBacktrace(); }

    ~HeavyweightLeakedObjectDetector()                                                   { getBacktraceMap().erase (this); }

private:
    //==============================================================================
    typedef std::map<HeavyweightLeakedObjectDetector<OwnerClass>*, String> BacktraceMap;

    //==============================================================================
    struct BacktraceMapHolder
    {
        BacktraceMapHolder() = default;

        ~BacktraceMapHolder()
        {
            if (map.size() > 0)
            {
                DBG ("*** Leaked objects detected: " << map.size() << " instance(s) of class " << getLeakedObjectClassName());
                DBG (getFormattedBacktracesString());

                /** If you hit this, then you've leaked one or more objects of the type specified by
                    the 'OwnerClass' template parameter - the name and stack trace of its creation should
                    have been printed by the lines above.

                    If you're leaking, it's probably because you're using old-fashioned, non-RAII techniques for
                    your object management. Tut, tut. Always, always use std::unique_ptrs, OwnedArrays,
                    ReferenceCountedObjects, etc, and avoid the 'delete' operator at all costs!
                */
                jassertfalse;
            }
        }

        String getFormattedBacktracesString() const
        {
            String str;

            int counter = 1;
            for (auto& bt : map)
            {
                str << "\nBacktrace " << String (counter++)                                << "\n"
                    << "-----------------------------------------------------------------" << "\n"
                    << bt.second;
            }

            return str;
        }

        BacktraceMap map;
    };

    static BacktraceMap& getBacktraceMap()
    {
        static BacktraceMapHolder holder;
        return holder.map;
    }

    static const char* getLeakedObjectClassName()
    {
        return OwnerClass::getLeakedObjectClassName();
    }
};

//==============================================================================
#if DOXYGEN || ! defined (JUCE_HEAVYWEIGHT_LEAK_DETECTOR)
 #if (DOXYGEN || JUCE_CHECK_MEMORY_LEAKS)
  /** This macro lets you embed a heavyweight leak-detecting object inside a class.

      To use it, simply declare a JUCE_HEAVYWEIGHT_LEAK_DETECTOR (YourClassName) inside a private section
      of the class declaration. E.g.

      @code
      class MyClass
      {
      public:
          MyClass();
          void blahBlah();

      private:
          JUCE_HEAVYWEIGHT_LEAK_DETECTOR (MyClass)
      };
      @endcode

      NB: you should only use this when you really need to track down a tricky memory leak, and
      should never leave one of these inside a class!

      @see HeavyweightLeakedObjectDetector, JUCE_LEAK_DETECTOR, LeakedObjectDetector
  */
  #define JUCE_HEAVYWEIGHT_LEAK_DETECTOR(OwnerClass) \
        friend class juce::HeavyweightLeakedObjectDetector<OwnerClass>; \
        static const char* getLeakedObjectClassName() noexcept { return #OwnerClass; } \
        juce::HeavyweightLeakedObjectDetector<OwnerClass> JUCE_JOIN_MACRO (leakDetector, __LINE__);
 #else
  #define JUCE_HEAVYWEIGHT_LEAK_DETECTOR(OwnerClass)
 #endif
#endif

} // namespace juce
