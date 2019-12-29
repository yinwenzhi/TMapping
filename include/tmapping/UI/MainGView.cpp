//
// Created by stumbo on 2019/12/20.
//

#include <iostream>

#include <QWheelEvent>

#include "MainGView.h"
#include "tmapping/MergedExp.h"
#include "tmapping/Exp.h"
#include "UITools.h"

using namespace std;

tmap::QNode::QNode(const ExpDataPtr& relatedExpData)
{
    static std::size_t STATIC_QNodeSerial = 0;
    serial = STATIC_QNodeSerial;
    auto exp = make_shared<Exp>(relatedExpData, 0);
    auto mergedExp = MergedExp::singleMergedFromExp(std::move(exp));
    relatedMergedExp = std::move(mergedExp);
    links.assign(relatedExpData->nGates(), Link{});
}

tmap::QNode::~QNode() {
    if (scene()) {
        scene()->removeItem(this);
    }
}

QRectF tmap::QNode::boundingRect() const
{
    if (mBoundingRect.isEmpty()) {
        auto bRect = relatedMergedExp->getMergedExpData()->getOutBounding(0.5);
        mBoundingRect.setTopLeft(UIT::TopoVec2QPt({bRect[2], bRect[0]}));
        mBoundingRect.setBottomRight(UIT::TopoVec2QPt({bRect[3], bRect[1]}));
    }
    return mBoundingRect;
}

void
tmap::QNode::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    const auto& centerPoint = boundingRect().center();
    QPointF halfDia{UIT::QMeter(0.3), UIT::QMeter(0.3)};
    QRectF middleHalfSq{centerPoint + halfDia, centerPoint - halfDia};

    auto relatedExpData = relatedMergedExp->getMergedExpData();
    switch (relatedExpData->type()) {

        case ExpDataType::Intersection:
            painter->setBrush(Qt::yellow);
            painter->drawEllipse(middleHalfSq);
            for (const auto& gate : relatedExpData->getGates()) {
                UIT::drawGate(painter, gate.get(), true);
                auto oldPen = painter->pen();
                painter->setPen(QPen(Qt::darkGray, 4, Qt::DashDotLine, Qt::FlatCap));
                painter->drawLine(centerPoint, UIT::TopoVec2QPt(gate->getPos()));
                painter->setPen(oldPen);
            }
            break;
        case ExpDataType::Corridor: {
            auto corr = dynamic_cast<Corridor*>(relatedExpData.get());
            auto pA = UIT::TopoVec2QPt(corr->getEndPointA());
            auto pB = UIT::TopoVec2QPt(corr->getEndPointB());
            QPen pen{Qt::darkGreen, 10};
            pen.setCapStyle(Qt::RoundCap);
            painter->setPen(pen);
            painter->drawLine(pA, pB);
            break;
        }
        case ExpDataType::Stair:
            cerr << FILE_AND_LINE << " unimplemented exp type!" << endl;
            break;
        case ExpDataType::BigRoom:
            cerr << FILE_AND_LINE << " unimplemented exp type!" << endl;
            break;
        case ExpDataType::SmallRoom:
            auto bRect = relatedExpData->getOutBounding(0.);
            auto tl = UIT::TopoVec2QPt({bRect[2], bRect[0]});
            auto br = UIT::TopoVec2QPt({bRect[3], bRect[1]});
            painter->drawRect(QRectF{tl,br});
            painter->setBrush(Qt::yellow);
            painter->drawRect(middleHalfSq);
            for (const auto& gate : relatedExpData->getGates()) {
                UIT::drawGate(painter, gate.get(), true);
//                painter->drawLine(centerPoint, UIT::TopoVec2QPt(gate->getPos()));
            }
            break;
    }
}

void tmap::QNode::changeSize()
{
    mBoundingRect.setWidth(0.);
    prepareGeometryChange();
    update();
}

tmap::QNodePtr tmap::QNode::makeOne(const tmap::ExpDataPtr& relatedExpData)
{
    return QNodePtr(new QNode(relatedExpData->clone()));
}

tmap::MainGView::MainGView(QWidget* parent) : QGraphicsView(parent)
{
    setScene(&mScene4FakeMap);
    /// 设置默认的缩放系数, 缩小至0.75倍
    setMatrix(transform().scale(0.75, 0.75).toAffine());
}

static constexpr double SCALE_MAX = 10.0;
static constexpr double SCALE_TIME = 1.2;

