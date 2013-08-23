/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

#ifndef JUCE_HASHMAP_H_INCLUDED
#define JUCE_HASHMAP_H_INCLUDED


//==============================================================================
/**
    A simple class to generate hash functions for some primitive types, intended for
    use with the HashMap class.
    @see HashMap
*/
struct DefaultHashFunctions
{
    /** Generates a simple hash from an integer. */
    int generateHash (const int key, const int upperLimit) const noexcept        { return std::abs (key) % upperLimit; }
    /** Generates a simple hash from an int64. */
    int generateHash (const int64 key, const int upperLimit) const noexcept      { return std::abs ((int) key) % upperLimit; }
    /** Generates a simple hash from a string. */
    int generateHash (const String& key, const int upperLimit) const noexcept    { return (int) (((uint32) key.hashCode()) % (uint32) upperLimit); }
    /** Generates a simple hash from a variant. */
    int generateHash (const var& key, const int upperLimit) const noexcept       { return generateHash (key.toString(), upperLimit); }
};


//==============================================================================
/**
    Holds a set of mappings between some key/value pairs.

    The types of the key and value objects are set as template parameters.
    You can also specify a class to supply a hash function that converts a key value
    into an hashed integer. This class must have the form:

    @code
    struct MyHashGenerator
    {
        int generateHash (MyKeyType key, int upperLimit)
        {
            // The function must return a value 0 <= x < upperLimit
            return someFunctionOfMyKeyType (key) % upperLimit;
        }
    };
    @endcode

    Like the Array class, the key and value types are expected to be copy-by-value
    types, so if you define them to be pointer types, this class won't delete the
    objects that they point to.

    If you don't supply a class for the HashFunctionType template parameter, the
    default one provides some simple mappings for strings and ints.

    @code
    HashMap<int, String> hash;
    hash.set (1, "item1");
    hash.set (2, "item2");

    DBG (hash [1]); // prints "item1"
    DBG (hash [2]); // prints "item2"

    // This iterates the map, printing all of its key -> value pairs..
    for (HashMap<int, String>::Iterator i (hash); i.next();)
        DBG (i.getKey() << " -> " << i.getValue());
    @endcode

    @tparam HashFunctionType The class of hash function, which must be copy-constructible.
    @see CriticalSection, DefaultHashFunctions, NamedValueSet, SortedSet
*/
template <typename KeyType,
          typename ValueType,
          class HashFunctionType = DefaultHashFunctions,
          class TypeOfCriticalSectionToUse = DummyCriticalSection>
class HashMap
{
private:
    typedef PARAMETER_TYPE (KeyType)   KeyTypeParameter;
    typedef PARAMETER_TYPE (ValueType) ValueTypeParameter;

public:
    //==============================================================================
    /** Creates an empty hash-map.

        @param numberOfSlots Specifies the number of hash entries the map will use. This will be
                            the "upperLimit" parameter that is passed to your generateHash()
                            function. The number of hash slots will grow automatically if necessary,
                            or it can be remapped manually using remapTable().
        @param hashFunction An instance of HashFunctionType, which will be copied and
                            stored to use with the HashMap. This parameter can be omitted
                            if HashFunctionType has a default constructor.
    */
    explicit HashMap (int numberOfSlots = defaultHashTableSize,
                      HashFunctionType hashFunction = HashFunctionType())
       : hashFunctionToUse (hashFunction), totalNumItems (0)
    {
        slots.insertMultiple (0, nullptr, numberOfSlots);
    }

    /** Destructor. */
    ~HashMap()
    {
        clear();
    }

    //==============================================================================
    /** Removes all values from the map.
        Note that this will clear the content, but won't affect the number of slots (see
        remapTable and getNumSlots).
    */
    void clear()
    {
        const ScopedLockType sl (getLock());

        for (int i = slots.size(); --i >= 0;)
        {
            HashEntry* h = slots.getUnchecked(i);

            while (h != nullptr)
            {
                const ScopedPointer<HashEntry> deleter (h);
                h = h->nextEntry;
            }

            slots.set (i, nullptr);
        }

        totalNumItems = 0;
    }

