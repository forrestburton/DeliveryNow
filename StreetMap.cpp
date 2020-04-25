#include "provided.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string>
#include <vector>
#include <functional>
#include <cctype>
#include "ExpandableHashMap.h"
using namespace std;

//Loads text file of GeoCoords into an expandable hash map

unsigned int hasher(const GeoCoord& g)
{
    return std::hash<string>()(g.latitudeText + g.longitudeText);
}

class StreetMapImpl
{
public:
    StreetMapImpl();
    ~StreetMapImpl();
    bool load(string mapFile);
    bool getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const;

private:
    bool isStreetName(string line);
    void insertInHashMap(const GeoCoord& coord, StreetSegment seg);

    ExpandableHashMap<GeoCoord, vector<StreetSegment>>* m_hashMap;
};

StreetMapImpl::StreetMapImpl()
{
    m_hashMap = new ExpandableHashMap<GeoCoord, vector<StreetSegment>>;
}

StreetMapImpl::~StreetMapImpl()
{
    delete m_hashMap;
}

bool StreetMapImpl::isStreetName(string line)
{
    for (int i = 0; i != line.size(); i++)
    {
        if (isalpha(line[i]))
            return true;
    }
    return false;
}


bool StreetMapImpl::load(string mapFile)
{
    m_hashMap->reset();

    ifstream inf(mapFile);  //open file

    if (!inf)  //test failure
    {
        cerr << "Cannot open expenses file!" << endl;
        return false;
    }

    string line, nameOfStreet;
    while (getline(inf, line))  //read each line
    {
        istringstream iss(line);  //creates input stringstream from line
        string startLat, startLong, endLat, endLong;

        if (isStreetName(line))   //if its a street name read in the name then continue
        {
            string temp;
            nameOfStreet = "";
            while (iss >> temp)
            {
                if (nameOfStreet != "")
                    nameOfStreet += " ";
                nameOfStreet += temp;
            }
            continue;
        }

        if (!(iss >> startLat >> startLong >> endLat >> endLong))  //if its not a segment then continue
            continue;

        GeoCoord start(startLat, startLong);   //starting coord
        GeoCoord end(endLat, endLong);         //ending coord
        StreetSegment s(start, end, nameOfStreet);   //dynamically allocate new segment: startcoord, endcoord, streetname
        insertInHashMap(start, s);   //map starting coord to segment
        StreetSegment sReversed(end, start, nameOfStreet);   //dynamically allocate new segment: startcoord, endcoord, streetname
        insertInHashMap(end, sReversed);   //map ending coord to segment 
    }
    return true;
}

void StreetMapImpl::insertInHashMap(const GeoCoord& coord, StreetSegment seg)
{
    if (m_hashMap->find(coord) != nullptr)  //association exists   //mapping geoCoords to a vector of street pointers
    {
        vector<StreetSegment>* segs = m_hashMap->find(coord);   //get vector of segments for this specific coord   
        segs->push_back(seg);   //add new streetsegment pointer to street segment to vector
    }
    else  //association doesn't exist
    {
        vector<StreetSegment> newSeg;
        newSeg.push_back(seg);          //add segment to vector 
        m_hashMap->associate(coord, newSeg);   //add new association 
    }
}

bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
    if (m_hashMap->find(gc) != nullptr)  //if geocoordinate exists in hashmap
    {
        while (!segs.empty())
        {
            vector<StreetSegment>::iterator it = segs.begin();
            it = segs.erase(it);
        }

        vector<StreetSegment>* neededSegs = m_hashMap->find(gc);   //get vector of segments mapped to specific geocoordinate
        for (vector<StreetSegment>::iterator it = (*neededSegs).begin(); it != (*neededSegs).end(); it++)  //copy segments into segs
            segs.push_back(*it);
        return true;
    }
    return false;
}

//******************** StreetMap functions ************************************

// These functions simply delegate to StreetMapImpl's functions.
// You probably don't want to change any of this code.

StreetMap::StreetMap()
{
    m_impl = new StreetMapImpl;
}

StreetMap::~StreetMap()
{
    delete m_impl;
}

bool StreetMap::load(string mapFile)
{
    return m_impl->load(mapFile);
}

bool StreetMap::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
    return m_impl->getSegmentsThatStartWith(gc, segs);
}
