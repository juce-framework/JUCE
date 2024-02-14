/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

namespace WindowsMediaCodec
{

class JuceIStream final : public ComBaseClassHelper<IStream>
{
public:
    JuceIStream (InputStream& in) noexcept
        : ComBaseClassHelper (0), source (in)
    {
    }

    JUCE_COMRESULT Commit (DWORD)                                        override { return S_OK; }
    JUCE_COMRESULT Write (const void*, ULONG, ULONG*)                    override { return E_NOTIMPL; }
    JUCE_COMRESULT Clone (IStream**)                                     override { return E_NOTIMPL; }
    JUCE_COMRESULT SetSize (ULARGE_INTEGER)                              override { return E_NOTIMPL; }
    JUCE_COMRESULT Revert()                                              override { return E_NOTIMPL; }
    JUCE_COMRESULT LockRegion (ULARGE_INTEGER, ULARGE_INTEGER, DWORD)    override { return E_NOTIMPL; }
    JUCE_COMRESULT UnlockRegion (ULARGE_INTEGER, ULARGE_INTEGER, DWORD)  override { return E_NOTIMPL; }

    JUCE_COMRESULT Read (void* dest, ULONG numBytes, ULONG* bytesRead) override
    {
        auto numRead = source.read (dest, (size_t) numBytes);

        if (bytesRead != nullptr)
            *bytesRead = (ULONG) numRead;

        return (numRead == (int) numBytes) ? S_OK : S_FALSE;
    }

    JUCE_COMRESULT Seek (LARGE_INTEGER position, DWORD origin, ULARGE_INTEGER* resultPosition) override
    {
        auto newPos = (int64) position.QuadPart;

        if (origin == STREAM_SEEK_CUR)
        {
            newPos += source.getPosition();
        }
        else if (origin == STREAM_SEEK_END)
        {
            auto len = source.getTotalLength();

            if (len < 0)
                return E_NOTIMPL;

            newPos += len;
        }

        if (resultPosition != nullptr)
            resultPosition->QuadPart = (ULONGLONG) newPos;

        return source.setPosition (newPos) ? S_OK : E_NOTIMPL;
    }

    JUCE_COMRESULT CopyTo (IStream* destStream, ULARGE_INTEGER numBytesToDo,
                           ULARGE_INTEGER* bytesRead, ULARGE_INTEGER* bytesWritten) override
    {
        uint64 totalCopied = 0;
        auto numBytes = (int64) numBytesToDo.QuadPart;

        while (numBytes > 0 && ! source.isExhausted())
        {
            char buffer [1024];

            auto numToCopy = (int) jmin ((int64) sizeof (buffer), (int64) numBytes);
            auto numRead = source.read (buffer, numToCopy);

            if (numRead <= 0)
                break;

            destStream->Write (buffer, (ULONG) numRead, nullptr);
            totalCopied += (ULONG) numRead;
        }

        if (bytesRead != nullptr)      bytesRead->QuadPart = totalCopied;
        if (bytesWritten != nullptr)   bytesWritten->QuadPart = totalCopied;

        return S_OK;
    }

    JUCE_COMRESULT Stat (STATSTG* stat, DWORD) override
    {
        if (stat == nullptr)
            return STG_E_INVALIDPOINTER;

        zerostruct (*stat);
        stat->type = STGTY_STREAM;
        stat->cbSize.QuadPart = (ULONGLONG) jmax ((int64) 0, source.getTotalLength());
        return S_OK;
    }

private:
    InputStream& source;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JuceIStream)
};

//==============================================================================
static const char* wmFormatName = "Windows Media";
static const char* const extensions[] = { ".mp3", ".wmv", ".asf", ".wm", ".wma", nullptr };

