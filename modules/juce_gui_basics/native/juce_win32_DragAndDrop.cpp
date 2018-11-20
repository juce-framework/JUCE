/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

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
            if (result == 0)
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

            if (index == 0 && celt > 0 && lpFormatEtc != 0)
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

            index += celt;
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

            if (source.ptd != 0)
            {
                dest.ptd = (DVTARGETDEVICE*) CoTaskMemAlloc (sizeof (DVTARGETDEVICE));
                *(dest.ptd) = *(source.ptd);
            }
        }

        JUCE_DECLARE_NON_COPYABLE (JuceEnumFormatEtc)
    };

    //==============================================================================
    class JuceDataObject  : public ComBaseClassHelper <IDataObject>
    {
    public:
        JuceDataObject (JuceDropSource* s, const FORMATETC* f, const STGMEDIUM* m)
            : dropSource (s), format (f), medium (m)
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
                pMedium->pUnkForRelease = 0;

                if (format->tymed == TYMED_HGLOBAL)
                {
                    auto len = GlobalSize (medium->hGlobal);
                    void* const src = GlobalLock (medium->hGlobal);
                    void* const dst = GlobalAlloc (GMEM_FIXED, len);

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
            if (f == 0)
                return E_INVALIDARG;

            if (f->tymed == format->tymed
                  && f->cfFormat == format->cfFormat
                  && f->dwAspect == format->dwAspect)
                return S_OK;

            return DV_E_FORMATETC;
        }

        JUCE_COMRESULT GetCanonicalFormatEtc (FORMATETC*, FORMATETC* pFormatEtcOut)
        {
            pFormatEtcOut->ptd = 0;
            return E_NOTIMPL;
        }

        JUCE_COMRESULT EnumFormatEtc (DWORD direction, IEnumFORMATETC** result)
        {
            if (result == 0)
                return E_POINTER;

            if (direction == DATADIR_GET)
            {
                *result = new JuceEnumFormatEtc (format);
                return S_OK;
            }

            *result = 0;
            return E_NOTIMPL;
        }

        JUCE_COMRESULT GetDataHere (FORMATETC*, STGMEDIUM*)                  { return DATA_E_FORMATETC; }
        JUCE_COMRESULT SetData (FORMATETC*, STGMEDIUM*, BOOL)                { return E_NOTIMPL; }
        JUCE_COMRESULT DAdvise (FORMATETC*, DWORD, IAdviseSink*, DWORD*)     { return OLE_E_ADVISENOTSUPPORTED; }
        JUCE_COMRESULT DUnadvise (DWORD)                                     { return E_NOTIMPL; }
        JUCE_COMRESULT EnumDAdvise (IEnumSTATDATA**)                         { return OLE_E_ADVISENOTSUPPORTED; }

    private:
        JuceDropSource* const dropSource;
        const FORMATETC* const format;
        const STGMEDIUM* const medium;

        JUCE_DECLARE_NON_COPYABLE (JuceDataObject)
    };

    //==============================================================================
    HDROP createHDrop (const StringArray& fileNames)
    {
        int totalBytes = 0;
        for (int i = fileNames.size(); --i >= 0;)
            totalBytes += (int) CharPointer_UTF16::getBytesRequiredFor (fileNames[i].getCharPointer()) + sizeof (WCHAR);

        HDROP hDrop = (HDROP) GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof (DROPFILES) + totalBytes + 4);

        if (hDrop != 0)
        {
            auto pDropFiles = (LPDROPFILES) GlobalLock (hDrop);
            pDropFiles->pFiles = sizeof (DROPFILES);
            pDropFiles->fWide = true;

            auto* fname = reinterpret_cast<WCHAR*> (addBytesToPointer (pDropFiles, sizeof (DROPFILES)));

            for (int i = 0; i < fileNames.size(); ++i)
            {
                auto bytesWritten = fileNames[i].copyToUTF16 (fname, 2048);
                fname = reinterpret_cast<WCHAR*> (addBytesToPointer (fname, bytesWritten));
            }

            *fname = 0;

            GlobalUnlock (hDrop);
        }

        return hDrop;
    }

    bool performDragDrop (FORMATETC* const format, STGMEDIUM* const medium, const DWORD whatToDo)
    {
        auto source = new JuceDropSource();
        auto data   = new JuceDataObject (source, format, medium);

        DWORD effect;
        auto res = DoDragDrop (data, source, whatToDo, &effect);

        data->Release();
        source->Release();

        return res == DRAGDROP_S_DROP;
    }
}

//==============================================================================
bool DragAndDropContainer::performExternalDragDropOfFiles (const StringArray& files, const bool canMove, Component*)
{
    FORMATETC format = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM medium = { TYMED_HGLOBAL, { 0 }, 0 };

    medium.hGlobal = DragAndDropHelpers::createHDrop (files);

    return DragAndDropHelpers::performDragDrop (&format, &medium, canMove ? (DWORD) (DROPEFFECT_COPY | DROPEFFECT_MOVE)
                                                                          : (DWORD) DROPEFFECT_COPY);
}

bool DragAndDropContainer::performExternalDragDropOfText (const String& text, Component*)
{
    FORMATETC format = { CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM medium = { TYMED_HGLOBAL, { 0 }, 0 };

    auto numBytes = CharPointer_UTF16::getBytesRequiredFor (text.getCharPointer());

    medium.hGlobal = GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT, numBytes + 2);
    WCHAR* const data = static_cast<WCHAR*> (GlobalLock (medium.hGlobal));

    text.copyToUTF16 (data, numBytes);
    format.cfFormat = CF_UNICODETEXT;

    GlobalUnlock (medium.hGlobal);

    return DragAndDropHelpers::performDragDrop (&format, &medium, DROPEFFECT_COPY | DROPEFFECT_MOVE);
}

} // namespace juce
