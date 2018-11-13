#pragma once

#include "JuceHeader.h"

namespace juce
{

template<typename T>
class SafeRef : public ReferenceCountedObject
{
public:
    typedef ReferenceCountedObjectPtr<SafeRef> Ptr;

    SafeRef (T* owner = nullptr)
        : owner (owner)
    {
    }

    ~SafeRef()
    {
        // If owner_ wasn't reset to nullptr then the user forgot to call reset()
        // in their destructor.
        jassert (owner == nullptr);
    }

    void reset (T* newOwner = nullptr)
    {
        ScopedWriteLock l (lock);
        owner = newOwner;
    }

    T* get () const
    {
        return owner;
    }

    // A scoped read-only access to the reference.
    // For additional write access one may use additional ScopedWriteLock on the reference's `lock`.
    class ScopedAccess
    {
    public:
        ScopedAccess (const Ptr& otherRef, bool tryLock = false)
            : ref (*otherRef)
        {
            if (tryLock)
                didLock = ref.lock.tryEnterRead();
            else
            {
                ref.lock.enterRead();
                didLock = true;
            }
            owner = didLock ? ref.owner : nullptr;
        }

        ~ScopedAccess()
        {
            if (didLock)
                ref.lock.exitRead();
        }

        T* operator*()
        {
            return owner;
        }

        T* operator->()
        {
            return owner;
        }

        operator bool() const
        {
            return owner != nullptr;
        }

    private:
        SafeRef& ref;
        bool didLock;
        T* owner;
    };

    // The lock is public, so owner can use `enterWrite` etc with it freely.
    ReadWriteLock lock;

private:
    T* owner;
};

} // namespace juce
