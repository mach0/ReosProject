#include "reoswatershedwidget.h"
#include "ui_reoswatershedwidget.h"

#include "reosmaptool.h"
#include "reossettings.h"
#include "reoswatershedmodule.h"
#include "reoswatershedtree.h"
#include "reosdelineatingwatershedwidget.h"
#include "reoslongitudinalprofilewidget.h"
#include "reosmenupopulator.h"

#include <QMessageBox>

ReosWatershedWidget::ReosWatershedWidget( ReosMap *map, ReosWatershedModule *module, QWidget *parent ) :
  QWidget( parent ),
  ui( new Ui::ReosWatershedWidget ),
  mMap( map ),
  mActionSelectWatershed( new QAction( QPixmap( ":/images/selectWatershed.svg" ), tr( "Select watershed on map" ), this ) ),
  mMapToolSelectWatershed( new ReosMapToolSelectMapItem( map, QStringLiteral( "Watershed" ) ) ),
  mActionRemoveWatershed( new QAction( QPixmap( ":/images/removeWatershed.svg" ), tr( "Remove watershed" ), this ) ),
  mActionDelineateWatershed( new QAction( QPixmap( ":/images/delineateWatershed.svg" ), tr( "Delineate watershed" ), this ) ),
  mDelineatingWidget( new ReosDelineatingWatershedWidget( module, map, this ) ),
  mActionLongitudinalProfile( new QAction( QPixmap( ":/images/longProfile.svg" ), tr( "Longitudinal profile" ) ) ),
  mLongitudinalProfileWidget( new ReosLongitudinalProfileWidget( map, this ) ),
  mCurrentMapOutlet( map ),
  mCurrentStreamLine( map )
{
  ui->setupUi( this );
  setModel( new ReosWatershedItemModel( module->watershedTree(), this ) );

  mActionDelineateWatershed->setCheckable( true );
  mActionLongitudinalProfile->setCheckable( true );
  QToolBar *toolBar = new QToolBar( this );
  toolBar->addAction( mActionSelectWatershed );
  toolBar->addAction( mActionRemoveWatershed );
  toolBar->addAction( mActionDelineateWatershed );
  toolBar->addAction( mActionLongitudinalProfile );
  static_cast<QBoxLayout *>( layout() )->insertWidget( 0, toolBar );
  mDelineatingWidget->setAction( mActionDelineateWatershed );
  mLongitudinalProfileWidget->setAction( mActionLongitudinalProfile );

  mMapToolSelectWatershed->setAction( mActionSelectWatershed );
  mActionSelectWatershed->setCheckable( true );
  mMapToolSelectWatershed->setSearchUnderPoint( true );
  mMapToolSelectWatershed->setCursor( Qt::ArrowCursor );
  connect( mMapToolSelectWatershed, &ReosMapToolSelectMapItem::found, this, &ReosWatershedWidget::onWatershedSelectedOnMap );

  connect( ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ReosWatershedWidget::onCurrentWatershedChange );

  std::unique_ptr<ReosMenuPopulator> menuPopulator = std::make_unique<ReosMenuPopulator>();
  menuPopulator->addAction( mActionLongitudinalProfile );
  menuPopulator->addAction( mActionRemoveWatershed );
  mMapToolSelectWatershed->setContextMenuPopulator( menuPopulator.release() );

  mCurrentMapOutlet.setWidth( 4 );
  mCurrentMapOutlet.setExternalWidth( 6 );
  mCurrentMapOutlet.setColor( QColor( 0, 155, 242 ) );
  mCurrentMapOutlet.setExternalColor( Qt::white );
  mCurrentMapOutlet.setZValue( 10 );

  connect( this, &ReosWatershedWidget::currentWatershedChanged, mLongitudinalProfileWidget, &ReosLongitudinalProfileWidget::setCurrentWatershed );

  connect( module, &ReosWatershedModule::hasBeenReset, this, &ReosWatershedWidget::onModuleReset );

  connect( mActionRemoveWatershed, &QAction::triggered, this, &ReosWatershedWidget::onRemoveWatershed );
}

ReosWatershedWidget::~ReosWatershedWidget()
{
  delete ui;
}

void ReosWatershedWidget::setModel( ReosWatershedItemModel *model )
{
  ui->treeView->setModel( model );
  mModelWatershed = model;
  connect( model, &ReosWatershedItemModel::watershedAdded, this, &ReosWatershedWidget::onWatershedAdded );
  connect( model, &ReosWatershedItemModel::watershedAdded, ui->treeView, &QTreeView::expand );
  connect( model, &ReosWatershedItemModel::dataChanged, this, &ReosWatershedWidget::onWatershedDataChanged );
}

