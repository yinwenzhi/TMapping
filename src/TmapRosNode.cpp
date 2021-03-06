//
// Created by stumbo on 2019/12/19.
//

#include "TmapRosNode.h"
#include <chrono>

using namespace tmap;
using namespace std;

tmap::TmapRosNode::TmapRosNode() :
        n(),
        srvNewExp(n.advertiseService(TMAP_STD_SERVICE_NAME_NEW_EXP,
                                     &TmapRosNode::cbSrvNewExp, this)),
        srvGateMovement(n.advertiseService(TMAP_STD_SERVICE_NAME_GATE_MOVE,
                                           &TmapRosNode::cbSrvGateMovement, this)),
        srvGetmaps(n.advertiseService(TMAP_STD_SERVICE_NAME_GET_MAPS,
                                      &TmapRosNode::cbSrvGetMaps, this)),
        srvSetSurviver(n.advertiseService(TMAP_STD_SERVICE_NAME_SET_SURVIVERS,
                                      &TmapRosNode::cbSrvSetSuriviers, this)),
        srvReset(n.advertiseService(TMAP_STD_SERVICE_NAME_RESET,
                                      &TmapRosNode::cbSrvReset, this)),
        srvChHis(n.advertiseService(TMAP_STD_SERVICE_GET_CHAMPION_HISTORY,
                                      &TmapRosNode::cbSrvGetEvoOfChampion, this)),
        srvSaveChampion(n.advertiseService(TMAP_STD_SERVICE_SAVE_CHAMPION,
                                      &TmapRosNode::cbSrvSaveChampion, this)),
        mTmappingCore(new TopoMapping)
{

}

static double temp;

bool TmapRosNode::cbSrvNewExp(tmapping::NewExpRequest& req,
                              tmapping::NewExpResponse& res)
{
    cout << "get new exp info" << endl;
#if TMAPPING_CONFIG_LOG_TIME
    auto startTime = std::chrono::system_clock::now();
    static bool once = false;
    if (!once) {
        once = true;
        cout << "nMap\t newExpTime \t gateTime \t All" << endl;
    }
#endif
    const auto& newExp = make_shared<Exp>(JsonHelper::Str2JS(req.jNewExp));
    mTmappingCore->arriveNewExp(newExp);
#if TMAPPING_CONFIG_LOG_TIME
    auto endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> diff(endTime - startTime);
    temp = diff.count();
//    cout << "\nTIME newExp: " << diff.count() << endl;
    cout << diff.count() << '\t';
#endif
    res.jChampionStatus = JsonHelper::JS2Str(mTmappingCore->getChampionDefendedCount());
    return true;
}

bool TmapRosNode::cbSrvGateMovement(tmapping::GateMovementRequest& req,
                                    tmapping::GateMovementResponse& res)
{
    cout << "get gate movement info" << endl;
    auto startTime = std::chrono::system_clock::now();
    auto gateID = JsonHelper::Str2JS(req.jGateMove).asInt();
    mTmappingCore->setLeftGate(gateID);
#if TMAPPING_CONFIG_LOG_TIME
    auto endTime = std::chrono::system_clock::now();
    std::chrono::duration<double> diff(endTime - startTime);
    cout << diff.count() << '\t' << diff.count() + temp << endl;
#endif
    return true;
}

bool
TmapRosNode::cbSrvGetMaps(tmapping::GetMapsRequest& req,
                          tmapping::GetMapsResponse& res)
{
    cout << "Processing [GetMaps] request..." << endl;
    auto topoMaps = mTmappingCore->getTopMaps(req.nMapRequired);
    res.jMaps = JsonHelper::JS2Str(topoMaps);
    cout << "Returned " << topoMaps["maps"].size() << " maps" << endl;
    return true;
}

bool TmapRosNode::cbSrvSetSuriviers(tmapping::SetSurviverMapsNumRequest& req,
                                    tmapping::SetSurviverMapsNumResponse& res)
{
    mTmappingCore->setNSurviverMaps(req.nMaps);
    cout << "The survivor count is set to " << req.nMaps << endl;
    return true;
}

bool TmapRosNode::cbSrvReset(std_srvs::EmptyRequest& req, std_srvs::EmptyResponse& res)
{
    mTmappingCore.reset(new TopoMapping);
    cout << "Mapping core is reseted" << endl;
    return true;
}

bool
TmapRosNode::cbSrvGetEvoOfChampion(tmapping::GetMapsRequest& req, tmapping::GetMapsResponse& res)
{
    res.jMaps = JsonHelper::JS2Str(mTmappingCore->getChampionHistory());
    return true;
}

bool TmapRosNode::cbSrvSaveChampion(std_srvs::EmptyRequest& req, std_srvs::EmptyResponse& res)
{
    int r = JsonHelper::saveJson(mTmappingCore->getTopMaps(1)["maps"][0], "currentChampion", true);
    if ( r>=0 ) {
        cout << "save Champion map success!";
    } else {
        cerr << "save Champion map failure!";
    }
    return r >= 0;
}

