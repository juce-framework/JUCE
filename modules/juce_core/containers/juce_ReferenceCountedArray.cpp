/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2018 - ROLI Ltd.

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

#if JUCE_UNIT_TESTS

class ReferenceCountedArrayTests   : public UnitTest
{
public:
    ReferenceCountedArrayTests() : UnitTest ("ReferenceCountedArray", "Containers") {}

    //==============================================================================
    void runTest() override
    {
        beginTest ("Add derived objects");
        {
            ReferenceCountedArray<TestDerivedObj> derivedArray;
            derivedArray.add (static_cast<TestDerivedObj*> (new TestBaseObj()));
            expectEquals (derivedArray.size(), 1);
            expectEquals (derivedArray.getObjectPointer (0)->getReferenceCount(), 1);
            expectEquals (derivedArray[0]->getReferenceCount(), 2);

            for (auto o : derivedArray)
                expectEquals (o->getReferenceCount(), 1);

            ReferenceCountedArray<TestBaseObj> baseArray;
            baseArray.addArray (derivedArray);

            for (auto o : baseArray)
                expectEquals (o->getReferenceCount(), 2);

            derivedArray.clearQuick();
            baseArray.clearQuick();

            auto* baseObject = new TestBaseObj();
            TestBaseObj::Ptr baseObjectPtr = baseObject;
            expectEquals (baseObject->getReferenceCount(), 1);

            auto* derivedObject = new TestDerivedObj();
            TestDerivedObj::Ptr derivedObjectPtr = derivedObject;
            expectEquals (derivedObject->getReferenceCount(), 1);

            baseArray.add (baseObject);
            baseArray.add (derivedObject);

            for (auto o : baseArray)
                expectEquals (o->getReferenceCount(), 2);

            expectEquals (baseObject->getReferenceCount(),    2);
            expectEquals (derivedObject->getReferenceCount(), 2);

            derivedArray.add (derivedObject);

            for (auto o : derivedArray)
                expectEquals (o->getReferenceCount(), 3);

            derivedArray.clearQuick();
            baseArray.clearQuick();

            expectEquals (baseObject->getReferenceCount(),    1);
            expectEquals (derivedObject->getReferenceCount(), 1);

            baseArray.add (baseObjectPtr);
            baseArray.add (derivedObjectPtr);

            for (auto o : baseArray)
                expectEquals (o->getReferenceCount(), 2);

            derivedArray.add (derivedObjectPtr);

            for (auto o : derivedArray)
                expectEquals (o->getReferenceCount(), 3);
        }
    }

private:
    struct TestBaseObj : public ReferenceCountedObject
    {
        using Ptr = ReferenceCountedObjectPtr<TestBaseObj>;

        TestBaseObj() = default;
    };

    struct TestDerivedObj : public TestBaseObj
    {
        using Ptr = ReferenceCountedObjectPtr<TestDerivedObj>;

        TestDerivedObj() = default;
    };
};

static ReferenceCountedArrayTests referenceCountedArrayTests;

#endif

} // namespace juce
