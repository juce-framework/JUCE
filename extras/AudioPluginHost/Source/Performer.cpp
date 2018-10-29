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
    const char *fileToLoad = "C:\\Users\\ben.crossman\\Desktop\\JUCE-ben\\extras\\AudioPluginHost\\Queen.rcf";
    ForteFile file;
    memset(&file, 0, sizeof(ForteFile));
    XmlArchive::Load(fileToLoad, file);
	
	// Devices
	for (auto d = 0U; d < file.Rack.MixerScene[0].Mixer.Group.InputGroup.size(); ++d)
	{
		auto group = file.Rack.MixerScene[0].Mixer.Group.InputGroup[d];
		Device newDevice;
		newDevice.ID = group.ID;
		newDevice.Name = group.Name;
		if (newDevice.Name == "Arpeggiator")
			continue;

		m_current.Rack.push_back(newDevice);
		if (newDevice.Name == "Korg M1")
		{
			newDevice.ID++;
			m_current.Rack.push_back(newDevice);
		}
	}

    // Setlists
    for (unsigned int sl = 0; sl < file.Rack.Setlists.Setlist.size(); ++sl)
    {
		auto &setlist = file.Rack.Setlists.Setlist[sl];
        SetList newSetList;
		newSetList.ID = setlist.ID;
        newSetList.Name = setlist.Name;
        for (unsigned int sr = 0; sr < setlist.SongRef.size(); ++sr)
            newSetList.SongIDs.push_back(setlist.SongRef[sr].ID);
        m_current.SetLists.push_back(newSetList);
    }

	m_current.CurrentSetListID = m_current.SetLists[file.Rack.Setlists.Active].ID;

    // Songs
    for (unsigned int si = 0; si < file.Rack.Setlists.Song.size(); ++si)
    {
		auto &song = file.Rack.Setlists.Song[si];
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
		auto &mixer = file.Rack.MixerScene[mi];

        string songName = mixer.Name;
        Replace(songName, "|", " ");
        TrimRight(songName);

        Performance performance;
        performance.ID = mixer.ID;
        performance.Name = songName;
        performance.Tempo = mixer.Mixer.Tempo.BPM;
        for (int ig = 0; ig<(int)mixer.Mixer.Group.InputGroup.size(); ++ig)
        {
			auto &group = mixer.Mixer.Group.InputGroup[ig];
            if (!group.Mute)
            {
				auto &onSetScene = group.PluginChain.PlugIn[0].OnSetScene;

				for (auto pass = 0U; pass < 2; ++pass)
				{
					if (pass == 1 && onSetScene.ProgramChange.size() <= 1)
						continue; 

					Zone zone;
					zone.Solo = false;
					zone.Mute = false;
					zone.DoubleOctave = false;
					zone.Arpeggiator = false;
					zone.Volume = group.Gain;
					zone.Device = NULL;
					zone.DeviceID = group.ID + pass;

					auto &filter = group.PluginChain.PlugIn[0].MIDIFilterSet.MIDIFilter[0];
					if (onSetScene.ProgramChange.size() <= 1 && filter.MapChannel.size() == 2 && abs(filter.MapChannel[0].Key.Transpose - filter.MapChannel[0].Key.Transpose) == 12) // see if this is a transpose
					{
						zone.DoubleOctave = true;
						zone.Transpose = filter.MapChannel[0].Key.Transpose < filter.MapChannel[1].Key.Transpose ? filter.MapChannel[0].Key.Transpose : filter.MapChannel[1].Key.Transpose;
					}
					else
						zone.Transpose = (pass < filter.MapChannel.size()) ? filter.MapChannel[pass].Key.Transpose : 0;

					zone.LowKey = (pass < filter.MapChannel.size()) ? filter.MapChannel[pass].Key.Low : 0;
					zone.HighKey = (pass < filter.MapChannel.size()) ? filter.MapChannel[pass].Key.High : 127;
				
					if (pass < onSetScene.ProgramChange.size())
					{
						zone.Program = onSetScene.ProgramChange[pass].Program;
						zone.Bank = onSetScene.ProgramChange[pass].Bank;
					}
					else
					{
						zone.Program = -1;
						zone.Bank = -1;
					}

					if (group.PluginChain.PlugIn[0].MIDIFilterSet.vMIDIFilter.size())
					{
						auto &filter2 = group.PluginChain.PlugIn[0].MIDIFilterSet.vMIDIFilter[0]; // should be only one
						if (!filter2.Disabled && filter2.Name.find("Arpeggiator") != -1)
							zone.Arpeggiator = true;
					}

					if (group.Name == "Arpeggiator")
					{
						// copy parameters to other plugin
						for (auto z = 0U; z<performance.Zones.size(); ++z)
							if (performance.Zones[z].Arpeggiator)
							{
								performance.Zones[z].LowKey = zone.LowKey;
								performance.Zones[z].HighKey = zone.HighKey;
								performance.Zones[z].Transpose = zone.Transpose;
							}
						continue;
					}

					performance.Zones.push_back(zone);

				}
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
		{
			for (auto d = 0U; d < m_current.Rack.size(); ++d)
				if (m_current.Performances[p].Zones[z].DeviceID == m_current.Rack[d].ID)
					m_current.Performances[p].Zones[z].Device = &m_current.Rack[d];
			if (m_current.Performances[p].Zones[z].Device == NULL)
			{
				swap(m_current.Performances[p].Zones[z], m_current.Performances[p].Zones.back());
				m_current.Performances[p].Zones.pop_back();
				z--;
			}
		}

}