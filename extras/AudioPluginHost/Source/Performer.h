#pragma once

#include <string>
#include <vector>

using namespace std;

class Device
{
public:
    int ID;
    string Name;

	template<class A>
	void Serialize(A& ar)
	{
		AR(ID);
		AR(Name);
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
		AR(DeviceID);
		AR(Bank);
		AR(Program);
		AR(Data);
		AR(Volume);
		AR(Solo);
		AR(Mute);
		AR(DoubleOctave);
		AR(Arpeggiator);
		AR(Transpose);
		AR(LowKey);
		AR(HighKey);
	}
};

class Performance
{
public:
    int ID;
    string Name;
    float Tempo;
    vector<Zone> Zones;

	template<class A>
	void Serialize(A& ar)
	{
		AR(ID);
		AR(Name);
		AR(Tempo);
		AR(Zones);
	}
};

class Song
{
public:
    int ID;
    string Name;
	// see if we can change OPX to use #3
    vector<int> PerformanceIDs;
    vector<Performance*> Performances;

	template<class A>
	void Serialize(A& ar)
	{
		AR(ID);
		AR(Name);
		AR(PerformanceIDs);
	}
};

class SetList
{
public:
	int ID;
    string Name;
    vector<Song*> Songs;
    vector<int> SongIDs;

	template<class A>
	void Serialize(A& ar)
	{
		AR(ID);
		AR(Name);
		AR(SongIDs);
	}
};

class PerformerFile
{
public:
    vector<SetList> SetLists;
    vector<Device> Rack;
    vector<Song> Songs;
    vector<Performance> Performances;
	int CurrentSetListID;

	template<class A>
	void Serialize(A& ar)
	{
		AR(CurrentSetListID);
		AR(SetLists);
		AR(Rack);
		AR(Songs);
		AR(Performances);
	}
};

class Performer
{
public:
    Performer();
    void ResolveIDs();

    PerformerFile m_current;
};