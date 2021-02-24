/***************************************************************************
                      reoswatershed.h
                     --------------------------------------
Date                 : 10-2020
Copyright            : (C) 2020 by Vincent Cloarec
email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef REOSWATERSHED_H
#define REOSWATERSHED_H

#include "reoscore.h"
#include "reosmapextent.h"
#include "reosrastercompressed.h"
#include "reosrasterwatershed.h"
#include "reosexception.h"
#include "memory"

#include "reosgeometryutils.h"
#include "reosrasterwatershed.h"
#include "reosencodedelement.h"
#include "reosparameter.h"
#include "reosconcentrationtimecalculation.h"

class ReosGisEngine;
class ReosWatershedRunoffModels;

class REOSCORE_EXPORT ReosWatershed: public QObject
{
    Q_OBJECT
  public:
    enum Type
    {
      None,
      Automatic,
      Manual,
      Residual
    };

    ReosWatershed();

    ReosWatershed( const QPolygonF &delineating,
                   const QPointF &outletPoint,
                   Type type );

    ReosWatershed( const QPolygonF &delineating,
                   const QPointF &outletPoint,
                   Type type,
                   const QPolygonF &downstreamLine,
                   const QPolygonF &streamPath );

    ReosWatershed( const QPolygonF &delineating,
                   const QPointF &outletPoint,
                   Type type,
                   const QPolygonF &downstreamLine,
                   const QPolygonF &streamPath,
                   const ReosRasterWatershed::Directions &direction,
                   const ReosRasterExtent &directionExent,
                   const QString &refLayerId );

    Type type() const {return mType;}

    //! Returns the name of the watershed
    ReosParameterString *name() const;

    //! Sets the name of the watershed
    void setName( const QString &name );

    //! Returns the extent of the watershed
    ReosMapExtent extent() const;

    //! Returns the delineating of the watershed
    QPolygonF delineating() const;

    //! Returns the outlet point of the watershed
    QPointF outletPoint() const;

    //! Returns whether the watrshed or its parent contoins direction data
    bool hasDirectiondata( const QString &layerId ) const;

    //! Returns the directions raster associated with this watershed, returned raster is invalid if there is none
    ReosRasterWatershed::Directions directions( const QString &layerId ) const;

    //! Returns the extent of the raster direction, \see directions()
    ReosRasterExtent directionExtent( const QString &layerId ) const;

    //! Removes direction data present in the watershed or in its children
    void removeDirectionData();

    /**
     * Returns whether the watershed includes the \a point
     *
     * \note if the point is exactly on a segment of the delineating polygon, this point is considered outside
     */
    bool contain( const QPointF &point ) const;

    /**
     * Returns how the polygon or polyline \a line is contained in the watershed
     *
     * \note if a point of the line is exactly on a segment of the delineating polygon, this point is considered outside
     */
    ReosInclusionType contain( const QPolygonF &line ) const;


    //! Returns how this watershed is included by \a other
    ReosInclusionType isContainedBy( const ReosWatershed &other ) const;

    //! Returns the count of upstream watersheds
    int upstreamWatershedCount() const;

    //! Returns the count of watersheds that are directly upstream
    int directUpstreamWatershedCount() const;

    //! Returns the ith direct upstream watershed, return nullptr if there is not
    ReosWatershed *directUpstreamWatershed( int i ) const;

    /**
     * Adds a \a upstream watershed (take ownership) and returns a pointer to this new watershed
     * If \a adaptUpstreamDelineating is true, the upstram watershed delineating will be adapted to with its directly
     * downstream watershed or its sibling.
     */
    ReosWatershed *addUpstreamWatershed( ReosWatershed *upstreamWatershed, bool adjustIfNeeded );

    /**
     * Removes (if present) the watershed from the direct upstream watershed at position \a i, but do not delete it, returns a pointer to it.
     * Sub watershed are removed from the extracted watershed and are put in \a this watershed
     */
    ReosWatershed *extractOnlyDirectUpstreamWatershed( int i );

    /**
     * Removes (if present) the watershed from the direct upstream watershed at position \a i, but do not delete it, returns a pointer to it.
     * Sub watershed are maintained in the extracted watershed
     */
    ReosWatershed *extractCompleteDirectUpstreamWatershed( int i );

    //! Returns the smallest sub watershed that is downstream the line, if the line is partially included by any watershed, ok is false and return nullptr
    //! If there is no watershed downstrean, return nullptr
    ReosWatershed *upstreamWatershed( const QPolygonF &line, bool &ok ) const;

    //! Returns the smallest upstream watershed that contains the point
    ReosWatershed *upstreamWatershed( const QPointF &point, bool excludeResidual = false );

    //! Returns, if exists, a pointer to the direct downstream watershed, if not returns nullptr
    ReosWatershed *downstreamWatershed() const;

    //! Returns the position in the downstream watershed
    int positionInDownstreamWatershed() const;

    //! Returns a list of all upstream watershed
    QList<ReosWatershed *> allUpstreamWatershed() const;

    //! Cuts the delineating of this delineating to fit in the \a other watershed delineating (cut all that is outside the other and intersect with sub watershed)
    void fitIn( const ReosWatershed &other );

    //! Adjusts the delineating of this watershed to not intersect with the \a other watershed delineating (cut all that is inside the other)
    void adjust( const ReosWatershed &other );

    //! Extents the delineating of this watershed to fit with the delineating of \a other
    void extentTo( const ReosWatershed &other );

    //! Returns the downstream line of the watersheds
    QPolygonF downstreamLine() const;

    //! Returns the stream path line of the watershed
    QPolygonF streamPath() const;

    //! Sets the stream path line of the watershed
    void setStreamPath( const QPolygonF &streamPath );

    //! Returns the residual watershed if exists, if not returns nullptr
    ReosWatershed *residualWatershed() const;

    //! Returns the longitudinale profile of the watershed
    QPolygonF profile() const;

    //! Sets the longitudinale profile of the watershed
    void setProfile( const QPolygonF &profile );

    void setGeographicalContext( ReosGisEngine *gisEngine );

    ReosParameterArea *area() const;
    ReosParameterSlope *slope() const;
    ReosParameterDouble *drop() const;
    ReosParameterDouble *longestPath() const;

    ReosParameterDuration *concentrationTime() const;
    ReosConcentrationTimeCalculation concentrationTimeCalculation() const;
    void setConcentrationTimeCalculation( const ReosConcentrationTimeCalculation &concentrationTimeCalculation );

    ReosEncodedElement encode() const;
    static ReosWatershed *decode( const ReosEncodedElement &element );

    bool operator==( const ReosWatershed &other ) const;

    ReosWatershedRunoffModels *runoffModels() const;

  signals:
    void changed();

  public slots:
    void calculateArea();

  private slots:
    void calculateSlope();
    void calculateLongerPath();
    void calculateDrop();
    void calculateConcentrationTime();

  private:
    Type mType = None;

    ReosMapExtent mExtent;
    QPolygonF mDelineating;
    QPointF mOutletPoint;
    QPolygonF mDownstreamLine;
    QPolygonF mStreamPath;
    QPolygonF mProfile;
    ReosGisEngine *mGisEngine = nullptr;

    ReosParameterString *mName;

    ReosParameterArea *mArea = nullptr;
    ReosParameterSlope *mSlope = nullptr;
    ReosParameterDouble *mDrop = nullptr;
    ReosParameterDouble *mLongestStreamPath = nullptr;

    ReosConcentrationTimeCalculation mConcentrationTimeCalculation;
    ReosParameterDuration *mConcentrationTimeValue = nullptr;

    ReosGisEngine *geographicalContext() const;

    struct DirectionData
    {
      ReosRasterByteCompressed directionRaster;
      ReosRasterExtent directionExtent;
    };

    std::map<QString, DirectionData> mDirectionData;

    std::vector<std::unique_ptr<ReosWatershed>> mUpstreamWatersheds;
    ReosWatershed *mDownstreamWatershed = nullptr;

    ReosWatershedRunoffModels *mRunoffModels;

    void init();
    void connectParameters();
    void updateResidual();
};

#endif // REOSWATERSHED_H
