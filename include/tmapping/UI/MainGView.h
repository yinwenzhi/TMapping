//
// Created by stumbo on 2019/12/20.
//

#ifndef TMAPPING_INCLUDE_TMAPPING_UI_MAINGVIEW_H
#define TMAPPING_INCLUDE_TMAPPING_UI_MAINGVIEW_H

#include <QGraphicsView>
#include <QWidget>
#include <QGraphicsItem>
#include <set>

#include "tmapping/StructedMap.h"
#include "QNode.h"

namespace tmap
{

class MainGView : public QGraphicsView
{
    Q_OBJECT
    QGraphicsScene mScene4FakeMap;
    std::set<QNodePtr> mNodesInFakeMap;

    QNodePtr mTheDrawingCorridor;

    bool mEnableFakeNodesMoving = true;
    bool mEnableNodeRestriction = false;
    bool mIsDrawingEdge = false;

protected:
    void wheelEvent(QWheelEvent * event) override ;
    void mousePressEvent(QMouseEvent * event) override ;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override ;

public:
    explicit MainGView(QWidget *parent = nullptr);

    void addNode2FakeMapFromExpData(const ExpDataPtr& usedExpData);

    void restrictQNode(QNode* qNode);

    void saveFakeMap(const std::string& mapName);

    void loadMap(const std::string& fileName);

    ~MainGView() override;

public Q_SLOTS:
    void SLOT_EnableMoving4FakeNodes(bool enableMove);
    void SLOT_EnableGridRestriction(bool enableRes);
    void SLOT_StartDrawingEdge(bool enableDrawing);
    void SLOT_RemoveSelectedNodes();
};
}


#endif //TMAPPING_INCLUDE_TMAPPING_UI_MAINGVIEW_H
