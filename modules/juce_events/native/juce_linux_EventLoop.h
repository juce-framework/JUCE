#ifndef JUCE_LINUX_EVENTLOOP_H_INCLUDED
#define JUCE_LINUX_EVENTLOOP_H_INCLUDED

namespace LinuxEventLoop
{
    struct CallbackFunctionBase
    {
        virtual ~CallbackFunctionBase() {}
        virtual bool operator()(int fd) = 0;
        bool active = true;
    };

    template <typename FdCallbackFunction>
    struct CallbackFunction : public CallbackFunctionBase
    {
        FdCallbackFunction callback;

        CallbackFunction (FdCallbackFunction c) : callback (c) {}

        bool operator() (int fd) override { return callback (fd); }
    };

    template <typename FdCallbackFunction>
    void setWindowSystemFd (int fd, FdCallbackFunction readCallback)
    {
        setWindowSystemFdInternal (fd, new CallbackFunction<FdCallbackFunction> (readCallback));
    }
    void removeWindowSystemFd() noexcept;

    void setWindowSystemFdInternal (int fd, CallbackFunctionBase* readCallback) noexcept;
}

#endif /* JUCE_LINUX_EVENTLOOP_H_INCLUDED */
