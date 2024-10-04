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

class ObjCHelpersTest final : public UnitTest
{
public:
    ObjCHelpersTest() : UnitTest { "ObjCHelpers", UnitTestCategories::native } {}

    void runTest() final
    {
        beginTest ("Range");
        {
            constexpr auto start = 10;
            constexpr auto length = 20;

            const auto juceRange = Range<int>::withStartAndLength (start, length);
            const auto nsRange = NSMakeRange (start, length);

            expect (nsRangeToJuce (nsRange) == juceRange);
            expect (NSEqualRanges (nsRange, juceRangeToNS (juceRange)));
        }

        beginTest ("String");
        {
            String juceString { "Hello world!" };
            NSString *nsString { @"Hello world!" };

            expect (nsStringToJuce (nsString) == juceString);
            expect ([nsString isEqualToString: juceStringToNS (juceString)]);
            expect ([nsString isEqualToString: nsStringLiteral ("Hello world!")]);
        }

        beginTest ("StringArray");
        {
            const StringArray stringArray { "Hello world!", "this", "is", "a", "test" };
            NSArray *nsArray { @[@"Hello world!", @"this", @"is", @"a", @"test"] };

            expect ([nsArray isEqualToArray: createNSArrayFromStringArray (stringArray)]);
        }

        beginTest ("Dictionary");
        {
            DynamicObject::Ptr data { new DynamicObject() };
            data->setProperty ("integer", 1);
            data->setProperty ("double", 2.3);
            data->setProperty ("boolean", true);
            data->setProperty ("string", "Hello world!");

            Array<var> array { 45, 67.8, true, "Hello array!" };
            data->setProperty ("array", array);

            const auto* nsDictionary = varToNSDictionary (data.get());
            expect (nsDictionary != nullptr);

            const auto clone = nsDictionaryToVar (nsDictionary);
            expect (clone.isObject());

            expect (clone.getProperty ("integer", {}).isInt());
            expect (clone.getProperty ("double",  {}).isDouble());
            expect (clone.getProperty ("boolean", {}).isBool());
            expect (clone.getProperty ("string",  {}).isString());
            expect (clone.getProperty ("array",   {}).isArray());

            expect (clone.getProperty ("integer", {}) == var { 1 });
            expect (clone.getProperty ("double",  {}) == var { 2.3 });
            expect (clone.getProperty ("boolean", {}) == var { true });
            expect (clone.getProperty ("string",  {}) == var { "Hello world!" });
            expect (clone.getProperty ("array",   {}) == var { array });
        }

        beginTest ("varToNSDictionary converts a void variant to an empty dictionary");
        {
            var voidVariant;

            const auto* nsDictionary = varToNSDictionary (voidVariant);
            expect (nsDictionary != nullptr);

            const auto result = nsDictionaryToVar (nsDictionary);
            expect (result.isObject());
            expect (result.getDynamicObject()->getProperties().isEmpty());
        }
    }
};

static ObjCHelpersTest objCHelpersTest;

} // namespace juce