    //==============================================================================
    /** Returns the current number of items in the map. */
    inline int size() const noexcept
    {
        return totalNumItems;
    }

    /** Returns the value corresponding to a given key.
        If the map doesn't contain the key, a default instance of the value type is returned.
        @param keyToLookFor    the key of the item being requested
    */
    inline ValueType operator[] (KeyTypeParameter keyToLookFor) const
    {
        const ScopedLockType sl (getLock());

        for (const HashEntry* entry = slots.getUnchecked (generateHashFor (keyToLookFor)); entry != nullptr; entry = entry->nextEntry)
            if (entry->key == keyToLookFor)
                return entry->value;

        return ValueType();
    }

    //==============================================================================
    /** Returns true if the map contains an item with the specied key. */
    bool contains (KeyTypeParameter keyToLookFor) const
    {
        const ScopedLockType sl (getLock());

        for (const HashEntry* entry = slots.getUnchecked (generateHashFor (keyToLookFor)); entry != nullptr; entry = entry->nextEntry)
            if (entry->key == keyToLookFor)
                return true;

        return false;
    }

    /** Returns true if the hash contains at least one occurrence of a given value. */
    bool containsValue (ValueTypeParameter valueToLookFor) const
    {
        const ScopedLockType sl (getLock());

        for (int i = getNumSlots(); --i >= 0;)
            for (const HashEntry* entry = slots.getUnchecked(i); entry != nullptr; entry = entry->nextEntry)
                if (entry->value == valueToLookFor)
                    return true;

        return false;
    }

    //==============================================================================
    /** Adds or replaces an element in the hash-map.
        If there's already an item with the given key, this will replace its value. Otherwise, a new item
        will be added to the map.
    */
    void set (KeyTypeParameter newKey, ValueTypeParameter newValue)
    {
        const ScopedLockType sl (getLock());
        const int hashIndex = generateHashFor (newKey);

        HashEntry* const firstEntry = slots.getUnchecked (hashIndex);

        for (HashEntry* entry = firstEntry; entry != nullptr; entry = entry->nextEntry)
        {
            if (entry->key == newKey)
            {
                entry->value = newValue;
                return;
            }
        }

        slots.set (hashIndex, new HashEntry (newKey, newValue, firstEntry));
        ++totalNumItems;

        if (totalNumItems > (getNumSlots() * 3) / 2)
            remapTable (getNumSlots() * 2);
    }

    /** Removes an item with the given key. */
    void remove (KeyTypeParameter keyToRemove)
    {
        const ScopedLockType sl (getLock());
        const int hashIndex = generateHashFor (keyToRemove);
        HashEntry* entry = slots.getUnchecked (hashIndex);
        HashEntry* previous = nullptr;

        while (entry != nullptr)
        {
            if (entry->key == keyToRemove)
            {
                const ScopedPointer<HashEntry> deleter (entry);

                entry = entry->nextEntry;

                if (previous != nullptr)
                    previous->nextEntry = entry;
                else
                    slots.set (hashIndex, entry);

                --totalNumItems;
            }
            else
            {
                previous = entry;
                entry = entry->nextEntry;
            }
        }
    }

    /** Removes all items with the given value. */
    void removeValue (ValueTypeParameter valueToRemove)
    {
        const ScopedLockType sl (getLock());

        for (int i = getNumSlots(); --i >= 0;)
        {
            HashEntry* entry = slots.getUnchecked(i);
            HashEntry* previous = nullptr;

            while (entry != nullptr)
            {
                if (entry->value == valueToRemove)
                {
                    const ScopedPointer<HashEntry> deleter (entry);

                    entry = entry->nextEntry;

                    if (previous != nullptr)
                        previous->nextEntry = entry;
                    else
                        slots.set (i, entry);

                    --totalNumItems;
                }
                else
                {
                    previous = entry;
                    entry = entry->nextEntry;
                }
            }
        }
    }

    /** Remaps the hash-map to use a different number of slots for its hash function.
        Each slot corresponds to a single hash-code, and each one can contain multiple items.
        @see getNumSlots()
    */
    void remapTable (int newNumberOfSlots)
    {
        HashMap newTable (newNumberOfSlots);

        for (int i = getNumSlots(); --i >= 0;)
            for (const HashEntry* entry = slots.getUnchecked(i); entry != nullptr; entry = entry->nextEntry)
                newTable.set (entry->key, entry->value);

        swapWith (newTable);
    }

