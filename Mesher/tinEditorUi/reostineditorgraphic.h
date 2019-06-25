/***************************************************************************
                      reostineditorgraphic.cpp
                     --------------------------------------
Date                 : 01-04-2019
Copyright            : (C) 2018 by Vincent Cloarec
email                : vcloarec at gmail dot com   /  projetreos at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef HDTINEDITORGRAPHIC_H
#define HDTINEDITORGRAPHIC_H

#include <QObject>
#include <QAction>
#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPixmap>

#include <qgsmapcanvas.h>
#include "qgsmeshlayer.h"
#include <qgsmaptool.h>
#include <qgsmapmouseevent.h>

#include "../../GIS/hdgismanager.h"
#include "../../Reos/reosmodule.h"

#include "../provider/meshdataprovider.h"
#include "reosmapmeshitem.h"
#include "hdtineditoruidialog.h"
#include "hdtineditornewdialog.h"


class ReosTinEditorUi;
class ReosTinUndoCommandNewVertex;
class ReosTinUndoCommandNewSegmentWithNewSecondVertex;
class ReosTinUndoCommandNewSegmentWithExistingSecondVertex;
class ReosTinUndoCommandRemoveVertex;
class ReosTinUndoCommandRemoveHardLine;


struct ReosMeshItemVertexAndNeighbours
{
    ReosMeshItemVertex *vertex=nullptr;
    QList<ReosMeshItemVertexAndNeighbours> neighbours;
};

struct PointAndNeighbours
{
    PointAndNeighbours(){}
    PointAndNeighbours(const ReosMeshItemVertexAndNeighbours &graphicsItems)
    {
        double x=graphicsItems.vertex->realWorldVertex()->x();
        double y=graphicsItems.vertex->realWorldVertex()->y();
        point=QPointF(x,y);
        for (auto nv:graphicsItems.neighbours)
        {
            neighbours.append(PointAndNeighbours(nv));
        }
    }

    QPointF point;
    QList<PointAndNeighbours> neighbours;
};

class ReosTinMapToolHardLineSegement:public ReosMapTool
{
    Q_OBJECT
public:
    ReosTinMapToolHardLineSegement(ReosMap *map, ReosTinEditorUi* uiEditor):ReosMapTool(map),
        mUiEditor(uiEditor),
        rubberBand(new QgsRubberBand(map->getMapCanvas(),QgsWkbTypes::LineGeometry))
    {
        rubberBand->setLineStyle(Qt::DashDotLine);
        rubberBand->setColor(Qt::red);
        rubberBand->setWidth(3);
    }

    bool isInProgress() const override
    {
         return (firstVertex!=nullptr);
    }

public slots:
    void reset() override
    {
        rubberBand->reset();
        firstVertex=nullptr;
        firstPoint=QPointF();
    }

protected:
    void canvasPressEvent(QgsMapMouseEvent *e) override;
    void canvasMoveEvent(QgsMapMouseEvent *e) override
    {
        rubberBand->movePoint(e->mapPoint());
    }

private:
    ReosMeshItemVertex *firstVertex=nullptr;
    QPointF firstPoint;
    ReosTinEditorUi *mUiEditor;
    QgsRubberBand *rubberBand;
};



class ReosTinEditorUi : public ReosModule
{
    Q_OBJECT
public:
    explicit ReosTinEditorUi(HdManagerSIG *gismanager,QObject *parent = nullptr);

    ReosMapMeshEditorItemDomain *domain() const {return mDomain;}
    void setMeshLayer(QgsMeshLayer *meshLayer);

    ReosMeshItemVertex *mapVertex(const QPointF &mapPoint) const;
    VertexPointer realWorldVertex(const QPointF &mapPoint) const;


    void doCommand(ReosTinUndoCommandNewVertex *command);
    void undoCommand(ReosTinUndoCommandNewVertex *command);
    void doCommand(ReosTinUndoCommandNewSegmentWithNewSecondVertex *command);
    void undoCommand(ReosTinUndoCommandNewSegmentWithNewSecondVertex *command);
    void doCommand(ReosTinUndoCommandNewSegmentWithExistingSecondVertex *command);
    void undoCommand(ReosTinUndoCommandNewSegmentWithExistingSecondVertex *command);

    void doCommand(ReosTinUndoCommandRemoveVertex *command);
    void undoCommand(ReosTinUndoCommandRemoveVertex *command);

    void doCommand(ReosTinUndoCommandRemoveHardLine *command);
    void undoCommand(ReosTinUndoCommandRemoveHardLine *command){}

public slots:
    void startNewVertex();
    void newVertex(const QPointF &mapPoint);
    void stopNewVertex();

    void startRemoveVertex();
    void removeVertexFromRect(const QRectF &selectionZone);

    void startNewHardLineSegment();
    void newSegment(ReosMeshItemVertex *firstVertx, const QPointF &secondMapPoint);
    void newSegment(ReosMeshItemVertex *firstVertex, ReosMeshItemVertex *secondVertex);
    void stopNewHardLineSegment();

    void startRemoveSegment();
    void removeSegmentFromRect(const QRectF &selectionZone);

    void populateDomain();

private slots :

    void currentLayerChanged(QgsMapLayer *layer);
    void layerHasToBeUpdated(QgsMapLayer *layer);
    void layerHasToBeRemoved(QgsMapLayer *layer);


private:

    QWidget *getWidget() const override {return uiDialog;}

    //////////////////////////////////////////////////////////
    /// \brief zValue
    /// \param p
    /// \return return the value calculated with the map position p
    ///
    double zValue(const QPointF &p);


    void updateMesh();
    void enableEditAction(bool enable);
    void newTinLayer();

    void widgetClosed();


    ReosMeshItemVertex* addMapVertex(VertexPointer vert);
    ReosMeshItemVertex* addMapVertex(const QPointF &mapPoint,VertexPointer vert);

    void setLevelMode();
    void setNoneMode();

    QPointF mapCoordinates(const QPointF &meshCoordinate) const;
    QPointF meshCoordinates(const QPointF &mapCordinate) const;

    VertexPointer addRealWorldVertex(const QPointF &mapPoint, double z);
    VertexPointer addRealWorldVertex(const QPointF &mapPoint);
    ReosMeshItemVertex* addMapVertex_2(VertexPointer realWorldVertex);

    void addSegment(VertexPointer v1, VertexPointer v2, QList<PointAndNeighbours> &oldNeigboursStructure);

    void removeVertex(VertexPointer mapVertex);

    ReosMeshItemVertexAndNeighbours saveStructure(ReosMeshItemVertex *vert) const;

    void restoreStructure(VertexPointer realWorldVertex, const PointAndNeighbours &structure) const;

    void updateGraphics(VertexPointer realWorldVertex);

    void updateMeshLayer();



private:
    ReosMapMeshEditorItemDomain *mDomain;
    HdManagerSIG *mGisManager;
    ReosMap *mMap;
    HdTinEditorUiDialog *uiDialog;
    HdTinEditorUiDialog::ZValueMode zValueMode=HdTinEditorUiDialog::none;

    std::unique_ptr<QgsCoordinateTransform> mTransform;

    QgsMeshLayer *mMeshLayer=nullptr;
    TINEditor *mEditor=nullptr;
    QHash<TINEditor*,QUndoStack*> mUndoStacks;

    QAction *actionNewTinLayer;

    QList<QAction*> actionEditList;
    QAction *actionNewVertex;
    ReosMapToolClickPoint *mapToolNewVertex;
    QAction *actionRemoveVertex;
    ReosMapToolSelection *mapToolRemoveVertex;
    QAction *actionNewHardLineSegment;
    ReosTinMapToolHardLineSegement *mapToolHardLineSegment;
    QAction* actionRemoveSegment;
    ReosMapToolSelection *mapToolRemoveSegment;

    QAction *actionTriangulateTIN;

    void newCommand(QUndoCommand *command) override
    {
        //override for disable the sending of this command to the undo stck because, undo not works because of potential CGAL issue
        command->redo();

        if (uiDialog->autoUpdate())
            updateMesh();
    }

};



class ReosTinUndoCommandNewVertex:public QUndoCommand
{

public:
    void undo() override{
        mEditor->undoCommand(this);
    }
    void redo() override{
        mEditor->doCommand(this);
    }

private:
    ReosTinUndoCommandNewVertex(ReosTinEditorUi* editor,QPointF mapPoint, double ZValue):mEditor(editor),mMapPoint(mapPoint),mZValue(ZValue)
    {
        setText(QObject::tr("Nouveau point"));
    }

    ReosTinEditorUi *mEditor;
    QPointF mMapPoint;
    double mZValue;
    friend class ReosTinEditorUi;
};


class ReosTinUndoCommandNewSegmentWithNewSecondVertex:public QUndoCommand
{

public:
    void undo() override{
        mEditor->undoCommand(this);
    }
    void redo() override{
        mEditor->doCommand(this);
    }

private:
    ReosTinUndoCommandNewSegmentWithNewSecondVertex(ReosTinEditorUi* editor,
                                                    QPointF mapPointFirstVertex,
                                                    QPointF mapPointSecondVertex,
                                                    double ZValueSecondVertex):
        mEditor(editor),mMapPointFirst(mapPointFirstVertex),mMapPointSecond(mapPointSecondVertex),mZValueSecond(ZValueSecondVertex)
    {

        setText(QObject::tr("Nouveaux point et segment"));
    }

    ReosTinEditorUi *mEditor;
    QPointF mMapPointFirst;
    QPointF mMapPointSecond;
    double mZValueSecond;
    QList<PointAndNeighbours> mVerticesPositionAndStructureMemory; //store the extremity's position and the position of new intersection vertices but also the old structure of their neighbours

    friend class ReosTinEditorUi;
};

class ReosTinUndoCommandNewSegmentWithExistingSecondVertex:public QUndoCommand
{

public:
    void undo() override{
        mEditor->undoCommand(this);
    }
    void redo() override{
        mEditor->doCommand(this);
    }

private:
    ReosTinUndoCommandNewSegmentWithExistingSecondVertex(ReosTinEditorUi* editor,
                                                    QPointF mapPointFirstVertex,
                                                    QPointF mapPointSecondVertex):
        mEditor(editor),mMapPointFirst(mapPointFirstVertex),mMapPointSecond(mapPointSecondVertex)
    {
        setText(QObject::tr("Nouveau segment"));
    }

    ReosTinEditorUi *mEditor;
    QPointF mMapPointFirst;
    QPointF mMapPointSecond;
    QList<PointAndNeighbours> mVerticesPositionAndStructureMemory;

    friend class ReosTinEditorUi;
};

class ReosTinUndoCommandRemoveVertex : public QUndoCommand
{
public:
    void undo() override{
        mEditor->undoCommand(this);
    }
    void redo() override{
        mEditor->doCommand(this);
    }

private:
    ReosTinUndoCommandRemoveVertex(ReosTinEditorUi* editor,
                                   const QPointF &mapPoint):
        mEditor(editor),mMapPoint(mapPoint) {}

    ReosTinEditorUi *mEditor;
    QPointF mMapPoint;

    friend class ReosTinEditorUi;
};

class ReosTinUndoCommandRemoveHardLine: public QUndoCommand
{
public:
    void undo() override{
        mEditor->undoCommand(this);
    }
    void redo() override{
        mEditor->doCommand(this);
    }

private:
    ReosTinUndoCommandRemoveHardLine(ReosTinEditorUi* editor,const QPointF &mapPoint1,const QPointF &mapPoint2):
        mEditor(editor),mMapPointForVertex1(mapPoint1),mMapPointForVertex2(mapPoint2)
    {}

    ReosTinEditorUi *mEditor;
    QPointF mMapPointForVertex1;
    QPointF mMapPointForVertex2;

    friend class ReosTinEditorUi;

};

#endif // HDTINEDITORGRAPHIC_H