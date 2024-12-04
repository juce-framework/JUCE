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

#if JUCE_UNIT_TESTS

static struct OwnedArrayTest : public UnitTest
{
    struct Base
    {
        Base() = default;
        virtual ~Base() = default;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Base)
    };

    struct Derived final : public Base
    {
        Derived() = default;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Derived)
    };

    struct DestructorObj
    {
        DestructorObj (OwnedArrayTest& p,
                       OwnedArray<DestructorObj>& arr)
            : parent (p), objectArray (arr)
        {}

        ~DestructorObj()
        {
            data = 0;

            for (auto* o : objectArray)
            {
                parent.expect (o != nullptr);
                parent.expect (o != this);

                if (o != nullptr)
                    parent.expectEquals (o->data, 956);
            }
        }

        OwnedArrayTest& parent;
        OwnedArray<DestructorObj>& objectArray;
        int data = 956;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DestructorObj)
    };

    OwnedArrayTest()
        : UnitTest ("OwnedArray", UnitTestCategories::containers)
    {}

    void runTest() override
    {
        beginTest ("After converting move construction, ownership is transferred");
        {
            OwnedArray<Derived> derived { new Derived{}, new Derived{}, new Derived{} };

            OwnedArray<Base> base  { std::move (derived) };

            expectEquals (base.size(), 3);
            expectEquals (derived.size(), 0);
        }

        beginTest ("After converting move assignment, ownership is transferred");
        {
            OwnedArray<Base> base;

            base = OwnedArray<Derived> { new Derived{}, new Derived{}, new Derived{} };

            expectEquals (base.size(), 3);
        }

        beginTest ("Iterate in destructor");
        {
            {
                OwnedArray<DestructorObj> arr;

                for (int i = 0; i < 2; ++i)
                    arr.add (new DestructorObj (*this, arr));
            }

            OwnedArray<DestructorObj> arr;

            for (int i = 0; i < 1025; ++i)
                arr.add (new DestructorObj (*this, arr));

            while (! arr.isEmpty())
                arr.remove (0);

            for (int i = 0; i < 1025; ++i)
                arr.add (new DestructorObj (*this, arr));

            arr.removeRange (1, arr.size() - 3);

            for (int i = 0; i < 1025; ++i)
                arr.add (new DestructorObj (*this, arr));

            arr.set (500, new DestructorObj (*this, arr));
        }
    }
} ownedArrayTest;

#endif

}
