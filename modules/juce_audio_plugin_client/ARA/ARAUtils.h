#pragma once

#include "JuceHeader.h"
#include <ARA_Library/PlugIn/ARAPlug.h>

namespace juce
{
    template<typename T>
    class SafeRef : public ReferenceCountedObject
    {
    public:
        typedef ReferenceCountedObjectPtr<SafeRef> Ptr;

        SafeRef (T* owner = nullptr)
            : owner_ (owner)
        {
        }

        ~SafeRef()
        {
            // If owner_ wasn't reset to nullptr then the user forgot to call reset()
            // in their destructor.
            jassert (owner_ == nullptr);
        }

        void reset (T* owner = nullptr)
        {
            ScopedWriteLock l (lock);
            owner_ = owner;
        }

        // A scoped read-only access to the reference.
        // For additional write access one may use additional ScopedWriteLock on the reference's `lock`.
        class ScopedAccess
        {
        public:
            ScopedAccess (const Ptr& ref, bool tryLock = false)
                : ref_ (*ref)
            {
                if (tryLock)
                    didLock = ref_.lock.tryEnterRead();
                else
                {
                    ref_.lock.enterRead();
                    didLock = true;
                }
                owner_ = didLock ? ref_.owner_ : nullptr;
            }

            ~ScopedAccess()
            {
                if (didLock)
                    ref_.lock.exitRead();
            }

            T* operator*()
            {
                return owner_;
            }

            T* operator->()
            {
                return owner_;
            }

            operator bool() const
            {
                return owner_ != nullptr;
            }

        private:
            SafeRef& ref_;
            bool didLock;
            T* owner_;
        };

        // The lock is public, so owner can use `enterWrite` etc with it freely.
        ReadWriteLock lock;

    private:
        T* owner_;
    };
}
