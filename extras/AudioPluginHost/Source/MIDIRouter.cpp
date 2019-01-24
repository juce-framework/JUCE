#include "MIDIRouter.h"

// t != 80 && t != 81 && t != 91 && t != 93

#include "MIDIRouter.h"
#include "stdio.h"
#include <cstring>
#include <iostream>
#include <math.h>
#if defined(WIN32)
#include <windows.h>
#elif defined(MACOS)
#define PLIST
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#else
#include <jack/jack.h>
#endif

#ifdef PLIST
#include "plist/plist.h"
#ifdef WIN32
#define PATH_SEPERATOR "\\"
#else
#define PATH_SEPERATOR "/"
#endif
#endif

#include <algorithm>



std::string base64_decode(std::string const& encoded_string);

#define MIDICHANNEL_ARPEGGIATOR 16 // purposely invalid (highest usable is 15)

std::string defaultstring;

void MIDIRouter::ProcessMidiStatic(double deltatime, std::vector< unsigned char > *message, void *userData)
{
    ((MIDIRouter *)userData)->ProcessMidi(deltatime, message, userData);
}

void MIDIRouter::ProcessMidi(double deltatime, std::vector< unsigned char > *message, void *userData)
{
#define MIDI_NOTEOFF         0x80
#define MIDI_NOTEON          0x90
#define MIDI_POLYKEYPRESSURE 0xA0   // AKA Polyphonic Aftertouch
#define MIDI_CONTROLCHANGE   0xB0
#define MIDI_PROGRAMCHANGE   0xC0
#define MIDI_CHANNELPRESSURE 0xD0   // AKA Aftertouch
#define MIDI_PITCHBEND       0xE0

#define MIDI_BANK_CHANGE     0x00
#define MIDI_MODULATION      0x01
#define MIDI_VOLUME          0x07

    lock_mutex(m_midiOutMutex);

    if (!m_midiOutPassThrough)
    {
        for (int ri = 0; ri<(int)m_racks.size(); ++ri)
        {
            m_racks[ri].m_anyNotesDown = false; // see if any notes currently down (so we know whether to restart sequence)
            for (int n = 0; n<128; ++n)
            {
                if (m_racks[ri].m_notesDown[n])
                {
                    m_racks[ri].m_anyNotesDown = true;
                    break;
                }
            }
        }
    }


    int status = (*message)[0] & 0xf0;   // scraping  channel
    int channel = (*message)[0] & 0x0f;   // isolating channel
    int data1 = (*message)[1] & 0x7f;
    bool data2valid = (*message).size()>2;
    int data2 = data2valid ? ((*message)[2] & 0x7f) : 0;

    if (status == MIDI_CONTROLCHANGE && data1 == 117) // power off
    {
        if (data2 == 127)
            m_shutdownPressCount++;
        if (m_shutdownPressCount == 1)
            PrintLCDScreen("Are you sure?", " ");
        if (m_shutdownPressCount>1)
        {
            PrintLCDScreen("Shutting Down", " ");
            system("shutdown /t 0 /s");
        }
    }
    else
    {
        if (m_shutdownPressCount) // to remove confirm text
        {
            if (m_setList.size()>0)
                UpdateLCDScreen();
            else
                PrintLCDScreen(" ", " ");
        }
        m_shutdownPressCount = 0;

    }

    if (status == MIDI_NOTEON && data2 == 0)
        status = MIDI_NOTEOFF;

    if (status == MIDI_CONTROLCHANGE && (data1 == MIDI_BANK_CHANGE || data1 == 0x20))
    {
        // ignore bank change 
    }
    else if (status == MIDI_CHANNELPRESSURE || status == MIDI_POLYKEYPRESSURE)
    {
        //ignore aftertouch
    }
    //else if (status==MIDI_PITCHBEND)
    //{
    //  //ignore pitch bend
    //}
#ifndef MACOS
    else if (status == MIDI_CONTROLCHANGE && data1 == MIDI_VOLUME)
    {
        //ignore volume changes
    }
#endif
    //else if (status==MIDI_CONTROLCHANGE && data1==MIDI_MODULATION)
    //{
    //  //ignore modulation
    //}
    else if (!m_midiOutPassThrough && status == MIDI_CONTROLCHANGE && data1 == 0x09)
    {
        m_globalVolume = ((float)data2) / 127.f;
        SetVolumes();
        printf("Volume=%d%%\n", int(m_globalVolume * 100));
    }
    else if (status == MIDI_PROGRAMCHANGE)
    {
        if (m_setList.size())
        {
            m_currentSong = data1 % (int)m_setList.size();
            m_pendingSong = m_currentSong;
        }

        if (m_midiOutPassThrough)
        {
            vector<unsigned char> message;
            message.push_back(status | channel);
            message.push_back(data1);
            m_midiOutPassThrough->sendMessage(&message);
        }
        else
        {
            UpdateCurrentRouting();
        }
    }
    else if (status == MIDI_CONTROLCHANGE && data1 == 111) // backward
    {
        if (data2>0)
        {
            m_currentSong--;
            if (m_currentSong<0)
                m_currentSong = m_setList.size() - 1;
            if (m_currentSong<0)
                m_currentSong = 0;
            m_pendingSong = m_currentSong;
            // fake program change
            if (m_midiOutPassThrough)
            {
                vector<unsigned char> message;
                message.push_back(MIDI_PROGRAMCHANGE);
                message.push_back(m_currentSong);
                m_midiOutPassThrough->sendMessage(&message);
            }
            UpdateCurrentRouting();
        }
        else
            UpdateLCDScreen();

    }
    else if (status == MIDI_CONTROLCHANGE && data1 == 116) // forward
    {
        if (data2>0)
        {
            m_currentSong++;
            if (m_currentSong >= (int)m_setList.size())
                m_currentSong = 0;
            m_pendingSong = m_currentSong;
            // fake program change
            if (m_midiOutPassThrough)
            {
                vector<unsigned char> message;
                message.push_back(MIDI_PROGRAMCHANGE);
                message.push_back(m_currentSong);
                m_midiOutPassThrough->sendMessage(&message);
            }
            UpdateCurrentRouting();
        }
        else
            UpdateLCDScreen();

    }
    else if (status == MIDI_CONTROLCHANGE && data1 == 115)
    {
        if (data2>0)
        {
            m_currentSong = m_pendingSong;

            // fake program change
            if (m_midiOutPassThrough)
            {
                vector<unsigned char> message;
                message.push_back(MIDI_PROGRAMCHANGE);
                message.push_back(m_currentSong);
                m_midiOutPassThrough->sendMessage(&message);
            }

            UpdateCurrentRouting();
        }
        else
            UpdateLCDScreen(); // just redraw
    }
    else if (status == MIDI_CONTROLCHANGE && data1 == 114 && data2 == 0x3f) // anticlockwise
    {
        m_pendingSong--;
        if (m_pendingSong<0)
            m_pendingSong = m_setList.size() - 1;
        if (m_pendingSong<0)
            m_pendingSong = 0;
        UpdateLCDScreen();
    }
    else if (status == MIDI_CONTROLCHANGE && data1 == 114 && data2 == 0x41) // clockwise
    {
        m_pendingSong++;
        if (m_pendingSong >= (int)m_setList.size())
            m_pendingSong = 0;
        UpdateLCDScreen();
    }



    else if (status == MIDI_CONTROLCHANGE && data1 == 113)
    {
        if (data2>0)
            LoadSet(m_pendingSet);
        else
            UpdateLCDScreen(); // just redraw
    }
    else if (status == MIDI_CONTROLCHANGE && data1 == 112 && data2 == 0x3f) // category anticlockwise
    {
        m_pendingSet--;
        if (m_pendingSet<0)
            m_pendingSet = m_sets.size() - 1;
        if (m_pendingSet<0)
            m_pendingSet = 0;
        if (m_sets.size()>0)
            PrintLCDScreen(m_sets[m_pendingSet].m_shortFile.c_str(), m_sets[m_pendingSet].m_setListName.c_str());
    }
    else if (status == MIDI_CONTROLCHANGE && data1 == 112 && data2 == 0x41) // category clockwise
    {
        m_pendingSet++;
        if (m_pendingSet >= (int)m_sets.size())
            m_pendingSet = 0;
        if (m_sets.size()>0)
            PrintLCDScreen(m_sets[m_pendingSet].m_shortFile.c_str(), m_sets[m_pendingSet].m_setListName.c_str());
    }






    else if (!m_midiOutPassThrough && (status == MIDI_NOTEON || status == MIDI_NOTEOFF))
    {
        for (int ri = 0; ri<(int)m_racks.size(); ++ri)
        {
            for (int sm = 0; sm<(int)m_racks[ri].m_sceneMidi.size(); ++sm)
            {
                SceneMidi &sceneMidi = m_racks[ri].m_sceneMidi[sm];

                if (data1 >= sceneMidi.m_low && data1 <= sceneMidi.m_high && !sceneMidi.m_arpeggiator)
                {
                    int note = data1 + sceneMidi.m_transpose;
                    if (note >= 0 && note <= 127)
                    {
                        if (sceneMidi.m_channel == MIDICHANNEL_ARPEGGIATOR)
                        {
                            // are we starting?
                            if (!m_racks[ri].m_anyNotesDown && status == MIDI_NOTEON)
                            {
                                m_racks[ri].m_arpeggiatorBeat = -1;
#if !defined(WIN32) && !defined(MACOS)
                                long int msec = 15000 / m_tempo;
                                long int sec = (msec / 1000);
                                long int nsec = (msec % 1000) * 1e06;

                                itimerspec ts;
                                ts.it_value.tv_sec = 0;
                                ts.it_value.tv_nsec = 1; // one nanosecond until first event (effectively instant)
                                ts.it_interval.tv_sec = sec;
                                ts.it_interval.tv_nsec = nsec;
                                timer_settime(m_appegiatorTimer, 0, &ts, NULL);
#endif
                            }

                            m_racks[ri].m_notesDown[note] = (status == MIDI_NOTEON);

                            if (status == MIDI_NOTEOFF)
                            {
                                // recalculate this with change
                                m_racks[ri].m_anyNotesDown = false; // see if any notes currently down (so we know whether to restart sequence)
                                for (int n = 0; n<128; ++n)
                                {
                                    if (m_racks[ri].m_notesDown[n])
                                    {
                                        m_racks[ri].m_anyNotesDown = true;
                                        break;
                                    }
                                }

                                // are we ending?
                                if (!m_racks[ri].m_anyNotesDown)
                                {
#if !defined(WIN32) && !defined(MACOS)
                                    itimerspec ts;
                                    ts.it_interval.tv_sec = ts.it_value.tv_sec = ts.it_interval.tv_nsec = ts.it_value.tv_nsec = ts.it_interval.tv_nsec = 0;
                                    timer_settime(m_appegiatorTimer, 0, &ts, NULL);
#endif
                                }
                            }
                        }
                        else
                        {
                            vector<unsigned char> message;
                            message.push_back(status | sceneMidi.m_channel);
                            message.push_back(note);
                            message.push_back(data2);
                            if (m_racks[ri].m_midiOut)
                                m_racks[ri].m_midiOut->sendMessage(&message);
                        }
                    }
                }
            }
        }
    }
    else if (m_midiOutPassThrough)
    {
        vector<unsigned char> message;
        message.push_back(status | channel);
        message.push_back(data1);
        if (data2valid)
            message.push_back(data2);
        m_midiOutPassThrough->sendMessage(&message);
    }
    else // pass through all other midi events to all racks
    {
        for (int ri = 0; ri<(int)m_racks.size(); ++ri)
        {
            if (m_racks[ri].m_midiOut && (status != MIDI_CONTROLCHANGE || !m_racks[ri].m_ccFilter[data1]))
            {

                vector<unsigned char> message;
                message.push_back(status | channel);
                message.push_back(data1);
                message.push_back(data2);
                m_racks[ri].m_midiOut->sendMessage(&message);
            }
        }
    }

    unlock_mutex(m_midiOutMutex);
}

