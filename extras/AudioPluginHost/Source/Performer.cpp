#include "Performer.h"
#include "ForteEmulator/ForteSDK/ForteTypes.h"

std::string  defaultstring;

void Replace(std::string &result, const std::string& replaceWhat, const std::string& replaceWithWhat)
{
    while (1)
    {
        const int pos = result.find(replaceWhat);
        if (pos == -1) break;
        result.replace(pos, replaceWhat.size(), replaceWithWhat);
    }
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


Performer::Performer()
{
    const char *fileToLoad = "D:\\Data\\Audio\\Synth Backups\\Software\\Performance\\Queen.rcf";
    ForteFile file;
    memset(&file, 0, sizeof(ForteFile));
    XmlArchive::Load(fileToLoad, file);

    // Setlists
    for (unsigned int sl = 0; sl < file.Rack.Setlists.Setlist.size(); ++sl)
    {
        ForteSetlist &setlist = file.Rack.Setlists.Setlist[sl];
        SetList newSetList;
        newSetList.Name = setlist.Name;
        for (unsigned int sr = 0; sr < setlist.SongRef.size(); ++sr)
            newSetList.SongIDs.push_back(setlist.SongRef[sr].ID);
        m_current.SetLists.push_back(newSetList);
    }

    // Songs
    for (unsigned int si = 0; si < file.Rack.Setlists.Song.size(); ++si)
    {
        ForteSong &song = file.Rack.Setlists.Song[si];
        Song newSong;
        newSong.ID = song.ID;
        newSong.Name = song.Name;
        for (unsigned int mr = 0; mr < song.MixerSceneRef.size(); ++mr)
            newSong.PerformanceIDs.push_back(song.MixerSceneRef[mr].ID);
        m_current.Songs.push_back(newSong);
    }

    // Performances
    for (unsigned int mi = 0; mi<file.Rack.MixerScene.size(); ++mi)
    {
        ForteMixerScene &mixer = file.Rack.MixerScene[mi];

        string songName = mixer.Name;
        Replace(songName, "|", " ");
        TrimRight(songName);

        Performance performance;
        performance.ID = mixer.ID;
        performance.Name = songName;
        performance.Tempo = mixer.Mixer.Tempo.BPM;
        for (int ig = 0; ig<(int)mixer.Mixer.Group.InputGroup.size(); ++ig)
        {
            ForteInputGroup &group = mixer.Mixer.Group.InputGroup[ig];
            if (!group.Mute)
            {
                Zone zone;
                zone.Solo = false;
                zone.Mute = false;
                zone.DoubleOctave = false;
                zone.Device = NULL;
                zone.DeviceID = group.ID;
                bool found = false;
                for (auto d = 0U; d < m_current.Rack.size(); ++d)
                    if (m_current.Rack[d].Name == group.Name)
                        found = true;
                if (!found)
                {
                    Device newDevice;
                    newDevice.ID = group.ID;
                    newDevice.Name = group.Name;
                    m_current.Rack.push_back(newDevice);
                }

                zone.Volume = group.Gain;

                ForteMIDIFilter &filter = group.PluginChain.PlugIn[0].MIDIFilterSet.MIDIFilter[0];

                for (auto i = 0U; i<filter.MapChannel.size(); ++i)
                {
                    if (filter.MapChannel[i].From == 1)
                    {
                        ForteKey &key = filter.MapChannel[i].Key;
                        zone.LowKey = key.Low;
                        zone.HighKey = key.High;
                        zone.Transpose = key.Transpose;
                    }
                }

                ForteOnSetScene &onSetScene = group.PluginChain.PlugIn[0].OnSetScene;
                if (onSetScene.ProgramChange.size()>0)
                {
                    zone.Program = onSetScene.ProgramChange[0].Program;
                    zone.Bank = onSetScene.ProgramChange[0].Bank;
                }
                else
                {
                    zone.Program = -1;
                    zone.Bank = -1;
                }

                performance.Zones.push_back(zone);
            }
        }
        m_current.Performances.push_back(performance);
    }

    ResolveIDs();
}

void Performer::ResolveIDs()
{
    // Resolve songs in setlists
    for (auto sl = 0U; sl < m_current.SetLists.size(); ++sl)
        for (auto sg = 0U; sg < m_current.SetLists[sl].SongIDs.size(); ++sg)
            for (auto i = 0U; i < m_current.Songs.size(); ++i)
                if (m_current.Songs[i].ID == m_current.SetLists[sl].SongIDs[sg])
                    m_current.SetLists[sl].Songs.push_back(&m_current.Songs[i]);

    // Resolve performances in songs
    for (auto sg = 0U; sg < m_current.Songs.size(); ++sg)
        for (auto p = 0U; p < m_current.Songs[sg].PerformanceIDs.size(); ++p)
            for (auto i = 0U; i < m_current.Performances.size(); ++i)
                if (m_current.Performances[i].ID == m_current.Songs[sg].PerformanceIDs[p])
                    m_current.Songs[sg].Performances.push_back(&m_current.Performances[i]);

    // Resolve devices in performances
    for (auto p = 0U; p < m_current.Performances.size(); ++p)
        for (auto z = 0U; z < m_current.Performances[p].Zones.size(); ++z)
            for (auto d = 0U; d < m_current.Rack.size(); ++d)
                if (m_current.Performances[p].Zones[z].DeviceID == m_current.Rack[d].ID)
                    m_current.Performances[p].Zones[z].Device = &m_current.Rack[d];
}