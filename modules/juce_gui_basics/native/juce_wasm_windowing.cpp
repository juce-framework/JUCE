namespace juce {
    class Desktop::NativeDarkModeChangeDetectorImpl{

    };
    std::unique_ptr<Desktop::NativeDarkModeChangeDetectorImpl> Desktop::createNativeDarkModeChangeDetectorImpl()
    {
        return nullptr;
    }

    void Desktop::setScreenSaverEnabled(bool) {

    }

    bool Desktop::canUseSemiTransparentWindows() noexcept {return true;}

    double Desktop::getDefaultMasterScale() {return 1.0;}

    void Displays::findDisplays(float masterScale) {}

    Typeface::Ptr Typeface::createSystemTypefaceFor(const juce::Font &font) {}

    Typeface::Ptr Font::getDefaultTypefaceForFont(const juce::Font &font) {}

    bool Process::isForegroundProcess() {
        return true;
    }

    bool Thread::createNativeThread(juce::Thread::Priority) {
        return false;
    }

    void SystemClipboard::copyTextToClipboard(const juce::String &text) {}

    String SystemClipboard::getTextFromClipboard() {}

    bool WindowUtils::areThereAnyAlwaysOnTopWindows() {
        return true;
    }

    void MouseInputSource::setRawMousePosition(Point<float>) {}

    Point<float> MouseInputSource::getCurrentRawMousePosition() {return Point<float>();}

    ComponentPeer *Component::createNewPeer(int styleFlags, void *nativeWindowToAttachTo) {}

    void LookAndFeel::playAlertSound() {}

    ImagePixelData::Ptr NativeImageType::create(Image::PixelFormat, int width, int height, bool clearImage) const {}


    bool TextLayout::createNativeLayout(const juce::AttributedString &) { return true;}


    class MouseCursor::PlatformSpecificHandle {
    public:
        explicit PlatformSpecificHandle(const MouseCursor::StandardCursorType type) {

        }

        explicit PlatformSpecificHandle(const detail::CustomMouseCursorInfo &info) {}

        static void showInWindow(PlatformSpecificHandle *handle, ComponentPeer *peer) {

        }

    };

    void MemoryMappedFile::openInternal(const juce::File &, juce::MemoryMappedFile::AccessMode, bool) {}
    MemoryMappedFile::~MemoryMappedFile() {

    }
    bool KeyPress::isKeyCurrentlyDown(int keyCode) {return false;}


    const int extendedKeyModifier               = 0x10000;

    const int KeyPress::spaceKey                = 0x0001;
    const int KeyPress::returnKey               = 0x0001;
    const int KeyPress::escapeKey               = 0x0001;
    const int KeyPress::backspaceKey            = 0x0001;
    const int KeyPress::deleteKey               = 0x0001         | extendedKeyModifier;
    const int KeyPress::insertKey               = 0x0001         | extendedKeyModifier;
    const int KeyPress::tabKey                  = 0x0001;
    const int KeyPress::leftKey                 = 0x0001           | extendedKeyModifier;
    const int KeyPress::rightKey                = 0x0001          | extendedKeyModifier;
    const int KeyPress::upKey                   = 0x0001             | extendedKeyModifier;
    const int KeyPress::downKey                 = 0x0001           | extendedKeyModifier;
    const int KeyPress::homeKey                 = 0x0001           | extendedKeyModifier;
    const int KeyPress::endKey                  = 0x0001            | extendedKeyModifier;
    const int KeyPress::pageUpKey               = 0x0001          | extendedKeyModifier;
    const int KeyPress::pageDownKey             = 0x0001           | extendedKeyModifier;
    const int KeyPress::F1Key                   = 0x0001             | extendedKeyModifier;
    const int KeyPress::F2Key                   = 0x0001             | extendedKeyModifier;
    const int KeyPress::F3Key                   = 0x0001             | extendedKeyModifier;
    const int KeyPress::F4Key                   = 0x0001             | extendedKeyModifier;
    const int KeyPress::F5Key                   = 0x0001             | extendedKeyModifier;
    const int KeyPress::F6Key                   = 0x0001             | extendedKeyModifier;
    const int KeyPress::F7Key                   = 0x0001             | extendedKeyModifier;
    const int KeyPress::F8Key                   = 0x0001             | extendedKeyModifier;
    const int KeyPress::F9Key                   = 0x0001             | extendedKeyModifier;
    const int KeyPress::F10Key                  = 0x0001            | extendedKeyModifier;
    const int KeyPress::F11Key                  = 0x0001            | extendedKeyModifier;
    const int KeyPress::F12Key                  = 0x0001            | extendedKeyModifier;
    const int KeyPress::F13Key                  = 0x0001            | extendedKeyModifier;
    const int KeyPress::F14Key                  = 0x0001            | extendedKeyModifier;
    const int KeyPress::F15Key                  = 0x0001            | extendedKeyModifier;
    const int KeyPress::F16Key                  = 0x0001            | extendedKeyModifier;
    const int KeyPress::F17Key                  = 0x0001            | extendedKeyModifier;
    const int KeyPress::F18Key                  = 0x0001            | extendedKeyModifier;
    const int KeyPress::F19Key                  = 0x0001            | extendedKeyModifier;
    const int KeyPress::F20Key                  = 0x0001            | extendedKeyModifier;
    const int KeyPress::F21Key                  = 0x0001            | extendedKeyModifier;
    const int KeyPress::F22Key                  = 0x0001            | extendedKeyModifier;
    const int KeyPress::F23Key                  = 0x0001            | extendedKeyModifier;
    const int KeyPress::F24Key                  = 0x0001            | extendedKeyModifier;
    const int KeyPress::F25Key                  = 0x31000;          // Windows doesn't support F-keys 25 or higher
    const int KeyPress::F26Key                  = 0x31001;
    const int KeyPress::F27Key                  = 0x31002;
    const int KeyPress::F28Key                  = 0x31003;
    const int KeyPress::F29Key                  = 0x31004;
    const int KeyPress::F30Key                  = 0x31005;
    const int KeyPress::F31Key                  = 0x31006;
    const int KeyPress::F32Key                  = 0x31007;
    const int KeyPress::F33Key                  = 0x31008;
    const int KeyPress::F34Key                  = 0x31009;
    const int KeyPress::F35Key                  = 0x3100a;

    const int KeyPress::numberPad0              = 0x0001        | extendedKeyModifier;
    const int KeyPress::numberPad1              = 0x0001        | extendedKeyModifier;
    const int KeyPress::numberPad2              = 0x0001        | extendedKeyModifier;
    const int KeyPress::numberPad3              = 0x0001        | extendedKeyModifier;
    const int KeyPress::numberPad4              = 0x0001        | extendedKeyModifier;
    const int KeyPress::numberPad5              = 0x0001        | extendedKeyModifier;
    const int KeyPress::numberPad6              = 0x0001        | extendedKeyModifier;
    const int KeyPress::numberPad7              = 0x0001        | extendedKeyModifier;
    const int KeyPress::numberPad8              = 0x0001        | extendedKeyModifier;
    const int KeyPress::numberPad9              = 0x0001        | extendedKeyModifier;
    const int KeyPress::numberPadAdd            = 0x0001            | extendedKeyModifier;
    const int KeyPress::numberPadSubtract       = 0x0001       | extendedKeyModifier;
    const int KeyPress::numberPadMultiply       = 0x0001       | extendedKeyModifier;
    const int KeyPress::numberPadDivide         = 0x0001         | extendedKeyModifier;
    const int KeyPress::numberPadSeparator      = 0x0001      | extendedKeyModifier;
    const int KeyPress::numberPadDecimalPoint   = 0x0001        | extendedKeyModifier;
    const int KeyPress::numberPadEquals         = 0x92 /*0x0001*/  | extendedKeyModifier;
    const int KeyPress::numberPadDelete         = 0x0001         | extendedKeyModifier;
    const int KeyPress::playKey                 = 0x30000;
    const int KeyPress::stopKey                 = 0x30001;
    const int KeyPress::fastForwardKey          = 0x30002;
    const int KeyPress::rewindKey               = 0x30003;
}