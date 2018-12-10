#include "juce_ARAStreams.h"

namespace juce
{

ARAHostArchiveInputStream::ARAHostArchiveInputStream (ARA::PlugIn::HostArchiveReader* reader)
: archiveReader (reader), position (0), size (reader->getArchiveSize())
{
}

int ARAHostArchiveInputStream::read (void* destBuffer, int maxBytesToRead)
{
    const int bytesToRead = std::min (maxBytesToRead, (int) (size - position));
    const int result =
        archiveReader->readBytesFromArchive (
            position,
            bytesToRead,
            (ARA::ARAByte*) destBuffer)
        ? bytesToRead
        : 0;
    position += result;
    return result;
}

bool ARAHostArchiveInputStream::setPosition (int64 newPosition)
{
    if (newPosition >= (int64) size)
        return false;
    position = (size_t) newPosition;
    return true;
}

bool ARAHostArchiveInputStream::isExhausted()
{
    return position >= size;
}

ARAHostArchiveOutputStream::ARAHostArchiveOutputStream (ARA::PlugIn::HostArchiveWriter* writer)
: archiveWriter (writer), position (0)
{
}

bool ARAHostArchiveOutputStream::write (const void* dataToWrite, size_t numberOfBytes)
{
    if (! archiveWriter->writeBytesToArchive (
            position,
            numberOfBytes,
            (const ARA::ARAByte*) dataToWrite))
        return false;
    position += numberOfBytes;
    return true;
}

bool ARAHostArchiveOutputStream::setPosition (int64 newPosition)
{
    if (newPosition > (int64) std::numeric_limits<size_t>::max())
        return false;
    position = (size_t) newPosition;
    return true;
}

} // namespace juce
