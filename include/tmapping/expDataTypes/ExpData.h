//
// Created by stumbo on 2019/11/15.
//

#ifndef TMAPPING_EXPDATA_H
#define TMAPPING_EXPDATA_H

#include <vector>
#include <memory>

#include "../tools/TopoTools.h"
#include "../gateTypes/GateTypes.h"
#include "../landmarkTypes/LandmarkTypes.h"

namespace tmap
{

enum class ExpDataType{Intersection, Corridor, Stair, BigRoom, SmallRoom};

class ExpData;
using ExpDataPtr = std::shared_ptr<ExpData>;
using ExpDataUnPtr = std::unique_ptr<ExpData>;
using ExpDataWePtr = std::weak_ptr<ExpData>;

struct MatchResult_IMPL{
    /// 可能性系数, 如果为0表示完全不可能
    double possibility;
    /// k = gateMapping2this[j], 则 mergedExpData->gates[j] 与this->gates[k]为同一gate
    std::vector<GateID> gateMapping2this;
    /// k = gateMapping2mergedExpData[j], 则 this->gates[j] 与mergedExpData->gates[k]为同一gate
    std::vector<GateID> gateMapping2mergedExpData;
    /// 两个ExpData的相对位移, 相对于that而言
    TopoVec2 displacement;
    /// 融合后产生的新ExpData, gate编号与that相吻合
    ExpDataPtr mergedExpData;
};

using MatchResult = std::unique_ptr<MatchResult_IMPL>;

/// 代表观测得到的一次地形数据, 比如一个路口, 一个房间的信息
class ExpData
{
    std::vector<GateUnPtr> mGates;
    std::vector<PLMUnPtr> posLandmarks;

public:
    virtual ExpDataType type() const = 0;

    const std::vector<GateUnPtr>& getGates() const;

    void addGate(GateUnPtr pGate);

    void addLandmark(PLMUnPtr pLandmark);

    GateID findTheCloestGate(const TopoVec2& gatePos);

    /**
     * @brief 计算两个ExpData是否类似
     * @param another 另一个用于比较的ExpData实例
     * @param selfWeight this的比重, 1:1就是1, 1:4就是0.25, 5:1就是5
     * @return 匹配的详细结果, 包含融合后的ExpData
     */
    MatchResult detailedMatch(const ExpData& another, double selfWeight = 1.0) const;

    /**
     * @brief 计算两个ExpData是否类似
     * @param another 另一个用于比较的ExpData实例
     * @param selfWeight this的比重, 1:1就是1, 1:4就是0.25, 5:1就是5
     * @return 匹配的结果, 只包含一个概率评分
     */
    double quickMatch(const ExpData& another, double selfWeight) const;

    size_t nGates() const;
};

}


#endif //TMAPPING_EXPDATA_H
