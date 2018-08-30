//------------------------------------------------------------------------------
//! \file        TestAnalysis.h
//! \description dummy implementation of audio source analysis for the ARA sample plug-in
//!              actual plug-ins will have an analysis implementation that is independent of ARA
//! \project     ARA SDK, examples
//------------------------------------------------------------------------------
// Copyright (c) 2012-2018, Celemony Software GmbH, All Rights Reserved.
// 
// License & Disclaimer:
// 
// This Software Development Kit (SDK) may not be distributed in parts or 
// its entirety without prior written agreement of Celemony Software GmbH. 
//
// This SDK must not be used to modify, adapt, reproduce or transfer any 
// software and/or technology used in any Celemony and/or Third-party 
// application and/or software module (hereinafter referred as "the Software"); 
// to modify, translate, reverse engineer, decompile, disassemble or create 
// derivative works based on the Software (except to the extent applicable laws 
// specifically prohibit such restriction) or to lease, assign, distribute or 
// otherwise transfer rights to the Software.
// Neither the name of the Celemony Software GmbH nor the names of its 
// contributors may be used to endorse or promote products derived from this 
// SDK without specific prior written permission.
//
// This SDK is provided by Celemony Software GmbH "as is" and any express or 
// implied warranties, including, but not limited to, the implied warranties of 
// non-infringement, merchantability and fitness for a particular purpose are
// disclaimed.
// In no event shall Celemony Software GmbH be liable for any direct, indirect, 
// incidental, special, exemplary, or consequential damages (including, but not 
// limited to, procurement of substitute goods or services; loss of use, data, 
// or profits; or business interruption) however caused and on any theory of 
// liability, whether in contract, strict liability, or tort (including 
// negligence or otherwise) arising in any way out of the use of this software, 
// even if advised of the possibility of such damage.
// Where the liability of Celemony Software GmbH is ruled out or restricted, 
// this will also apply for the personal liability of employees, representatives 
// and vicarious agents.
// The above restriction of liability will not apply if the damages suffered
// are attributable to willful intent or gross negligence or in the case of 
// physical injury.
//------------------------------------------------------------------------------

#pragma once

#include <atomic>
#include <future>
#include <vector>

namespace ARA
{
namespace PlugIn
{

class ARATestAudioSource;
class HostAudioReader;

}    // namespace PlugIn
}    // namespace ARA

/*******************************************************************************/
// The results of our fake analysis are found "notes" detected in the audio source data
class TestAnalysisNote
{
public:
    float getFrequency () const { return _frequency; }
    void setFrequency (float frequency) { _frequency = frequency; }

    float getVolume () const { return _volume; }
    void setVolume (float volume) { _volume = volume; }

    double getStartTime () const { return _startTime; }
    void setStartTime (double startTime) { _startTime = startTime; }

    double getDuration () const { return _duration; }
    void setDuration (double duration) { _duration = duration; }

private:
    float _frequency;
    float _volume;
    double _startTime;
    double _duration;
};

/*******************************************************************************/
class TestAnalysisResult
{
public:
    const std::vector<TestAnalysisNote>& getNotes () const { return _notes; }
    void setNotes (std::vector<TestAnalysisNote> notes) { _notes = notes; }

private:
    std::vector<TestAnalysisNote> _notes;
};

/*******************************************************************************/
class TestAnalysisTask
{
public:
    explicit TestAnalysisTask (ARA::PlugIn::ARATestAudioSource* audioSource);
    ~TestAnalysisTask ();

    ARA::PlugIn::ARATestAudioSource* getAudioSource () const { return _audioSource; }

    float getProgress () const { return _progress; }
    bool isDone () const;
    void cancelSynchronously ();

    const TestAnalysisResult* getAnalysisResult ();

private:
    ARA::PlugIn::ARATestAudioSource* const _audioSource;
    ARA::PlugIn::HostAudioReader* const _hostAudioReader;
    TestAnalysisResult* _analysisResult;
    std::future<void> _future;
    std::atomic<float> _progress;
    std::atomic<bool> _shouldCancel;
};
