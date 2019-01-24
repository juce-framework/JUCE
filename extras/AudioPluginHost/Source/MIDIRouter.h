#pragma once

#include "Performer.h"
//#include "../rtmidi-2.1.0/RtMidi.h"


#ifdef WIN32
#include <windows.h>
#define Mutex CRITICAL_SECTION
#define create_mutex(_mutex) InitializeCriticalSectionAndSpinCount(&_mutex, 0x00000400)
#define lock_mutex(_mutex) EnterCriticalSection( &_mutex )
#define unlock_mutex(_mutex) LeaveCriticalSection( &_mutex )
#define destroy_mutex(_mutex) DeleteCriticalSection(&_mutex)
#else
#include <pthread.h>
#define Mutex pthread_mutex_t
#define create_mutex(_mutex) pthread_mutex_init(&_mutex, NULL)
#define lock_mutex(_mutex) pthread_mutex_lock(&_mutex)
#define unlock_mutex(_mutex) pthread_mutex_unlock( &_mutex )
#define destroy_mutex(_mutex) pthread_mutex_destroy(&_mutex)
#include <time.h>
#include <signal.h>
#endif

class MIDIRouter
{
public:
    MIDIRouter(const char *a_baseDir, const char *a_alreadyRunningRCF = NULL);

    ~MIDIRouter();

protected:
    void ProcessMidi(double deltatime, std::vector< unsigned char > *message, void *userData);
    static void ProcessMidiStatic(double deltatime, std::vector< unsigned char > *message, void *userData);
    void UpdateCurrentRouting();
    void PrintLCDScreen(const char *text1, const char *text2);
    void SetupKeylab();
    void UpdateLCDScreen();
    void SetVolumes();
    void CreateFriendlySongnames();
    void ArpegiatorUpdate();
    void LoadSet(int a_set, bool a_alreadyLaunched = false, bool a_printLCD = true);
    void EnsureOneSetList(ForteFile &a_forteFile);
    void OptimizeLines(string &, string &);

    struct SceneMidi
    {
        int m_channel;
        int m_transpose;
        int m_low;
        int m_high;
        int m_bank;
        int m_program;
        bool m_arpeggiator;
        int m_lastNote;
    };
    SceneMidi SetupSceneMidi(ForteMapChannel &mapChannel, int ri);

    struct Rack
    {
        int m_id;
        string m_name;
        string m_groupname;
        float m_volume;
        bool m_disabled;
        bool m_ccFilter[128];
        vector<SceneMidi> m_sceneMidi;
        RtMidiOut *m_midiOut;

        bool m_notesDown[128];
        bool m_anyNotesDown;
        int m_arpeggiatorBeat;
    };

    std::vector<Rack> m_racks;

    struct Set
    {
        std::string m_rcfFile;
        std::string m_shortFile;
        int m_setListIndex;
        int m_defaultSetListIndex;
        std::string m_setListName;

    };
    std::vector<Set> m_sets;

    int         m_currentSong;
    int         m_pendingSong;
    int         m_currentSet;
    int         m_pendingSet;
    struct SongInfo { std::string m_line[2]; };
    std::vector<SongInfo> m_setList; // todo rename this

    ForteFile   m_forteFile;
    string      m_baseDir;
    float       m_globalVolume;
    int         m_tempo;
    bool        m_volumesDirty;
    string      m_clientName;
    RtMidiIn*   m_midiIn;
    RtMidiOut*  m_midiOutLCD;
    RtMidiOut*  m_midiOutPassThrough;

    Mutex       m_midiOutMutex;
#if !defined(WIN32) && !defined(MACOS)
    static void ArpegiatorUpdateStatic(union sigval arg);
    timer_t m_appegiatorTimer;
#endif
    int         m_shutdownPressCount;

};

