//
// Created by stumbo on 2019/11/15.
//

#include "ExpCollection.h"
#include "Exp.h"
#include "MergedExp.h"
#include "MapTwig.h"

void tmap::ExpCollection::setLeftGateOfCurrent(size_t leftGate)
{
    mExperiencesData.back()->setLeftGate(leftGate);
}

void tmap::ExpCollection::setLeftGateOfCurrent(const TopoVec2& gatePos)
{
    auto cloestGate = mExperiencesData.back()->expData()->findTheCloestGate(gatePos);
    setLeftGateOfCurrent(cloestGate);
}

void tmap::ExpCollection::addNewExpAndAddLoopClosures(tmap::ExpPtr expPtr,
                                                      tmap::MapTwigCollection& twigMaster)
{
    expPtr->setSerial(mExperiencesData.size());

    auto & vecSameType = mClassification[expPtr->expData()->type()];

    for (size_t i = vecSameType.size() - 1; i >= 0; ++i) {
        Exp* sameTypeExp = vecSameType[i];
        double poss1 = sameTypeExp->expData()->quickMatch(*expPtr->expData(), 1);
        if (poss1 < TOLLERANCE_1ST_MATCH_EXP) {
            continue;
        }

        //TODO 不要忘了单独闭环的情况
        auto& mergedExps = sameTypeExp->getMergedExps();
        for (const auto& iter : mergedExps) {
            auto mergedExp = iter.lock();
            if (mergedExp) {
                auto matchResult = mergedExp->detailedMatching(*expPtr->expData());
                if (matchResult->possibility > TOLLERANCE_2ND_MATCH_MERGEDEXP) {
                    /// VERY IMPORTANT PART
                    MergedExpPtr theNewMerged;
                    auto closureTwigs = mergedExp->getLoopClosureMaps();
                    if (!closureTwigs.empty()) {
                        /// TODO 准备构造新的MergedExp
//                        theNewMerged
                    }
                    for (auto & twig2born : closureTwigs) {
                        if (!twig2born->hasChildren()) {
                            twig2born->setDieAt(expPtr->serial());
                            auto newTwigAssumingNew = twigMaster.bornOne(twig2born, 1.0);
                            newTwigAssumingNew->nodeCountPlus();
                        }
                        auto twigWithClosure = twigMaster.bornOne(twig2born,
                                matchResult->possibility);
                    }
                }
            } /// TODO 根据没用的数量来判断要不要重新做这个表
        }

        sameTypeExp->cleanUpMergedExps();
    }

    mExperiencesData.push_back(std::move(expPtr));
}