//==============================================================================
class WMAudioReader final : public AudioFormatReader
{
public:
    WMAudioReader (InputStream* const input_)
        : AudioFormatReader (input_, TRANS (wmFormatName)),
          wmvCoreLib ("Wmvcore.dll")
    {
        JUCE_LOAD_WINAPI_FUNCTION (wmvCoreLib, WMCreateSyncReader, wmCreateSyncReader,
                                   HRESULT, (IUnknown*, DWORD, IWMSyncReader**))

        if (wmCreateSyncReader != nullptr)
        {
            checkCoInitialiseCalled();

            HRESULT hr = wmCreateSyncReader (nullptr, WMT_RIGHT_PLAYBACK, wmSyncReader.resetAndGetPointerAddress());

            if (SUCCEEDED (hr))
                hr = wmSyncReader->OpenStream (new JuceIStream (*input));

            if (SUCCEEDED (hr))
            {
                WORD streamNum = 1;
                hr = wmSyncReader->GetStreamNumberForOutput (0, &streamNum);
                hr = wmSyncReader->SetReadStreamSamples (streamNum, false);

                scanFileForDetails();
            }
        }
    }

    ~WMAudioReader() override
    {
        if (wmSyncReader != nullptr)
            wmSyncReader->Close();
    }

    bool readSamples (int* const* destSamples, int numDestChannels, int startOffsetInDestBuffer,
                      int64 startSampleInFile, int numSamples) override
    {
        if (sampleRate <= 0)
            return false;

        checkCoInitialiseCalled();

        clearSamplesBeyondAvailableLength (destSamples, numDestChannels, startOffsetInDestBuffer,
                                           startSampleInFile, numSamples, lengthInSamples);

        const auto stride = (int) (numChannels * sizeof (int16));

        while (numSamples > 0)
        {
            if (! bufferedRange.contains (startSampleInFile))
            {
                const bool hasJumped = (startSampleInFile != bufferedRange.getEnd());

                if (hasJumped)
                    wmSyncReader->SetRange ((QWORD) (startSampleInFile * 10000000 / (int64) sampleRate), 0);

                ComSmartPtr<INSSBuffer> sampleBuffer;
                QWORD sampleTime, duration;
                DWORD flags, outputNum;
                WORD streamNum;

                HRESULT hr = wmSyncReader->GetNextSample (1, sampleBuffer.resetAndGetPointerAddress(),
                                                          &sampleTime, &duration, &flags, &outputNum, &streamNum);

                if (sampleBuffer != nullptr)
                {
                    BYTE* rawData = nullptr;
                    DWORD dataLength = 0;
                    hr = sampleBuffer->GetBufferAndLength (&rawData, &dataLength);

                    if (dataLength == 0)
                        return false;

                    if (hasJumped)
                        bufferedRange.setStart ((int64) ((sampleTime * (QWORD) sampleRate) / 10000000));
                    else
                        bufferedRange.setStart (bufferedRange.getEnd()); // (because the positions returned often aren't contiguous)

                    bufferedRange.setLength ((int64) dataLength / (int64) stride);

                    buffer.ensureSize ((size_t) dataLength);
                    memcpy (buffer.getData(), rawData, (size_t) dataLength);
                }
                else if (hr == NS_E_NO_MORE_SAMPLES)
                {
                    bufferedRange.setStart (startSampleInFile);
                    bufferedRange.setLength (256);
                    buffer.ensureSize (256 * (size_t) stride);
                    buffer.fillWith (0);
                }
                else
                {
                    return false;
                }
            }

            auto offsetInBuffer = (int) (startSampleInFile - bufferedRange.getStart());
            auto* rawData = static_cast<const int16*> (addBytesToPointer (buffer.getData(), offsetInBuffer * stride));
            auto numToDo = jmin (numSamples, (int) (bufferedRange.getLength() - offsetInBuffer));

            for (int i = 0; i < numDestChannels; ++i)
            {
                JUCE_BEGIN_IGNORE_WARNINGS_MSVC (28182)
                jassert (destSamples[i] != nullptr);

                auto srcChan = jmin (i, (int) numChannels - 1);
                const int16* src = rawData + srcChan;
                int* const dst = destSamples[i] + startOffsetInDestBuffer;

                for (int j = 0; j < numToDo; ++j)
                {
                    dst[j] = (int) (((uint32) *src) << 16);
                    src += numChannels;
                }
                JUCE_END_IGNORE_WARNINGS_MSVC
            }

            startSampleInFile += numToDo;
            startOffsetInDestBuffer += numToDo;
            numSamples -= numToDo;
        }

        return true;
    }

private:
    DynamicLibrary wmvCoreLib;
    ComSmartPtr<IWMSyncReader> wmSyncReader;
    MemoryBlock buffer;
    Range<int64> bufferedRange;

