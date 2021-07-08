/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
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

namespace DragAndDropHelpers
{
    //==============================================================================
    struct JuceDropSource   : public ComBaseClassHelper<IDropSource>
    {
        JuceDropSource() {}

        JUCE_COMRESULT QueryContinueDrag (BOOL escapePressed, DWORD keys) override
        {
            if (escapePressed)
                return DRAGDROP_S_CANCEL;

            if ((keys & (MK_LBUTTON | MK_RBUTTON)) == 0)
                return DRAGDROP_S_DROP;

            return S_OK;
        }

        JUCE_COMRESULT GiveFeedback (DWORD) override
        {
            return DRAGDROP_S_USEDEFAULTCURSORS;
        }
    };

    //==============================================================================
    struct JuceEnumFormatEtc   : public ComBaseClassHelper<IEnumFORMATETC>
    {
        JuceEnumFormatEtc (const FORMATETC* f)  : format (f) {}

        JUCE_COMRESULT Clone (IEnumFORMATETC** result) override
        {
            if (result == nullptr)
                return E_POINTER;

            auto newOne = new JuceEnumFormatEtc (format);
            newOne->index = index;
            *result = newOne;
            return S_OK;
        }

        JUCE_COMRESULT Next (ULONG celt, LPFORMATETC lpFormatEtc, ULONG* pceltFetched) override
        {
            if (pceltFetched != nullptr)
                *pceltFetched = 0;
            else if (celt != 1)
                return S_FALSE;

            if (index == 0 && celt > 0 && lpFormatEtc != nullptr)
            {
                copyFormatEtc (lpFormatEtc [0], *format);
                ++index;

                if (pceltFetched != nullptr)
                    *pceltFetched = 1;

                return S_OK;
            }

            return S_FALSE;
        }

        JUCE_COMRESULT Skip (ULONG celt) override
        {
            if (index + (int) celt >= 1)
                return S_FALSE;

            index += (int) celt;
            return S_OK;
        }

        JUCE_COMRESULT Reset() override
        {
            index = 0;
            return S_OK;
        }

    private:
        const FORMATETC* const format;
        int index = 0;

        static void copyFormatEtc (FORMATETC& dest, const FORMATETC& source)
        {
            dest = source;

            if (source.ptd != nullptr)
            {
                dest.ptd = (DVTARGETDEVICE*) CoTaskMemAlloc (sizeof (DVTARGETDEVICE));

                if (dest.ptd != nullptr)
                    *(dest.ptd) = *(source.ptd);
            }
        }

        JUCE_DECLARE_NON_COPYABLE (JuceEnumFormatEtc)
    };

    //==============================================================================
    class JuceDataObject  : public ComBaseClassHelper <IDataObject>
    {
    public:
        JuceDataObject (const FORMATETC* f, const STGMEDIUM* m)
            : format (f), medium (m)
        {
        }

        ~JuceDataObject()
        {
            jassert (refCount == 0);
        }

        JUCE_COMRESULT GetData (FORMATETC* pFormatEtc, STGMEDIUM* pMedium)
        {
            if ((pFormatEtc->tymed & format->tymed) != 0
                 && pFormatEtc->cfFormat == format->cfFormat
                 && pFormatEtc->dwAspect == format->dwAspect)
            {
                pMedium->tymed = format->tymed;
                pMedium->pUnkForRelease = nullptr;

                if (format->tymed == TYMED_HGLOBAL)
                {
                    auto len = GlobalSize (medium->hGlobal);
                    void* const src = GlobalLock (medium->hGlobal);
                    void* const dst = GlobalAlloc (GMEM_FIXED, len);

                    if (src != nullptr && dst != nullptr)
                        memcpy (dst, src, len);

                    GlobalUnlock (medium->hGlobal);

                    pMedium->hGlobal = dst;
                    return S_OK;
                }
            }

            return DV_E_FORMATETC;
        }

        JUCE_COMRESULT QueryGetData (FORMATETC* f)
        {
            if (f == nullptr)
                return E_INVALIDARG;

            if (f->tymed == format->tymed
                  && f->cfFormat == format->cfFormat
                  && f->dwAspect == format->dwAspect)
                return S_OK;

            return DV_E_FORMATETC;
        }

        JUCE_COMRESULT GetCanonicalFormatEtc (FORMATETC*, FORMATETC* pFormatEtcOut)
        {
            pFormatEtcOut->ptd = nullptr;
            return E_NOTIMPL;
        }

        JUCE_COMRESULT EnumFormatEtc (DWORD direction, IEnumFORMATETC** result)
        {
            if (result == nullptr)
                return E_POINTER;

            if (direction == DATADIR_GET)
            {
                *result = new JuceEnumFormatEtc (format);
                return S_OK;
            }

            *result = nullptr;
            return E_NOTIMPL;
        }

        JUCE_COMRESULT GetDataHere (FORMATETC*, STGMEDIUM*)                  { return DATA_E_FORMATETC; }
        JUCE_COMRESULT SetData (FORMATETC*, STGMEDIUM*, BOOL)                { return E_NOTIMPL; }
        JUCE_COMRESULT DAdvise (FORMATETC*, DWORD, IAdviseSink*, DWORD*)     { return OLE_E_ADVISENOTSUPPORTED; }
        JUCE_COMRESULT DUnadvise (DWORD)                                     { return E_NOTIMPL; }
        JUCE_COMRESULT EnumDAdvise (IEnumSTATDATA**)                         { return OLE_E_ADVISENOTSUPPORTED; }

