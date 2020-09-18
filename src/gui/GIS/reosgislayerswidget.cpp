/***************************************************************************
                      reosgislayerwidget.cpp
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

#include "reosgislayerswidget.h"

#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <qgslayertreeview.h>
#include <qgslayertree.h>
#include <qgsvectorlayerproperties.h>
#include <qgsrasterlayerproperties.h>
#include <qgsmeshlayerproperties.h>
#include <qgsmapcanvas.h>
#include <qgsmessagebar.h>

#include "reosgisengine.h"
#include "reossettings.h"
#include "reosmap.h"

ReosGisLayersWidget::ReosGisLayersWidget( ReosGisEngine *engine, ReosMap *map, QWidget *parent ):
  QWidget( parent ),
  mGisEngine( engine ),
  mMap( map ),
  mTreeView( new QgsLayerTreeView( this ) ),
  mToolBar( new QToolBar( this ) ),
  mActionLoadVectorLayer( new QAction( QPixmap( ":/images/mActionAddVectorLayer.png" ), tr( "Add Vector Layer" ), this ) ),
  mActionLoadRasterLayer( new QAction( QPixmap( ":/images/mActionAddRasterLayer.png" ), tr( "Add Raster Layer" ), this ) ),
  mActionLoadMeshLayer( new QAction( QPixmap( ":/images/mActionAddMeshLayer.svg" ), tr( "Add Mesh Layer" ), this ) )
{
  mTreeView->setModel( engine->layerTreeModel() );

  setLayout( new QVBoxLayout() );

  layout()->addWidget( mToolBar );
  layout()->addWidget( mTreeView );

  mToolBar->addAction( mActionLoadVectorLayer );
  mToolBar->addAction( mActionLoadRasterLayer );
  mToolBar->addAction( mActionLoadMeshLayer );

  connect( mActionLoadVectorLayer, &QAction::triggered, this, &ReosGisLayersWidget::onLoadVectorLayer );
  connect( mActionLoadRasterLayer, &QAction::triggered, this, &ReosGisLayersWidget::onLoadRasterLayer );
  connect( mActionLoadMeshLayer, &QAction::triggered, this, &ReosGisLayersWidget::onLoadMeshLayer );


  connect( mTreeView, &QAbstractItemView::doubleClicked, this, &ReosGisLayersWidget::onTreeLayerDoubleClick );
}

void ReosGisLayersWidget::onLoadVectorLayer()
{
  ReosSettings settings;
  QString path = settings.value( QStringLiteral( "/Path/GisLayer" ) ).toString();

  const QString vectorFileName = QFileDialog::getOpenFileName( this, tr( "Load Vector Layer" ), path, mGisEngine->vectorLayerFilters() );
  const QFileInfo fileInfo( vectorFileName );
  if ( fileInfo.exists() )
    if ( ! mGisEngine->addVectorLayer( vectorFileName, fileInfo.fileName() ) )
      QMessageBox::warning( this, tr( "Loading Vector Layer" ), tr( "Invalid vector layer, file not loaded." ) );

  settings.setValue( QStringLiteral( "/Path/GisLayer" ), fileInfo.path() );
}

void ReosGisLayersWidget::onLoadRasterLayer()
{
  ReosSettings settings;
  QString path = settings.value( QStringLiteral( "/Path/GisLayer" ) ).toString();

  const QString rasterFileName = QFileDialog::getOpenFileName( this, tr( "Load Raster Layer" ), path, mGisEngine->rasterLayerFilters() );
  const QFileInfo fileInfo( rasterFileName );
  if ( fileInfo.exists() )
    if ( ! mGisEngine->addRasterLayer( rasterFileName, fileInfo.fileName() ) )
      QMessageBox::warning( this, tr( "Loading Raster Layer" ), tr( "Invalid raster layer, file not loaded." ) );

  settings.setValue( QStringLiteral( "/Path/GisLayer" ), fileInfo.path() );
}

void ReosGisLayersWidget::onLoadMeshLayer()
{
  ReosSettings settings;
  QString path = settings.value( QStringLiteral( "/Path/GisLayer" ) ).toString();
  QString filter = mGisEngine->meshLayerFilters();

  if ( filter.isEmpty() )
    return;

  const QString meshFileName = QFileDialog::getOpenFileName( this, tr( "Load Mesh Layer" ), path, filter );

  const QFileInfo fileInfo( meshFileName );
  if ( fileInfo.exists() )
    if ( ! mGisEngine->addMeshLayer( meshFileName, fileInfo.fileName() ) )
      QMessageBox::warning( this, tr( "Loading Raster Layer" ), tr( "Invalid raster layer, file not loaded." ) );

}

void ReosGisLayersWidget::onTreeLayerDoubleClick()
{
  if ( ! mTreeView && !mMap )
    return;

  QgsMapLayer *mapLayer = mTreeView->currentLayer();

  if ( !mapLayer )
    return;

  QgsMapCanvas *mapCanvas = qobject_cast<QgsMapCanvas *>( mMap->mapCanvas() );

  std::unique_ptr<QDialog> dial = nullptr;
  QgsMessageBar messageBar;
  switch ( mapLayer->type() )
  {
    case QgsMapLayerType::VectorLayer:
      dial.reset( new QgsVectorLayerProperties( mapCanvas, &messageBar, qobject_cast<QgsVectorLayer *>( mapLayer ), this ) );
      break;
    case QgsMapLayerType::RasterLayer:
      dial.reset( new QgsRasterLayerProperties( mapLayer, mapCanvas, this ) );
    case QgsMapLayerType::MeshLayer:
      dial.reset( new QgsMeshLayerProperties( mapLayer, mapCanvas, this ) );
    default:
      break;
  }

  if ( dial )
    dial->exec();

}