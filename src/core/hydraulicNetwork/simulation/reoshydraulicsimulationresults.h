/***************************************************************************
  reoshydraulicsimulationresults.h - ReosHydraulicSimulationResults

 ---------------------
 begin                : 1.4.2022
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
#ifndef REOSHYDRAULICSIMULATIONRESULTS_H
#define REOSHYDRAULICSIMULATIONRESULTS_H

#include <QObject>
#include <QVector>

#include "reoscore.h"
#include "reosmeshdatasetsource.h"

class ReosHydraulicSimulation;
class ReosHydrograph;
class ReosMesh;
class ReosSpatialPosition;

class REOSCORE_EXPORT ReosHydraulicSimulationResults : public ReosMeshDatasetSource
{
    Q_OBJECT
  public:
    enum class DatasetType
    {
      None,
      WaterLevel,
      WaterDepth,
      Velocity,
    };

    ReosHydraulicSimulationResults( const ReosHydraulicSimulation *simulation, QObject *parent = nullptr );

    int groupCount() const override;
    QString groupName( int groupIndex ) const override;
    bool groupIsScalar( int groupIndex ) const override;

    QString groupId( DatasetType type ) const;
    QString groupId( int groupIndex ) const;
    DatasetType datasetType( int groupIndex ) const;
    int groupIndex( DatasetType type ) const;

    virtual QDateTime runDateTime() const = 0;
    virtual int datasetIndex( int groupIndex, const QDateTime &time ) const = 0;

    //! Returns reuslt hydrograph related to the ids of boundaries
    virtual QMap<QString, ReosHydrograph *> outputHydrographs() const = 0;

    double interpolateResultOnMesh( ReosMesh *mesh, const ReosSpatialPosition &position, const QDateTime &time, DatasetType dataType );

    virtual QString unitString( DatasetType dataType ) const = 0;

    QVector<double> resultValues( DatasetType datasetType, int index ) const;

  protected:
    void registerGroups( const QList<DatasetType> &types );

  private:
    QList<DatasetType> mGroupIndexToType;
};

#endif // REOSHYDRAULICSIMULATIONRESULTS_H
