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

struct HashMapTest final : public UnitTest
{
    HashMapTest()
        : UnitTest ("HashMap", UnitTestCategories::containers)
    {}

    void runTest() override
    {
        doTest<AddElementsTest> ("AddElementsTest");
        doTest<AccessTest> ("AccessTest");
        doTest<RemoveTest> ("RemoveTest");
        doTest<PersistantMemoryLocationOfValues> ("PersistantMemoryLocationOfValues");
    }

    //==============================================================================
    struct AddElementsTest
    {
        template <typename KeyType>
        static void run (UnitTest& u)
        {
            AssociativeMap<KeyType, int> groundTruth;
            HashMap<KeyType, int> hashMap;

            RandomKeys<KeyType> keyOracle (300, 3827829);
            Random valueOracle (48735);

            int totalValues = 0;
            for (int i = 0; i < 10000; ++i)
            {
                auto key = keyOracle.next();
                auto value = valueOracle.nextInt();

                bool contains = (groundTruth.find (key) != nullptr);
                u.expectEquals ((int) contains, (int) hashMap.contains (key));

                groundTruth.add (key, value);
                hashMap.set (key, value);

                if (! contains) totalValues++;

                u.expectEquals (hashMap.size(), totalValues);
            }
        }
    };

    struct AccessTest
    {
        template <typename KeyType>
        static void run (UnitTest& u)
        {
            AssociativeMap<KeyType, int> groundTruth;
            HashMap<KeyType, int> hashMap;

            fillWithRandomValues (hashMap, groundTruth);

            for (auto pair : groundTruth.pairs)
                u.expectEquals (hashMap[pair.key], pair.value);
        }
    };

    struct RemoveTest
    {
        template <typename KeyType>
        static void run (UnitTest& u)
        {
            AssociativeMap<KeyType, int> groundTruth;
            HashMap<KeyType, int> hashMap;

            fillWithRandomValues (hashMap, groundTruth);
            auto n = groundTruth.size();

            Random r (3827387);

            for (int i = 0; i < 100; ++i)
            {
                auto idx = r.nextInt (n-- - 1);
                auto key = groundTruth.pairs.getReference (idx).key;

                groundTruth.pairs.remove (idx);
                hashMap.remove (key);

                u.expect (! hashMap.contains (key));

                for (auto pair : groundTruth.pairs)
                    u.expectEquals (hashMap[pair.key], pair.value);
            }
        }
    };

    // ensure that the addresses of object references don't change
    struct PersistantMemoryLocationOfValues
    {
        struct AddressAndValue { int value; const int* valueAddress; };

        template <typename KeyType>
        static void run (UnitTest& u)
        {
            AssociativeMap<KeyType, AddressAndValue> groundTruth;
            HashMap<KeyType, int> hashMap;

            RandomKeys<KeyType> keyOracle (300, 3827829);
            Random valueOracle (48735);

            for (int i = 0; i < 1000; ++i)
            {
                auto key = keyOracle.next();
                auto value = valueOracle.nextInt();

                hashMap.set (key, value);

                if (auto* existing = groundTruth.find (key))
                {
                    // don't change the address: only the value
                    existing->value = value;
                }
                else
                {
                    groundTruth.add (key, { value, &hashMap.getReference (key) });
                }

                for (auto pair : groundTruth.pairs)
                {
                    const auto& hashMapValue = hashMap.getReference (pair.key);

                    u.expectEquals (hashMapValue, pair.value.value);
                    u.expect (&hashMapValue == pair.value.valueAddress);
                }
            }

            auto n = groundTruth.size();
            Random r (3827387);

            for (int i = 0; i < 100; ++i)
            {
                auto idx = r.nextInt (n-- - 1);
                auto key = groundTruth.pairs.getReference (idx).key;

                groundTruth.pairs.remove (idx);
                hashMap.remove (key);

                for (auto pair : groundTruth.pairs)
                {
                    const auto& hashMapValue = hashMap.getReference (pair.key);

                    u.expectEquals (hashMapValue, pair.value.value);
                    u.expect (&hashMapValue == pair.value.valueAddress);
                }
            }
        }
    };

    //==============================================================================
    template <class Test>
    void doTest (const String& testName)
    {
        beginTest (testName);

        Test::template run<int> (*this);
        Test::template run<void*> (*this);
        Test::template run<String> (*this);
    }

    //==============================================================================
    template <typename KeyType, typename ValueType>
    struct AssociativeMap
    {
        struct KeyValuePair { KeyType key; ValueType value; };

        ValueType* find (KeyType key)
        {
            auto n = pairs.size();

            for (int i = 0; i < n; ++i)
            {
                auto& pair = pairs.getReference (i);

                if (pair.key == key)
                    return &pair.value;
            }

            return nullptr;
        }

        void add (KeyType key, ValueType value)
        {
            if (ValueType* v = find (key))
                *v = value;
            else
                pairs.add ({key, value});
        }

        int size() const { return pairs.size(); }

        Array<KeyValuePair> pairs;
    };

    template <typename KeyType, typename ValueType>
    static void fillWithRandomValues (HashMap<KeyType, int>& hashMap, AssociativeMap<KeyType, ValueType>& groundTruth)
    {
        RandomKeys<KeyType> keyOracle (300, 3827829);
        Random valueOracle (48735);

        for (int i = 0; i < 10000; ++i)
        {
            auto key = keyOracle.next();
            auto value = valueOracle.nextInt();

            groundTruth.add (key, value);
            hashMap.set (key, value);
        }
    }

    //==============================================================================
    template <typename KeyType>
    class RandomKeys
    {
    public:
        RandomKeys (int maxUniqueKeys, int seed) : r (seed)
        {
            for (int i = 0; i < maxUniqueKeys; ++i)
                keys.add (generateRandomKey (r));
        }

        const KeyType& next()
        {
            int i = r.nextInt (keys.size() - 1);
            return keys.getReference (i);
        }
    private:
        static KeyType generateRandomKey (Random&);

        Random r;
        Array<KeyType> keys;
    };
};

template <> int   HashMapTest::RandomKeys<int>  ::generateRandomKey (Random& rnd) { return rnd.nextInt(); }
template <> void* HashMapTest::RandomKeys<void*>::generateRandomKey (Random& rnd) { return reinterpret_cast<void*> (rnd.nextInt64()); }

template <> String HashMapTest::RandomKeys<String>::generateRandomKey (Random& rnd)
{
    String str;

    int len = rnd.nextInt (8)+1;
    for (int i = 0; i < len; ++i)
        str += static_cast<char> (rnd.nextInt (95) + 32);

    return str;
}

static HashMapTest hashMapTest;

} // namespace juce
