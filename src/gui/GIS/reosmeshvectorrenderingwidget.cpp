/***************************************************************************
  reosmeshvectorrenderingwidget.cpp - ReosMeshVectorRenderingWidget

 ---------------------
 begin                : 8.5.2022
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
#include "reosmeshvectorrenderingwidget.h"
#include "ui_reosmeshvectorrenderingwidget.h"

#include <qgsmeshrenderersettings.h>

#include "reosmesh.h"
#include "reosguicontext.h"
#include "reosencodedelement.h"
#include "reosstyleregistery.h"
#include "reosmeshscalarrenderingwidget.h"

struct VectorSettings
{
  QgsMeshRendererVectorSettings qgisSettings;
  int lifeTime = 15;
  int tracesFps = 15;
  int tracesMaxSpeed = 200;
  double tracesPersistence = 0.2;
  double tracesTailFactor = 1.0;
  double traceWidth = 2.0;
};

VectorSettings vectorSettings_( ReosMesh *mesh, const QString &datasetId )
{
  const ReosEncodedElement encodedSymbology = mesh->datasetVectorGroupSymbology( datasetId );

  VectorSettings ret;
  encodedSymbology.getData( QStringLiteral( "dynamic-traces-life-time" ), ret.lifeTime );
  encodedSymbology.getData( QStringLiteral( "dynamic-traces-max-speed" ), ret.tracesMaxSpeed );
  encodedSymbology.getData( QStringLiteral( "dynamic-traces-fps" ), ret.tracesFps );
  encodedSymbology.getData( QStringLiteral( "dynamic-traces-tails-persistence" ), ret.tracesPersistence );
  encodedSymbology.getData( QStringLiteral( "dynamic-traces-tail-factor" ), ret.tracesTailFactor );
  encodedSymbology.getData( QStringLiteral( "dynamic-traces-width" ), ret.traceWidth );

  if ( encodedSymbology.description() == QStringLiteral( "dataset-vector-symbology" ) )
  {
    QString docString;
    encodedSymbology.getData( QStringLiteral( "symbology" ), docString );

    QDomDocument doc( QStringLiteral( "dataset-vector-symbology" ) );

    if ( doc.setContent( docString ) )
    {
      QDomElement domElem = doc.firstChildElement( QStringLiteral( "vector-settings" ) );
      QgsReadWriteContext context;
      QgsMeshRendererVectorSettings vectorSettings;
      vectorSettings.readXml( domElem );
      ret.qgisSettings = vectorSettings;
    }
  }

  return ret;;
}

ReosMeshVectorRenderingWidget::ReosMeshVectorRenderingWidget( ReosMesh *mesh, const QString &datasetId, const ReosGuiContext &guiContext ):
  ReosStackedPageWidget( guiContext.parent() ),
  ui( new Ui::ReosMeshVectorRenderingWidget ),
  mMesh( mesh ),
  mDatasetId( datasetId ),
  mGuiContext( guiContext ),
  mWidthParameter( new ReosParameterDouble( QString(), false, this ) ),
  mMinimumLengthParameter( new ReosParameterDouble( QString(), false, this ) ),
  mMaximumLengthParameter( new ReosParameterDouble( QString(), false, this ) ),
  mMaximumTailLengthParameter( new ReosParameterDouble( QString(), false, this ) ),
  mTracesTailsPersitenceParameter( new ReosParameterDouble( QString(), false, this ) ),
  mTracesTailFactorParameter( new ReosParameterDouble( QString(), false, this ) ),
  mDynamicTraceWidthParameter( new ReosParameterDouble( QString(), false, this ) )
{
  ui->setupUi( this );

  ui->mSymbologyTypeCombo->addItem( tr( "Arrows" ), static_cast<int>( QgsMeshRendererVectorSettings::Arrows ) );
  ui->mSymbologyTypeCombo->addItem( tr( "Stream lines" ), static_cast<int>( QgsMeshRendererVectorSettings::Streamlines ) );
  ui->mSymbologyTypeCombo->addItem( tr( "Traces" ), static_cast<int>( QgsMeshRendererVectorSettings::Traces ) );

  ui->mColorModeCombo->addItem( tr( "Unique color" ), static_cast<int>( QgsInterpolatedLineColor::SingleColor ) );
  ui->mColorModeCombo->addItem( tr( "Color ramp" ), static_cast<int>( QgsInterpolatedLineColor::ColorRamp ) );

  ui->mStreamLineSeedingMethodCombo->addItem( tr( "On Mesh" ), static_cast<int>( QgsMeshRendererVectorStreamlineSettings::MeshGridded ) );
  ui->mStreamLineSeedingMethodCombo->addItem( tr( "Randomly" ), static_cast<int>( QgsMeshRendererVectorStreamlineSettings::Random ) );

  ui->mWidthWidget->setDouble( mWidthParameter );
  ui->mWidthWidget->enableSpacer( ReosParameterWidget::SpacerInMiddle );
  ui->mMinimumLengthWidget->setDouble( mMinimumLengthParameter );
  ui->mMinimumLengthWidget->enableSpacer( ReosParameterWidget::SpacerInMiddle );
  ui->mMaximumLengthWidget->setDouble( mMaximumLengthParameter );
  ui->mMaximumLengthWidget->enableSpacer( ReosParameterWidget::SpacerInMiddle );
  ui->mDynamicTracesWidth->setDouble( mDynamicTraceWidthParameter );
  ui->mDynamicTracesWidth->enableSpacer( ReosParameterWidget::SpacerInMiddle );

  ui->mMaximumTailLengthWidget->setDouble( mMaximumTailLengthParameter );
  ui->mMaximumTailLengthWidget->enableSpacer( ReosParameterWidget::SpacerInMiddle );
  ui->mTracesTailFactorDoubleWidget->setDouble( mTracesTailFactorParameter );
  ui->mTracesTailFactorDoubleWidget->enableSpacer( ReosParameterWidget::SpacerInMiddle );
  ui->mTracesTailsPersistentDoubleWidget->setDouble( mTracesTailsPersitenceParameter );
  ui->mTracesTailsPersistentDoubleWidget->enableSpacer( ReosParameterWidget::SpacerInMiddle );

  VectorSettings vectorSettings = vectorSettings_( mMesh, mDatasetId );

  //******************* QGIS settings
  QgsMeshRendererVectorSettings qgisSettings = vectorSettings.qgisSettings;

  ui->mSymbologyTypeCombo->setCurrentIndex( ui->mSymbologyTypeCombo->findData( static_cast<int>( qgisSettings.symbology() ) ) );
  ui->mColorModeCombo->setCurrentIndex( ui->mColorModeCombo->findData( static_cast<int>( qgisSettings.coloringMethod() ) ) );
  ui->mUniqueColorButton->setColor( qgisSettings.color() );
  mWidthParameter->setValue( qgisSettings.lineWidth() );

  QgsMeshRendererVectorArrowSettings arrowSettings = qgisSettings.arrowSettings();
  mMinimumLengthParameter->setValue( arrowSettings.minShaftLength() );
  mMaximumLengthParameter->setValue( arrowSettings.maxShaftLength() );
  ui->mShaftHeadSlider->setValue( static_cast<int>( 100 - arrowSettings.arrowHeadLengthRatio() * 100.0 ) );
  ui->mHeadWidthSpinBox->setValue( static_cast<int>( arrowSettings.arrowHeadWidthRatio() * 100 ) );

  QgsMeshRendererVectorStreamlineSettings streamLineSettings = qgisSettings.streamLinesSettings();
  ui->mStreamLineSeedingMethodCombo->setCurrentIndex( ui->mStreamLineSeedingMethodCombo->findData(
        static_cast<int>( streamLineSettings.seedingMethod() ) ) );
  ui->mStreamLineDensitySpinBox->setEnabled( streamLineSettings.seedingMethod() == QgsMeshRendererVectorStreamlineSettings::Random );
  ui->mStreamLineDensitySpinBox->setValue( static_cast<int>( streamLineSettings.seedingDensity() * 100 ) );

  QgsMeshRendererVectorTracesSettings traceSettings = qgisSettings.tracesSettings();
  mMaximumTailLengthParameter->setValue( traceSettings.maximumTailLength() );
  ui->mTracesParticulesCountSpinBox->setValue( traceSettings.particlesCount() );
  switch ( traceSettings.maximumTailLengthUnit() )
  {
    case Qgis::RenderUnit::Millimeters:
      ui->mTailUnitCombo->setCurrentIndex( 0 );
      break;
    case Qgis::RenderUnit::MapUnits:
      break;
    case Qgis::RenderUnit::Pixels:
      ui->mTailUnitCombo->setCurrentIndex( 2 );
      break;
    case Qgis::RenderUnit::Percentage:
      break;
    case Qgis::RenderUnit::Points:
      ui->mTailUnitCombo->setCurrentIndex( 1 );
      break;
    case Qgis::RenderUnit::Inches:
    case Qgis::RenderUnit::Unknown:
      break;
    case Qgis::RenderUnit::MetersInMapUnits:
      ui->mTailUnitCombo->setCurrentIndex( 3 );
      break;
  }
  //****************

  ui->mTracesLifeTimeSpinBox->setValue( vectorSettings.lifeTime );
  ui->mTracesFPSSpinBox->setValue( vectorSettings.tracesFps );
  ui->mTracesMaxSpeedSpinBox->setValue( vectorSettings.tracesMaxSpeed );
  mDynamicTraceWidthParameter->setValue( vectorSettings.traceWidth );
  mTracesTailFactorParameter->setValue( vectorSettings.tracesTailFactor );
  mTracesTailsPersitenceParameter->setValue( vectorSettings.tracesPersistence );

  updateWidget();

  connect( ui->mSymbologyTypeCombo, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, [this]
  {
    updateMeshSettings();
    updateWidget();
  } );

  connect( ui->mColorModeCombo, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, [this]
  {
    updateMeshSettings();
    updateWidget();
  } );
  connect( ui->mUniqueColorButton, &ReosColorButton::colorChanged, this, &ReosMeshVectorRenderingWidget::updateMeshSettings );

  connect( ui->mColorRampToolButton, &QToolButton::clicked, this, [this]
  {
    addOtherPage( new ReosMeshScalarRenderingWidget( mMesh->vectorColorShaderSettings(), ReosGuiContext( mGuiContext, this ) ), true );
  } );
  connect( ui->mShaftHeadSlider, &QSlider::valueChanged, this, &ReosMeshVectorRenderingWidget::updateMeshSettings );
  connect( mWidthParameter, &ReosParameter::valueChanged, this, &ReosMeshVectorRenderingWidget::updateMeshSettings );
  connect( mMinimumLengthParameter, &ReosParameter::valueChanged, this, &ReosMeshVectorRenderingWidget::updateMeshSettings );
  connect( mMaximumLengthParameter, &ReosParameter::valueChanged, this, &ReosMeshVectorRenderingWidget::updateMeshSettings );
  connect( ui->mHeadWidthSpinBox, QOverload<int>::of( &QSpinBox::valueChanged ), this, &ReosMeshVectorRenderingWidget::updateMeshSettings );

  connect( ui->mStreamLineSeedingMethodCombo, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, [this]
  {
    QgsMeshRendererVectorStreamlineSettings::SeedingStartPointsMethod method =
    static_cast<QgsMeshRendererVectorStreamlineSettings::SeedingStartPointsMethod>( ui->mStreamLineSeedingMethodCombo->currentData().toInt() );
    ui->mStreamLineDensitySpinBox->setEnabled( method == QgsMeshRendererVectorStreamlineSettings::Random );

    updateMeshSettings();
  } );

  connect( ui->mStreamLineDensitySpinBox, QOverload<int>::of( &QSpinBox::valueChanged ), this, &ReosMeshVectorRenderingWidget::updateMeshSettings );

  ui->mTailUnitCombo->addItem( tr( "Millimeters" ) );
  ui->mTailUnitCombo->addItem( tr( "Points" ) );
  ui->mTailUnitCombo->addItem( tr( "Pixels" ) );
  ui->mTailUnitCombo->addItem( tr( "Meters at Scale" ) );

  connect( mMaximumTailLengthParameter, &ReosParameter::valueChanged, this, &ReosMeshVectorRenderingWidget::updateMeshSettings );
  connect( ui->mTailUnitCombo, QOverload<int>::of( &QComboBox::currentIndexChanged ), this, &ReosMeshVectorRenderingWidget::updateMeshSettings );
  connect( ui->mTracesParticulesCountSpinBox, QOverload<int>::of( &QSpinBox::valueChanged ), this, &ReosMeshVectorRenderingWidget::updateMeshSettings );

  connect( ui->mTracesLifeTimeSpinBox, QOverload<int>::of( &QSpinBox::valueChanged ), this, &ReosMeshVectorRenderingWidget::updateMeshSettings );
  connect( ui->mTracesParticulesCountSpinBox, QOverload<int>::of( &QSpinBox::valueChanged ), this, &ReosMeshVectorRenderingWidget::updateMeshSettings );
  connect( ui->mTracesFPSSpinBox, QOverload<int>::of( &QSpinBox::valueChanged ), this, &ReosMeshVectorRenderingWidget::updateMeshSettings );
  connect( ui->mTracesMaxSpeedSpinBox, QOverload<int>::of( &QSpinBox::valueChanged ), this, &ReosMeshVectorRenderingWidget::updateMeshSettings );
  connect( mTracesTailsPersitenceParameter, &ReosParameter::valueChanged, this, &ReosMeshVectorRenderingWidget::updateMeshSettings );
  connect( mTracesTailFactorParameter, &ReosParameter::valueChanged, this, &ReosMeshVectorRenderingWidget::updateMeshSettings );
  connect( mDynamicTraceWidthParameter, &ReosParameter::valueChanged, this, &ReosMeshVectorRenderingWidget::updateMeshSettings );

  ui->mBackButton->setIconSize( ReosStyleRegistery::instance()->toolBarIconSize() );
  connect( ui->mBackButton, &QPushButton::clicked, this, &ReosStackedPageWidget::backToPreviousPage );
}

ReosMeshVectorRenderingWidget::~ReosMeshVectorRenderingWidget()
{
  delete ui;
}

void ReosMeshVectorRenderingWidget::updateMeshSettings()
{
  //****** QGIS symbology
  QgsMeshRendererVectorSettings vectSettings = vectorSettings_( mMesh, mDatasetId ).qgisSettings;
  vectSettings.setSymbology( static_cast<QgsMeshRendererVectorSettings::Symbology>( ui->mSymbologyTypeCombo->currentData().toInt() ) );
  vectSettings.setColoringMethod( static_cast<QgsInterpolatedLineColor::ColoringMethod>( ui->mColorModeCombo->currentData().toInt() ) );
  vectSettings.setColor( ui->mUniqueColorButton->color() );
  vectSettings.setLineWidth( mWidthParameter->value() );

  QgsMeshRendererVectorArrowSettings arrowSettings = vectSettings.arrowSettings();
  arrowSettings.setArrowHeadLengthRatio( ( 100 - ui->mShaftHeadSlider->value() ) / 100.0 );
  arrowSettings.setMinShaftLength( mMinimumLengthParameter->value() );
  arrowSettings.setMaxShaftLength( mMaximumLengthParameter->value() );
  arrowSettings.setArrowHeadWidthRatio( ui->mHeadWidthSpinBox->value() / 100.0 );

  QgsMeshRendererVectorStreamlineSettings streamLineSettings = vectSettings.streamLinesSettings();
  streamLineSettings.setSeedingMethod( static_cast < QgsMeshRendererVectorStreamlineSettings::SeedingStartPointsMethod >(
                                         ui->mStreamLineSeedingMethodCombo->currentData().toInt() ) );
  streamLineSettings.setSeedingDensity( ui->mStreamLineDensitySpinBox->value() / 100.0 );

  QgsMeshRendererVectorTracesSettings traceSettings = vectSettings.tracesSettings();
  traceSettings.setMaximumTailLength( mMaximumTailLengthParameter->value() );
  traceSettings.setParticlesCount( ui->mTracesParticulesCountSpinBox->value() );
  switch ( ui->mTailUnitCombo->currentIndex() )
  {
    case 0:
      traceSettings.setMaximumTailLengthUnit( Qgis::RenderUnit::Millimeters );
      break;
    case 1:
      traceSettings.setMaximumTailLengthUnit( Qgis::RenderUnit::Points );
      break;
    case 2:
      traceSettings.setMaximumTailLengthUnit( Qgis::RenderUnit::Pixels );
      break;
    case 3:
      traceSettings.setMaximumTailLengthUnit( Qgis::RenderUnit::MetersInMapUnits );
      break;
  }

  vectSettings.setArrowsSettings( arrowSettings );
  vectSettings.setStreamLinesSettings( streamLineSettings );
  vectSettings.setTracesSettings( traceSettings );

  QDomDocument doc( QStringLiteral( "dataset-vector-symbology" ) );
  doc.appendChild( vectSettings.writeXml( doc ) ) ;

  ReosEncodedElement encodedElem( QStringLiteral( "dataset-vector-symbology" ) );
  QString docString = doc.toString();
  encodedElem.addData( QStringLiteral( "symbology" ), docString );
  //*******************
  encodedElem.addData( QStringLiteral( "dynamic-traces-life-time" ), ui->mTracesLifeTimeSpinBox->value() );
  encodedElem.addData( QStringLiteral( "dynamic-traces-max-speed" ), ui->mTracesMaxSpeedSpinBox->value() );
  encodedElem.addData( QStringLiteral( "dynamic-traces-fps" ), ui->mTracesFPSSpinBox->value() );
  encodedElem.addData( QStringLiteral( "dynamic-traces-tails-persistence" ), mTracesTailsPersitenceParameter->value() );
  encodedElem.addData( QStringLiteral( "dynamic-traces-tail-factor" ), mTracesTailFactorParameter->value() );
  encodedElem.addData( QStringLiteral( "dynamic-traces-width" ), mDynamicTraceWidthParameter->value() );

  mMesh->setDatasetVectorGroupSymbology( encodedElem, mDatasetId );
}

void ReosMeshVectorRenderingWidget::updateWidget()
{
  QgsMeshRendererVectorSettings::Symbology type = static_cast<QgsMeshRendererVectorSettings::Symbology>( ui->mSymbologyTypeCombo->currentData().toInt() );
  switch ( type )
  {
    case QgsMeshRendererVectorSettings::Arrows:
      ui->mArrowShapeGroupBox->setVisible( true );
      ui->mStreamLineGroupBox->setVisible( false );
      ui->mTracesGroupBox->setVisible( false );
      break;
    case QgsMeshRendererVectorSettings::Streamlines:
      ui->mArrowShapeGroupBox->setVisible( false );
      ui->mStreamLineGroupBox->setVisible( true );
      ui->mTracesGroupBox->setVisible( false );
      break;
    case QgsMeshRendererVectorSettings::Traces:
      ui->mArrowShapeGroupBox->setVisible( false );
      ui->mStreamLineGroupBox->setVisible( false );
      ui->mTracesGroupBox->setVisible( true );
      break;
  }

  QgsInterpolatedLineColor::ColoringMethod colorMeth = static_cast<QgsInterpolatedLineColor::ColoringMethod>( ui->mColorModeCombo->currentData().toInt() );

  switch ( colorMeth )
  {
    case QgsInterpolatedLineColor::SingleColor:
      ui->mUniqueColorButton->setVisible( true );
      ui->mColorRampToolButton->setVisible( false );
      break;
    case QgsInterpolatedLineColor::ColorRamp:
      ui->mUniqueColorButton->setVisible( false );
      ui->mColorRampToolButton->setVisible( true );
      break;
  }
}
