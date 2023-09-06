namespace juce {

    bool File::copyInternal (const File& dest) const
    {
        return false;
    }

    void File::findFileSystemRoots (Array<File>& destArray)
    {

    }

    bool File::isHidden() const
    {
        return false;
    }

    bool File::isSymbolicLink() const
    {
        return false;
    }

    String File::getNativeLinkedTarget() const
    {
        return "";
    }

//==============================================================================
    class DirectoryIterator::NativeIterator::Pimpl
    {
    public:
        Pimpl (const File& directory, const String& wc)
                : parentDir (File::addTrailingSeparator (directory.getFullPathName())),
                  wildCard (wc)
        {
        }

        ~Pimpl()
        {
        }

        bool next (String& filenameFound,
                   bool* const isDir, bool* const isHidden, int64* const fileSize,
                   Time* const modTime, Time* const creationTime, bool* const isReadOnly)
        {
            return false;
        }

    private:
        String parentDir, wildCard;
        JUCE_DECLARE_NON_COPYABLE (Pimpl)
    };

    DirectoryIterator::NativeIterator::NativeIterator (const File& directory, const String& wildCardStr)
            : pimpl (new DirectoryIterator::NativeIterator::Pimpl (directory, wildCardStr))
    {
    }

    DirectoryIterator::NativeIterator::~NativeIterator() {}

    bool DirectoryIterator::NativeIterator::next (String& filenameFound,
                                                  bool* isDir, bool* isHidden, int64* fileSize,
                                                  Time* modTime, Time* creationTime, bool* isReadOnly)
    {
        return pimpl->next (filenameFound, isDir, isHidden, fileSize, modTime, creationTime, isReadOnly);
    }


    bool File::isOnCDRomDrive() const
    {
        return false;
    }
    bool File::isOnHardDisk() const
    {
        return true;
    }
    bool File::isOnRemovableDrive() const
    {
        return false;
    }
    String File::getVersion() const
    {
        return {}; // xxx not yet implemented
    }
    static File resolveXDGFolder (const char* const type, const char* const fallbackFolder)
    {
        return File();
    }
    File File::getSpecialLocation (const SpecialLocationType type)
    {
        return File();
    }

    bool File::moveToTrash() const
    {
        return false;
    }

    int64 File::getBytesFreeOnVolume() const
    {
        return 0;
    }


    bool NamedPipe::isOpen() const { return false;}
    NamedPipe::~NamedPipe() {}


    class NamedPipe::Pimpl
    {
    public:
        Pimpl (const String& pipePath, bool createPipe)
        {

        }

        ~Pimpl()
        {
        }

        bool connect (int timeOutMilliseconds)
        {

        }

        int read (char* destBuffer, int maxBytesToRead, int timeOutMilliseconds)
        {

        }

        int write (const char* sourceBuffer, int numBytesToWrite, int timeOutMilliseconds)
        {

            return 0;
        }

        static bool createFifo (const String& name, bool mustNotExist)
        {
            return false;
        }

        bool createFifos (bool mustNotExist)
        {
            return false;
        }

        static constexpr auto invalidPipe = -1;

        class PipeDescriptor
        {
        public:
            template <typename Fn>
            int get (Fn&& fn)
            {

            }

            void close()
            {

            }

            int get()
            {

            }

        private:
        };


    private:
        static void signalHandler (int) {}

        static uint32 getTimeoutEnd (int timeOutMilliseconds)
        {
            return 0;
        }

        static bool hasExpired (uint32 timeoutEnd)
        {
            return false;
        }

        int openPipe (const String& name, int flags, uint32 timeoutEnd)
        {
            return 0;
        }

        int openPipe (bool isInput, uint32 timeoutEnd)
        {
           return 0;
        }

        static void waitForInput (int handle, int timeoutMsecs) noexcept
        {

        }

        static void waitToWrite (int handle, int timeoutMsecs) noexcept
        {

        }
    };

    NamedPipe::NamedPipe() {}

    bool NamedPipe::createNewPipe(const juce::String &pipeName, bool mustNotExist) {
        return false;
    }


    void NamedPipe::close()
    {



    }

    bool NamedPipe::openInternal (const String& pipeName, bool createPipe, bool mustNotExist)
    {

        return true;
    }

    int NamedPipe::read (void* destBuffer, int maxBytesToRead, int timeOutMilliseconds)
    {

    }

    int NamedPipe::write (const void* sourceBuffer, int numBytesToWrite, int timeOutMilliseconds)
    {
    }


    class ChildProcess::ActiveProcess
    {
    public:
        ActiveProcess (const StringArray& arguments, int streamFlags)
        {

        }

        ~ActiveProcess()
        {

        }

        bool isRunning() noexcept
        {
           return true;
        }

        int read (void* dest, int numBytes) noexcept
        {

            return 0;
        }

        bool killProcess() const noexcept
        {
            return true;
        }

        uint32 getExitCode() noexcept
        {


            return 0;
        }

    };

    ChildProcess::ChildProcess() {}

    bool ChildProcess::start(const juce::String &command, int streamFlags) {
        return false;
    }

    bool ChildProcess::start(const juce::StringArray &arguments, int streamFlags) {return false;}

    ChildProcess::~ChildProcess() {}

    void StreamingSocket::close() {}
    int StreamingSocket::waitUntilReady(bool readyForReading, int timeoutMsecs) {return 0;}
    StreamingSocket::~StreamingSocket() {}

    int StreamingSocket::write(const void *sourceBuffer, int numBytesToWrite) {}

    int StreamingSocket::read(void *destBuffer, int maxBytesToRead, bool blockUntilSpecifiedAmountHasArrived) {}

}