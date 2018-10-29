#pragma once

#include <string>
#include <vector>

using namespace std;

class Device
{
public:
    int ID;
    string Name;
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
    int Transpose;
    int LowKey;
    int HighKey;
};

class Performance
{
public:
    int ID;
    string Name;
    float Tempo;
    vector<Zone> Zones;
};

class Song
{
public:
    int ID;
    string Name;
    vector<int> PerformanceIDs;
    vector<Performance*> Performances;
};

class SetList
{
public:
    string Name;
    vector<Song*> Songs;
    vector<int> SongIDs;
};

class PerformerFile
{
public:
    vector<SetList> SetLists;
    vector<Device> Rack;
    vector<Song> Songs;
    vector<Performance> Performances;
};

class Performer
{
public:
    Performer();
    void ResolveIDs();

    PerformerFile m_current;
};