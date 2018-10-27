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
    int Tempo;
    vector<Zone> Zones;
};

class Song
{
    string Name;
    vector<int> PerformanceIDs;
    vector<Performance> Performances;
};

class SetList
{
    string Name;
    vector<Song> Songs;
    vector<int> SongIDs;
};

class PerformerFile
{
    vector<SetList> SetLists;
    vector<Device> Rack;
    vector<Performance> Performances;
};