#if !defined(WIN32) && !defined(MACOS)
void MIDIRouter::ArpegiatorUpdateStatic(union sigval arg)
{
    ((MIDIRouter *)arg.sival_ptr)->ArpegiatorUpdate();
}
#endif

void MIDIRouter::ArpegiatorUpdate()
{
    if (!m_midiOutPassThrough)
    {
        lock_mutex(m_midiOutMutex);

        // Arpeggiator
        for (int ri = 0; ri<(int)m_racks.size(); ++ri)
        {
            for (int sm = 0; sm<(int)m_racks[ri].m_sceneMidi.size(); ++sm)
            {
                SceneMidi &sceneMidi = m_racks[ri].m_sceneMidi[sm];
                if (sceneMidi.m_arpeggiator)
                {
                    // cancel last note
                    if (sceneMidi.m_lastNote >= 0)
                    {
                        vector<unsigned char> message;
                        message.push_back(MIDI_NOTEOFF | sceneMidi.m_channel);
                        message.push_back(sceneMidi.m_lastNote);
                        if (m_racks[ri].m_midiOut)
                            m_racks[ri].m_midiOut->sendMessage(&message);
                        sceneMidi.m_lastNote = -1;
                    }

                    for (int n = 0; n<128; ++n)
                    {
                        if (m_racks[ri].m_notesDown[n]) // find lowest
                        {
                            m_racks[ri].m_arpeggiatorBeat++;

                            vector<unsigned char> message;
                            message.push_back(MIDI_NOTEON | sceneMidi.m_channel);
                            message.push_back(n + 12 * (m_racks[ri].m_arpeggiatorBeat % 3));
                            message.push_back(0x7f);
                            if (m_racks[ri].m_midiOut)
                                m_racks[ri].m_midiOut->sendMessage(&message);

                            sceneMidi.m_lastNote = message[1];
                            break; // only do lowest
                        }
                    }
                    break; //only do one
                }
            }
        }
        unlock_mutex(m_midiOutMutex);
    }

}

