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
class Animator::Impl
{
public:
    virtual ~Impl() = default;

    virtual double getDurationMs() const
    {
        return 0.0;
    }

    void start()
    {
        shouldStart = true;
        shouldComplete = false;
    }

    void complete()
    {
        shouldComplete = true;
    }

    Animator::Status update (double timestampMs)
    {
        if (std::exchange (shouldStart, false))
        {
            onStart (timestampMs);
            running = true;
        }

        if (! running)
            return Animator::Status::idle;

        const auto status = internalUpdate (timestampMs);

        if (status != Animator::Status::finished && ! shouldComplete)
            return status;

        shouldComplete = false;
        running = false;
        onComplete();
        return Animator::Status::finished;
    }

    virtual bool isComplete() const { return shouldComplete; }

    virtual void onStart (double timeStampMs) = 0;
    virtual void onComplete() = 0;
    virtual Animator::Status internalUpdate (double timestampMs) = 0;

    bool shouldStart = false, shouldComplete = false, running = false;
};

//==============================================================================
Animator::Animator (std::shared_ptr<Impl> ai)
    : ptr (std::move (ai))
{
    jassert (ptr != nullptr);
}

double Animator::getDurationMs() const
{
    return ptr->getDurationMs();
}

void Animator::start() const
{
    ptr->start();
}

void Animator::complete() const
{
    ptr->complete();
}

Animator::Status Animator::update (double timestampMs) const
{
    return ptr->update (timestampMs);
}

bool Animator::isComplete() const
{
    return ptr->isComplete();
}

//==============================================================================
#if JUCE_UNIT_TESTS

struct AnimatorTests  : public UnitTest
{
    AnimatorTests()
        : UnitTest ("Animator", UnitTestCategories::gui)
    {
    }

    struct TestEvents
    {
        enum class TestEventType
        {
            animatorStarted,
            animatorEnded
        };

        struct TestEvent
        {
            TestEvent (TestEventType e, int a) : eventType (e), animatorId (a) {}

            TestEventType eventType;
            int animatorId;
        };

        void started (int animatorId)
        {
            events.emplace_back (TestEventType::animatorStarted, animatorId);
        }

        void ended (int animatorId)
        {
            events.emplace_back (TestEventType::animatorEnded, animatorId);
        }

        bool animatorStartedBeforeAnotherStarted (int animator, int before)
        {
            for (const auto& event : events)
            {
                if (event.animatorId == before && event.eventType == TestEventType::animatorStarted)
                    return false;

                if (event.animatorId == animator && event.eventType == TestEventType::animatorStarted)
                    return true;
            }

            return false;
        }

        bool animatorStartedBeforeAnotherEnded (int animator, int before)
        {
            for (const auto& event : events)
            {
                if (event.animatorId == before && event.eventType == TestEventType::animatorEnded)
                    return false;

                if (event.animatorId == animator && event.eventType == TestEventType::animatorStarted)
                    return true;
            }

            return false;
        }

    private:
        std::vector<TestEvent> events;
    };

    void runTest() override
    {
        beginTest ("AnimatorSet starts end ends Animators in the right order");
        {
            TestEvents events;

            auto createTestAnimator = [&events] (int animatorId)
            {
                return ValueAnimatorBuilder {}
                    .withOnStartCallback ([&events, animatorId]
                                          {
                                              events.started (animatorId);
                                              return [] (auto) {};
                                          })
                    .withOnCompleteCallback ([&events, animatorId]
                                             {
                                                 events.ended (animatorId);
                                             })
                    .build();
            };

            auto stage1 = AnimatorSetBuilder { createTestAnimator (1) };
            stage1.followedBy (createTestAnimator (2));
            stage1.togetherWith (createTestAnimator (3));

            Animator animator = stage1.build();
            animator.start();

            for (double timeMs = 0.0; animator.update (timeMs) != Animator::Status::finished; timeMs += 16.667)
                ;

            expect (events.animatorStartedBeforeAnotherEnded (1, 2));
            expect (! events.animatorStartedBeforeAnotherEnded (2, 1));
            expect (events.animatorStartedBeforeAnotherEnded (3, 1));
            expect (events.animatorStartedBeforeAnotherStarted (3, 2));
        }
    }
};

static AnimatorTests animatorTests;

#endif

} // namespace juce
