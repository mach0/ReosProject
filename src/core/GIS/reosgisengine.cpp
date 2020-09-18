/***************************************************************************
                      reosgisengine.cpp
                     --------------------------------------
Date                 : 17-09-2020
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

#include "reosgisengine.h"

#include <qgslayertreemodel.h>
#include <qgsproject.h>
#include <qgsvectorlayer.h>
#include <qgsrasterlayer.h>
#include <qgsmeshlayer.h>
#include <qgsprovidermetadata.h>
#include <qgsproviderregistry.h>
#include <qgsapplication.h>

ReosGisEngine::ReosGisEngine( QObject *parent ): ReosModule( parent )
{
  QgsApplication::init();
  QgsProviderRegistry::instance( QGIS_PLUGINS );
  mLayerTreeModel = new QgsLayerTreeModel( QgsProject::instance()->layerTreeRoot(), this );
}

bool ReosGisEngine::addVectorLayer( QString uri, QString name )
{
  std::unique_ptr<QgsVectorLayer> vectorLayer( new QgsVectorLayer( uri, name ) );

  if ( vectorLayer->isValid() )
  {
    QgsProject::instance()->addMapLayer( vectorLayer.release() );
    message( tr( "Vector layer loaded: %1" ).arg( uri ) );
    return true;
  }
  else
  {
    warning( tr( "Vector layer not loaded: %1" ).arg( uri ) );
    return false;
  }
}

bool ReosGisEngine::addRasterLayer( QString uri, QString name )
{
  std::unique_ptr<QgsRasterLayer> rasterlayer( new QgsRasterLayer( uri, name ) );

  if ( rasterlayer->isValid() )
  {
    QgsProject::instance()->addMapLayer( rasterlayer.release() );
    message( tr( "Raster layer loaded: %1" ).arg( uri ) );
    return true;
  }
  else
  {
    warning( tr( "Raster layer not loaded: %1" ).arg( uri ) );
    return false;
  }
}

bool ReosGisEngine::addMeshLayer( QString uri, QString name )
{
  std::unique_ptr<QgsMeshLayer> meshLayer( new QgsMeshLayer( uri, name, "mdal" ) );

  if ( meshLayer->isValid() )
  {
    QgsProject::instance()->addMapLayer( meshLayer.release() );
    message( tr( "Mesh layer loaded: %1" ).arg( uri ) );
    return true;
  }
  else
  {
    warning( tr( "Mesh layer not loaded: %1" ).arg( uri ) );
    return false;
  }

}

QAbstractItemModel *ReosGisEngine::layerTreeModel() {return mLayerTreeModel;}

QString ReosGisEngine::vectorLayerFilters() const
{
  QgsProviderMetadata *meta = QgsProviderRegistry::instance()->providerMetadata( "ogr" );
  return meta->filters( QgsProviderMetadata::FilterType::FilterVector );
}

QString ReosGisEngine::rasterLayerFilters() const
{
  QgsProviderMetadata *meta = QgsProviderRegistry::instance()->providerMetadata( "gdal" );
  return meta->filters( QgsProviderMetadata::FilterType::FilterRaster );
}

QString ReosGisEngine::meshLayerFilters() const
{
  QgsProviderMetadata *meta = QgsProviderRegistry::instance()->providerMetadata( "mdal" );

  if ( meta )
    return meta->filters( QgsProviderMetadata::FilterType::FilterMesh );
  else
  {
    error( tr( "Could not find MDAL" ) );
    return QString();
  }
}