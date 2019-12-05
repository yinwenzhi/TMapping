//
// Created by stumbo on 2019/11/17.
//

#include "MapTwig.h"
#include "Tmapping.h"
#include <iostream>

using namespace std;

tmap::MapTwigPtr tmap::MapTwig::getAdamTwig()
{
    MapTwigPtr adam(new MapTwig(0, nullptr, 0, 1.0));
    return adam;
}

tmap::MapTwig::MapTwig(size_t bornAt, MapTwigPtr father,
                       size_t nSerial, double confidence) : borndAt(bornAt),
                                                            father(std::move(father)),
                                                            nSerial(nSerial),
                                                            mConfidence(confidence)
{}

tmap::MapTwigPtr tmap::MapTwig::bornOne(size_t newSerial, double newConfidence)
{
    if (status != MapTwigStatus::EXPIRED) {
        cerr << FILE_AND_LINE << " You didn't born a branch from a EXPIRED branch!" << endl;
    }
    MapTwigPtr newTwig(
            new MapTwig(this->mDieAt, shared_from_this(), newSerial, newConfidence));
    /// 在后代列表后插入这个新的后代
    this->mChildren.emplace_back(newTwig);
    return newTwig;
}

void tmap::MapTwig::setExpired()
{
    if (status == MapTwigStatus::EXPIRED) {
        cerr << FILE_AND_LINE << " You expired a mapTwig more than once!" << endl;
    }
    status = MapTwigStatus::EXPIRED;
}

tmap::MapTwigStatus tmap::MapTwig::getStatus() const
{
    return status;
}

const tmap::MergedExpPtr& tmap::MapTwig::getTheArrivingSimiliarExp() const
{
    return theArrivingSimiliarExp;
}

double tmap::MapTwig::xCoe(double coe)
{
    mConfidence *= coe;
    return mConfidence;
}

const vector<tmap::MapTwigWePtr>& tmap::MapTwig::getChildren() const
{
    return mChildren;
}

const vector<tmap::MergedExpPtr>& tmap::MapTwig::getLoopClosures() const
{
    return mLoopClosures;
}

bool tmap::MapTwig::hasChildren() const
{
    return !mChildren.empty();
}

double tmap::MapTwig::getConfidence() const
{
    return mConfidence;
}

void tmap::MapTwig::nodeCountPlus()
{
    nTopoNode++;
}

void tmap::MapTwig::setDieAt(size_t dieAt)
{
    mDieAt = dieAt;
}
