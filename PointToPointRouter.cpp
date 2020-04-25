#include "provided.h"
#include <queue>
#include <list>
#include <set>
#include "ExpandableHashMap.h"
#include <list>
using namespace std;

//Initializes a StreetMap object containing an expandable hash map containing coordinates and uses those coordinates to contruct a viable route from a starting coordinate to ending coordinate

class PointToPointRouterImpl
{
public:
    PointToPointRouterImpl(const StreetMap* sm);
    ~PointToPointRouterImpl();
    DeliveryResult generatePointToPointRoute(
        const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        double& totalDistanceTravelled) const;

private:
    bool getBestRoute(list<StreetSegment>& route, const GeoCoord& start, const GeoCoord& end, double& totalDistanceTravelled) const;
    void getRouteHistory(ExpandableHashMap<GeoCoord, GeoCoord>& routeMap, list<GeoCoord>& solutionOfCoords, const GeoCoord& end, double& totalDistanceTravelled) const;
    void rankSegs(vector<StreetSegment>& possibleSegs, const GeoCoord& end, int First, int Last) const;
    int quickSortSegs(vector<StreetSegment>& possibleSegs, const GeoCoord& end, int low, int high) const;
    void swapSegs(vector<StreetSegment>& possibleSegs, const int& low, const int& high) const;

    const StreetMap* m_streetMap;
};

PointToPointRouterImpl::PointToPointRouterImpl(const StreetMap* sm)
    :m_streetMap(sm)
{
}

PointToPointRouterImpl::~PointToPointRouterImpl()
{
}

DeliveryResult PointToPointRouterImpl::generatePointToPointRoute(
    const GeoCoord& start,
    const GeoCoord& end,
    list<StreetSegment>& route,
    double& totalDistanceTravelled) const
{
    vector<StreetSegment> startSegs;
    vector<StreetSegment> endSegs;

    if (m_streetMap->getSegmentsThatStartWith(start, startSegs) == false || m_streetMap->getSegmentsThatStartWith(end, endSegs) == false)  
        return BAD_COORD;
    else
    {
        while (!route.empty())   
        {
            list<StreetSegment>::iterator it = route.begin();
            it = route.erase(it);
        }

        if (start == end)  
        {
            totalDistanceTravelled = 0;  
            return DELIVERY_SUCCESS;
        }

        if (getBestRoute(route, start, end, totalDistanceTravelled)) //find the best route from start to end   
            return DELIVERY_SUCCESS;
    }

    return NO_ROUTE;
}

bool PointToPointRouterImpl::getBestRoute(list<StreetSegment>& route, const GeoCoord& start, const GeoCoord& end, double& totalDistanceTravelled) const
{
    ExpandableHashMap<GeoCoord, GeoCoord> routeMap; 
    vector<StreetSegment> possibleSegs;  
    list<GeoCoord> solutionOfCoords; 
    set<GeoCoord> history;  
    queue<GeoCoord> g;
    g.push(start);
    history.insert(start);
    while (!g.empty())
    {
        GeoCoord cur = g.front();
        g.pop();
        if (cur == end)    
        {
            getRouteHistory(routeMap, solutionOfCoords, end, totalDistanceTravelled); 

            list<GeoCoord>::iterator ahead = solutionOfCoords.begin();
            list<GeoCoord>::iterator behind = ahead;
            ahead++;
            while (ahead != solutionOfCoords.end())
            {
                GeoCoord startSeg = *behind;  //start of seg
                GeoCoord endSeg = *ahead;  //end of seg
                //get name of street
                vector<StreetSegment> temp;
                string streetName;
                m_streetMap->getSegmentsThatStartWith(startSeg, temp);   //temp holds vector containing segments that start wiht startSeg
                vector<StreetSegment>::iterator it = temp.begin();
                while (it != temp.end())  //iterate through vector until correct seg is found, which we know if endSegs match
                {
                    if ((*it).end == endSeg)
                    {
                        streetName = (*it).name;  //get name of street
                        break;
                    }
                    it++;
                }
                StreetSegment s(startSeg, endSeg, streetName);  //build street segment and add to route(list containing streetsegments that contain the correct route)
                route.push_back(s);
                behind = ahead;  //increment behind
                ahead++;  //increment ahead
            }
            return true;
        }

        m_streetMap->getSegmentsThatStartWith(cur, possibleSegs);  //get all segments with this coordinate
        int numSegs = possibleSegs.size();   //number of segments with this coordinate

        if (numSegs >= 2)  //if there is only 1 seg, there's nothing to sort
            rankSegs(possibleSegs, end, 0, numSegs - 1);   //sort segments based off distance to end GeoCoord

        int currentSegsChecked = 0;
        while (currentSegsChecked != numSegs)     //checking each possible coordinate I can move to from the current coordinate (cur)
        {
            StreetSegment s = possibleSegs[currentSegsChecked];         //right now im checking each possible coord I can move to randomly, which is what I don't wants
            GeoCoord possible = s.end;             //possible coordinate I can move to 

            if (history.find(possible) == history.end())     //make sure I haven't visited this coordinte before
            {
                g.push(possible);            //push coord onto queue
                history.insert(possible);        //Mark that I've been to this coordinate
                routeMap.associate(possible, cur);          //mark in history that I traveled from cur->new cur(possible0
            }
            currentSegsChecked++;
        }
    }
    return false;
}