    void checkCoInitialiseCalled()
    {
        [[maybe_unused]] const auto result = CoInitialize (nullptr);
    }

    void scanFileForDetails()
    {
        if (auto wmHeaderInfo = wmSyncReader.getInterface<IWMHeaderInfo>())
        {
            QWORD lengthInNanoseconds = 0;
            WORD lengthOfLength = sizeof (lengthInNanoseconds);
            WORD streamNum = 0;
            WMT_ATTR_DATATYPE wmAttrDataType;
            wmHeaderInfo->GetAttributeByName (&streamNum, L"Duration", &wmAttrDataType,
                                              (BYTE*) &lengthInNanoseconds, &lengthOfLength);

            if (auto wmProfile = wmSyncReader.getInterface<IWMProfile>())
            {
                ComSmartPtr<IWMStreamConfig> wmStreamConfig;
                auto hr = wmProfile->GetStream (0, wmStreamConfig.resetAndGetPointerAddress());

                if (SUCCEEDED (hr))
                {
                    if (auto wmMediaProperties = wmStreamConfig.getInterface<IWMMediaProps>())
                    {
                        DWORD sizeMediaType;
                        hr = wmMediaProperties->GetMediaType (nullptr, &sizeMediaType);

                        HeapBlock<WM_MEDIA_TYPE> mediaType;
                        mediaType.malloc (sizeMediaType, 1);
                        hr = wmMediaProperties->GetMediaType (mediaType, &sizeMediaType);

                        if (mediaType->majortype == WMMEDIATYPE_Audio)
                        {
                            auto* inputFormat = reinterpret_cast<WAVEFORMATEX*> (mediaType->pbFormat);

                            sampleRate = inputFormat->nSamplesPerSec;
                            numChannels = inputFormat->nChannels;
                            bitsPerSample = inputFormat->wBitsPerSample != 0 ? inputFormat->wBitsPerSample : 16;
                            lengthInSamples = (lengthInNanoseconds * (QWORD) sampleRate) / 10000000;
                        }
                    }
                }
            }
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WMAudioReader)
};

}

//==============================================================================
WindowsMediaAudioFormat::WindowsMediaAudioFormat()
    : AudioFormat (TRANS (WindowsMediaCodec::wmFormatName),
                   StringArray (WindowsMediaCodec::extensions))
{
}

WindowsMediaAudioFormat::~WindowsMediaAudioFormat() = default;

Array<int> WindowsMediaAudioFormat::getPossibleSampleRates()    { return {}; }
Array<int> WindowsMediaAudioFormat::getPossibleBitDepths()      { return {}; }

bool WindowsMediaAudioFormat::canDoStereo()     { return true; }
bool WindowsMediaAudioFormat::canDoMono()       { return true; }
bool WindowsMediaAudioFormat::isCompressed()    { return true; }

//==============================================================================
AudioFormatReader* WindowsMediaAudioFormat::createReaderFor (InputStream* sourceStream, bool deleteStreamIfOpeningFails)
{
    std::unique_ptr<WindowsMediaCodec::WMAudioReader> r (new WindowsMediaCodec::WMAudioReader (sourceStream));

    if (r->sampleRate > 0)
        return r.release();

    if (! deleteStreamIfOpeningFails)
        r->input = nullptr;

    return nullptr;
}

AudioFormatWriter* WindowsMediaAudioFormat::createWriterFor (OutputStream* /*streamToWriteTo*/, double /*sampleRateToUse*/,
                                                             unsigned int /*numberOfChannels*/, int /*bitsPerSample*/,
                                                             const StringPairArray& /*metadataValues*/, int /*qualityOptionIndex*/)
{
    jassertfalse; // not yet implemented!
    return nullptr;
}

} // namespace juce