    /** Returns the number of slots which are available for hashing.
        Each slot corresponds to a single hash-code, and each one can contain multiple items.
        @see getNumSlots()
    */
    inline int getNumSlots() const noexcept
    {
        return slots.size();
    }

    //==============================================================================
    /** Efficiently swaps the contents of two hash-maps. */
    template <class OtherHashMapType>
    void swapWith (OtherHashMapType& otherHashMap) noexcept
    {
        const ScopedLockType lock1 (getLock());
        const typename OtherHashMapType::ScopedLockType lock2 (otherHashMap.getLock());

        slots.swapWith (otherHashMap.slots);
        std::swap (totalNumItems, otherHashMap.totalNumItems);
    }

    //==============================================================================
    /** Returns the CriticalSection that locks this structure.
        To lock, you can call getLock().enter() and getLock().exit(), or preferably use
        an object of ScopedLockType as an RAII lock for it.
    */
    inline const TypeOfCriticalSectionToUse& getLock() const noexcept      { return lock; }

    /** Returns the type of scoped lock to use for locking this array */
    typedef typename TypeOfCriticalSectionToUse::ScopedLockType ScopedLockType;

private:
    //==============================================================================
    class HashEntry
    {
    public:
        HashEntry (KeyTypeParameter k, ValueTypeParameter val, HashEntry* const next)
            : key (k), value (val), nextEntry (next)
        {}

        const KeyType key;
        ValueType value;
        HashEntry* nextEntry;

        JUCE_DECLARE_NON_COPYABLE (HashEntry)
    };

public:
    //==============================================================================
    /** Iterates over the items in a HashMap.

        To use it, repeatedly call next() until it returns false, e.g.
        @code
        HashMap <String, String> myMap;

        HashMap<String, String>::Iterator i (myMap);

        while (i.next())
        {
            DBG (i.getKey() << " -> " << i.getValue());
        }
        @endcode

        The order in which items are iterated bears no resemblence to the order in which
        they were originally added!

        Obviously as soon as you call any non-const methods on the original hash-map, any
        iterators that were created beforehand will cease to be valid, and should not be used.

        @see HashMap
    */
    class Iterator
    {
    public:
        //==============================================================================
        Iterator (const HashMap& hashMapToIterate)
            : hashMap (hashMapToIterate), entry (nullptr), index (0)
        {}

        /** Moves to the next item, if one is available.
            When this returns true, you can get the item's key and value using getKey() and
            getValue(). If it returns false, the iteration has finished and you should stop.
        */
        bool next()
        {
            if (entry != nullptr)
                entry = entry->nextEntry;

            while (entry == nullptr)
            {
                if (index >= hashMap.getNumSlots())
                    return false;

                entry = hashMap.slots.getUnchecked (index++);
            }

            return true;
        }

        /** Returns the current item's key.
            This should only be called when a call to next() has just returned true.
        */
        KeyType getKey() const
        {
            return entry != nullptr ? entry->key : KeyType();
        }

        /** Returns the current item's value.
            This should only be called when a call to next() has just returned true.
        */
        ValueType getValue() const
        {
            return entry != nullptr ? entry->value : ValueType();
        }

    private:
        //==============================================================================
        const HashMap& hashMap;
        HashEntry* entry;
        int index;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Iterator)
    };

private:
    //==============================================================================
    enum { defaultHashTableSize = 101 };
    friend class Iterator;

    HashFunctionType hashFunctionToUse;
    Array <HashEntry*> slots;
    int totalNumItems;
    TypeOfCriticalSectionToUse lock;

    int generateHashFor (KeyTypeParameter key) const
    {
        const int hash = hashFunctionToUse.generateHash (key, getNumSlots());
        jassert (isPositiveAndBelow (hash, getNumSlots())); // your hash function is generating out-of-range numbers!
        return hash;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HashMap)
};


#endif   // JUCE_HASHMAP_H_INCLUDED