    private:
        const FORMATETC* const format;
        const STGMEDIUM* const medium;

        JUCE_DECLARE_NON_COPYABLE (JuceDataObject)
    };

    //==============================================================================
    HDROP createHDrop (const StringArray& fileNames)
    {
        size_t totalBytes = 0;
        for (int i = fileNames.size(); --i >= 0;)
            totalBytes += CharPointer_UTF16::getBytesRequiredFor (fileNames[i].getCharPointer()) + sizeof (WCHAR);

        struct Deleter
        {
            void operator() (void* ptr) const noexcept { GlobalFree (ptr); }
        };

        auto hDrop = std::unique_ptr<void, Deleter> ((HDROP) GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof (DROPFILES) + totalBytes + 4));

        if (hDrop != nullptr)
        {
            auto pDropFiles = (LPDROPFILES) GlobalLock (hDrop.get());

            if (pDropFiles == nullptr)
                return nullptr;

            pDropFiles->pFiles = sizeof (DROPFILES);
            pDropFiles->fWide = true;

            auto* fname = reinterpret_cast<WCHAR*> (addBytesToPointer (pDropFiles, sizeof (DROPFILES)));

            for (int i = 0; i < fileNames.size(); ++i)
            {
                auto bytesWritten = fileNames[i].copyToUTF16 (fname, 2048);
                fname = reinterpret_cast<WCHAR*> (addBytesToPointer (fname, bytesWritten));
            }

            *fname = 0;

            GlobalUnlock (hDrop.get());
        }

        return static_cast<HDROP> (hDrop.release());
    }

    struct DragAndDropJob   : public ThreadPoolJob
    {
        DragAndDropJob (FORMATETC f, STGMEDIUM m, DWORD d, std::function<void()>&& cb)
            : ThreadPoolJob ("DragAndDrop"),
              format (f), medium (m), whatToDo (d),
              completionCallback (std::move (cb))
        {
        }

        JobStatus runJob() override
        {
            ignoreUnused (OleInitialize (nullptr));

            auto* source = new JuceDropSource();
            auto* data = new JuceDataObject (&format, &medium);

            DWORD effect;
            DoDragDrop (data, source, whatToDo, &effect);

            data->Release();
            source->Release();

            OleUninitialize();

            if (completionCallback != nullptr)
                MessageManager::callAsync (std::move (completionCallback));

            return jobHasFinished;
        }

        FORMATETC format;
        STGMEDIUM medium;
        DWORD whatToDo;

        std::function<void()> completionCallback;
    };

    class ThreadPoolHolder   : private DeletedAtShutdown
    {
    public:
        ThreadPoolHolder() = default;

        ~ThreadPoolHolder()
        {
            // Wait forever if there's a job running. The user needs to cancel the transfer
            // in the GUI.
            pool.removeAllJobs (true, -1);

            clearSingletonInstance();
        }

        JUCE_DECLARE_SINGLETON_SINGLETHREADED (ThreadPoolHolder, false)

        // We need to make sure we don't do simultaneous text and file drag and drops,
        // so use a pool that can only run a single job.
        ThreadPool pool { 1 };
    };

    JUCE_IMPLEMENT_SINGLETON (ThreadPoolHolder)
}

//==============================================================================
bool DragAndDropContainer::performExternalDragDropOfFiles (const StringArray& files, const bool canMove,
                                                           Component*, std::function<void()> callback)
{
    if (files.isEmpty())
        return false;

    FORMATETC format = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM medium = { TYMED_HGLOBAL, { nullptr }, nullptr };

    medium.hGlobal = DragAndDropHelpers::createHDrop (files);

    auto& pool = DragAndDropHelpers::ThreadPoolHolder::getInstance()->pool;
    pool.addJob (new DragAndDropHelpers::DragAndDropJob (format, medium,
                                                         canMove ? (DROPEFFECT_COPY | DROPEFFECT_MOVE) : DROPEFFECT_COPY,
                                                         std::move (callback)),
                true);

    return true;
}

bool DragAndDropContainer::performExternalDragDropOfText (const String& text, Component*, std::function<void()> callback)
{
    if (text.isEmpty())
        return false;

    FORMATETC format = { CF_TEXT, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM medium = { TYMED_HGLOBAL, { nullptr }, nullptr };

    auto numBytes = CharPointer_UTF16::getBytesRequiredFor (text.getCharPointer());

    medium.hGlobal = GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT, numBytes + 2);

    if (medium.hGlobal == nullptr)
        return false;

    auto* data = static_cast<WCHAR*> (GlobalLock (medium.hGlobal));

    text.copyToUTF16 (data, numBytes + 2);
    format.cfFormat = CF_UNICODETEXT;

    GlobalUnlock (medium.hGlobal);

    auto& pool = DragAndDropHelpers::ThreadPoolHolder::getInstance()->pool;
    pool.addJob (new DragAndDropHelpers::DragAndDropJob (format,
                                                        medium,
                                                        DROPEFFECT_COPY | DROPEFFECT_MOVE,
                                                        std::move (callback)),
                 true);

    return true;
}

} // namespace juce