void MIDIRouter::SetVolumes()
{
    for (int ri = 0; ri<(int)m_racks.size(); ++ri)
    {
        // set volume (and disables too)                
        if (m_racks[ri].m_midiOut)
        {
            vector<unsigned char> message;
            message.push_back(MIDI_CONTROLCHANGE | 0); // midi channel 1
            message.push_back(0x07); // volume
            float vol = m_racks[ri].m_volume * m_globalVolume * 100; // TODO need to scale this a bit to compensate for all ranges
            if (vol>127.f) vol = 127.f;
            if (vol<1.f) vol = 1.f; // dont want 0 because will disable    
            message.push_back(m_racks[ri].m_disabled ? 0 : int(vol));
            m_racks[ri].m_midiOut->sendMessage(&message);
        }
    }
}

void stringreplace(string &str, string src, string dst)
{
    while (str.find(src) != -1)
        str.replace(str.find(src), src.length(), dst);
}

void TrimRight(std::string& str, const char* chars2remove = " ")
{
    if (!str.empty())
    {
        std::string::size_type pos = str.find_last_not_of(chars2remove);

        if (pos != std::string::npos)
            str.erase(pos + 1);
        else
            str.erase(str.begin(), str.end()); // make empty
    }
}

void MIDIRouter::PrintLCDScreen(const char *text1, const char *text2)
{
    if (m_midiOutLCD)
    {
        std::string ip;

        if (strcmp(text1, " ") == 0 && strcmp(text2, " ") == 0)
        {
#ifdef WIN32
            WSADATA wsa;
            WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
            char szHostName[255];
            gethostname(szHostName, 255);
            struct hostent *host_entry;
            host_entry = gethostbyname(szHostName);
            text1 = szHostName;
            ip = inet_ntoa(*(struct in_addr *)*host_entry->h_addr_list);
            if (ip.length() <= 13)
                ip = "IP:" + ip;
            text2 = ip.c_str();
#ifdef WIN32
            WSACleanup();
#endif
        }

        unsigned char mes[14 + 16 * 2];
        int meslen = 0;
        memcpy(mes, "\xF0\x00\x20\x6B\x7F\x42\x04\x00\x60\x01", 10); meslen += 10;
        memcpy(mes + meslen, text1, strlen(text1) + 1); meslen += strlen(text1) + 1;
        mes[meslen++] = 0x02;
        memcpy(mes + meslen, text2, strlen(text2) + 1); meslen += strlen(text2) + 1;
        mes[meslen++] = 0xF7;

        // send to lcd
        vector<unsigned char> message(meslen);
        memcpy(message.data(), mes, meslen);
        m_midiOutLCD->sendMessage(&message);
    }
}