void tmap::MainGView::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        auto curTrans = transform();
        auto curScale = curTrans.m11();
        if (event->delta() > 0 && curScale <= SCALE_MAX) {
            curTrans.scale(SCALE_TIME, SCALE_TIME);
        }
        else if (event->delta() < 0 && curScale >= 1 / SCALE_MAX) {
            curTrans.scale(1 / SCALE_TIME, 1 / SCALE_TIME);
        }
        setMatrix(curTrans.toAffine());
    } else {
        QGraphicsView::wheelEvent(event);
    }
}

void tmap::MainGView::addNode2FakeMap(const tmap::ExpDataPtr& usedExpData)
{
    auto qNode = QNode::makeOne(usedExpData);
    mNodesInFakeMap.insert(qNode);
    mScene4FakeMap.addItem(qNode.get());
    qNode->setFlag(QGraphicsItem::ItemIsMovable, mEnableFakeNodesMoving);
}

void tmap::MainGView::SLOT_EnableMoving4FakeNodes(bool enableMove)
{
    mEnableFakeNodesMoving = enableMove;
    for (auto& item : mScene4FakeMap.items()) {
        if (auto qNode = dynamic_cast<QNode*>(item)) {
            if (qNode->relatedMergedExp->getMergedExpData()->type() != ExpDataType::Corridor) {
                qNode->setFlag(QGraphicsItem::ItemIsMovable, enableMove);
            }
        }
    }
}

void tmap::MainGView::SLOT_EnableGridRestriction(bool enableRes)
{
    mEnableNodeRestriction = enableRes;
}

void tmap::MainGView::restrictQNode(tmap::QNode* qNode)
{
    if (qNode &&
        qNode->scene() == &mScene4FakeMap &&
        qNode->relatedMergedExp->getMergedExpData()->type() != ExpDataType::Corridor) {
        auto pos = qNode->pos();
        auto roundedP = UIT::QPt2TopoVec(pos).round2();
        qNode->setPos(UIT::TopoVec2QPt(roundedP));
    }
}

