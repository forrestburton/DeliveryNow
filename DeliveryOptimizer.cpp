#include "provided.h"
#include <vector>
using namespace std;

//**IN PROGRESS** Optimizes delivery route 

class DeliveryOptimizerImpl
{
public:
    DeliveryOptimizerImpl(const StreetMap* sm);
    ~DeliveryOptimizerImpl();
    void optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const;

private:
    const StreetMap* m_streetMap;
    PointToPointRouter* m_router;
};

DeliveryOptimizerImpl::DeliveryOptimizerImpl(const StreetMap* sm)
    :m_streetMap(sm)
{
    m_router = new PointToPointRouter(m_streetMap);
}

DeliveryOptimizerImpl::~DeliveryOptimizerImpl()
{
    delete m_router;
}

void DeliveryOptimizerImpl::optimizeDeliveryOrder(   //DeliveryRequest contains a string for the name of the item and a GeoCoord location for the location to deliver
    const GeoCoord& depot,
    vector<DeliveryRequest>& deliveries,
    double& oldCrowDistance,
    double& newCrowDistance) const
{
    oldCrowDistance = 0;
    GeoCoord startLocation = depot;

    for (vector<DeliveryRequest>::iterator it = deliveries.begin(); it != deliveries.end(); it++)
    {
        GeoCoord deliveryLocation = (*it).location;  //get location to deliver
        double distance = distanceEarthMiles(startLocation, deliveryLocation);   //distance from starting location to delivery
        startLocation = deliveryLocation; //update starting location
        oldCrowDistance += distance;
    }
    oldCrowDistance += distanceEarthMiles(startLocation, depot);   //have to go back to depot
    newCrowDistance = oldCrowDistance;
}

//******************** DeliveryOptimizer functions ****************************

DeliveryOptimizer::DeliveryOptimizer(const StreetMap* sm)
{
    m_impl = new DeliveryOptimizerImpl(sm);
}

DeliveryOptimizer::~DeliveryOptimizer()
{
    delete m_impl;
}

void DeliveryOptimizer::optimizeDeliveryOrder(
    const GeoCoord& depot,
    vector<DeliveryRequest>& deliveries,
    double& oldCrowDistance,
    double& newCrowDistance) const
{
    return m_impl->optimizeDeliveryOrder(depot, deliveries, oldCrowDistance, newCrowDistance);
}