void MIDIRouter::SetupKeylab()
{
    for (int m = 0; m<60; ++m)
    {
        unsigned char mes[12];
        int meslen = 0;
        memcpy(mes, "\xF0\x00\x20\x6B\x7F\x42\x02\x00", 8); meslen += 8;
        unsigned char parameter = 3;
        unsigned char control = 0;
        unsigned char value = 0;
        // order volume / knob 1 #3 / 9 disable knobs / Slider 1 #9 / 8 disable sliders / 16 pads (midi and note) / rewind (mode and cc) / forward (mode and cc) / stop (mode and cc)
        if (m == 0)
        {
            control = 0x30;
            value = 9;
        }
        else if (m == 1)
        {
            control = 1;
            value = 3;
        }
        else if (m<2 + 9)
        {
            parameter = 1; // mode
            control = m; // knob 2 to knob 10
            value = 0; // disable
        }
        else if (m == 2 + 9)
        {
            control = 0x0b; // slider 1
            value = 9; // also setup slider 1 like volume
        }
        else if (m<2 + 9 + 1 + 8)
        {
            parameter = 5; // max val
            control = (m<2 + 1 + 8 + 4) ? m : (m + 0x3c);
            value = 0; // another way of disable (didn't work other way)
        }
        else if (m<2 + 9 + 1 + 8 + 16)
        {
            parameter = 2; // midi channel (defaults to 10 on pads)
            control = 0x70 + (m - 2 - 9 - 1 - 8);
            value = 0; // midi channel 1
        }
        else if (m<2 + 9 + 1 + 8 + 16 * 2)
        {
            control = 0x70 + (m - 2 - 9 - 1 - 8 - 16);
            value = 0x15 + (m - 2 - 9 - 1 - 8 - 16); // Low A up
        }
        else if (m == 54)
        {
            parameter = 1;
            control = 0x5b; // rewind
            value = 8; // for some reason this mode value instead of 3
        }
        else if (m == 55)
        {
            parameter = 3;
            control = 0x5b;
            value = 111;
        }
        else if (m == 56)
        {
            parameter = 1;
            control = 0x5c; // forward
            value = 8;
        }
        else if (m == 57)
        {
            parameter = 3;
            control = 0x5c;
            value = 116;
        }
        else if (m == 58)
        {
            parameter = 1;
            control = 0x59; // stop
            value = 8;
        }
        else if (m == 59)
        {
            parameter = 3;
            control = 0x59;
            value = 117;
        }


        mes[meslen++] = parameter;
        mes[meslen++] = control;
        mes[meslen++] = value;
        mes[meslen++] = 0xf7;

        // send to lcd
        vector<unsigned char> message(meslen);
        memcpy(message.data(), mes, meslen);
        m_midiOutLCD->sendMessage(&message);
    }
}


void MIDIRouter::UpdateLCDScreen()
{
    if (m_setList.size()>0)
        PrintLCDScreen(m_setList[m_pendingSong].m_line[0].c_str(), m_setList[m_pendingSong].m_line[1].c_str());
    else
        PrintLCDScreen("No set loaded", " ");
}

MIDIRouter::SceneMidi MIDIRouter::SetupSceneMidi(ForteMapChannel &mapChannel, int ri)
{
    SceneMidi sceneMidi;
    ForteKey &key = mapChannel.Key;
    sceneMidi.m_low = key.Low;
    sceneMidi.m_high = key.High;
    sceneMidi.m_transpose = key.Transpose;
    sceneMidi.m_program = -1;
    sceneMidi.m_bank = 0;
    vector<ForteCC> &cc = mapChannel.CC;
    for (int c = 0; c<(int)cc.size(); ++c)
    {
        if (cc[c].From == "All" && cc[c].To == "Disabled")
            memset(&m_racks[ri].m_ccFilter, 1, sizeof(m_racks[ri].m_ccFilter)); // should be ok to just have one but need to check
        else if (cc[c].To == "Disabled")
            m_racks[ri].m_ccFilter[atoi(cc[c].From.c_str())] = true;
        else
            m_racks[ri].m_ccFilter[atoi(cc[c].From.c_str())] = false; // assume remap parameter, we disable CC#3 by default
    }
    return sceneMidi;
}


