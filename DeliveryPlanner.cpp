#include "provided.h"
#include <vector>
using namespace std;

//Uses contructed routes to generate directions based upon such routes

class DeliveryPlannerImpl
{
public:
    DeliveryPlannerImpl(const StreetMap* sm);
    ~DeliveryPlannerImpl();
    DeliveryResult generateDeliveryPlan(
        const GeoCoord& depot,
        const vector<DeliveryRequest>& deliveries,
        vector<DeliveryCommand>& commands,
        double& totalDistanceTravelled) const;

private:
    void getTravelDirection(string& travelDirection, const double& angle) const;
    void proceedCommand(vector<DeliveryCommand>& commands, const GeoCoord& startCoord, const StreetSegment& endSeg, const double& startAngle) const;

    const StreetMap* m_streetMap;
    DeliveryOptimizer* m_deliveryOptimizer;   
};

DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm)
    :m_streetMap(sm)
{
    m_deliveryOptimizer = new DeliveryOptimizer(m_streetMap);
}

DeliveryPlannerImpl::~DeliveryPlannerImpl()
{
    delete m_deliveryOptimizer;
}

DeliveryResult DeliveryPlannerImpl::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,        
    double& totalDistanceTravelled) const
{
    double oldCrowDistance = 0;
    double newCrowDistance = 0;
    vector<DeliveryRequest> betterDeliveries = deliveries;
    m_deliveryOptimizer->optimizeDeliveryOrder(depot, betterDeliveries, oldCrowDistance, newCrowDistance);

    PointToPointRouter router(m_streetMap);  
    vector<list<StreetSegment>> deliveryRoute; 
    totalDistanceTravelled = 0;
    list<StreetSegment> currentRoute;
    GeoCoord start = depot;
    double distStartToFinish = 0;

    for (vector<DeliveryRequest>::iterator it = betterDeliveries.begin(); it != betterDeliveries.end(); it++)
    {
        GeoCoord finish = (*it).location;
       
        distStartToFinish = 0;
        DeliveryResult deliveryResult = router.generatePointToPointRoute(start, finish, currentRoute, distStartToFinish);  
        if (deliveryResult == NO_ROUTE)
            return NO_ROUTE;
        else if (deliveryResult == BAD_COORD)
            return BAD_COORD;
        else
        {
            totalDistanceTravelled += distStartToFinish; 
            deliveryRoute.push_back(currentRoute);   
            start = finish;      
        }
    }

    distStartToFinish = 0;
    DeliveryResult deliveryResult = router.generatePointToPointRoute(start, depot, currentRoute, distStartToFinish);
    if (deliveryResult == NO_ROUTE)
        return NO_ROUTE;
    else if (deliveryResult == BAD_COORD)
        return BAD_COORD;
    else
    {
        totalDistanceTravelled += distStartToFinish;
        deliveryRoute.push_back(currentRoute);  
    }

    vector<DeliveryRequest>::iterator request = betterDeliveries.begin();
    for (int i = 0; i != deliveryRoute.size(); i++)
    {
        //GET ROUTE
        list<StreetSegment> itemRoute = deliveryRoute[i];

        if (currentRoute.size() < 1)  //if no routes to traverse, we already completed all our deliveries
        {
            DeliveryCommand done;
            string deliveryItem = request->item;  
            done.initAsDeliverCommand(deliveryItem);  
            request++;
            continue;
        }

        //PROCESS SEGMENTS OF ROUTE
        list<StreetSegment>::iterator ahead = itemRoute.begin();
        list<StreetSegment>::iterator behind = ahead;
        while (behind != itemRoute.end())  
        {
            GeoCoord startCoord;
            double startAngle;
            if (ahead == itemRoute.end())  //check if done
            {
                behind = ahead;
                break;
            }

            if (behind->name != ahead->name)   //turn?
            {
                double turnAngle = angleBetween2Lines((*behind), (*ahead));
                string turnCommand;
                if (turnAngle >= 1 && turnAngle < 180)
                    turnCommand = "left";
                else if (turnAngle >= 180 && turnAngle <= 359)
                    turnCommand = "right";

                if (turnCommand != "")  
                {
                    DeliveryCommand turn;
                    string turnStreetName = (*ahead).name;  
                    turn.initAsTurnCommand(turnCommand, turnStreetName);
                    commands.push_back(turn);
                }
                startCoord = (*ahead).start;  
                startAngle = angleOfLine(*ahead);
                behind = ahead;
                ahead++;
            }
            else
            {
                startCoord = (*ahead).start;  
                startAngle = angleOfLine(*ahead);
            }

            while (ahead != itemRoute.end() && behind->name == ahead->name)   
            {
                behind = ahead;
                ahead++;
            }
            StreetSegment endSeg = (*behind);  
            proceedCommand(commands, startCoord, endSeg, startAngle);
        }

        if (i != deliveryRoute.size() - 1)  //check if heading back to starting location
        {
            DeliveryCommand delivered;
            delivered.initAsDeliverCommand(request->item);
            commands.push_back(delivered);
            request++;  
        }
    }
    return DELIVERY_SUCCESS;
}

void DeliveryPlannerImpl::proceedCommand(vector<DeliveryCommand>& commands, const GeoCoord& startCoord, const StreetSegment& endSeg, const double& startAngle) const
{
    GeoCoord endCoord = endSeg.end;
    string streetName = endSeg.name;

    double travelDistance = distanceEarthMiles(startCoord, endCoord); 
    string travelDirection;
    getTravelDirection(travelDirection, startAngle);   

    DeliveryCommand proceed;  
    proceed.initAsProceedCommand(travelDirection, streetName, travelDistance);
    commands.push_back(proceed);
}

void DeliveryPlannerImpl::getTravelDirection(string& travelDirection, const double& angle) const
{
    if (angle < 22.5)
    {
        travelDirection = "east";
        return;
    }
    if (angle < 67.5)
    {
        travelDirection = "northeast";
        return;
    }
    if (angle < 112.5)
    {
        travelDirection = "north";
        return;
    }
    if (angle < 157.5)
    {
        travelDirection = "northwest";
        return;
    }
    if (angle < 202.5)
    {
        travelDirection = "west";
        return;
    }
    if (angle < 247.5)
    {
        travelDirection = "southwest";
        return;
    }
    if (angle < 292.5)
    {
        travelDirection = "south";
        return;
    }
    if (angle < 337.5)
    {
        travelDirection = "southeast";
        return;
    }
    travelDirection = "east";
}

//******************** DeliveryPlanner functions ******************************

DeliveryPlanner::DeliveryPlanner(const StreetMap* sm)
{
    m_impl = new DeliveryPlannerImpl(sm);
}

DeliveryPlanner::~DeliveryPlanner()
{
    delete m_impl;
}

DeliveryResult DeliveryPlanner::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    return m_impl->generateDeliveryPlan(depot, deliveries, commands, totalDistanceTravelled);
}
