//************************************************************************************************
//
// PreSonus Plug-In Extensions
// Written and placed in the PUBLIC DOMAIN by PreSonus Software Ltd.
// Created by  : PreSonus Software Ltd., 03/2015
//
//************************************************************************************************

#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/base/falignpush.h"

namespace Presonus {

    //************************************************************************************************
    // IGainReductionInfo
    /**    Interface to report gain reduction imposed to the audio signal by the plug-in to the
     host for display in the UI. Implemented by the VST3 edit controller class.
     */
    //************************************************************************************************

    struct IGainReductionInfo: Steinberg::FUnknown
    {
        /** Get current gain reduction for display. The returned value in dB is either 0.0 (no reduction)
         or negative. The host calls this function periodically while the plug-in is active.
         The value is used AS IS for UI display purposes, without imposing additional ballistics or
         presentation latency compensation. Be sure to return zero if processing is bypassed internally.
         For multiple reduction stages, please report the sum in dB here.
         */
        virtual double PLUGIN_API getGainReductionValueInDb () = 0;

        static const Steinberg::FUID iid;
    };

    DECLARE_CLASS_IID (IGainReductionInfo, 0x8e3c292c, 0x95924f9d, 0xb2590b1e, 0x100e4198)
    DEF_CLASS_IID(IGainReductionInfo)

} // namespace Presonus

#include "pluginterfaces/base/falignpop.h"