void PointToPointRouterImpl::rankSegs(vector<StreetSegment>& possibleSegs, const GeoCoord& end, int First, int Last) const
{
    if (Last - First >= 1)  //can only sort 2 or more segs
    {
        int pivotIndex;
        pivotIndex = quickSortSegs(possibleSegs, end, First, Last);
        rankSegs(possibleSegs, end, First, pivotIndex - 1);    //left
        rankSegs(possibleSegs, end, pivotIndex + 1, Last);   //right
    }
}

int PointToPointRouterImpl::quickSortSegs(vector<StreetSegment>& possibleSegs, const GeoCoord& end, int low, int high) const
{
    //quicksort partition function
    int pivotIndex = low;
    GeoCoord pivot = possibleSegs[low].end;

    do
    {
        while (low <= high && distanceEarthMiles(possibleSegs[low].end, end) <= distanceEarthMiles(pivot, end))
            low++;
        while (distanceEarthMiles(possibleSegs[high].end, end) > distanceEarthMiles(pivot, end))
            high--;
        if (low < high)
            swapSegs(possibleSegs, high, low);
    } while (low < high);

    swapSegs(possibleSegs, pivotIndex, high);
    pivotIndex = high;
    return pivotIndex;
}

void PointToPointRouterImpl::swapSegs(vector<StreetSegment>& possibleSegs, const int& low, const int& high) const
{
    StreetSegment temp = possibleSegs[high];
    possibleSegs[high] = possibleSegs[low];
    possibleSegs[low] = temp;
}

void PointToPointRouterImpl::getRouteHistory(ExpandableHashMap<GeoCoord, GeoCoord>& routeMap, list<GeoCoord>& solutionOfCoords, const GeoCoord& end, double& totalDistanceTravelled) const
{
    const GeoCoord* curr = &end;

    while (curr != nullptr)   //while not at start
    {
        solutionOfCoords.push_front(*curr);
        const GeoCoord* prev = curr;
        curr = routeMap.find(*curr);
        if (curr != nullptr)
            totalDistanceTravelled += distanceEarthMiles(*curr, *prev);  //update total distance travelled
    }
}


//******************** PointToPointRouter functions ***************************

// These functions simply delegate to PointToPointRouterImpl's functions.
// You probably don't want to change any of this code.

PointToPointRouter::PointToPointRouter(const StreetMap* sm)
{
    m_impl = new PointToPointRouterImpl(sm);
}

PointToPointRouter::~PointToPointRouter()
{
    delete m_impl;
}

DeliveryResult PointToPointRouter::generatePointToPointRoute(
    const GeoCoord& start,
    const GeoCoord& end,
    list<StreetSegment>& route,
    double& totalDistanceTravelled) const
{
    return m_impl->generatePointToPointRoute(start, end, route, totalDistanceTravelled);
}
