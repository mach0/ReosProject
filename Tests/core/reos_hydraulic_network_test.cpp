/***************************************************************************
                      reos_hydraulic_structure_2d_test.cpp
                     --------------------------------------
Date                 : Januay-2022
Copyright            : (C) 2022 by Vincent Cloarec
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
#include <QModelIndexList>

#include "reoshydraulicnetwork.h"
#include "reoshydraulicelementmodel.h"
#include "reosgisengine.h"
#include "reoshydrographsource.h"
#include "reoshydrographrouting.h"
#include "reoshydraulicscheme.h"
#include "reosarea.h"
#include "reosrunoffmodel.h"
#include "reostransferfunction.h"
#include "reoshydraulicsimulation.h"
#include "reoshydraulicstructure2d.h"
#include "reoshydraulicstructureboundarycondition.h"
#include "reostimewindowsettings.h"
#include "reos_testutils.h"

#define WAITING_TIME_FOR_LOOP 100

class ReoHydraulicNetworkTest: public QObject
{
    Q_OBJECT
  private slots:

    void initTestCase();
    void addRemoveElement();
    void calculationPropagation();

  private:
    ReosHydraulicNetwork *mNetwork = nullptr;
    ReosWatershedModule *mWatershedModule = nullptr;
    ReosModule *mRootModule = nullptr;
    ReosGisEngine *mGisEngine = nullptr;
    ReosHydraulicElementModel *mElementModel = nullptr;

};

void ReoHydraulicNetworkTest::initTestCase()
{
  mRootModule = new ReosModule( QStringLiteral( "root" ), this );
  mRootModule->setProjectFileName( "ooooo" );
  mGisEngine = new ReosGisEngine( this );
  mGisEngine->setCrs( ReosGisEngine::crsFromEPSG( 32620 ) );
  mWatershedModule = new ReosWatershedModule( mRootModule, mGisEngine );
  mNetwork = new ReosHydraulicNetwork( mRootModule, mGisEngine, mWatershedModule );
  mElementModel = new ReosHydraulicElementModel( mNetwork );
  ReosSimulationEngineRegistery::instance()->registerEngineFactory( new ReosSimulationEngineFactoryDummy );

  QVERIFY( mNetwork->hydraulicNetworkElements().isEmpty() );
  QCOMPARE( mNetwork->schemeCount(), 1 );
}

void ReoHydraulicNetworkTest::addRemoveElement()
{
  QCOMPARE( mElementModel->rowCount( QModelIndex() ), 0 );
  ReosHydraulicNetworkElement *junctionNode1 = mNetwork->addElement( new ReosHydrographJunction( ReosSpatialPosition( QPointF( 10, 10 ), mGisEngine->crs() ), mNetwork ) );
  ReosHydraulicNetworkElement *junctionNode2 = mNetwork->addElement( new ReosHydrographJunction( ReosSpatialPosition( QPointF( 10, 20 ), mGisEngine->crs() ), mNetwork ) );
  QCOMPARE( mElementModel->rowCount( QModelIndex() ), 2 );

  mNetwork->removeElement( junctionNode1 );
  QCOMPARE( mElementModel->rowCount( QModelIndex() ), 1 );
  junctionNode1 = mNetwork->addElement( new ReosHydrographJunction( ReosSpatialPosition( QPointF( 10, 10 ), mGisEngine->crs() ), mNetwork ) );
  QCOMPARE( mElementModel->rowCount( QModelIndex() ), 2 );

  ReosHydraulicNetworkElement *link = mNetwork->addElement(
                                        new ReosHydrographRoutingLink( qobject_cast<ReosHydrographJunction *>( junctionNode1 ),
                                            qobject_cast<ReosHydrographJunction *>( junctionNode1 ),
                                            mNetwork ) );

  QCOMPARE( mElementModel->rowCount( QModelIndex() ), 3 );
  QCOMPARE( mNetwork->networkExtent(), ReosMapExtent( 10.0, 10.0, 10.0, 20.0 ) );

  QList<ReosHydraulicNetworkElement *> allElements = mNetwork->hydraulicNetworkElements();
  QVERIFY( allElements.contains( junctionNode1 ) );
  QVERIFY( allElements.contains( junctionNode2 ) );
  QVERIFY( allElements.contains( link ) );

  QModelIndex index = mElementModel->elementToIndex( junctionNode1 );
  QCOMPARE( junctionNode1, mElementModel->indexToElement( index ) );
  index = mElementModel->elementToIndex( junctionNode2 );
  QCOMPARE( junctionNode2, mElementModel->indexToElement( index ) );
  index = mElementModel->elementToIndex( link );
  QCOMPARE( link, mElementModel->indexToElement( index ) );

  mNetwork->removeElement( junctionNode1 );
  QCOMPARE( mElementModel->rowCount( QModelIndex() ), 1 );
  allElements = mNetwork->hydraulicNetworkElements();
  QVERIFY( !allElements.contains( junctionNode1 ) );
  QVERIFY( allElements.contains( junctionNode2 ) );
  QVERIFY( !allElements.contains( link ) );
  QCOMPARE( mNetwork->networkExtent(), ReosMapExtent( 10.0, 20.0, 10.0, 20.0 ) );
}

void ReoHydraulicNetworkTest::calculationPropagation()
{
  mNetwork->changeScheme( 0 );
  QVERIFY( mNetwork->currentScheme() );

  //**** Mount a watershed
  QPolygonF watershedPolygon;
  watershedPolygon << QPointF( 0, 0 ) << QPointF( 100, 0 ) << QPointF( 100, 100 ) << QPointF( 0, 100 );
  ReosWatershed *watershed =
    mWatershedModule->watershedTree()->addWatershed( new  ReosWatershed( watershedPolygon, QPointF( 0, 0 ) ) );
  watershed->concentrationTime()->setValue( ReosDuration( 10, ReosDuration::minute ) );
  watershed->calculateArea();
  QVERIFY( watershed->areaParameter()->value() == ReosArea( 1, ReosArea::ha ) );
  std::unique_ptr<ReosRunoffConstantCoefficientModel> runoffModel( new ReosRunoffConstantCoefficientModel( "runoff" ) );
  runoffModel->coefficient()->setValue( 1 );
  watershed->runoffModels()->addRunoffModel( runoffModel.get() );
  watershed->setCurrentTransferFunction( ReosTransferFunctionSCSUnitHydrograph::staticType() );
  ReosHydrographNodeWatershed *watershedNode =
    new ReosHydrographNodeWatershed( watershed, mWatershedModule->meteoModelsCollection(), mNetwork );
  mNetwork->addElement( watershedNode );

  //**** Mount a rainfall
  std::unique_ptr<ReosSeriesRainfall> rainfallSerie( new ReosSeriesRainfall );
  rainfallSerie->setReferenceTime( QDateTime( QDate( 2010, 02, 01 ), QTime( 2, 0, 0 ), Qt::UTC ) );
  rainfallSerie->setTimeStep( ReosDuration( 5, ReosDuration::minute ) );
  rainfallSerie->appendValue( 5 );
  rainfallSerie->appendValue( 10 );
  rainfallSerie->appendValue( 20 );
  std::unique_ptr<ReosRainfallGaugedRainfallItem> rainItem(
    new ReosRainfallGaugedRainfallItem( QStringLiteral( "rainfall" ), QString(), rainfallSerie.release() ) );

  ReosMeteorologicModel *meteoModel = mWatershedModule->meteoModelsCollection()->meteorologicModel( 0 );
  meteoModel->associate( watershed, rainItem.get() );

  //**** Mount a 2D structure
  QPolygonF domainD2;
  domainD2 << QPointF( 0, 0 )
           << QPointF( 10, 0 )
           << QPointF( 10, 10 )
           << QPointF( 6, 10 )
           << QPointF( 4, 10 )
           << QPointF( 0, 10 );
  ReosHydraulicStructure2D *structure2D = new ReosHydraulicStructure2D( domainD2, mGisEngine->crs(), mNetwork->context() );
  mNetwork->addElement( structure2D );
  if ( structure2D->simulationCount() != 0 )
    structure2D->removeSimulation( 0 );
  structure2D->addSimulation( QStringLiteral( "dummy-simulation" ) );
  structure2D->timeWindowSettings()->setCombineMethod( ReosTimeWindowSettings::Union );
  QCOMPARE( structure2D->simulationCount(), 1 );
  ReosMapExtent searchZone( -0.1, -0.1, 0.1, 0.1 );
  ReosGeometryStructureVertex *vertex1 = structure2D->geometryStructure()->searchForVertex( searchZone );
  QVERIFY( vertex1 );
  searchZone = ReosMapExtent( -0.1, 9.9, 0.1, 10.1 );
  ReosGeometryStructureVertex *vertex2 = structure2D->geometryStructure()->searchForVertex( searchZone );
  QVERIFY( vertex2 );
  structure2D->geometryStructure()->addBoundaryCondition( vertex1, vertex2, "BC_1" );
  QCOMPARE( structure2D->boundaryConditions().count(), 1 );
  searchZone = ReosMapExtent( 3.9, 9.9, 4.1, 10.1 );
  vertex1 = structure2D->geometryStructure()->searchForVertex( searchZone );
  searchZone = ReosMapExtent( 5.9, 9.9, 6.1, 10.1 );
  vertex2 = structure2D->geometryStructure()->searchForVertex( searchZone );
  structure2D->geometryStructure()->addBoundaryCondition( vertex1, vertex2, "BC_2" );
  QCOMPARE( structure2D->boundaryConditions().count(), 2 );
  searchZone = ReosMapExtent( 9.9, 9.9, 10.1, 10.1 );
  vertex1 = structure2D->geometryStructure()->searchForVertex( searchZone );
  searchZone = ReosMapExtent( 9.9, -0.1, 10.1, 0.1 );
  vertex2 = structure2D->geometryStructure()->searchForVertex( searchZone );
  structure2D->geometryStructure()->addBoundaryCondition( vertex1, vertex2, "BC_3" );
  QCOMPARE( structure2D->boundaryConditions().count(), 3 );
  const QList<ReosHydraulicStructureBoundaryCondition *> boundaryConditions = structure2D->boundaryConditions();
  ReosHydraulicStructureBoundaryCondition *bc1 = nullptr;
  ReosHydraulicStructureBoundaryCondition *bc2 = nullptr;
  ReosHydraulicStructureBoundaryCondition *bc3 = nullptr;
  for ( ReosHydraulicStructureBoundaryCondition *bc : boundaryConditions )
  {
    if ( bc->elementNameParameter()->value() == "BC_1" )
      bc1 = bc;
    if ( bc->elementNameParameter()->value() == "BC_2" )
      bc2 = bc;
    if ( bc->elementNameParameter()->value() == "BC_3" )
      bc3 = bc;
  }
  QVERIFY( bc1 && bc2 && bc3 );
  structure2D->setCurrentSimulation( 0 );
  QVERIFY( structure2D->currentSimulation() );

  //**** Mount a junction with a hydrograph
  ReosHydrographJunction *junction =
    new ReosHydrographJunction(
    ReosSpatialPosition( QPointF( 5.0, 20.0 ), mGisEngine->crs() ), mNetwork );
  mNetwork->addElement( junction );

  //**** Mount a simple junction for downstream
  ReosHydrographJunction *junctionDownstream =
    new ReosHydrographJunction(
    ReosSpatialPosition( QPointF( 15.0, 5.0 ), mGisEngine->crs() ), mNetwork );
  mNetwork->addElement( junctionDownstream );

  std::unique_ptr<ReosHydrograph> hydrograh( new ReosHydrograph );
  hydrograh->setValue( QDateTime( QDate( 2010, 02, 01 ), QTime( 1, 55, 0 ), Qt::UTC ), 0 );
  hydrograh->setValue( ReosDuration( 5, ReosDuration::minute ), 15 );
  hydrograh->setValue( ReosDuration( 10, ReosDuration::minute ), 10 );
  hydrograh->setValue( ReosDuration( 18, ReosDuration::minute ), 5 );
  hydrograh->setValue( ReosDuration( 20, ReosDuration::minute ), 0 );
  QVERIFY( junction->gaugedHydrographsStore()->hydrographCount() == 0 );
  junction->gaugedHydrographsStore()->addHydrograph( hydrograh.release() );
  QVERIFY( junction->gaugedHydrographsStore()->hydrographCount() == 1 );
  junction->setInternalHydrographOrigin( ReosHydrographJunction::GaugedHydrograph );
  junction->setGaugedHydrographIndex( 0 );

  //connect nodes
  ReosHydrographRoutingLink *link1 = new ReosHydrographRoutingLink( watershedNode, bc1, mNetwork );
  mNetwork->addElement( link1 );
  ReosHydrographRoutingLink *link2 = new ReosHydrographRoutingLink( junction, bc2, mNetwork );
  mNetwork->addElement( link2 );
  ReosHydrographRoutingLink *link3 = new ReosHydrographRoutingLink( bc3, junctionDownstream, mNetwork );
  mNetwork->addElement( link3 );

  QVERIFY( bc1->conditionType() == ReosHydraulicStructureBoundaryCondition::Type::InputFlow );
  QVERIFY( bc2->conditionType() == ReosHydraulicStructureBoundaryCondition::Type::InputFlow );
  QVERIFY( bc3->conditionType() == ReosHydraulicStructureBoundaryCondition::Type::OutputLevel );

  QCOMPARE( watershedNode->outputHydrograph()->valueCount(), 0 );

  std::unique_ptr<ReosMeshGeneratorProcess> meshProcess( structure2D->getGenerateMeshProcess() );
  std::unique_ptr<ModuleProcessControler> controller = std::make_unique<ModuleProcessControler>( meshProcess.get() );
  controller->waitForFinished();

  ReosModule::Message message;
  ReosSimulationData simData = structure2D->simulationData( mNetwork->currentSchemeId(), message );
  std::unique_ptr<ReosSimulationPreparationProcess> preparationProcess(
    structure2D->getPreparationProcessSimulation( simData, mNetwork->currentScheme()->calculationContext(), message ) );
  QVERIFY( preparationProcess );
  controller = std::make_unique<ModuleProcessControler>( preparationProcess.get() );
  controller->waitForFinished();

  QCOMPARE( watershedNode->outputHydrograph()->valueCount(), 71 );

  // the preparation process lead to update the upstream calculation
  // So for now, the 2D structure can automatically defined the time window from upstream
  // and we tet the time window settings

  ReosTimeWindow tw = structure2D->timeWindow();
  ReosTimeWindow mtw;

  QCOMPARE( tw.start(), QDateTime( QDate( 2010, 02, 01 ), QTime( 1, 55, 0 ), Qt::UTC ) );
  QCOMPARE( tw.end(), QDateTime( QDate( 2010, 02, 01 ), QTime( 2, 48, 45 ), Qt::UTC ) );

  structure2D->timeWindowSettings()->startOffsetParameter()->setValue( ReosDuration( 3, ReosDuration::minute ) );
  structure2D->timeWindowSettings()->endOffsetParameter()->setValue( ReosDuration( 5, ReosDuration::minute ) );

  tw = structure2D->timeWindow();
  QCOMPARE( tw.start(), QDateTime( QDate( 2010, 02, 01 ), QTime( 1, 58, 0 ), Qt::UTC ) );
  QCOMPARE( tw.end(), QDateTime( QDate( 2010, 02, 01 ), QTime( 2, 53, 45 ), Qt::UTC ) );
  mtw = mGisEngine->mapTimeWindow();
  QVERIFY( tw == mtw );

  structure2D->timeWindowSettings()->setOriginStart( ReosTimeWindowSettings::End );
  structure2D->timeWindowSettings()->startOffsetParameter()->setValue( ReosDuration( -2, ReosDuration::hour ) );

  tw = structure2D->timeWindow();
  QCOMPARE( tw.start(), QDateTime( QDate( 2010, 02, 01 ), QTime( 0, 48, 45 ), Qt::UTC ) );
  QCOMPARE( tw.end(), QDateTime( QDate( 2010, 02, 01 ), QTime( 2, 53, 45 ), Qt::UTC ) );
  mtw = mGisEngine->mapTimeWindow();
  QVERIFY( tw == mtw );

  structure2D->timeWindowSettings()->startOffsetParameter()->setValue( ReosDuration( 0, ReosDuration::minute ) );
  structure2D->timeWindowSettings()->endOffsetParameter()->setValue( ReosDuration( 0, ReosDuration::minute ) );
  structure2D->timeWindowSettings()->setOriginStart( ReosTimeWindowSettings::Begin );

  tw = structure2D->timeWindow();
  QCOMPARE( tw.start(), QDateTime( QDate( 2010, 02, 01 ), QTime( 1, 55, 0 ), Qt::UTC ) );
  QCOMPARE( tw.end(), QDateTime( QDate( 2010, 02, 01 ), QTime( 2, 48, 45 ), Qt::UTC ) );
  mtw = mGisEngine->mapTimeWindow();
  QVERIFY( tw == mtw );

  // we have to wait that upstream calculation is finished
  simulateEventLoop( WAITING_TIME_FOR_LOOP );

  QString error;
  ReosSimulationProcess *simulationProcess =
    structure2D->createSimulationProcess( mNetwork->currentScheme()->calculationContext(), error );

  controller = std::make_unique<ModuleProcessControler>( simulationProcess );
  controller->waitForFinished();

  // we have to wait that propagation is fnished in the last link
  simulateEventLoop( WAITING_TIME_FOR_LOOP );

  QCOMPARE( bc3->outputHydrograph()->valueCount(), 75 );
  ReosHydrograph *downstreamHydro = junctionDownstream->outputHydrograph();
  QCOMPARE( downstreamHydro->valueCount(), 75 );

  // Update calculation context on the downstream bc of the structure, that should change nothing
  bc3->updateCalculationContext( mNetwork->currentScheme()->calculationContext() );
  simulateEventLoop( WAITING_TIME_FOR_LOOP );

  QCOMPARE( bc3->outputHydrograph()->valueCount(), 75 );

  structure2D->updateResults( mNetwork->currentScheme()->id() );
  QCOMPARE( bc3->outputHydrograph()->valueCount(), 75 );

  simulateEventLoop( WAITING_TIME_FOR_LOOP );

  QCOMPARE( bc3->outputHydrograph()->valueCount(), 75 );

  mWatershedModule->meteoModelsCollection()->addMeteorologicModel( QStringLiteral( "Other meteomodel" ) );
  QCOMPARE( mWatershedModule->meteoModelsCollection()->modelCount(), 2 );

  ReosHydraulicScheme *scheme = mNetwork->addNewScheme( tr( "Other scheme" ), mWatershedModule->meteoModelsCollection()->meteorologicModel( 1 ) );
  QCOMPARE( scheme->meteoModel()->name()->value(), QStringLiteral( "Other meteomodel" ) );

  QCOMPARE( mNetwork->schemeCount(), 2 );
  QCOMPARE( watershedNode->outputHydrograph()->valueCount(), 71 );
  QCOMPARE( link1->outputHydrograph()->valueCount(), 71 );
  QCOMPARE( bc3->outputHydrograph()->valueCount(), 75 );
  QCOMPARE( junction->outputHydrograph()->valueCount(), 5 );
  //Scheme changed
  mNetwork->changeScheme( 1 );
  QCOMPARE( watershedNode->outputHydrograph()->valueCount(), 0 );
  QCOMPARE( link1->outputHydrograph()->valueCount(), 0 );
  QCOMPARE( bc3->outputHydrograph()->valueCount(), 0 );
  QCOMPARE( junction->outputHydrograph()->valueCount(), 0 );

  tw = structure2D->timeWindow();
  QCOMPARE( tw.start(), QDateTime() );
  QCOMPARE( tw.end(), QDateTime() );
  mtw = mGisEngine->mapTimeWindow();
  QVERIFY( tw == mtw );

  simulateEventLoop( WAITING_TIME_FOR_LOOP );

  QCOMPARE( watershedNode->outputHydrograph()->valueCount(), 0 );
  QCOMPARE( link1->outputHydrograph()->valueCount(), 0 );
  QCOMPARE( bc3->outputHydrograph()->valueCount(), 0 );
  QCOMPARE( junction->outputHydrograph()->valueCount(), 5 ); //hydrograph on junction is the same with this scheme

  tw = structure2D->timeWindow();
  QCOMPARE( tw.start(),  QDateTime( QDate( 2010, 02, 01 ), QTime( 1, 55, 0 ), Qt::UTC ) );
  QCOMPARE( tw.end(), QDateTime( QDate( 2010, 02, 01 ), QTime( 2, 15, 0 ), Qt::UTC ) );
  mtw = mGisEngine->mapTimeWindow();
  QVERIFY( tw == mtw );

  mNetwork->changeScheme( 0 );
  structure2D->updateCalculationContext( mNetwork->calculationContext() );
  QCOMPARE( watershedNode->outputHydrograph()->valueCount(), 0 );
  QCOMPARE( link1->outputHydrograph()->valueCount(), 0 );
  QCOMPARE( bc3->outputHydrograph()->valueCount(), 0 );
  QCOMPARE( junction->outputHydrograph()->valueCount(), 0 );

  simulateEventLoop( WAITING_TIME_FOR_LOOP );

  QCOMPARE( watershedNode->outputHydrograph()->valueCount(), 71 );
  QCOMPARE( link1->outputHydrograph()->valueCount(), 71 );
  QCOMPARE( bc3->outputHydrograph()->valueCount(), 75 );
  QCOMPARE( junction->outputHydrograph()->valueCount(), 5 );

  simulateEventLoop( WAITING_TIME_FOR_LOOP );

  tw = structure2D->timeWindow();
  QCOMPARE( tw.start(), QDateTime( QDate( 2010, 02, 01 ), QTime( 1, 55, 0 ), Qt::UTC ) );
  QCOMPARE( tw.end(), QDateTime( QDate( 2010, 02, 01 ), QTime( 2, 48, 45 ), Qt::UTC ) );
  mtw = mGisEngine->mapTimeWindow();
  QVERIFY( tw == mtw );
}



QTEST_MAIN( ReoHydraulicNetworkTest )
#include "reos_hydraulic_network_test.moc"