void ReosWatershedWidget::onWatershedAdded( const QModelIndex &index )
{
  clearSelection();
  ReosWatershed *ws = mModelWatershed->indexToWatershed( index );
  if ( !ws )
    return;
  mMapWatersheds.insert( ws, formatWatershedPolygon( ReosMapPolygon( mMap, ws->delineating() ) ) );
  ui->treeView->selectionModel()->select( index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows );
  ui->treeView->setCurrentIndex( index );
}

void ReosWatershedWidget::onWatershedSelectedOnMap( ReosMapItem *item, const QPointF &pos )
{
  QList<ReosWatershed *> keys = mMapWatersheds.keys();

  for ( ReosWatershed *ws : keys )
  {
    if ( mMapWatersheds[ws].isItem( item ) )
    {
      //Watershed found, return the more upstream under the point pos
      ReosWatershed *uws = ws->upstreamWatershed( pos, true );
      if ( uws )
        ws = uws;
      ui->treeView->setCurrentIndex( mModelWatershed->watershedToIndex( ws ) );
      return;
    }
  }
}

void ReosWatershedWidget::onRemoveWatershed()
{
  QModelIndex currentndex = ui->treeView->currentIndex();

  ReosWatershed *ws = mModelWatershed->indexToWatershed( currentndex );

  if ( !ws )
    return;

  if ( QMessageBox::warning( this, tr( "Removing watershed" ), tr( "Do you want to remove the current watershed '%1'?" ).arg( ws->name() ),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) == QMessageBox::No )
    return;

  mModelWatershed->removeWatershed( currentndex );
  mMapWatersheds.remove( ws );
  mCurrentMapOutlet.resetPoint();
  emit currentWatershedChanged( nullptr );
}

void ReosWatershedWidget::onCurrentWatershedChange( const QItemSelection &selected, const QItemSelection &deselected )
{
  ReosWatershed *currentWatershed = nullptr;
  QModelIndex currentIndex;
  if ( selected.indexes().count() > 0 )
  {
    currentIndex = selected.indexes().at( 0 );
    currentWatershed = mModelWatershed->indexToWatershed( currentIndex );
  }
  ReosWatershed *previousWatershed = nullptr;
  if ( deselected.indexes().count() > 0 )
    previousWatershed = mModelWatershed->indexToWatershed( deselected.indexes().at( 0 ) );

  QMap<ReosWatershed *, ReosMapPolygon>::iterator it = mMapWatersheds.find( previousWatershed );
  if ( it != mMapWatersheds.end() )
  {
    it.value().setFillColor( QColor() );
  }

  it = mMapWatersheds.find( currentWatershed );
  if ( it != mMapWatersheds.end() )
  {
    it.value().setFillColor( QColor( 0, 255, 0, 30 ) );
    mCurrentMapOutlet.resetPoint( currentWatershed->outletPoint() );
  }
  else
  {
    mMapWatersheds.insert( currentWatershed, formatWatershedPolygon( ReosMapPolygon( mMap, currentWatershed->delineating() ) ) );
    onCurrentWatershedChange( selected, deselected );
    return;
  }

  emit currentWatershedChanged( currentWatershed );
}

void ReosWatershedWidget::onWatershedDataChanged( const QModelIndex &index )
{
  if ( ui->treeView->currentIndex() == index )
  {
    ReosWatershed *currentWatershed = mModelWatershed->indexToWatershed( index );
    mCurrentMapOutlet.resetPoint( currentWatershed->outletPoint() );
  }
}

void ReosWatershedWidget::onModuleReset()
{
  mMapWatersheds.clear();

  const QList<ReosWatershed *> allWs = mModelWatershed->allWatersheds();

  for ( ReosWatershed *ws : allWs )
    mMapWatersheds.insert( ws, formatWatershedPolygon( ReosMapPolygon( mMap, ws->delineating() ) ) );
}

ReosMapPolygon &ReosWatershedWidget::formatWatershedPolygon( ReosMapPolygon &&watershedPolygon )
{
  watershedPolygon.setDescription( QStringLiteral( "Watershed" ) );
  watershedPolygon.setWidth( 3 );
  watershedPolygon.setColor( QColor( 0, 200, 100 ) );
  watershedPolygon.setExternalWidth( 5 );
  return watershedPolygon;
}

void ReosWatershedWidget::clearSelection()
{
  QMap<ReosWatershed *, ReosMapPolygon>::iterator it = mMapWatersheds.begin();
  while ( it != mMapWatersheds.end() )
    ( it++ ).value().setFillColor( QColor() );
}
