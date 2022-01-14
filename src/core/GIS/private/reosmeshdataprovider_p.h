/***************************************************************************
  reosmeshdataprovider_p.h - ReosMeshDataProvider_p

 ---------------------
 begin                : 13.1.2022
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
#ifndef REOSMESHDATAPROVIDER_P_H
#define REOSMESHDATAPROVIDER_P_H

#include <qgsmeshdataprovider.h>
#include <qgsmeshdataset.h>
#include <qgsprovidersublayerdetails.h>

#include "reosmesh.h"
#include "reosmeshgenerator.h"

class ReosMeshGenerator;
class ReosMeshFrameData;

class ReosMeshDataProvider_p: public QgsMeshDataProvider
{
    Q_OBJECT
  public:
    ReosMeshDataProvider_p(): QgsMeshDataProvider( "mesh", QgsDataProvider::ProviderOptions() ) {}

    void generateMesh( const ReosMeshGenerator &generator );

//***********************
    // QgsMeshDatasetSourceInterface interface
  public:
    bool addDataset( const QString &uri ) {return false;}
    QStringList extraDatasets() const {return QStringList();}
    int datasetGroupCount() const {return 0;};
    int datasetCount( int groupIndex ) const {return 0;}
    QgsMeshDatasetGroupMetadata datasetGroupMetadata( int groupIndex ) const {return QgsMeshDatasetGroupMetadata(); }
    QgsMeshDatasetMetadata datasetMetadata( QgsMeshDatasetIndex index ) const {return QgsMeshDatasetMetadata();}
    QgsMeshDatasetValue datasetValue( QgsMeshDatasetIndex index, int valueIndex ) const {return QgsMeshDatasetValue();}
    QgsMeshDataBlock datasetValues( QgsMeshDatasetIndex index, int valueIndex, int count ) const {return QgsMeshDataBlock();}
    QgsMesh3dDataBlock dataset3dValues( QgsMeshDatasetIndex index, int faceIndex, int count ) const {return QgsMesh3dDataBlock();}
    bool isFaceActive( QgsMeshDatasetIndex index, int faceIndex ) const {return false;}
    QgsMeshDataBlock areFacesActive( QgsMeshDatasetIndex index, int faceIndex, int count ) const {return QgsMeshDataBlock();}
    bool persistDatasetGroup( const QString &outputFilePath,
                              const QString &outputDriver,
                              const QgsMeshDatasetGroupMetadata &meta,
                              const QVector<QgsMeshDataBlock> &datasetValues,
                              const QVector<QgsMeshDataBlock> &datasetActive,
                              const QVector<double> &times ) {return false;}
    bool persistDatasetGroup( const QString &outputFilePath,
                              const QString &outputDriver,
                              QgsMeshDatasetSourceInterface *source,
                              int datasetGroupIndex )
    {return false;}

    // QgsMeshDataSourceInterface interface
  public:
    int vertexCount() const;
    int faceCount() const;
    int edgeCount() const {return 0;}
    void populateMesh( QgsMesh *mesh ) const;
    bool saveMeshFrame( const QgsMesh &mesh ) {return false;}

    // QgsDataProvider interface
  public:
    QgsCoordinateReferenceSystem crs() const {return QgsCoordinateReferenceSystem();}
    QgsRectangle extent() const {return QgsRectangle( 0, 0, 10, 10 );}
    bool isValid() const {return true;}
    QString name() const {return "ReosMeshMemory";}
    QString description() const {return "reos mesh";}

    // QgsMeshDataProvider interface
  public:
    void close() {}

//***********************

  private:
    QgsMesh mCacheMesh;

    static QgsMesh convertFrameFromReos( const ReosMeshFrameData &reosMesh );

};


class ReosMeshProviderMetaData: public QgsProviderMetadata
{
  public:
    ReosMeshProviderMetaData() : QgsProviderMetadata( QStringLiteral( "ReosMeshMemory" ), QStringLiteral( "reos mesh" ) ) {}
    QString filters( FilterType type ) override {return QString();}
    QList<QgsMeshDriverMetadata> meshDriversMetadata() override {return QList<QgsMeshDriverMetadata>();}
    ReosMeshDataProvider_p *createProvider( const QString &uri, const QgsDataProvider::ProviderOptions &options, QgsDataProvider::ReadFlags flags = QgsDataProvider::ReadFlags() ) override
    {
      return new ReosMeshDataProvider_p();
    }

    bool createMeshData( const QgsMesh &mesh,
                         const QString &fileName,
                         const QString &driverName,
                         const QgsCoordinateReferenceSystem &crs ) const override {return false;}

    bool createMeshData( const QgsMesh &mesh,
                         const QString &uri,
                         const QgsCoordinateReferenceSystem &crs ) const override  {return false;}

    QVariantMap decodeUri( const QString &uri ) const override {return QVariantMap();}
    QString encodeUri( const QVariantMap &parts ) const override {return QString();}
    ProviderCapabilities providerCapabilities() const override {return FileBasedUris;}
    QgsProviderMetadata::ProviderMetadataCapabilities capabilities() const override {return QgsProviderMetadata::LayerTypesForUri;}
    QList< QgsProviderSublayerDetails > querySublayers( const QString &uri, Qgis::SublayerQueryFlags flags = Qgis::SublayerQueryFlags(), QgsFeedback *feedback = nullptr ) const override {return QList< QgsProviderSublayerDetails >() ;}
};

#endif // REOSMESHDATAPROVIDER_P_H