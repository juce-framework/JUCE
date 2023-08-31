namespace juce {

    std::unique_ptr<Desktop::NativeDarkModeChangeDetectorImpl> Desktop::createNativeDarkModeChangeDetectorImpl()
    {
        return nullptr;
    }

    class MouseCursor::PlatformSpecificHandle {
    public:
        explicit PlatformSpecificHandle(const MouseCursor::StandardCursorType type) {

        }

        explicit PlatformSpecificHandle(const detail::CustomMouseCursorInfo &info) {}

        static void showInWindow(PlatformSpecificHandle *handle, ComponentPeer *peer) {

        }

    };
}