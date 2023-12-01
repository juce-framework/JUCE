/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

class SharedResourcePointerTest final : public UnitTest
{
public:
    SharedResourcePointerTest()
        : UnitTest ("SharedResourcePointer", UnitTestCategories::memory) {}

    void runTest() final
    {
        beginTest ("Only one instance is created");
        {
            static int count = 0;
            struct CountIncrementer { CountIncrementer() { ++count; } };
            expect (count == 0);

            const SharedResourcePointer<CountIncrementer> instance1;
            expect (count == 1);

            const SharedResourcePointer<CountIncrementer> instance2;
            expect (count == 1);

            expect (&instance1.get() == &instance2.get());
        }

        beginTest ("The shared object is destroyed when the reference count reaches 0");
        {
            static int count = 0;
            struct ReferenceCounter
            {
                ReferenceCounter() { ++count; }
                ~ReferenceCounter() { --count; }
            };

            expect (count == 0);

            {
                const SharedResourcePointer<ReferenceCounter> instance1;
                const SharedResourcePointer<ReferenceCounter> instance2;
                expect (count == 1);
            }

            expect (count == 0);
        }

        beginTest ("getInstanceWithoutCreating()");
        {
            struct Object{};

            expect (SharedResourcePointer<Object>::getSharedObjectWithoutCreating() == std::nullopt);

            {
                const SharedResourcePointer<Object> instance;
                expect (&SharedResourcePointer<Object>::getSharedObjectWithoutCreating()->get() == &instance.get());
            }

            expect (SharedResourcePointer<Object>::getSharedObjectWithoutCreating() == std::nullopt);
        }

        beginTest ("Create objects with private constructors");
        {
            class ObjectWithPrivateConstructor
            {
            private:
                ObjectWithPrivateConstructor() = default;
                friend SharedResourcePointer<ObjectWithPrivateConstructor>;
            };

            SharedResourcePointer<ObjectWithPrivateConstructor> instance;
        }
    }
};

static SharedResourcePointerTest sharedResourcePointerTest;

} // namespace juce
