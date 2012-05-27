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

#ifndef __JUCE_OSX_OBJCHELPERS_JUCEHEADER__
#define __JUCE_OSX_OBJCHELPERS_JUCEHEADER__


/* This file contains a few helper functions that are used internally but which
   need to be kept away from the public headers because they use obj-C symbols.
*/
namespace
{
    //==============================================================================
    static inline String nsStringToJuce (NSString* s)
    {
        return CharPointer_UTF8 ([s UTF8String]);
    }

    static inline NSString* juceStringToNS (const String& s)
    {
        return [NSString stringWithUTF8String: s.toUTF8()];
    }

    static inline NSString* nsStringLiteral (const char* const s) noexcept
    {
        return [NSString stringWithUTF8String: s];
    }

    static inline NSString* nsEmptyString() noexcept
    {
        return [NSString string];
    }
}

//==============================================================================
class ObjCClassBuilder
{
public:
    ObjCClassBuilder (Class superClass, const String& name)
        : cls (objc_allocateClassPair (superClass, name.toUTF8(), 0))
    {
    }

    Class getClass()
    {
        objc_registerClassPair (cls);
        return cls;
    }

    template <typename Type>
    void addIvar (const char* name)
    {
        BOOL b = class_addIvar (cls, name, sizeof (Type), (uint8_t) rint (log2 (sizeof (Type))), @encode (Type));
        jassert (b); (void) b;
    }

    template <typename FunctionType>
    void addMethod (SEL selector, FunctionType callbackFn, const char* signature)
    {
        BOOL b = class_addMethod (cls, selector, (IMP) callbackFn, signature);
        jassert (b); (void) b;
    }

    static id sendSuperclassMessage (id self, SEL selector)
    {
        objc_super s = { self, [NSObject class] };
        return objc_msgSendSuper (&s, selector);
    }

    static String getRandomisedName (const char* root)
    {
        return root + String::toHexString (Random::getSystemRandom().nextInt64());
    }

private:
    Class cls;

    JUCE_DECLARE_NON_COPYABLE (ObjCClassBuilder);
};

#endif   // __JUCE_OSX_OBJCHELPERS_JUCEHEADER__
