#pragma once

#include "juce_ARA_audio_plugin.h"

namespace juce
{

class ARAHostArchiveInputStream : public InputStream
{
public:
    ARAHostArchiveInputStream (ARA::PlugIn::HostArchiveReader*);

    int64 getPosition() override { return (int64) position; }
    int64 getTotalLength() override { return (int64) size; }

    bool setPosition (int64) override;
    bool isExhausted() override;
    int read (void*, int) override;

private:
    ARA::PlugIn::HostArchiveReader* archiveReader;
    size_t position, size;
};

class ARAHostArchiveOutputStream : public OutputStream
{
public:
    ARAHostArchiveOutputStream (ARA::PlugIn::HostArchiveWriter*);

    int64 getPosition() override { return (int64) position; }
    void flush() override {}

    bool setPosition (int64) override;
    bool write (const void*, size_t) override;

private:
    ARA::PlugIn::HostArchiveWriter* archiveWriter;
    size_t position;
};

} // namespace juce
