/***************************************************************************
  reospolylinesstructure_p.h - ReosPolylinesStructure_p

 ---------------------
 begin                : 10.1.2022
 copyright            : (C) 2022 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef REOSPOLYLINESSTRUCTURE_P_H
#define REOSPOLYLINESSTRUCTURE_P_H

#include <qgsvectorlayer.h>
#include <qgscoordinatetransform.h>

#include "reospolylinesstructure.h"

class ReosPolylineStructureVectorLayerUndoCommandRemoveLine;
class ReosPolylineStructureVectorLayerUndoCommandAddLine;

typedef QgsFeatureId SegmentId;

#define InvalidSegment std::numeric_limits<SegmentId>::max()


class ReosStructureVertexHandler_p : public ReosGeometryStructureVertex
{
  public:

    ReosStructureVertexHandler_p( QgsVectorLayer *source, QgsFeatureId fid, int pos );

    QPointF position( const QgsCoordinateTransform &transform = QgsCoordinateTransform() );

    SegmentId firstLinkedLine( int posInLine ) const;

    void attachLine( SegmentId fid, int pos );
    void detachLine( SegmentId fid );
    QSet<SegmentId> attachedLines() const;
    bool hasLineAttached() const;
    bool oneOtherLine( SegmentId id, SegmentId *otherLine ) const;
    int posInLine( SegmentId id ) const
    {
      int index = mLinkedSegments.indexOf( id );
      if ( index >= 0 )
        return mLinkedSegments.at( index ).pos;

      return -1;
    }

    void move( const QgsPointXY &newPosition );

  private:

    struct PositionInFeature
    {
      PositionInFeature() = default;

      //! Contructs a generic position, that is, this instance is equal to all position related to line with \a id
      PositionInFeature( SegmentId id ): fid( id ) {}

      //! Contructs a position on line with \a id at position \a p on the line
      PositionInFeature( SegmentId id, int p ): fid( id ), pos( p ) {}

      bool operator==( const PositionInFeature &other ) const
      {
        if ( other.pos == -1 )
        {
          return fid == other.fid;
        }
        return fid == other.fid && pos == other.pos;
      }

      SegmentId fid;
      int pos = -1;
    };

    QList<PositionInFeature> mLinkedSegments;

    QgsVectorLayer *mSource = nullptr;
};

typedef ReosStructureVertexHandler_p *VertexP;
typedef std::shared_ptr<ReosStructureVertexHandler_p> VertexS;
typedef std::weak_ptr<ReosStructureVertexHandler_p> VertexW;
typedef std::array<VertexS, 2> Segment;

class ReosPolylineStructureVectorLayer: public ReosPolylinesStructure
{
    Q_OBJECT
  public:

    ReosPolylineStructureVectorLayer( const QString &wktCrs );
    ReosPolylineStructureVectorLayer( const QPolygonF &boundary, const QString &wktCrs );

    void addPolylines( const QPolygonF &polyline, const QString &sourceCrs = QString( ) ) override;
    QPolygonF polyline( const QString &destinationCrs = QString(), const QString &id = QString() ) const override;

    QPolygonF boundary( const QString &destinationCrs ) const override;

    QgsVectorLayer *data() override {return mVectorLayer.get();}

    void removeAll() override;
    void translate( const QPointF &translation, const QString &crs, const QString &id = QString() ) override {};

    bool vertexCanBeMoved( ReosGeometryStructureVertex *vertex, const ReosSpatialPosition &newPosition ) const override;
    void moveVertex( ReosGeometryStructureVertex *vertex, const ReosSpatialPosition &newPosition ) override;
    void insertVertex( const ReosSpatialPosition &point, qint64 lineId ) override;
    void removeVertex( ReosGeometryStructureVertex *vertex ) override;

    ReosMapExtent extent( const QString &destinationCrs ) const override;
    ReosGeometryStructureVertex *searchForVertex( const ReosMapExtent &zone ) const override;
    bool searchForLine( const ReosMapExtent &zone, qint64 &id ) const override;
    QPointF vertexPosition( ReosGeometryStructureVertex *vertex, const QString &crs ) const override;
    QList<QPointF> neighborsPositions( ReosGeometryStructureVertex *vertex, const QString &crs ) const override;
    Data structuredLinesData( const QString &destinationCrs = QString() ) const override;

    QUndoStack *undoStack() const override;

    void setTolerance( double tolerance, const QString &wktCrs = QString() );

  private:
    typedef QgsFeatureId SegmentId;
    typedef ReosStructureVertexHandler_p *VertexP;
    typedef std::shared_ptr<ReosStructureVertexHandler_p> VertexH;
    typedef std::array<VertexH, 2> Segment;

    ReosPolylineStructureVectorLayer() = default;

    // Data member defining the structure ********
    std::unique_ptr<QgsVectorLayer> mVectorLayer;
    QMap<QgsFeatureId, Segment> mSegments;
    QList<VertexP> mBoundariesVertex;
    // *******************************************
    double mTolerance = 0.01;

    VertexS createVertex( QgsFeatureId id, int positionInFeature );
    const QgsCoordinateTransform toLayerTransform( const QString &crs ) const;
    const QgsCoordinateTransform toDestinationTransform( const QString &destinationCrs ) const;

    QgsFeatureIterator closeLines( const ReosMapExtent &zone, QgsRectangle &rect ) const;
    QgsFeatureIterator closeLines( const QgsRectangle &rectSource, QgsRectangle &rectLayer, const QgsCoordinateTransform &transform ) const;
    VertexS searchForVertexPrivate( QgsFeatureIterator &it, const QgsRectangle &rect ) const;
    QList<ReosStructureVertexHandler_p *> neighorsVertices( ReosGeometryStructureVertex *vertex,  QList<SegmentId> &fids ) const;

    QgsPointXY toLayerCoordinates( const QPointF &position, const QgsCoordinateTransform &transform ) const;
    QgsPointXY toLayerCoordinates( const ReosSpatialPosition &position ) const;
    Segment idToSegment( SegmentId id ) const;
    VertexS idToVertex( SegmentId id, int pos );
    bool idToOneLinkedSegment( SegmentId id, int pos, SegmentId *linkedSeg );

    VertexP oppositeVertexPointer( VertexP other, SegmentId sid ) const;
    VertexS oppositeVertex( VertexP other, SegmentId sid ) const;

    //! Returns the boundary lines on each side of \a vertex, order consistent with vertices order
    QPair<SegmentId, SegmentId> boundarieLines( VertexP vertex );

    void purge();

    friend class ReosPolylineStructureVectorLayerUndoCommandRemoveLine;
    friend class ReosPolylineStructureVectorLayerUndoCommandAddLine;
};



class ReosPolylineStructureVectorLayerUndoCommandRemoveLine : public QUndoCommand
{
  public:
    ReosPolylineStructureVectorLayerUndoCommandRemoveLine( QgsFeatureId id, bool onBoundary, ReosPolylineStructureVectorLayer *structure );
    void redo() override;
    void undo() override;

  private:
    QgsFeatureId mId;
    ReosPolylineStructureVectorLayer *mStructure = nullptr;
    Segment mSeg;
    int mBoundaryPos = -1 ; //if the line is on boundary, position of first vertex

};

class ReosPolylineStructureVectorLayerUndoCommandAddLine : public QUndoCommand
{
  public:
    ReosPolylineStructureVectorLayerUndoCommandAddLine(
      QgsFeatureId idLineToAdd,
      VertexS vert0,
      VertexS vert1,
      bool onBoundary,
      ReosPolylineStructureVectorLayer *structure );

    void redo() override;
    void undo() override;

  private:
    QgsFeatureId mIdToAdd;
    SegmentId mExistingLine0 = InvalidSegment;
    int mPosInExistingLine0 = -1;
    SegmentId mExistingLine1 = InvalidSegment;
    int mPosInExistingLine1 = -1;
    VertexW mVert0 ;
    VertexW mVert1 ;
    bool mOnBoundary = false;
    ReosPolylineStructureVectorLayer *mStructure = nullptr;
};




#endif // REOSPOLYLINESSTRUCTURE_P_H
