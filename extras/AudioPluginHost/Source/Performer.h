#pragma once

#include <string>
#include <vector>
#include "ForteEmulator/ForteSDK/XmlArchive.h"

using namespace std;

class Device
{
public:
    int ID;
    string Name;
    string PluginName;
    void *m_node;
    void *m_gainNode;

	template<class A>
	void Serialize(A& ar)
	{
		AR(ID, XmlAttribute);
        AR(Name, XmlAttribute);
        AR(PluginName, XmlAttribute);
    }
};

class Zone
{
public:
    int DeviceID;
    Device *Device;
    int Bank;
    int Program;
    string Data;
    float Volume;
    bool Solo;
    bool Mute;
    bool DoubleOctave;
	bool Arpeggiator;
    int Transpose;
    int LowKey;
    int HighKey;

	template<class A>
	void Serialize(A& ar)
	{
        if (ar.IsSaving() && Mute)
            return; // dont save the muted zones

        if (ar.IsSaving())
            Solo = false;

		AR(DeviceID, XmlAttribute);
		AR(Bank, XmlAttribute);
		AR(Program, XmlAttribute);
		AR(Data);
		AR(Volume, XmlAttribute);
        AR(Solo, XmlAttribute);
        AR(Mute, XmlAttribute);
        AR(DoubleOctave, XmlAttribute);
		AR(Arpeggiator, XmlAttribute);
		AR(Transpose, XmlAttribute);
		AR(LowKey, XmlAttribute);
		AR(HighKey, XmlAttribute, 127);
	}
};

class PerformanceType
{
public:
    int ID;
    string Name;
    float Tempo;
    vector<Zone> Zone;

	template<class A>
	void Serialize(A& ar)
	{
		AR(ID, XmlAttribute);
		AR(Name, XmlAttribute);
		AR(Tempo, XmlAttribute);
		AR(Zone);
	}
};

class Integer
{
public:
    Integer(int id = 0) : ID(id) {}

    int ID;

    template<class A>
    void Serialize(A& ar)
    {
        AR(ID, XmlAttribute);
    }
};

class Song
{
public:
    int ID;
    string Name;
	// see if we can change OPX to use #3
    vector<Integer> Performance;
    vector<PerformanceType*> PerformancePtr;

	template<class A>
	void Serialize(A& ar)
	{
		AR(ID, XmlAttribute);
		AR(Name, XmlAttribute);
		AR(Performance);
	}
};

class SetList
{
public:
	int ID;
    string Name;
    vector<Song*> SongPtr;
    vector<Integer> Song;

	template<class A>
	void Serialize(A& ar)
	{
		AR(ID, XmlAttribute);
		AR(Name, XmlAttribute);
		AR(Song);
	}
};

class RacksType
{
public:
    vector<Device> Rack;
    template<class A>
    void Serialize(A& ar)
    {
        AR(Rack);
    }
};

class SetListsType
{
public:
    vector<SetList> SetList;
    template<class A>
    void Serialize(A& ar)
    {
        AR(SetList);
    }
};

class SongsType
{
public:
    vector<Song> Song;
    template<class A>
    void Serialize(A& ar)
    {
        AR(Song);
    }
};

class PerformancesType
{
public:
    vector<PerformanceType> Performance;
    template<class A>
    void Serialize(A& ar)
    {
        AR(Performance);
    }
};

class PerformerFile
{
public:
    SetListsType SetLists;
    RacksType Racks;
    SongsType Songs;
    PerformancesType Performances;
	int CurrentSetListID;

	template<class A>
	void Serialize(A& ar)
	{
		AR(CurrentSetListID, XmlAttribute);
		AR(SetLists);
		AR(Racks);
		AR(Songs);
		AR(Performances);
	}
};

class Performer
{
public:
    void Import(const char *file);
    void ResolveIDs();

    template<class A>
    void Serialize(A& ar)
    {
        AR(Root);
    }

    PerformerFile Root;
};