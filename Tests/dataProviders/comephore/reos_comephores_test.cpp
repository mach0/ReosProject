/***************************************************************************
                      reos_raster_test.cpp
                     --------------------------------------
Date                 : 04-09-2020
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
#include<QtTest/QtTest>
#include <QObject>

#include "reos_testutils.h"
#include "reosgriddedrainitem.h"
#include "reosgriddedrainfallprovider.h"
#include "reosnetcdfutils.h"
#include "reosgisengine.h"

class ReosComephoreTest: public QObject
{
    Q_OBJECT

  private slots:
    void netcdfFile();
    void createProvider();
    void createRainfall();
};

void ReosComephoreTest::netcdfFile()
{
  ReosNetCdfFile file( COMEPHORE_FILES_PATH + QStringLiteral( "/comephore_nc/comephore_1km-1h_202001.nc" ), false );
  QVERIFY( file.isValid() );

  QVERIFY( file.hasVariable( QStringLiteral( "RR" ) ) );
  QCOMPARE( file.variableDimensionCount( QStringLiteral( "RR" ) ), 3 );

  QStringList dimensionNames = file.variableDimensionNames( QStringLiteral( "RR" ) );
  QVERIFY( dimensionNames.contains( QStringLiteral( "time" ) ) );
  QVERIFY( dimensionNames.contains( QStringLiteral( "X" ) ) );
  QVERIFY( dimensionNames.contains( QStringLiteral( "Y" ) ) );

  QCOMPARE( file.dimensionLength( QStringLiteral( "time" ) ), 744 );
  QCOMPARE( file.dimensionLength( QStringLiteral( "X" ) ), 1536 );
  QCOMPARE( file.dimensionLength( QStringLiteral( "Y" ) ), 1536 );

  QString proj4Crs = file.globalStringAttributeValue( "crs_proj4_string" );

  QString crs = ReosGisEngine::crsFromProj( proj4Crs );

  std::unique_ptr<ReosDataProvider> compatibleProvider( ReosDataProviderRegistery::instance()->createCompatibleProvider(
        COMEPHORE_FILES_PATH + QStringLiteral( "/comephore_nc/comephore_1km-1h_202001.nc" ), ReosGriddedRainfall::staticType() ) );
  QVERIFY( compatibleProvider );

}

void ReosComephoreTest::createProvider()
{
  std::unique_ptr<ReosDataProvider> compatibleProvider( ReosDataProviderRegistery::instance()->createCompatibleProvider(
        COMEPHORE_FILES_PATH + QStringLiteral( "/tif.tif" ), ReosGriddedRainfall::staticType() ) );
  QVERIFY( !compatibleProvider );

  compatibleProvider.reset( ReosDataProviderRegistery::instance()->createCompatibleProvider(
                              COMEPHORE_FILES_PATH + QStringLiteral( "/tif" ), ReosGriddedRainfall::staticType() ) );
  QVERIFY( compatibleProvider );

  ReosGriddedRainfallProvider *provider = qobject_cast<ReosGriddedRainfallProvider *>( compatibleProvider.get() );

  QString comephoresPath( COMEPHORE_FILES_PATH + QStringLiteral( "/tif.tif" ) );
  provider->setDataSource( comephoresPath );
  QVERIFY( !provider->isValid() );

  comephoresPath = QString( COMEPHORE_FILES_PATH + QStringLiteral( "/tif" ) );
  provider->setDataSource( comephoresPath );
  QVERIFY( provider->isValid() );

  QCOMPARE( provider->count(), 73 );
  QCOMPARE( provider->startTime( 0 ), QDateTime( QDate( 2018, 02, 18 ), QTime( 0, 0, 0 ), Qt::UTC ) );
  QCOMPARE( provider->endTime( 0 ), QDateTime( QDate( 2018, 02, 18 ), QTime( 1, 0, 0 ), Qt::UTC ) );
  QCOMPARE( provider->startTime( 1 ), QDateTime( QDate( 2018, 02, 18 ), QTime( 1, 0, 0 ), Qt::UTC ) );
  QCOMPARE( provider->endTime( 1 ), QDateTime( QDate( 2018, 02, 18 ), QTime( 2, 0, 0 ), Qt::UTC ) );
  QCOMPARE( provider->startTime( 50 ), QDateTime( QDate( 2018, 02, 20 ), QTime( 2, 0, 0 ), Qt::UTC ) );
  QCOMPARE( provider->endTime( 50 ), QDateTime( QDate( 2018, 02, 20 ), QTime( 3, 0, 0 ), Qt::UTC ) );

  ReosRasterExtent extent = provider->extent();

  QCOMPARE( extent.xCellSize(), 1000.0 );
  QCOMPARE( extent.yCellSize(), -1000.0 );
  QCOMPARE( extent.xCellCount(), 101 );
  QCOMPARE( extent.yCellCount(), 170 );

  QVector<double> values = provider->data( 70 );
  QCOMPARE( values.at( 8581 ), 2.2 );
  QCOMPARE( values.at( 8584 ), 3.0 );
  QVERIFY( std::isnan( values.last() ) );

  double min = 0, max = 0;
  QVERIFY( !provider->getDirectMinMax( min, max ) );
  provider->calculateMinMax( min, max );
  QCOMPARE( min, 0.1 );
  QCOMPARE( max, 12.4 );
}

void ReosComephoreTest::createRainfall()
{
  std::unique_ptr<ReosGriddedRainfall> rainfall =
    std::make_unique<ReosGriddedRainfall>( COMEPHORE_FILES_PATH + QStringLiteral( "/tif" ), QStringLiteral( "comephore" ) );

  QCOMPARE( rainfall->gridCount(), 73 );
  QCOMPARE( rainfall->startTime( 0 ), QDateTime( QDate( 2018, 02, 18 ), QTime( 0, 0, 0 ), Qt::UTC ) );
  QCOMPARE( rainfall->endTime( 0 ), QDateTime( QDate( 2018, 02, 18 ), QTime( 1, 0, 0 ), Qt::UTC ) );
  QCOMPARE( rainfall->startTime( 1 ), QDateTime( QDate( 2018, 02, 18 ), QTime( 1, 0, 0 ), Qt::UTC ) );
  QCOMPARE( rainfall->endTime( 1 ), QDateTime( QDate( 2018, 02, 18 ), QTime( 2, 0, 0 ), Qt::UTC ) );
  QCOMPARE( rainfall->startTime( 50 ), QDateTime( QDate( 2018, 02, 20 ), QTime( 2, 0, 0 ), Qt::UTC ) );
  QCOMPARE( rainfall->endTime( 50 ), QDateTime( QDate( 2018, 02, 20 ), QTime( 3, 0, 0 ), Qt::UTC ) );

  ReosRasterExtent extent = rainfall->rasterExtent();

  QCOMPARE( extent.xCellSize(), 1000.0 );
  QCOMPARE( extent.yCellSize(), -1000.0 );
  QCOMPARE( extent.xCellCount(), 101 );
  QCOMPARE( extent.yCellCount(), 170 );

  QVector<double> values = rainfall->intensityValues( 70 );
  QCOMPARE( values.at( 8581 ), 2.2 );
  QCOMPARE( values.at( 8584 ), 3.0 );
  QVERIFY( std::isnan( values.last() ) );

  double min, max;
  QVERIFY( !rainfall->getDirectMinMaxValue( min, max ) );
  rainfall->calculateMinMaxValue( min, max );
  QCOMPARE( min, 0.1 );
  QCOMPARE( max, 12.4 );
}


QTEST_MAIN( ReosComephoreTest )
#include "reos_comephores_test.moc"