void MIDIRouter::UpdateCurrentRouting()
{
    if (m_forteFile.Rack.Setlists.Setlist.empty())
        return;
    int setListIndex = m_forteFile.Rack.Setlists.Active;
    ForteSetlist &setlist = m_forteFile.Rack.Setlists.Setlist[setListIndex];

    for (int ri = 0; ri<(int)m_racks.size(); ++ri)
    {
        m_racks[ri].m_disabled = true;
        memset(&m_racks[ri].m_ccFilter, 0, sizeof(m_racks[ri].m_ccFilter));
        m_racks[ri].m_ccFilter[3] = true; // always filter #3 unless specified
        m_racks[ri].m_sceneMidi.clear();
    }

    int count = 0;
    for (unsigned int sr = 0; sr<setlist.SongRef.size(); ++sr)
    {
        ForteSongRef &songId = setlist.SongRef[sr];

        for (unsigned int si = 0; si<m_forteFile.Rack.Setlists.Song.size(); ++si)
        {
            ForteSong &song = m_forteFile.Rack.Setlists.Song[si];
            if (song.ID == songId.ID)
            {
                for (unsigned int mr = 0; mr<song.MixerSceneRef.size(); ++mr)
                {
                    ForteMixerSceneRef &mixerRef = song.MixerSceneRef[mr];
                    for (unsigned int mi = 0; mi<m_forteFile.Rack.MixerScene.size(); ++mi)
                    {
                        ForteMixerScene &mixer = m_forteFile.Rack.MixerScene[mi];
                        if (mixer.ID == mixerRef.ID)
                        {
                            if (m_currentSong == count)
                            {

                                m_tempo = int(mixer.Mixer.Tempo.BPM);

                                printf("%s|%s\n", m_setList[m_currentSong].m_line[0].c_str(), m_setList[m_currentSong].m_line[1].c_str());
                                UpdateLCDScreen();

                                for (int ri = 0; ri<(int)m_racks.size(); ++ri)
                                {
                                    for (int ig = 0; ig<(int)mixer.Mixer.Group.InputGroup.size(); ++ig)
                                    {
                                        ForteInputGroup &group = mixer.Mixer.Group.InputGroup[ig];

                                        if (!group.Mute && (m_racks[ri].m_groupname == group.Name || group.Name == "Arpeggiator"))
                                        {
                                            m_racks[ri].m_disabled = false;

                                            float vol = (float)pow(10, group.Gain / 10.0);
                                            m_racks[ri].m_volume = vol;


                                            ForteMIDIFilterSet &filters = group.PluginChain.PlugIn[0].MIDIFilterSet;
                                            if (filters.MIDIFilter.size())
                                            {
                                                ForteMIDIFilter &filter = filters.MIDIFilter[0]; // just use first one (one keyboard controller only)
                                                if (!filter.Disabled)
                                                {
                                                    for (int mc = 0; mc<(int)filter.MapChannel.size(); ++mc)
                                                    {
                                                        SceneMidi sceneMidi = SetupSceneMidi(filter.MapChannel[mc], ri);
                                                        sceneMidi.m_channel = (group.Name == "Arpeggiator") ? MIDICHANNEL_ARPEGGIATOR : filter.MapChannel[mc].To - 1;
                                                        sceneMidi.m_arpeggiator = false;
                                                        m_racks[ri].m_sceneMidi.push_back(sceneMidi);
                                                    }
                                                }
                                            }

                                            // Appregiator reciever
                                            if (filters.vMIDIFilter.size())
                                            {
                                                ForteMIDIFilter &filter2 = filters.vMIDIFilter[0]; // should be only one
                                                if (!filter2.Disabled)
                                                {
                                                    for (int mc = 0; mc<(int)filter2.MapChannel.size(); ++mc)
                                                    {
                                                        SceneMidi sceneMidi = SetupSceneMidi(filter2.MapChannel[mc], ri);
                                                        sceneMidi.m_channel = filter2.MapChannel[mc].To - 1;
                                                        sceneMidi.m_arpeggiator = true;
                                                        sceneMidi.m_lastNote = -1;
                                                        m_racks[ri].m_sceneMidi.push_back(sceneMidi);
                                                    }
                                                }
                                            }

                                            if (!m_racks[ri].m_sceneMidi.size())
                                            {
                                                // no inputs so lets make a fake one (e.g. wav streamer)
                                                SceneMidi sceneMidi;
                                                sceneMidi.m_low = 127;
                                                sceneMidi.m_high = 0;
                                                sceneMidi.m_transpose = 0;
                                                sceneMidi.m_program = -1;
                                                sceneMidi.m_bank = 0;
                                                sceneMidi.m_channel = 0;
                                                sceneMidi.m_arpeggiator = false;
                                                m_racks[ri].m_sceneMidi.push_back(sceneMidi);
                                            }

                                            // find one of structs above with the same midi channel to the program change
                                            ForteOnSetScene &onSetScene = group.PluginChain.PlugIn[0].OnSetScene;
                                            for (int pc = 0; pc<(int)onSetScene.ProgramChange.size(); ++pc)
                                            {
                                                for (int sm = 0; sm<(int)m_racks[ri].m_sceneMidi.size(); ++sm)
                                                {
                                                    if (onSetScene.ProgramChange[pc].Channel == m_racks[ri].m_sceneMidi[sm].m_channel)
                                                    {
                                                        m_racks[ri].m_sceneMidi[sm].m_program = onSetScene.ProgramChange[pc].Program;
                                                        m_racks[ri].m_sceneMidi[sm].m_bank = onSetScene.ProgramChange[pc].Bank;
                                                        break; // just set first one
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                    count++;
                }
            }
        }
    }


    if (!m_midiOutPassThrough)
    {
        SetVolumes(); // will enable too

#if !defined(WIN32) && !defined(MACOS)
                      // cancel timer
        itimerspec ts;
        ts.it_interval.tv_sec = ts.it_value.tv_sec = ts.it_interval.tv_nsec = ts.it_value.tv_nsec = ts.it_interval.tv_nsec = 0;
        timer_settime(m_appegiatorTimer, 0, &ts, NULL);
#endif

        // set up current programs
        for (int ri = 0; ri<(int)m_racks.size(); ++ri)
        {
            for (int sm = 0; sm<(int)m_racks[ri].m_sceneMidi.size(); ++sm)
            {
                SceneMidi &sceneMidi = m_racks[ri].m_sceneMidi[sm];
                if (sceneMidi.m_program >= 0 && m_racks[ri].m_midiOut)
                {
                    vector<unsigned char> message;
                    message.push_back(MIDI_CONTROLCHANGE | sceneMidi.m_channel);
                    message.push_back(MIDI_BANK_CHANGE);
                    message.push_back(0);
                    m_racks[ri].m_midiOut->sendMessage(&message);

                    message.clear();
                    message.push_back(MIDI_CONTROLCHANGE | sceneMidi.m_channel);
                    message.push_back(0x20);
                    message.push_back(sceneMidi.m_bank);
                    m_racks[ri].m_midiOut->sendMessage(&message);

                    message.clear();
                    message.push_back(MIDI_PROGRAMCHANGE | sceneMidi.m_channel);
                    message.push_back(sceneMidi.m_program);
                    m_racks[ri].m_midiOut->sendMessage(&message);
                }
            }
        }
    }
}


void MIDIRouter::OptimizeLines(string &songName1, string &songName2)
{
    bool line2free = (songName2 == "");
    while (songName1.size()>16 && line2free) // nothing on line 2 and line 1 is too long, try putting it to next line
    {
        if (char* ptr = (char*)strrchr(songName1.c_str(), ' '))
        {
            songName2 = std::string(ptr + 1) + " " + songName2;
            songName1.resize(ptr - songName1.c_str());
        }
    }

    TrimRight(songName2); // procedure above may have added space

                          // if still not fit, remove spaces
    if (songName1.size()>16 || songName2.size()>16)
    {
        stringreplace(songName1, " ", "");
        if (line2free)
            stringreplace(songName2, " ", "");
    }

    // oh well trunc
    if (songName1.size()>16)
        songName1.resize(16);
    if (songName2.size()>16)
        songName2.resize(16);
    if (songName1 == "")
        songName1 = " ";
    if (songName2 == "")
        songName2 = " ";
}

void MIDIRouter::CreateFriendlySongnames()
{

    // create LCD friendly song names
    int setListIndex = m_forteFile.Rack.Setlists.Active;
    if (m_forteFile.Rack.Setlists.Setlist.size())
    {
        ForteSetlist &setlist = m_forteFile.Rack.Setlists.Setlist[setListIndex];
        for (unsigned int sr = 0; sr<setlist.SongRef.size(); ++sr)
        {
            ForteSongRef &songId = setlist.SongRef[sr];

            for (unsigned int si = 0; si<m_forteFile.Rack.Setlists.Song.size(); ++si)
            {
                ForteSong &song = m_forteFile.Rack.Setlists.Song[si];
                if (song.ID == songId.ID)
                {
                    for (unsigned int mr = 0; mr<song.MixerSceneRef.size(); ++mr)
                    {
                        ForteMixerSceneRef &mixerRef = song.MixerSceneRef[mr];
                        bool stop = false;
                        for (unsigned int mi = 0; mi<m_forteFile.Rack.MixerScene.size(); ++mi)
                        {
                            ForteMixerScene &mixer = m_forteFile.Rack.MixerScene[mi];
                            if (mixer.ID == mixerRef.ID)
                            {
                                if (stop)
                                {
                                    int t = 0;
                                }

                                stop = true;
                                string songName1, songName2;
                                songName1 = song.Name;
                                songName2 = mixer.Name;
                                size_t found = songName2.find("|");
                                if (found != -1)
                                    songName2 = string(songName2.c_str() + found + 1);
                                TrimRight(songName2);

                                if (songName1 == songName2)
                                    songName2 = "";


                                OptimizeLines(songName1, songName2);
                                SongInfo newsong;
                                newsong.m_line[0] = songName1;
                                newsong.m_line[1] = songName2;
                                m_setList.push_back(newsong);

                            }
                        }
                    }
                }
            }
        }
    }
}

void MIDIRouter::LoadSet(int a_set, bool a_alreadyLaunched, bool a_printLCD)
{
    // clean up first
#ifdef LINUX
    for (int i = 0; i<(int)m_racks.size(); ++i)
        if (m_racks[i].m_name.find("energyXT") == -1)
            delete m_racks[i].m_midiOut;
#endif

    m_racks.clear();
    m_setList.clear();
    m_forteFile.Rack.MixerScene.clear();
    m_forteFile.Rack.Setlists.Setlist.clear();
    m_forteFile.Rack.Setlists.Song.clear();

#ifdef WIN32
    if (!a_alreadyLaunched)
    {
        PrintLCDScreen("Loading", " ");

        system("taskkill /F /T /IM Forte.exe");

        HKEY hKey;
        RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\brainspawn\\forte 2\\Configuration", 0L, KEY_ALL_ACCESS, &hKey);
        RegDeleteValueA(hKey, "Safe_Mode");
        RegCloseKey(hKey);

        string command("start \"C:\\Program Files (x86)\\brainspawn\\forte 2 Performer Edition\\forte.exe\" \"");

        XmlArchive::Load(m_sets[a_set].m_rcfFile.c_str(), m_forteFile);
        PrintLCDScreen("Loading", " ");
        EnsureOneSetList(m_forteFile);
        if (m_forteFile.Rack.Setlists.Active != m_sets[a_set].m_setListIndex)
        {
            string tempFilename(m_baseDir + "temp.rcf_bk");
            m_forteFile.Rack.Setlists.Active = m_sets[a_set].m_setListIndex;
            XmlArchive::Save(tempFilename.c_str(), m_forteFile);
            command += tempFilename;
        }
        else
            command += m_sets[a_set].m_rcfFile;
        command += "\"";
        system(command.c_str());

        MEMORYSTATUSEX statex;
        statex.dwLength = sizeof(statex);
        unsigned long long lastAvail = 0;
        statex.ullAvailPhys = 1;
        while (statex.ullAvailPhys != lastAvail)
        {
            PrintLCDScreen("Loading", " ");
            lastAvail = statex.ullAvailPhys;
            Sleep(500);
            GlobalMemoryStatusEx(&statex);
        }
    }
    else
#endif
    {
        XmlArchive::Load(m_sets[a_set].m_rcfFile.c_str(), m_forteFile);
        EnsureOneSetList(m_forteFile);
    }

#ifdef LINUX
    m_forteFile.Rack.Setlists.Active = m_sets[a_set].m_setListIndex;
#endif

    m_currentSet = a_set;
    m_currentSong = 0;
    m_pendingSong = 0;

    if (!m_forteFile.Rack.MixerScene.size())
        return;

    CreateFriendlySongnames();

    // use first song to determine all racks needed
    for (int i = 0; i<(int)m_forteFile.Rack.MixerScene[0].Mixer.Group.InputGroup.size(); ++i)
    {
        Rack rack;
        rack.m_id = m_forteFile.Rack.MixerScene[0].Mixer.Group.InputGroup[i].ID;
        rack.m_groupname = m_forteFile.Rack.MixerScene[0].Mixer.Group.InputGroup[i].Name;
        rack.m_name = m_forteFile.Rack.MixerScene[0].Mixer.Group.InputGroup[i].PluginChain.PlugIn[0].Name;
        rack.m_midiOut = NULL;
        rack.m_arpeggiatorBeat = -1;
        memset(rack.m_notesDown, 0, sizeof(rack.m_notesDown));
        m_racks.push_back(rack);
    }

#ifdef LINUX
    // create midi ports
    for (int i = 0; i<(int)m_racks.size(); ++i)
    {
        if (m_racks[i].m_name.find("energyXT") == -1)
        {
            m_racks[i].m_midiOut = new RtMidiOut(RtMidi::UNSPECIFIED, m_clientName);
            string outName = m_racks[i].m_name + " Midi Out";
            m_racks[i].m_midiOut->openVirtualPort(outName);
        }
    }
#endif

    if (m_midiOutPassThrough && a_printLCD)
    {
        UpdateLCDScreen();
        if (m_setList.size()>0)
            printf("%s|%s\n", m_setList[m_currentSong].m_line[0].c_str(), m_setList[m_currentSong].m_line[1].c_str());
    }

#ifdef LINUX 
    UpdateCurrentRouting();
#endif
}


int compareString(const void * a, const void * b)
{
    string *ap = (string*)a;
    string *bp = (string*)b;
    return(ap->compare(*bp));
}

#ifdef WIN32


int list_files(const string &a_dir, const string &a_wildcard, std::vector<std::string> &o_files)
{
#ifdef PLIST
    o_files.push_back("D:\\Data\\Programming\\Private\\MIDIRouter\\MIDIRouter\\Mainstage Preset.concert");
#else
    o_files.clear();
    WIN32_FIND_DATAA ffd;
    HANDLE hFind = FindFirstFileA((a_dir + a_wildcard).c_str(), &ffd);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            std::string fullpath = a_dir;
            fullpath += string(ffd.cFileName);
            o_files.push_back(fullpath);
        } while (FindNextFileA(hFind, &ffd) != 0);
    }
    FindClose(hFind);
    qsort(o_files.data(), o_files.size(), sizeof(std::string), compareString);
#endif
    return int(o_files.size());
}

#endif

void MIDIRouter::EnsureOneSetList(ForteFile &a_forteFile)
{
    if (a_forteFile.Rack.Setlists.Setlist.size() == 0)
    {
        ForteSetlist setlist;
        setlist.Name = " ";
        for (unsigned int mi = 0; mi<a_forteFile.Rack.MixerScene.size(); ++mi)
        {
            string name(a_forteFile.Rack.MixerScene[mi].Name);
            if (name == "SaveState")
                continue;
            ForteMixerSceneRef mixerRef;
            mixerRef.ID = a_forteFile.Rack.MixerScene[mi].ID;
            ForteSong song;
            song.ID = mi * 1000 + 1000;
            song.Name = name;
            song.MixerSceneRef.push_back(mixerRef);
            a_forteFile.Rack.Setlists.Song.push_back(song);
            ForteSongRef songId;
            songId.ID = song.ID;
            setlist.SongRef.push_back(songId);
        }
        a_forteFile.Rack.Setlists.Setlist.push_back(setlist);
    }
}
#ifdef PLIST
vector<string> extract_nodes_from_plist(string filename)
{
    vector<string> result;
    FILE *fp = fopen(filename.c_str(), "rb");
    if (fp == NULL)
    {
        printf("Could not open \"%s\"\n", filename.c_str());
        exit(1);
    }
    fseek(fp, 0, SEEK_END);
    auto size_out = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *plist_bin = (char*)malloc(size_out);
    fread(plist_bin, size_out, 1, fp);
    fclose(fp);

    plist_t root_node;
    plist_from_bin(plist_bin, size_out, &root_node);

    auto nodes = plist_dict_get_item(root_node, "nodes");
    auto arrayCount = plist_array_get_size(nodes);
    for (uint32_t a = 0; a<arrayCount; ++a)
    {
        auto stringnode = plist_array_get_item(nodes, a);
        char *str;
        plist_get_string_val(stringnode, &str);
        result.push_back(str);
    }
    free(plist_bin);
    return result;
}
#endif

MIDIRouter::MIDIRouter(const char *a_baseDir, const char *a_alreadyRunningRCF) : m_currentSong(0), m_pendingSong(0), m_pendingSet(0), m_baseDir(a_baseDir), m_globalVolume(1.f), m_volumesDirty(true), m_clientName("Forte Emulator"), m_midiIn(NULL), m_midiOutLCD(NULL), m_midiOutPassThrough(NULL), m_shutdownPressCount(0)
{
    string currentRCF(a_alreadyRunningRCF == NULL ? "" : a_alreadyRunningRCF);

    // load setlists
    vector<string> files;
#ifdef MACOS
    files.push_back(a_baseDir);
#else
    list_files(a_baseDir, "*.rcf", files);
#endif
    for (int i = 0; i<(int)files.size(); ++i)
    {
        printf("Loading \"%s\"\n", files[i].c_str());
        if (files[i].find("_bk") != -1)
            continue; // dont include backup files
        Set set;
        set.m_rcfFile = files[i];

#ifdef PLIST

        ForteSetlist setlist;
        m_forteFile.Rack.Setlists.Setlist.push_back(setlist);

        files[i] += string(PATH_SEPERATOR) + "Concert.patch" + PATH_SEPERATOR; ;

        auto categories = extract_nodes_from_plist(files[i] + "data.plist");
        for (uint32_t c = 0; c<categories.size(); ++c)
        {
            auto patches = extract_nodes_from_plist(files[i] + categories[c] + PATH_SEPERATOR + "data.plist");
            for (uint32_t p = 0; p<patches.size(); ++p)
            {
                auto patchName = patches[p];
                stringreplace(patchName, ".patch", "");

                string songName1(patchName), songName2;
                OptimizeLines(songName1, songName2);

                SongInfo songinfo;
                songinfo.m_line[0] = songName1;
                songinfo.m_line[1] = songName2;
                //printf("%s\n",songName1.c_str());
                m_setList.push_back(songinfo);
            }
        }

#else

        ForteFile   forteFile;
        XmlArchive::Load(files[i].c_str(), forteFile);
        EnsureOneSetList(forteFile);
        set.m_defaultSetListIndex = forteFile.Rack.Setlists.Active;
        for (int s = 0; s<(int)forteFile.Rack.Setlists.Setlist.size(); ++s)
        {
            set.m_setListIndex = s;
            set.m_setListName = forteFile.Rack.Setlists.Setlist[s].Name;
            set.m_shortFile = files[i];
            stringreplace(set.m_shortFile, ".rcf", "");
            stringreplace(set.m_shortFile, a_baseDir, "");
            m_sets.push_back(set);
        }
#endif
    }

    create_mutex(m_midiOutMutex);

    m_midiIn = new RtMidiIn(RtMidi::UNSPECIFIED, m_clientName);
    bool m_useMidiOX = false;
    for (int pass = 0; pass<3; ++pass)
    {
        for (int i = 0; i<(int)m_midiIn->getPortCount(); i++)
        {
            if (
                (pass == 0 && strstr(m_midiIn->getPortName(i).c_str(), "KeyLab") != NULL) || // first use KeyLab
                (pass == 1 && strstr(m_midiIn->getPortName(i).c_str(), "Internal MIDI") == NULL) || // then try a non virtual keyboard
                (pass == 2 && strstr(m_midiIn->getPortName(i).c_str(), "Internal MIDI") != NULL) // finally use the virtual and flag it so we dont cause a lookback
                )
            {
                m_midiIn->openPort(i, "Midi In");
                m_midiIn->setCallback(&ProcessMidiStatic, this);
                if (pass == 2) m_useMidiOX = true;
                pass = 3; // force exit
                break;
            }
        }
    }

#ifdef WIN32
    if (!m_useMidiOX)
    {
        m_midiOutPassThrough = new RtMidiOut(RtMidi::UNSPECIFIED, m_clientName);
        for (int i = 0; i<(int)m_midiOutPassThrough->getPortCount(); i++)
        {
            if (strstr(m_midiOutPassThrough->getPortName(i).c_str(), "Internal MIDI") != NULL)
                m_midiOutPassThrough->openPort(i);
        }
    }
#endif

#ifdef MACOS
    m_midiOutPassThrough = new RtMidiOut(RtMidi::UNSPECIFIED, m_clientName);
    m_midiOutPassThrough->openVirtualPort();
#endif

#ifdef LINUX
    // create a timer for the appegiator
    sigevent se;
    se.sigev_notify = SIGEV_THREAD;
    se.sigev_notify_function = ArpegiatorUpdateStatic;
    se.sigev_notify_attributes = NULL;
    se.sigev_value.sival_ptr = this;
    timer_create(CLOCK_REALTIME, &se, &m_appegiatorTimer);
#endif

    m_midiOutLCD = new RtMidiOut(RtMidi::UNSPECIFIED, m_clientName);
    for (int i = 0; i<(int)m_midiOutLCD->getPortCount(); i++)
    {
        if (strstr(m_midiOutLCD->getPortName(i).c_str(), "KeyLab") != NULL)
            m_midiOutLCD->openPort(i, "KeyLab LCD Midi Out"); // bug in Catia wont show green connections when this is made
    }



    // find the one that's running
    for (int s = 0; s<(int)m_sets.size(); ++s)
    {
        if (m_sets[s].m_rcfFile == currentRCF && m_sets[s].m_setListIndex == m_sets[s].m_defaultSetListIndex)
            LoadSet(s, true, false);
    }

    SetupKeylab();
    PrintLCDScreen("Engine Loaded", "Select setlist");
}


MIDIRouter::~MIDIRouter()
{
    for (int i = 0; i<(int)m_racks.size(); ++i)
        delete m_racks[i].m_midiOut;
    delete m_midiIn;
    delete m_midiOutLCD;
    delete m_midiOutPassThrough;
}