void tmap::MainGView::SLOT_StartDrawingEdge(bool enableDrawing)
{
    mIsDrawingEdge = enableDrawing;

    if (enableDrawing) {
        setCursor(Qt::CrossCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }
}

void tmap::MainGView::mousePressEvent(QMouseEvent* event)
{
    QGraphicsView::mousePressEvent(event);
    const auto & clickPosInView = event->pos();
    const auto & clickPosInScene = mapToScene(clickPosInView);
    auto item = scene()->itemAt(clickPosInScene);

    if (item) {
        if (auto clickedQNode = dynamic_cast<QNode*>(item)) {
            if (mIsDrawingEdge) { // TODO 检查外部的Mode是否准确
                /// 被点中的QNode的坐标系中点击的位置
                const auto& clickPosInItem = clickedQNode->mapFromScene(clickPosInScene);
                /// 被点中的QNode对应的ExpData数据
                const auto& clickedExpData = clickedQNode->relatedMergedExp->getMergedExpData();
                /// 查找被点中的GateID
                int clickedGateID = clickedExpData->
                        findGateAtPos(UIT::QPt2TopoVec(clickPosInItem),UIT::pix2meter(15));
                if (clickedGateID >= 0) {
                    auto& toNewQNode = clickedQNode->links[clickedGateID].to;
                    if (toNewQNode.expired()) {
                        /// 这个Gate还没有连接任何的地方, 开始连接

                        /// 被点中的Gate
                        const auto& clickedGate = clickedExpData->getGates()[clickedGateID];
                        /// 新Gate按照这个Gate制作
                        auto newGate = clickedGate->clone();
                        newGate->setPos({0,0});
                        newGate->setNormalVec(clickedGate->getNormalVec().rotate(180));
                        /// 开始制作新的Corridor
                        auto newCorridor = make_shared<Corridor>();
                        newCorridor->addGate(std::move(newGate));
                        newCorridor->setEndGateA(0);
                        /// 制作对应的QNode
                        auto newQNode = QNode::makeOne(newCorridor);
                        /// 添加连接关系
                        clickedQNode->links[clickedGateID].to = newQNode;
                        clickedQNode->links[clickedGateID].at = 0;
                        MapNode::Link l;
                        l.to = clickedQNode->shared_from_this();
                        l.at = clickedGateID;
                        newQNode->links.push_back(std::move(l));
                        /// 添加刚刚制作好的QNode
                        mScene4FakeMap.addItem(newQNode.get());
                        newQNode->setPos(clickedQNode->mapToScene(
                                UIT::TopoVec2QPt(clickedGate->getPos())));
                        mNodesInFakeMap.insert(newQNode);
                        mTheDrawingCorridor = std::move(newQNode);
                        mTheDrawingCorridor->setZValue(-1);
                    }
                }
            }
        }
    }
}

void tmap::MainGView::mouseMoveEvent(QMouseEvent* event)
{
    QGraphicsView::mouseMoveEvent(event);
    if (mIsDrawingEdge && mTheDrawingCorridor) {
        const auto& clickPosInScene = mapToScene(event->pos());
        const auto& clickPosInQNode = mTheDrawingCorridor->mapFromScene(clickPosInScene);
        dynamic_cast<Corridor*>(mTheDrawingCorridor->relatedMergedExp->getMergedExpData().get())
                ->setEndPointB(UIT::QPt2TopoVec(clickPosInQNode));
        mTheDrawingCorridor->changeSize();
    }
}

void tmap::MainGView::mouseReleaseEvent(QMouseEvent* event)
{
    QGraphicsView::mouseReleaseEvent(event);
    const auto & clickPosInView = event->pos();
    const auto & clickPosInScene = mapToScene(clickPosInView);
    auto item = scene()->itemAt(clickPosInScene);

    if (item) {
        if (auto clickedQNode = dynamic_cast<QNode*>(item)) {
            if (mEnableNodeRestriction) {
                if (clickedQNode->relatedMergedExp->getMergedExpData()->type() != ExpDataType::Corridor) {
                    restrictQNode(clickedQNode);
                }
            }
            else if (mIsDrawingEdge && mTheDrawingCorridor) {
                if (clickedQNode != mTheDrawingCorridor.get()) {
                    /// 被点中的QNode的坐标系中点击的位置
                    const auto& clickPosInItem = clickedQNode->mapFromScene(clickPosInScene);
                    /// 被点中的QNode对应的ExpData数据
                    const auto& clickedExpData = clickedQNode->relatedMergedExp->getMergedExpData();
                    /// 查找被点中的GateID
                    int clickedGateID = clickedExpData->
                            findGateAtPos(UIT::QPt2TopoVec(clickPosInItem),UIT::pix2meter(15));
                    if (clickedGateID >= 0) {
                        auto& toNewQNode = clickedQNode->links[clickedGateID].to;
                        if (toNewQNode.expired()) {
                            /// 这个Gate还没有连接任何的地方, 那就连接到这个Gate

                            /// 被点中的Gate
                            const auto& clickedGate = clickedExpData->getGates()[clickedGateID];
                            /// 新Gate按照这个Gate制作
                            auto newGate = clickedGate->clone();
                            auto gatePosInClickedNode = UIT::TopoVec2QPt(clickedGate->getPos());
                            auto gatePosInScene = clickedQNode->mapToScene(gatePosInClickedNode);
                            auto gatePosInThis = mTheDrawingCorridor->mapFromScene(gatePosInScene);
                            newGate->setPos(UIT::QPt2TopoVec(gatePosInThis));
                            newGate->setNormalVec(clickedGate->getNormalVec().rotate(180));
                            /// 添加新的Gate
                            auto theDrawingCorridor = dynamic_cast<Corridor*>
                            (mTheDrawingCorridor->relatedMergedExp->getMergedExpData().get());
                            theDrawingCorridor->addGate(std::move(newGate));
                            theDrawingCorridor->setEndGateB(1);
                            if (theDrawingCorridor->getGates().size() != 2) {
                                cerr << FILE_AND_LINE << " You add a gate before corridor complete!"
                                     << endl;
                            }
                            /// 添加连接关系
                            clickedQNode->links[clickedGateID].to = mTheDrawingCorridor;
                            clickedQNode->links[clickedGateID].at = 1;
                            MapNode::Link l;
                            l.to = clickedQNode->shared_from_this();
                            l.at = clickedGateID;
                            mTheDrawingCorridor->links.push_back(std::move(l));
                            /// 完成制作, 确定图形
                            mTheDrawingCorridor->changeSize();
                            mTheDrawingCorridor.reset();
                            return;
                        }
                    }
                }
                const auto& clickPosInQNode = mTheDrawingCorridor->mapFromScene(clickPosInScene);
                dynamic_cast<Corridor*>(mTheDrawingCorridor->relatedMergedExp->getMergedExpData().get())
                        ->setEndPointB(UIT::QPt2TopoVec(clickPosInQNode));
                mTheDrawingCorridor->changeSize();
                mTheDrawingCorridor.reset();
            }
        }
    }

}

tmap::MainGView::~MainGView()
{
    /// QNode是智能指针管理的, 不要交给scene校徽
    mNodesInFakeMap.clear();
}
